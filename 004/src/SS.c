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

#include "des.c"

#define SS_HOST "127.0.0.1"
#define SS_PORT 23334

#define INVALIDID "Invalid (client or service)ID"
#define INVALIDREQUEST "Invalid request"
#define INVALIDADDRESS "Invalid address"
#define INVALIDTIME "Invalid time"

const char username[] = "Bob";
const char clientIDList[][200] = {
    "Bob",
    "Alice"}; // 客户端 ID 的列表

const char serviceIDList[][200] = {
    "testService",
    "service"};
const char clientIDListLen = 2;
const char serviceIDListLen = 2;

/**
 * 新建 socket
 * @param addr char* 目标 ip
 * @param port int 目标 ip 的端口
 * @return sockaddr_in 结构体
*/
struct sockaddr_in *newSockaddr_in(char *addr, int port);

/**
 * 对接收到到的东西进行处理
 * @param clientAddr struct sockaddr_in* 客户端地址
 * @param clientSock int 客户端套接字
*/
void serve(struct sockaddr_in *clientAddr, int clientSock);

/**
 * 检查是否含有对应的客户端 ID
 * @param clientID char* 客户端 ID 的字符串
 * @return 0 存在 1 不存在
*/
int checkClientIDList(char *clientID);

/**
 * 生成 key_client_SS
*/
void getKeyCSS(char res[17]);

/**
 * 检查是否含有对应的服务 ID
 * @param serviceID char* 客户端 ID 的字符串
 * @return 0 存在 1 不存在
*/
int checkServiceIDList(char *serviceID);

int main()
{
  int ss_socket;
  if ((ss_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("ss socket");
    return 1;
  }
  else
  {
    // ss
    struct sockaddr_in *SS_sockaddr = newSockaddr_in(SS_HOST, SS_PORT);

    // 绑定
    if (bind(ss_socket, (struct sockaddr *)SS_sockaddr, sizeof(struct sockaddr_in)) < 0)
    {
      perror("bind");
      return 1;
    }
    printf("ss bind ip: %s:%d\n", SS_HOST, SS_PORT);

    // 开始监听
    if (listen(ss_socket, 20) < 0)
    {
      perror("listen");
      return 1;
    }
    printf("ss start to listen\n");

    // 接收
    while (1)
    {
      struct sockaddr_in clientAddr; // 客户端
      socklen_t clientAddrSize = sizeof(clientAddr);
      int clientSock;

      // 如果接收到了
      if ((clientSock = accept(ss_socket, (struct sockaddr *)&clientAddr, &clientAddrSize)) >= 0)
      {
        int pid = fork();
        if (pid == 0)
        {
          printf("-------START-------\n");
          serve(&clientAddr, clientSock);
          printf("--------END--------\n\n");
          return 0;
        }
      }
    }
    close(ss_socket);
  }
  return 0;
}

struct sockaddr_in *newSockaddr_in(char *addr, int port)
{
  struct sockaddr_in *sockaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  sockaddr->sin_addr.s_addr = inet_addr(addr);
  sockaddr->sin_family = AF_INET;
  sockaddr->sin_port = htons(port);
  return sockaddr;
}

void serve(struct sockaddr_in *clientAddr, int clientSock)
{
  char request[1024]; // 请求

  // 显示请求方的相关信息
  printf("from %s: %d\n", inet_ntoa(clientAddr->sin_addr), clientAddr->sin_port);

  // 循环等待请求
  int reqLen = read(clientSock, request, 1024);
  while (reqLen <= 0)
  {
    reqLen = read(clientSock, request, 1024);
  }
  request[reqLen] = 0;

  // 判断请求有效性
  char serviceID[1024]; // 服务 ID
  char ST[1024];        // ST 凭据
  if (sscanf(request, "%[^,],%s", serviceID, ST) <= 0)
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }
  else if (checkServiceIDList(serviceID))
  {
    printf("[SEND]: %s\n", INVALIDID);
    write(clientSock, INVALIDID, strlen(INVALIDID));
    return;
  }

  // 得到消息 E
  char messageE[1024];
  strcpy(messageE, request);
  printf("[RECV E]: %s\n", request); // 显示收到的信息

  // 获取 key_SS
  FILE *keySSFile = fopen("./keySS", "r");
  char keySS[17];
  fread(keySS, 1, 16, keySSFile);

  // 解密 ST
  char ST_encoded[1024];                                                          // 加密后的 ST
  char ST_decoded[1024];                                                          // 解密收的 ST
  int ST_encoded_len = intChar2char(ST, strlen(ST), ST_encoded);                  // 加密后的 ST 长度
  int ST_decoded_len = decodeFull(ST_encoded, ST_encoded_len, ST_decoded, keySS); // 解密后的 ST 长度
  printf("[MSG ST]: %s\n", ST_decoded);

  // 判断 ST 有效性
  char clientID_in_E[1024];
  char clientAddr_in_E[1024];
  char keyCSS[17];
  time_t timestamp;
  int requestArgsNum = sscanf(ST_decoded, "<%[^,],%[^,],%ld,%[^>]>", clientID_in_E, clientAddr_in_E, &timestamp, keyCSS); // 读取到的参数个数
  char *clienAddrStr = inet_ntoa(clientAddr->sin_addr);                                                                   // ST 中的地址
  time_t now = time(NULL);                                                                                                // 当前时间

  if (requestArgsNum <= 0) // 参数不够
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }
  else if (strcmp(clienAddrStr, clientAddr_in_E) != 0) // 地址不对
  {
    printf("[SEND]: %s\n", INVALIDADDRESS);
    write(clientSock, INVALIDADDRESS, strlen(INVALIDADDRESS));
    return;
  }
  else if (timestamp < now) // 时间戳不对
  {
    printf("[NOW]: %ld\n", now);
    printf("[SEND]: %s\n", INVALIDTIME);
    write(clientSock, INVALIDTIME, strlen(INVALIDTIME));
    return;
  }

  // 避免粘包，进行一次无效发送
  write(clientSock, "RECV G", 1024);

  // 接收请求（消息 G）
  reqLen = read(clientSock, request, 1024);
  request[reqLen];

  // 判断请求有效性
  if (reqLen <= 0)
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }

  // 得到消息 G
  char messageG[1024];
  strcpy(messageG, request);
  printf("[RECV G ]: %s\n", messageG);

  // 解密消息 G
  char messageG_encoded[1024]; // 加密后的消息 G
  char messageG_decoded[1024]; // 解密后的消息 G
  int messageG_encoded_len = intChar2char(messageG,
                                          strlen(messageG),
                                          messageG_encoded); // 加密后的消息 G 长度
  int messageG_decoded_len = decodeFull(messageG_encoded,
                                        messageG_encoded_len,
                                        messageG_decoded,
                                        keyCSS); // 解密后的消息 G 长度
  printf("[MSG G]: %s\n", messageG_decoded);

  // 验证消息 G 的有效性
  char clientID_in_G[1024];
  requestArgsNum = sscanf(messageG_decoded, "<%[^,],%ld>", clientID_in_G, &timestamp);
  if (requestArgsNum <= 0) // 参数不够
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }
  else if (strcmp(clientID_in_E, clientID_in_G) != 0) // 客户端 ID 不对
  {
    printf("[SEND]: %s\n", INVALIDID);
    write(clientSock, INVALIDID, strlen(INVALIDID));
    return;
  }

  // 生成消息 H
  time_t TS = timestamp + 1l;
  char messageH[1024];
  sprintf(messageH, "<%s,%ld>", clientID_in_E, TS);
  printf("[MSG H]: %s\n", messageH);

  // 加密消息 H
  char messageH_encoded[1024];            // 加密后的消息 H
  char messageH_encoded_transfered[1024]; // 加密后进行转换的消息 H
  int messageH_encoded_len = encodeFull(messageH,
                                        strlen(messageH),
                                        messageH_encoded,
                                        keyCSS); // 加密后消息 H 的长度
  char2intChar(messageH_encoded,
               messageH_encoded_len,
               messageH_encoded_transfered);

  // 发送消息 H
  write(clientSock, messageH_encoded_transfered, strlen(messageH_encoded_transfered));
  printf("[SEND H]: %s\n", messageH_encoded_transfered);

  free(keySSFile);
  close(clientSock);
  return;
}

int checkClientIDList(char *clientID)
{
  for (int i = 0; i < clientIDListLen; i++)
  {
    if (!strcmp(clientID, clientIDList[i]))
    {
      return 0;
    }
  }
  return 1;
}

int checkServiceIDList(char *serviceID)
{
  for (int i = 0; i < serviceIDListLen; i++)
  {
    if (!strcmp(serviceID, serviceIDList[i]))
    {
      return 0;
    }
  }
  return 1;
}
