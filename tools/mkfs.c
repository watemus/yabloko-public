#include "../fs/fs.h"
#include "../lib/crypto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* basename(char* path) {
    char* c = strrchr(path, '/');
    if (c && *c) {
        return c + 1;
    }
    return path;
}

void write_file(struct dir* dir, int num, char* name, unsigned* sector_offset, unsigned key, encryption_policy enc,
                FILE* image) {
    char sector[sector_size];
    memset(sector, 0, sizeof sector);
    struct dirent *dirent = &dir->entries[num];
    dirent->offset_sectors = *sector_offset;
    dirent->size_bytes = 0;
    dirent->is_encrypted = enc.is_encrypted;

    FILE* file = fopen(name, "rb");
    if (!file) {
        perror(name);
        exit(1);
    }

    size_t read_size;

    while ((read_size = fread(sector, 1, sizeof(sector), file))) {
        if (enc.is_encrypted) {
            for (int i = 0; i < sector_size; i += 4) {
                unsigned block = 0;
                memcpy(&block, sector + i, 4);
                block ^= key ^ enc.initial_value;
                enc.initial_value = block;
                memcpy(sector + i, &block, 4);
            }
        }
        if (fwrite(sector, 1, sizeof(sector), image) != sizeof(sector)) {
            perror(name);
            exit(1);
        }
        (*sector_offset)++;
        dirent->size_bytes += read_size;
    }

    if (fclose(file)) {
        perror(name);
        exit(1);
    }

    dirent->reserved = 0;
    dirent->name[sizeof(dirent->name) - 1] = '\0';
    strncpy(dirent->name, basename(name), sizeof(dirent->name) - 1);
}

int main(int argc, char* argv[]) {
    struct dir dir = {.reserved = 0};

    if (argc < 5) {
        fprintf(stderr, "Usage: %s OUT.FS KERNEL.BIN [FILE_NAME IS_ENCRYPTED(y/n)...] INITIAL_VALUE KEY\n", argv[0]);
        return 1;
    }

    FILE* image = fopen(argv[1], "wb");
    if (!image) {
        perror(argv[1]);
        return 1;
    }


    unsigned initial_value = strtoul(argv[argc - 2], NULL, 10);
    unsigned key = strtoul(argv[argc - 1], NULL, 10);

    dir.initial_value = initial_value;

    if (fwrite(&dir, sizeof(dir), 1, image) < 1) {
        perror("fwrite");
        return 1;
    }
    uint32_t sector_offset = 1;



    encryption_policy enc = {.is_encrypted = 0, .initial_value = initial_value};
    write_file(&dir, 0, argv[2], &sector_offset, key, enc, image);
    enc.is_encrypted = 1;
    write_file(&dir, 1, "checkfile", &sector_offset, key, enc, image);

    for (int i = 3; i < argc - 2; i += 2) {
        char* name = argv[i];
        char is_encrypted = (argv[i + 1][0] == 'y');
        enc.is_encrypted = is_encrypted;
        write_file(&dir, 2 + (i - 3) / 2, name, &sector_offset, key, enc, image);
    }

    fseek(image, 0, SEEK_SET);
    if (fwrite(&dir, sizeof(dir), 1, image) < 1) {
        perror("fwrite");
        return 1;
    }

    if (fclose(image)) {
        perror(argv[0]);
        return 1;
    }

    return 0;
}
