#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINE 1024

// 判断是否为北斗三号卫星（PRN > 16）
int is_bds3(const char* prn) 
{
    if (prn[0] == 'C' || prn[0] == 'c') 
    {
        int num = atoi(prn + 1);
        return num > 16;
    }
    return 0;
}

int main() 
{
    FILE* fp_in, * fp_out;
    char line[MAX_LINE];
    char prn[4];
    int epoch = 0, sat_count, bds3_count;

    // 打开输入文件（直接指定文件名）
    fp_in = fopen("E:\\Program file\\rtklib_ppp\\2025300\\nnor3000.25o", "r");
    if (fp_in == NULL) 
    {
        printf("无法打开文件 nnor3000.25o\n");
        return 1;
    }

    // 打开输出文件
    fp_out = fopen("E:\\Program file\\rtklib_ppp\\2025300\\WHU_IGSresult.txt", "w");
    if (fp_out == NULL) 
    {
        printf("无法创建输出文件\n");
        fclose(fp_in);
        return 1;
    }

    // 写入表头
    fprintf(fp_out, "历元 北斗三号卫星数\n");

    // 跳过文件头
    while (fgets(line, MAX_LINE, fp_in)) 
    {
        if (strstr(line, "END OF HEADER"))
            break;
    }

    // 处理每个历元
    while (fgets(line, MAX_LINE, fp_in)) 
    {
        if (line[0] == '>') 
        {  // 新的历元
            epoch++;
            bds3_count = 0;

            // 获取卫星总数
            char* token = strtok(line, " ");
            for (int i = 0; i < 8; i++) 
                token = strtok(NULL, " ");
            sat_count = atoi(token); //该函数提取出字符中的数字，当开头遇到字母，返回0

            // 读取该历元的所有卫星
            for (int i = 0; i < sat_count; i++) 
            {
                fgets(line, MAX_LINE, fp_in);
                strncpy(prn, line, 3);
                prn[3] = '\0';

                if (is_bds3(prn))
                    bds3_count++;
            }

            // 写入结果
            fprintf(fp_out, "%d     %d\n", epoch, bds3_count);
            printf("历元 %d:     %d 颗北斗三号卫星\n", epoch, bds3_count);
        }
    }

    printf("\n完成！共处理 %d 个历元\n", epoch);
    printf("结果已保存到 result.txt\n");

    fclose(fp_in);
    fclose(fp_out);
    return 0;
}