#ifndef GRIDGEOMETRY_H
#define GRIDGEOMETRY_H
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QImage>

struct GridVertexData {
    QVector3D position; // Position of the vertex
    QVector3D normal;   // Normal vector
    QVector2D texCoord; // Texture coordinate
};

class GridGeometry : protected QOpenGLFunctions
{
public:
    GridGeometry();
    ~GridGeometry();

    void initGridGeometry();
    void drawGridGeometry(QOpenGLShaderProgram* program);

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    int gridWidth = 0, gridDepth = 0;
    unsigned char *data;
    QImage image;
};

#endif // GRIDGEOMETRY_H
