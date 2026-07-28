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

// 设备类型枚举 与CSV数字完全对应
typedef enum {
    DEV_STATION     = 0,    // 电站总站
    DEV_TRANSFORMER = 1,    // 箱变/变压器
    DEV_PCS         = 2,    // PCS变流器
    DEV_BMS         = 3,    // BMS电池管理系统
    DEV_CABIN       = 4,    // 储能舱
    DEV_MODULE      = 5,    // 电池模组
    DEV_CELL        = 7     // 电芯（温度告警核心）
} DevType;

// 设备树节点 带父指针 支持快速溯源
typedef struct DevNode {
    char dev_id[ID_MAX_LEN];
    char parent_id[ID_MAX_LEN];
    char name[NAME_MAX_LEN];
    int dev_type;
    float temperature;

    struct DevNode* parent;     // 父节点指针 告警向上溯源
    struct DevNode* children[MAX_CHILD];
    int child_cnt;
} DevNode;

// 全局设备管理器
typedef struct {
    DevNode* root;
} DevManager;

// 统一错误码
#define RET_OK              0
#define ERR_MALLOC_FAIL    -1
#define ERR_PARENT_NOT_FOUND -2
#define ERR_ID_DUPLICATE   -3
#define ERR_FILE_OPEN      -4
#define ERR_FORMAT_ERROR   -5

// ================== 函数声明 ==================
// 设备管理
DevManager* CreateDevManager();
void DestroyDevManager(DevManager* mgr);
int AddNewDevice(DevManager* mgr, const char* dev_id, const char* parent_id, int type, const char* name, float temp);
DevNode* FindNodeById(DevManager* mgr, const char* dev_id);
int DeleteDeviceById(DevManager* mgr, const char* dev_id);

// CSV读写
int LoadCsvData(DevManager* mgr, const char* path);
int SaveDataToCsv(DevManager* mgr, const char* path);

// 打印与告警
void PrintAllTree(DevManager* mgr);
void PrintAllAlarmCell(DevManager* mgr);

#endif
