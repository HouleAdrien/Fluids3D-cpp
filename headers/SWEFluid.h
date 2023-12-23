#ifndef SWEFLUID_H
#define SWEFLUID_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QVector2D>
#include "GridGeometry.h"
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
    float isBorder; // Is the point of fluid a border
};

enum class Border {
    South,
    East,
    West,
    North
};


class SWEFluid : protected QOpenGLFunctions {
public:
    SWEFluid(GridGeometry* _grid,int _BaseWaterHeight);
    ~SWEFluid();

    int gridWidth, gridDepth;
    int BaseWaterHeight;
    void initGridGeometry();
    void drawGridGeometry(QOpenGLShaderProgram* program);
    void ShallowWaterStep( float dt);
    void updateVertexBuffer();
    float getWaterHeight(float x, float z);
    void CreateInitialWave(Border border);

    GridGeometry* grid;
private:
    void computeNormal();
    QVector3D calculateNormal(float heightLeft, float heightRight, float heightDown, float heightUp);

    void SpreadBorderToGround();
    std::vector<std::vector<int>> vertexIndexMap;
    void Advect(std::vector<VertexData>& deplacement, const std::vector<VertexData>& source, float dt );
    void Update_height( float dt);
    void Update_velocities( float dt);
    float InterpolateWaterH(const std::vector<VertexData>& source, float x, float y);
    QVector2D InterpolateVelocity(const std::vector<VertexData>& source, float x, float y) ;
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    std::vector<VertexData> vertices;
    std::vector<VertexData> globalGridInfos;

    float gravity=-9.81f;
};

#endif // SWEFLUID_H
