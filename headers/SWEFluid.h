#ifndef SWEFLUID_H
#define SWEFLUID_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QVector2D>
#include <vector>
#include <QElapsedTimer>

struct VertexData {
    QVector3D position; // Position of the vertex
    QVector3D normal;   // Normal vector
    QVector2D texCoord; // Texture coordinate
    float waterHeight;  // Height of the water
    QVector2D velocity; // 2D velocity vector (vx, vz)
    float groundHeight; // Height of the ground
    float fluidHeight;  // Height of the fluid
};

class SWEFluid : protected QOpenGLFunctions {
public:
    SWEFluid(int gridWidth,int gridDepth);
    ~SWEFluid();

    int gridWidth, gridDepth;
    void setWaterHeightAt(int x, int y, double height) ;
    void initGridGeometry();
    void drawGridGeometry(QOpenGLShaderProgram* program);
    void ShallowWaterStep( float dt);
    void updateVertexBuffer();

    float Dx, Dy;

private:
    void Advect(std::vector<VertexData>& deplacement, const std::vector<VertexData>& source, float dt );
    void Update_height( float dt);
    void Update_velocities( float dt);
    float Interpolate(const std::vector<VertexData>& source, float x, float y);
    QVector2D InterpolateVelocity(const std::vector<VertexData>& source, float x, float y) ;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    std::vector<VertexData> vertices;
    float gravity=9.81f;
    float safetyFactor=1;// A value between 0.5 and 1 to ensure stability.

};

#endif // SWEFLUID_H
