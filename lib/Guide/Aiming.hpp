#pragma once
#include <opencv2/opencv.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <bits/stdc++.h>

class Aiming
{
public:
    const static float g = 9.8F;
    const static float kk = 0.001F;
    const static float v = 13.9F;
public:
    Eigen::Vector3f posi_xyz;

public:
    void GetRealCoordinateX(double angle, double Lx, double Ly, double Lz, int twist_mode)
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

public:
    Aiming();
    ~Aiming();
};
