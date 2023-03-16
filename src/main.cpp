#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "../lib/SerialCOM/SerialCom.h"
#include "../lib/DataStruct/Structs.hpp"

using namespace std;
using namespace cv;

SerialPort serialPort;

double SerialPort::pitch_now = 0.0;
double SerialPort::yaw_now = 0.0;

// def functions
void PortThread();//串口通信
void UDPThread();//UDP通信


int main(int argc, char const *argv[])
{
    //参数提取颜色信息
    int color;
    if (argc == 1)
    {
        cout<<"请输入敌方装甲板颜色，0 == 红，1 == 蓝"<<endl;
        cin>>color;
    }
    else if (argc == 2)
    {
        if (int i = std::stoi(argv[1]);i == 0||i == 1)
        {
            cout<<"敌方装甲板颜色代号为"<<i<<endl;
            color = i;
        }
        else
        {
            cout<<"非法参数"<<endl;
            return 1;
        }
    }
    
    
    
}

void PortThread()
{
    if (serialPort.init("/dev/ttyUSB0", 115200))
        serialPort.runService();
}