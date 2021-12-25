//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "Vector.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    // objects是meshtriangle的数组
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

// 光线和场景的交点
Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

// 把所有发光的objects(meshtriangle)当成光源, 进行随机采样
// pos 交点
// pdf 概率密度函数
void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    // 计算所有发光的对象的总面积
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    // 随机一个面积
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            // 如果面积累计大于随机面积, 则在这个对象(meshtriangle)上采样
            if (p <= emit_area_sum){
                // 这个函数最终会随机取到一个三角形上的一个随机的点的coordinates和法线赋值给pos
                // pdf最后会计算得到1/对象(meshitriangle)的面积
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // 计算交点
    Intersection interP = intersect(ray);
    // 如果不相交, 则返回黑色
    if (!interP.happened) return Vector3f();

    // 自身发光
    Vector3f selfL = interP.m->getEmission();

    // 1. 计算直接光照
    // 初始化直接光照为0
    Vector3f dirL;
    // 从发光的对象里随机取一个点, 以及概率
    Intersection sampleInter;
    float dirP;
    sampleLight(sampleInter, dirP);
    // 生成反射光线, 光源起点往前挪动一点, 避免和起点相交
    Ray dirRefRay(interP.coords, normalize(sampleInter.coords-interP.coords));
    dirRefRay.origin = dirRefRay(EPSILON);
    // 计算这条光线和环境的交点, 以判断这条光线是否被遮挡
    Intersection blockInter = intersect(dirRefRay);
    // 如果交点是光源, 则计算直接光源
    if (blockInter.happened && blockInter.m->hasEmission()) {
        // 获取这条光线的radiance, 以及BRDF, 等数据
        Vector3f dirBRDF = interP.m->eval(-ray.direction, dirRefRay.direction, interP.normal);
        float blockDistSquare = dotProduct(blockInter.coords-interP.coords, blockInter.coords-interP.coords);
        // 计算直接光照
        dirL = blockInter.emit * dirBRDF * dotProduct(dirRefRay.direction, interP.normal)
            * dotProduct(-dirRefRay.direction, blockInter.normal) / blockDistSquare / dirP;
    };

    // 2. 计算间接光照
    // 初始化间接光照为0
    Vector3f indirL;
    // 测试俄罗斯轮盘赌, 如果通过, 则计算间接光照
    if (get_random_float()<=RussianRoulette) {
        // 根据材质随机生成一条光线
        Vector3f indirRefDir = interP.m->sample(-ray.direction, interP.normal);
        Ray indirRefRay(interP.coords, normalize(indirRefDir));
        // 计算这条光线和scene的交点
        Intersection indirInter = intersect(indirRefRay);
        // 如果交点所处的对象是不发光, 则递归计算间接光照, 注意这里要除以俄罗斯轮盘赌的概率
        if (indirInter.happened && !indirInter.m->hasEmission()) {
            Vector3f indirBRDF = interP.m->eval(-ray.direction, indirRefRay.direction, interP.normal);
            indirL = this->castRay(indirRefRay, 0) * indirBRDF * dotProduct(indirRefDir, interP.normal)
            / interP.m->pdf(-ray.direction, indirRefDir, interP.normal) / RussianRoulette;
        }
    }

    // 总的光照等于直接光照加间接光照
    return selfL + dirL + indirL;
}