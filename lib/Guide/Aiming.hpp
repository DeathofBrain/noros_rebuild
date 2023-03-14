#pragma once
#include <opencv2/opencv.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include "../DataStruct/Enums.hpp"
#include "../Detector/ArmorBox.hpp"
#include <bits/stdc++.h>

class Aiming
{
public:
    const static float g = 9.8F;
    const static float kk = 0.001F;
    const static float v = 13.9F;

public:
    Eigen::Vector3f posi_xyz;
    std::vector<double> x_vector = std::vector<double>(2, 0);
    std::vector<double> y_vector = std::vector<double>(2, 0);
    std::vector<double> z_vector = std::vector<double>(2, 0);
    float yaw_now;
    float pitch_now;
    float yaw_last;

public:
    void GetRealCoordinateX(Eigen::Vector3f &posi_xyz, double angle, double Lx, double Ly, double Lz, int twist_mode)
    {
        angle = -angle;
        Eigen::MatrixXd T(4, 4);
        Eigen::MatrixXd PB(4, 1);
        Eigen::MatrixXd PA(4, 1);
        // 齐次变换矩阵
        if (twist_mode == 0)
        {
            T << 1, 0, 0, Lx,
                0, cos(angle), sin(angle), Ly,
                0, -sin(angle), cos(angle), Lz,
                0, 0, 0, 1;
        }
        else if (twist_mode == 1)
        {
            T << cos(angle), sin(angle), 0, Lx,
                -sin(angle), cos(angle), 0, Ly,
                0, 0, 1, Lz,
                0, 0, 0, 1;
        }
        PB << posi_xyz(0), posi_xyz(1), posi_xyz(2);
        PA = T * PB;
        posi_xyz(0) = PA(0);
        posi_xyz(1) = PA(1);
        posi_xyz(2) = PA(2);
    }

    std::vector<float> GetAngle()
    {
        float vx, vy, t, t_k, x_2, dy, y_2, yaw, pitch, angle, y_actual, log_result, f, f1; // m/s
        std::vector<float> theta(3, 0);
        double temp = posi_xyz(0);
        y_2 = posi_xyz(2);
        x_2 = sqrt(posi_xyz(0) * posi_xyz(0) + posi_xyz(1) * posi_xyz(1));
        angle = atan2(y_2, x_2);

        for (unsigned i = 0; i < 20; i++)
        {
            vx = v * cos(angle);
            vy = v * sin(angle);

            t_k = 0;
            log_result = log(kk * vx * t_k + 1);
            f = (1 / kk) * log_result - x_2;
            f1 = vx / (kk * vx * t_k + 1);
            t = t_k - f / f1;

            while (t - t_k > 0.01)
            {
                t_k = t;
                log_result = log(kk * vx * t_k + 1);
                f = (1 / kk) * log_result - x_2;
                f1 = vx / (kk * vx * t_k + 1);
                t = t_k - f / f1;
            }
            y_actual = vy * t - g * t * t / 2;
            dy = y_2 - y_actual;
            if (fabs(dy) < 0.01)
            {
                break;
            }
            angle = angle + 0.1 * dy;
        }
        pitch = angle;
        yaw = atan2(posi_xyz(0), posi_xyz(1));

        theta[0] = yaw;
        theta[1] = pitch;
        theta[2] = t + 0.5;

        return theta;
    }
    /**
     * @brief 计算平均速度和加速度
     *
     * @param posi_xyz 坐标
     */
    void velocity(Eigen::Vector3f posi_xyz)
    {
        float dt = 0.02F;
        int data_size = 7;
        x_vector.push_back(posi_xyz[0]);
        y_vector.push_back(posi_xyz[1]);
        z_vector.push_back(posi_xyz[2]);
        if ((x_vector.size()) < (data_size + 2))
        {
            x_vector[1] = 0;
            y_vector[1] = 0;
            z_vector[1] = 0;
            if (x_vector.size() > 3)
            {
                x_vector[0] = (x_vector[x_vector.size() - 1] - x_vector[2]) / (dt * (x_vector.size() - 3));
                y_vector[0] = (y_vector[x_vector.size() - 1] - y_vector[2]) / (dt * (x_vector.size() - 3));
                z_vector[0] = (z_vector[x_vector.size() - 1] - z_vector[2]) / (dt * (x_vector.size() - 3));
            }
            else
            {
                x_vector[0] = 0;
                y_vector[0] = 0;
                z_vector[0] = 0;
            }
        }
        else
        {
            x_vector[0] = (x_vector[data_size + 1] - x_vector[2]) / (dt * 5);
            y_vector[0] = (y_vector[data_size + 1] - y_vector[2]) / (dt * 5);
            z_vector[0] = (z_vector[data_size + 1] - z_vector[2]) / (dt * 5);
            x_vector[1] = (x_vector[data_size + 1] - 2 * x_vector[data_size - 2] + x_vector[data_size - 5]) / (9 * pow(dt, 2));
            y_vector[1] = (y_vector[data_size + 1] - 2 * y_vector[data_size - 2] + y_vector[data_size - 5]) / (9 * pow(dt, 2));
            z_vector[1] = (z_vector[data_size + 1] - 2 * z_vector[data_size - 2] + z_vector[data_size - 5]) / (9 * pow(dt, 2));
            x_vector.erase(x_vector.begin() + 2, x_vector.begin() + 3);
            y_vector.erase(y_vector.begin() + 2, y_vector.begin() + 3);
            z_vector.erase(z_vector.begin() + 2, z_vector.begin() + 3);
        }
    }

public:
    Aiming() = default;
    ~Aiming() = default;

public:
    std::pair<float, float> Guide(ArmorBox targetArmor)
    {
        float x0 = posi_xyz[0];
        float y0 = posi_xyz[1];
        float z0 = -posi_xyz[2];
        float dfoot = 0.14F;
        posi_xyz[0] = -posi_xyz[0];
        posi_xyz[2] = -posi_xyz[2];
        GetRealCoordinateX(posi_xyz, pitch_now / 180 * 3.14, 0, 0.03, 0.03, 0);
        velocity(posi_xyz);
        auto theta = GetAngle();
        yaw_last = yaw_now;
        return {theta[1] / 3.14 * 180, theta[0] / 3.14 * 180};
    }
};
