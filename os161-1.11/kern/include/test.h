#ifndef _TEST_H_
#define _TEST_H_
#include "opt-A2.h"
/*
 * Declarations for test code and other miscellaneous functions.
 */


/* These are only actually available if OPT_SYNCHPROBS is set. */
int catmouse(int, char **);
int createcars(int, char **);

/*
 * Test code.
 */

/* lib tests */
int arraytest(int, char **);
int bitmaptest(int, char **);
int queuetest(int, char **);

/* thread tests */
int threadtest(int, char **);
int threadtest2(int, char **);
int threadtest3(int, char **);
int semtest(int, char **);
int locktest(int, char **);
int cvtest(int, char **);

/* filesystem tests */
int fstest(int, char **);
int readstress(int, char **);
int writestress(int, char **);
int writestress2(int, char **);
int createstress(int, char **);
int printfile(int, char **);

/* other tests */
int malloctest(int, char **);
int mallocstress(int, char **);
int nettest(int, char **);

/* Kernel menu system */
void menu(char *argstr);

#if OPT_A2
runprogram(char *progname, unsigned int nargs, char **args);
#else
int runprogram(char *progname);
#endif

#endif /* _TEST_H_ */
