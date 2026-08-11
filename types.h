typedef struct {
    unsigned short int x[4];
} word;

typedef struct {
    word r;
} column;

typedef struct {
    column c[4];
} matrix;


typedef struct {
    word k[60];
} expandedKey;

typedef struct {
    // Well... That's 128 bits (8*16)... Not sure about the way I'm doing it though...
    // val[0] is the most significant byte and val[15] the least significant byte
    unsigned char val[16];
    unsigned short int len;
} GCMBlock;
