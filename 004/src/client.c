#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "./des.c"

#define AS_HOST "127.0.0.1"
#define SS_HOST "127.0.0.1"
#define TGS_HOST "127.0.0.1"
#define AS_PORT 23333
#define SS_PORT 23334
#define TGS_PORT 23335

/**
 * 新建 socket
 * @param addr char* 目标 ip
 * @param port int 目标 ip 的端口
 * @return sockaddr_in 结构体
*/
struct sockaddr_in *newSockaddr_in(char *addr, int port);

/**
 * 判断响应是否有效
 * @param response char* 响应字符串
 * @return 0 为有效，1 为无效
*/
int isInvalid(char *response);

struct sockaddr_in *newSockaddr_in(char *addr, int port)
{
  struct sockaddr_in *sockaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  sockaddr->sin_addr.s_addr = inet_addr(addr);
  sockaddr->sin_family = AF_INET;
  sockaddr->sin_port = htons(port);
  return sockaddr;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("usage: ./client keyClientFile\n");
    return 0;
  }
  else
  {
    // 获取主密钥
    FILE *keyClientFile = fopen(argv[1], "r");
    char keyClient[17];
    fread(keyClient, 1, 16, keyClientFile);

    int clientSock; // 客户端套接字

    const char clientUser[] = "Bob";        // 用户 ID
    const char serviceID[] = "testService"; // 服务 ID

    time_t now;

    char request[1024];  // 请求
    char response[1024]; // 响应
    int resLen;          // 响应字符串长度
    char error[1024];    // 错误相关

    char keyCTGS[17]; // key_client_TGS
    char keyCSS[17];  // key_client_SS

    // 消息 A 相关
    char messageA[1024];
    char messageA_encoded[1027];
    char messageA_decoded[1024];
    int messageA_encoded_Len = 0;

    // 消息 B 相关
    char messageB[1024];

    // 消息 C 相关
    char messageC[1024];

    // 消息 D 相关
    char messageD[1024];
    char messageD_encoded[1024];
    char messageD_encoded_transfered[1024];
    int messageD_encoded_len;

    // 消息 E 相关
    char messageE[1024];
    char messageE_encoded[1024];
    char messageE_decoded[1024];
    char messageE_encoded_len = 0;

    // 消息 F 相关
    char messageF[1024];
    char messageF_encoded[1024];
    char messageF_decoded[1024];
    char messageF_encoded_len = 0;

    // 消息 G 相关
    char messageG[1024];
    char messageG_encoded[1024];
    char messageG_encoded_transfered[1024];
    char messageG_encoded_len = 0;

    // 消息 H 相关
    char messageH[1024];
    char messageH_encoded[1024];
    char messageH_decoded[1024];
    char messageH_encoded_transfered[1024];
    int messageH_encoded_len = 0;

    // AS
    {
      printf("-------AS START-------\n");

      // 新建套接字
      if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
        printf("[ERROR]: client socket failed\n");
        return 1;
      }

      // 尝试连接
      struct sockaddr_in *AS_sockaddr = newSockaddr_in(AS_HOST, AS_PORT);
      if (connect(clientSock, (struct sockaddr *)AS_sockaddr, sizeof(struct sockaddr_in)) < 0)
      {
        printf("[ERROR]: connect AS failed\n");
        return 1;
      }

      // 发送第一次明文请求
      sprintf(request, "authentication %s", clientUser);
      write(clientSock, request, strlen(request));
      printf("[SEND]: %s\n", request);

      // 收到响应
      resLen = read(clientSock, response, 1024);
      response[resLen] = 0;

      // 判断响应是否有效
      if (isInvalid(response))
      {
        close(clientSock);
        free(AS_sockaddr);
        return 1;
      }

      // 消息 A
      printf("[RECV A]: %s\n", response);
      strcpy(messageA, response);

      // 消息 A 的解密
      messageA_encoded_Len = intChar2char(messageA, resLen, messageA_encoded);
      decodeFull(messageA_encoded, messageA_encoded_Len, messageA_decoded, keyClient);
      printf("[MSG A]: %s\n", messageA_decoded);
      strcpy(keyCTGS, messageA_decoded);

      // 收到消息 B
      resLen = read(clientSock, response, 1024);
      response[resLen] = 0;

      // 判断响应是否有效
      if (isInvalid(response))
      {
        close(clientSock);
        free(AS_sockaddr);
        return 1;
      }

      // 储存消息 B
      strcpy(messageB, response);
      printf("[RECV B]: %s\n", messageB);

      close(clientSock);
      free(AS_sockaddr);

      printf("--------AS END--------\n\n");
    }

    // TGS
    {
      printf("-------TGS START-------\n");

      // 新建套接字
      if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
        printf("[ERROR]: client socket failed\n");
        return 1;
      }

      // 尝试连接
      struct sockaddr_in *TGS_sockaddr = newSockaddr_in(TGS_HOST, TGS_PORT);
      if (connect(clientSock, (struct sockaddr *)TGS_sockaddr, sizeof(struct sockaddr)))
      {
        printf("[ERROR]: connect TGS failed\n");
        return 1;
      }

      // 发送消息 C
      sprintf(messageC, "%s,%s", serviceID, messageB);
      printf("[SEND C]: %s\n", messageC);
      write(clientSock, messageC, strlen(messageC));

      // 接受 E
      resLen = read(clientSock, response, 1024);
      response[resLen] = 0;

      // 判断响应是否有效
      if (isInvalid(response))
      {
        close(clientSock);
        free(TGS_sockaddr);
        return 1;
      }

      // 储存 E
      strcpy(messageE, response);
      printf("[RECV E]: %s\n", messageE);

      // 生成消息 D
      now = time(NULL);
      sprintf(messageD, "<%s,%ld>", clientUser, now);
      printf("[MSG D ]: %s\n", messageD);

      // 加密消息 D
      messageD_encoded_len = encodeFull(messageD, strlen(messageD), messageD_encoded, messageA_decoded);
      char2intChar(messageD_encoded, messageD_encoded_len, messageD_encoded_transfered);

      // 发送 D
      write(clientSock, messageD_encoded_transfered, strlen(messageD_encoded_transfered));
      printf("[SEND D ]: %s\n", messageD_encoded_transfered);

      // 收到响应（消息 F）
      resLen = read(clientSock, response, 1024);
      response[resLen] = 0;

      // 判断响应是否无效
      if (isInvalid(response))
      {
        close(clientSock);
        free(TGS_sockaddr);
        return 1;
      }

      // 解码消息 F
      strcpy(messageF, response);
      printf("[RECV F]: %s\n", messageF);
      messageF_encoded_len = intChar2char(messageF, strlen(messageF), messageF_encoded);
      decodeFull(messageF_encoded, messageF_encoded_len, messageF_decoded, keyCTGS);
      printf("[MSG F]: %s\n", messageF_decoded);
      strcpy(keyCSS, messageF_decoded);

      printf("--------TGS END--------\n\n");
    }

    // SS
    {
      printf("-------TGS START-------\n");

      // 新建套接字
      if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
        printf("[ERROR]: client socket failed\n");
        return 1;
      }

      // 尝试连接
      struct sockaddr_in *SS_sockaddr = newSockaddr_in(SS_HOST, SS_PORT);
      if (connect(clientSock, (struct sockaddr *)SS_sockaddr, sizeof(struct sockaddr)))
      {
        printf("[ERROR]: connect SS failed\n");
        return 1;
      }

      // 发送消息 E
      messageE[strlen(messageE)] = 0;
      write(clientSock, messageE, strlen(messageE) + 1);
      printf("[SEND E]: %s\n", messageE);

      // 避免粘包，进行一次无效接受
      read(clientSock, response, 1024);

      // 生成消息 G
      now = time(NULL);
      sprintf(messageG, "<%s,%ld>", clientUser, now);
      printf("[MSG G]: %s\n", messageG);

      // 加密消息 G
      messageG_encoded_len = encodeFull(messageG, strlen(messageG), messageG_encoded, keyCSS);
      char2intChar(messageG_encoded, messageG_encoded_len, messageG_encoded_transfered);

      // 发送消息 G
      write(clientSock, messageG_encoded_transfered, strlen(messageG_encoded_transfered));
      printf("[SEND G]: %s\n", messageG_encoded_transfered);

      // 收到响应（消息 H）
      resLen = read(clientSock, response, 1024);
      response[resLen] = 0;

      // 判断响应是否有效
      if (resLen <= 0 || isInvalid(response))
      {
        close(clientSock);
        free(SS_sockaddr);
        return 1;
      }

      strcpy(messageH, response);
      printf("[RECV H]: %s\n", messageH);

      // 解码消息 H
      messageH_encoded_len = intChar2char(messageH, strlen(messageH), messageH_encoded);
      decodeFull(messageH_encoded, messageH_encoded_len, messageH_decoded, keyCSS);
      printf("[MSG H]: %s\n", messageH_decoded);

      // 判断是否认证成功
      time_t timestamp;
      char clientID_in_H[1024];
      sscanf(messageH_decoded, "<%[^,],%ld>", clientID_in_H, &timestamp);

      if (timestamp == now + 1l && strcmp(clientID_in_H, clientUser) == 0)
      {
        printf("[FIN]: Authentication success\n");
      }
      else
      {
        printf("[ERROR]: Authentication failed\n");
      }

      printf("--------SS END--------\n\n");
    }

    printf("[END]: clear AS socket\n");

    printf("clear all\n");
    return 0;
  }
}

int isInvalid(char *response)
{
  char error[1024];
  if (sscanf(response, "Invalid %s", error) != 0)
  {
    printf("[ERROR]: %s\n", response);
    return 1;
  }
  return 0;
}