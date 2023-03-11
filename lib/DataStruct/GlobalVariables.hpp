namespace camera
{
    /**
     * @brief 相机的内参矩阵
     */
    inline double fx = 2345.20;
    inline double fy = 2343.22;
    inline double u0 = 712.478;
    inline double v0 = 567.407;

    /**
     * @brief 相机的畸变参数
     */
    // 1).径向畸变：（Radial Distortion）
    inline double k1 = -0.1022;
    inline double k2 = 0.3699;
    inline double k3 = 0.0;
    // 2).切向畸变：（Tangential Distortion）
    inline double p1 = 0;
    inline double p2 = 0;
} // namespace camera

namespace armor
{
    inline int BigArmor_width = 230;
    inline int BigArmor_height = 65;

    inline int SmallArmor_width = 135;
    inline int SmallArmor_height = 56;
} // namespace armor
