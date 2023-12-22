#ifndef SPHERE_H
#define SPHERE_H

#include <QVector3D>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <random>
#include <vector>
#include "GridGeometry.h"
#include "SWEFluid.h"

struct sphere_VertexData {
    QVector3D position;
    QVector3D velocity;
    QVector3D accumulatedForce;
    QVector3D offset;
};

class Sphere : protected QOpenGLFunctions {
public:
    Sphere( GridGeometry* _terrain,QVector3D CenterPosition, float radius, int numSegments,SWEFluid* _fluid);
    ~Sphere();
    void UpdateParticles(float dt);
    void render(QOpenGLShaderProgram* program);
    int mass =1000;
    std::vector<sphere_VertexData> vertices;

private:
    void createSphereGeometry(QVector3D CenterPosition, float radius, int numSegments) ;
    void handleGroundInteraction(QVector3D& newPosition, QVector3D& velocity, QVector3D& accumulatedForce, float groundHeight, float dt);
    void handleBuoyancy(QVector3D& newPosition, QVector3D& velocity, QVector3D& accumulatedForce, float waterHeight, float dt);
    float timevalue = 0.0f;
    QVector3D position;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf; // Index buffer for triangles

    QVector<GLuint> indices;
    float gravity = -9.81;
    float restitutionCoefficient = 2; // Adjust as needed for particle rebound
    GridGeometry* terrain;
    float slidingCoefficient = 2.0f; // Example value, adjust as needed
    float frictionCoefficient =0.4f;

    SWEFluid* fluid;

};

#endif // SPHERE_H
