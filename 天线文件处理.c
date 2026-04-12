#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>

#define FIN_LINE 23107

int main(void)
{
	char buff[1024] = { 0 }; //用来存储读取进来的字符串
	int i = 1;
	FILE* p_r_atx = fopen("E:\\Program file\\rtklib_ppp\\2025300\\WHU_IGS\\igs20_2361\\igs20_2361.atx", "r");

	if (!p_r_atx)
	{
		perror("open flie false");

		return 1;
	}

	FILE* p_w_atx = fopen("E:\\Program file\\rtklib_ppp\\2025300\\WHU_IGS\\igs20_2361\\igs20_2361_copy.atx", "w");
	if (!p_w_atx)
	{
		perror("open flie false");

		return 1;
	}
	while (i <= FIN_LINE)
	{
		fgets(buff, sizeof(buff), p_r_atx);
		fputs(buff, p_w_atx);
		i++;
	}
	printf("复制完成\n");
	fclose(p_r_atx);
	fclose(p_w_atx);

	return 0;
}