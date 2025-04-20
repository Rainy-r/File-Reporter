#if !defined(DETECT_DUPS_H)
#define DETECT_DUPS_H

#define ArrayLength(Array) (sizeof(Array)) / (Array)[0]

#define MAX_FILE_LENGTH 255
#define NO_STRS 10000
#define CHUNK_SIZE (1024 * 30) // 30 Kb
#define MD5_DIGEST_LENGTH 16

#include <unistd.h>
#include <openssl/evp.h>
#include <ftw.h>
#include <fcntl.h>
#include <time.h> 
#include <stdint.h>
#include "uthash.h"

typedef uint8_t uint8;
typedef uint64_t uint64;
typedef int bool32;

typedef struct PathList {
    uint64 INodeValue;
    //    const char *FilePath; 
    const char **Paths; // You would have to allocate a number of paths for this.
    uint8 *HashKey;

    int Size; 
    UT_hash_handle hh;
} PathList;

static int render_file_info(const char *FilePath, const struct stat *StatBuffer,
			      int TFlag, struct FTW *FTWBuffer); 
#endif
