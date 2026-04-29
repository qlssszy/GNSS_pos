#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define EPOH 2880
#define MAXNUM 1024

#define INT_EWL "C:\\Users\\11755\\Desktop\\BDS_MPAR\\IGS\\2025350\\AM_NUM\\TF\\MPAR\\EWL"
#define INT_WL "C:\\Users\\11755\\Desktop\\BDS_MPAR\\IGS\\2025350\\AM_NUM\\TF\\MPAR\\WL"
#define INT_NL "C:\\Users\\11755\\Desktop\\BDS_MPAR\\IGS\\2025350\\AM_NUM\\TF\\MPAR\\NL"

int main(void)
{
	struct dirent* entry_ewl, * entry_wl, * entry_nl;
	DIR* dp_ewl, * dp_wl, * dp_nl;
	char buff[MAXNUM] = { '\0' };
	int n = 1;
	int fix_ar = 0;
	int count_ar = 0;

	char read_buff_ewl[MAXNUM] = { '\0' };
	char read_buff_wl[MAXNUM] = { '\0' };
	char read_buff_nl[MAXNUM] = { '\0' };

	char write_buff_ewl[MAXNUM] = { '\0' };
	char write_buff_wl[MAXNUM] = { '\0' };
	char write_buff_nl[MAXNUM] = { '\0' };

	char* sta_name_src = NULL;
	char* sta_name_des = NULL;

	//打开文件夹
	dp_ewl = opendir(INT_EWL);
	dp_wl = opendir(INT_WL);
	dp_nl = opendir(INT_NL);
	if (!(dp_ewl && dp_wl && dp_nl))
	{
		perror("打开输入文件目录失败：");

		return 0;
	}

	/*ewl*/
	printf("process ewl file:\n");
	while ((entry_ewl = readdir(dp_ewl))!=NULL)
	{
		if (!(strncmp(entry_ewl->d_name, ".", 1) && (strncmp(entry_ewl->d_name, "..", 1))))
			continue;
		if (strcmp(entry_ewl->d_name, "results") == 0)//跳过子目录
			continue;
		//构建每个要读取文件的输入路径
		snprintf(read_buff_ewl, MAXNUM, "%s\\%s", INT_EWL, entry_ewl->d_name);
		//构建结果输出路径
		snprintf(write_buff_ewl, MAXNUM, "%s\\results\\%s", INT_EWL, entry_ewl->d_name);
		strcat(write_buff_ewl, ".arbnum");

		sta_name_src = strstr(read_buff_ewl, entry_ewl->d_name);
		sta_name_des = strstr(write_buff_ewl, entry_ewl->d_name);

		//打开输入文件
		FILE* fp_ewl_r = fopen(read_buff_ewl, "r");
		if (!fp_ewl_r)
		{
			perror("打开超宽巷文件失败：");
			continue;
		}
		//创建输出文件
		FILE* fp_ewl_w=fopen(write_buff_ewl, "w");
		if (!fp_ewl_w)
		{
			perror("创建输出文件失败：");
			//fclose(fp_ewl_w);
			continue;
		}

		n = 1;
		fix_ar = 0;
		count_ar = 0;
		while (fgets(buff, sizeof(buff), fp_ewl_r))
		{
			if (buff[0] == '*')
			{
				// 处理历元行
				//buff[strlen(buff) - 1] = ' '; // 替换换行符
				buff[strcspn(buff, "\r\n")] = '\0';
				//fputs(buff, fp_ewl_w);
				fprintf(fp_ewl_w, "%s ", buff);

				// 读取并处理该历元的所有卫星对数据
				fix_ar = 0;

				long line_start_ewl = 0;
				int hit_next_epoch_ewl = 0;

				while (1)
				{
					line_start_ewl = ftell(fp_ewl_r);

					if (!fgets(buff, sizeof(buff), fp_ewl_r))
					{
						// 文件结束，说明当前历元后面没有更多内容
						break;
					}

					if (buff[0] == '*')
					{
						// 读到了下一个历元，回退到这一行开头，交给外层 while 处理
						hit_next_epoch_ewl = 1;
						break;
					}

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
				}

				if (hit_next_epoch_ewl)
				{
					fseek(fp_ewl_r, line_start_ewl, SEEK_SET);
				}
				fprintf(fp_ewl_w, "          %d\n", fix_ar);
				count_ar += fix_ar;
				n++;
			}
		}

		fprintf(fp_ewl_w, "超宽巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%.2f\n",
			n - 1, count_ar, n > 1 ? ((float)count_ar) / (n - 1) : 0);
		printf("Processed file: %s -> %s\n", sta_name_src, sta_name_des);
		//关闭文件
		fclose(fp_ewl_r);
		fclose(fp_ewl_w);
	}

	/*wl*/
	printf("\nprocess wl file:\n");
	while ((entry_wl = readdir(dp_wl))!=NULL)
	{
		if (!(strncmp(entry_wl->d_name, ".", 1) && strncmp(entry_wl->d_name, "..", 1)))
			continue;
		if (strcmp(entry_wl->d_name, "results") == 0)
			continue;
		//构建每个要读取文件的输入路径
		snprintf(read_buff_wl, MAXNUM, "%s\\%s", INT_WL, entry_wl->d_name);
		//构建结果输出路径
		snprintf(write_buff_wl, MAXNUM, "%s\\results\\%s", INT_WL, entry_wl->d_name);
		strcat(write_buff_wl, ".arbnum");

		sta_name_src = strstr(read_buff_wl, entry_wl->d_name);
		sta_name_des = strstr(write_buff_wl, entry_wl->d_name);

		//打开文件
		FILE* fp_wl_r = fopen(read_buff_wl, "r");
		FILE* fp_wl_w = fopen(write_buff_wl, "w");
		if (!(fp_wl_r && fp_wl_w))
		{
			perror("宽巷文件打开/创建失败：");
			//fclose(fp_wl_w);
			continue;
		}

		n = 1;
		fix_ar = 0;
		count_ar = 0;

		while (fgets(buff, sizeof(buff), fp_wl_r))
		{
			if (buff[0] == '*')
			{
				// 处理历元行
				buff[strcspn(buff,"\r\n")] = ' '; // 替换换行符
				//fputs(buff, fp_wl_w);
				fprintf(fp_wl_w, "%s", buff);

				// 读取并处理该历元的所有卫星对数据
				fix_ar = 0;

				long line_start_wl = 0;
				int hit_next_epoch_wl = 0;

				while (1)
				{
					line_start_wl = ftell(fp_wl_r);

					if (!fgets(buff, sizeof(buff), fp_wl_r))
					{
						break;
					}

					if (buff[0] == '*')
					{
						hit_next_epoch_wl = 1;
						break;
					}

					if (strlen(buff) > 48 && buff[48] == '1')
					{
						fix_ar++;
					}
				}

				if (hit_next_epoch_wl)
				{
					fseek(fp_wl_r, line_start_wl, SEEK_SET);
				}
				fprintf(fp_wl_w, "          %d\n", fix_ar);
				count_ar += fix_ar;
				n++;
			}
		}

		fprintf(fp_wl_w, "宽巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%.2f\n",
			n - 1, count_ar, n > 1 ? ((float)count_ar) / (n - 1) : 0);
		printf("Processed file: %s -> %s\n", sta_name_src, sta_name_des);
		//printf("宽巷提取完成\n");
		fclose(fp_wl_r);
		fclose(fp_wl_w);
	}

	/*nl*/
	printf("\nprocess nl file:\n");
	while ((entry_nl = readdir(dp_nl))!=NULL)
	{
		if (!(strncmp(entry_nl->d_name, ".", 1) && strncmp(entry_nl->d_name, "..", 1)))
			continue;
		if (strcmp(entry_nl->d_name, "results") == 0)
			continue;
		//构建每个要读取文件的输入路径
		snprintf(read_buff_nl, MAXNUM, "%s\\%s", INT_NL, entry_nl->d_name);
		//构建结果输出路径
		snprintf(write_buff_nl, MAXNUM, "%s\\results\\%s", INT_NL, entry_nl->d_name);
		strcat(write_buff_nl, ".arbnum");

		sta_name_src = strstr(read_buff_nl, entry_nl->d_name);
		sta_name_des = strstr(write_buff_nl, entry_nl->d_name);

		//打开文件
		FILE* fp_nl_r = fopen(read_buff_nl, "r");
		FILE* fp_nl_w = fopen(write_buff_nl, "w");
		if (!(fp_nl_r && fp_nl_w))//建议分开判断
		{
			perror("窄文件打开/创建失败：");
			//fclose(fp_nl_w);
			continue;
		}

		n = 1;
		fix_ar = 0;
		count_ar = 0;

		while (fgets(buff, sizeof(buff), fp_nl_r))
		{
			if (buff[0] == '*')
			{
				// 处理历元行
				buff[strcspn(buff,"\r\n")] = ' '; // 替换换行符
				//fputs(buff, fp_nl_w);
				fprintf(fp_nl_w, "%s", buff);

				// 读取并处理该历元的所有卫星对数据
				fix_ar = 0;

				long line_start_nl = 0;
				int hit_next_epoch_nl = 0;

				while (1)
				{
					line_start_nl = ftell(fp_nl_r);

					if (!fgets(buff, sizeof(buff), fp_nl_r))
					{
						break;
					}

					if (buff[0] == '*')
					{
						hit_next_epoch_nl = 1;
						break;
					}

					if (strlen(buff) > 48 && buff[48] == '1')
					{
						fix_ar++;
					}
				}

				if (hit_next_epoch_nl)
				{
					fseek(fp_nl_r, line_start_nl, SEEK_SET);
				}
				fprintf(fp_nl_w, "          %d\n", fix_ar);
				count_ar += fix_ar;
				n++;
			}
		}

		fprintf(fp_nl_w, "窄巷：总共处理了%d个历元，所有的历元总共固定了%d个模糊度，平均每个历元固定的模糊度数为%.2f\n",
			n - 1, count_ar, n > 1 ? ((float)count_ar) / (n - 1) : 0);
		printf("Processed file: %s -> %s\n", sta_name_src, sta_name_des);
		fclose(fp_nl_w);
		fclose(fp_nl_r);
	}
	
	closedir(dp_ewl);
	closedir(dp_wl);
	closedir(dp_nl);

	return 0;
}








