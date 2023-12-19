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

    void setTextureHeightMaps(const QImage& image0, const QImage& image1, const QImage& image2, const QImage& image3);
    void setTextureHeightMapsFromPaths(const QString& path0, const QString& path1, const QString& path2, const QString& path3);
    void setHeightMap(const QImage& image);
    float getHeightAtPosition(float x, float z);
    QVector3D getVertex(int index) const;
    GLushort getIndex(int index) const;
    QVector2D getGradientAtPosition(float x, float z);
    QImage heightMapTexture;
    float maxHeight = 30;
private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    int gridWidth , gridDepth ;


    QOpenGLTexture* gTextureHeight0;   // Texture for height range 0
    QOpenGLTexture* gTextureHeight1;   // Texture for height range 1
    QOpenGLTexture* gTextureHeight2;   // Texture for height range 2
    QOpenGLTexture* gTextureHeight3;

    std::vector<GridVertexData> vertices;
     std::vector<GLushort> indices;

};

#endif // GRIDGEOMETRY_H
