#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "md5.c"

#define BLOCKSIZE 64

unsigned long b = 0;       // length (bits) of input block
unsigned long kLength = 0; // length (bits) of input key
char *M;                   // Message
char *k;                   // 密钥
char *KPlus;               // 根据输入内容和密钥生成的数据块
char *Si;                  // K+ ^ ipad
char *So;                  // K+ ^ opad

#define ipad 0x36 // 00110110
#define opad 0x5c // 01011100

/**
 * 获取文件的大小，并设置初始的相关值
 * @param inputFilename char* 输入文件名
 * @param keyFilename char* 密钥文件名
 * @return 1 | 0 ，其中0代表密钥不符合要求
*/
int getFileSize(char *inputFilename, char *keyFilename);

/**
 * 生成 K+ ，顺带生成 Si 和 So
*/
void generateKPlus();

/**
 * 哈希函数
 * @param S unsigned char* 第一个字符串
 * @param SLength unsigned long 第一个字符串的长度
 * @param M unsigned char* 第二个字符串
 * @param MLength unsigned long 第二个字符串的长度
 * @return MD5_CTX 结构体
*/
MD5_CTX Hash(unsigned char *S, unsigned long SLength, unsigned char *M, unsigned long MLength);

/**
 * 释放内存
*/
void freeAll();

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("usage: ./a.out inputFile keyFile\n");
    return 0;
  }
  else
  {
    FILE *inputFile, *keyFile;
    if (!getFileSize(argv[1], argv[2]))
    {
      printf("密钥和文本不匹配\n");
      return 0;
    }

    inputFile = fopen(argv[1], "r");
    keyFile = fopen(argv[2], "r");

    fread(M, 1, b, inputFile);
    fread(k, 1, kLength, keyFile);

    generateKPlus();

    printf("消息为：\n%s\n", M);

    // 第一次哈希
    MD5_CTX firstResult = Hash(Si, BLOCKSIZE, M, b); // 第一次结果
    unsigned char firstResultString[16];             // 第一次结果的字符串
    MD5_Encode(firstResult.content, firstResultString, 4);
    printf("第一次结果：\n");
    for (int i = 0; i < 4 * 4; i++)
    {
      printf("%02x", firstResultString[i]);
    }
    putchar('\n');

    //第二次哈希
    MD5_CTX secondResult = Hash(So, BLOCKSIZE, firstResultString, 16); // 第二次结果
    unsigned char secondResultString[16];                              // 第二次结果的字符串
    printf("第二次结果：\n");
    MD5_Encode(secondResult.content, secondResultString, 4);
    for (int i = 0; i < 4 * 4; i++)
    {
      printf("%02x", secondResultString[i]);
    }
    putchar('\n');

    fclose(inputFile);
    fclose(keyFile);
    freeAll();
    return 0;
  }
}

MD5_CTX Hash(unsigned char *S, unsigned long SLength, unsigned char *M, unsigned long MLength)
{
  /**
   * 拼接两个字符串
   * 不敢用 strcat() 怕有 0 的存在
  */
  unsigned char SM[MLength + SLength + 1];
  SM[SLength + MLength] = 0;

  // 第一段
  for (unsigned long i = 0; i < SLength; i++)
  {
    SM[i] = S[i];
  }

  // 第二段
  for (unsigned long i = 0; i < MLength; i++)
  {
    SM[i + SLength] = M[i];
  }

  MD5_CTX res = MD5(SM, SLength + MLength);

  return res;
}

void freeAll()
{
  free(M);
  free(k);
  free(KPlus);
  free(Si);
  free(So);
}

int getFileSize(char *inputFilename, char *keyFilename)
{
  struct stat inputFileBuffer, keyFileBuffer;
  stat(inputFilename, &inputFileBuffer);
  stat(keyFilename, &keyFileBuffer);
  b = inputFileBuffer.st_size;     // 消息大小
  kLength = keyFileBuffer.st_size; // 密钥长度
  printf("文件大小为 %ld 字节\n密钥长度为 %ld 字节\n", b, kLength);

  // 如果密钥不符合要求
  if (BLOCKSIZE < kLength)
  {
    return 0;
  }
  else
  {
    M = (char *)malloc(b);
    k = (char *)malloc(BLOCKSIZE);
    return 1;
  }
}

void generateKPlus()
{
  KPlus = (char *)malloc(BLOCKSIZE);

  // 右边补位0
  for (unsigned long i = 0; i < kLength; i++)
  {
    KPlus[i] = k[i];
  }

  for (unsigned long i = kLength; i < BLOCKSIZE; i++)
  {
    KPlus[i] = 0;
  }

  // 顺带获取 Si 和 So
  Si = (char *)malloc(BLOCKSIZE);
  So = (char *)malloc(BLOCKSIZE);

  for (unsigned long i = 0; i < BLOCKSIZE; i++)
  {
    Si[i] = KPlus[i] ^ ipad;
    So[i] = KPlus[i] ^ opad;
  }
}