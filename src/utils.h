#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "cJSON.h"

uint32_t murmur_hash_combine(int32_t h1, int32_t h2);
const cJSON* load_json(const char *filepath);

#endif