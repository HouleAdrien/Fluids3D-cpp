#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QTimer>
#include "SWEFluid.h"
#include "GridGeometry.h"
#include "cube.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();
    GridGeometry* grid = nullptr;

    Cube* cube= nullptr;
    static bool isTransparent() { return m_transparent; }
    static void setTransparent(bool t) { m_transparent = t; }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setFoV(float fov);


public slots:
    void cleanup();
     void updateSimulation();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupVertexAttribs();
    void updateProjectionMatrix();
    void renderTestPoint(const QVector3D& point);

    float m_currentFoV;
    bool m_core;
    QPoint m_last_position;
    QOpenGLShaderProgram *m_program;
    QOpenGLShaderProgram* m_skyboxProgram;
    QOpenGLShaderProgram* m_sunProgram;
    QOpenGLShaderProgram* grid_program;
    QOpenGLShaderProgram* m_cubeProgram;
    QOpenGLShaderProgram* rayProgram;

    int m_mvp_matrix_loc;
    int m_normal_matrix_loc;
    int m_light_pos_loc;


    int grid_mvp_matrix_loc;
    int grid_normal_matrix_loc;
    int grid_light_pos_loc;

    int cube_mvp_matrix_loc;
    int cube_normal_matrix_loc;
    int cube_light_pos_loc;


    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    static bool m_transparent;

    SWEFluid* swefluid = nullptr;


    QOpenGLVertexArrayObject sunVAO;
    QOpenGLBuffer sunVBO;

    QOpenGLVertexArrayObject skyboxVAO;
    QOpenGLBuffer skyboxVBO;

    QVector3D sunPosition;

    QPoint lastPos;

    QVector3D cameraPosition;
    QVector3D rayOrigin;
    QVector3D rayEnd;
    bool drawRay = false;

};

#endif
