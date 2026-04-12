#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "dirent.h"

#define RE_WGS84  6378137.0             /* 地球长半轴 (WGS84) (m) */
#define FE_WGS84  (1.0 / 298.257223563) /* 地球扁率 (WGS84) */
#define PI        3.1415926535897932    /* pi */
#define DEG2RAD   0.017453292519943295  /*角度转弧度*/
#define RAD2DEG   57.29577951308232087  /*弧度转角度*/
#define SQR(x)      ((x)*(x))

#define INPUT_DIR "E:\\c_primer_plus_Study\\cStudy\\XYZ2ENU\\02_zhongqi\\2\\IFB2A"  // 文件夹路径
#define MAX_FILENAME_LEN 256

void xyztoenu(FILE* p, FILE* fp, FILE* fpstation,char* ID);
void xyztoblh(FILE* p, FILE* fp);
void blhtoxyz(FILE* p, FILE* fp);
void blhtoenu(FILE* p, FILE* fp);
void enutoxyz(FILE* p, FILE* fp);
void ecef2pos(const double* r, double* pos);
void pos2ecef(const double* pos, double* r);
void ecef2enu(const double* pos, const double* r, double* e);
void enu2ecef(const double* pos, const double* e, double* r);
void extract(char* outpstr, const int nPos, const int nCount, char* inpstr);


int main()
{
    struct dirent* entry;
    DIR* dp;
    dp = opendir(INPUT_DIR);
    char* ext;
    char fpRead[MAX_FILENAME_LEN]={ '\0' };
    char fpWrite[MAX_FILENAME_LEN]={ '\0' };

    FILE* fpstation = NULL;/*测站文件*/

    if (dp == NULL) {
        perror("opendir: Unable to open directory.");
        return 1;
    }

    while ((entry = readdir(dp)) != NULL) {
        // 查找后缀为 .POS 的文件
        if (strncmp(entry->d_name, ".", 1) == 0) continue;
        if (!(ext = strrchr(entry->d_name, '.'))) continue;
        if (strstr(entry->d_name, ".pos") != NULL) {
            char id[5] = "", ID[5] = "";
            strncpy(id, entry->d_name, 4);/*获取测站名*/
            for (int i = 0; id[i] != '\0'; i++) {
                if (islower(id[i]))
                    ID[i] = toupper(id[i]);  // 将字符转换为大写
                else ID[i] = id[i];
                //else strcpy(ID, id);
            }
            fpstation = fopen("E:\\c_primer_plus_Study\\cStudy\\XYZ2ENU\\02_zhongqi\\IGS0OPSSNX_20240600000_01D_01D_SOL.SNX", "r");
            if (fpstation == NULL) {
                perror("fopen: Unable to open input file.");
            }
            // 构建输入文件路径
            snprintf(fpRead, MAX_FILENAME_LEN, "%s\\%s", INPUT_DIR, entry->d_name);

            // 构建输出文件路径
            //strncpy(fpWrite, fpRead, strlen(fpRead) - 3);
            snprintf(fpWrite, MAX_FILENAME_LEN, "%s\\result\\%s", INPUT_DIR, entry->d_name);
            strcat(fpWrite, ".enu");

            // 打开输入文件
            FILE* input_file = fopen(fpRead, "r");
            if (input_file == NULL) {
                perror("fopen: Unable to open input file.");
                continue;
            }

            // 打开输出文件
            FILE* output_file = fopen(fpWrite, "w");
            if (output_file == NULL) {
                perror("fopen: Unable to open output file.");
                fclose(input_file);
                continue;
            }

            // 处理文件
            xyztoenu(input_file, output_file, fpstation,ID);

            // 关闭文件
            fclose(input_file);
            fclose(output_file);
            fclose(fpstation);
            printf("Processed file: %s -> %s\n", fpRead, fpWrite);
        }
    }
    closedir(dp);
    return 0;
    
    //FILE* fpRead = NULL; FILE* fpWrite = NULL; int k=0;
    ///* 输入文件*/
    //fpRead = fopen("E:\\c_primer_plus_Study\\cStudy\\XYZ2ENU\\04_24060\\FIN\\IISC_UC_GRB2.pos", "r");
    ///* 输出文件*/
    //fpWrite = fopen("E:\\c_primer_plus_Study\\cStudy\\XYZ2ENU\\04_24060\\FIN\\IISC_UC_GRB2T_ENU.txt", "a");
    //printf("Please enter the coordinate transformation you want to perform:\n");
    //printf("(xyztoenu:0 ; xyztoblh:1 ; blhtoxyz:2 ;blhtoenu:3 ;enutoxyz:4 )\n");
    //scanf("%d",&k);
    //switch(k)
    //{
    //case 0 :xyztoenu(fpRead, fpWrite); break;
    //case 1: xyztoblh(fpRead, fpWrite); break;
    //case 2: blhtoxyz(fpRead, fpWrite); break;
    //case 3: blhtoenu(fpRead, fpWrite); break;
    //case 4: enutoxyz(fpRead, fpWrite); break;
    //}
    //fclose(fpRead);    /*关闭文件*/
    //fclose(fpWrite);   /*关闭文件*/
    //printf("END!\n");
    //system("pause");   /*显示消息*/
    //return 0;
}

/* ecef转站心坐标系的变换矩阵
* 参数   : double *pos      I   大地坐标 {lat,lon} (rad)
*          double *E        O   Ecef到站心坐标的变换矩阵 */
void xyz2enu(const double* pos, double* E)
{
    double sinp = sin(pos[0]), cosp = cos(pos[0]), sinl = sin(pos[1]), cosl = cos(pos[1]);
    
    E[0] = -sinl;      E[3] = cosl;       E[6] = 0.0;
    E[1] = -sinp * cosl; E[4] = -sinp * sinl; E[7] = cosp;
    E[2] = cosp * cosl;  E[5] = cosp * sinl;  E[8] = sinp;
}

/* 矩阵乘法(C=alpha*A*B+beta*C)
* 参数   : char   *tr       I  矩阵转置标志 ("N":normal,"T":transpose)
*          int    n,k,m     I  矩阵 A,B的大小
*          double alpha     I  alpha
*          double *A,*B     I  (transposed) matrix A (n x m), B (m x k)
*          double beta      I  beta
*          double *C        IO matrix C (n x k)*/
void matmul(const char* tr, int n, int k, int m, double alpha, const double* A, const double* B, double beta, double* C)
{
    double d;
    int    i, j, x, f = tr[0] == 'N' ? (tr[1] == 'N' ? 1 : 2) : (tr[1] == 'N' ? 3 : 4);

    for (i = 0; i < n; i++)
        for (j = 0; j < k; j++)
        {
            d = 0.0;
            switch (f)
            {
            case 1:
                for (x = 0; x < m; x++)
                    d += A[i + x * n] * B[x + j * m];/*"NN"--1--A*B   */
                break;
            case 2:
                for (x = 0; x < m; x++)
                    d += A[i + x * n] * B[j + x * k];/*"NT"--2--A*BT  */
                break;
            case 3:
                for (x = 0; x < m; x++)
                    d += A[x + i * m] * B[x + j * m];/*"TN"--3--AT*B  */
                break;
            case 4:
                for (x = 0; x < m; x++)
                    d += A[x + i * m] * B[j + x * k];/*"TT"--4--AT*BT */
                break;
            }
            if (beta == 0.0) /*beta=0则C = alpha*d；beta!=0则有C = alpha*d + beta*C  */
                C[i + j * n] = alpha * d;
            else
                C[i + j * n] = alpha * d + beta * C[i + j * n];
        }
}

/* ecef坐标转站心坐标
* 参数   : double *pos      I   大地坐标 {lat,lon} (rad)
*          double *r        I   ecef坐标向量 {x,y,z}
*          double *e        O   站心坐标 {e,n,u}*/
void ecef2enu(const double* pos, const double* r, double* e)
{
    double E[9];

    xyz2enu(pos, E);
    matmul("NN", 3, 1, 3, 1.0, E, r, 0.0, e);
}

/* 向量内积
* 参数   : double *a,*b     I   vector a,b (n x 1)
*          int    n         I   size of vector a,b
* return : a'*b  */
double dot(const double* a, const double* b, int n)
{
    double c = 0.0;

    while (--n >= 0)
        c += a[n] * b[n];
    return c;
}

/* ecef坐标转为大地坐标
* 参数   : double *r        I   ecef坐标 {x,y,z} (m)
*          double *pos      O   大地坐标 {lat,lon,h} (rad,m)*/
void ecef2pos(const double* r, double* pos)
{
    double e2 = FE_WGS84 * (2.0 - FE_WGS84), r2 = dot(r, r, 2), z, zk, v = RE_WGS84, sinp;

    for (z = r[2], zk = 0.0; fabs(z - zk) >= 1E-4;)
    {
        zk = z;/*假设点到地心所在水平线的距离和法线与旋转轴交点所在水平面的距离相等*/
        sinp = z / sqrt(r2 + z * z);/* sinB=z/sqrt(r2 + z * z) 计算纬度B，其中sqrt(r2 + z * z)为点到球心的距离*/
        v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);/*相当于N，N=v=a/W,W=sqrt(1-e^2*sinB*sinB)*/
        z = r[2] + v * e2 * sinp;/*计算点到法线与旋转轴交点所在水平面的距离*/
    }
    pos[0] = atan(z / sqrt(r2));
    pos[1] = atan2(r[1], r[0]) ;
    pos[2] = sqrt(r2 + z * z) - v;
}

/* 大地坐标转ecef
* 参数   : double *pos      I   大地坐标 {lat,lon,h} (rad,m)
*          double *r        O   ecef坐标 {x,y,z} (m)*/
void pos2ecef(const double* pos, double* r)
{
    double sinp = sin(pos[0]), cosp = cos(pos[0]), sinl = sin(pos[1]), cosl = cos(pos[1]);
    double e2 = FE_WGS84 * (2.0 - FE_WGS84), v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);
    /*e2为椭球偏心率的平方 v（N）为椭球曲率半径*/
    r[0] = (v + pos[2]) * cosp * cosl;        /*x=(N+h)cos(lat)cos(lon)*/
    r[1] = (v + pos[2]) * cosp * sinl;        /*y=(N+h)cos(lat)sin(lon)*/
    r[2] = (v * (1.0 - e2) + pos[2]) * sinp;  /*z=(N(1-e^2)+h)sin(lat)*/
}

/* 站心坐标转ecef
* 参数   : double *pos      I   大地坐标 {lat,lon} (rad)
*          double *r        I   ecef坐标向量 {x,y,z}
*          double *e        O   站心坐标 {e,n,u}*/
void enu2ecef(const double* pos, const double* e, double* r)
{
    double E[9];

    xyz2enu(pos, E);
    matmul("TN", 3, 1, 3, 1.0, E, e, 0.0, r);
}


/* 从字符串提取坐标信息
* 参数   : char* outpstr       O   output string
*          int   nPos          I   起始位置
*          int   nCount        I   提取的字符串长度
*          char* inpstr        I   input string */
void extract(char* outpstr, const int nPos, const int nCount, char* inpstr)
{
    int i, n;
    char* str, c;

    n = strlen(inpstr);
    if (n <= 0) return;

    str = inpstr + nPos;/*起始位置*/
    for (i = 0; i < nCount; i++) {
        c = *(str + i);
        // *(outpstr + i) = c;
        if (c) {
            *(outpstr + i) = c;/*输出读取的字符*/
        }
        else {
            *(outpstr + i) = '\0';
            break;
        }
    }
    *(outpstr + nCount) = '\0';
}


void xyztoenu(FILE* p,FILE* fp,FILE* fpstation,char* ID) 
{
    
    double pos[3] = { '\0' }, r[3] = { '\0' }, e[3] = { '\0' };
    double xyz[3] = { '\0' };
    double x = 0, y = 0, z = 0;
    char line[1024] = { 0 };
    while (!feof(fpstation)) {
        fgets(line, 1024, fpstation);   //读取一行
        if (!strncmp(line, "+SOLUTION/ESTIMATE", 17)) {
            while (fgets(line, 1024, fpstation)) {//读取一行
                if (!strncmp(line + 14, ID, 4)) {/*匹配测站名*/
                    if ((!strncmp(line + 7, "STAX", 4))) xyz[0] = atof(line + 47);
                    if ((!strncmp(line + 7, "STAY", 4))) xyz[1] = atof(line + 47);
                    if ((!strncmp(line + 7, "STAZ", 4))) xyz[2] = atof(line + 47);
                }
                if (!strncmp(line, "-SOLUTION/ESTIMATE", 17)) continue;
            }   

        }
        else continue;
    }

    if (xyz[0] == 0 || xyz[1] == 0 || xyz[2] == 0) {
        printf("Please enter the %4s station coordinates :\n",ID);
        scanf("%lf %lf %lf", &x, &y, &z);
        //printf("Please enter X coordinates :\n");
        //scanf("%f", &x);
        //printf("Please enter Y coordinates :\n");
        //scanf("%f", &y);
        //printf("Please enter Z coordinates :\n");
        //scanf("%f", &z);
        if ((xyz[0]=x) == 0 || (xyz[1]=y) == 0 || (xyz[2]=z) == 0)
            return 0;
    }
    //xyz[0] = -3857171.2191, xyz[1] = 3108692.7777, xyz[2] = 4004040.1090;  //原点xyz坐标
    //double xyz[3] = { 1337935.6256,6070317.1330,1427877.4403 };
    /*测试数据原点-2179123.5109  4399687.6767  4057616.3710 */
    ecef2pos(xyz, pos);                                                    //xyz转blh

    char StrLine[1024] = { 0 };   //每行最大读取的字符数
    double ecefXYZ[3] = { 0 }, rms[3] = { 0 }, fixedrate = 0.0, pdop[3] = { 0 }, epoch_pdop = 0.0, rms_total[3] = {0};
    char time[15] = {0},cover_time[15] = { 0 };
    int stat = 0, i = 0, total = 0, nb = 0, fixnb = 0, sat=0,satnb[3] = { 0 },fix_nb=0;
    double fixrms[3] = { 0 };

    while (!feof(p))
    {
        r[0] = 0, r[1] = 0, r[2] = 0;
        e[0] = 0, e[1] = 0, e[2] = 0;
        fgets(StrLine, 1024, p);   //读取一行
        if (!strncmp(StrLine, "%",1))continue;
        char temp[60] = { 0 };
        total += 1;/*total number of epoch*/
        extract(time, 11, 12, StrLine);

        /*从每行字符串中读取XYZ*/
        extract(temp, 25, 13, StrLine);
        ecefXYZ[0] = atof(temp);
        extract(temp, 40, 13, StrLine);
        ecefXYZ[1] = atof(temp);
        extract(temp, 55, 13, StrLine);
        ecefXYZ[2] = atof(temp);
        extract(temp, 69,  3, StrLine);/*read stat*/
        stat = atoi(temp);
        extract(temp, 73,  3, StrLine);/*read number of sat*/
        sat = atoi(temp);
        extract(temp, 77, 4, StrLine);/*read pdop*/
        epoch_pdop = atof(temp);
        /*统计卫星数*/
        satnb[2] += sat;/*总卫星数*/
        pdop[2] += epoch_pdop;
        if (total == 1) {
            satnb[0] = satnb[1] = sat;
            pdop[0] = pdop[1] = epoch_pdop;
        }
        if (satnb[0] < sat)/*MAX SAT*/
            satnb[0] = sat;
        if (satnb[1] > sat)/*MIN SAT*/
            satnb[1] = sat;
        if (pdop[0] < epoch_pdop)/*MAX PDOP*/
            pdop[0] = epoch_pdop;
        if (pdop[1] > epoch_pdop)/*MIN PDOP*/
            pdop[1] = epoch_pdop;

        r[0] = xyz[0] - ecefXYZ[0];
        r[1] = xyz[1] - ecefXYZ[1];
        r[2] = xyz[2] - ecefXYZ[2];

        ecef2enu(pos, r, e);/*get enu*/

        if (nb <= 20) {/*连续20历元(10min)收敛则认为开始收敛*/
            if (fabs(e[0]) < 0.1 && fabs(e[1]) < 0.1 && fabs(e[2]) < 0.1) {
                nb++; 
                rms[0] += SQR(e[0]);
                rms[1] += SQR(e[1]);
                rms[2] += SQR(e[2]);
                if(nb==1)strcpy(cover_time , time);/*记录首历元时间*/
            }
            else {
                nb=0;
                rms[0] =0;
                rms[1] = 0;
                rms[2] = 0;
            }
        }
        else {
            nb++;
            rms[0] += SQR(e[0]);
            rms[1] += SQR(e[1]);
            rms[2] += SQR(e[2]);
        }

        for (i = 0; i < 3; i++) {
            rms_total[i] += SQR(e[i]);
        }

        if (nb !=0&&stat==1) {/*fixed number*/
            fixnb ++;
        }
        else if(nb==0)fixnb=0;

        if ((fabs(e[0]) < 0.1 && fabs(e[1]) < 0.1 && fabs(e[2]) < 0.1)) {/*stat == 1 && 固定历元RMS,&&(fabs(e[0]) < 0.1 && fabs(e[1]) < 0.1 && fabs(e[2]) < 0.1)*/
            fix_nb++;
            fixrms[0] += SQR(e[0]);
            fixrms[1] += SQR(e[1]);
            fixrms[2] += SQR(e[2]);
        }

        //fprintf(fp, "%8.4f %8.4f %8.4f\n", e[0], e[1], e[2]);
        fprintf(fp, "%s %8.4f %8.4f %8.4f %2d\n", time, e[0], e[1], e[2],stat);
        memset(temp, 0, sizeof temp);//清空数组
        memset(time, 0, sizeof time);
    }
    for (i = 0; i < 3; i++) {
        rms[i] = sqrt(rms[i]/nb);
        rms_total[i] = sqrt(rms_total[i] / total);
        fixrms[i]= sqrt(fixrms[i] / fix_nb);
    }
    fixedrate = ((double)fixnb / total)*100;/*rate of ambiguity successful fixed*/
    double avgsat= (double)satnb[2] / total;/*average sat number*/
    pdop[2] = pdop[2] / total;
    fprintf(fp, "RMS: %2.4f %2.4f %2.4f; fixed_rate=%3.2f%; coverage_time:%s\n", rms[0], rms[1], rms[2],fixedrate, cover_time);
    fprintf(fp, "PLANE RMS: %2.4f, POINT RMS:%2.4f; \n", sqrt(SQR(rms[0]) + SQR(rms[1])), sqrt(SQR(rms[0]) + SQR(rms[1]) + SQR(rms[2])));
    fprintf(fp, "MAXSAT : %2d; MINSAT : %2d; avgSAT :%2f\n", satnb[0], satnb[1], avgsat);
    fprintf(fp, "MAXPDOP: %2.2f; MINPDOP: %2.2f; avgPDOP:%2.2f\n", pdop[0], pdop[1], pdop[2]);
    fprintf(fp, "TOTAL RMS: %2.4f %2.4f %2.4f; \n", rms_total[0], rms_total[1], rms_total[2]);
    fprintf(fp, "PLANE RMS: %2.4f, POINT RMS:%2.4f; \n", sqrt(SQR(rms_total[0])+SQR(rms_total[1])), sqrt(SQR(rms_total[0]) + SQR(rms_total[1])+SQR(rms_total[2])));
    fprintf(fp, "FIX_RMS: %2.4f %2.4f %2.4f;\n", fixrms[0], fixrms[1], fixrms[2]);

    memset(pos, 0, sizeof pos);
    memset(r, 0, sizeof r);
    memset(e, 0, sizeof e);
    memset(xyz, 0, sizeof xyz);
    memset(rms, 0, sizeof rms);
    memset(satnb, 0, sizeof satnb);
}

void xyztoblh(FILE* p, FILE* fp)
{
    double pos[3] = { '\0' };

    char StrLine[1024] = { 0 };

    double ecefXYZ[3] = { 0 };

    while (!feof(p))
    {
        fgets(StrLine, 1024, p);   //读取一行
        char temp[60] = { 0 };

        /*从每行字符串中读取XYZ*/
        extract(temp, 25, 13, StrLine);
        ecefXYZ[0] = atof(temp);
        extract(temp, 41, 12, StrLine);
        ecefXYZ[1] = atof(temp);
        extract(temp, 56, 12, StrLine);
        ecefXYZ[2] = atof(temp);

        ecef2pos(ecefXYZ, pos);

        fprintf(fp, "%.9f %.9f %.9f\n", pos[0] * RAD2DEG, pos[1] * RAD2DEG, pos[2]);
        memset(temp, 0, sizeof temp);

    }
    memset(pos, 0, sizeof pos);

}

void blhtoxyz(FILE* p, FILE* fp) 
{
    double xyz[3] = { '\0' };
    double degBLH[3] = { 0 };
    double radBLH[3] = { 0 };

    char StrLine[1024] = { 0 };

    while (!feof(p))
    {
        fgets(StrLine, 1024, p);   //读取一行
        char temp[60] = { 0 };

        
        //extract(temp, 25, 13, StrLine);
        //degBLH[0] = atof(temp);
        //radBLH[0] = degBLH[0] * DEG2RAD;
        //extract(temp, 41, 12, StrLine);
        //degBLH[1] = atof(temp);
        //radBLH[1] = degBLH[1] * DEG2RAD;
        //extract(temp, 56, 12, StrLine);
        //degBLH[2] = atof(temp);
        //radBLH[2] = degBLH[2];
        extract(temp, 0, 12, StrLine);
        degBLH[0] = atof(temp);
        radBLH[0] = degBLH[0] * DEG2RAD;
        extract(temp, 13, 13, StrLine);
        degBLH[1] = atof(temp);
        radBLH[1] = degBLH[1] * DEG2RAD;
        extract(temp, 27, 13, StrLine);
        degBLH[2] = atof(temp);
        radBLH[2] = degBLH[2];

        pos2ecef(radBLH, xyz);

        fprintf(fp, "%.4f %.4f %.4f\n", xyz[0] , xyz[1], xyz[2]);
        memset(temp, 0, sizeof temp);

    }
    memset(xyz, 0, sizeof xyz);
}

void blhtoenu(FILE* p, FILE* fp) 
{
    double xyz_blh[3] = { 0 }, degBLH[3] = { 0 }, radBLH[3] = { 0 }, ecefXYZ[3] = { 0 };
    double xyz[3] = { 0 }, r[3] = { 0 }, blh[3] = { 0 }, e[3] = { 0 }, rad_blh[3] = {0};

    blh[0] = 39.760696136; blh[1] = 116.348777224; blh[2] = 32.152907598;/*输入站心坐标系原点*/

    rad_blh[0] = blh[0] * DEG2RAD;
    rad_blh[1] = blh[1] * DEG2RAD;
    rad_blh[2] = blh[2];

    char StrLine[1024] = { 0 };

    while (!feof(p))
    {
        fgets(StrLine, 1024, p);   //读取一行
        char temp[60] = { 0 };

        extract(temp, 0, 12, StrLine);
        degBLH[0] = atof(temp);
        radBLH[0] = degBLH[0] * DEG2RAD;
        extract(temp, 13, 13, StrLine);
        degBLH[1] = atof(temp);
        radBLH[1] = degBLH[1] * DEG2RAD;
        extract(temp, 27, 12, StrLine);
        degBLH[2] = atof(temp);
        radBLH[2] = degBLH[2];

        pos2ecef(radBLH, xyz_blh); /*读取的blh坐标转xyz*/
        pos2ecef(rad_blh, xyz);    /*参考点坐标blh转xyz*/

        r[0] = xyz[0] - xyz_blh[0];
        r[1] = xyz[1] - xyz_blh[1];
        r[2] = xyz[2] - xyz_blh[2];


        ecef2enu(rad_blh, r, e);
        fprintf(fp, "%.4f %.4f %.4f\n", e[0], e[1], e[2]);
        memset(temp, 0, sizeof temp);

    }
    memset(xyz, 0, sizeof xyz);
    memset(r, 0, sizeof r);
    memset(blh, 0, sizeof blh);
    memset(e, 0, sizeof e);
    
}

void enutoxyz(FILE* p, FILE* fp) 
{
    double xyz[3] = { '\0' }, xyz0[3] = { 0 }, pos[3] = { '\0' };
    double enu[3] = { 0 }, dxyz[3] = { 0 };

    xyz0[0] = -2179123.5109, xyz0[1] = 4399687.6767, xyz0[2] = 4057616.3710;  //起始点xyz坐标

    ecef2pos(xyz0,pos);/**/

    char StrLine[1024] = { 0 };

    while (!feof(p))
    {
        fgets(StrLine, 1024, p);
        char temp[60] = { 0 };

        extract(temp, 0, 8, StrLine);
        enu[0] = atof(temp);
        extract(temp, 10, 7, StrLine);
        enu[1] = atof(temp);
        extract(temp, 19, 7, StrLine);
        enu[2] = atof(temp);

        enu2ecef(pos,enu,dxyz);
        xyz[0] = xyz0[0] - dxyz[0];
        xyz[1] = xyz0[1] - dxyz[1];
        xyz[2] = xyz0[2] - dxyz[2];

        fprintf(fp, "%.4f %.4f %.4f\n", xyz[0], xyz[1], xyz[2]);
        memset(temp, 0, sizeof temp);

    }
    memset(xyz, 0, sizeof xyz);
    memset(enu, 0, sizeof enu);
    memset(pos, 0, sizeof pos);

}

