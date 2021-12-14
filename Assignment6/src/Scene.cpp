//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    // objects是scene的属性, vector数组, 里面的元素就是add进去的bunny trianglemesh
    // 第二个参数1代表最后分割的叶子节点的三角形数量
    // 第三个参数是分割方法
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
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

// Implementation of the Whitted-syle light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refracton direction and cast two new rays into the scene
// by calling the castRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refractin depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is duffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
// ray 光线(视线)
// depth 弹射次数
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // 如果光线弹射次数大于限制, 则返回黑色
    if (depth > this->maxDepth) {
        return Vector3f(0.0,0.0,0.0);
    }
    // 这条光线和scene相交的结果
    Intersection intersection = Scene::intersect(ray);
    // 相交的材质, 反射、折射的性质等等
    Material *m = intersection.m;
    // 相交的对象, 三角形、 圆形
    Object *hitObject = intersection.obj;
    // 背景色
    Vector3f hitColor = this->backgroundColor;
    // float tnear = kInfinity;
    Vector2f uv;
    uint32_t index = 0;
    // 如果相交
    if(intersection.happened) {

        // 交点
        Vector3f hitPoint = intersection.coords;
        // 交点在对象上的法线, 例如相交对象是三角形, 那么这个就是三角形面在交点上的法线
        Vector3f N = intersection.normal; // normal
        Vector2f st; // st coordinates
        // 获取交点处的三角形属性, 备注: 代码里只重复获取了法线
        hitObject->getSurfaceProperties(hitPoint, ray.direction, index, uv, N, st);
        // Vector3f tmp = hitPoint;
        // 获取相交对象(三角形)的材质
        switch (m->getType()) {
            // 如果是反射和折射
            case REFLECTION_AND_REFRACTION:
            {
                // 反射方向
                Vector3f reflectionDirection = normalize(reflect(ray.direction, N));
                // 折射方向
                Vector3f refractionDirection = normalize(refract(ray.direction, N, m->ior));
                // 精度问题, 如果方向是往内, 则起点往内移一点, 如果往外, 则往外移一点
                Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ?
                                             hitPoint - N * EPSILON :
                                             hitPoint + N * EPSILON;
                Vector3f refractionRayOrig = (dotProduct(refractionDirection, N) < 0) ?
                                             hitPoint - N * EPSILON :
                                             hitPoint + N * EPSILON;
                // 反射光线和折射光线再到scene里传播, 递归执行, 找到相交的颜色
                Vector3f reflectionColor = castRay(Ray(reflectionRayOrig, reflectionDirection), depth + 1);
                Vector3f refractionColor = castRay(Ray(refractionRayOrig, refractionDirection), depth + 1);
                // 计算出反射比
                float kr;
                fresnel(ray.direction, N, m->ior, kr);
                // 最终颜色是反射+折射
                hitColor = reflectionColor * kr + refractionColor * (1 - kr);
                break;
            }
            case REFLECTION:
            {
                // 只有反射
                float kr;
                fresnel(ray.direction, N, m->ior, kr);
                Vector3f reflectionDirection = reflect(ray.direction, N);
                Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ?
                                             hitPoint + N * EPSILON :
                                             hitPoint - N * EPSILON;
                hitColor = castRay(Ray(reflectionRayOrig, reflectionDirection),depth + 1) * kr;
                break;
            }
            default:
            {
                // [comment]
                // We use the Phong illumation model int the default case. The phong model
                // is composed of a diffuse and a specular reflection component.
                // [/comment]
                Vector3f lightAmt = 0, specularColor = 0;
                Vector3f shadowPointOrig = (dotProduct(ray.direction, N) < 0) ?
                                           hitPoint + N * EPSILON :
                                           hitPoint - N * EPSILON;
                // [comment]
                // Loop over all lights in the scene and sum their contribution up
                // We also apply the lambert cosine law
                // [/comment]
                // 光源循环
                for (uint32_t i = 0; i < get_lights().size(); ++i)
                {
                    // 判断是否是面光源
                    auto area_ptr = dynamic_cast<AreaLight*>(this->get_lights()[i].get());
                    if (area_ptr)
                    {
                        // Do nothing for this assignment
                    }
                    else
                    {
                        Vector3f lightDir = get_lights()[i]->position - hitPoint;
                        // square of the distance between hitPoint and the light
                        float lightDistance2 = dotProduct(lightDir, lightDir);
                        lightDir = normalize(lightDir);
                        float LdotN = std::max(0.f, dotProduct(lightDir, N));
                        Object *shadowHitObject = nullptr;
                        float tNearShadow = kInfinity;
                        // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                        // 照射点和光源的连线是否和scene里的对象相交, 如果相交, 则处在阴影中
                        bool inShadow = bvh->Intersect(Ray(shadowPointOrig, lightDir)).happened;
                        // 光源强度
                        lightAmt += (1 - inShadow) * get_lights()[i]->intensity * LdotN;
                        // 高光
                        Vector3f reflectionDirection = reflect(-lightDir, N);
                        specularColor += powf(std::max(0.f, -dotProduct(reflectionDirection, ray.direction)),
                                              m->specularExponent) * get_lights()[i]->intensity;
                    }
                }
                hitColor = lightAmt * (hitObject->evalDiffuseColor(st) * m->Kd + specularColor * m->Ks);
                break;
            }
        }
    }
    // 不相交则默认返回scene的背景色
    return hitColor;
}