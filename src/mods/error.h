#ifndef ERROR_H
#define ERROR_H 1

void error_log(char *, ...);
void error_print(void);
char *error_get(void);
void error_clear(void);

#endif