#pragma once

#include <stdint.h>

/* Directory structure:

         32-byte entries
├─────────────┬───────────────────────────────┤
│Initial value│   Reserved                    │
│uint32       │   char[28]                    │
├──────┬──────┼────────┬────────┬─────────────┤
│Offset│Size  │Reserved│Name    │Is encrypted │
│uint32│uint32│uint32  │char[19]│char[1]      │
├──────┼──────┼────────┼────────┼─────────────┤
│ ...  │      │        │        │             │

Offset is in sectors (zero-based),
size is in bytes, name is 0-terminated.

*/

enum {
    dir_block_len = 32,
	sector_size = 512,
	ents_in_dir = (sector_size - dir_block_len) / dir_block_len,
};

struct dirent {
	uint32_t offset_sectors;
	uint32_t size_bytes;
	uint32_t reserved;
	char name[19];
    char is_encrypted;
};

struct dir {
    int initial_value;
	char reserved[28];
	struct dirent entries[ents_in_dir];
};

struct stat {
	uint32_t size;
	uint32_t reserved[3];
};

/* Find file by name and fill information in buf.
 * Returns zero on success, nonzero on failure. */
int stat(const char* name, struct stat *buf);

/* Find file by name and read it into buffer with size bufsize.
 * At most (bufsize & ~511) bytes will be read.
 * Return number of bytes read or -1 on failure. */
int read_file(const char* name, void* buf, uint32_t bufsize);
