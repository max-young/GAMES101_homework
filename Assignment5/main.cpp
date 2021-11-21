#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Light.hpp"
#include "Renderer.hpp"

// In the main function of the program, we create the scene (create objects and lights)
// as well as set the options for the render (image width and height, maximum recursion
// depth, field-of-view, etc.). We then call the render function().
int main()
{
    // 场景的长宽
    Scene scene(1280, 960);

    // 构造球的指针(重心, 半径)
    auto sph1 = std::make_unique<Sphere>(Vector3f(-1, 0, -12), 2);
    // 球的材质
    sph1->materialType = DIFFUSE_AND_GLOSSY;
    // 漫反射颜色
    sph1->diffuseColor = Vector3f(0.6, 0.7, 0.8);

    // 第二个球
    auto sph2 = std::make_unique<Sphere>(Vector3f(0.5, -0.5, -8), 1.5);
    sph2->ior = 1.5;
    // 材质
    sph2->materialType = REFLECTION_AND_REFRACTION;

    // 场景增加两个球
    scene.Add(std::move(sph1));
    scene.Add(std::move(sph2));

    // 4个顶点
    Vector3f verts[4] = {{-5,-3,-6}, {5,-3,-6}, {5,-3,-16}, {-5,-3,-16}};
    // 下面传参2, 代表两个三角形
    // 两个三角形的三个顶点对应的顶点索引, 这里面的最大数字是3, 代表有4个顶点
    uint32_t vertIndex[6] = {0, 1, 3, 1, 2, 3};
    // 两个三角形的
    Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    // 三角形网状结构
    auto mesh = std::make_unique<MeshTriangle>(verts, vertIndex, 2, st);
    mesh->materialType = DIFFUSE_AND_GLOSSY;

    scene.Add(std::move(mesh));
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 0.5));
    scene.Add(std::make_unique<Light>(Vector3f(30, 50, -12), 0.5));    

    Renderer r;
    r.Render(scene);

    return 0;
}