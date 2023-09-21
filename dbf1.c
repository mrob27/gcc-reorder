#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*

REVISION HISTORY
 20220128 "p1.txt" is now always in the local directory. If opening it
fails (for example no permissions on current directory) it opens
/dev/null instead.

*/

#include "dbf1.h"
FILE * rml_dbf1 = 0;
FILE * rml_rpt2 = 0;

#ifndef RML_WHICHMAIN
# define RML_WHICHMAIN "unknown"
#endif

int dbf1_init(const char * rpt1, const char * report2)
{
  if (rml_dbf1 == 0) {
    if (rpt1) {
      rml_dbf1 = fopen(rpt1, "a"); /* If this fails it will return 0 */
    }
  }
  if (rml_dbf1 == 0) {
    rml_dbf1 = fopen("/dev/null", "w"); /* ... then we do this */
  }
  if (rml_rpt2 == 0) {
    if (report2) {
      rml_rpt2 = fopen(report2, "a"); /* If this fails it will return 0 */
    }
  }
  if (rml_rpt2 == 0) {
    rml_rpt2 = fopen("/dev/null", "w"); /* ... then we do this */
  }
  fprintf(rml_dbf1, "---------------------------\n");
  time_t now;
  time(&now);
  char buf[32];
  strftime(buf, sizeof buf, "%Y%m%d.%H:%M:%S", gmtime(&now));
  fprintf(rml_dbf1, "%s\n", buf);
  fprintf(rml_dbf1, "dbf1_init(%s, %s) in PID %d %s\n",
                         rpt1, report2, ((int)getpid()), RML_WHICHMAIN);
  return 0;
}

int dbf1_end()
{
  if (rml_dbf1) {
    fclose(rml_dbf1);
    rml_dbf1 = 0;
  }
  if (rml_rpt2) {
    fclose(rml_rpt2);
    rml_rpt2 = 0;
  }
  return 0;
}

void dbf1_fatal_exit(int x)
{
  volatile int * p1;
  fprintf(rml_dbf1, "dbf1_fatal_exit %d in %s\n", x, RML_WHICHMAIN);
  fflush(rml_dbf1);
  p1 = (int *) -1;
  *p1 = 0;
}

