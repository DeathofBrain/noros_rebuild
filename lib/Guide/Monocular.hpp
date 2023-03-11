#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <DataStruct/Enums.hpp>
#include <DataStruct/GlobalVariables.hpp>
#include <iostream>

class Monocular
{
public:
    // Camera params
    cv::Mat CAMERA_MATRIX;    // IntrinsicMatrix		  fx,fy,cx,cy
    cv::Mat DISTORTION_COEFF; // DistortionCoefficients k1,k2,p1,p2

    // Object points in world coordinate
    std::vector<cv::Point3f> SMALL_ARMOR_POINTS_3D;
    std::vector<cv::Point3f> BIG_ARMOR_POINTS_3D;

    // Targets
    std::vector<cv::Point2f> targetContour;
    cv::Point2f targetCenter;
    ArmorType targetType;

    // calculated by solvePnP
    // s[R|t]=s'  s->world coordinate;s`->camera coordinate
    cv::Mat rVec; // rot rotation between camera and target center
    cv::Mat tVec; // trans tanslation between camera and target center

    // results
    double x_pos, y_pos, z_pos; // 装甲板坐标
    double distance;            // 相机距装甲板的距离
    int shoot;                  // 射击标识

public:
    /**
     * @brief 初始化相机相机内参
     */
    Monocular()
    {
        using namespace camera;
        this->CAMERA_MATRIX = (cv::Mat_<double>(3, 3) << fx, 0, u0, 0, fy, v0, 0, 0, 1);
        this->DISTORTION_COEFF = (cv::Mat_<double>(5, 1) << k1, k2, p1, p2, k3);
        this->rVec = cv::Mat::zeros(3, 1, CV_64FC1);
        this->tVec = cv::Mat::zeros(3, 1, CV_64FC1);
    }

    /**
     * @brief 设置装甲板的实际宽和高
     *
     * @param armorType BIG or SMALL
     */
    void setArmorSize(ArmorType armorType)
    {
        using namespace armor;
        double half_x;
        double half_y;
        switch (armorType)
        {
        case SMALL_ARMOR:
            //	cout<<"SMALL"<<endl;
            half_x = SmallArmor_width / 2.0;
            half_y = SmallArmor_height / 2.0;
            SMALL_ARMOR_POINTS_3D.push_back(cv::Point3f(-half_x, half_y, 0));  // tl top left
            SMALL_ARMOR_POINTS_3D.push_back(cv::Point3f(half_x, half_y, 0));   // tr top right
            SMALL_ARMOR_POINTS_3D.push_back(cv::Point3f(half_x, -half_y, 0));  // br below right
            SMALL_ARMOR_POINTS_3D.push_back(cv::Point3f(-half_x, -half_y, 0)); // bl below left
            break;
        case BIG_ARMOR:
            // /cout<<"BIG"<<endl;
            half_x = BigArmor_width / 2.0;
            half_y = BigArmor_height / 2.0;
            BIG_ARMOR_POINTS_3D.push_back(cv::Point3f(-half_x, half_y, 0));  // tl top left
            BIG_ARMOR_POINTS_3D.push_back(cv::Point3f(half_x, half_y, 0));   // tr top right
            BIG_ARMOR_POINTS_3D.push_back(cv::Point3f(half_x, -half_y, 0));  // br below right
            BIG_ARMOR_POINTS_3D.push_back(cv::Point3f(-half_x, -half_y, 0)); // bl below left
            break;
        default:
            break;
        }
    }

    /**
     * @brief 设置装甲板所对应的图像二维坐标
     *
     * @param contour 装甲板图像的四个轮廓点：left_up, right_up, left_down, right_down
     * @param center  装甲板图像的中心坐标
     * @param type    装甲板的类型
     */
    void setTarget2D(std::vector<cv::Point2f> contour, cv::Point2f center, ArmorType type)
    {
        this->targetContour = contour;
        this->targetCenter = center;
        this->targetType = type;
    }

    /**
     * @brief 定位，计算装甲板在相机坐标系下的三维坐标
     */
    void getArmorCoordinate()
    {
        if (!CAMERA_MATRIX.empty() && !DISTORTION_COEFF.empty())
        {
            switch (targetType)
            {
            case SMALL_ARMOR:
                solvePnP(SMALL_ARMOR_POINTS_3D, targetContour, CAMERA_MATRIX, DISTORTION_COEFF, rVec, tVec, false, cv::SOLVEPNP_ITERATIVE);
                break;
            case BIG_ARMOR:
                solvePnP(BIG_ARMOR_POINTS_3D, targetContour, CAMERA_MATRIX, DISTORTION_COEFF, rVec, tVec, false, cv::SOLVEPNP_ITERATIVE);
                break;
            default:
                std::cout << "----------------------Error: No Matched Armor Type!----------------------" << std::endl;
                break;
            }

            this->x_pos = tVec.at<double>(0, 0);
            this->y_pos = tVec.at<double>(1, 0);
            this->z_pos = tVec.at<double>(2, 0);
            this->distance = sqrt(x_pos * x_pos + y_pos * y_pos + z_pos * z_pos);
            if (this->distance < 3000)
                this->shoot = 1;
            else
                this->shoot = 0;
        }
        else
        {
            std::cout << "Warnning: CAMERA_MATRIX or DISTORTION_COEFF is null!" << std::endl;
        }
        this->clearCoordinateInfo();
    }

    /**
     * @brief 清空记录的所有坐标信息：2D, 3D
     *
     */
    void clearCoordinateInfo()
    {
        // 2D
        this->targetContour.clear();

        // 3D
        this->SMALL_ARMOR_POINTS_3D.clear();
        this->BIG_ARMOR_POINTS_3D.clear();
    }

    /**
     * @brief 显示一些定位信息
     */
    void showDebugInfo(int showCordin, int showDist)
    {
        if (showCordin == 1)
        {
            std::cout << "x: " << this->x_pos << std::endl;
            std::cout << "y: " << this->y_pos << std::endl;
            std::cout << "z: " << this->z_pos << std::endl;
        }

        if (showDist == 1)
        {
            std::cout << "distance: " << this->distance << std::endl;
        }
    }

public:
};
