#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN         1024
#define ID_MAX_LEN      64
#define NAME_MAX_LEN    128
#define MAX_CHILD       64
#define ALARM_TEMP      30.0f
#define HASH_TABLE_SIZE 1024

// 设备类型枚举
typedef enum {
    DEV_STATION     = 0,
    DEV_TRANSFORMER = 1,
    DEV_PCS         = 2,
    DEV_BMS         = 3,
    DEV_CABIN       = 4,
    DEV_MODULE      = 5,
    DEV_CELL        = 7
} DevType;

// 设备节点（多叉树 + 哈希冲突链表复用brother指针）
typedef struct DevNode {
    char dev_id[ID_MAX_LEN];
    char parent_id[ID_MAX_LEN];
    char name[NAME_MAX_LEN];
    int dev_type;
    float temperature;

    struct DevNode* parent;
    struct DevNode* children[MAX_CHILD];
    int child_cnt;

    struct DevNode* brother;   // 哈希表链地址法冲突指针
} DevNode;

// 哈希表（链地址法）
typedef struct HashTable {
    DevNode* bucket[HASH_TABLE_SIZE];
} HashTable;

// 设备管理器
typedef struct {
    DevNode* root;
    HashTable* hash_table;
} DevManager;

// 统一错误码
#define RET_OK              0
#define ERR_MALLOC_FAIL    -1
#define ERR_PARENT_NOT_FOUND -2
#define ERR_ID_DUPLICATE   -3
#define ERR_FILE_OPEN      -4
#define ERR_FORMAT_ERROR   -5
#define ERR_ROOT_EXIST     -6
#define ERR_NODE_NOT_FOUND -7
#define ERR_PARAM_INVALID  -8

// ========== 哈希表模块 ==========
int HashCalc(const char* dev_id);
HashTable* CreateHashTable();
void DestroyHashTable(HashTable* table);
int HashInsertNode(HashTable* table, DevNode* node);
int HashDelNode(HashTable* table, const char* dev_id);
DevNode* HashFindNode(DevManager* manager, const char* target_id);
int GetHashCollisionCount(void);

// ========== 设备管理核心 ==========
DevManager* CreateDevManager();
void DestroyDevManager(DevManager* mgr);
void DestroyDeviceTree(DevManager* mgr);
int AddNewDevice(DevManager* mgr, const char* dev_id, const char* parent_id, int type, const char* name, float temp);
int DeleteDeviceById(DevManager* mgr, const char* dev_id);

// ========== CSV持久化 ==========
int LoadCsvData(DevManager* mgr, const char* path);
int SaveDataToCsv(DevManager* mgr, const char* path);

// 打印与告警
void PrintAllTree(DevManager* mgr);
void PrintAllAlarmCell(DevManager* mgr);
void PrintAllTreeToStream(DevManager* mgr, FILE* fp);
void PrintAllAlarmToStream(DevManager* mgr, FILE* fp);

#endif // DATA_STRUCT_H