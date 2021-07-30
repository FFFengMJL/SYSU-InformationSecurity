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

#define TGS_HOST "127.0.0.1"
#define TGS_PORT 23335

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
 * 检查服务 ID 是否存在
 * @param serviceID char* 服务 ID 字符串
 * @return 0 存在 1 不存在
*/
int checkServiceIDList(char *serviceID);

int main()
{
  // 申请套接字
  int TGS_socket;
  if ((TGS_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    // 报错并退出
    perror("TGS socket");
    return 1;
  }
  else
  {
    // 生对应地址结构体
    struct sockaddr_in *TGS_sockaddr = newSockaddr_in(TGS_HOST, TGS_PORT);

    // 进行地址和端口绑定
    if (bind(TGS_socket, (struct sockaddr *)TGS_sockaddr, sizeof(struct sockaddr_in)) < 0)
    {
      perror("bind");
      return 1;
    }
    printf("TGS bind ip: %s:%d\n", TGS_HOST, TGS_PORT);

    // 开始监听
    if (listen(TGS_socket, 20) < 0)
    {
      perror("listen");
      return 1;
    }
    printf("TGS start to listen\n");

    // 阻塞式接受
    while (1)
    {
      struct sockaddr_in clientAddr; // 客户端
      socklen_t clientAddrSize = sizeof(clientAddr);
      int clientSock;

      // 如果接收到了
      if ((clientSock = accept(TGS_socket, (struct sockaddr *)&clientAddr, &clientAddrSize)) >= 0)
      {
        // 新建子进程用于处理请求
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
    close(TGS_socket);
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

  // 获取 key_TGS
  FILE *keyTGSFile = fopen("./keyTGS", "r");
  char keyTGS[17];
  fread(keyTGS, 1, 17, keyTGSFile);

  // 显示请求方的相关信息
  printf("from %s: %d\n", inet_ntoa(clientAddr->sin_addr), clientAddr->sin_port);

  // 循环等待请求
  int reqLen = read(clientSock, request, 1024);
  while (reqLen <= 0)
  {
    reqLen = read(clientSock, request, 1024);
  }
  request[reqLen] = 0;

  char clientID_in_B[1024], serviceID[1024], clientAddr_in_B[1024], messageB[1024];
  time_t validity;
  int requestArgsNum = sscanf(request, "%[^,],%s", serviceID, messageB);

  // 判断请求是否有效
  if (requestArgsNum == 0 || strlen(messageB) <= 0)
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }
  printf("[RECV C]: %s\n", request); // 显示收到的信息

  // 解码消息 B
  char messageB_encoded[1024]; // 加密后的消息 B
  char messageB_decoded[1024]; // 解密后的消息 B
  int messageB_encoded_len = intChar2char(messageB, strlen(messageB), messageB_encoded);
  int messageB_decoded_len = decodeFull(messageB_encoded, messageB_encoded_len, messageB_decoded, keyTGS);
  messageB[messageB_decoded_len] = 0;
  printf("[MSG B]: %s\n", messageB_decoded);

  // 判断消息 B 是否有效
  char keyCTGS[17]; // key_client_TGS
  sscanf(messageB_decoded, "<%[^,],%[^,],%ld, %[^>]>", clientID_in_B, clientAddr_in_B, &validity, keyCTGS);
  char *clientAddrStr = inet_ntoa(clientAddr->sin_addr);
  if (checkClientIDList(clientID_in_B) || checkServiceIDList(serviceID)) // ID 无效
  {
    printf("[SEND]: %s\n", INVALIDID);
    write(clientSock, INVALIDID, strlen(INVALIDID));
    return;
  }
  else if (strcmp(clientAddrStr, clientAddr_in_B) != 0) // 地址无效
  {
    printf("[SEND]: %s\n", INVALIDADDRESS);
    write(clientSock, INVALIDADDRESS, strlen(INVALIDADDRESS));
    return;
  }

  // 获取密钥 keySS
  FILE *keySSFile = fopen("./keySS", "r");
  char keySS[17];
  fread(keySS, 1, 16, keySSFile);

  // 生成 key_client_SS
  char keyCSS[17];
  getKeyCSS(keyCSS);

  // 生成 ST
  char ST[1024], ST_encoded[1024], ST_encoded_transfered[1024]; // ST 相关
  int ST_encoded_len = 0;
  sprintf(ST, "<%s,%s,%ld,%s>", clientID_in_B, clientAddr_in_B, time(NULL) + 900l, keyCSS);
  printf("[MSG ST]: %s\n", ST);

  // 加密 ST
  ST_encoded_len = encodeFull(ST, strlen(ST), ST_encoded, keySS);
  char2intChar(ST_encoded, ST_encoded_len, ST_encoded_transfered);

  // 生成消息 E
  char messageE[1024];
  sprintf(messageE, "%s,%s", serviceID, ST_encoded_transfered);

  // 发送消息 E
  write(clientSock, messageE, strlen(messageE));
  printf("[SEND E]: %s\n", messageE);

  // 接收请求（消息 D）
  reqLen = read(clientSock, request, 1024);
  request[reqLen] = 0;
  // 判断请求有效性
  if (reqLen <= 0) // 无效请求
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }
  char messageD[1024]; // 消息 D
  strcpy(messageD, request);
  printf("[RECV D]: %s\n", messageD);

  // 解密消息 D
  char messageD_encoded[1024];                                                                              // 加密后的消息 D
  char messageD_decoded[1024];                                                                              // 解密后的消息 D
  int messageD_encoded_len = intChar2char(messageD, strlen(messageD), messageD_encoded);                    // 解密后的消息 D 的长度
  int messageD_decoded_len = decodeFull(messageD_encoded, messageD_encoded_len, messageD_decoded, keyCTGS); // 解密后的消息 D 的长度
  messageD_decoded[messageD_decoded_len] = 0;
  printf("[MSG D]: %s\n", messageD_decoded);

  // 判断消息 D 的有效性
  time_t now = time(NULL), timestamp;
  char clientID_in_D[1024];
  sscanf(messageD_decoded, "<%[^,],%ld>", clientID_in_D, &timestamp);
  if (timestamp < now || timestamp > now + 900l) // 无效时间戳
  {
    printf("[SEND]: %s\n", INVALIDTIME);
    write(clientSock, INVALIDTIME, strlen(INVALIDTIME));
    return;
  }
  else if (strcmp(clientID_in_D, clientID_in_B) != 0) // ID 不一致
  {
    printf("[SEND]: %s\n", INVALIDID);
    write(clientSock, INVALIDID, strlen(INVALIDID));
    return;
  }

  // 生成消息 F
  char messageF[1024]; // 消息 F
  strcpy(messageF, keyCSS);
  printf("[MSG F]: %s\n", messageF);

  // 加密消息 F
  char messageF_encoded[1024];                                                                  // 加密后的消息 F
  char messageF_encoded_transfered[1024];                                                       // 加密后的转换显示的消息 F
  int messageF_encoded_len = encodeFull(messageF, strlen(messageF), messageF_encoded, keyCTGS); // 加密后的消息 F 的长度
  char2intChar(messageF_encoded, messageF_encoded_len, messageF_encoded_transfered);

  // 发送消息 F
  write(clientSock, messageF_encoded_transfered, strlen(messageF_encoded_transfered));
  printf("[SEND F ]: %s\n", messageF_encoded_transfered);

  close(clientSock);
  fclose(keySSFile);
  fclose(keyTGSFile);
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

void getKeyCSS(char res[17])
{
  unsigned char tmp;
  srand(time(NULL));
  for (int i = 0; i < 16; i++)
  {
    tmp = rand() % 16;
    if (tmp >= 10)
    {
      res[i] = tmp - 10 + 'a';
    }
    else
    {
      res[i] = tmp + '0';
    }
  }
  res[16] = 0;
}