#pragma once
#include <opencv2/opencv.hpp>
#include <DataStruct/Enums.hpp>
#include <deque>

struct Points
{
    cv::Point2f center;
    cv::Point2f vertices[4];
    double distance;
    int number;
};

struct PicData
{
    /**
     * @brief 相机获取到的图片
     * 
     */
    cv::Mat src;
    /**
     * @brief 二值化图片
     * 
     */
    cv::Mat binImg;
    /**
     * @brief 时间戳
     * 
     */
    long timestamp;
    /**
     * @brief 装甲打击点向量
     * 
     */
    std::deque<Points> armorPoint;
    /**
     * @brief 敌方装甲板颜色
     * 
     */
    EnemyColor enemyColor;
    /**
     * @brief 图像通道相减的阈值
     * 
     */
    int rgb_threshold;
    /**
     * @brief 删除无参构造，防止出错
     * 
     */
    PicData() = delete;

    PicData(cv::Mat src_, int color, int rgb_threshold_):
        enemyColor((EnemyColor)color),
        rgb_threshold(rgb_threshold_)
    {
        src = src_.clone();
    }

    ~PicData() = default;
};
/**
 * @brief 灯条类
 * 
 */
struct LightBar
{
    cv::RotatedRect lightRect;	        //最小外接旋转矩
	cv::Point2f center;		            //灯条中心点坐标
	float length;			            //灯条长度
	float broad;
	float area;
	float angle;			            //灯条的旋转角度

    LightBar() = default;

    LightBar(const cv::RotatedRect &lightRect):
        lightRect(lightRect)
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

