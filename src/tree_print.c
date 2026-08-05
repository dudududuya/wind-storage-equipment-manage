#include "../inc/data_struct.h"
#include <stdio.h>

// 递归打印树形层级（内部通用实现）
static void PrintRecur(FILE* fp, DevNode* node, int level)
{
    if (!node) return;
    for (int i = 0; i < level; i++)
        fprintf(fp, "  ");
    fprintf(fp, "└─ [%s] %s | 类型:%d 温度:%.2f℃\n",
           node->dev_id, node->name, node->dev_type, node->temperature);

    for (int i = 0; i < node->child_cnt; i++)
        PrintRecur(fp, node->children[i], level + 1);
}

// 原接口：打印到控制台（兼容原有调用）
void PrintAllTree(DevManager* mgr)
{
    PrintAllTreeToStream(mgr, stdout);
}

// 新接口：打印到指定文件流（供引擎内存流使用）
void PrintAllTreeToStream(DevManager* mgr, FILE* fp)
{
    if (!mgr || !mgr->root)
    {
        fprintf(fp, "暂无设备数据\n");
        return;
    }
    fprintf(fp, "========== 完整设备树 ==========\n");
    PrintRecur(fp, mgr->root, 0);
    fprintf(fp, "===============================\n");
}

// 递归查找超温电芯（内部通用实现）
static void FindAlarmRecur(FILE* fp, DevNode* node)
{
    if (!node) return;

    if (node->dev_type == DEV_CELL && node->temperature >= ALARM_TEMP)
    {
        fprintf(fp, "\n【高温告警】电芯ID: %s | 名称: %s | 温度: %.2f℃ (阈值: %.0f℃ )\n",
               node->dev_id, node->name, node->temperature, ALARM_TEMP);
        fprintf(fp, "  完整层级链路: ");
        DevNode* path[32];
        int path_cnt = 0;
        DevNode* p = node;
        while (p)
        {
            path[path_cnt++] = p;
            p = p->parent;
        }
        for (int i = path_cnt - 1; i >= 0; i--)
        {
            fprintf(fp, "%s", path[i]->name);
            if (i > 0) fprintf(fp, " → ");
        }
        fprintf(fp, "\n");
    }

    for (int i = 0; i < node->child_cnt; i++)
        FindAlarmRecur(fp, node->children[i]);
}

// 原接口：打印到控制台（兼容原有调用）
void PrintAllAlarmCell(DevManager* mgr)
{
    PrintAllAlarmToStream(mgr, stdout);
}

// 新接口：打印到指定文件流（供引擎内存流使用）
void PrintAllAlarmToStream(DevManager* mgr, FILE* fp)
{
    if (!mgr || !mgr->root)
    {
        fprintf(fp, "暂无设备数据\n");
        return;
    }
    fprintf(fp, "========== 高温告警电芯列表 ==========\n");
    FindAlarmRecur(fp, mgr->root);
    fprintf(fp, "=====================================\n");
}