#ifndef CUBE_H
#define CUBE_H

#include <QVector3D>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <random>
#include <vector>
#include "../headers/GridGeometry.h"

struct cube_VertexData {
    QVector3D position;
    QVector3D velocity;
    QVector3D accumulatedForce;
};

class Cube : protected QOpenGLFunctions {
public:
    Cube( GridGeometry* _terrain);
    ~Cube();
    void UpdateParticles(float dt);
    void render(QOpenGLShaderProgram* program);
    void createPointGeometry(QVector3D InstantiatedPosition);

private:
    QVector3D position;
    QOpenGLBuffer arrayBuf;
    std::vector<cube_VertexData> vertices;
    float gravity = -9.81;
    float restitutionCoefficient = 2; // Adjust as needed for particle rebound
    GridGeometry* terrain;
    float slidingCoefficient = 2.0f; // Example value, adjust as needed
    float frictionCoefficient =0.4f;

};

#endif // CUBE_H
