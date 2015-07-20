/* getPass.hpp */
#ifndef _GETPASS_H
#define _GETPASS_H


#include <string.h>
#include <iostream>
#include <stdio.h>
#include <list>
#include <map>
#include <iostream>
#include <string>
#include <stdlib.h>


using namespace std;

/* additional includes for -lcrypt*/
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

extern string *PASSWORD;
extern bool DEBUG;
bool getPass();

#define DB_UID  "na8"
#define OR_DATABASE "testDB"
#define PQ_DATABASE "pqUser"
#define PASSWORD_FILE "pq.pass"
#define NUM_DES_BLOCKS  2
#define DES_BLOCK_SIZE  64
#define BITS_IN_A_BYTE  8

typedef list<string>::iterator dRI_t;


class dataRow {

  public:
      string ID;
      list<string> sList;
      dataRow() {  }
      dataRow(string s) { sList.push_back(s); }
      dataRow(string s1, string s2, string s3) { sList.push_back(s1); sList.push_back(s2); sList.push_back(s3); }
      void add_record( ) { }
      void add_record(string s) { sList.push_back(s); }
      void add_record(string *s) { sList.push_back(*s); }
      void print_record() { for(dRI_t i=sList.begin(); i!=sList.end(); i++) cout <<"  "<<*i<<endl; cout << endl; }
      void print_table_record() {
        for(dRI_t i=sList.begin(); i!=sList.end(); i++) cout <<"<td>"<<*i<<"</td>\n"; cout << endl; }
};


#endif
