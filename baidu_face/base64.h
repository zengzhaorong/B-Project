#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>

int base64_encode(const unsigned char *indata, int inlen, unsigned char *outdata, int *outlen);
int base64_decode(const unsigned char *indata, int inlen, unsigned char *outdata, int *outlen);


#endif // BASE64_H
