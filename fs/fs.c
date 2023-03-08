#include "fs.h"
#include "../lib/string.h"
#include "../drivers/ata.h"
#include "../console.h"

int stat(const char* name, struct stat *bquf) {
    panic("stat not implemented");
}

int read_file(const char* name, void* buf, uint32_t bufsize, int authorized) {
    struct dir dir;
    encryption_policy enc = {0, 0};
    read_sectors_ATA_PIO((uint32_t)&dir, 1, 1, enc);
    enc.initial_value = dir.initial_value;
    for (int i = 0; i < ents_in_dir; ++i) {
        if (strncmp(dir.entries[i].name, name, 19) == 0) {
            enc.is_encrypted = dir.entries[i].is_encrypted;
            if (enc.is_encrypted && !authorized) {
                return -1;
            }
            uint32_t sectors = (dir.entries[i].size_bytes + sector_size - 1) / sector_size;
            uint32_t bytes = sectors * sector_size;
            if (bytes > bufsize) {
                panic("too big file");
            }
            read_sectors_ATA_PIO((uint32_t)buf, 1 + dir.entries[i].offset_sectors, sectors, enc);
            return bytes;
        }
    }
    panic("file not found");
}
