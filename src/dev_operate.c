#include "../inc/data_struct.h"

// 递归查找节点
static DevNode* FindRecursive(DevNode* node, const char* dev_id)
{
    if (!node) return NULL;
    if (strcmp(node->dev_id, dev_id) == 0)
        return node;
    for (int i = 0; i < node->child_cnt; i++)
    {
        DevNode* res = FindRecursive(node->children[i], dev_id);
        if (res) return res;
    }
    return NULL;
}

// 递归销毁节点及其所有子节点
static void DestroyNode(DevNode* node)
{
    if (!node) return;
    for (int i = 0; i < node->child_cnt; i++)
        DestroyNode(node->children[i]);
    free(node);
}

DevManager* CreateDevManager()
{
    DevManager* mgr = (DevManager*)malloc(sizeof(DevManager));
    if (!mgr) return NULL;
    mgr->root = NULL;
    return mgr;
}

void DestroyDevManager(DevManager* mgr)
{
    if (!mgr) return;
    DestroyNode(mgr->root);
    free(mgr);
}

DevNode* FindNodeById(DevManager* mgr, const char* dev_id)
{
    if (!mgr || !dev_id) return NULL;
    return FindRecursive(mgr->root, dev_id);
}

int AddNewDevice(DevManager* mgr, const char* dev_id, const char* parent_id, int type, const char* name, float temp)
{
    if (!mgr || !dev_id || !name) return ERR_MALLOC_FAIL;

    // ID去重
    if (FindNodeById(mgr, dev_id) != NULL)
        return ERR_ID_DUPLICATE;

    // 创建新节点
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

    // 根节点判定：parent_id为空 或 为"root"
    if (strlen(parent_id) == 0 || strcmp(parent_id, "root") == 0)
    {
        mgr->root = new_node;
        return RET_OK;
    }

    // 查找父节点并挂载
    DevNode* parent = FindNodeById(mgr, parent_id);
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

    parent->children[parent->child_cnt++] = new_node;
    new_node->parent = parent;
    return RET_OK;
}

// 从父节点的子列表中移除目标节点
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

int DeleteDeviceById(DevManager* mgr, const char* del_id)
{
    if (!mgr || !del_id) return -1;
    DevNode* del_node = FindNodeById(mgr, del_id);
    if (!del_node) return -1;

    // 从父节点解绑
    if (del_node == mgr->root)
        mgr->root = NULL;
    else
        RemoveChild(del_node->parent, del_id);

    // 级联销毁所有子设备
    DestroyNode(del_node);
    return RET_OK;
}
