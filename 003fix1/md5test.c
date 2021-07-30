#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "md5.c"

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("usage: ./md5 inputFile\n");
    return 0;
  }
  FILE *inputFile;
  struct stat inputFileBuffer;
  stat(argv[1], &inputFileBuffer);
  long inputFileSize = inputFileBuffer.st_size;      // 输入文件的大小
  char *message = (char *)malloc(inputFileSize + 1); // 消息

  // 读取文件
  inputFile = fopen(argv[1], "r");
  inputFileSize = fread(message, 1, inputFileSize, inputFile);

  printf("输入消息为：\n%s\n", message);

  // 获取结果
  MD5_CTX res = MD5(message, inputFileSize);
  unsigned char rres[16];
  MD5_Encode(res.content, rres, 4);

  // 输出结果
  printf("哈希结果为: \n");
  for (int i = 0; i < 16; i++)
  {
    printf("%02x", rres[i]);
  }
  printf("\n");

  return 0;
}