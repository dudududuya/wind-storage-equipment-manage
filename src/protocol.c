#include "../inc/protocol.h"

/**
 * 创建服务端监听套接字
 */
int CreateServerSocket(const char* path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket create");
        return -1;
    }

    /* 清理残留的套接字文件 */
    unlink(path);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("socket bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 10) < 0) {
        perror("socket listen");
        close(fd);
        return -1;
    }

    return fd;
}

/**
 * 客户端连接服务端
 */
int ConnectServerSocket(const char* path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket create");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

/**
 * 保证发送全部数据（解决流式套接字半包问题）
 */
int SendAll(int fd, const void* buf, int len)
{
    int total = 0;
    const char* ptr = (const char*)buf;

    while (total < len) {
        int n = send(fd, ptr + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("send");
            return -1;
        }
        total += n;
    }
    return total;
}

/**
 * 保证接收全部数据
 */
int RecvAll(int fd, void* buf, int len)
{
    int total = 0;
    char* ptr = (char*)buf;

    while (total < len) {
        int n = recv(fd, ptr + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recv");
            return -1;
        }
        if (n == 0) return total;
        total += n;
    }
    return total;
}