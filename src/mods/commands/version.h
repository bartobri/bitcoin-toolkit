#ifndef VERSION_H
#define VERSION_H 1

#include <stddef.h>

#define VERSION_COMMAND "version"

typedef struct Version *Version;

int version_new(Version);
int version_serialize(unsigned char *, Version);
int version_new_serialize(unsigned char *);
size_t version_deserialize(unsigned char *, Version *, size_t);
void version_free(Version);
char *version_to_json(Version v);
size_t version_sizeof(void);
size_t version_max_user_agent(void);

#endif
