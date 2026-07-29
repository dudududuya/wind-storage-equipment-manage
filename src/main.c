#include "../inc/data_struct.h"
#include <stdio.h>

static void AddDeviceMenu(DevManager* mgr)
{
    char dev_id[ID_MAX_LEN], parent_id[ID_MAX_LEN], name[NAME_MAX_LEN];
    int type;
    float temp = 0.0f;

    printf("\n=== 新增设备 ===\n");
    printf("请输入设备ID: ");
    scanf("%s", dev_id);
    printf("请输入父设备ID(根节点直接回车): ");
    getchar();
    fgets(parent_id, ID_MAX_LEN, stdin);
    parent_id[strcspn(parent_id, "\n")] = '\0';
    printf("请输入设备类型(0总站 1箱变 2PCS 3BMS 4储能仓 5电池簇 6模组 7电芯): ");
    scanf("%d", &type);
    printf("请输入设备名称: ");
    getchar();
    fgets(name, NAME_MAX_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';

    if (type == DEV_CELL)
    {
        printf("请输入电芯温度: ");
        scanf("%f", &temp);
    }

    int ret = AddNewDevice(mgr, dev_id, parent_id, type, name, temp);
    switch (ret)
    {
        case RET_OK:
            printf("设备添加成功\n");
            break;
        case ERR_ID_DUPLICATE:
            printf("添加失败：设备ID已存在\n");
            break;
        case ERR_PARENT_NOT_FOUND:
            printf("添加失败：父设备不存在\n");
            break;
        default:
            printf("添加失败，错误码: %d\n", ret);
    }
}

static void DelDeviceMenu(DevManager* mgr)
{
    char del_id[ID_MAX_LEN];
    printf("\n=== 删除设备 ===\n");
    printf("请输入要删除的设备ID: ");
    scanf("%s", del_id);
    int ret = DeleteDeviceById(mgr, del_id);
    if (ret == RET_OK)
        printf("删除成功，该设备及其所有子设备已移除\n");
    else
        printf("删除失败：未找到该设备\n");
}

int main(void)
{
    int sel;
    int save_ret;

    printf("程序启动中...\n");
    DevManager* mgr = CreateDevManager();
    if (!mgr)
    {
        printf("内存分配失败，程序退出\n");
        return -1;
    }
    printf("设备管理器创建成功\n");

    // 自动加载CSV设备数据
    printf("正在加载CSV设备数据...\n");
    int load_ret = LoadCsvData(mgr, "./data/dev.csv");
    if (load_ret != RET_OK)
        printf("警告：CSV加载异常\n");

    while (1)
    {
        printf("\n======== 风电储能电站设备管理系统 ========\n");
        printf("1. 打印完整设备树\n");
        printf("2. 查看高温告警电芯\n");
        printf("3. 手动新增设备\n");
        printf("4. 删除指定设备\n");
        printf("0. 保存并退出程序\n");
        printf("请输入操作序号: ");
        scanf("%d", &sel);

        switch (sel)
        {
            case 1:
                PrintAllTree(mgr);
                break;
            case 2:
                PrintAllAlarmCell(mgr);
                break;
            case 3:
                AddDeviceMenu(mgr);
                break;
            case 4:
                DelDeviceMenu(mgr);
                break;
            case 0:
                printf("正在保存数据...\n");
                save_ret = SaveDataToCsv(mgr, "./data/dev.csv");
                if (save_ret == RET_OK)
                    printf("数据保存成功\n");
                else
                    printf("数据保存失败\n");
                DestroyDevManager(mgr);
                printf("程序正常退出\n");
                return 0;
            default:
                printf("输入无效，请输入0-4\n");
        }
    }
}
