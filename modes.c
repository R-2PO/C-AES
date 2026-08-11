#include "aes.c"


/* GCM */





#define GCMBLOCKLENGTH 128
#define GCMBLOCKBYTES 16


void printGCMBlock (GCMBlock X) {
    for (unsigned short int i=0; i<16; i++) {
        printf("%02x ", X.val[i]);
    }
    printf("| %i\n", X.len);
}


GCMBlock GCMShiftRight (GCMBlock X, unsigned short int s) {
    // Shifts the bit string X to the right by s bits
    
    GCMBlock result;
    result.len = X.len - s;

    // Initialize result to the same value as X
    for (unsigned short int i=0; i<GCMBLOCKBYTES; i++) {
        result.val[i] = X.val[i];
    }

    unsigned char carry = 0;
    unsigned char mask;
    unsigned char old;

    // The first byte that actually matters
    unsigned short int start = GCMBLOCKBYTES - (X.len/8);
    if (X.len%8 != 0) {
        start--;
    }

    // Shift by whole blocks
    while (s >= 8) {
        for (unsigned short int j=GCMBLOCKBYTES-1; j>start; j--) {
            result.val[j] = result.val[j-1];
        }
        result.val[start] = 0;
        s = s-8;
        start++;
    }

    if (s == 0) {
        // Avoid the last lines, it's faster (and I'm too lazy to think about the case s=0 for the rest of the code)
        return result;
    }

    mask = (1<<s) - 1; // s ones at the end

    // Shift every byte while carrying left to right
    for (unsigned short i = start; i<GCMBLOCKBYTES; i++) {
        old = result.val[i];
        result.val[i] = (result.val[i] >> s) + carry;
        carry = (old & mask) << (8-s);
    }
    
    return result;
}




GCMBlock GCMMSB (GCMBlock X, unsigned short int s) {
    // Returns the s most significant bits of the bit string X

    // If you shift to the right enough, you only get the MSB
    return GCMShiftRight(X, X.len-s);
}


GCMBlock GCMLSB (GCMBlock X, unsigned short int s) {
    // Returns the s least significant bits of the bit string X
    
    GCMBlock result;
    GCMBlock mask = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, .len=128};

    unsigned short int i = 15;
    result.len = s;

    while (s >= 8) {
        mask.val[i] = 0xff;
        i--;
        s = s-8;
    }

    mask.val[i] = (1<<s)-1;

    for (unsigned short int i=0; i<16; i++) {
        result.val[i] = X.val[i] & mask.val[i];
    }

    return result;
}



GCMBlock GCMConcatenate (GCMBlock A, GCMBlock B) {
    // Concatenates the two bit strings A and B
   
    GCMBlock result;

    // First, shift A (stored in result) to the left by a whole number of bytes, we'll compensate later by shifting right

    for (unsigned short int i=0; i<GCMBLOCKBYTES; i++) {
        result.val[i] = A.val[i];
    }

    unsigned short int n = B.len/8; // The number of times to shift
    if (B.len%8 != 0) {
        n++;
    }

    for (unsigned short int i=0; i<n; i++) {
        for (unsigned short int j=0; j<GCMBLOCKBYTES-1; j++) {
            result.val[j] = result.val[j+1];
        }
        result.val[GCMBLOCKBYTES-1] = 0;
    }

    // Compensate by shifting right
    result.len = A.len + n*8;
    if (B.len%8 != 0) {
        result = GCMShiftRight(result, 8-(B.len%8));
    }

    for (unsigned short int i=0; i<GCMBLOCKBYTES;i++) {
        result.val[i] = result.val[i] + B.val[i];
    }

    return result;
}


GCMBlock GCMAdd (GCMBlock A, GCMBlock B) {
    // Add A and B together (this is a REAL addition, not the XOR shown as \oplus in NIST SP800-38D)
    // Stops at the maximum size of the bit strings, and ignores any remainder (the same as mod 2^max(A.len,B.len) )
    // This is because this function will be used by GCMInc

    GCMBlock result = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
    if (A.len >= B.len) {
        result.len = A.len;
    }
    else {
        result.len = B.len;
    }

    unsigned short int remainderMask = 0xff00; // The mask to get the remainder
    unsigned short int valueMask = 0xff;
    unsigned short int remainder = 0;
    unsigned short int sum;

    unsigned short int start = GCMBLOCKBYTES - (result.len/8);
    if (result.len%8 != 0) {
        start--;
    }
    
    for (unsigned short int i=GCMBLOCKBYTES - 1; i>=start; i--) {
        sum = A.val[i] + B.val[i] + remainder;
        remainder = (sum & remainderMask) >> 8;
        result.val[i] = sum & valueMask;
    }

    // Only keep the bits within the length of the block
    unsigned char mask = (1 << (result.len%8)) - 1;
    if (result.len%8 != 0) {
        result.val[start] = result.val[start] & mask;
    }

    return result;
}



GCMBlock GCMXOR (GCMBlock A, GCMBlock B) {
    // Perform a bitwise XOR operation on A and B
    // The length of the result is the same as the max of the lengths of A and B
    
    // Create result and set its length to the max of A.len and B.len
    GCMBlock result = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
    if (A.len >= B.len) {
        result.len = A.len;
    }
    else {
        result.len = B.len;
    }
    
    // The first byte that matters
    unsigned short int start = GCMBLOCKBYTES - (result.len/8);
    if (result.len%8 != 0) {
        start--;
    }

    // Perform the XOR on every byte
    for (unsigned short int i=start; i<GCMBLOCKBYTES; i++) {
        result.val[i] = A.val[i] ^ B.val[i];
    }

    return result;
}



GCMBlock GCMInc (GCMBlock X, unsigned long long int s) {
    // Increment the last s bits of X

    // That's litterally the implementation in NIST SP800-38D
    GCMBlock one = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, .len = 1};
    GCMBlock A = GCMMSB(X,X.len-s);
    GCMBlock B = GCMLSB(X,s);
    B = GCMAdd(B,one);
    return GCMConcatenate(A,B);
}




GCMBlock GCMMultiply (GCMBlock A, GCMBlock B) {
    // Multiplies two blocks A and B
    // The length of both blocks must be 128.

    GCMBlock Z = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, .len = 128};
    GCMBlock V = B;
    GCMBlock R = {.val = {0xe1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, .len=128};

    unsigned char last; // To keep the last byte of V after shifting it

    for (unsigned char i=0; i<128; i++) {
        if ((A.val[i/8] & (1<<(7-i%8))) != 0) { // Get the i-th bit of A
            Z = GCMXOR(Z,V);
        }
        last = GCMLSB(V,1).val[GCMBLOCKBYTES-1];
        V = GCMShiftRight(V,1);
        V.len = 128; // Even though we shift, the length shouldn't change
        if (last == 1) {
            V = GCMXOR(V,R);
        }
    }

    return Z;
}




GCMBlock GCMGHASH (GCMBlock* X, unsigned long long int m, GCMBlock H) {
    // Returns the GHASH of the list of blocks X (of length m), using the hash subkey H
    
    GCMBlock Y = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, .len = 128};

    for (unsigned short int i=0; i<m; i++) {
        Y = GCMXOR(Y,X[i]);
        Y = GCMMultiply(Y,H);
    }

    return Y;
}



GCMBlock* GCMGCTRAES (GCMBlock* X, GCMBlock ICB, unsigned long long int n, expandedKey expKey) {
    // Performs GCTR with AES128 on X, that contains n-1 complete GCMBlocks and the last (nth) GCMBlock may not be full (with data towards the end)
    // ICB is the initial counter block

    GCMBlock* Y = (GCMBlock *)malloc(sizeof(GCMBlock)*(n));
    GCMBlock CB[n];
    GCMBlock CBOut = {.len = 128};
    unsigned short int CBConverted[16];
    unsigned short int CBConvertedOut[16];

    if (n == 0) {
        return Y;
    }


    // Initialize CB with the same value as ICB
    for (unsigned short int i=0; i<GCMBLOCKBYTES; i++) {
        CB[0].val[i] = ICB.val[i];
    }
    CB[0].len = 128;

    for (unsigned short int i=1; i<n; i++) {
        CB[i] = GCMInc(CB[i-1],32);
        CB[i].len = 128;
        Y[i].len = 128;
    }

    for (unsigned short int i=0; i<n-1; i++) {
        // Convert CB into the right format for AES Ciphering
        for (unsigned short int j=0; j<16; j++) {
            CBConverted[j] = CB[i].val[j];
        }
        Cipher(CBConverted, 10, expKey, CBConvertedOut);

        //Convert back into GCMBlock
        for (unsigned short int j=0; j<16; j++) {
            CBOut.val[j] = CBConvertedOut[j];
        }
        Y[i] = GCMXOR(X[i],CBOut);
    }

    // Convert CB into the right format for AES Ciphering
    for (unsigned short int j=0; j<16; j++) {
        CBConverted[j] = CB[n-1].val[j];
    }
    Cipher(CBConverted, 10, expKey, CBConvertedOut);

    //Convert back into GCMBlock
    for (unsigned short int j=0; j<16; j++) {
        CBOut.val[j] = CBConvertedOut[j];
    }
    
    CBOut = GCMMSB(CBOut, X[n-1].len);
    Y[n-1] = GCMXOR(X[n-1],CBOut);
    Y[n-1].len = X[n-1].len;

    return Y;
}













// Now for the hardest part...

// TODO: Put all the identical code for Cipher and InvCipher in a separate function

unsigned char* AESGCMCipher (unsigned char* P, unsigned long long int PLen, unsigned char* A, unsigned long long int ALen, GCMBlock IV, unsigned short int* key, unsigned char* T, unsigned short int tagbLen, unsigned long long int* resultLen) {
    // Performs an AES cipher in GCM mode
    // P is the plaintext to cipher, of length in bytes PLen
    // A is the additional authenticated data, of length in bytes ALen
    // IV is the initialization vector, must be 96 bits long, as recommended by NIST SP800-38D
    // key is the key used for the cipher
    // T is a pointer to the variable where the tag will be stored
    // tagbLen is the length of the tag in bits, and should be either 128, 120, 112, 104 or 96
    // resultLen is the pointer where the length of the result will be stored (in bytes)
    //
    // Returns the cipher

    unsigned short int HIn[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // The input for the first cipher
    unsigned short int HOut[16]; // The output for the first cipher

    GCMBlock H;
    GCMBlock J0;
    GCMBlock S;
    GCMBlock* ABlocks;
    GCMBlock* CBlocks;
    GCMBlock* PBlocks;
    GCMBlock* SInter; // The intermediate list of GCMBlocks for obtaining S
    GCMBlock* TInter; // The same thing, but for T
    GCMBlock one = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, .len = 32}; // To be used when initializing J0
    GCMBlock zero = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, .len = 128}; // To be used for concatenating

    unsigned long long int m; // The number of GCMBlocks necessary to store A
    unsigned long long int n; // The number of GCMBlocks necessary to store P
    unsigned long long int p; // The number of GCMBlocks necessary to store SInter
    unsigned long long int u;
    unsigned long long int v;
    unsigned long long int CbLen = PLen*8; // The bit length of C, which is the same as the bit length of P
    unsigned long long int AbLen = ALen*8; // The bit length of A
    unsigned long long int mask;

    unsigned char tagLen = tagbLen/8;
    unsigned char* C;

    // Convert P to a list of GCMBlocks
    n = PLen / 16;
    if (PLen%16 != 0) {
        n++;
    }

    PBlocks = (GCMBlock *)malloc(sizeof(GCMBlock)*(n));

    for (unsigned long long int i=0; i<n-1; i++) {
        for (unsigned char j=0; j<16; j++) {
            PBlocks[i].val[j] = P[i*16+j];
        }

        // Set the length of the i-th block
        PBlocks[i].len = 128;
    }

    if (PLen%16 == 0) {
        for (unsigned char j=0; j<16; j++) {
            PBlocks[n-1].val[j] = P[(n-1)*16 + j];
        }
        PBlocks[n-1].len = 128;
    }
    else {
        for (unsigned char j=0; j<PLen%16; j++) {
            PBlocks[n-1].val[16-(PLen%16)+j] = P[(n-1)*16+j];
        }
        PBlocks[n-1].len = (PLen%16)*8;
    }




    // Convert A to a list of GCMBlocks
    m = ALen / 16;
    if (ALen%16 != 0) {
        m++;
    }

    ABlocks = (GCMBlock *)malloc(sizeof(GCMBlock)*(m));

    for (unsigned long long int i=0; i<m-1; i++) {
        for (unsigned char j=0; j<16; j++) {
            ABlocks[i].val[j] = A[i*16+j];
        }

        // Set the length of the i-th block
        ABlocks[i].len = 128;
    }

    if (ALen%16 == 0) {
        for (unsigned char j=0; j<16; j++) {
            ABlocks[m-1].val[j] = A[(m-1)*16 + j];
        }
        ABlocks[m-1].len = 128;
    }
    else {
        for (unsigned char j=0; j<ALen%16; j++) {
            ABlocks[m-1].val[16-(ALen%16)+j] = A[(m-1)*16+j];
        }
        ABlocks[m-1].len = (ALen%16)*8;
    }



    expandedKey expKey = KeyExpansion(key,4,10);

    // Initialize H
    Cipher(HIn, 10, expKey, HOut);

    // Convert HOut into a GCMBlock
    for (unsigned char i=0; i<16; i++) {
        H.val[i] = HOut[i];
    }
    H.len = 128;

    // Initialize J0
    J0 = GCMConcatenate(IV, one);

    CBlocks = GCMGCTRAES(PBlocks,GCMInc(J0,32),n,expKey);

    u = 128*n - CbLen;

    v = 128*m - AbLen;

    p = (AbLen + v + CbLen + u + 128)/128; // S is necessarily a whole number of blocks
    


    // Build S
    SInter = (GCMBlock *)malloc(sizeof(GCMBlock)*(p));
    // The first m-1 blocks of ABlocks are full, we can just add them
    for (unsigned long long int i=0; i<m-1; i++) {
        SInter[i] = ABlocks[i];
    }
    // Then we can shift the last block of ABlocks by concatenating it with the right amount of zeros
    if (ABlocks[m-1].len != 128) {
        zero.len = 128-ABlocks[m-1].len;
        SInter[m-1] = GCMConcatenate(ABlocks[m-1],zero);
    }
    else {
        SInter[m-1] = ABlocks[m-1];
    }

    // The first n-1 blocks of CBlocks are full, we can just add them
    for (unsigned long long int i=0; i<n-1; i++) {
        SInter[m+i] = CBlocks[i];
    }
    // Then we can shift the last block of ABlocks by concatenating it with the right amount of zeros
    if (CBlocks[n-1].len != 128) {
        zero.len = 128-CBlocks[n-1].len;
        SInter[m+n-1] = GCMConcatenate(CBlocks[n-1],zero);
    }
    else {
        SInter[m+n-1] = CBlocks[n-1];
    }

    // Add AbLen
    mask = 0xff;
    for (unsigned char i=0; i<8; i++) {
        SInter[p-1].val[7-i] = (AbLen >> 8*i) & mask;
    }

    // Add CbLen
    for (unsigned char i=0; i<8; i++) {
        SInter[p-1].val[15-i] = (CbLen >> 8*i) & mask;
    }
    SInter[p-1].len = 128;

    S = GCMGHASH(SInter, p, H);

    TInter = GCMGCTRAES(&S,J0,1,expKey);

    for (unsigned short int i=0; i<tagLen; i++) {
        T[i] = TInter[0].val[(16-tagLen)+i];
    }
    
    *resultLen = PLen;


    // Convert C from GCMBlocks
    C = (unsigned char *)malloc(sizeof(unsigned char)*(PLen));

    // The n-1 first blocks pose no problem as they are full
    for (unsigned long long int i=0; i<n-1; i++) {
        for (unsigned char j=0; j<16; j++) {
            C[i*16+j] = CBlocks[i].val[j];
        }
    }

    // Get the bytes from the last block
    for (unsigned char i=0; i<PLen%16; i++) {
        C[(n-1)*16+i] = CBlocks[n-1].val[(16-PLen%16)+i];
    }

    return C;
}





unsigned char* AESGCMInvCipherAndAuthenticate (unsigned char* C, unsigned long long int CLen, unsigned char* A, unsigned long long int ALen, GCMBlock IV, unsigned short int* key, unsigned char* T, unsigned short int tagbLen, unsigned char* success, unsigned long long int* resultLen) {
    // Performs an AES decipher in GCM mode
    // C is the ciphertext to decipher, of length in bytes CLen
    // A is the additional authenticated data, of length in bytes ALen
    // IV is the initialization vector, must be 96 bits long, as recommended by NIST SP800-38D
    // key is the key used for the cipher
    // T is the authentication tag
    // tagbLen is the length of the tag in bits, and should be either 128, 120, 112, 104 or 96
    // success is a pointer to a variable that will be set to 1 if the authentication was successful, and 0 if it wasn't
    // resultLen is the pointer where the length of the result will be stored (in bytes)
    //
    // returns the cipher
    
    unsigned short int HIn[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // The input for the first cipher
    unsigned short int HOut[16]; // The output for the first cipher

    GCMBlock H;
    GCMBlock J0;
    GCMBlock S;
    GCMBlock* ABlocks;
    GCMBlock* CBlocks;
    GCMBlock* PBlocks;
    GCMBlock* SInter; // The intermediate list of GCMBlocks for obtaining S
    GCMBlock* TInter; // The same thing, but for T
    GCMBlock one = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, .len = 32}; // To be used when initializing J0
    GCMBlock zero = {.val = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, .len = 128}; // To be used for concatenating

    unsigned long long int m; // The numebr of GCMBlocks necessary to store A
    unsigned long long int n; // The number of GCMBlocks necessary to store C
    unsigned long long int p; // The number of GCMBlocks necessary to store SInter
    unsigned long long int u;
    unsigned long long int v;
    unsigned long long int CbLen = CLen*8; // The bit length of C, which is the same as the bit length of P
    unsigned long long int AbLen = ALen*8; // The bit length of A
    unsigned long long int mask;
    unsigned char tagLen = tagbLen/8;

    unsigned char* P;
    unsigned char* newT; // The new tag

    if (tagbLen%8 != 0 || tagbLen < 96 || tagbLen > 128) {
        *success = 0;
        *resultLen = 0;
        P = (unsigned char *)malloc(sizeof(unsigned char)*(1));
        P[0] = 0;
        return P;
    }


    // Convert C to a list of GCMBlocks
    n = CLen / 16;
    if (CLen%16 != 0) {
        n++;
    }

    CBlocks = (GCMBlock *)malloc(sizeof(GCMBlock)*(n));

    for (unsigned long long int i=0; i<n-1; i++) {
        for (unsigned char j=0; j<16; j++) {
            CBlocks[i].val[j] = C[i*16+j];
        }

        // Set the length of the i-th block
        CBlocks[i].len = 128;
    }

    if (CLen%16 == 0) {
        for (unsigned char j=0; j<16; j++) {
            CBlocks[n-1].val[j] = C[(n-1)*16 + j];
        }
        CBlocks[n-1].len = 128;
    }
    else {
        for (unsigned char j=0; j<CLen%16; j++) {
            CBlocks[n-1].val[16-(CLen%16)+j] = C[(n-1)*16+j];
        }
        CBlocks[n-1].len = (CLen%16)*8;
    }




    // Convert A to a list of GCMBlocks
    m = ALen / 16;
    if (ALen%16 != 0) {
        m++;
    }

    ABlocks = (GCMBlock *)malloc(sizeof(GCMBlock)*(m));

    for (unsigned long long int i=0; i<m-1; i++) {
        for (unsigned char j=0; j<16; j++) {
            ABlocks[i].val[j] = A[i*16+j];
        }

        // Set the length of the i-th block
        ABlocks[i].len = 128;
    }

    if (ALen%16 == 0) {
        for (unsigned char j=0; j<16; j++) {
            ABlocks[m-1].val[j] = A[(m-1)*16 + j];
        }
        ABlocks[m-1].len = 128;
    }
    else {
        for (unsigned char j=0; j<ALen%16; j++) {
            ABlocks[m-1].val[16-(ALen%16)+j] = A[(m-1)*16+j];
        }
        ABlocks[m-1].len = (ALen%16)*8;
    }




    expandedKey expKey = KeyExpansion(key,4,10);

    // Initialize H
    Cipher(HIn, 10, expKey, HOut);
    
    // Convert HOut into a GCMBlock
    for (unsigned char i=0; i<16; i++) {
        H.val[i] = HOut[i];
    }
    H.len = 128;

    // Initialize J0
    J0 = GCMConcatenate(IV, one);

    PBlocks = GCMGCTRAES(CBlocks,GCMInc(J0,32),n,expKey);

    u = 128*n - CbLen;

    v = 128*m - AbLen;

    p = (AbLen + v + CbLen + u + 128)/128; // S is necessarily a whole number of blocks




    // Build S
    SInter = (GCMBlock *)malloc(sizeof(GCMBlock)*(p));
    // The first m-1 blocks of ABlocks are full, we can just add them
    for (unsigned long long int i=0; i<m-1; i++) {
        SInter[i] = ABlocks[i];
    }
    // Then we can shift the last block of ABlocks by concatenating it with the right amount of zeros
    if (ABlocks[m-1].len != 128) {
        zero.len = 128-ABlocks[m-1].len;
        SInter[m-1] = GCMConcatenate(ABlocks[m-1],zero);
    }
    else {
        SInter[m-1] = ABlocks[m-1];
    }

    // The first n-1 blocks of CBlocks are full, we can just add them
    for (unsigned long long int i=0; i<n-1; i++) {
        SInter[m+i] = CBlocks[i];
    }
    // Then we can shift the last block of ABlocks by concatenating it with the right amount of zeros
    if (CBlocks[n-1].len != 128) {
        zero.len = 128-CBlocks[n-1].len;
        SInter[m+n-1] = GCMConcatenate(CBlocks[n-1],zero);
    }
    else {
        SInter[m+n-1] = CBlocks[n-1];
    }

    // Add AbLen
    mask = 0xff;
    for (unsigned char i=0; i<8; i++) {
        SInter[p-1].val[7-i] = (AbLen >> 8*i) & mask;
    }

    // Add CbLen
    for (unsigned char i=0; i<8; i++) {
        SInter[p-1].val[15-i] = (CbLen >> 8*i) & mask;
    }
    SInter[p-1].len = 128;

    S = GCMGHASH(SInter, p, H);

    TInter = GCMGCTRAES(&S,J0,1,expKey);

    // Allocate memory to store the new tag
    newT = (unsigned char *)malloc(sizeof(unsigned char)*(tagLen));

    for (unsigned short int i=0; i<tagLen; i++) {
        newT[i] = TInter[0].val[(16-tagLen)+i];
    }
    
    *resultLen = CLen;
    *success = 1;

    for (unsigned short int i=0; i<tagLen; i++) {
        if (newT[i] != T[i]) {
            // The two tags are not equal, authentication fails
            *success = 0;
        }
    }

    // Convert P from GCMBlocks
    P = (unsigned char *)malloc(sizeof(unsigned char)*(CLen));

    // The n-1 first blocks pose no problem as they are full
    for (unsigned long long int i=0; i<n-1; i++) {
        for (unsigned char j=0; j<16; j++) {
            P[i*16+j] = PBlocks[i].val[j];
        }
    }

    // Get the bytes from the last block
    for (unsigned char i=0; i<CLen%16; i++) {
        P[(n-1)*16+i] = PBlocks[n-1].val[(16-CLen%16)+i];
    }

    return P;
}
