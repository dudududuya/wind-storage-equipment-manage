#include "../inc/data_struct.h"
static int hash_collision = 0;

// 简单字符串哈希算法
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

// 插入节点到哈希表
int HashInsertNode(HashTable* table, DevNode* node)
{
    if (!table || !node) return ERR_MALLOC_FAIL;
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
    // 头插法
    node->brother = table->bucket[idx];
    table->bucket[idx] = node;
    return RET_OK;
}

// 根据ID查找节点
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

// 哈希表删除指定节点
int HashDelNode(HashTable* table, const char* dev_id)
{
    if (!table || !dev_id) return RET_OK;
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
    return ERR_PARENT_NOT_FOUND;
}
