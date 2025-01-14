// stolen from https://github.com/dhavalhirdhav/LearnOS/blob/master/drivers/ata/ata.c

#include <stdint.h>

#include "ata.h"
#include "port.h"

/*
 BSY: a 1 means that the controller is busy executing a command. No register should be accessed (except the digital output register) while this bit is set.
RDY: a 1 means that the controller is ready to accept a command, and the drive is spinning at correct speed..
WFT: a 1 means that the controller detected a write fault.
SKC: a 1 means that the read/write head is in position (seek completed).
DRQ: a 1 means that the controller is expecting data (for a write) or is sending data (for a read). Don't access the data register while this bit is 0.
COR: a 1 indicates that the controller had to correct data, by using the ECC bytes (error correction code: extra bytes at the end of the sector that allows to verify its integrity and, sometimes, to correct errors).
IDX: a 1 indicates the the controller retected the index mark (which is not a hole on hard-drives).
ERR: a 1 indicates that an error occured. An error code has been placed in the error register.
*/

#define STATUS_BSY 0x80
#define STATUS_RDY 0x40
#define STATUS_DRQ 0x08
#define STATUS_DF 0x20
#define STATUS_ERR 0x01

//This is really specific to our OS now, assuming ATA bus 0 master
//Source - OsDev wiki
static void ATA_wait_BSY();
static void ATA_wait_DRQ();
void read_sectors_ATA_PIO(uint32_t target_address, uint32_t LBA, uint8_t sector_count, encryption_policy enc)
{

	ATA_wait_BSY();
	port_byte_out(0x1F6, 0xE0 | ((LBA >>24) & 0xF));
	port_byte_out(0x1F2, sector_count);
	port_byte_out(0x1F3, (uint8_t) LBA);
	port_byte_out(0x1F4, (uint8_t)(LBA >> 8));
	port_byte_out(0x1F5, (uint8_t)(LBA >> 16));
	port_byte_out(0x1F7, 0x20); //Send the read command

	uint16_t *target = (uint16_t*) target_address;

	for (int j = 0; j < sector_count; j++)
	{
		ATA_wait_BSY();
		ATA_wait_DRQ();
		for (int i = 0; i < 256; i++) {
            target[i] = port_word_in(0x1F0);
            if (i % 2 == 1 && enc.is_encrypted) {
                uint32_t *block = (uint32_t*)(target + i - 1);
                int val = *block ^ enc.initial_value;
                asm volatile ("mov %%cr3, %%eax;"
                              "xor %%eax, %%edx;"
                : "=d"(val)
                : "d"(val)
                : "eax");
                enc.initial_value = *block;
                *block = val;
            }
        }
		target += 256;
	}
}

void write_sectors_ATA_PIO(uint32_t LBA, uint8_t sector_count, uint32_t* bytes)
{
	ATA_wait_BSY();
	port_byte_out(0x1F6,0xE0 | ((LBA >>24) & 0xF));
	port_byte_out(0x1F2,sector_count);
	port_byte_out(0x1F3, (uint8_t) LBA);
	port_byte_out(0x1F4, (uint8_t)(LBA >> 8));
	port_byte_out(0x1F5, (uint8_t)(LBA >> 16));
	port_byte_out(0x1F7,0x30); //Send the write command

	for (int j =0;j<sector_count;j++)
	{
		ATA_wait_BSY();
		ATA_wait_DRQ();
		for(int i=0;i<256;i++)
		{
			port_long_out(0x1F0, bytes[i]);
		}
	}
}

static void ATA_wait_BSY()   //Wait for bsy to be 0
{
	while (port_byte_in(0x1F7) & STATUS_BSY);
}
static void ATA_wait_DRQ()  //Wait fot drq to be 1
{
	while(!(port_byte_in(0x1F7) & STATUS_RDY));
}
