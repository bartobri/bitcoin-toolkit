#ifndef VERSION_H
#define VERSION_H 1

#include <stddef.h>

#define VERSION_COMMAND "version"

typedef struct Version *Version;

Version version_new(void);
size_t version_serialize(Version, unsigned char **);
size_t version_new_serialize(unsigned char **);
size_t version_deserialize(unsigned char *, Version *, size_t);
void version_free(Version);

#endif
