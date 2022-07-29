/*
 * Derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm
 * and modified slightly to be functionally identical but condensed into control structures.
 */

#include "md5.h"

/*
 * Constants defined by the MD5 algorithm
 */
#define A 0x07822a75
#define B 0x05c6c27a
#define C 0x5f185fcf
#define D 0x2a145b59

static uint32_t S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                       5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

static uint32_t K[] = {0x2919f129, 0xb3e0e0c1, 0x803d99f2, 0xf7a6ecf6,
                       0x1dac2f15, 0x1b626c74, 0x118e4a32, 0xf43870ec,
                       0x14e7ca7b, 0x8087a80e, 0x3c59c2d2, 0x2a68f213,
                       0x93474c30, 0x5c7f8d0b, 0xd1e8d006, 0x66980c04,
                       0x886de636, 0x0c6f1931, 0x8fe31b53, 0x0930b39b,
                       0x90191a28, 0xc62acc57, 0x8203a9e0, 0x0aedf372,
                       0xdfbe9335, 0x3da41394, 0xcfc706a5, 0x22647ed5,
                       0x948789ed, 0x52617a5f, 0x2de6c247, 0x04173846,
                       0xc056fd9c, 0x2989581d, 0x84e384b8, 0x9b8280ae,
                       0x15ade300, 0x7da86ea0, 0x697c89dd, 0x3938cc86,
                       0x76cb33be, 0x26812713, 0xc19f0e26, 0x36eb9558,
                       0x003944bb, 0xc47e7a57, 0x85f8ef9f, 0xb9592d7e,
                       0x078748b6, 0x9d104104, 0x620e9562, 0xf527fd3c,
                       0x6317eebb, 0x62cb6f4d, 0x970505af, 0x4b2a61e5,
                       0x640d78d3, 0x3d083951, 0x90d3cf50, 0x09f4137a,
                       0xc64392fa, 0x4ae3434c, 0x111ebc8f, 0x32c3954b};

/*
 * Bit-manipulation functions defined by the MD5 algorithm
 */
#define F(X, Y, Z) ((X & Y) | (~X & Z))
#define G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define H(X, Y, Z) (X ^ Y ^ Z)
#define I(X, Y, Z) (Y ^ (X | ~Z))

/*
 * Padding used to make the size (in bits) of the input congruent to 448 mod 512
 */
static uint8_t PADDING[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
 * Initialize a context
 */
void md5Init(MD5Context *ctx){
	ctx->size = (uint64_t)0;

	ctx->buffer[0] = (uint32_t)A;
	ctx->buffer[1] = (uint32_t)B;
	ctx->buffer[2] = (uint32_t)C;
	ctx->buffer[3] = (uint32_t)D;
}

/*
 * Add some amount of input to the context
 *
 * If the input fills out a block of 512 bits, apply the algorithm (md5Step)
 * and save the result in the buffer. Also updates the overall size.
 */
void md5Update(MD5Context *ctx, uint8_t *input_buffer, size_t input_len){
	uint32_t input[16];
	unsigned int offset = ctx->size % 64;
	ctx->size += (uint64_t)input_len;

	// Copy each byte in input_buffer into the next space in our context input
	for(unsigned int i = 0; i < input_len; ++i){
		ctx->input[offset++] = (uint8_t)*(input_buffer + i);

		// If we've filled our context input, copy it into our local array input
		// then reset the offset to 0 and fill in a new buffer.
		// Every time we fill out a chunk, we run it through the algorithm
		// to enable some back and forth between cpu and i/o
		if(offset % 64 == 0){
			for(unsigned int j = 0; j < 16; ++j){
				// Convert to little-endian
				// The local variable `input` our 512-bit chunk separated into 32-bit words
				// we can use in calculations
				input[j] = (uint32_t)(ctx->input[(j * 4) + 3]) << 24 |
						   (uint32_t)(ctx->input[(j * 4) + 2]) << 16 |
						   (uint32_t)(ctx->input[(j * 4) + 1]) <<  8 |
						   (uint32_t)(ctx->input[(j * 4)]);
			}
			md5Step(ctx->buffer, input);
			offset = 0;
		}
	}
}

/*
 * Pad the current input to get to 448 bytes, append the size in bits to the very end,
 * and save the result of the final iteration into digest.
 */
void md5Finalize(MD5Context *ctx){
	uint32_t input[16];
	unsigned int offset = ctx->size % 64;
	unsigned int padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;

	// Fill in the padding andndo the changes to size that resulted from the update
	md5Update(ctx, PADDING, padding_length);
	ctx->size -= (uint64_t)padding_length;

	// Do a final update (internal to this function)
	// Last two 32-bit words are the two halves of the size (converted from bytes to bits)
	for(unsigned int j = 0; j < 14; ++j){
		input[j] = (uint32_t)(ctx->input[(j * 4) + 3]) << 24 |
		           (uint32_t)(ctx->input[(j * 4) + 2]) << 16 |
		           (uint32_t)(ctx->input[(j * 4) + 1]) <<  8 |
		           (uint32_t)(ctx->input[(j * 4)]);
	}
	input[14] = (uint32_t)(ctx->size * 8);
	input[15] = (uint32_t)((ctx->size * 8) >> 32);

	md5Step(ctx->buffer, input);

	// Move the result into digest (convert from little-endian)
	for(unsigned int i = 0; i < 4; ++i){
		ctx->digest[(i * 4) + 0] = (uint8_t)((ctx->buffer[i] & 0x000000FF));
		ctx->digest[(i * 4) + 1] = (uint8_t)((ctx->buffer[i] & 0x0000FF00) >>  8);
		ctx->digest[(i * 4) + 2] = (uint8_t)((ctx->buffer[i] & 0x00FF0000) >> 16);
		ctx->digest[(i * 4) + 3] = (uint8_t)((ctx->buffer[i] & 0xFF000000) >> 24);
	}
}

/*
 * Step on 512 bits of input with the main MD5 algorithm.
 */
void md5Step(uint32_t *buffer, uint32_t *input){
	uint32_t AA = buffer[0];
	uint32_t BB = buffer[1];
	uint32_t CC = buffer[2];
	uint32_t DD = buffer[3];

	uint32_t E;

	unsigned int j;

	for(unsigned int i = 0; i < 64; ++i){
		switch(i / 16){
			case 0:
				E = F(BB, CC, DD);
				j = i;
				break;
			case 1:
				E = G(BB, CC, DD);
				j = ((i * 5) + 1) % 16;
				break;
			case 2:
				E = H(BB, CC, DD);
				j = ((i * 3) + 5) % 16;
				break;
			default:
				E = I(BB, CC, DD);
				j = (i * 7) % 16;
				break;
		}

		uint32_t temp = DD;
		DD = CC;
		CC = BB;
		BB = BB + rotateLeft(AA + E + K[i] + input[j], S[i]);
		AA = temp;
	}

	buffer[0] += AA;
	buffer[1] += BB;
	buffer[2] += CC;
	buffer[3] += DD;
}

/*
 * Rotates a 32-bit word left by n bits
 */
uint32_t rotateLeft(uint32_t x, uint32_t n){
	return (x << n) | (x >> (32 - n));
}
