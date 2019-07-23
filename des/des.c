#include"DES.h"

int main(void) {
	u64 plaintext = 0x0123456789abcdef;
	u64 Rkey[16];
	u64 cipher;
	u8 key[8] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };

	KeyGenerate(key, Rkey);

	cipher = DES_Encryption(plaintext, Rkey);
	plaintext = DES_Decryption(cipher, Rkey);

	printf("Plaintext  : %llx\n", plaintext);
	printf("Ciphertext : %llx\n", cipher);
	printf("Plaintext  : %llx\n", plaintext);

	return 0;
}

