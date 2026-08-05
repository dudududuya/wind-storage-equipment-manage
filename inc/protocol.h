#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Unix 域套接字路径 */
#define SOCK_PATH       "/tmp/wind_engine.sock"
#define MAX_CMD_LEN     1024    // 命令最大长度
#define MAX_CONTENT_LEN 131072  // 响应内容最大128KB

/* 通信工具函数 */
int CreateServerSocket(const char* path);
int ConnectServerSocket(const char* path);
int SendAll(int fd, const void* buf, int len);
int RecvAll(int fd, void* buf, int len);

#endif // PROTOCOL_H