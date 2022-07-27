#include <stdio.h>

int main(void) {
    FILE *f = fopen("/challenge/flag.txt", "r");

    if (f == NULL) {
        puts("[-] Something is wrong. Please, contact admin.");
        return 0;
    }

    char buffer[1024] = {0};

    size_t length = fread(buffer, sizeof(char), 1024, f);

    fclose(f);

    fwrite(buffer, sizeof(char), length, stdout);

    return 0;
}
