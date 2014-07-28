Following the task laid out in "c test.txt", I decided the key to a
solution would be a quick ability to map from a response record back
to the request record. This suggested to me relatively simple
solutions using either a hashtable, or a binary search tree.

I found some errors in "c test.txt"'s problem specification, notably,
the timestamp is not of the form: MMDDYYYYMMSS but IS of the form
MMDDYYHHMMSS.

The problem specification was also vague in certain areas. I assumed
the following:

1. All requests would precede the response in the log
2. For any MSN ID, there would only be at most one request and one
   response in the log
3. Not all requests would have responses
4. Not all responses would have requests

The sample data file has 10,000 records. To verify my solution, I
ripped out the key fields from each record using an emacs macro, and
created a quick tcl program using tcl associative arrays to match the
response/request records. (rqrp.tcl).

I created a binary search tree version of the program using a binary
search tree implementation I wrote.

I also redid this with a hashtable using a widely available BSD
licensed hash table implementation available at github.

http://troydhanson.github.com/uthash/

The two versions arrive at the same results. The hashtable
implementation, written entirely as c macros, is pretty darn fast, and
certainly faster than my binary search tree mechanism (which itself, I
thought was pretty speedy.)

All versions are included.

In the c versions, you can make them and run them on linux or under
windows/cygwin:

$ make
$ ./report

You can run the tcl verification if you wish
$ tclsh rqrp.tcl

My results are in the file "Results".

The two implementations are very similar. I tried to separate two
layers:

1) report.c -- which does the file io, reads each record, has the
business logic, and presents the results

2) txnstats.c -- an underlying layer that implements the
sorting/searching structure and functions. A binary search tree in one
implementation, a hash table in another, and potentially other
structures in other implementations. 

This would allow a new implementation to be easily, modularly,
switched in after implementing these insert/search/sort operations:

   int txnstats_open();
   int txnstats_insert(char *id, char *timestamp);
   int txnstats_find_id(char *id, char *timestamp);
   int txnstats_recordcount();
   int txnstats_close();
   void txnstats_print();

And indeed, the file report.c is identical for both the binary search
tree and hashtable approach -- the only changes is to the file
txnstats.c and the Makefile.

=========================
NOTE: c test.txt suggests:

"5:  The application should be in a state to be able to be deployed to
production."

Well, sort of. I think the code is good and is maintainable. 

I think it can be deployed as is, but depending on how it is to be
used, I would certainly recommend additional testing and perhaps a
testing suite, and perhaps additional exploration of how to make it
faster.

What will size of the average log file be?
How often will this report run?
What impact would an error in this report have?

The answers to those questions would probably be key to determine if
this code is ready for production, or needs additional investigation
or verification.

