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
	FILE* fp_wlAR = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\nnor_UC_par_bd.pos.wlres", "r");
	FILE* fp_nlAR = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\nnor_UC_par_bd.pos.nlres", "r");

	/*创建三个结果输出文件*/
	FILE* fp_ewlAR_num = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\fix_ar_res\\nnor_UC_par_bd_FF.pos.ewlres.arnum", 
		"w"); //超宽巷文件
	FILE* fp_wlAR_num = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\fix_ar_res\\nnor_UC_par_bd_FF.pos.wlres.arnum",
		"w"); //宽巷文件
	FILE* fp_nlAR_num = fopen("E:\\Program file\\rtklib_ppp\\2025300\\results_mpar\\五频\\fix_ar_res\\nnor_UC_par_bd_FF.pos.nlres.arnum",
		"w"); //窄巷文件

	if (!(fp_ewlAR  && fp_nlAR && fp_wlAR && fp_ewlAR_num && fp_nlAR_num && fp_wlAR_num))
	{
		perror("文件打开失败：");

		return 0;
	}
	/*写入文件头内容到输出文件中*/
	fprintf(fp_ewlAR_num, "       历元                           该历元固定模糊度数量\n"); 
	fprintf(fp_wlAR_num, "       历元                           该历元固定模糊度数量\n"); 
	fprintf(fp_nlAR_num, "       历元                           该历元固定模糊度数量\n"); 

	///*处理超宽巷文件*/
	//printf("超宽巷处理：\n");
	//while (n <= EPOH && fgets(buff, sizeof(buff), fp_ewlAR))
	//{
	//	int i = extract_sat_count(buff);

	//	buff[strlen(buff) - 1] =' '; //替换换行符
	//	if (buff[0] == '*')
	//	{
	//		printf("历元 %d: %s\n", n, buff);
	//		fputs(buff, fp_ewlAR_num);
	//		while (i > 1)
	//		{
	//			fgets(buff, sizeof(buff), fp_ewlAR);
	//			if (buff[48] == '1')
	//			{
	//				fix_ar++;
	//			}
	//			if (buff[89] == '1')
	//			{
	//				fix_ar++;
	//			}
	//			if (buff[130] == '1')
	//			{
	//				fix_ar++;
	//			}
	//			i--;
	//		}
	//		fprintf(fp_ewlAR_num, "          %d\n", fix_ar);
	//		count_ar += fix_ar;
	//		fix_ar = 0;
	//	}
	//	n++;
	//}
	//fprintf(fp_ewlAR_num, "超宽巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%f",
	//	n-1, count_ar, ((float)count_ar) / (n-1));
	//printf("超宽巷提取完成\n");


	/*超宽巷处理*/
	printf("\n\n超宽巷处理：\n");
	n = 1;
	fix_ar = 0;
	count_ar = 0;

	while (fgets(buff, sizeof(buff), fp_ewlAR))
	{
		if (buff[0] == '*')
		{
			// 处理历元行
			buff[strlen(buff) - 1] = ' '; // 替换换行符
			printf("历元 %d: %s\n", n, buff);
			fputs(buff, fp_ewlAR_num);

			// 读取并处理该历元的所有卫星对数据
			fix_ar = 0;
			long current_pos_ewl = ftell(fp_ewlAR); // 保存当前位置

			while (fgets(buff, sizeof(buff), fp_ewlAR) && buff[0] != '*')
			{
				// 检查固定标志
				if (strlen(buff) > 48 && buff[48] == '1')
				{
					fix_ar++;
				}
				if (strlen(buff) > 89 && buff[89] == '1')
				{
					fix_ar++;
				}
				if (strlen(buff) > 130 && buff[130] == '1')
				{
					fix_ar++;
				}
				current_pos_ewl = ftell(fp_ewlAR);
			}

			current_pos_ewl = ftell(fp_ewlAR);
			// 回退文件指针，因为已经读取到了下一个'*'行
			if (buff[0] == '*')
			{
				fseek(fp_ewlAR, current_pos_ewl - strlen(buff) - 1, SEEK_SET);
			}
			fprintf(fp_ewlAR_num, "          %d\n", fix_ar);
			count_ar += fix_ar;
			n++;
		}
	}

	fprintf(fp_ewlAR_num, "超宽巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%.2f\n",
		n - 1, count_ar, n > 1 ? ((float)count_ar) / (n - 1) : 0);
	printf("窄巷提取完成\n");

	/*处理宽巷文件*/
	//printf("\n\n宽巷处理：\n");
	//n = 1;
	//fix_ar = 0;
	//count_ar = 0;
	//while (fgets(buff, sizeof(buff), fp_wlAR))
	//{
	//	//int i = extract_sat_count(buff);

	//	buff[strlen(buff) - 1] = ' '; //替换换行符
	//	if (buff[0] == '*')
	//	{
	//		while (n <= EPOH)
	//		{
	//			printf("历元 %d: %s\n", n, buff);
	//			fputs(buff, fp_wlAR_num);
	//			for (fgets(buff, sizeof(buff), fp_wlAR); buff[0] != '*'; fgets(buff, sizeof(buff), fp_wlAR))
	//			{
	//				if (buff[48] == '1')
	//				{
	//					fix_ar++;
	//				}
	//			}
	//			fprintf(fp_wlAR_num, "          %d\n", fix_ar); 
	//			count_ar += fix_ar;
	//			fix_ar = 0;
	//			n++;
	//			buff[strlen(buff) - 1] = ' '; //替换换行符
	//		}

	//	}
	//	
	//}
	//fprintf(fp_wlAR_num, "宽巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%f",
	//	n - 1, count_ar, ((float)count_ar) / (n - 1));
	//printf("宽巷提取完成\n");

	/*处理宽巷文件*/
	printf("\n\n宽巷处理：\n");
	n = 1;
	fix_ar = 0;
	count_ar = 0;

	while (fgets(buff, sizeof(buff), fp_wlAR))
	{
		if (buff[0] == '*')
		{
			// 处理历元行
			buff[strlen(buff) - 1] = ' '; // 替换换行符
			printf("历元 %d: %s\n", n, buff);
			fputs(buff, fp_wlAR_num);

			// 读取并处理该历元的所有卫星对数据
			fix_ar = 0;
			long current_pos_wl = ftell(fp_wlAR); // 保存当前位置

			while (fgets(buff, sizeof(buff), fp_wlAR) && buff[0] != '*')
			{
				// 检查固定标志
				if (strlen(buff) > 48 && buff[48] == '1')
				{
					fix_ar++;
				}
				current_pos_wl = ftell(fp_wlAR);
			}

			current_pos_wl = ftell(fp_wlAR);
			// 回退文件指针，因为已经读取到了下一个'*'行
			if (buff[0] == '*')
			{
				//printf("buff长度: %d\n", (int)strlen(buff));
				//printf("current_pos: %ld\n", current_pos);
				//printf("计算回退位置: %ld\n", current_pos - strlen(buff));
			
				fseek(fp_wlAR, current_pos_wl - strlen(buff) - 1, SEEK_SET);
				//fgets(debug_buff, sizeof(debug_buff), fp_wlAR);
				//printf("回退后读到的内容: %s\n", debug_buff);
			}
			fprintf(fp_wlAR_num, "          %d\n", fix_ar);
			count_ar += fix_ar;
			n++;
		}
	}

	fprintf(fp_wlAR_num, "宽巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%.2f\n",
		n - 1, count_ar, n > 1 ? ((float)count_ar) / (n - 1) : 0);
	printf("宽巷提取完成\n");


	/*窄巷处理*/
	printf("\n\n窄巷处理：\n");
	n = 1;
	fix_ar = 0;
	count_ar = 0;

	while (fgets(buff, sizeof(buff), fp_nlAR))
	{
		if (buff[0] == '*')
		{
			// 处理历元行
			buff[strlen(buff) - 1] = ' '; // 替换换行符
			printf("历元 %d: %s\n", n, buff);
			fputs(buff, fp_nlAR_num);

			// 读取并处理该历元的所有卫星对数据
			fix_ar = 0;
			long current_pos_nl = ftell(fp_nlAR); // 保存当前位置

			while (fgets(buff, sizeof(buff), fp_nlAR) && buff[0] != '*')
			{
				// 检查固定标志
				if (strlen(buff) > 48 && buff[48] == '1')
				{
					fix_ar++;
				}
				current_pos_nl = ftell(fp_nlAR);
			}

			current_pos_nl = ftell(fp_nlAR);
			// 回退文件指针，因为已经读取到了下一个'*'行
			if (buff[0] == '*')
			{
				fseek(fp_nlAR, current_pos_nl - strlen(buff) - 1, SEEK_SET);
			}
			fprintf(fp_nlAR_num, "          %d\n", fix_ar);
			count_ar += fix_ar;
			n++;
		}
	}

	fprintf(fp_nlAR_num, "窄巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%.2f\n",
		n - 1, count_ar, n > 1 ? ((float)count_ar) / (n - 1) : 0);
	printf("窄巷提取完成\n");

	fclose(fp_ewlAR);
	fclose(fp_ewlAR_num);
	fclose(fp_wlAR);
	fclose(fp_wlAR_num);
	fclose(fp_nlAR);
	fclose(fp_nlAR_num);
	
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







