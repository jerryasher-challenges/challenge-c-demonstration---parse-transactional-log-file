// Jerry Asher
// ja2038@gmail.com
// 623 455 6342
// copyright 2014 - Jerry Asher

#include <stdio.h>
#include <string.h>

// "c test"
// see "c test.txt" for specifications
// see "data.txt" for sample data

#include "txnstats.h"

int main(int argc, char *argv[])
{
  char *logname = "data.txt";
  FILE *logfile;
  if (argc > 1) {
    if (strcmp(argv[1], "-h") == 0) {
      printf("Usage: report: [OPTION] [FILE]\n"
             "Generate a report for log file FILE. "
             "If filename is not specified, data.txt is used.\n"
             "-h print help message.\n");
      return 0;
    } else {
      logname = argv[1];
    }
  }

  logfile = fopen (argc > 1 ? argv[1] : logname, "r");
  if (logfile == NULL) {
    perror ("Error opening log file");
  } else {

    // init transaction search structure
    if (txnstats_open() == 0) {

      int completed_transactions = 0;
      int missing_requests = 0;
      int total_elapsed_seconds = 0; 

      // assume txn request always comes before txn response in log
      // file assume txn ids are a 14 char string

      // assume specification in "c test.txt" is wrong and timestamp
      // is of form MMDDYYHHMMSS and NOT MMDDYYYYMMSS which doesn't
      // make much sense

      // assume timestamps can occur within standard unix time (after
      //   unix epoch before 2038)
      // don't assume every request has a response or vice-versa.

      int lines = 0;
      int requests = 0;
      int responses = 0;

      int minresponse = -1;
      int maxresponse = 0;
      char minid[15], maxid[15];
      minid[0] = '\0';
      maxid[0] = '\0';

      int txn_frequencies[31];

      int i; for (i = 0; i < 31; i++) {txn_frequencies[i] = 0;}

      short keepreading = 1; // set to 0 on EOF or bad data
      short noerr = 1;       // set to 0 if underlying txn library fails
      while (keepreading && noerr) {

        // process each line by
        // 1. read needed input to obtain timestamp, txn/resp, msn id
        // 2. insert requests into search structure AND process responses
        // 3. then flush rest of line w getc until newline or eof

        // so then

        // 1. read needed input by fscanf the line
        // input line is at least 95 characters containing a pattern like
        // @{010813124706$RPMS .*MSN12010EC77BA01C|.*
        // followed by indeterminate more characters, ended with a newline

        // assumption: the MSN field can be uniquely determined by
        // searching for "|MSN" and the field itself is contained
        // within the first 100 chars of a line.

        char timestamp[13];
        char txntype[5];
       
#define LINELEN 100
        char line[LINELEN];
        if (fgets(line, LINELEN, logfile) != NULL) {
          // printf("Read %d chars of line %s\n", strlen(line), line);
          short numfields = sscanf(line, "@{%12s$%4s", timestamp, txntype);
          if (numfields == 2) {

            char id[15];
            id[0] = '\0';

            // scan for the MSN field, skip over what we just read
            char *msn = strstr(&line[19], "|MSN");
            if (msn) {

              // now pick up the msn field
              strncpy(id, msn+4, 14);
              id[14] = '\0';

              // printf("timestamp: %s txntype: %s id:%s\n", timestamp, txntype, id);

              // 2. process that input
              if (0 == strncmp(txntype, "RQMS", 4)) {

                requests++;
                // we read a request so insert id and timestamp into
                // [search structure]
                if  (1 != (noerr = txnstats_insert(id, timestamp))) {
                  printf(" Error inserting request timestamp %s, id %s.\n",
                         timestamp, id);
                }
              } else {
                responses++;
                // we read a response so search for request and process delta time
                int response_time = txnstats_find_id(id, timestamp);
                if (response_time >= 0) {
              
                  total_elapsed_seconds += response_time;
                  completed_transactions++;

                  if (response_time >= 0 && response_time <= 30) {
                    txn_frequencies[response_time]++;
                  }

                  if (minresponse == -1 || response_time < minresponse) {
                    minresponse = response_time;
                    strcpy(minid, id);
                  }

                  if (response_time > maxresponse) {
                    maxresponse = response_time;
                    strcpy(maxid, id);

                    /* printf("Found max time response: id is %s(%s)" */
                    /*     " response_time is %d\n", */
                    /*     id, maxid, maxresponse); */

                  }
                } else {
                  // if request was not found, just drop response on floor
                  // as we assume requests precede responses
                  missing_requests++;
                }
              }

              // 3. All done so flush rest of line w getc until \n or eof

              short keepflushing = 1;
              char c;
              while(keepflushing && (c = fgetc(logfile)) != EOF) {
                switch (c)
                  {
                  case EOF:
                    // printf(" EOF\n");
                    keepreading = 0;
                    keepflushing = 0;
                    break;
                  case '\n':
                    // printf(" newline\n");
                    keepflushing = 0;
                    break;
                  default:;
                    // printf(" %c", c);
                  }
              }

              // printf(" after flush keepreading: %d\n", keepreading);
            } else {
              printf(" could not find MSN field on record %d\n", lines + 1);
              keepreading = 0;
            }
          } else {
            if (numfields != EOF) {
              printf(" bad read at line %d. numfields = %d\n",
                     lines + 1, numfields);
            }
            keepreading = 0;
          }
        }

        if (feof(logfile)) {
          // printf("feof\n");
          keepreading = 0;
        } 

        if (ferror(logfile)) {
          printf("ferror reading logfile\n");
          keepreading = 0;
        }

        if (keepreading && noerr) lines++;

      }

      fclose (logfile);

      printf("Processed:\n"
             "%d records\n"
             "%d complete transactions\n"
             "%d responses were missing requests\n"
             "%d requests were missing responses\n\n",
             lines, completed_transactions,
             missing_requests,
             requests - completed_transactions);

      if (completed_transactions > 0) {

        float avgresponse
          = (float) total_elapsed_seconds / (float) completed_transactions;

        printf(" Minimum response time: %d seconds (txn id: %s)\n",
               minresponse, minid);
        printf(" Maximum response time: %d seconds (txn id: %s)\n\n",
               maxresponse, maxid);

        printf(" Total elapsed seconds: %d\n", total_elapsed_seconds);
        printf(" Average response time: %f seconds\n\n", avgresponse);

        // txnstats_print();

        printf("Response Time | Txns\n");
        for (i = 0; i < 31; i++) {
          printf("%13d   %4d\n", i, txn_frequencies[i]);
        }
      }
      txnstats_close();

    } else {
      printf("Could not allocate transaction search structures.\n");
      return 1;
    }
  }
  return 0;
}

