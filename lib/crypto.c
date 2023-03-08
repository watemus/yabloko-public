#include "crypto.h"
#include "../console.h"
#include "../cpu/isr.h"
#include "../cpu/gdt.h"
#include "../drivers/keyboard.h"
#include "../drivers/vga.h"
#include "../drivers/ata.h"
#include "../drivers/misc.h"
#include "../drivers/uart.h"
#include "../fs/fs.h"
#include "../lib/string.h"
#include "../proc.h"

int authorize() {
    vga_clear_screen();
    printk("Enter password (32bit unsigned integer) or guest or halt\n");

    printk("\n> ");
    while (1) {
        if (kbd_buf_size > 0 && kbd_buf[kbd_buf_size-1] == '\n') {
            if (!strncmp("halt\n", kbd_buf, kbd_buf_size)) {
                qemu_shutdown();
            } else if (!strncmp("guest\n", kbd_buf, kbd_buf_size)) {
                kbd_buf_size = 0;
                return 0;
            } else {
                unsigned key;
                char buf[512];
                int res = atoun(kbd_buf, &key, kbd_buf_size - 1);
                if (res == -1) {
                    printk("Incorrect format of password. It must be 32bit unsigned integer.");
                } else {
                    asm volatile("mov %%eax, %%cr3;"
                            : "=a"(key)
                            : "a"(key));
                    read_file("checkfile", buf, 512, 1);
                    if (strncmp("caosbest", buf, 8) != 0) {
                        printk("Incorrect password :(\n");
                    } else {
                        kbd_buf_size = 0;
                        return 1;
                    }
                }
            }
            kbd_buf_size = 0;
            printk("\n> ");
        }
        asm("hlt");
    }
    return 0;
}