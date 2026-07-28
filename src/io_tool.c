
#include "../inc/data_struct.h"

// 创建空哈希表
HashTable* CreateHashTable()
{
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL)
        return NULL;
    memset(table->bucket, 0, sizeof(table->bucket));
    return table;
}

// 销毁哈希表（仅释放表头，节点由树统一释放）
void DestroyHashTable(HashTable* table)
{
    if (table != NULL)
        free(table);
}

// 创建无温度节点
DevNode* CreateNodeNoTemp(const char* id, const char* name, DevType type)
{
    DevNode* node = (DevNode*)malloc(sizeof(DevNode));
    if (!node) return NULL;
    strncpy(node->dev_id, id, ID_MAX_LEN - 1);
    strncpy(node->dev_name, name, NAME_MAX_LEN - 1);
    node->dev_id[ID_MAX_LEN - 1] = '\0';
    node->dev_name[NAME_MAX_LEN - 1] = '\0';
    node->type = type;
    node->temp = 0.0f;
    node->has_temp = 0;
    node->parent = NULL;
    node->child = NULL;
    node->brother = NULL;
    return node;
}

// 创建电芯带温度节点
DevNode* CreateNodeWithTemp(const char* id, const char* name, DevType type, float temp)
{
    DevNode* node = CreateNodeNoTemp(id, name, type);
    if (!node) return NULL;
    node->temp = temp;
    node->has_temp = 1;
    return node;
}

// 递归释放整棵设备树
static void DestroyNodeRecur(DevNode* node)
{
    if (!node) return;
    DestroyNodeRecur(node->child);
    DestroyNodeRecur(node->brother);
    free(node);
}

// 创建全局管理器
DevManager* CreateDevManager()
{
    DevManager* mgr = (DevManager*)malloc(sizeof(DevManager));
    if (!mgr) return NULL;
    memset(mgr, 0, sizeof(DevManager));
    mgr->hash_table = CreateHashTable();
    if (!mgr->hash_table)
    {
        free(mgr);
        return NULL;
    }
    return mgr;
}

// 释放全部资源
void DestroyDevManager(DevManager* manager)
{
    if (!manager) return;
    DestroyNodeRecur(manager->root);
    DestroyHashTable(manager->hash_table);
    free(manager);
}
