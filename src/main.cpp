#include <iostream>
#include <string>
#include <thread>
#include <opencv2/opencv.hpp>
#include "../lib/SerialCOM/SerialCom.h"
#include "../lib/DataStruct/Structs.hpp"

using namespace std;
using namespace cv;

SerialPort serialPort;

double SerialPort::pitch_now = 0.0;
double SerialPort::yaw_now = 0.0;

// def functions
void PortThread(); // 串口通信
void UDPThread();  // UDP通信

int main(int argc, char const *argv[])
{
    // 参数提取颜色信息
    int color;
    EnemyColor enemyColor;
    if (argc == 1)
    {
        cout << "请输入敌方装甲板颜色 0 == 红 1 == 蓝" << endl;
        cin >> color;
        if (color == 0 || color == 1)
        {
            enemyColor = (EnemyColor)color;
        }
        else
        {
            cout << "非法参数" << endl;
            return 1;
        }
    }
    else if (argc == 2)
    {
        if (int i = std::stoi(argv[1]); i == 0 || i == 1)
        {
            cout << "敌方装甲板颜色代号为" << i << endl;
            color = i;
            enemyColor = (EnemyColor)color;
        }
        else
        {
            cout << "非法参数" << endl;
            return 1;
        }
    }
    // 检测视频流
    int videoflag = 0;
    VideoCapture cap; // 读取视频流
    if (videoflag == 1)
    {
        cap.open("/home/neuqrm/NOROS/top.avi");
    }
    // 相机相关文件路径
    //const string camera_config_path = HIK_CONFIG_FILE_PATH"/camera_config.yaml";             // 相机配置文件路径
    //const string intrinsic_para_path = HIK_CALI_FILE_PATH"/caliResults/calibCameraData.yml"; // 相机内参文件路径
    //相机初始化
    //TODO
    //启动串口通信
    thread(PortThread).detach();
    //启动UDP通信
    //thread(UDPThread).detach();//TODO
    
}

void PortThread()
{
    if (serialPort.init("/dev/ttyUSB0", 115200))
        serialPort.runService();
}