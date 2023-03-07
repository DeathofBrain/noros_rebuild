#pragma once
#include <opencv2/opencv.hpp>
#include <DataStruct/Structs.hpp>

class ArmorBox
{
public:
    LightBar L_;                  // 装甲板的左灯条
    LightBar R_;                  // 装甲板的右灯条
    int L_index;                  // 左灯条的下标
    int R_index;                  // 右灯条的下标
    int armorNum;                 // 装甲板上的数字
    cv::Point2f armorVertices[4]; // 装甲板的四个顶点 bl->tl->tr->br 左下 左上 右上 右下
    ArmorType type;               // 装甲板类型
    cv::Point2f center;           // 装甲板中心点
    cv::Rect armorRect;           // 装甲板的矩形获取roi用
    int armorArea;                // 装甲板面积
    float armorAngle;             // 装甲板（灯条）角度
    cv::Mat armorImg;             // 装甲板的图片（透射变换获得）

public:
    /**
     * @brief 设置装甲板顶点
     *
     * @param l_light 左灯条
     * @param r_light 右灯条
     */
    void setArmorVertices(const LightBar &l_light, const LightBar &r_light)
    {
        cv::Size exLSize((l_light.lightRect.size.width), (l_light.lightRect.size.height));
        cv::Size exRSize((r_light.lightRect.size.width), (r_light.lightRect.size.height));
        cv::RotatedRect exLLight(l_light.center, exLSize, this->armorAngle);
        cv::RotatedRect exRLight(r_light.center, exRSize, this->armorAngle);

        cv::Point2f pts_l[4];
        exLLight.points(pts_l);
        cv::Point2f upper_l = (pts_l[1] + pts_l[2]) / 2;
        cv::Point2f lower_l = (pts_l[0] + pts_l[3]) / 2;

        cv::Point2f pts_r[4];
        exRLight.points(pts_r);
        cv::Point2f upper_r = (pts_r[2] + pts_r[1]) / 2;
        cv::Point2f lower_r = (pts_r[0] + pts_r[3]) / 2;
        // 装甲板的四个顶点
        this->armorVertices[0] = upper_l;
        this->armorVertices[1] = upper_r;
        this->armorVertices[2] = lower_r;
        this->armorVertices[3] = lower_l;
    }
    /**
     * @brief 获得装甲板中心点
     *
     * @param bl 左下
     * @param tl 左上
     * @param tr 右上
     * @param br 右下
     */
    void crossPointof(const cv::Point2f &bl, const cv::Point2f &tl, const cv::Point2f &tr, const cv::Point2f &br)
    {
        float a1 = tr.y - bl.y;
        float b1 = tr.x - bl.x;
        float c1 = bl.x * tr.y - tr.x * bl.y;

        float a2 = br.y - tl.y;
        float b2 = br.x - tl.x;
        float c2 = tl.x * br.y - br.x * tl.y;

        float d = a1 * b2 - a2 * b1;
        if (d == 0.0)
        {
            this->center = cv::Point2f(FLT_MAX, FLT_MAX);
        }
        else
        {
            this->center = cv::Point2f((b2 * c1 - b1 * c2) / d, (c1 * a2 - c2 * a1) / d);
        }
    }

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
    /**
     * @brief 由灯条信息构造装甲板
     *
     * @param l_light 左灯条
     * @param r_light 右灯条
     * @param shriTimes 缩放倍率
     */
    ArmorBox(const LightBar &l_light, const LightBar &r_light, int shriTimes)
    {
        this->L_ = l_light;
        this->R_ = r_light;
        armorNum = 0;
        armorAngle = (l_light.angle + r_light.angle) / 2;
        setArmorVertices(l_light, r_light);
        crossPointof(armorVertices[0], armorVertices[1], armorVertices[2], armorVertices[3]);
        armorRect = boundingRect(std::vector<cv::Point2f>(armorVertices, armorVertices + 4));
        float lightX = (armorVertices[1].x - armorVertices[0].x + armorVertices[2].x - armorVertices[3].x) / 2;
        float lightY = (armorVertices[3].y - armorVertices[0].y + armorVertices[2].y - armorVertices[1].y) / 2;
        if ((lightX / lightY) > 3.3)
        {
            type = BIG_ARMOR;
        }
        else if ((lightX / lightY) > 2.6)
        {
            if (lightX > 130)
            {
                type = BIG_ARMOR;
            }
            else
            {
                type = SMALL_ARMOR;
            }
        }
        else
        {
            type = SMALL_ARMOR;
        }
    }

    ~ArmorBox() = default;

public:
    

};
