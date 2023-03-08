#pragma once

typedef struct {
    unsigned is_encrypted;
    unsigned initial_value;
} encryption_policy;

int authorize();