#ifndef SQUARE_H
#define SQUARE_H
#include "Vec3.h"
#include <vector>
#include "Mesh.h"
#include <cmath>

struct RaySquareIntersection{
    bool intersectionExists;
    float t;
    float u,v;
    Vec3 intersection;
    Vec3 normal;
};


class Square : public Mesh {
public:
    Vec3 m_normal;
    Vec3 m_bottom_left;
    Vec3 m_right_vector;
    Vec3 m_up_vector;

    Square() : Mesh() {}
    Square(Vec3 const & bottomLeft , Vec3 const & rightVector , Vec3 const & upVector , float width=1. , float height=1. ,
           float uMin = 0.f , float uMax = 1.f , float vMin = 0.f , float vMax = 1.f) : Mesh() {
        setQuad(bottomLeft, rightVector, upVector, width, height, uMin, uMax, vMin, vMax);
    }

    void setQuad( Vec3 const & bottomLeft , Vec3 const & rightVector , Vec3 const & upVector , float width=1. , float height=1. ,
                  float uMin = 0.f , float uMax = 1.f , float vMin = 0.f , float vMax = 1.f) {
        m_right_vector = rightVector;
        m_up_vector = upVector;
        m_normal = Vec3::cross(rightVector , upVector);
        m_bottom_left = bottomLeft;

        m_normal.normalize();
        m_right_vector.normalize();
        m_up_vector.normalize();

        m_right_vector = m_right_vector*width;
        m_up_vector = m_up_vector*height;

        vertices.clear();
        vertices.resize(4);
        vertices[0].position = bottomLeft;                                      vertices[0].u = uMin; vertices[0].v = vMin;
        vertices[1].position = bottomLeft + m_right_vector;                     vertices[1].u = uMax; vertices[1].v = vMin;
        vertices[2].position = bottomLeft + m_right_vector + m_up_vector;       vertices[2].u = uMax; vertices[2].v = vMax;
        vertices[3].position = bottomLeft + m_up_vector;                        vertices[3].u = uMin; vertices[3].v = vMax;
        vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = m_normal;
        triangles.clear();
        triangles.resize(2);
        triangles[0][0] = 0;
        triangles[0][1] = 1;
        triangles[0][2] = 2;
        triangles[1][0] = 0;
        triangles[1][1] = 2;
        triangles[1][2] = 3;


    }

    void build_arrays() {
        Mesh::build_arrays();
        recomputeQuad();
    }

    void recomputeQuad() {
        m_right_vector = vertices[1].position - vertices[0].position;
        m_up_vector = vertices[3].position - vertices[0].position;
        m_bottom_left = vertices[0].position;
        m_normal = Vec3::cross(m_right_vector, m_up_vector);

        m_right_vector.normalize();
        m_up_vector.normalize();
        m_normal.normalize();

        vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = m_normal;
    }

    RaySquareIntersection intersect(const Ray &ray) const {
        RaySquareIntersection intersection;
        intersection.t = FLT_MAX;
        intersection.intersectionExists = false;

        Vec3 origin = ray.origin();
        Vec3 direction = ray.direction();
        Vec3 point = 0.5f*(vertices[0].position + vertices[2].position);

        float numerator = Vec3::dot(point - origin, m_normal);
        float denominator = Vec3::dot(m_normal, direction);

        if(denominator > -0.0001f) return intersection;

        float t = numerator/denominator;

        if(t > 100000.f) return intersection;

        if(t < 0.0001f) return intersection;

        Vec3 AB = vertices[1].position - vertices[0].position;
        Vec3 AC = vertices[3].position - vertices[0].position;

        Vec3 intersectionPoint = origin + t*direction;
        Vec3 AM = intersectionPoint - vertices[0].position;

        if(Vec3::dot(AB, AM) < 0.f || Vec3::dot(AB, AM) > Vec3::dot(AB, AB)) return intersection;
        if(Vec3::dot(AC, AM) < 0.f || Vec3::dot(AC, AM) > Vec3::dot(AC, AC)) return intersection;

        intersection.t = t;
        intersection.intersectionExists = true;
        intersection.intersection = intersectionPoint;
        intersection.normal = m_normal;

        return intersection;
    }
};
#endif // SQUARE_H
