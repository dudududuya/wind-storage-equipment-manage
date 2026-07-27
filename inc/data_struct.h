#ifndef C_DATA_STRUCT_H
#define C_DATA_STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局常量宏定义
#define BUF_LINE        1024
#define BUF_LEN         1024
#define ID_MAX_LEN      24
#define NAME_MAX_LEN    64
#define HASH_TABLE_SIZE 1024
#define ALARM_TEMP      60.0f

// 设备类型枚举：匹配储能完整层级
typedef enum
{
    DEV_DZ,     // 电站总站（根节点）
    DEV_XB,     // 箱变
    DEV_PCS,    // PCS变流器
    DEV_BMS,    // BMS电池管理系统
    DEV_PCSC,   // PCS储能仓
    DEV_BMSMZ,  // 电池模组
    DEV_MZDX,   // 模组单元
    DEV_CELL    // 电芯（温度告警核心设备）
} DevType;

// 树节点结构体：孩子兄弟表示法（固定child、brother双指针，增加parent反向溯源）
typedef struct DevNode
{
    char dev_id[ID_MAX_LEN];    // 设备唯一编号
    char dev_name[NAME_MAX_LEN];// 设备名称
    DevType type;               // 设备分类
    float temp;                 // 设备温度，用于判断过热告警
    int has_temp;               // 标记：1=有温度采集(电芯)，0=无温度
    struct DevNode* parent;     // 父设备指针，告警时快速向上追溯设备路径
    struct DevNode* child;      // 当前设备第一个子设备
    struct DevNode* brother;    // 同一父节点下，相邻同级设备
} DevNode;

// 哈希表结构：加速设备ID精准查找
typedef struct HashTable
{
    DevNode* bucket[HASH_TABLE_SIZE];
} HashTable;

// 全局管理器：整合整项目所有资源
typedef struct DevManager
{
    DevNode* root;                          // 设备树根节点（电站总站）
    HashTable* hash_table;                  // 哈希索引表
    // 各类型设备自增计数器，用于自动生成唯一设备ID
    int cnt_dz, cnt_xb, cnt_pcs, cnt_bms;
    int cnt_pcsc, cnt_bmsmz, cnt_mzdx, cnt_cell;
} DevManager;

// ==================== 函数声明 ====================
// 1. 内存初始化与销毁工具 src/io_tool.c
DevManager* CreateDevManager();
void DestroyDevManager(DevManager* manager);
// 无温度设备专用（箱变/PCS等）
DevNode* CreateNodeNoTemp(const char* id, const char* name, DevType type);
// 带温度设备专用（仅电芯）
DevNode* CreateNodeWithTemp(const char* id, const char* name, DevType type, float temp);


// 2. 哈希表索引工具 src/hash_table.c
int HashCalc(const char* dev_id);
DevNode* HashFindNode(DevManager* manager, const char* target_id);

// 3. 设备增删业务 src/dev_operate.c
int AddNewDevice(DevManager* manager, DevType type, const char* parent_id, const char* dev_name, float temp);
int DeleteDeviceById(DevManager* manager, const char* del_id);

// 4. CSV文件持久化读写 src/file_io.c
int LoadCsvData(DevManager* manager, const char* csv_file_path);
int SaveDataToCsv(DevManager* manager, const char* csv_file_path);

// 5. 树形打印、过热电芯告警查询 src/tree_print.c
void PrintWholeDevTree(DevManager* manager);
void PrintAllAlarmCell(DevManager* manager);

#endif
