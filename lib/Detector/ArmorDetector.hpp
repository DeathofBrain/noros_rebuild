#pragma once
#include <opencv2/opencv.hpp>
#include <Detector/ArmorBox.hpp>

/**
 * @brief 装甲检测类
 *
 */
class ArmorDetector
{
public:
    PicData src_;                          // 图像结构体
    ArmorSVM svm;                          // SVM
    DetectorState state = ARMOR_NOT_FOUND; // 识别状态
    std::vector<LightBar> lights;          // 找到的所有灯条
    std::vector<ArmorBox> armors;          // 找到的所有装甲板
    int targetNum = -1;                    // 操作手设置的目标装甲板
    ArmorParam armorParam;                 // 装甲板参数
    ArmorBox targetArmor;                  // 目标装甲板
public:
    ArmorDetector() = default;
    ArmorDetector(PicData src) : src_(src){};
    // 带有目标装甲板编号的构造函数
    ArmorDetector(PicData src, int targetNum) : src_(src), targetNum(targetNum){};
    ~ArmorDetector() = default;

public:
    /**
     * @brief 重置装甲板的各项信息，清空上一帧信息
     */
    void resetDetector()
    {
        state = LIGHTS_NOT_FOUND;
        lights.clear();
        armors.clear();
    }
    /**
     * @brief 程序是否识别到装甲板
     */
    bool isFoundArmor()
    {
        if (this->state == ARMOR_FOUND)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    /**
     * @brief 遍历图像中的所有轮廓，筛选可能的灯条轮廓
     */
    void findLights()
    {
        std::vector<std::vector<cv::Point>> lightContours; // roiIng中的候选灯条轮廓
        cv::Mat contourImg;                                // 给findContours用的图像，防止findContours改变roiImg
        src_.binImg.copyTo(contourImg);
        cv::findContours(contourImg, lightContours, 0, 2); // 在二值图像（binary image）中寻找轮廓（contour） CV_RETR_EXTERNAL = 0, CV_CHAIN_APPROX_SIMPLE = 2
        cv::RotatedRect lightRect;                         // 拟合椭圆来的灯条旋转矩形
        LightBar light;                                    // Temp变量
        for (auto &&lightContour : lightContours)
        {
            if (lightContour.size() < 6)
                continue; // 轮廓点数小于6，不可拟合椭圆
            if (cv::contourArea(lightContour) < armorParam.min_area)
            {
                continue; // 面积筛选滤去小发光点
            }
            lightRect = cv::fitEllipse(lightContour); // 拟合椭圆
            light = LightBar(lightRect);              // 构造为灯条
            if (std::abs(light.angle) > armorParam.max_angle)
                continue; // 角度筛选，滤去一些竖直偏角偏大的
            lights.emplace_back(light);
        }
        if (lights.size() < 2)
        {
            state = LIGHTS_NOT_FOUND; // 灯条少于两条则设置状态为没找到灯条
            return;
        }
        // 将灯条从左到右排序
        sort(lights.begin(), lights.end(), [](LightBar &a1, LightBar &a2)
             { return a1.center.x < a2.center.x; });
        state = LIGHTS_FOUND;
    }
    /**
     * @brief 删除游离灯条导致的错误装甲板
     *
     */
    void eraseErrorRepeatArmor()
    {
        int length = armors.size();
        auto it = armors.begin();
        for (size_t i = 0; i < length; i++)
            for (size_t j = i + 1; j < length; j++)
            {
                if (armors[i].l_lightindex == armors[j].l_lightindex ||
                    armors[i].l_lightindex == armors[j].r_lightindex ||
                    armors[i].r_lightindex == armors[j].l_lightindex ||
                    armors[i].r_lightindex == armors[j].r_lightindex)
                {
                    armors[i].getDeviationAngle() > armors[j].getDeviationAngle() ? armors.erase(it + i) : armors.erase(it + j);
                }
            }
    }
    /**
     * @brief 组合装甲板
     *
     */
    void matchArmors()
    {
        for (int i = 0; i < lights.size() - 1; i++)
        {
            for (int j = i + 1; j < lights.size(); j++) // 从左至右，每个灯条与其他灯条一次匹配判断
            {
                ArmorBox armor = ArmorBox(lights[i], lights[j]); // 利用左右灯条构建装甲板
                if (armor.isSuitableArmor())                     // 如果是合适的装甲板，则设置其他装甲板信息
                {
                    armor.l_lightindex = i;     // 左灯条的下标
                    armor.r_lightindex = j;     // 右灯条的下标
                    armor.calcuArmorArea();     // 计算装甲板面积
                    armors.emplace_back(armor); // 将匹配好的装甲板push入armors中
                }
            }
            eraseErrorRepeatArmor(); // 删除游离灯条导致的错误装甲板
        }
        if (armors.empty())
        {
            state = ARMOR_NOT_FOUND; // 如果armors目前仍为空，则设置状态为ARMOR_NOT_FOUND
            return;
        }
        else
        {
            state = ARMOR_FOUND; // 如果非空（有装甲板）则设置状态ARMOR_FOUND
            return;
        }
    }
    /**
     * @brief 通过SVM获得装甲板的图片和标号
     *
     */
    void SVMProcess()
    {
        if (!armors.empty())
        {
            svm.loadImg(src_.src);
            for (auto &armor : armors)
            {
                svm.getArmorImg(armor);
                svm.getArmorNum(armor);
            }
        }
    }
    /**
     * @brief 若有目标序号则优先击打，否则击打最大目标
     *
     */
    void setTargetArmor()
    {
        if (targetNum == -1)
        {
            int maxArea = 0, maxNo = 0;
            for (int i = 0; i < this->armors.size(); ++i)
            {
                if (armors[i].armorArea > maxArea)
                {
                    maxArea = armors[i].armorArea;
                    maxNo = i;
                }
            }
            this->targetArmor = armors[maxNo];
        }
        else
        {
            for (auto &&armor : armors)
            {
                if (armor.armorNum == targetNum)
                {
                    targetArmor = armor;
                }
            }
        }
    }

public:
    void start()
    {
        // 首先，载入并处理图像
        svm.loadImg(src_.src); // 对源图像预处理

        // 随后，重设detector的内容，清空在上一帧中找到的灯条和装甲板，同时检测器状态重置为LIGHTS_NOT_FOUND（最低状态）
        resetDetector();

        // 第三步，在当前图像中找出所有的灯条
        findLights();

        // 第四步，如果状态为LIGHTS_FOUND（找到多于两个灯条），则
        if (state == LIGHTS_FOUND)
        {
            // 将每两个灯条匹配为一个装甲板，如果匹配出来的装甲板是合适的，则压入armors中
            matchArmors();

            // 对每个识别到的装甲板进行SVM处理
            SVMProcess();

            // 如果找到了灯条，则设置好目标装甲板和上一个装甲板
            if (state == ARMOR_FOUND)
            {
                setTargetArmor();
            }
        }
    }
public:
    void throwShowImgs()
    {
        //TODO:抛出需要通过UDP传输的图像结构体
    }
};