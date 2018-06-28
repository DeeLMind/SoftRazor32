#ifndef _INC_MD5C_H_
#define _INC_MD5C_H_

#ifndef _STDINT
#include <stdint.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef GUID_DEFINED
#include <Guiddef.h>
#endif

/* MD5 context. */
typedef struct _MD5_CTX
{
  uint32_t        state[4];        /* state (ABCD) */
  uint32_t        count[2];        /* number of bits, modulo 2^64 (lsb first) */
  uint8_t         buffer[64];      /* input buffer */
} MD5_CTX, *PMD5_CTX;

typedef union _UNIMD5
{
  uint8_t         digest_byte[16];
  uint16_t        digest_word[8];
  uint32_t        digest_dword[4];
  uint64_t        digest_qword[2];
  GUID            digest_guid;
} UNIMD5, *PUNIMD5;

void          MD5_Init(PMD5_CTX context);
void          MD5_Update(PMD5_CTX context, uint8_t *input, uint32_t inputLen);
void          MD5_UpdateString(PMD5_CTX context, const char *string);
void          MD5_String(char *string, uint8_t digest[16]);
void          MD5_Final(uint8_t digest[16], PMD5_CTX context);

#endif