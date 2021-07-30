#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "md5.c"

int main(int argc, char *argv[])
{
  int i;
  // unsigned char encrypt[] = "12345678910\n12345678910\n12345678910\n12345678910\n12345678910"; // e10adc3949ba59abbe56e057f20f883e
  unsigned char encrypt[] = "My name is mijialong.\nThis is SYSU.\nWe have twenty-first weeks!\nSYSU is a 985 and 211 university?\nThis is a test file.\n"; // e10adc3949ba59abbe56e057f20f883e

  unsigned char decrypt[16];

  for (int i = 0; i < 16; i++)
  {
    decrypt[i] = 0;
  }
  MD5_CTX md5;
  MD5Init(&md5);
  MD5Update(&md5, encrypt, strlen((char *)encrypt));
  MD5Final(&md5, decrypt);
  printf("加密前:%s\n加密后:", encrypt);
  for (i = 0; i < 16; i++)
  {
    printf("%02x", decrypt[i]);
  }
  printf("\n");

  return 0;
}