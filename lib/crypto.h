#pragma once

typedef struct {
    int is_encrypted;
    int initial_value;
} encryption_policy;

// Encrypts block using key and previous block (CBC)
unsigned encrypt(unsigned block, unsigned key, unsigned prev_block);

// Decrypts block using key and previous block (CBC)
unsigned decrypt(unsigned block, unsigned key, unsigned prev_block);
