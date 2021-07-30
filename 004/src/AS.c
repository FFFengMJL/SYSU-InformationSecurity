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

#define AS_HOST "127.0.0.1"
#define AS_PORT 23333

#define INVALIDID "Invalid ID"
#define INVALIDREQUEST "Invalid request"

const char username[] = "Bob";
const char clientIDList[][200] = {
    "Bob",
    "Alice"}; // 客户端 ID 的列表
const char clientIDListLen = 2;

/**
 * 新建 socket
 * @param addr char* 目标 ip
 * @param port int 目标 ip 的端口
 * @return sockaddr_in 结构体
*/
struct sockaddr_in *newSockaddr_in(char *addr, int port);

/**
 * 对接收到到的请求进行处理
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
 * 随机生成 key_client_TGS
*/
void getKeyCTGS(char res[17]);

int main()
{
  // 申请套接字
  int AS_socket;
  if ((AS_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    // 报错并退出
    perror("AS socket");
    return 1;
  }
  else
  {
    // 生对应地址结构体
    struct sockaddr_in *AS_sockaddr = newSockaddr_in(AS_HOST, AS_PORT);

    // 进行地址和端口绑定
    if (bind(AS_socket, (struct sockaddr *)AS_sockaddr, sizeof(struct sockaddr_in)) < 0)
    {
      perror("bind");
      return 1;
    }
    printf("AS bind ip: %s:%d\n", AS_HOST, AS_PORT);

    // 开始监听
    if (listen(AS_socket, 20) < 0)
    {
      perror("listen");
      return 1;
    }
    printf("AS start to listen\n");

    // 阻塞式接受
    while (1)
    {
      struct sockaddr_in clientAddr; // 客户端
      socklen_t clientAddrSize = sizeof(clientAddr);
      int clientSock;

      // 如果接收到了
      if ((clientSock = accept(AS_socket, (struct sockaddr *)&clientAddr, &clientAddrSize)) >= 0)
      {
        // 新建进程用于处理请求
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
    close(AS_socket);
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
  int reqLen = read(clientSock, request, 1024);

  // 循环等待请求
  while (reqLen <= 0)
  {
    reqLen = read(clientSock, request, 1024);
  }
  request[reqLen] = 0;
  printf("[RECV]: %s\n", request);

  // 判断请求是否有效
  char clientID[1024];
  int reqArgsNum = sscanf(request, "authentication %s", clientID);

  // 无效 ID
  if (reqArgsNum <= 0)
  {
    printf("[SEND]: %s\n", INVALIDREQUEST);
    write(clientSock, INVALIDREQUEST, strlen(INVALIDREQUEST));
    return;
  }
  // ID 不存在
  else if (checkClientIDList(clientID))
  {
    printf("[SEND]: %s\n", INVALIDID);
    write(clientSock, INVALIDID, strlen(INVALIDID));
    return;
  }

  // 如果 ID 有效，需要通过 keyClient 加密暂时生成的会话密钥 key_client_TGS

  // 获取 keyClient
  FILE *keyClientFile = fopen("./keyClient", "r");
  char keyClient[17];
  fread(keyClient, 1, 16, keyClientFile);
  keyClient[16] = 0;

  // 生成 key_client_TGS
  char key_CTGS[17]; // key_CTGS 需要是16位16进制数
  getKeyCTGS(key_CTGS);

  // 生成消息 A
  char messageA[1024];

  // 加密消息 A
  encodeFull(key_CTGS, 16, messageA, keyClient);
  char messageA_transfered[1024];
  char2intChar(messageA, 24, messageA_transfered);

  // 发送消息 A
  write(clientSock, messageA_transfered, strlen(messageA_transfered));
  printf("[SEND A]: %s\n", messageA_transfered);

  // 获取 keyTGS
  FILE *keyTGSFile = fopen("./keyTGS", "r");
  char keyTGS[17];
  fread(keyTGS, 1, 16, keyTGSFile);
  char messageB[1024], messageB_encoded[1024];

  // 生成消息 B
  sprintf(messageB, "<%s,%s,%ld,%s>", clientID, inet_ntoa(clientAddr->sin_addr), time(NULL) + 900l, key_CTGS);
  printf("[MSG B]: %s\n", messageB);

  // 加密消息 B
  int messageB_encoded_len = encodeFull(messageB, strlen(messageB), messageB_encoded, keyTGS);
  messageB_encoded[messageB_encoded_len] = 0;
  char messageB_encoded_transfered[1024];
  char2intChar(messageB_encoded, messageB_encoded_len, messageB_encoded_transfered);

  // 发送消息 B
  write(clientSock, messageB_encoded_transfered, strlen(messageB_encoded_transfered));
  printf("[SEND B ]: %s\n", messageB_encoded_transfered);

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

void getKeyCTGS(char res[17])
{
  unsigned char tmp;
  srand(time(NULL) + 1);
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