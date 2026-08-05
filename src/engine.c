/**
 * @file engine.c
 * @brief 存储引擎守护进程（APUE标准双fork，Unix域套接字通信）
 */

#include "../inc/data_struct.h"
#include "../inc/protocol.h"
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DevManager* g_mgr = NULL;
static int g_listen_fd = -1;
static char g_data_path[PATH_MAX + 128] = "./data/dev.csv";
static volatile int g_running = 1;

/* 信号处理：优雅退出 */
static void SigExitHandler(int sig)
{
    (void)sig;
    g_running = 0;
}

/* 统一注册退出信号（关闭SA_RESTART） */
static void RegisterExitSignal(int sig_no)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SigExitHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig_no, &sa, NULL);
}

/* ============================================================
 * 标准双fork守护进程化
 * ============================================================ */
void Daemonize()
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("first fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("second fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    RegisterExitSignal(SIGTERM);

    if (chdir("/") < 0) {
        perror("chdir /");
        exit(EXIT_FAILURE);
    }

    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int null_fd = open("/dev/null", O_RDWR);
    if (null_fd < 0) exit(EXIT_FAILURE);
    dup2(null_fd, STDIN_FILENO);
    dup2(null_fd, STDOUT_FILENO);
    dup2(null_fd, STDERR_FILENO);
    close(null_fd);
}

/* ============================================================
 * 处理单条客户端请求
 * ============================================================ */
static void HandleClient(int client_fd)
{
    int cmd_len;
    char cmd[MAX_CMD_LEN] = {0};
    int ret_code = 0;
    char content[MAX_CONTENT_LEN] = {0};

    /* 1. 读取命令长度 */
    if (RecvAll(client_fd, &cmd_len, sizeof(cmd_len)) <= 0) {
        close(client_fd);
        return;
    }

    /* 2. 读取命令内容 */
    if (cmd_len <= 0 || cmd_len >= MAX_CMD_LEN) {
        close(client_fd);
        return;
    }
    if (RecvAll(client_fd, cmd, cmd_len) <= 0) {
        close(client_fd);
        return;
    }
    cmd[cmd_len] = '\0';

    /* 3. 解析并执行命令 */
    char cmd_type[32];
    char args[MAX_CMD_LEN];
    sscanf(cmd, "%s %[^\n]", cmd_type, args);

    if (strcmp(cmd_type, "ADD") == 0) {
        char dev_id[ID_MAX_LEN], parent_id[ID_MAX_LEN], name[NAME_MAX_LEN];
        int type; float temp;
        sscanf(args, "%s %s %d %s %f", dev_id, parent_id, &type, name, &temp);
        ret_code = AddNewDevice(g_mgr, dev_id, parent_id, type, name, temp);
        snprintf(content, sizeof(content),
                 ret_code == RET_OK ? "添加成功" : "添加失败，错误码 %d", ret_code);
    }
    else if (strcmp(cmd_type, "DELETE") == 0) {
        char del_id[ID_MAX_LEN];
        sscanf(args, "%s", del_id);
        ret_code = DeleteDeviceById(g_mgr, del_id);
        snprintf(content, sizeof(content),
                 ret_code == RET_OK ? "删除成功" : "删除失败，未找到设备");
    }
    else if (strcmp(cmd_type, "PRINT_TREE") == 0) {
        char* buffer = NULL;
        size_t size = 0;
        FILE* mem = open_memstream(&buffer, &size);
        if (!mem) {
            ret_code = -1;
            snprintf(content, sizeof(content), "内存流错误");
        } else {
            PrintAllTreeToStream(g_mgr, mem);
            fclose(mem);
            if (buffer) {
                size_t copy_len = size < sizeof(content)-1 ? size : sizeof(content)-1;
                memcpy(content, buffer, copy_len);
                content[copy_len] = '\0';
                free(buffer);
            }
            ret_code = RET_OK;
        }
    }
    else if (strcmp(cmd_type, "PRINT_ALARM") == 0) {
        char* buffer = NULL;
        size_t size = 0;
        FILE* mem = open_memstream(&buffer, &size);
        if (!mem) {
            ret_code = -1;
            snprintf(content, sizeof(content), "内存流错误");
        } else {
            PrintAllAlarmToStream(g_mgr, mem);
            fclose(mem);
            if (buffer) {
                size_t copy_len = size < sizeof(content)-1 ? size : sizeof(content)-1;
                memcpy(content, buffer, copy_len);
                content[copy_len] = '\0';
                free(buffer);
            }
            ret_code = RET_OK;
        }
    }
    else if (strcmp(cmd_type, "SAVE") == 0) {
        ret_code = SaveDataToCsv(g_mgr, g_data_path);
        snprintf(content, sizeof(content),
                 ret_code == RET_OK ? "保存成功" : "保存失败");
    }
    else if (strcmp(cmd_type, "EXIT") == 0) {
        snprintf(content, sizeof(content), "引擎正在关闭...");
        /* 先发送响应再退出 */
        int content_len = strlen(content);
        SendAll(client_fd, &ret_code, sizeof(ret_code));
        SendAll(client_fd, &content_len, sizeof(content_len));
        SendAll(client_fd, content, content_len);
        close(client_fd);
        g_running = 0;
        return;
    }
    else {
        ret_code = -1;
        snprintf(content, sizeof(content), "未知命令: %s", cmd_type);
    }

    /* 4. 发送响应：错误码 + 内容长度 + 内容 */
    int content_len = strlen(content);
    SendAll(client_fd, &ret_code, sizeof(ret_code));
    SendAll(client_fd, &content_len, sizeof(content_len));
    SendAll(client_fd, content, content_len);

    close(client_fd);
}

/* ============================================================
 * 资源清理
 * ============================================================ */
static void Cleanup()
{
    if (g_mgr) {
        SaveDataToCsv(g_mgr, g_data_path);
        DestroyDevManager(g_mgr);
        g_mgr = NULL;
    }
    if (g_listen_fd != -1) {
        close(g_listen_fd);
        unlink(SOCK_PATH);
        g_listen_fd = -1;
    }
}

/* ============================================================
 * 主入口
 * ============================================================ */
int main(int argc, char* argv[])
{
    int foreground = (argc >= 2 && strcmp(argv[1], "--foreground") == 0);

    /* 提前获取数据文件绝对路径 */
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        strncpy(g_data_path, cwd, sizeof(g_data_path) - 1);
        g_data_path[sizeof(g_data_path) - 1] = '\0';
        strncat(g_data_path, "/data/dev.csv", sizeof(g_data_path) - strlen(g_data_path) - 1);
    }
    else
    {
        strncpy(g_data_path, "./data/dev.csv", sizeof(g_data_path) - 1);
        g_data_path[sizeof(g_data_path) - 1] = '\0';
    }

    if (!foreground) {
        Daemonize();
    } else {
        RegisterExitSignal(SIGTERM);
        RegisterExitSignal(SIGINT);
        signal(SIGCHLD, SIG_IGN);
    }

    /* 初始化设备管理器 */
    g_mgr = CreateDevManager();
    if (!g_mgr) {
        fprintf(stderr, "CreateDevManager 失败\n");
        return -1;
    }

    LoadCsvData(g_mgr, g_data_path);

    /* 创建监听套接字 */
    g_listen_fd = CreateServerSocket(SOCK_PATH);
    if (g_listen_fd == -1) {
        fprintf(stderr, "创建服务端套接字失败\n");
        DestroyDevManager(g_mgr);
        return -1;
    }

    if (foreground) {
        printf("存储引擎已启动（前台模式）\n");
        printf("套接字路径: %s\n", SOCK_PATH);
        printf("数据文件路径: %s\n", g_data_path);
        printf("按 Ctrl+C 优雅退出\n");
    }

    /* 主循环：接受客户端连接 */
    while (g_running) {
        int client_fd = accept(g_listen_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                if (!g_running) break;
                continue;
            }
            perror("accept");
            break;
        }
        HandleClient(client_fd);
    }

    Cleanup();
    if (foreground) printf("\n引擎已正常退出\n");
    return 0;
}