/* pqDB.cpp */

#include "pqDB.hpp"

extern string DB_HOST;

typedef map<int, dataRow *> dataRow_t;
extern string db_err_msg;

//------------------------------------------------------------
bool pqDB::pq_check()
{
  pq_open();
  return true;

  if(database_opened==0 || C==0) {
      if(pq_open()==0) return false;
      return true;
  }

  pq_lock();

  work *W;

  try {
      W= new work(*C);
  }
  catch (const exception &e) {
       cerr << "pq_check() :: " << e.what() << endl; fflush(stdout);
       db_err_msg= e.what();
       delete W; 
       pq_unlock();
       pq_close();
       if(pq_open()) {
           cerr << "Successfully opened database\n"; 
           return true;;
       }
       cerr << "Failed to open database\n"; 
       return false;
  }
  catch (...) {
       cerr << "pq_check Unhandled exception" << endl;
  }
  delete W;
  pq_unlock();

  return true;
}

//------------------------------------------------------------
void pqDB::pq_lock()
{
  pthread_mutex_lock(&pq_mutex);

  if(NAC_THREADS ==0) return;

  while(in_session) {
      pthread_cond_wait(&pq_cond, &pq_mutex);
  }
  in_session=true;
}

//------------------------------------------------------------
void pqDB::pq_unlock()
{
  if(NAC_THREADS>0) {
      for (uint i = 1; i < NAC_THREADS; i++) pthread_cond_signal(&pq_cond);
      in_session=false;
  }
  pthread_mutex_unlock(&pq_mutex);
}

//------------------------------------------------------------
bool pqDB::pq_open()
{
  if(database_opened && C!=0 && C->is_open() ) return true;

  pq_lock();

  char connection_info[128];
  bzero(connection_info, 128);

  if(PASSWORD==0 || PASSWORD->length() == 0) {
      if(!getPass()) return false;
  }

  if(DB_HOST.length() == 0) DB_HOST = LOCAL_DB_HOST;

  sprintf( connection_info, "host=%s dbname=%s user=%s password=%s", 
	DB_HOST.c_str(), PQ_DATABASE, DB_UID, PASSWORD->c_str());

  try {
    delete C;
    C = 0;
    C = new connection (connection_info);
  }
  catch(const exception &e) {
      cerr << "pqDB::pq_open() " << e.what() << endl; fflush(stderr);
       db_err_msg=e.what();
      return false;
  }
  catch (...) {
       // This is really unexpected (see above)
       cerr << "pqDB::pq_open() Unhandled exception" << endl;
  }
  database_opened=1;
  pq_unlock();
  return true;
}

//------------------------------------------------------------
void pqDB::pq_close()
{
  if(database_opened==0 && C==0) return;
  delete C;
  C=0;
  database_opened=0;
}


//---------------------------------------------
bool pqDB::pq_exec(const string buffer)
{
  bool myRet=true;

  if(!pq_check()) { return false; };

  pq_lock();

  work *W= new work(*C);

  try {
      W->exec(buffer);
      W->commit();
  }
  catch (const exception &e) {
      cerr << "Exception thrown for (PQ) execute_command: "<< e.what() << endl; fflush(stderr);
      db_err_msg= e.what();
      cerr << "-------" << endl << buffer << endl;
      myRet=false;
  }
  catch (...) {
       // This is really unexpected (see above)
       cerr << "pq_exec Unhandled exception" << endl;
  }
  delete W;

  pq_unlock();

  return myRet;
}

//---------------------------------------------
string pqDB::pq_string (const string buffer)
{
  string *myS;
  myS=0;

  if(!pq_check()) { return ""; };

  pq_lock();
  work *W= new work(*C);

  try {
      result R = W->exec(buffer);
      if(R.size()==0) return 0;
      myS = new string( R[0][0].c_str());
      W->commit();
  }
  catch (const exception &e) {
      cerr<<"Exception thrown for (PQ) pq_string(): "<< e.what() << endl << buffer << endl;  fflush(stderr);
      db_err_msg=e.what();
      W->commit();
      myS=0;
  }
  catch (...) {
       // This is really unexpected (see above)
       cerr << "pq_string Unhandled exception" << endl;
  }
  delete W;
  pq_unlock();

  if(myS==0) return "";

  return *myS;
}

//---------------------------------------------
int pqDB::pq_count (const string buffer)
{
  if(!pq_check()) { return -1; };

  int ret = -1;

  pq_lock();
  work *W= new work(*C);

  try {
      result R = W->exec(buffer);
      result::const_iterator rit = R.begin();
      if (rit != R.end()) rit->at(0).to(ret);
      W->commit();
  }
  catch (const exception &e) {
    cerr << "Exception thrown for (PQ) pq_count(): "<< e.what() << endl; fflush(stderr);
      db_err_msg=e.what();
    ret=-1;
  }
  catch (...) {
       // This is really unexpected (see above)
       cerr << "pq_count Unhandled exception" << endl;
  }

  delete W;
  pq_unlock();
  return ret;
} 

//---------------------------------------------
dataRow_t *pqDB::pq_rows (const string buffer)
{
  dataRow_t *my_list = new dataRow_t;

  if(!pq_check()) { return 0; };

  pq_lock();
  work *W= new work(*C);

  try {

      result R = W->exec(buffer);

      unsigned int myCounter = 0;

      // Process each successive result tuple
      for (result::const_iterator c = R.begin(); c != R.end(); ++c) 
      {
         dataRow *dR = new dataRow();

         unsigned int loc=0, total=c.size();

         for( ; loc < total; loc++) {
             string *s = new string( c[loc].as(string()) );
             dR->add_record(*s);
         }
         my_list->insert ( make_pair ( myCounter++, dR) );
      }
      W->commit();
  }
  catch (const exception &e) {
    // All exceptions thrown by libpqxx are derived from std::exception
    cerr << "pq_rows Exception: " << e.what() << endl; fflush(stdout);
      db_err_msg=e.what();
    cerr << "pq_rows buffer: " << buffer << endl; fflush(stdout);
  }
  catch (...) {
    // This is really unexpected (see above)
    cerr << "pq_rows Unhandled exception" << endl;
    cerr << "pq_rows buffer: " << buffer << endl;
  }
  delete W;
  pq_unlock();
  return my_list;
}


typedef map<string, int> map_t;

//--------------------------------------------------------------
map<string, int> pqDB::pq_list(const string buffer)
{
  map_t *my_list = new map_t();

  if(!pq_check()) { return *my_list; };

  pq_lock();

  work *W= new work(*C);

  try {

      result R = W->exec(buffer);

      // Process each successive result tuple
      for (result::const_iterator c = R.begin(); c != R.end(); ++c) 
      {
        string *myS = new string (c[0].as( string()) );
        //my_list[ *myS ] = 1; 
        //my_list->insert( make_pair(*myS, c.num()) );
        my_list->insert( make_pair(*myS, 1) );
      }
      W->commit();
  }
  catch (const exception &e) {
       // All exceptions thrown by libpqxx are derived from std::exception
       cerr << "pq_list() : (" << buffer << ") : " << e.what() << endl; fflush(stdout);
      db_err_msg=e.what();
  }
  catch (...) {
       // This is really unexpected (see above)
       cerr << "pq_list Unhandled exception" << endl;
  }
  delete W;
  pq_unlock();
  return *my_list;
}

//---------------------------------
string pqDB::get_current_date()
{
  string myS;

  if(!pq_check()) { return myS; };
  myS = pq_string("select to_char(current_timestamp, 'YYYY-MM-DD:HH24:MI:SS')");
  return myS;
}

//--------------------------------------------------------------
list_t *pqDB::pq_list_t(const string buffer)
{
  list_t *my_list = new list_t();

  if(!pq_check()) { return 0; };

  pq_lock();

  work *W= new work(*C);

  try {

      result R = W->exec(buffer);

      // Process each successive result tuple
      for (result::const_iterator c = R.begin(); c != R.end(); ++c)
      {
        string myS(c[0].as( string()));
        my_list->push_back(myS);
      }
      W->commit();
  }
  catch (const exception &e) {
       // All exceptions thrown by libpqxx are derived from std::exception
       cerr << "pq_list() : (" << buffer << ") : " << e.what() << endl; fflush(stdout);
      db_err_msg=e.what();
  }
  catch (...) {
       // This is really unexpected (see above)
       cerr << "pq_list Unhandled exception" << endl;
  }
  delete W;
  pq_unlock();
  return my_list;
}

