#ifndef SPHERE_H
#define SPHERE_H

#include <QVector3D>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <random>
#include <vector>
#include "GridGeometry.h"

struct sphere_VertexData {
    QVector3D position;
    QVector3D velocity;
    QVector3D accumulatedForce;
    QVector3D offset;
};

class Sphere : protected QOpenGLFunctions {
public:
    Sphere( GridGeometry* _terrain,QVector3D CenterPosition, float radius, int numSegments);
    ~Sphere();
    void UpdateParticles(float dt);
    void render(QOpenGLShaderProgram* program);
  
private:
    void createSphereGeometry(QVector3D CenterPosition, float radius, int numSegments) ;

    float timevalue = 0.0f;
    QVector3D position;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf; // Index buffer for triangles
    std::vector<sphere_VertexData> vertices;
    QVector<GLuint> indices;
    float gravity = -9.81;
    float restitutionCoefficient = 2; // Adjust as needed for particle rebound
    GridGeometry* terrain;
    float slidingCoefficient = 2.0f; // Example value, adjust as needed
    float frictionCoefficient =0.4f;

};

#endif // SPHERE_H
