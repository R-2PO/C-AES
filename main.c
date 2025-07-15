#include "aes.c"
#include "modes.c"


void main() {
    unsigned char in[] = "This is a text to cipher.";
    unsigned short int key[] = {0x2b,0x7e,0x15,0x16, 0x28,0xae,0xd2,0xa6, 0xab,0xf7,0x15,0x88, 0x09,0xcf,0x4f,0x3c, 0xdc,0x38,0xaf,0x1a, 0xc2,0x6f,0xdd,0x6a, 0xee,0xf9,0x87,0x9f, 0x19,0x2a,0xf,0xae};
    unsigned char* out;
    int resultLength;
    out = AES256ECBCipher(in,25,key,&resultLength);
    for (short int i=0; i<resultLength; i++) {
        printf("%02x ",out[i]);
    }
    printf("\n");
    out = AES256ECBInvCipher(out,resultLength,key,&resultLength);
    for (short int i=0; i<resultLength; i++) {
        printf("%c",out[i]);
    }
    printf("\n");
}
