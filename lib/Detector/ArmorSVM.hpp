#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include "../Detector/ArmorBox.hpp"

class ArmorSVM
{
public:
    cv::Ptr<cv::ml::SVM> svm;    // svm模型
    cv::Mat p;                   // 载入到SVM中识别的矩阵
    cv::Size armorImgSize;       // SVM模型的识别图片大小（训练集的图片大小）
    cv::Mat warpPerspective_src; // 透射变换的原图
    cv::Mat warpPerspective_dst; // 透射变换生成的目标图
    cv::Mat warpPerspective_mat; // 透射变换的变换矩阵
    cv::Point2f srcPoints[4];    // 透射变换的原图上的目标点 tl->tr->br->bl  左上 右上 右下 左下
    cv::Point2f dstPoints[4];    // 透射变换的目标图中的点   tl->tr->br->bl  左上 右上 右下 左下
public:
    //初始化SVM
    ArmorSVM()
    {
        svm = cv::ml::StatModel::load<cv::ml::SVM>(NOROS_CONFIGS"/svm0528.xml");
        if (svm.empty())
        {
            std::cout << "Svm load error! Please check the path!" << std::endl;
            exit(0);
        }
        armorImgSize = cv::Size(40, 40);
        p = cv::Mat();
        warpPerspective_mat = cv::Mat(3, 3, CV_32FC1);
        dstPoints[0] = cv::Point2f(0, 0);
        dstPoints[1] = cv::Point2f(armorImgSize.width, 0);
        dstPoints[2] = cv::Point2f(armorImgSize.width, armorImgSize.height);
        dstPoints[3] = cv::Point2f(0, armorImgSize.height);
    }
    ~ArmorSVM() = default;

public:
    /**
     * @brief 加载透射变换的原图
     *
     * @param srcImg 原图
     */
    void loadImg(cv::Mat &srcImg)
    {
        srcImg.copyTo(warpPerspective_src);
        cv::cvtColor(warpPerspective_src, warpPerspective_src, 6);
        cv::threshold(warpPerspective_src, warpPerspective_src, 20, 255, cv::ThresholdTypes::THRESH_BINARY);
    }
    /**
     * @brief 获取装甲图片
     *
     * @param armor 装甲板对象
     */
    void getArmorImg(ArmorBox &armor)
    {
        // 设置装甲板四个角
        for (size_t i = 0; i < 4; i++)
        {
            srcPoints[i] = armor.armorVertices[i];
        }
        // 通过透射变换获得装甲图片
        warpPerspective_mat = cv::getPerspectiveTransform(srcPoints, dstPoints);                                                                                                              // 得到透射变换矩阵
        warpPerspective(warpPerspective_src, warpPerspective_dst, warpPerspective_mat, armorImgSize, cv::InterpolationFlags::INTER_NEAREST, cv::BorderTypes::BORDER_CONSTANT, cv::Scalar(0)); // 透射变换获得装甲图片
        warpPerspective_dst.copyTo(armor.armorImg);                                                                                                                                           // 将获得的图片复制到armor.armorImg
        // cv::imshow("armor.armoeImg",armor.armorImg);
    }
    /**
     * @brief 通过SVM获取装甲板编号
     *
     * @param armor 装甲板类型对象
     */
    void getArmorNum(ArmorBox &armor)
    {
        // 使armorImg适应SVM模型样本量要求
        p = armor.armorImg.reshape(1, 1);
        p.convertTo(p, CV_32FC1);
        // 根据SVM的结果设置装甲数
        armor.armorNum = (int)svm->predict(p);
        // cout<<"ArmorNum:"<<armor.armorNum<<endl;
    }
};