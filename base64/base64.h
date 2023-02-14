
#ifndef BASE46_H
#define BASE46_H

#include <string.h>
#include <stdlib.h>

int base64_decode(char *text, unsigned char *dst, int numBytes );
int base64_encode(char *text, int numBytes, char **encodedText);

#endif //BASE46_H