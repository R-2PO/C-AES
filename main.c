#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "modes.c"



void main() {
    unsigned char in[] = "Some secret text ciphered with AES in GCM mode, which gives both security and data authentication.";
    unsigned char additional[] = "Some additional data, that is not ciphered";
    unsigned short int key[16] = {0xb3, 0x5e, 0xc, 0xf1, 0x82, 0x49, 0xf6, 0x2f, 0x36, 0x10, 0x7c, 0x5c, 0xd8, 0xc7, 0x12, 0xa6};
    GCMBlock IV = {.val = {0x00, 0x00, 0x00, 0x00, 0x23, 0x9b, 0xcf, 0xe2, 0x3c, 0xb8, 0xe9, 0xab, 0x20, 0x73, 0x5f, 0x4}, .len = 96};
    unsigned char tag[16];
    unsigned char* ciphered;
    unsigned char* deciphered;
    unsigned long long int resultLength;
    unsigned char success;


    ciphered = AESGCMCipher(in, 98, additional, 42, IV, key, tag, 128, &resultLength);

    printf("Ciphered data:\n");
    for (unsigned long long int i=0; i<resultLength; i++) {
        printf("%02x ", ciphered[i]);
    }

    printf("\n\nTag: ");
    for (unsigned short int i=0; i<16; i++) {
        printf("%02x ", tag[i]);
    }



    // Uncomment the next line to change the data, making the authentication fail
    // ciphered[0] = 0;


    deciphered = AESGCMInvCipherAndAuthenticate(ciphered, resultLength, additional, 42, IV, key, tag, 128, &success, &resultLength);

    printf("\n\n\nDeciphered data:\n");

    if (success) {
        printf("Plaintext: ");
        for (unsigned long long int i=0; i<resultLength; i++) {
            printf("%c", deciphered[i]);
        }
        printf("\n");
    }
    else {
        printf("FAILED TO AUTHENTICATE DATA\n");
    }
}
