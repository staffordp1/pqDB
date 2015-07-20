#ifndef DB_H
#define DB_H

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <list>
#include <map>

#include <iostream>
#include <libpq-fe.h>
#include <pqxx/pqxx>

using namespace pqxx;
using namespace std;

#include "getPass.hpp"
#define LOCAL_HOST "localhost"
#define LOCAL_DB_HOST "localhost"
extern string DB_HOST;

extern uint NAC_THREADS;

typedef list<string>::iterator dRI_t;
typedef list<string> list_t;
typedef map<int, dataRow *> dataRow_t;
typedef map<string, int> map_t;


class pqDB {

  char *pqHost;
  connection *C;

  int database_opened;
  pthread_mutex_t pq_mutex;
  pthread_cond_t pq_cond;
  bool in_session;
  void pq_lock();
  void pq_unlock();

public:

  string db_err_msg;

  pqDB(string host) {

	database_opened=0;   

	pthread_mutex_init(&pq_mutex, NULL); 
        pthread_cond_init(&pq_cond, NULL);
        in_session=false;

	C=0; 

        if(host.length() == 0) {
            uint len = strlen(LOCAL_DB_HOST) + 1;
            len += len%8;
            pqHost = (char *)malloc(len);
            bzero(pqHost, len);
            strcpy(pqHost, LOCAL_DB_HOST);
        } else {
            uint len = host.length()+1;
            len += len%8;
            pqHost = (char *)malloc(len);
            bzero(pqHost, len);
            strcpy(pqHost, host.c_str());
        }
  }
  pqDB() {
        database_opened=0;
        pthread_mutex_init(&pq_mutex, NULL);
        pthread_cond_init(&pq_cond, NULL);
        in_session=false;
        C=0;
        uint len = strlen(DB_HOST.c_str()) + 1;
        len += len%8;
        pqHost = (char *)malloc(len);
        bzero(pqHost, len);
        strcpy(pqHost, DB_HOST.c_str());
  }

  ~pqDB() { if(database_opened) pq_close(); }
  void reset() { if(in_session) pq_unlock(); }
  void commit();
  bool pq_open();
  void pq_close();
  bool pq_exec(const string buffer);
  string pq_string (const string );
  int pq_count (const string );
  dataRow_t *pq_rows(const string buffer);
  map_t pq_list(const string buffer);
  string get_current_date();
  bool pq_check();
  list_t *pq_list_t(const string buffer);
};
#endif
