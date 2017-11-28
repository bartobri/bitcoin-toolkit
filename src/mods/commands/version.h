#ifndef VERSION_H
#define VERSION_H 1

typedef struct Version *Version;

Version version_new(void);
size_t version_serialize(Version, unsigned char **);
void version_free(Version);

#endif
