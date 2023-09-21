
#include "dbf1.h"
#include "ord1.h"

#if 0
/* This is impossible because ord1.c is included by gcc.c after gcc.c
   has already included system.h */
# define INCLUDE_LIST
# include "system.h"
# include <list>
static std::list<std::string> ord1_list;
#endif

const char * rml_p1_pathname = 0;
const char * rml_r1_pathname = 0;
const char * rml_r2_pathname = 0;
FILE * rml_ordf = 0;
char * rml_ord_txt_buf;

#define ORD_MAX_PASSES 500
#define ORD_MAX_NAMELEN 50
#define ORD_TEXTMAX ((ORD_MAX_PASSES*ORD_MAX_NAMELEN)>>1)

#define ORD_MAX_GROUPS 20

typedef struct lel1
{
  char * l1name;
  int    repeat;
} lel1;

lel1 ord1_list[ORD_MAX_PASSES]; /* Pointers to pass names. All will point to locations within rml.ord_txt_buf */
int ord1_llen;
int ord1_nnames[ORD_MAX_PASSES]; /* Number of pass names in each group */
int ord1_name1[ORD_MAX_PASSES]; /* Index to first pass name in each group */
int ord1_ngroups = 0; /* Number of groups */
const char * ord1_groups[ORD_MAX_GROUPS];
int ord1_curgroup = -1;
/* const char * ord1_sel_group = "loop"; / * Name of the group. This pointer will be reset to somewhere within rml.ord_txt_buf */

/* Normal strcmp function, written out in plain C to avoid all the
   complicated LOCALE-related things that the stdlib version does.
   In addition, this version performs the additional service of making
   it easy to distinguish the 'initial substring' cases from the other
   unequal cases. Examples:

    rpm_strcmp("car",  "cat")  == -2  (normal mismatch, b is bigger)
    rpm_strcmp("car",  "cart") == -1  (initial substring match, b is bigger)
    rpm_strcmp("car",  "car")  ==  0  (identical)
    rpm_strcmp("cart", "car")  ==  1  (initial substring match, a is bigger)
    rpm_strcmp("cat",  "car")  ==  2  (normal mismatch, a is bigger)
*/
int rpm_strcmp(const char *a, const char *b)
{
  int rv = 0;
  int l;

  rv = 0;
  l = 0;
  while((*a) && (*b) && (rv == 0)) {
    if (*a > *b) {
      /* both have a char at this position and a is bigger */
      rv = 2;
    } else if (*a < *b) {
      /* both have a char at this position and b is bigger */
      rv = -2;
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
  } /* else: same len, all bytes the same, let rv remain zero */
  return rv;
} /* end of rpm.strcmp() */

/* strlen with length limit, and without LOCALE-related complications */
int rpm_strnlen(const char *s, int maxlen)
{
  int  i;
  
  for(i=0; (i<maxlen) && s[i]; i++) {
    /* nothing */
  }
  
  return i;
} /* End of rpm.strnlen() */

/* scan a string for newline or null (or any of the low control characters
   apart from tab: we don't care about the rest) */
int rpm_sl_eol(const char *s, int maxlen)
{
  int  i;
  for(i=0; (i<maxlen) && ((s[i] == 9) || (s[i] > 31)); i++) {
    /* nothing */
  }
  return i;
} /* End of rpm.sl_eol() */

/* scan a string for whitespace (including space and any of the low control
   characters) */
int rpm_sl_spc(const char *s, int maxlen)
{
  int  i;
  for(i=0; (i<maxlen) && (s[i] > 32); i++) {
    /* nothing */
  }
  return i;
} /* End of rpm.sl_eol() */

int ordf_init(const char * fromwhere)
{
  char fmt1[25]; /* "%50s" */
  char fmt2[25]; /* "%50[_a-z0-9]%1[:]" */
  char fmt3[25]; /* "%50[_a-z0-9] %d" */
  char fmt4[25]; /* "%50[_a-z0-9 :]" */
  char s1[ORD_MAX_NAMELEN]; int nb, i;
  char * inp; char * i2; char * i3;
  char s2[ORD_MAX_NAMELEN];
  char s3[ORD_MAX_NAMELEN];
  int gg, mult;

  /* rml.p1_pathname = env.get("RML1_ORD_PATH"); */
  rml_p1_pathname = getenv("RML1_ORD_PATH");
  rml_r1_pathname = getenv("RML1_RPT1_PATH");
  rml_r2_pathname = getenv("RML1_RPT2_PATH");
  dbf1_init(rml_r1_pathname, rml_r2_pathname);
  fprintf(rml_dbf1, "ordf_init from %s\n", fromwhere);
  fprintf(rml_dbf1, "  r1_pathname == '%s'\n", rml_r1_pathname);
  fprintf(rml_dbf1, "  r2_pathname == '%s'\n", rml_r2_pathname);

  rml_ord_txt_buf = (char *) xmalloc(ORD_TEXTMAX);
  fprintf(rml_dbf1, "  got %lx from xmalloc\n", (long int) rml_ord_txt_buf);
  rml_ordf = 0;
  /* ord1_nnames[0] = 0; */
  ord1_llen = 0;
  ord1_ngroups = 0;
  ord1_groups[0] = "loop";
  if (rml_p1_pathname && rml_ord_txt_buf) {
    rml_ordf = fopen(rml_p1_pathname, "r");
    s1[0] = 0;
    if (rml_ordf) {
      /* Read the file raw data into memory, but only up to ORD_TEXTMAX
         bytes (including the null termination) */
      nb = (int) fread((void *)rml_ord_txt_buf, 1, ORD_TEXTMAX-1, rml_ordf);
      rml_ord_txt_buf[nb] = 0;
      /* Create format strings for reading pass names and group names
         from the in-memory text file */
      sprintf(fmt1, "%%%ds", ORD_MAX_NAMELEN-1);              /* "%49s" */
      fprintf(rml_dbf1, "  fmt1 == '%s'\n", fmt1);
      sprintf(fmt2, "%%%d[*_a-z0-9]%%1[:]", ORD_MAX_NAMELEN-1); /* "%49[_a-z0-9]%1[:]" */
      fprintf(rml_dbf1, "  fmt2 == '%s'\n", fmt2);
      sprintf(fmt3, "%%%d[*_a-z0-9] %%d", ORD_MAX_NAMELEN-1);   /* "%49[*_a-z0-9] %d" */
      fprintf(rml_dbf1, "  fmt3 == '%s'\n", fmt3);
      sprintf(fmt4, "%%%d[*_a-z0-9 :]", ORD_MAX_NAMELEN-1);     /* "%49[_a-z0-9 :]" */
      fprintf(rml_dbf1, "  fmt4 == '%s'\n", fmt4);
      /* Set inp to point to the beginning of the first line of text.
         We will read successive lines with sscanf until we get to the
         end of the number of bytes we got from fread */
      inp = rml_ord_txt_buf;
      fprintf(rml_dbf1, "  %d bytes in %s\n", nb, rml_p1_pathname);
      i = 0; gg = 1;
      while ( ((inp-rml_ord_txt_buf) < nb)   /* More text to scan */
         && (i < ORD_MAX_PASSES)         /* Still room in ord1_list[] */
         && (gg)                         /* Still going (stops when no sscanf
                                            pattern works) */
         && (1 == sscanf(inp, fmt4, s1)) /* Got a string */
      ) {
        fprintf(rml_dbf1, "fmt4 matched '%s'\n", s1); fflush(rml_dbf1);
        /* We scanned 1 string (line of text). Point i2 at the newline or
           null at the end of this line. */
        i2 = inp + rpm_sl_eol(s1, ORD_MAX_NAMELEN);
        *i2 = 0; /* turn newline into null */
        mult = 0;
        if (sscanf(s1, fmt2, s2, s3) == 2)
        { /* We got a name with a colon ':' */
          fprintf(rml_dbf1,"  fmt2 matches '%s' and '%s'\n",s2,s3); fflush(rml_dbf1);
          /* This is a context specifier, the name of a pass-group. */
          ord1_groups[ord1_ngroups] = inp;
          ord1_curgroup = ord1_ngroups;
          fprintf(rml_dbf1, "  set ord1_curgroup := %d\n", ord1_curgroup);
          ord1_nnames[ord1_curgroup] = 0;
          fprintf(rml_dbf1, "  set ord1_nnames[%d] := 0\n", ord1_curgroup);
          ord1_name1[ord1_curgroup] = ord1_llen;
          fprintf(rml_dbf1, "  set ord1_name1[%d] := %d\n",
                                                    ord1_curgroup, ord1_llen);
          ord1_ngroups++;
          fprintf(rml_dbf1, "  set ord1_ngroups := %d\n",  ord1_ngroups);
          i2[-1] = 0; /* Remove the ':' */
          fprintf(rml_dbf1, "target group '%s' selected\n",
                                                ord1_groups[ord1_curgroup]);
        } else if (sscanf(s1, fmt3, s2, &mult) == 2)
        { /* We got a name with a space and a number */
          fprintf(rml_dbf1, "  fmt3 matches '%s' x %d\n", s2, mult); fflush(rml_dbf1);
          fprintf(rml_dbf1, "  got pass name '%s' with mult %d\n", inp, mult);
          i3 = inp + rpm_sl_spc(s1, ORD_MAX_NAMELEN);
          *i3 = 0; /* turn space into null */
          ord1_list[ord1_llen].l1name = inp;
          ord1_list[ord1_llen].repeat = mult;
          ord1_llen++;
          fprintf(rml_dbf1, "  set ord1_llen := %d\n", ord1_llen);
          /* %%% increment current nnames[] counter */
          if (ord1_curgroup < 0) {
            fprintf(rml_dbf1, "\n\nERROR: No group name before pass name '%s'\n",
                             inp);
            dbf1_fatal_exit(-105);
          }
          ord1_nnames[ord1_curgroup]++;
          fprintf(rml_dbf1, "  set ord1_nnames[%d] := %d\n", ord1_curgroup,
                                                 ord1_nnames[ord1_curgroup]);
        } else if (sscanf(s1, fmt1, s2) == 1)
        { /* We got a name with nothing after it */
          fprintf(rml_dbf1, "  fmt1 matches '%s'\n", s2); fflush(rml_dbf1);
          /* save a pointer to this name */
          fprintf(rml_dbf1, "  ord1_list[%d] := '%s'\n", ord1_llen, inp);
          ord1_list[ord1_llen].l1name = inp;
          ord1_list[ord1_llen].repeat = 1;
          ord1_llen++;
          fprintf(rml_dbf1, "  set ord1_llen := %d\n", ord1_llen);
          /* %%% increment current nnames[] counter */
          if (ord1_curgroup < 0) {
            fprintf(rml_dbf1, "\n\nERROR: No group name before pass name '%s'\n",
                             inp);
            dbf1_fatal_exit(-106);
          }
          ord1_nnames[ord1_curgroup]++;
          fprintf(rml_dbf1, "  set ord1_nnames[%d] := %d\n", ord1_curgroup,
                                                 ord1_nnames[ord1_curgroup]);
        } else {
          fprintf(rml_dbf1, "\n\nERROR: Input '%s' did not match any pattern\n",
                   s1);
          dbf1_fatal_exit(-104);
        }
        /* Set pointer for next sscanf call */
        inp = i2+1;
      }
      /* Done reading names, remember how many we got */
      ord1_nnames[0] = ord1_llen; /* %%% remove */
      if (ord1_llen <= 0) {
        fprintf(rml_dbf1, "ordf_init: got no pass names from %s:\n", rml_p1_pathname);
      }
    } else {
      fprintf(rml_dbf1, "ordf_init: rml_ordf is 0, fopen failed?\n");
    }
  } else {
    fprintf(rml_dbf1, "ordf_init: rml.p1_pathname == 0\n");
  }
  if (ord1_nnames[0] > 0) {
    fprintf(rml_dbf1, "ordf_init: got %d pass names from %s:\n",
      ord1_nnames[0], rml_p1_pathname);
    fprintf(rml_dbf1, "  ord1_llen == %d\n", ord1_llen);
    fprintf(rml_dbf1, "  ord1_ngroups == %d\n", ord1_ngroups);
    for (i=0; i<ord1_ngroups; i++) {
      fprintf(rml_dbf1, "    ord1_nnames[%d] == %d\n", i, ord1_nnames[i]);
    }
    fprintf(rml_dbf1, "  group 0:\n");
    for(i=0; i<ord1_nnames[0]; i++) {
      fprintf(rml_dbf1, "  %3d %s x %d",
                        i, ord1_list[i].l1name, ord1_list[i].repeat);
      if (rpm_strcmp(ord1_list[i].l1name, "qux") == 0) {
        fprintf(rml_dbf1, " which is qux");
      }
      fprintf(rml_dbf1, "\n");
    }
  }
  fflush(rml_dbf1);
  return 0;
} /* End of ordf.init() */

/* Given a group name, see if it matches any group and return the group index
   and size */
int ord1_group_match(const char *groupname, int * group_num, int * n_reorders)
{
  int rv;
  int i;
  if (n_reorders) { *n_reorders = 0; }
  for(i=0; i<ord1_ngroups; i++) {
    rv = rpm_strcmp(groupname, ord1_groups[i]);
    if (rv == 0) {
      /* Exact match */
      if (group_num) { *group_num = i; }
      if (n_reorders) { *n_reorders = ord1_nnames[i]; }
      return 1;
    }
  }
  return 0;
}

int ord1_lookup(const int group, const char *passname, int * repeat)
{
  int i, j;
  if (passname) {
    fprintf(rml_dbf1, "o1l(%s)", passname);
  }
  if (passname && (ord1_nnames[group] > 0)) {
    for(j=0; j<ord1_nnames[group]; j++) {
      i = ord1_name1[group] + j;
      if (rpm_strcmp(ord1_list[i].l1name, passname) == 0) {
        fprintf(rml_dbf1, " == %d\n", i);
        if (repeat) {
          *repeat = ord1_list[i].repeat;
        }
        return i;
      }
    }
  }
  /* If we get here the name was not in our list */
  fprintf(rml_dbf1, " not found\n");
  return -1;
} /* End of ord1.lookup */

const char * ord1_index(int group_num, int j, int * repeat)
{
  int i;

  if ((group_num < 0) || (group_num >= ord1_ngroups)) {
    return 0;
  }
  if ((j < 0) || (j > ord1_nnames[group_num])) {
    return 0;
  }
  i = ord1_name1[group_num] + j;
  if ((i>=0) && (i<ord1_llen)) {
    if (repeat) {
      *repeat = ord1_list[i].repeat;
    }
    return ord1_list[i].l1name;
  }
  /* If we get here the index is out of range */
  return 0;
} /* End of ord1.index */

int ordf_end()
{
  if (rml_ordf) {
    fclose(rml_ordf);
  }
  return 0;
}
