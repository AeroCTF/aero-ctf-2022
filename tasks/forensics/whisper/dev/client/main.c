#include "main.h"

int main() {
    setup();

    char * user1 = (char*) malloc(USERNAME_SIZE);
    char * user2 = (char*) malloc(USERNAME_SIZE);
    char * user3 = (char*) malloc(USERNAME_SIZE);
    char * user4 = (char*) malloc(USERNAME_SIZE);

    memset(user1, 0, USERNAME_SIZE);
    memset(user2, 0, USERNAME_SIZE);
    memset(user3, 0, USERNAME_SIZE);
    memset(user4, 0, USERNAME_SIZE);
    
    strcpy(user1, "bebra");
    strcpy(user2, "bebrasniffer");
    strcpy(user3, "bebrus");
    strcpy(user4, "bebrilla");

    uint8_t key_rc4[] = "nuhai_bebru!";
    uint8_t key_aes[] = "jKi10f9lqWfyh0@1";
    uint8_t key_rc4_2[] = "rfhcsjdj4kaqwseasdqy";


    uint64_t token1 = 0;
    uint64_t token2 = 0;
    uint64_t token3 = 0;

    size_t ret = create_channel(RC4_T, key_rc4, 12, &token1);
    printf("{!} ret = %d, token: %08llx\n", ret, token1);
    
    char * msg = (char*) malloc(128);
    
    // user1 write
    memset(msg, 0, 128);
    strcpy(msg, "Are you here?");

    ret = add_message_to_channel(token1, 128, msg, user1);
    printf("{!} ret: %d\n", ret);

    // user2 write
    memset(msg, 0, 128);
    strcpy(msg, "Yes");

    ret = add_message_to_channel(token1, 128, msg, user2);
    printf("{!} ret: %d\n", ret);

    // user1 write
    memset(msg, 0, 128);
    strcpy(msg, "We need to use other channel with AES encryption!");

    ret = add_message_to_channel(token1, 128, msg, user1);
    printf("{!} ret: %d\n", ret);

    // user2 write
    memset(msg, 0, 128);
    strcpy(msg, "Okay");

    ret = add_message_to_channel(token1, 128, msg, user2);
    printf("{!} ret: %d\n", ret);

    // user1 write
    memset(msg, 0, 128);
    strcpy(msg, "And change nick!");

    ret = add_message_to_channel(token1, 128, msg, user1);
    printf("{!} ret: %d\n", ret);


    // create another channel
    ret = create_channel(AES_T, key_aes, 16, &token2);
    printf("{!} ret = %d, token: %08llx\n", ret, token2);

    // user1 write
    memset(msg, 0, 128);
    strcpy(msg, "Okay, this is good channel, first part of password is: Aero{4b3bru5_15_n0t_4b0bu5");

    ret = add_message_to_channel(token2, 128, msg, user3);
    printf("{!} ret: %d\n", ret);

    // user2 write
    memset(msg, 0, 128);
    strcpy(msg, "I see");

    ret = add_message_to_channel(token2, 128, msg, user4);
    printf("{!} ret: %d\n", ret);

    ret = create_channel(RC4_T, key_rc4_2, 18, &token3);
    printf("{!} ret = %d, token: %08llx\n", ret, token3);

    memset(msg, 0, 128);
    strcpy(msg, "last part is: _but_y0u_sh0000uld_kn0000w_11t_v333ry_we3Ell}");

    ret = add_message_to_channel(token3, 128, msg, user4);
    printf("{!} ret: %d\n", ret);

    memset(msg, 0, 128);
    free(msg);

    // debug
    // char * out = (char*) malloc(4096);
    // memset(out, 0, 4096);
    // ret = get_message_from_channel(token1, 2048, out, 0);
    // printf("ret: %d, data: %s\n", ret, out);

    // memset(out, 0, 4096);
    // ret = get_message_from_channel(token2, 2048, out, 0);
    // printf("ret: %d, data: %s\n", ret, out);

    // memset(out, 0, 4096);
    // ret = get_message_from_channel(token3, 2048, out, 0);
    // printf("ret: %d, data: %s\n", ret, out);

    return 0;
};

void setup(void) {
    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);

    fd = open("/dev/whisper", 0);

    if (fd < 0) {
        printf("{-} Error in device open, err = %d\n", fd);
        exit(0);
    }
}; 


uint64_t create_channel(uint8_t cipher, uint8_t* key, uint32_t keySize, uint64_t * tokenOut) {
    create_chl_req * req = (create_chl_req*) malloc(sizeof(create_chl_req));

    req->magic = MAGIC_CONST;
    req->cipher = cipher;
    req->key = key;
    req->keySize = keySize;
    req->tokenOut = tokenOut;
    uint64_t code = ioctl(fd, CREATE_CHANNEL, req);
    free(req);

    return code;
};

uint64_t destroy_channel(uint64_t token) {
    destroy_chl_req * req = (destroy_chl_req*) malloc(sizeof(destroy_chl_req));

    req->magic = MAGIC_CONST;
    req->accessToken = token;

    uint64_t code = ioctl(fd, DESTROY_CHANNEL, req);
    free(req);
    return code;
};


uint64_t add_message_to_channel(uint64_t token, uint32_t size, uint8_t * data, char * user) {
    add_msg_req * req = (add_msg_req*) malloc(sizeof(add_msg_req));

    req->magic = MAGIC_CONST;
    req->accessToken = token;
    req->size = size;
    req->inData = data;
    req->user = user;

    uint64_t code = ioctl(fd, ADD_MSG, req);
    free(req);

    return code;
}

size_t get_message_from_channel(uint64_t token, uint32_t size, uint8_t * data,
    uint32_t offset) {
    get_msg_req * req = (get_msg_req*) malloc(sizeof(get_msg_req));

    req->magic = MAGIC_CONST;
    req->accessToken = token;
    req->size = size;
    req->outData = data;
    req->offset = offset;

    uint64_t code = ioctl(fd, GET_MSG, req);
    free(req);

    return code;
}


