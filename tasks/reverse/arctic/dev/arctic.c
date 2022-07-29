#include <stdio.h>
#include <stdlib.h>

#include "threefish.h"


uint64_t key[] = {
    0xada91b550b9a73b0,
    0x61507af7837f9941,
    0x9494a0451db3c6a0,
    0x3a9cad867ce7ea4c,
};

uint64_t tweak[] = {
    0xef3e09c055674e80,
    0x377eff2c9a936186,
};


__attribute__((aligned(0x10000))) int check_flag(uint8_t *flag) {
    // fake flag: "not_flag{how_did_you_find_this??},_now_please_find_the_real_flag"

    ThreefishKey_t ctx;

    asm (
        "xor %rax, %rax;"
        "nop;"
        "xor %rbx, %rbx;"
        "nop;"
        "xor %rcx, %rcx;"
        "nop;"
        "xor %rdx, %rdx;"
        "nop;"
    );

    // 0xe0ff00415506b890 -> "nop; mov eax, 0x415506; jmp eax"
    uint64_t plaintext[] = {
        0x38e76ac984fd93ac, 0x21d68d29ba044b73, 0xbe8dbf15974396ce, 0x21ff9c1b91ed0551, 0xbda2fc6b85ecdb69, 0xe0ff00415506b890, 0x9aa4a4f5f15e2aa7, 0x0221d54f140500e8,
    };

    uint64_t ciphertext[] = {
        0x6353c0dddb255b65, 0x280638028af8e1cc, 0x2df42cc9b357253f, 0x2a352e99849a4330, 0xce7a7054825ea935, 0x2843a085fa911f83, 0x82af923a6aad886b, 0x3178f42d3ac0d784,
    };

    ThreefishSetKey(&ctx, key, tweak);

    for (size_t i = 0; i < 64; i++) {
        ((uint8_t *)plaintext)[i] ^= flag[i];

        ThreefishEncrypt(&ctx, plaintext, plaintext);
        ThreefishEncrypt(&ctx, plaintext + 4, plaintext + 4);
    }

    int result = 0;

    for (size_t i = 0; i < 8; i++) {
        if (plaintext[i] != ciphertext[i]) {
            result = 1;
        }
    }

    return result;
}


int validate() {
    uint8_t flag[64] = {0};

    printf("[*] Please, enter the flag:\n> ");
    scanf("%s", flag);

    return check_flag(flag);
}


int main(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("[*] Hello!\n");

    if (!validate()) {
        printf("[+] Correct flag.\n");
    }
    else {
        printf("[-] Wrong flag.\n");
    }

    return 0;
}

void _start(void) {
    _Exit(main());
}
