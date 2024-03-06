#include "be_raytracer.hpp"
#include "be_color.hpp"
#include "be_components.hpp" // IWYU pragma: keep
#include "be_gameCoordinator.hpp"
#include "be_timer.hpp"
#include "be_utilityFunctions.hpp"

namespace be{

RayHitOpt RayTracer::rayTriangleIntersection(const Ray& ray, const Triangle& trianglePrimitive){
    return rayTriangleIntersection(ray, trianglePrimitive.p0, trianglePrimitive.p1, trianglePrimitive.p2);
}

RayHitOpt RayTracer::rayTriangleIntersection(const Ray& ray, const Vector3& p0, const Vector3& p1, const Vector3& p2){
    Vector3 e0 = p1 - p0;
    Vector3 e1 = p2 - p0;

    Vector3 w = ray.getDirection();
    Vector3 o = ray.getOrigin();

    Vector3 tmp = Vector3::cross(e0, e1);
    if(tmp.getNorm() == 0.f){
        return RayHit::NO_HIT;
    }

    Vector3 n = Vector3::normalize(tmp);
    Vector3 q = Vector3::cross(w, e1);
    float a = Vector3::dot(e0, q);
    
    // counter clock wise order
    if(Vector3::dot(n, w) >= 0){
        return RayHit::NO_HIT;
    }

    if(isZero(a)){
        return RayHit::NO_HIT;
    }

    Vector3 s = (o-p0) / a;
    Vector3 r = Vector3::cross(s, e0);

    float b0 = Vector3::dot(s, q);
    if(b0 < 0){
        return RayHit::NO_HIT;
    }
    float b1 = Vector3::dot(r, w);
    if(b1 < 0){
        return RayHit::NO_HIT;
    }
    float b2 = 1 - b0 - b1;
    if(b2 < 0){
        return RayHit::NO_HIT;
    }

    float t = Vector3::dot(e1, r);
    if(t < 0){
        return RayHit::NO_HIT;
    }

    Vector4 res = {b0,b1,b2,t};
    return RayHit(res);
}

Vector3 RayTracer::shade(RayHit hit, Triangle triangle) const {
    return Color::toSRGB(Color::RED); // red tmp
}

std::vector<Triangle> RayTracer::getTriangles() const{
    std::vector<Triangle> allTriangles = {};
    #ifndef NDEBUG
    fprintf(stdout, "There are %zu objects in the scene!\n", _Scene->getObjects().size());
    #endif

    for(auto obj : _Scene->getObjects()){
        
        auto model = GameCoordinator::getComponent<ComponentModel>(obj)._Model;
        auto triangles = model->getTrianglePrimitives();
        #ifndef NDEBUG
        fprintf(stdout, "there are %zu triangles in the object `%d'\n", triangles.size(), obj);
        #endif

        auto material = GameCoordinator::getComponent<ComponentMaterial>(obj)._Material;
        auto transform = GameCoordinator::getComponent<ComponentTransform>(obj)._Transform;
        Matrix4x4 modelMatrix = transform->getModelTransposed();
        for(auto& triangle : triangles){
            triangle.p0 = (modelMatrix * Vector4(triangle.p0, 1.f)).xyz();
            triangle.p1 = (modelMatrix * Vector4(triangle.p1, 1.f)).xyz();
            triangle.p2 = (modelMatrix * Vector4(triangle.p2, 1.f)).xyz();
            triangle._Material = material;
        }

        allTriangles.insert(allTriangles.end(), triangles.begin(), triangles.end());
    }

    return allTriangles;
}


// helper progress bar


void RayTracer::run(Vector3 backgroundColor, bool verbose){
    if(!_IsRunning){
        _IsRunning = true;
        uint32_t width = _Image->getWidth();
        uint32_t height = _Image->getHeight();

        Timer timer{};
        if(verbose){
            fprintf(stdout, "Start ray tracing at `%dx%d' resolution...\n", width, height);
        }
        timer.start();
        _Image->clear(backgroundColor);

        auto primitives = getTriangles();

        
        for(uint j = 0; j<height; j++){
            #ifndef NDEBUG
            float progress = j / (height+1.f);
            displayProgressBar(progress);
            #endif

            for(uint32_t i = 0; i<width; i++){
                float u = (static_cast<float>(i) + 0.5f) / width;
                float v = 1.f - (static_cast<float>(j) + 0.5f) / height;
                Ray curRay = _Camera->rayAt(u,v);

                for(auto& triangle : primitives){
                    RayHitOpt hit = rayTriangleIntersection(curRay, triangle);
                    if(hit.has_value()){
                        _Image->set(i, j, shade(hit.value(), triangle));
                        break; // for now
                    }
                }
            }
        }

        if(verbose){
            fprintf(stdout, "\nRay tracing executed in `%d ms'\n", timer.getTicks());
        }
        _IsRunning = false;
    }
}
    
}