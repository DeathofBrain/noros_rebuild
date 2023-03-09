#pragma once
#include <deque>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <DataStruct/Structs.hpp>
/**
 * @brief 图像预处理类
 * 
 */
class PreProcess
{
public:
    /**
     * @brief 装甲板图像结构体
     * 
     */
    PicData _src;
public:

    PreProcess() = default;

    PreProcess(PicData pic):_src(pic){};

    ~PreProcess() = default;
    /**
     * @brief 图像预处理部分：图像通道相减
     * 
     */
    void RGBImgProcess()
    {
        //1.通道分离并根据敌方颜色通道相减，得到灰度图
        cv::Mat grayImg = cv::Mat::zeros( _src.src.size(), CV_8UC1 );
        cv::Mat binImg = cv::Mat::zeros( _src.src.size(), CV_8UC1 );
        std::vector<cv::Mat> channels;
        cv::split(_src.src,channels);
        //2.图像二值化
        if(_src.enemyColor == RED)
        {
            grayImg = channels.at(1) - channels.at(0);
        }
        else
        {
            grayImg = channels.at(1) - channels.at(2);
        }
        cv::threshold(grayImg, _src.binImg, _src.rgb_threshold, 255, cv::THRESH_BINARY);
    }

    void OutSrc(PicData& out)
    {
        out = std::move(this->_src);
    }
};
