#ifndef SYSTEM_DATATYPES_STRING_H_
#define SYSTEM_DATATYPES_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int startsWith(const char *string, const char *prefix);
extern int startsWithIgnoreCase(const char *string, const char *prefix);
extern int endsWith(const char *string, const char *suffix);
extern int endsWithIgnoreCase(const char *string, const char *suffix);
extern void stringToLower(char *string);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /* SYSTEM_DATATYPES_STRING_H_ */
