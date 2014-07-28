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

// binary search tree approach

// for each request, insert request node into a binary tree
// for each response, search tree for request and process

// note: supports only one, static binary tree at a time.

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
// operations on a binary tree node
// private to this file

// our binary tree node
typedef struct txnnode {
  char id[15];            // MSN field eg: "160A0EC77BAA83"
  time_t request_time;    // timestamp of request
  int response_time_secs; // complete transaction response time in seconds
  struct txnnode *left;   // binary tree left
  struct txnnode *right;  // binary tree right
} transaction;

// our binary tree root
static transaction *txnroot = NULL;

// make a node for each request
// doesn't put it into tree
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
    txn->left = NULL;
    txn->right = NULL;
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
  txnroot = NULL;
  return 0;
}

// do nothing in this implementation
// memory will be freed as process exits
int txnstats_close()
{
  return 0;
}

transaction * _txnstats_insert(transaction *head, char *id, time_t time)
{

  if (head == NULL) {
    head = make_transaction_request_node(id, time);
    if (head == NULL) {
      printf("_txnstats_insert: mtrn returned null\n");
    }
  } else {

    // returns < 0 if head < (id, time)
    int delta = compare_transactions(head, id, time);

    if (delta < 0) {
      head->right = _txnstats_insert(head->right, id, time);
    } else if (delta > 0) {
      head->left = _txnstats_insert(head->left, id, time);
    }
  }
  return head;
}

// insert a node into the binary tree
// returns 1 on success
int txnstats_insert(char *id, char *timestamp)
{
  time_t time = parse_timestamp(timestamp);
  if (txnroot == NULL) {
    txnroot = make_transaction_request_node(id, time);
    if (txnroot == NULL) {
      printf("txnstats_insert: mtrn returned null\n");
    }
  } else {
    txnroot = _txnstats_insert(txnroot, id, time);
    if (txnroot == NULL) {
      printf("txnstats_insert: _txnstats_insert returned null\n");
    }
  }
  return (txnroot != NULL);
}

transaction *_txnstats_find_id(transaction *head, char *id)
{

  transaction *found;

  if (head == NULL) {
    found = NULL;
  } else {
    int comparison = strcmp(id, head->id);
    if (comparison == 0) {
      found = head;
    } else if (comparison < 0) {
      found = _txnstats_find_id(head->left, id);
    } else {
      found =_txnstats_find_id(head->right, id);
    }
  }
  return found;
}

// returns response time if transaction was found
// otherwise returns -1
int txnstats_find_id(char *id, char *timestamp)
{
  transaction *txn = _txnstats_find_id(txnroot, id);

  int response_time;

  if (txn) {
    time_t time = parse_timestamp(timestamp);
    response_time = process_transaction_response(txn, time);
  } else {
    response_time = -1;
  }

  return response_time;
}

int _txnstats_recordcount(transaction *txn)
{
  int count = 0;

  if(txn) {
    count = _txnstats_recordcount(txn->left);
    count++;
    count += _txnstats_recordcount(txn->right);
  }
  return count;
}

int txnstats_recordcount()
{
  return _txnstats_recordcount(txnroot);
}

void _txnstats_print(transaction *txn) 
{
  if(txn) {
    _txnstats_print(txn->left);
    char timestr[27];
    ctime_r(&txn->request_time, timestr);
    printf("transaction %s at %.*s response time %d\n",
           txn->id,
           (int)strlen(timestr)-1,
           timestr,
           txn->response_time_secs);
    _txnstats_print(txn->right);
  }
}

void txnstats_print()
{
  _txnstats_print(txnroot);
}

