unsigned short int concatenate (unsigned short int a, unsigned short int b, short int lenB) {
    unsigned short int result = a;
    result = result << lenB;
    result = result + b;
    return result;
}


unsigned short int LSB (unsigned short int a, short int s) {
    unsigned short int mask = 0;
    unsigned short int j = 1;
    unsigned short int result;

    for (short int i=0; i<s; i++) {
        mask += j;
        j = j<<1;
    }
    return mask&a;
}


unsigned short int MSB (unsigned short int a, short int s, short int lenA) {
    if (s>lenA) {
        //Return 0 if asked for too many bits
        return 0;
    }

    return a>>(lenA-s);
}
