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
#include "sphere.h"
#include "camera.h"
#include "WaterFrameBuffers.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();
    GridGeometry* grid = nullptr;

    std::vector<Sphere*> spheres;
    static bool isTransparent() { return m_transparent; }
    static void setTransparent(bool t) { m_transparent = t; }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setFoV(float fov);
    void ChangeTerrain(int index);

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
    void updateSpheres(float dt);
    void resolveSphereCollisions();
    void initializeSkybox();
    void RenderScene(bool withWater,QVector4D clippingPlane);

    float m_currentFoV;
    bool m_core;
    QPoint m_last_position;
    QOpenGLShaderProgram *m_program;
    QOpenGLShaderProgram* m_skyboxProgram;
    QOpenGLShaderProgram* m_sunProgram;
    QOpenGLShaderProgram* grid_program;
    QOpenGLShaderProgram* m_sphereProgram;

    int m_mvp_matrix_loc;
    int m_normal_matrix_loc;
    int m_light_pos_loc;
    int m_model_matrix_loc;
    int reflect_texture_loc;
    int refract_texture_loc;
    int timeLocation;
    int cameraposloc;

    int grid_mvp_matrix_loc;
    int grid_normal_matrix_loc;
    int grid_light_pos_loc;
    int grid_plane_loc;

    int sphere_mvp_matrix_loc;
    int sphere_normal_matrix_loc;
    int sphere_light_pos_loc;
    int sphere_plane_loc;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    Camera* camera;

    QMatrix4x4 m_model;
    static bool m_transparent;

    SWEFluid* swefluid = nullptr;
    WaterFrameBuffers* waterFrameBuffers = nullptr;


    QOpenGLVertexArrayObject sunVAO;
    QOpenGLBuffer sunVBO;

    QOpenGLVertexArrayObject skyboxVAO;
    QOpenGLBuffer skyboxVBO;
    GLuint cubemapTexture;

    QVector3D sunPosition;

    QPoint lastPos;

    QVector3D rayOrigin;
    QVector3D rayEnd;

    float time = 0;

};

#endif
