#ifndef VERACK_H
#define VERACK_H 1

#define VERACK_COMMAND "verack"

typedef struct Verack *Verack;

Verack verack_new(void);
void verack_free(Verack);

#endif
