#ifndef GCC_DBF1
#define GCC_DBF1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern FILE * rml_dbf1;
extern FILE * rml_rpt2;

int dbf1_init(const char * pathname, const char * report2);
int dbf1_end(void);
void dbf1_fatal_exit(int x);

#ifdef __cplusplus
}
#endif

#endif /* ! GCC_DBF1 */

