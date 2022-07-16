#pragma once
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

char *b64_encode(const unsigned char *in, size_t len);

#ifdef __cplusplus
}
#endif