#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define EPOH 2880
#define MAXNUM 1024

int extract_sat_count(char* buff); //计算一个历元的卫星对数


int main(void)
{
	char buff[MAXNUM] = { '0' };
	int n = 1;
	int fix_ar = 0;
	int count_ar = 0;

	FILE* fp_ewlAR = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\nnor_UC_par_bd.pos.ewlres", "r");
	//FILE* fp_wlAR = ("", "r");
	//FILE* fp_nlAR = ("", "r");

	FILE* fp_ewlAR_num = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\fix_ar_res\\nnor_UC_par_bd.pos.ewlres.arnum", 
		"w"); //自动创建文件

	if (!(fp_ewlAR /* && fp_nlAR && fp_wlAR*/))
	{
		perror("文件打开失败：");

		return 0;
	}
	fprintf(fp_ewlAR_num, "       历元                           该历元固定模糊度数量\n"); //将文件头写入文件
	while (n <= EPOH && fgets(buff, sizeof(buff), fp_ewlAR))
	{
		int i = extract_sat_count(buff);

		buff[strlen(buff) - 1] =' '; //替换换行符
		if (buff[0] == '*')
		{
			printf("历元 %d: %s\n", n, buff);
			fputs(buff, fp_ewlAR_num);
			while (i > 1)
			{
				fgets(buff, sizeof(buff), fp_ewlAR);
				if (buff[48] == '1')
				{
					fix_ar++;
				}
				if (buff[89] == '1')
				{
					fix_ar++;
				}
				if (buff[130] == '1')
				{
					fix_ar++;
				}
				i--;
			}
			fprintf(fp_ewlAR_num, "          %d\n", fix_ar);
			count_ar += fix_ar;
			fix_ar = 0;
		}
		n++;
	}
	fprintf(fp_ewlAR_num, "总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%f",
		n-1, count_ar, ((float)count_ar) / (n-1));
	printf("提取完成\n");
	fclose(fp_ewlAR);
	fclose(fp_ewlAR_num);
	
	return 0;
}

int extract_sat_count(char* buff)
{
	if (!(isdigit(buff[32]))) //判断字符是否为数字
	{
		return (buff[33] - '0');
	}
	if (isdigit(buff[32]))
	{
		return ((buff[32] - '0') * 10 + (buff[33] - '0'));
	}
}







