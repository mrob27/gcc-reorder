#ifndef GCC_DBF2
#define GCC_DBF2

#ifdef __cplusplus
extern "C" {
#endif

extern const char * p1_pathname;
extern FILE * ordf;

int rpm_strcmp(char *a, char *b, int maxlen);
int ordf_init(void);
int ordf_end(void);

#ifdef __cplusplus
}
#endif

#endif /* ! GCC_DBF2 */
