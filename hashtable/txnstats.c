// Jerry Asher
// ja2038@gmail.com
// 623 455 6342
// copyright 2014 - Jerry Asher

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// "c test"
// see "c test.txt" for specifications
// see "data.txt" for sample data

// hashtable approach

// USES Troy Hanson's uthash macros
// https://github.com/troydhanson/uthash[GitHub project page].

#include "uthash.h"

// for each request, insert request node into a hash table
// for each response, lookup in hash for request and process

// helper - parse our timestamp
time_t parse_timestamp(char *timestamp)
{

  int month, dd, yy, hh, mm, ss;
  short numfields = sscanf(timestamp, "%2d%2d%2d%2d%2d%2d",
                           &month, &dd, &yy, &hh, &mm, &ss);

  time_t time = (time_t) -1;

  if (6 == numfields) {
      
    struct tm t;
    t.tm_mon = month - 1;
    t.tm_mday = dd;
    t.tm_year = 100 + yy;
    t.tm_hour = hh;
    t.tm_min = mm;
    t.tm_sec = ss;
    t.tm_isdst = -1;

    time = mktime(&t);
  }
  return time;
}

//////////////////////////////
// operations on a hash node
// private to this file

// our hash key, value node
typedef struct txnnode {
  char id[15];            // Hash KEY - MSN field eg: "160A0EC77BAA83"
  time_t request_time;    // timestamp of request
  int response_time_secs; // complete transaction response time in seconds

  UT_hash_handle hh;      // makes this structure hashable

} transaction;

// our hash table
static transaction *txns = NULL;

// make a node for each request
// doesn't put it into the hash table
transaction *make_transaction_request_node(char *id, time_t time)
{
  // allocate a transaction structure
  transaction *txn = malloc(sizeof(transaction));
  if (txn) {
    // init id
    strcpy(txn->id, id);
    txn->request_time = time;

    // init rest of node
    txn->response_time_secs = -1;
  }
  return txn;
}

// given the request transaction and the response timestamp
// determines the delta response time in seconds (rts)
int process_transaction_response(transaction *txn, time_t response_time)
{
  int rts = -1;
  if (txn) {
    rts = txn->response_time_secs = difftime(response_time, txn->request_time);
  }
  
  return rts;

}

// define a way to compare two transactions for insertion sorting
// return < 0 if txn comes before (id2, time2)
// return  0  if txn is (id2, time2)
// return > 0 if txn comes after (id2, time2)

int compare_transactions(transaction *txn, char *id2, time_t time2)
{
  int idcomparison = strcmp(txn->id, id2);
  if (idcomparison == 0) {
    int delta = difftime(txn->request_time, time2);
    return delta;
  } else {
    return idcomparison;
  }
}

//////////////////////////////
// operations on the binary tree
// global to users of the binary tree

// initialize the static structure
int txnstats_open()
{
  txns = NULL;
  return 0;
}

// do nothing in this implementation
// memory will be freed as process exits
int txnstats_close()
{
  return 0;
}

// insert a node into the binary tree
// returns 1 on success
int txnstats_insert(char *id, char *timestamp)
{
  time_t time = parse_timestamp(timestamp);
  
  transaction *txn = make_transaction_request_node(id, time);
  if (txn != NULL) {
    // ASSUME THE REQUEST ID WILL BE UNIQUE IN THE DATA.TXT FILE
    HASH_ADD_STR(txns, id, txn); /* id is the key */
    return 1;
  }
  return 0;
}

// returns response time if transaction was found
// otherwise returns -1
int txnstats_find_id(char *id, char *timestamp)
{
  transaction *txn;
  HASH_FIND_STR(txns, id, txn);

  int response_time;

  if (txn) {
    time_t time = parse_timestamp(timestamp);
    response_time = process_transaction_response(txn, time);
  } else {
    response_time = -1;
  }

  return response_time;
}

int txnstats_recordcount()
{
  return HASH_COUNT(txns);
}

void txnstats_print() 
{
  if(txns) {
    transaction *txn;
    for (txn = txns; txn != NULL; txn=txn->hh.next) {
      char timestr[27];
      ctime_r(&txn->request_time, timestr);
      printf("transaction %s at %.*s response time %d\n",
             txn->id,
             (int)strlen(timestr)-1,
             timestr,
             txn->response_time_secs);
    }
  }
}



