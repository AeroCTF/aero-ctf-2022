#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#define NULLPTR NULL
#define TRUE 1
#define FALSE 0
#define DEFAULT_BUFFER_SIZE 2048
#define USERNAME_SIZE 128
#define MAGIC_CONST 0x55778899aabbccdd 
#define N 256   // 2^8

// typedef unsigned char uint8_t; 
// typedef unsigned short uint16_t;
// typedef unsigned int uint32_t;
// typedef unsigned long long uint64_t;


enum CIPHER_TYPE {AES_T, RC4_T, NONE_T};
enum ACTIONS {
    CREATE_CHANNEL = 0x4411, 
    DESTROY_CHANNEL = 0x5522,
    ADD_MSG = 0x6633,
    GET_MSG = 0x7744
};

typedef struct {
    uint64_t accessToken;
    uint8_t cipher;
    uint8_t * key;
    uint32_t keySize;

    char * infoBuffer;
    uint32_t infoOffset;
    uint32_t infoCurSize;
} channel_t;

typedef struct {
    uint64_t magic;
    uint8_t cipher;
    uint8_t * key;
    uint32_t keySize;
    uint64_t * tokenOut;
} create_chl_req;

typedef struct {
    uint64_t magic;
    uint64_t accessToken;
} destroy_chl_req;

typedef struct {
    uint64_t magic;
    uint64_t accessToken;
    uint32_t offset;
    uint32_t size;
    uint8_t * outData;
} get_msg_req;


typedef struct {
    uint64_t magic;
    uint64_t accessToken;
    uint32_t size;
    uint8_t * inData;
    char * user;
} add_msg_req;

int fd = 0; // device fd

void setup(void);

uint64_t create_channel(uint8_t cipher, uint8_t* key, uint32_t keySize, uint64_t*);
uint64_t destroy_channel(uint64_t);
uint64_t add_message_to_channel(uint64_t token, uint32_t size, uint8_t * data, char * user);
size_t get_message_from_channel(uint64_t token, uint32_t size, uint8_t * data, uint32_t offset);
