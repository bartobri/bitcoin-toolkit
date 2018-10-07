#ifndef VERACK_H
#define VERACK_H 1

#include <stddef.h>

#define VERACK_COMMAND "verack"

typedef struct Verack *Verack;

int verack_new(Verack);
size_t verack_sizeof(void);

#endif
