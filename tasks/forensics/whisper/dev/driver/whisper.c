#include "whisper.h"


static noinline long ioctl_handler(struct file * file, unsigned int cmd, 
    unsigned long arg) 
{
    long ret = -1;

    switch (cmd) {
        case CREATE_CHANNEL:
            ret = create_channel(arg);
            break;
        case DESTROY_CHANNEL:
            ret = destroy_channel(arg);
            break;
        case ADD_MSG:
            ret = add_message_to_channel(arg);
            break;
        case GET_MSG:
            ret = get_message_from_channel(arg);
            break;
        default:
            break;
    }

    return ret;
};



static noinline size_t dec_aes(channel_t* pChnl) {
    struct AES_ctx ctx;
    int i = 0;

    for (i = 0; i < pChnl->infoCurSize; i += 16) {
        AES_init_ctx(&ctx, pChnl->key);
        AES_ECB_decrypt(&ctx, pChnl->infoBuffer + i);
    }
    return 0;
};

static noinline size_t dec_rc4(channel_t* pChnl) {
    uint8_t * outBuf = (uint8_t*) kmalloc(pChnl->infoCurSize, GFP_KERNEL);
    RC4(pChnl->key, pChnl->infoBuffer, outBuf, pChnl->infoCurSize, pChnl->keySize);
    memset(pChnl->infoBuffer, 0, pChnl->infoCurSize);
    kfree(pChnl->infoBuffer);
    pChnl->infoBuffer = outBuf;
    return 0;
};

static noinline size_t enc_aes(channel_t* pChnl) {
    struct AES_ctx ctx;
    int i = 0;

    for (i = 0; i < pChnl->infoCurSize; i += 16) {
        AES_init_ctx(&ctx, pChnl->key);
        AES_ECB_encrypt(&ctx, pChnl->infoBuffer + i);
    }

    return 0;
}

static noinline size_t enc_rc4(channel_t* pChnl) {
    return dec_rc4(pChnl);
}


static noinline size_t decrypt_chnl(channel_t* pChnl) {
    size_t ret = 0;

    switch(pChnl->cipher) {
        case AES_T:
            dec_aes(pChnl);
            break;
        case RC4_T:
            dec_rc4(pChnl);
            break;
        case NONE_T:
            ret = 1;
            break;
    }
    
    return ret;
}

static noinline size_t encrypt_chnl(channel_t* pChnl) {
    size_t ret = 0;

    switch(pChnl->cipher) {
        case AES_T:
            enc_aes(pChnl);
            break;
        case RC4_T:
            enc_rc4(pChnl);
            break;
        case NONE_T:
            ret = 1;
            break;
    }
    
    return ret;
}


static noinline size_t add_message_to_channel(unsigned long arg) {
    add_msg_req * uReq = (add_msg_req*) kmalloc(sizeof(add_msg_req), GFP_KERNEL);
    channel_t * pChnl = NULLPTR;
    char * msg = NULLPTR;
    char * username = NULLPTR;
    char * data = NULLPTR;

    if (copy_from_user((void *)uReq, (void *)arg, sizeof(add_msg_req))) {
        kfree(uReq);
        return -1;
    }

    // check magic const
    if (uReq->magic != MAGIC_CONST) {
        kfree(uReq);
        return -1;
    }

    pChnl = find_chl_by_token(uReq->accessToken);
    
    if (pChnl == NULLPTR) {
        kfree(uReq);
        return -1;
    }

    username = (char*) kmalloc(USERNAME_SIZE, GFP_KERNEL);
    memset(username, 0, USERNAME_SIZE);

    if (copy_from_user((void *)username, (void *)uReq->user, USERNAME_SIZE)) {
        kfree(uReq);
        return -1;
    }

    data = (char*) kmalloc(uReq->size, GFP_KERNEL);
    memset(data, 0, uReq->size);

    if (copy_from_user((void *)data, (void *)uReq->inData, uReq->size)) {
        kfree(uReq);
        return -1;
    }

    // check size of added message
    if ((uReq->size+USERNAME_SIZE) > pChnl->infoCurSize) {
        pChnl->infoCurSize += (uReq->size - pChnl->infoCurSize + 16 + USERNAME_SIZE);
        if (pChnl->infoCurSize % 16 != 0) {
            pChnl->infoCurSize += (16 - (pChnl->infoCurSize%16));
        }
        pChnl->infoBuffer = (char*) krealloc(pChnl->infoBuffer, pChnl->infoCurSize, GFP_KERNEL);
    }

    // check if info is empty
    if (pChnl->infoOffset != 0) {
        decrypt_chnl(pChnl);
    }

    msg = (char*) kmalloc(uReq->size + USERNAME_SIZE + 16, GFP_KERNEL);
    snprintf(msg, uReq->size + USERNAME_SIZE + 16, "<%s>:%s\n", username, data);
    memcpy(pChnl->infoBuffer + pChnl->infoOffset, msg, strlen(msg));
    encrypt_chnl(pChnl);
    pChnl->infoOffset += strlen(msg);

    memset(msg, 0, uReq->size + USERNAME_SIZE + 16);
    memset(data, 0, uReq->size);
    kfree(msg);
    kfree(data);

    return 0;
};

static noinline size_t get_message_from_channel(unsigned long arg) {
    get_msg_req * uReq = (get_msg_req*) kmalloc(sizeof(get_msg_req), GFP_KERNEL);
    channel_t * pChnl = NULLPTR;
    uint32_t readSize = 0;

    if (copy_from_user((void *)uReq, (void *)arg, sizeof(get_msg_req))) {
        kfree(uReq);
        return -1;
    }

    // check magic const
    if (uReq->magic != MAGIC_CONST) {
        kfree(uReq);
        return -1;
    }

    pChnl = find_chl_by_token(uReq->accessToken);
    
    if (pChnl == NULLPTR) {
        kfree(uReq);
        return -1;
    }

    if (uReq->offset > pChnl->infoOffset) {
        kfree(uReq);
        return -1;
    }

    if (uReq->size > pChnl->infoCurSize) {
        readSize = pChnl->infoCurSize;
    } else {
        readSize = uReq->size;
    }

    decrypt_chnl(pChnl);

    if (copy_to_user((void*)uReq->outData, pChnl->infoBuffer+uReq->offset, readSize)) {
        kfree(uReq);
        return -1;
    }

    encrypt_chnl(pChnl);
    return 0;
};

static noinline size_t create_channel(unsigned long arg) {
    create_chl_req * uReq = (create_chl_req*) kmalloc(sizeof(create_chl_req), GFP_KERNEL);
    channel_t * newChannel = NULLPTR;
    uint8_t * key = NULLPTR;

    if (copy_from_user((void *)uReq, (void *)arg, sizeof(create_chl_req))) {
        kfree(uReq);
        return 1;
    }

    // check magic const
    if (uReq->magic != MAGIC_CONST) {
        kfree(uReq);
        return 2;
    }
    
    if (uReq->cipher != AES_T && uReq->cipher != RC4_T && uReq->cipher != NONE_T) {
        kfree(uReq);
        return 3;
    }

    // if cipher type is AES, check key size 
    if (uReq->cipher == AES_T) {
        if (uReq->keySize != 16) {
            kfree(uReq);
            return 4;
        }
    }

    // create key in kernel-space and copy from user-space
    key = (uint8_t*) kmalloc(uReq->keySize, GFP_KERNEL);

    if (copy_from_user((void *)key, (void *)uReq->key, uReq->keySize)) {
        kfree(uReq);
        kfree(key);
        return 5;
    }

    // create channel
    newChannel = (channel_t*) kmalloc(sizeof(channel_t), GFP_KERNEL);
    newChannel->cipher = uReq->cipher;
    newChannel->key = key;
    newChannel->keySize = uReq->keySize;
    newChannel->infoBuffer = (char*) kmalloc(DEFAULT_BUFFER_SIZE, GFP_KERNEL);
    newChannel->infoOffset = 0;
    newChannel->infoCurSize = DEFAULT_BUFFER_SIZE;

    // generate access token
    get_random_bytes(&newChannel->accessToken, sizeof(uint64_t));

    if (copy_to_user((void *)uReq->tokenOut, (void *)&newChannel->accessToken, sizeof(uint64_t))) {
        kfree(uReq);
        kfree(key);
        kfree(newChannel);
        return 5;
    }

    add_chnl(newChannel);

    return 0;
}

static noinline size_t destroy_channel(unsigned long arg) {
    destroy_chl_req * uReq = (destroy_chl_req*) kmalloc(sizeof(destroy_chl_req), GFP_KERNEL);
    channel_t * pChnl = NULLPTR;

    if (copy_from_user((void *)uReq, (void *)arg, sizeof(destroy_chl_req))) {
        kfree(uReq);
        return -1;
    }

    // check magic const
    if (uReq->magic != MAGIC_CONST) {
        kfree(uReq);
        return -1;
    }

    pChnl = find_chl_by_token(uReq->accessToken);
    
    if (pChnl == NULLPTR) {
        kfree(uReq);
        return -1;
    }

    remove_chnl(pChnl);

    if (pChnl->infoBuffer != NULLPTR) {
        kfree(pChnl->infoBuffer);
        pChnl->infoOffset = 0;
    }

    if (pChnl->key != NULLPTR) {
        kfree(pChnl->key);
    }

    kfree(pChnl);
    return 0;
};

static noinline size_t add_chnl(channel_t * pChnl) {
    channel_elem_t * pNewChnlEntry = NULLPTR;
    
    if (pChnl == NULLPTR) {
        return -1;
    }

    pNewChnlEntry = (channel_elem_t*) kmalloc(sizeof(channel_elem_t),
         GFP_KERNEL);

    pNewChnlEntry->next = NULLPTR;
    pNewChnlEntry->cur = pChnl;
    
    if (pChannelList == NULLPTR) {
        pChannelList = pNewChnlEntry;    
    } else {
        pNewChnlEntry->next = pChannelList;
        pChannelList = pNewChnlEntry;
    }

    return 0;
}

static noinline size_t remove_chnl(channel_t * pChnl) {
    channel_elem_t * pCur = pChannelList;
    channel_elem_t * pPrev = NULLPTR;

    while (pCur != NULLPTR) {
        if (pCur->cur == pChnl)
            break;
        pPrev = pCur;
        pCur = pCur->next;
    }

    if (pCur == NULLPTR) {
        return -1;
    }

    if (pPrev == NULLPTR) {
       pChannelList = pCur->next;
       return 0; 
    }

    pPrev->next = pCur->next;
    return 0;
}

static noinline channel_t * find_chl_by_token(uint64_t token) {
    channel_elem_t * pCur = pChannelList;

    while (pCur != NULLPTR) {
        if (pCur->cur->accessToken == token)
                return pCur->cur;
        pCur = pCur->next;
    }

    return NULLPTR;
};

static noinline void swaps(unsigned char *a, unsigned char *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static noinline size_t KSA(uint8_t *key, unsigned char *S, size_t keySize) {

    int len = keySize;
    int j = 0;
    int i = 0;


    for(i = 0; i < N; i++)
        S[i] = i;

    for(i = 0; i < N; i++) {
        j = (j + S[i] + key[i % len]) % N;

        swaps(&S[i], &S[j]);
    }

    return 0;
}

static noinline size_t PRGA(unsigned char* S, uint8_t* plaintext, 
    uint8_t* ciphertext, size_t plainSize) {
    size_t n = 0;
    int i = 0;
    int j = 0;
    int rnd = 0;
    size_t len = 0;

    for(n = 0, len = plainSize; n < len; n++) {
        i = (i + 1) % N;
        j = (j + S[i]) % N;

        swaps(&S[i], &S[j]);
        rnd = S[(S[i] + S[j]) % N];

        ciphertext[n] = rnd ^ plaintext[n];

    }

    return 0;
}

static noinline size_t RC4(uint8_t *key, uint8_t *in, 
    uint8_t * out, size_t inSize, size_t keySize) {

    unsigned char S[N];
    KSA(key, S, keySize);
    PRGA(S, in, out, inSize);

    return 0;
}

static noinline void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key)
{
  unsigned i, j, k;
  uint8_t tempa[4]; // Used for the column/row operations
  
  // The first round key is the key itself.
  for (i = 0; i < Nk; ++i)
  {
    RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
    RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
    RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
    RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
  }

  // All other round keys are found from the previous round keys.
  for (i = Nk; i < Nb * (Nr + 1); ++i)
  {
    {
      k = (i - 1) * 4;
      tempa[0]=RoundKey[k + 0];
      tempa[1]=RoundKey[k + 1];
      tempa[2]=RoundKey[k + 2];
      tempa[3]=RoundKey[k + 3];

    }

    if (i % Nk == 0)
    {
      // This function shifts the 4 bytes in a word to the left once.
      // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

      // Function RotWord()
      {
        const uint8_t u8tmp = tempa[0];
        tempa[0] = tempa[1];
        tempa[1] = tempa[2];
        tempa[2] = tempa[3];
        tempa[3] = u8tmp;
      }

      // SubWord() is a function that takes a four-byte input word and 
      // applies the S-box to each of the four bytes to produce an output word.

      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }

      tempa[0] = tempa[0] ^ Rcon[i/Nk];
    }

    j = i * 4; k=(i - Nk) * 4;
    RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
    RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
    RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
    RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
  }
}

static noinline void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key)
{
  KeyExpansion(ctx->RoundKey, key);
}

static noinline void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey)
{
  uint8_t i,j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
    }
  }
}

static noinline void SubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxValue((*state)[j][i]);
    }
  }
}

static noinline void ShiftRows(state_t* state)
{
  uint8_t temp;

  temp           = (*state)[0][1];
  (*state)[0][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[3][1];
  (*state)[3][1] = temp;

  temp           = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp           = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  temp           = (*state)[0][3];
  (*state)[0][3] = (*state)[3][3];
  (*state)[3][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[1][3];
  (*state)[1][3] = temp;
}

static noinline uint8_t xtime(uint8_t x)
{
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

// MixColumns function mixes the columns of the state matrix
static noinline void MixColumns(state_t* state)
{
  uint8_t i;
  uint8_t Tmp, Tm, t;
  for (i = 0; i < 4; ++i)
  {  
    t   = (*state)[i][0];
    Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
    Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][3] ^ t ;              Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
  }
}

static noinline void InvMixColumns(state_t* state)
{
  int i;
  uint8_t a, b, c, d;
  for (i = 0; i < 4; ++i)
  { 
    a = (*state)[i][0];
    b = (*state)[i][1];
    c = (*state)[i][2];
    d = (*state)[i][3];

    (*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
    (*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
    (*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
    (*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
  }
}

static noinline void InvSubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxInvert((*state)[j][i]);
    }
  }
}

static noinline void InvShiftRows(state_t* state)
{
  uint8_t temp;

  // Rotate first row 1 columns to right  
  temp = (*state)[3][1];
  (*state)[3][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[0][1];
  (*state)[0][1] = temp;

  // Rotate second row 2 columns to right 
  temp = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  // Rotate third row 3 columns to right
  temp = (*state)[0][3];
  (*state)[0][3] = (*state)[1][3];
  (*state)[1][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[3][3];
  (*state)[3][3] = temp;
}

// Cipher is the main function that encrypts the PlainText.
static noinline void Cipher(state_t* state, const uint8_t* RoundKey)
{
  uint8_t round = 0;

  AddRoundKey(0, state, RoundKey);
  for (round = 1; ; ++round)
  {
    SubBytes(state);
    ShiftRows(state);
    if (round == Nr) {
      break;
    }
    MixColumns(state);
    AddRoundKey(round, state, RoundKey);
  }
  AddRoundKey(Nr, state, RoundKey);
}

static noinline void InvCipher(state_t* state, const uint8_t* RoundKey)
{
  uint8_t round = 0;

  AddRoundKey(Nr, state, RoundKey);

  for (round = (Nr - 1); ; --round)
  {
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(round, state, RoundKey);
    if (round == 0) {
      break;
    }
    InvMixColumns(state);
  }

}

static noinline void AES_ECB_encrypt(const struct AES_ctx* ctx, uint8_t* buf)
{
  Cipher((state_t*)buf, ctx->RoundKey);
}

static noinline void AES_ECB_decrypt(const struct AES_ctx* ctx, uint8_t* buf)
{
  InvCipher((state_t*)buf, ctx->RoundKey);
}

static int __init init_dev(void) {
    int dev_reg = misc_register(&whisper_dev);
    if (dev_reg < 0){
        printk(KERN_INFO "{-} Failed to register dev!");
    }

    return 0;
}

static void __exit exit_dev(void) {

    misc_deregister(&whisper_dev);
}

module_init(init_dev);
module_exit(exit_dev);