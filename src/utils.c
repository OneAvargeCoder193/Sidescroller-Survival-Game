#include "utils.h"

#include <stdio.h>

uint32_t murmur_hash_combine(int32_t h1, int32_t h2) {
    const uint32_t seed = 0; // Initial seed
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const int32_t r1 = 15;
    const int32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    uint32_t k1 = h1;
    uint32_t k2 = h2;

    k1 *= c1;
    k1 = (k1 << r1) | (k1 >> (32 - r1));
    k1 *= c2;
    hash ^= k1;
    hash = (hash << r2) | (hash >> (32 - r2));
    hash = hash * m + n;

    k2 *= c1;
    k2 = (k2 << r1) | (k2 >> (32 - r1));
    k2 *= c2;
    hash ^= k2;
    hash = (hash << r2) | (hash >> (32 - r2));
    hash = hash * m + n;

    hash ^= 8; // Length of the data (two 4-byte integers)

    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;

    return hash;
}

const cJSON* load_json(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error opening JSON file %s\n", filepath);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    
    fclose(file);
    
    const cJSON *json = cJSON_Parse(data);
    
    free(data);
    
    if (!json) {
        fprintf(stderr, "Error parsing JSON file %s\n", filepath);
    }
    return json;
}