#ifndef VERSION_H
#define VERSION_H 1

typedef struct Version *Version;

Version        version_new(void);
unsigned char *version_serialize(Version);

#endif
