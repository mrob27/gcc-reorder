#ifndef GCC_DBF2
#define GCC_DBF2

#ifdef __cplusplus
extern "C" {
#endif

extern const char * rml_p1_pathname;
extern FILE * rml_ordf;
extern FILE * rml_report1;

int rpm_strcmp(const char *a, const char *b);

int ordf_init(const char * fromwhere);
int ord1_group_match(const char *groupname, int * group_num, int * n_reorders);
int ord1_lookup(const int group, const char *passname, int * repeat);
const char * ord1_index(int group_num, int i, int * repeat);
int ordf_end(void);

#ifdef __cplusplus
}
#endif

#endif /* ! GCC_DBF2 */
