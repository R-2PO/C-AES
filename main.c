#include "aes.c"
#include "modes.c"


void main() {
    unsigned short int result = MSB(43,3,6);
    printf("%x\n",result);
}
