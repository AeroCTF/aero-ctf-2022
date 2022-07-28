#include <cstdio>
#include <cstdint>
#include <iostream>

#include "md5.h"


void print_hex(uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }

    printf("\n");
}


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
        return 0;
    }

    auto filename = argv[1];

    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        std::cout << "Failed to open file " << filename << std::endl;
        return 0;
    }

    MD5Context ctx;
    md5Init(&ctx);

    uint8_t buffer[64];

    do {
        size_t len = fread(buffer, sizeof(char), 64, f);

        if (len == 0) {
            break;
        }

        md5Update(&ctx, buffer, len);
    } while (true);

    fclose(f);

    md5Finalize(&ctx);

    print_hex(ctx.digest, 16);

    return 0;
}
