#include "../inc/data_struct.h"

static int hash_collision = 0;

// BKDR字符串哈希算法（高效低冲突）
int HashCalc(const char* dev_id)
{
    unsigned int hash = 0;
    while (*dev_id != '\0')
    {
        hash = hash * 31 + (*dev_id);
        dev_id++;
    }
    return hash % HASH_TABLE_SIZE;
}

int GetHashCollisionCount(void)
{
    return hash_collision;
}

// 创建哈希表
HashTable* CreateHashTable()
{
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) return NULL;
    memset(table, 0, sizeof(HashTable));
    return table;
}

// 销毁哈希表（仅释放桶结构，节点内存由设备树统一管理）
void DestroyHashTable(HashTable* table)
{
    if (!table) return;
    free(table);
}

// 插入节点（头插法，O(1)）
int HashInsertNode(HashTable* table, DevNode* node)
{
    if (!table || !node) return ERR_PARAM_INVALID;
    int idx = HashCalc(node->dev_id);
    DevNode* p = table->bucket[idx];

    // 查重
    while (p)
    {
        if (strcmp(p->dev_id, node->dev_id) == 0)
            return ERR_ID_DUPLICATE;
        p = p->brother;
        hash_collision++;
    }

    // 头插法入链
    node->brother = table->bucket[idx];
    table->bucket[idx] = node;
    return RET_OK;
}

// 按ID查找节点（平均O(1)）
DevNode* HashFindNode(DevManager* manager, const char* target_id)
{
    if (!manager || !target_id) return NULL;
    HashTable* table = manager->hash_table;
    int idx = HashCalc(target_id);
    DevNode* p = table->bucket[idx];

    while (p)
    {
        if (strcmp(p->dev_id, target_id) == 0)
            return p;
        p = p->brother;
    }
    return NULL;
}

// 删除指定节点
int HashDelNode(HashTable* table, const char* dev_id)
{
    if (!table || !dev_id) return ERR_PARAM_INVALID;
    int idx = HashCalc(dev_id);
    DevNode* cur = table->bucket[idx];
    DevNode* pre = NULL;

    while (cur)
    {
        if (strcmp(cur->dev_id, dev_id) == 0)
        {
            if (pre == NULL)
                table->bucket[idx] = cur->brother;
            else
                pre->brother = cur->brother;
            return RET_OK;
        }
        pre = cur;
        cur = cur->brother;
    }
    return ERR_NODE_NOT_FOUND;
}