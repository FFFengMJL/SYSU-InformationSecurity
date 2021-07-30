#include <stdio.h>
#include <stdlib.h>

#define F(b, c, d) ((b & c) | (~b & d))        // 第一轮
#define G(b, c, d) ((b & d) | (c & ~d))        // 第二轮
#define H(b, c, d) (b ^ c ^ d)                 // 第三轮
#define I(b, c, d) (c ^ (b | ~d))              // 第四轮
#define CLS(x, s) ((x << s) | (x >> (32 - s))) // 循环左移

typedef struct
{
  unsigned int content[4];
} MD5_CTX;

// 初始向量，小端
const unsigned int IV[4] = {
    0x67452301,
    0xEFCDAB89,
    0x98BADCFE,
    0x10325476};

unsigned int CV[4] = {
    0x67452301,
    0xEFCDAB89,
    0x98BADCFE,
    0x10325476};

// 各轮各次迭代运算采用的 T 值
const int T_TABLE[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

// 各轮各次迭代运算 (1 .. 64) 采用的左循环移位的位数 s 值
const int S_TABLE[] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

// 暂时没用上
const int X_TABLE[4][16] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},  // k = j
    {1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12},  // k = (1 + 5 * j) % 16
    {5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2},  // k = (5 + 3 * j) % 16
    {0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9}}; // k = (7 * j) % 16

unsigned int *MessagePadded; // 由字符串转化而成的32位块
unsigned long blockLength;   // 512位块的长度
unsigned char *messagePaddingTmp;

/**
 * 初始化向量
*/
void CVInit(unsigned int CV[4]);

/**
 * 填充步骤
 * @param originMessage char* 原始消息
 * @param messageLength unsigned long long 为了避免特殊情况，将明文长度（字节数）作为输入
*/
void padMessage(char *originMessage, unsigned long messageLength);

/**
 * 主要函数
 * @param originMessage char* 原始消息
 * @param messageLength unsigned long long 为了避免特殊情况，将明文长度（字节数）作为输入
*/
MD5_CTX MD5(char *originMessage, unsigned long messageLength);

/**
 * 将字符串转换为数字
 * @param src unsigned char* 源字符串
 * @param charLength unsigned long 字符串长度
 * @return 一个 unsigned int 数组指针
*/
unsigned int *MD5_Decode(unsigned char *src, unsigned int *dst, unsigned long charLength);

/**
 * 将数字转换为字符串
 * @param src unsigned int* 源数组
 * @param intLength unsigned long 数组长度
*/
unsigned char *MD5_Encode(unsigned int *src, unsigned char *dst, unsigned long intLength);

/**
 * MD5 压缩函数
 * @paramthisCV char* 输入向量，128位
 * @param Y int* 分组，每个分组为一个块，512位
 * @param res int* 返回结果
*/
void H_MD5(int *Y, unsigned int *res);

/**
 * 释放内存
*/
void clear();

void clear()
{
  free(MessagePadded);
  free(messagePaddingTmp);
}

MD5_CTX MD5(char *originMessage, unsigned long messageLength)
{
  MD5_CTX res;
  padMessage(originMessage, messageLength); // 消息填充
  CVInit(CV);                               // 初始化 CV

  for (int i = 0; i < blockLength; i++)
  {
    unsigned int tmp[4] = {0, 0, 0, 0};
    H_MD5(MessagePadded + i * 16, tmp);
    for (int j = 0; j < 4; j++)
    {
      // printf("%08x\n", CV[j]);
      CV[j] += tmp[j];
    }
  }

  for (int i = 0; i < 4; i++)
  {
    res.content[i] = CV[i];
  }

  clear();
  return res;
}

void CVInit(unsigned int thisCV[4])
{
  for (int i = 0; i < 4; i++)
  {
    thisCV[i] = IV[i];
  }
}

void padMessage(char *originMessage, unsigned long messageLength)
{
  blockLength = messageLength / 64 + (((messageLength * 8) % 512) >= 448 ? 2 : 1);
  messagePaddingTmp = (unsigned char *)malloc(blockLength * 64);
  for (int i = 0; i < messageLength; i++)
  {
    messagePaddingTmp[i] = originMessage[i];
  }

  // 后续填充为0
  for (int i = messageLength; i < blockLength * 64; i++)
  {
    messagePaddingTmp[i] = 0;
  }

  messagePaddingTmp[messageLength] = 0x80; // 结束的第一位为1

  MessagePadded = (unsigned int *)malloc(blockLength * 16 * sizeof(unsigned int) / sizeof(char));
  MD5_Decode(messagePaddingTmp, MessagePadded, blockLength * 64);
  unsigned int front32 = ((messageLength * 8) >> 32) & 0x00000000ffffffff; // 前32位，但是需要倒序放在最后
  unsigned int behind32 = (messageLength * 8) & 0x00000000ffffffff;        // 后32位，倒序放在最前
  MessagePadded[blockLength * 16 - 2] = behind32;
  MessagePadded[blockLength * 16 - 1] = front32;

  return;
}

unsigned char *MD5_Encode(unsigned int *src, unsigned char *dst, unsigned long intLength)
{
  for (int i = 0; i < intLength; i++)
  {
    dst[i * 4 + 3] = (src[i] >> 24) & 0x000000ff;
    dst[i * 4 + 2] = (src[i] >> 16) & 0x000000ff;
    dst[i * 4 + 1] = (src[i] >> 8) & 0x000000ff;
    dst[i * 4] = src[i] & 0x000000ff;
  }

  return dst;
}

unsigned int *MD5_Decode(unsigned char *src, unsigned int *dst, unsigned long charLength)
{
  for (int i = 0; i < charLength / 4; i++)
  {
    dst[i] = (src[i * 4]) |
             (src[i * 4 + 1] << 8) |
             (src[i * 4 + 2] << 16) |
             (src[i * 4 + 3] << 24);
  }

  return dst;
}

void H_MD5(int *Y, unsigned int res[4])
{
  unsigned int thisCV[4];
  unsigned int nextCV[4];

  for (int i = 0; i < 4; i++)
  {
    thisCV[i] = CV[i];
  }
  // 四轮循环，每轮循环16步
  for (int j = 0; j < 4; j++)
  {
    for (int i = 0; i < 16; i++)
    {
      // 每次迭代的参数都有变化
      switch (j)
      {
      case 0:
        nextCV[1] = thisCV[1] +
                    CLS((thisCV[0] +
                         F(thisCV[1], thisCV[2], thisCV[3]) +
                         Y[i] +
                         T_TABLE[i]),
                        S_TABLE[i]);
        break;
      case 1:
        nextCV[1] = thisCV[1] +
                    CLS((thisCV[0] +
                         G(thisCV[1], thisCV[2], thisCV[3]) +
                         Y[(1 + 5 * i) % 16] +
                         T_TABLE[i + j * 16]),
                        S_TABLE[i + j * 16]);
        break;
      case 2:
        nextCV[1] = thisCV[1] +
                    CLS((thisCV[0] +
                         H(thisCV[1], thisCV[2], thisCV[3]) +
                         Y[(5 + 3 * i) % 16] +
                         T_TABLE[i + j * 16]),
                        S_TABLE[i + j * 16]);
        break;
      case 3:
        nextCV[1] = thisCV[1] +
                    CLS((thisCV[0] +
                         I(thisCV[1], thisCV[2], thisCV[3]) +
                         Y[(7 * i) % 16] +
                         T_TABLE[i + j * 16]),
                        S_TABLE[i + j * 16]);
        break;
      default:
        break;
      }
      nextCV[2] = thisCV[1];
      nextCV[3] = thisCV[2];
      nextCV[0] = thisCV[3];

      // 迭代
      for (int i = 0; i < 4; i++)
      {
        thisCV[i] = nextCV[i];
      }

      // if ((j * 16 + i) % 16 == 15)
      // {
      //   printf("迭代：%d\n", j * 16 + i + 1);
      //   for (int i = 0; i < 4; i++)
      //   {
      //     printf("%08x\n", thisCV[i]);
      //   }
      //   putchar('\n');
      // }
    }
  }

  for (int i = 0; i < 4; i++)
  {
    res[i] = thisCV[i];
  }
}
