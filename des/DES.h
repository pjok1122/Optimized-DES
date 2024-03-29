#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "table.h"

typedef unsigned int u32;
typedef uint8_t u8;
typedef uint64_t u64;

/* -------------키 생성 알고리즘 -----------*/
void KeyGenerate(u8 *key, u64 *Rkey);
void ShiftLeft(u8 *Key, int count);
void ParityDrop(u8 *key, u8 *L, u8 *R);
u64 CompressionPbox(u8 *L, u8 *R);
void CharToBit(char ch, u8 *bits);
void CharToBits(char *chr, u8 *bits, int len);

/* ------------- 암복호 알고리즘 ----------*/
u64 DES_IP(u64 in);
u64 DES_FP(u64 in);
u32 DES_RoundFunction(u32 in, u64 rkey);
u64 DES_Encryption(u64 in, u64 *Rkey);
u64 DES_Decryption(u64 in, u64 *Rkey);



void CharToBit(char ch, u8 *bits) {
	for (int i = 0; i < 8; i++) {
		bits[7 - i] = (ch & 1);
		ch = ch >> 1;
	}
}
void CharToBits(char *chr, u8 *bits, int len) {
	for (int i = 0; i <len; i++) {
		CharToBit(chr[i], bits + 8 * i);
	}
}

void ParityDrop(u8 *key, u8 *L, u8 *R) {
	int count = 0;
	u8 tmp[64];
	u8 tmp2[56];

	CharToBits(key, tmp, 8);
	for (int i = 0; i < 56; i++)
		tmp2[i] = tmp[Parity[i] - 1];

	for (int i = 0; i < 56; i++) {
		if (i < 28) L[i] = tmp2[i];
		else		R[i - 28] = tmp2[i];
	}
}

void ShiftLeft(u8 *Key, int count) {
	for (int i = 1; i <= count; i++) {
		u8 tmp = Key[0];
		for (int j = 1; j < 28; j++)
			Key[j - 1] = Key[j];
		Key[27] = tmp;
	}
}

u64 CompressionPbox(u8 *L, u8 *R) {
	u8 tmp[56];
	u8 Rk[48];
	u64 Rkey = 0;
	for (int i = 0; i < 28; i++) tmp[i] = L[i];
	for (int i = 0; i < 28; i++) tmp[28 + i] = R[i];
	for (int i = 0; i < 48; i++) Rk[i] = tmp[ComPbox[i] - 1];	//Compression P_box

	for (int i = 0; i < 48; i++) {		//Save as u64 type
		Rkey = Rkey << 1;
		Rkey += Rk[i];
	}
	return Rkey;
}
void KeyGenerate(u8 *key, u64 *Rkey) {
	u8 L[28], R[28];
	ParityDrop(key, L, R);
	for (int r = 0; r < 16; r++) {
		ShiftLeft(L, Shift[r]);
		ShiftLeft(R, Shift[r]);
		Rkey[r] = CompressionPbox(L, R);
	}

}

u64 DES_IP(u64 in) {
	u64 out = 0;
	for (int i = 0; i<64; i++) {
		out = out << 1;
		out += (in >> (64 - IP[i])) & 1;
	}
	return out;
}
u64 DES_FP(u64 in) {
	u64 out = 0;
	for (int i = 0; i < 64; i++) {
		out = out << 1;
		out += (in >> (64 - FP[i])) & 1;
	}
	return out;
}

u32 DES_RoundFunction(u32 in, u64 rkey) {
	char text[8] = { (in >> 27) & 0x1e, (in >> 23) & 0x1e, (in >> 19) & 0x1e, (in >> 15) & 0x1e, (in >> 11) & 0x1e, (in >> 7) & 0x1e, (in >> 3) & 0x1e, (in << 1) & 0x1e };
	u32 result = 0;

	for (int i = 0; i < 8; i++)
		text[i] = (((text[(i + 7) % 8] >> 1) & 1) << 5) | text[i] | ((text[(i + 1) % 8] >> 4) & 1); //Ex-Pbox
	for (int i = 0; i < 8; i++) {
		text[i] = text[i] ^ ((rkey >> (42 - 6 * i)) & 0x3f);			//KeyXor
		text[i] = Sbox[i][text[i]];									//Sbox64
	}

	for (int i = 0; i < 32; i++) {
		result = result << 1;
		result += (text[(Pbox[i] - 1) / 4] >> (3 - ((Pbox[i] - 1) % 4))) & 1;		//St-Permutation
	}

	return result;
}

u64 DES_Encryption(u64 in, u64 *Rkey) {

	in = DES_IP(in);				//IP

	u32 L = in >> 32;
	u32 R = in & 0xffffffff;
	u32 tmp;
	u64 result = 0;

	/* -------Round Function and Swapper --------- */
	for (int r = 0; r < 16; r++) {
		tmp = R;
		R = L ^ DES_RoundFunction(R, Rkey[r]);
		L = tmp;

		if (r == 15) {
			tmp = L;
			L = R;
			R = tmp;
		}
		//printf("%2d Round : %08x %08x \n", r, L, R);
	}

	result = L;
	result = (result << 32) | R;
	result = DES_FP(result);		//FP

	return result;
}

u64 DES_Decryption(u64 in, u64 *Rkey) {
	in = DES_IP(in);				//IP

	u32 L = in >> 32;
	u32 R = in & 0xffffffff;
	u32 tmp;
	u64 result = 0;

	/* -------Round Function and Swapper --------- */
	for (int r = 0; r < 16; r++) {
		tmp = R;
		R = L ^ DES_RoundFunction(R, Rkey[15 - r]);
		L = tmp;

		if (r == 15) {
			tmp = L;
			L = R;
			R = tmp;
		}
		//printf("%2d Round : %08x %08x \n", r, L, R);
	}
	result = L;
	result = (result << 32) | R;

	result = DES_FP(result);			//FP

	return result;
}