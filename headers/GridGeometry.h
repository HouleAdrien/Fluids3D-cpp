#ifndef GRIDGEOMETRY_H
#define GRIDGEOMETRY_H
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QImage>
#include <QOpenGLTexture>

struct GridVertexData {
    QVector3D position; // Position of the vertex
    QVector3D normal;   // Normal vector
    QVector2D texCoord; // Texture coordinate
};

class GridGeometry : protected QOpenGLFunctions
{
public:
    GridGeometry(int _gridWidth,int _gridDepth);
    ~GridGeometry();

    void initGridGeometry();
    void drawGridGeometry(QOpenGLShaderProgram* program);

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    int gridWidth , gridDepth ;
    void setHeightMap(const QImage& image);
    QOpenGLTexture* heightMapTexture;
    float maxHeight = 30;

};

#endif // GRIDGEOMETRY_H
