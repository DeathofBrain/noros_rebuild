#pragma once
#include <opencv2/opencv.hpp>
#include "../DataStruct/Enums.hpp"
#include <deque>

// struct Points
// {
//     cv::Point2f center;      // 中心点
//     cv::Point2f vertices[4]; // 装甲板四个角
//     double distance;         // 距离
//     int number;              // 编号
// };

struct PicData
{
    cv::Mat src;                   // 相机获取到的图片
    cv::Mat binImg;                // 二值化图片
    long timestamp;                // 时间戳
    EnemyColor enemyColor;         // 敌方装甲板颜色
    int rgb_threshold;             // 图像通道相减的阈值
    int svm_threshold;             // SVM的阈值

    PicData() = default;

    PicData(cv::Mat src_, int color, int rgb_threshold_) : enemyColor((EnemyColor)color),
                                                           rgb_threshold(rgb_threshold_)
    {
        src = src_.clone();
    }

    ~PicData() = default;
};

/**
 * @brief 灯条结构体
 *
 */
struct LightBar
{
    cv::RotatedRect lightRect; // 最小外接旋转矩
    cv::Point2f center;        // 灯条中心点坐标
    float length;              // 灯条长度
    float broad;
    float area;
    float angle; // 灯条的旋转角度

    LightBar() = default;

    LightBar(const cv::RotatedRect &lightRect) : lightRect(lightRect)
    {
        length = MAX(lightRect.size.height, lightRect.size.width);
        broad = MIN(lightRect.size.height, lightRect.size.width);
        center = lightRect.center;
        area = length * broad;
        if (lightRect.angle > 90)
        {
            angle = lightRect.angle - 180;
        }
        else
        {
            angle = lightRect.angle;
        }
    }

    ~LightBar() = default;
};

/**
 * @brief 装甲板相关的各项参数
 *
 */
struct ArmorParam
{

    float min_area = 10.0F;            // 灯条允许的最小面积
    float max_angle = 45.0F;           // 灯条允许的最大偏角
    float max_angle_diff = 6.0F;       // 两个灯条之间允许的最大角度差
    float max_lengthDiff_ratio = 0.5F; // 两个灯条之间允许的最大长度差比值
    float max_deviation_angle = 50.0F; // 两灯条最大错位角
    float max_y_diff_ratio = 0.5F;     // max y
    float max_x_diff_ratio = 4.5F;     // max x
};