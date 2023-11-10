#ifndef MAILLAGE_H
#define MAILLAGE_H
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Maillage : protected QOpenGLFunctions
{
public:
    Maillage();
    ~Maillage();

    void initGridGeometry();
    void drawGridGeometry(QOpenGLShaderProgram* program);

private:
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    int gridWidth, gridHeight;
};

#endif // MAILLAGE_H
