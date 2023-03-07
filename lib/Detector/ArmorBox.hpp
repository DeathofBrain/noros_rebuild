#pragma once
#include <opencv2/opencv.hpp>
#include <DataStruct/Structs.hpp>




class ArmorBox
{
public:
    LightBar L_;                                        //装甲板的左灯条
    LightBar R_;                                        //装甲板的右灯条
    int L_index;                                        //左灯条的下标
    int R_index;                                        //右灯条的下标
    int armorNum;                                       //装甲板上的数字
    cv::Point2f armorVertices[4];                       //装甲板的四个顶点 左下 左上 右上 右下
    ArmorType type;                                     //装甲板类型
    cv::Point2f center;                                 //装甲板中心点
    cv::Rect armorRect;                                 //装甲板的矩形获取roi用
    int armorArea;                                      //装甲板面积
    float armorAngle;                                   //装甲板（灯条）角度
    cv::Mat armorImg;                                   //装甲板的图片（透射变换获得）

public:
    /**
     * @brief 默认构造函数
     * 
     */
    ArmorBox()
    {
        L_index = -1;
        R_index = -1;
        L_ = LightBar();
        R_ = LightBar();
        armorNum = 0;
        type = SMALL_ARMOR;
        center = cv::Point2f();
        armorRect = cv::Rect();
        armorImg = cv::Mat();
    }

    


    ~ArmorBox() = default;

public:
    







};


