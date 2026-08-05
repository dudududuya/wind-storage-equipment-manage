#include "../inc/data_struct.h"

// 递归销毁节点及其子树（内部辅助）
static void DestroyNode(DevNode* node)
{
    if (!node) return;
    for (int i = 0; i < node->child_cnt; i++)
        DestroyNode(node->children[i]);
    free(node);
}

// 递归移除子树所有节点的哈希索引（内部辅助）
static void RemoveHashRecur(HashTable* table, DevNode* node)
{
    if (!node) return;
    for (int i = 0; i < node->child_cnt; i++)
        RemoveHashRecur(table, node->children[i]);
    HashDelNode(table, node->dev_id);
}

// 创建设备管理器
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
    mgr->root = NULL;
    return mgr;
}

// 销毁设备管理器（全资源释放）
void DestroyDevManager(DevManager* mgr)
{
    if (!mgr) return;
    DestroyNode(mgr->root);
    DestroyHashTable(mgr->hash_table);
    free(mgr);
}

// 仅销毁设备树，保留管理器结构
void DestroyDeviceTree(DevManager* mgr)
{
    if (!mgr) return;
    RemoveHashRecur(mgr->hash_table, mgr->root);
    DestroyNode(mgr->root);
    mgr->root = NULL;
}

// 新增设备
int AddNewDevice(DevManager* mgr, const char* dev_id, const char* parent_id, int type, const char* name, float temp)
{
    if (!mgr || !dev_id || !name) return ERR_PARAM_INVALID;

    // 1. 哈希查重
    if (HashFindNode(mgr, dev_id) != NULL)
        return ERR_ID_DUPLICATE;

    // 2. 创建新节点
    DevNode* new_node = (DevNode*)malloc(sizeof(DevNode));
    if (!new_node) return ERR_MALLOC_FAIL;
    memset(new_node, 0, sizeof(DevNode));

    strncpy(new_node->dev_id, dev_id, ID_MAX_LEN - 1);
    strncpy(new_node->parent_id, parent_id, ID_MAX_LEN - 1);
    strncpy(new_node->name, name, NAME_MAX_LEN - 1);
    new_node->dev_type = type;
    new_node->temperature = temp;
    new_node->child_cnt = 0;
    new_node->parent = NULL;
    new_node->brother = NULL;

    // 3. 根节点处理（禁止重复根节点，防止内存泄漏）
    if (strlen(parent_id) == 0 || strcmp(parent_id, "root") == 0)
    {
        if (mgr->root != NULL)
        {
            free(new_node);
            return ERR_ROOT_EXIST;
        }
        mgr->root = new_node;
        HashInsertNode(mgr->hash_table, new_node);
        return RET_OK;
    }

    // 4. 哈希查找父节点
    DevNode* parent = HashFindNode(mgr, parent_id);
    if (!parent)
    {
        free(new_node);
        return ERR_PARENT_NOT_FOUND;
    }
    if (parent->child_cnt >= MAX_CHILD)
    {
        free(new_node);
        return ERR_MALLOC_FAIL;
    }

    // 5. 挂载到父节点
    parent->children[parent->child_cnt++] = new_node;
    new_node->parent = parent;

    // 6. 插入哈希索引
    HashInsertNode(mgr->hash_table, new_node);

    return RET_OK;
}

// 从父节点移除子节点（内部辅助）
static int RemoveChild(DevNode* parent, const char* child_id)
{
    if (!parent) return -1;
    for (int i = 0; i < parent->child_cnt; i++)
    {
        if (strcmp(parent->children[i]->dev_id, child_id) == 0)
        {
            for (int j = i; j < parent->child_cnt - 1; j++)
                parent->children[j] = parent->children[j + 1];
            parent->child_cnt--;
            return 0;
        }
    }
    return -1;
}

// 删除设备（连同所有子设备一并删除）
int DeleteDeviceById(DevManager* mgr, const char* del_id)
{
    if (!mgr || !del_id) return ERR_PARAM_INVALID;

    // 1. 哈希定位目标节点
    DevNode* del_node = HashFindNode(mgr, del_id);
    if (!del_node) return ERR_NODE_NOT_FOUND;

    // 2. 递归移除整棵子树的哈希索引
    RemoveHashRecur(mgr->hash_table, del_node);

    // 3. 从父节点摘除
    if (del_node == mgr->root)
    {
        mgr->root = NULL;
    }
    else
    {
        RemoveChild(del_node->parent, del_id);
    }

    // 4. 递归释放子树内存
    DestroyNode(del_node);
    return RET_OK;
}