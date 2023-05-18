#define _CRT_SECURE_NO_WARNINGS
#include "stdio.h"
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;
int T;
int cols = 320, rows = 240;
int k = 0;
int T_Sum = 0;
bool flag0;
int flag2 = -1,flag3,flag4;
int Optimized_a = 0, Optimized_b = 0;
int Optimized_m = 0, Optimized_n = 0;
string Img_Name;
//---------------------------------------------形态学特征值
struct ML_Inf
{
    double Axis_Ratio;//轴比
    double Area;//面积
    int Tag;//2表示螺钉,0表示垫片,1表示螺帽
};
ML_Inf ML_Information[500];
ML_Inf ML_P_Information[10];
Mat Read_Img = Mat(rows, cols, CV_8UC1);
Mat Bi_Img = Mat(rows, cols, CV_8UC1);
Mat Bl_Img = Mat(rows, cols, CV_8UC1);
Mat O_Img = Mat::zeros(rows, cols, CV_8UC3);

void Scan();
void Grow(int i, int j);
void Create_Information(bool flag);
void Train_Grow(int x, int y, Mat Model, int channel);

void ML_Process(string Category, int Low_value, int Up_value)
{
    for (int n = Low_value; n < Up_value + 1; n++)
    {
        if (Category == "Unknow") {}
        else if (Category == "Nut") {
            Img_Name = "tuerca_";
            flag3 = 1;
        }
        else if (Category == "Ring") {
            Img_Name = "arandela_";
            flag3 = 0;
        }
        else if (Category == "Screw") {
            Img_Name = "tornillo_";
            flag3 = 2;
        }
        if (n < 10)
        {
            Img_Name = "data/" + Category + "/" + Img_Name + "000" + to_string(n) + ".pgm";
        }
        else if (n >= 10 && n < 100)
            Img_Name = "data/" + Category + "/" + Img_Name + "00" + to_string(n) + ".pgm";
        else if (n >= 100 && n < 1000)
            Img_Name = "data/" + Category + "/" + Img_Name + "0" + to_string(n) + ".pgm";

        if (imread(Img_Name, 0).empty())printf("empty!\n");
        else
        {
            Read_Img = imread("data/pattern.pgm", 0) - imread(Img_Name, 0);
        }
        threshold(Read_Img, Bi_Img, 50, 255, 0);
        blur(Bi_Img, Bl_Img, { 5,5 });
        threshold(Bl_Img, Bi_Img, 50, 255, 0);

        flag0 = 0;
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if ((Bi_Img.at<uchar>(i, j) == 255) && (i == 0 || i == rows - 1 || j == 0 || j == cols - 1))flag0 = 1;
            }
        }
        if (flag0 == 1) {}//前景处于边界,不做为样本
        else//前景不处于边界,可做为样本
        {  
            int q = 0;
            k = 0;
            Scan();//扫描连通区域,标记为31,32,33....
            T_Sum += k;
            Create_Information(1);
        }
    }
    T_Sum --;
}
void Scan()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (Bi_Img.at<uchar>(i, j) == 255)
            {
                k++;
                Grow(i, j);
            }
            else
            {
                Bi_Img.at<uchar>(i, j) == 0;
            }
        }
    }
}
void Grow(int i, int j)
{
    Bi_Img.at<uchar>(i, j) = 30 + k;
    if (Bi_Img.at<uchar>(i + 1, j) == 255)Grow(i + 1, j);
    if (Bi_Img.at<uchar>(i - 1, j) == 255)Grow(i - 1, j);
    if (Bi_Img.at<uchar>(i, j - 1) == 255)Grow(i, j - 1);
    if (Bi_Img.at<uchar>(i, j + 1) == 255)Grow(i, j + 1);
}
void Create_Information(bool flag)
{
    for (int p = 1; p < k + 1; p++)
    {
        int sum_x = 0, sum_y = 0, sum_point = 0;
        double length = 0;
        double Axis_Max = 0, Axis_Min = 1000;
        double center_x = 0, center_y = 0;

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (Bi_Img.at<uchar>(i, j) == 30 + p)
                {
                    sum_x += i;
                    sum_y += j;
                    sum_point++;
                }
            }
        }
        if (sum_point != 0)
        {
            center_x = sum_x / sum_point;
            center_y = sum_y / sum_point;
        }
        else printf("!!ERROR!!");
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (Bi_Img.at<uchar>(i, j) == 30 + p)
                {
                    if ((i == 0 || i == rows - 1 || j == 0 || j == cols - 1))
                    {
                        length = sqrt((center_x - i) * (center_x - i) + (center_y - j) * (center_y - j));
                        if (length > Axis_Max)Axis_Max = length;
                        if (length < Axis_Min)Axis_Min = length;
                    }
                    else if (Bi_Img.at<uchar>(i - 1, j) == 0 || Bi_Img.at<uchar>(i + 1, j) == 0 || Bi_Img.at<uchar>(i, j - 1) == 0 || Bi_Img.at<uchar>(i, j + 1) == 0)
                    {
                        length = sqrt((center_x - i) * (center_x - i) + (center_y - j) * (center_y - j));
                        if (length > Axis_Max)Axis_Max = length;
                        if (length < Axis_Min)Axis_Min = length;
                    }
                }
            }
        }
        if (flag)
        {
            ML_Information[flag2].Tag = flag3;
            ML_Information[flag2].Axis_Ratio = 20 * (Axis_Max / Axis_Min);
            ML_Information[flag2].Area = sum_point / 26;
            flag2++;
        }
        else
        {
            ML_P_Information[flag4].Tag = 10;
            ML_P_Information[flag4].Axis_Ratio = 20 * (Axis_Max / Axis_Min);
            ML_P_Information[flag4].Area = sum_point / 26;
            flag4++;
        }
    }
}
void Train_Model()
{
    Mat Model = Mat::zeros(600, 600, CV_8UC3);
    for (int i = 0; i < T_Sum; i++)
    {
        int x = 600 - 5 * (int)(ML_Information[i].Area) + 50;
        int y = 600 - (int)(ML_Information[i].Axis_Ratio * 5);
        int channel = ML_Information[i].Tag;
        Model.at<Vec3b>(x,y)[channel] = 255;
        Train_Grow(x, y, Model,channel);
    }
    Mat Model_Binary = Mat::zeros(600, 600, CV_8UC3);
    for (int i = 0; i < 600; i++)
    {
        for (int j = 0; j < 600; j++)
        {
            if (Model.at<Vec3b>(i,j)[0]>= Model.at<Vec3b>(i, j)[1]&& Model.at<Vec3b>(i, j)[0]>= Model.at<Vec3b>(i, j)[2])
                Model_Binary.at<Vec3b>(i, j)[0] = 255;
            if (Model.at<Vec3b>(i, j)[1] >= Model.at<Vec3b>(i, j)[0] && Model.at<Vec3b>(i, j)[1] >= Model.at<Vec3b>(i, j)[2])
                Model_Binary.at<Vec3b>(i, j)[1] = 255;
            if (Model.at<Vec3b>(i, j)[2] >= Model.at<Vec3b>(i, j)[1] && Model.at<Vec3b>(i, j)[2] >= Model.at<Vec3b>(i, j)[0])
                Model_Binary.at<Vec3b>(i, j)[2] = 255;
        }
    }
    imshow("快速模型", Model_Binary);
    imshow("精确模型",Model);
    waitKey(0);
    imwrite("Model_Binary.jpg",Model_Binary);
    imwrite("Model.jpg",Model);
}
void Draw_()
{
    Mat Draw_Img = Mat::zeros(600, 600, CV_8UC3);
    for (int i = 0; i < T_Sum; i++)
    {
        int x = 600 - 5 * (int)(ML_Information[i].Area) + 50;
        int y = 600 - (int)(ML_Information[i].Axis_Ratio * 5);
        int channel = ML_Information[i].Tag;
        Draw_Img.at<Vec3b>(x, y)[channel] = 255;
    }
    imshow("特征二维分布", Draw_Img);
    waitKey(0);
}
void Train_Grow(int x, int y, Mat Model,int channel)
{
    for (int i = 0; i < 600; i++)
    {
        for (int j = 0; j < 600; j++)
        {
            int Half_Len = sqrt((x - i) * (x - i) + (y - j) * (y - j))/2;
            if (Model.at<Vec3b>(i, j)[channel] < 255 - Half_Len)
            {
                Model.at<Vec3b>(i, j)[channel] = 255 - Half_Len;
            }
        }
    }
}
void Predict_ImgModel(string Test_Path)
{
    Mat P_Model = imread("Model_Binary.jpg");
    Mat L_Img = P_Model.clone();
    Read_Img = imread("data/pattern.pgm", 0) - imread(Test_Path, 0);
    O_Img = imread(Test_Path);
    threshold(Read_Img, Bi_Img, 50, 255, 0);
    blur(Bi_Img, Bl_Img, { 5,5 });
    threshold(Bl_Img, Bi_Img, 50, 255, 0);
    k = 0;
    Scan();
    flag4 = 0;
    Create_Information(0);
    for (int i = 0; i < flag4; i++)
    {
        int x = 600 - 5 * (ML_P_Information[i].Area) + 50;
        int y = 600 - (ML_P_Information[i].Axis_Ratio * 5);
        L_Img.at<Vec3b>(x, y) = { 0,0,0 };
        L_Img.at<Vec3b>(x, y-1) = { 0,0,0 };
        L_Img.at<Vec3b>(x+1, y) = { 0,0,0 };
        L_Img.at<Vec3b>(x, y+1) = { 0,0,0 };
        L_Img.at<Vec3b>(x-1, y) = { 0,0,0 };

        if (P_Model.at<Vec3b>(x, y)[0] > 200)
            ML_P_Information[i].Tag = 0;
        else if (P_Model.at<Vec3b>(x, y)[1] > 200)
            ML_P_Information[i].Tag = 1;
        else ML_P_Information[i].Tag = 2;
    }
    for (int i = 0; i < flag4; i++)
    {
        for (int m = 0; m < rows; m++)
        {
            for (int n = 0; n < cols; n++)
            {
                if (Bi_Img.at<uchar>(m, n) == i + 31)
                {
                    O_Img.at<Vec3b>(m, n)[ML_P_Information[i].Tag] = 255;
                }
            }
        }
    }
    printf("----螺钉被标记为红色,螺帽被标记为绿色,垫片被标记为蓝色----\n\n");
    imshow("预测结果", O_Img);
    imshow("点分布", L_Img);
    waitKey(0);
}
void Reset()
{
    T_Sum = 0;
    flag2 = 0;
    for (int i = 0; i < 500; i++)
    {
        ML_Information[i].Area = -1;
        ML_Information[i].Axis_Ratio = -1;
        ML_Information[i].Tag = -1;
    }
}
void Test_Per_Process(string Category,int Low_value,int Up_value)
{
    ML_Process(Category, Low_value, Up_value);
}
void Test(string ModelPath)
{
    float Error_Rate = T_Sum - 1;
    Mat Test_Model = imread(ModelPath);
    for (int i = 0; i < T_Sum - 1; i++)
    {
        int Current_Tag = -1;
        int x = 600 - 5 * (ML_Information[i].Area) + 50;
        int y = 600 - (ML_Information[i].Axis_Ratio * 5);
        if (x > 650 || x < 0 || y>650 || y < 0);
        else
        {
            if (Test_Model.at<Vec3b>(x, y)[0] > 200)
                Current_Tag = 0;
            else if (Test_Model.at<Vec3b>(x, y)[1] > 200)
                Current_Tag = 1;
            else Current_Tag = 2;
            if ((Current_Tag != -1) && (ML_Information[i].Tag != -1))
            {
                if (ML_Information[i].Tag == Current_Tag)
                    Error_Rate--;
            }
        }
    }
    Error_Rate /= T_Sum;
    printf("错误率为:%f\n",Error_Rate/100);
}
void main()
{
	if(1)
	{
		ML_Process("Nut",0,30);
		ML_Process("Ring",0,20);
		ML_Process("Screw",0,30);
        Draw_();
        if (1)Train_Model();
	}
    if (1)
    {
        Reset();
        Test_Per_Process("Nut",31,163);
        printf("Nut");
        Test("Model_Binary.jpg");
        Reset();
        Test_Per_Process("Ring",21,100);
        printf("Ring");
        Test("Model_Binary.jpg");
        Reset();
        Test_Per_Process("Screw",31,142);
        printf("Screw");
        Test("Model_Binary.jpg");
    }
	if (1)
	{
        Predict_ImgModel("data/test2.pgm");
        Predict_ImgModel("data/test.pgm");
	}
}
