#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"




unsigned short int SBoxTable[256] = {99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118, 202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117, 9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132, 83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210, 205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115, 96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138, 112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158, 225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22};

unsigned short int InvSBoxTable[256] = {82, 9, 106, 213, 48, 54, 165, 56, 191, 64, 163, 158, 129, 243, 215, 251, 124, 227, 57, 130, 155, 47, 255, 135, 52, 142, 67, 68, 196, 222, 233, 203, 84, 123, 148, 50, 166, 194, 35, 61, 238, 76, 149, 11, 66, 250, 195, 78, 8, 46, 161, 102, 40, 217, 36, 178, 118, 91, 162, 73, 109, 139, 209, 37, 114, 248, 246, 100, 134, 104, 152, 22, 212, 164, 92, 204, 93, 101, 182, 146, 108, 112, 72, 80, 253, 237, 185, 218, 94, 21, 70, 87, 167, 141, 157, 132, 144, 216, 171, 0, 140, 188, 211, 10, 247, 228, 88, 5, 184, 179, 69, 6, 208, 44, 30, 143, 202, 63, 15, 2, 193, 175, 189, 3, 1, 19, 138, 107, 58, 145, 17, 65, 79, 103, 220, 234, 151, 242, 207, 206, 240, 180, 230, 115, 150, 172, 116, 34, 231, 173, 53, 133, 226, 249, 55, 232, 28, 117, 223, 110, 71, 241, 26, 113, 29, 41, 197, 137, 111, 183, 98, 14, 170, 24, 190, 27, 252, 86, 62, 75, 198, 210, 121, 32, 154, 219, 192, 254, 120, 205, 90, 244, 31, 221, 168, 51, 136, 7, 199, 49, 177, 18, 16, 89, 39, 128, 236, 95, 96, 81, 127, 169, 25, 181, 74, 13, 45, 229, 122, 159, 147, 201, 156, 239, 160, 224, 59, 77, 174, 42, 245, 176, 200, 235, 187, 60, 131, 83, 153, 97, 23, 43, 4, 126, 186, 119, 214, 38, 225, 105, 20, 99, 85, 33, 12, 125};



word Rcon[10] = {{.x = {0x01,0,0,0}}, {.x = {0x02,0,0,0}}, {.x = {0x04,0,0,0}}, {.x = {0x08,0,0,0}}, {.x = {0x10,0,0,0}}, {.x = {0x20,0,0,0}}, {.x = {0x40,0,0,0}}, {.x = {0x80,0,0,0}}, {.x = {0x1b,0,0,0}}, {.x = {0x36,0,0,0}}};




unsigned short int add (unsigned short int a, unsigned short int b) {
    return a^b;
}


short int deg (unsigned short int a) {
    unsigned short int sub = 1;
    short int degree = -1;
    for (short int i=0; i<16; i++) {
        if ((a & sub) != 0) {
            degree = i;
        }
        sub = sub << 1;
    }
    return degree;
}



unsigned short int xTimes (unsigned short int b) {
    b = b*2;
    if (b>255) {
        // b_7=1
        b = b - 256; // Remove the first 1
        b = b^27; // 27 = 0b00011011
    }
    return b;
}


unsigned short int multiply (unsigned short int a, unsigned short int b) {
    unsigned short int sub = 1;
    unsigned short int result = 0;
    unsigned short int part;
    for (short int i=0; i<8; i++) {
        if ((b & sub) != 0) {
            part = a;
            for (short int j=0; j<i; j++) {
                part = xTimes(part);
            }
            result = add(result,part);
        }
        sub = sub << 1;
    }
    return result;
}



word multiplyWords (word a, word b) {
    word result;
    result.x[0] = add(add(multiply(a.x[0],b.x[0]), multiply(a.x[3],b.x[1]))  ,  add(multiply(a.x[2],b.x[2]), multiply(a.x[1],b.x[3])));
    result.x[1] = add(add(multiply(a.x[1],b.x[0]), multiply(a.x[0],b.x[1]))  ,  add(multiply(a.x[3],b.x[2]), multiply(a.x[2],b.x[3])));
    result.x[2] = add(add(multiply(a.x[2],b.x[0]), multiply(a.x[1],b.x[1]))  ,  add(multiply(a.x[0],b.x[2]), multiply(a.x[3],b.x[3])));
    result.x[3] = add(add(multiply(a.x[3],b.x[0]), multiply(a.x[2],b.x[1]))  ,  add(multiply(a.x[1],b.x[2]), multiply(a.x[0],b.x[3])));
    return result;
}



word addWords (word a, word b) {
    word result;
    for (short int i=0; i<4; i++) {
        result.x[i] = add(a.x[i],b.x[i]);
    }
    return result;
}




void euclidean_division (unsigned short int a, unsigned short int b, unsigned short int *quotient, unsigned short int *remainder) {
    unsigned short int r;
    short int d; // The degree of r
    short int db = deg(b);
    unsigned short int q=0;

    r = a;
    d = deg(r);
    while (d>=db) {
        r = add(r,b*pow(2,d-db));
        q = q + pow(2,d-db);
        d = deg(r);
    }
    *quotient = q;
    *remainder = r;
}


unsigned short int inverse (unsigned short int a) {
    unsigned short int b = 0x11b; // 0x11b = 0b100011011
    unsigned short int remainders[10];
    unsigned short int quotients[10];
    unsigned short int y[10];
    short int last_index = 1;

    remainders[0] = b;
    remainders[1] = a;
    while (remainders[last_index] != 0) {
        euclidean_division(remainders[last_index-1],remainders[last_index],&quotients[last_index],&remainders[last_index+1]);

        last_index++;
    }
    y[last_index-1] = 0;
    y[last_index-2] = 1;
    for (short int i=last_index-3; i>=0; i--) {
        y[i] = add(multiply(quotients[i+1],y[i+1]), y[i+2]);
    }
    return y[0];
}




unsigned short int SBox (unsigned short int a) {
    unsigned short int a_inv = inverse(a);
    short int b[8];
    short int r[8];
    unsigned short int result = 0;
    unsigned short int c = 0x63;

    // Split a
    for (short int i=0; i<8; i++) {
        b[i] = a_inv&1;
        a_inv = a_inv>>1;
    }
    
    for (short int i=0; i<8; i++) {
        r[i] = b[i];
        for (short int j=4; j<8; j++) {
            r[i] = r[i]^b[(i+j)%8];
        }
    }
    
    // Assemble the result
    for (short int i=7; i>=0; i--) {
        result = result<<1;
        result = result + r[i];
    }
    result = add(result,c);
    return result;
}




matrix SubBytes (matrix s) {
    matrix result;
    for (short int c=0; c<4; c++) {
        for (short int r=0; r<4; r++) {
            result.c[c].r.x[r] = SBoxTable[s.c[c].r.x[r]];
        }
    }
    return result;
}



matrix InvSubBytes (matrix s) {
    matrix result;
    for (short int c=0; c<4; c++) {
        for (short int r=0; r<4; r++) {
            result.c[c].r.x[r] = InvSBoxTable[s.c[c].r.x[r]];
        }
    }
    return result;
}




word SubWord (word a) {
    word result;
    for (short int i=0; i<4; i++) {
        result.x[i] = SBoxTable[a.x[i]];
    }
    return result;
}




matrix ShiftRows (matrix s) {
    matrix result;
    result.c[0].r.x[0] = s.c[0].r.x[0];
    result.c[1].r.x[0] = s.c[1].r.x[0];
    result.c[2].r.x[0] = s.c[2].r.x[0];
    result.c[3].r.x[0] = s.c[3].r.x[0];
    for (short int r=1; r<4; r++) {
        for (short int c=0; c<4; c++) {
            result.c[c].r.x[r] = s.c[(r+c)%4].r.x[r];
        }
    }
    return result;
}


matrix InvShiftRows (matrix s) {
    matrix result;
    result.c[0].r.x[0] = s.c[0].r.x[0];
    result.c[1].r.x[0] = s.c[1].r.x[0];
    result.c[2].r.x[0] = s.c[2].r.x[0];
    result.c[3].r.x[0] = s.c[3].r.x[0];
    for (short int r=1; r<4; r++) {
        for (short int c=0; c<4; c++) {
            result.c[c].r.x[r] = s.c[(c-r+4)%4].r.x[r];
        }
    }
    return result;
}




matrix MixColumns (matrix s) {
    matrix result;
    word word_mult = {.x = {2,1,1,3}};
    word column;
    for (short int c=0; c<4; c++) {
        column = s.c[c].r;
        column = multiplyWords(word_mult,column);
        result.c[c].r = column;
    }
    return result;
}



matrix InvMixColumns (matrix s) {
    matrix result;
    word word_mult = {.x = {0x0e,0x09,0x0d,0x0b}};
    word column;
    for (short int c=0; c<4; c++) {
        column = s.c[c].r;
        column = multiplyWords(word_mult, column);
        result.c[c].r = column;
    }
    return result;
}





matrix AddRoundKey (matrix s, expandedKey k, short int round) {
    matrix result;
    word column_result;
    word column;
    for (short int c=0; c<4; c++) {
        column = s.c[c].r;
        column_result = addWords(column,k.k[4*round+c]);
        result.c[c].r = column_result;
    }
    return result;
}





word RotWord (word a) {
    word result = {.x = {a.x[1],a.x[2],a.x[3],a.x[0]}};
    return result;
}







expandedKey KeyExpansion (unsigned short int* key, short int Nk, short int Nr) {
    word wKey[8];
    expandedKey result;
    word temp;
    int i = 0;
    
    // Convert the key into words
    for (short int i=0; i<Nk; i++) {
        wKey[i].x[0] = key[4*i];
        wKey[i].x[1] = key[4*i+1];
        wKey[i].x[2] = key[4*i+2];
        wKey[i].x[3] = key[4*i+3];
    }

    while (i<Nk) {
        result.k[i] = wKey[i];
        i++;
    }
    while (i<=4*Nr+3) {
        temp = result.k[i-1];
        if (i%Nk == 0) {
            temp = addWords(SubWord(RotWord(temp)),Rcon[i/Nk-1]);
        }
        else if (Nk == 8 && i%Nk == 4) {
            temp = SubWord(temp);
        }
        result.k[i] = addWords(result.k[i-Nk],temp);
        i++;
    }
    return result;
}




void Cipher (unsigned short int* in, short int Nr, expandedKey expKey, unsigned short int* out) {
    matrix state;
    word RoundKey[4];
    for (short int c=0; c<4; c++) {
        for (short int r=0; r<4; r++) {
            state.c[c].r.x[r] = in[r+4*c];
        }
    }
    state = AddRoundKey(state,expKey,0);
    for (short int round=1; round<Nr; round++) {
        state = MixColumns(ShiftRows(SubBytes(state)));
        state = AddRoundKey(state,expKey,round);
    }
    state = ShiftRows(SubBytes(state));
    state = AddRoundKey(state,expKey,Nr);
    for (short int c=0; c<4; c++) {
        for (short int r=0; r<4; r++) {
            out[r+4*c] = state.c[c].r.x[r];
        }
    }
}




void InvCipher (unsigned short int* in, short int Nr, expandedKey expKey, unsigned short int* out) {
    matrix state;
    for (short int c=0; c<4; c++) {
        for (short int r=0; r<4; r++) {
            state.c[c].r.x[r] = in[r+4*c];
        }
    }
    state = AddRoundKey(state,expKey,Nr);
    for (short int round=Nr-1; round>0; round--) {
        state = InvSubBytes(InvShiftRows(state));
        state = AddRoundKey(state,expKey,round);
        state = InvMixColumns(state);
    }
    state = InvSubBytes(InvShiftRows(state));
    state = AddRoundKey(state,expKey,0);
    for (short int c=0; c<4; c++) {
        for (short int r=0; r<4; r++) {
            out[r+4*c] = state.c[c].r.x[r];
        }
    }
}






unsigned char* AESECBCipher (unsigned char* in, int length, unsigned short int* key, short int Nk, short int Nr, int* resultLength) {
    if (length%16 == 0) {*resultLength = length;}
    else {*resultLength = length+16-length%16;}
    char* out = (char *)malloc(sizeof(char)*(*resultLength));
    expandedKey expKey = KeyExpansion(key, Nk, Nr);
    unsigned short int roundIn[16];
    unsigned short int roundOut[16];
    short int n = 0;
    while (n*16<length) {
        for (short int i=0; i<16; i++) {
            if (n*16+i<length) {
                roundIn[i] = in[n*16+i];
            }
            else {
                roundIn[i] = 0;
            }
        }
        
        Cipher(roundIn, Nr, expKey, roundOut);

        for (short int i=0; i<16; i++) {
            out[n*16+i] = roundOut[i];
        }

        n++;
    }
    return out;
}


unsigned char* AES128ECBCipher (unsigned char* in, int length, unsigned short int* key, int* resultLength) {
    return AESECBCipher(in,length,key,4,10,resultLength);
}

unsigned char* AES192ECBCipher (unsigned char* in, int length, unsigned short int* key, int* resultLength) {
    return AESECBCipher(in,length,key,6,12,resultLength);
}

unsigned char* AES256ECBCipher (unsigned char* in, int length, unsigned short int* key, int* resultLength) {
    return AESECBCipher(in,length,key,8,14,resultLength);
}









unsigned char* AESECBInvCipher (unsigned char* in, int length, unsigned short int* key, short int Nk, short int Nr, int* resultLength) {
    if (length%16 == 0) {*resultLength = length;}
    else {*resultLength = length+16-length%16;}
    char* out = (char *)malloc(sizeof(char)*(*resultLength));
    expandedKey expKey = KeyExpansion(key, Nk, Nr);
    unsigned short int roundIn[16];
    unsigned short int roundOut[16];
    short int n = 0;
    while (n*16<length) {
        for (short int i=0; i<16; i++) {
            if (n*16+i<length) {
                roundIn[i] = in[n*16+i];
            }
            else {
                roundIn[i] = 0;
            }
        }

        InvCipher(roundIn, Nr, expKey, roundOut);

        for (short int i=0; i<16; i++) {
            out[n*16+i] = roundOut[i];
        }

        n++;
    }
    return out;
}


unsigned char* AES128ECBInvCipher (unsigned char* in, int length, unsigned short int* key, int* resultLength) {
    return AESECBInvCipher(in,length,key,4,10,resultLength);
}

unsigned char* AES192ECBInvCipher (unsigned char* in, int length, unsigned short int* key, int* resultLength) {
    return AESECBInvCipher(in,length,key,6,12,resultLength);
}

unsigned char* AES256ECBInvCipher (unsigned char* in, int length, unsigned short int* key, int* resultLength) {
    return AESECBInvCipher(in,length,key,8,14,resultLength);
}






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
