#include "../inc/data_struct.h"

/**
 * CSV行拆分，兼容双引号包裹字段
 */
static int split_csv_line(char* line, char** fields, int max_fields)
{
    if (!line || !fields || max_fields <= 0)
        return 0;

    int col_cnt = 0;
    int in_quote = 0;
    char* ptr = line;
    fields[col_cnt++] = ptr;

    while (*ptr != '\0' && col_cnt < max_fields)
    {
        if (*ptr == '"')
        {
            in_quote = !in_quote;
            ptr++;
            continue;
        }

        if (*ptr == ',' && in_quote == 0)
        {
            *ptr = '\0';
            fields[col_cnt++] = ptr + 1;
        }
        ptr++;
    }
    return col_cnt;
}

// 加载CSV设备数据
int LoadCsvData(DevManager* mgr, const char* path)
{
    if (!mgr || !path) return ERR_FILE_OPEN;

    FILE* fp = fopen(path, "r");
    if (!fp)
    {
        fprintf(stderr, "[错误] 无法打开文件: %s\n", path);
        return ERR_FILE_OPEN;
    }

    char buf[BUF_LEN];
    char* fields[16];
    int line_num = 0;

    // 跳过表头
    if (!fgets(buf, BUF_LEN, fp))
    {
        fclose(fp);
        return ERR_FORMAT_ERROR;
    }
    line_num++;

    int success = 0, fail = 0;
    while (fgets(buf, BUF_LEN, fp))
    {
        line_num++;
        buf[strcspn(buf, "\n\r")] = '\0';
        if (strlen(buf) == 0)
            continue;

        int col_num = split_csv_line(buf, fields, 16);
        if (col_num < 8)
        {
            fprintf(stderr, "[警告] 第%d行字段不足(%d列)，跳过\n", line_num, col_num);
            fail++;
            continue;
        }

        const char* id = fields[0];
        const char* type_str = fields[1];
        const char* name = fields[2];
        const char* pid = fields[3];
        const char* temp_str = fields[7];

        int type = atoi(type_str);
        float temp = atof(temp_str);
        int ret = AddNewDevice(mgr, id, pid, type, name, temp);

        if (ret == RET_OK)
            success++;
        else
            fail++;
    }

    fclose(fp);
    fprintf(stderr, "[CSV加载] 成功 %d 台，失败 %d 台\n", success, fail);
    return RET_OK;
}

// 递归写入设备节点
static void WriteNodeRecur(FILE* fp, DevNode* node)
{
    if (!node) return;
    fprintf(fp, "%s,%d,%s,%s,0,0,0,%.2f,0,\"{}\"\n",
            node->dev_id, node->dev_type, node->name,
            node->parent_id, node->temperature);
    for (int i = 0; i < node->child_cnt; i++)
        WriteNodeRecur(fp, node->children[i]);
}

// 保存数据到CSV
int SaveDataToCsv(DevManager* mgr, const char* path)
{
    if (!mgr || !path) return ERR_FILE_OPEN;
    FILE* fp = fopen(path, "w");
    if (!fp) return ERR_FILE_OPEN;

    fprintf(fp, "dev_id,dev_type,name,parent_id,soc,voltage,current,temperature,alarm_code,ext_json\n");
    WriteNodeRecur(fp, mgr->root);

    fclose(fp);
    return RET_OK;
}