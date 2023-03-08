#include "crypto.h"

unsigned encrypt(unsigned block, unsigned key, unsigned prev_block) {
    return block ^ key ^ prev_block;
}

unsigned decrypt(unsigned block, unsigned key, unsigned prev_block) {
    return block ^ key ^ prev_block;
}