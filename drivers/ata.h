#pragma once
#include <stdint.h>
#include "../lib/crypto.h"

void read_sectors_ATA_PIO(uint32_t target_address, uint32_t LBA, uint8_t sector_count,
                          encryption_policy enc = {0, 0});
void write_sectors_ATA_PIO(uint32_t LBA, uint8_t sector_count, uint32_t* bytes);
