#include "dbf1.h"
#include "dbf2.h"

#if 0
/* This is impossible because dbf2.c is included by gcc.c after gcc.c
   has already included system.h */
# define INCLUDE_LIST
# include "system.h"
# include <list>
static std::list<std::string> ord1_list;
#endif

const char * p1_pathname = 0;
FILE * ordf = 0;

char ord_text[1000];
char * ord1_list[100];

int rpm_strcmp(char *a, char *b)
{
  int rv = 0;
  int l;

  rv = 0;
  l = 0;
  while((*a) && (*b) && (rv == 0)) {
    if (*a > *b) {
      rv = 1;
    } else if (*a < *b) {
      rv = -1;
    }
    l++;
    a++;
    b++;
  }
  if (rv) {
    /* We already have an answer */
  } else if (*a) {
    /* b is an initial substring of a */
    rv = 1;
  } else if (*b) {
    /* a is an initial substring of b */
    rv = -1;
  } /* else: same len, all bytes the same, leave at zero */
  return rv;
}

int ordf_init()
{
  char s1[51]; int nb; char * inp;
  dbf1_init();
  fprintf(dbf1, "ordf_init\n");
  p1_pathname = env.get("RML1_ORD_PATH");
  ordf = 0;
  if (p1_pathname) {
    fprintf(dbf1, "ordf_init: p1_pathname == '%s'\n", p1_pathname);
    ordf = fopen(p1_pathname, "r");
    s1[0] = 0;
    if (ordf) {
      nb = (int) fread((void *)ord_text, 1, 999, ordf);
      ord_text[nb] = 0;
      inp = ord_text;
      fprintf(dbf1, "  %d bytes in ordf\n", nb);
      while (((inp-ord_text) < nb) && (1 == sscanf(inp, "%50s", s1)) ) {
        fprintf(dbf1, "  got '%s'\n", s1);
        inp = inp + strlen(s1) + 1;
      }
    } else {
      fprintf(dbf1, "  ordf is 0, fopen failed?\n");
    }
  } else {
    fprintf(dbf1, "ordf_init: p1_pathname == 0\n");
  }
  return 0;
}

int ordf_end()
{
  if (ordf) {
    fclose(ordf);
  }
  return 0;
}
