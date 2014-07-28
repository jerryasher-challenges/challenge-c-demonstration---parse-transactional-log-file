// Jerry Asher
// ja2038@gmail.com
// 623 455 6342
// copyright 2014 - Jerry Asher

#ifndef _TXNSTATS_H
#define _TXNSTATS_H

int txnstats_open();
int txnstats_insert(char *id, char *timestamp);
int txnstats_find_id(char *id, char *timestamp);
int txnstats_recordcount();
int txnstats_close();
void txnstats_print();

#endif
