#ifndef VERACK_H
#define VERACK_H 1

#define VERACK_COMMAND "verack"

typedef struct Verack *Verack;

int verack_new(Verack);
void verack_free(Verack);

#endif
