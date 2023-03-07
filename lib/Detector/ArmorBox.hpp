#pragma once
#include <opencv2/opencv.hpp>
#include <DataStruct/Structs.hpp>

class ArmorBox
{
public:
    LightBar l_light;             // 装甲板的左灯条
    LightBar r_light;             // 装甲板的右灯条
    int l_lightindex;             // 左灯条的下标
    int r_lightindex;             // 右灯条的下标
    int armorNum;                 // 装甲板上的数字
    cv::Point2f armorVertices[4]; // 装甲板的四个顶点 bl->tl->tr->br 左下 左上 右上 右下
    ArmorType type;               // 装甲板类型
    cv::Point2f center;           // 装甲板中心点
    cv::Rect armorRect;           // 装甲板的矩形获取roi用
    int armorArea;                // 装甲板面积
    float armorAngle;             // 装甲板（灯条）角度
    cv::Mat armorImg;             // 装甲板的图片（透射变换获得）
    ArmorParam armorParam;        // 装甲板相关的各项参数

public:
    /**
     * @brief 设置装甲板顶点
     *
     * @param l_lightlight 左灯条
     * @param r_lightlight 右灯条
     */
    void setArmorVertices(const LightBar &l_lightlight, const LightBar &r_lightlight)
    {
        cv::Size exLSize((l_lightlight.lightRect.size.width), (l_lightlight.lightRect.size.height));
        cv::Size exRSize((r_lightlight.lightRect.size.width), (r_lightlight.lightRect.size.height));
        cv::RotatedRect exLLight(l_lightlight.center, exLSize, this->armorAngle);
        cv::RotatedRect exRLight(r_lightlight.center, exRSize, this->armorAngle);

        cv::Point2f pts_l[4];
        exLLight.points(pts_l);
        cv::Point2f upper_lightl = (pts_l[1] + pts_l[2]) / 2;
        cv::Point2f lower_lightl = (pts_l[0] + pts_l[3]) / 2;

        cv::Point2f pts_r[4];
        exRLight.points(pts_r);
        cv::Point2f upper_lightr = (pts_r[2] + pts_r[1]) / 2;
        cv::Point2f lower_lightr = (pts_r[0] + pts_r[3]) / 2;
        // 装甲板的四个顶点
        this->armorVertices[0] = upper_lightl;
        this->armorVertices[1] = upper_lightr;
        this->armorVertices[2] = lower_lightr;
        this->armorVertices[3] = lower_lightl;
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
        l_lightindex = -1;
        r_lightindex = -1;
        l_light = LightBar();
        r_light = LightBar();
        armorNum = 0;
        type = SMALL_ARMOR;
        center = cv::Point2f();
        armorRect = cv::Rect();
        armorImg = cv::Mat();
    }
    /**
     * @brief 由灯条信息构造装甲板
     *
     * @param l_lightlight 左灯条
     * @param r_lightlight 右灯条
     */
    ArmorBox(const LightBar &l_lightlight, const LightBar &r_lightlight)
    {
        this->l_light = l_lightlight;
        this->r_light = r_lightlight;
        armorNum = 0;
        armorAngle = (l_lightlight.angle + r_lightlight.angle) / 2;
        setArmorVertices(l_lightlight, r_lightlight);
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
    // 装甲板左右灯条角度差
    float getAngleDiff() const
    {
        return abs(l_light.angle - r_light.angle); // 灯条的角度差
    }

    // 灯条错位度角(两灯条中心连线与水平线夹角)
    float getDeviationAngle() const
    {
        float delta_x = r_light.center.x - l_light.center.x;               // Δx
        float delta_y = r_light.center.y - l_light.center.y;               // Δy
        float deviationAngle = abs(atan(delta_y / delta_x)) * 180 / CV_PI; // tanθ=Δy/Δx
        return deviationAngle;
    }

    // 灯条位置差距 两灯条中心x方向差距比值
    float getDislocationX() const
    {
        float meanLen = (l_light.length + r_light.length) / 2;
        float xDiff = abs(l_light.center.x - r_light.center.x); // y轴方向上的距离比值（y轴距离与灯条平均值的比）
        float xDiff_ratio = xDiff / meanLen;
        return xDiff_ratio;
    }

    // 灯条位置差距 两灯条中心Y方向差距比值
    float getDislocationY() const
    {
        float meanLen = (l_light.length + r_light.length) / 2;
        float yDiff = abs(l_light.center.y - r_light.center.y); // x轴方向上的距离比值（x轴距离与灯条平均值的比）
        float yDiff_ratio = yDiff / meanLen;
        return yDiff_ratio;
    }

    // 左右灯条长度差比值
    float getLengthRation() const
    {
        float length_diff = abs(l_light.length - r_light.length);
        float lengthDiffRation = length_diff / MAX(l_light.length, r_light.length);
        return lengthDiffRation;
    }

    // 判断装甲板选定是否合适
    bool isSuitableArmor() const
    {
        return this->getAngleDiff() < armorParam.max_angle_diff &&           // 灯条角度差判断，需小于允许的最大角差
               this->getDeviationAngle() < armorParam.max_deviation_angle && // 灯条错位度角(两灯条中心连线与水平线夹角)判断
               this->getDislocationX() < armorParam.max_x_diff_ratio &&      // 灯条位置差距 两灯条中心x、y方向差距不可偏大（用比值作为衡量依据）
               this->getDislocationY() < armorParam.max_y_diff_ratio &&      // 灯条位置差距 两灯条中心x、y方向差距不可偏大（用比值作为衡量依据）
               this->getLengthRation() < armorParam.max_lengthDiff_ratio;
    }

    // 计算装甲板的面积
    void calcuArmorArea()
    {
        this->armorArea = this->armorRect.area();
    }
};

class ArmorDetector : public ArmorBox
{
public:
    PicData src_;
    
};


