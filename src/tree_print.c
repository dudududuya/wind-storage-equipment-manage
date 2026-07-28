#include "../inc/data_struct.h"
#include <stdio.h>

// 递归打印树形层级
static void PrintRecur(DevNode* node, int level)
{
    if (!node) return;
    // 层级缩进
    for (int i = 0; i < level; i++)
        printf("  ");
    printf("└─ [%s] %s | 类型:%d 温度:%.2f℃\n",
           node->dev_id, node->name, node->dev_type, node->temperature);

    for (int i = 0; i < node->child_cnt; i++)
        PrintRecur(node->children[i], level + 1);
}

void PrintAllTree(DevManager* mgr)
{
    if (!mgr || !mgr->root)
    {
        printf("暂无设备数据\n");
        return;
    }
    printf("========== 完整设备树 ==========\n");
    PrintRecur(mgr->root, 0);
    printf("===============================\n");
}

// 递归查找超温电芯并打印完整溯源路径
static void FindAlarmRecur(DevNode* node)
{
    if (!node) return;

    // 仅电芯类型且温度超过告警阈值
    if (node->dev_type == DEV_CELL && node->temperature >= ALARM_TEMP)
    {
        printf("\n【高温告警】电芯ID: %s | 名称: %s | 温度: %.2f℃ (阈值: %.0f℃ )\n",
               node->dev_id, node->name, node->temperature, ALARM_TEMP);
        // 向上溯源完整设备链路
        printf("  完整层级链路: ");
        DevNode* path[32];
        int path_cnt = 0;
        DevNode* p = node;
        while (p)
        {
            path[path_cnt++] = p;
            p = p->parent;
        }
        // 从根节点到电芯倒序打印
        for (int i = path_cnt - 1; i >= 0; i--)
        {
            printf("%s", path[i]->name);
            if (i > 0) printf(" → ");
        }
        printf("\n");
    }

    for (int i = 0; i < node->child_cnt; i++)
        FindAlarmRecur(node->children[i]);
}

void PrintAllAlarmCell(DevManager* mgr)
{
    if (!mgr || !mgr->root)
    {
        printf("暂无设备数据\n");
        return;
    }
    printf("========== 高温告警电芯列表 ==========\n");
    FindAlarmRecur(mgr->root);
    printf("=====================================\n");
}
