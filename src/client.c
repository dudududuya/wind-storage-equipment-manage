#include "../inc/protocol.h"
#include "../inc/data_struct.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    printf("===== 风电储能设备管理系统 (客户端) =====\n");
    printf("请确保存储引擎已启动 (./engine --foreground 或 ./engine)\n\n");

    int _scan_ret;

    while (1) {
        printf("\n1. 新增设备\n");
        printf("2. 删除设备\n");
        printf("3. 打印设备树\n");
        printf("4. 查看告警电芯\n");
        printf("5. 保存数据\n");
        printf("0. 退出客户端\n");
        printf("请选择: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != EOF && ch != '\n');
            printf("输入无效，请输入数字\n");
            continue;
        }
        int ch;
        while ((ch = getchar()) != EOF && ch != '\n');

        if (choice == 0) {
            printf("客户端已退出\n");
            break;
        }

        char cmd[MAX_CMD_LEN];
        switch (choice) {
            case 1: {
                char dev_id[ID_MAX_LEN], parent_id[ID_MAX_LEN], name[NAME_MAX_LEN];
                int type; float temp = 0;

                printf("设备ID: ");
                _scan_ret = scanf("%s", dev_id);
                (void)_scan_ret;

                printf("父设备ID(根节点输root): ");
                _scan_ret = scanf("%s", parent_id);
                (void)_scan_ret;

                printf("类型(0总站1箱变2PCS3BMS4仓5簇6模组7电芯): ");
                _scan_ret = scanf("%d", &type);
                (void)_scan_ret;

                printf("名称: ");
                _scan_ret = scanf("%s", name);
                (void)_scan_ret;

                printf("温度: ");
                _scan_ret = scanf("%f", &temp);
                (void)_scan_ret;
                
                snprintf(cmd, sizeof(cmd), "ADD %s %s %d %s %.2f", dev_id, parent_id, type, name, temp);
                break;
            }
            case 2: {
                char del_id[ID_MAX_LEN];
                printf("删除设备ID: ");
                _scan_ret = scanf("%s", del_id);
                (void)_scan_ret;
                snprintf(cmd, sizeof(cmd), "DELETE %s", del_id);
                break;
            }
            case 3: snprintf(cmd, sizeof(cmd), "PRINT_TREE"); break;
            case 4: snprintf(cmd, sizeof(cmd), "PRINT_ALARM"); break;
            case 5: snprintf(cmd, sizeof(cmd), "SAVE"); break;
            default: printf("无效选项\n"); continue;
        }

        /* 连接引擎 */
        int sock_fd = ConnectServerSocket(SOCK_PATH);
        if (sock_fd == -1) {
            printf("无法连接引擎，请确认引擎已启动\n");
            continue;
        }

        /* 发送命令：长度 + 内容 */
        int cmd_len = strlen(cmd);
        if (SendAll(sock_fd, &cmd_len, sizeof(cmd_len)) < 0 ||
            SendAll(sock_fd, cmd, cmd_len) < 0) {
            printf("发送请求失败\n");
            close(sock_fd);
            continue;
        }

        /* 接收响应：错误码 + 内容长度 + 内容 */
        int ret_code;
        int content_len;
        char content[MAX_CONTENT_LEN] = {0};

        if (RecvAll(sock_fd, &ret_code, sizeof(ret_code)) <= 0 ||
            RecvAll(sock_fd, &content_len, sizeof(content_len)) <= 0 ||
            content_len <= 0 ||
            RecvAll(sock_fd, content, content_len) <= 0) {
            printf("接收响应失败\n");
            close(sock_fd);
            continue;
        }
        content[content_len] = '\0';
        close(sock_fd);

        /* 打印结果 */
        if (ret_code == RET_OK)
            printf("\n 操作成功\n");
        else
            printf("\n 操作失败 (错误码 %d)\n", ret_code);
        printf("%s", content);
    }

    return 0;
}