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
