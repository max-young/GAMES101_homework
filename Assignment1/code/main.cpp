#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>


constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    // 单位矩阵
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    model(0, 0) = cos(rotation_angle * MY_PI / 180);
    model(0, 1) = -sin(rotation_angle * MY_PI / 180);
    model(1, 0) = sin(rotation_angle * MY_PI / 180);
    model(1, 1) = cos(rotation_angle * MY_PI / 180);
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    float n = -zNear;
    float f = -zFar;
    float t = fabs(n) * tan(eye_fov / 2 * MY_PI / 180);
    float b = -t;
    float r = t * aspect_ratio;
    float l = -r;

    projection(0, 0) = 2 * n / (r - l);
    projection(0, 2) = (l + r) / (l - r);
    projection(1, 1) = 2 * n / (t - b);
    projection(1, 2) = (b + t) / (b - t);
    projection(2, 2) = (f + n) / (n - f);
    projection(2, 3) = 2 * f * n / (f - n);
    projection(3, 2) = 1;
    projection(3, 3) = 0;

    return projection;
}

// 主程序
int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    // 根据命令行参数获取旋转角度和输出文件名称
    // 参数示例用法: main −r 20 image.png
    // argc[0]就是main文件, argc[2]就是20旋转角度, argc[3]就是输出文件
    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    // 初始化光栅化器的尺寸, 像素数量是700*700
    rst::rasterizer r(700, 700);

    // 视点(相机的中心位置, 人眼位置)
    Eigen::Vector3f eye_pos = {0, 0, 5};

    // 三个三位点的数组, 构成一个三角形
    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    // 单个向量的数组
    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    // 将上面两个值放入字典, 下面两个值是对应的key
    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    std::cout << filename;

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        // 将光栅化器初始化, 所有像素都变成0, 0, 0
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        // 光栅化器的旋转矩阵, 绕Z轴旋转angle角度
        r.set_model(get_model_matrix(angle));
        // 光栅化器的平移矩阵, 减去人眼坐标, 得到相对于人眼的位置
        r.set_view(get_view_matrix(eye_pos));
        // 投影变换矩阵
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}