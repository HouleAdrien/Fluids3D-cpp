#include "../headers/glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()
#include <QPainter>
#include <QApplication>
#include <QScreen>


float skyboxVertices[] = {
    // Positions
    -500.0f,  500.0f, -500.0f,
    -500.0f, -500.0f, -500.0f,
    500.0f, -500.0f, -500.0f,
    500.0f, -500.0f, -500.0f,
    500.0f,  500.0f, -500.0f,
    -500.0f,  500.0f, -500.0f,

    -500.0f, -500.0f,  500.0f,
    -500.0f, -500.0f, -500.0f,
    -500.0f,  500.0f, -500.0f,
    -500.0f,  500.0f, -500.0f,
    -500.0f,  500.0f,  500.0f,
    -500.0f, -500.0f,  500.0f,

    500.0f, -500.0f, -500.0f,
    500.0f, -500.0f,  500.0f,
    500.0f,  500.0f,  500.0f,
    500.0f,  500.0f,  500.0f,
    500.0f,  500.0f, -500.0f,
    500.0f, -500.0f, -500.0f,

    -500.0f, -500.0f,  500.0f,
    -500.0f,  500.0f,  500.0f,
    500.0f,  500.0f,  500.0f,
    500.0f,  500.0f,  500.0f,
    500.0f, -500.0f,  500.0f,
    -500.0f, -500.0f,  500.0f,

    -500.0f,  500.0f, -500.0f,
    500.0f,  500.0f, -500.0f,
    500.0f,  500.0f,  500.0f,
    500.0f,  500.0f,  500.0f,
    -500.0f,  500.0f,  500.0f,
    -500.0f,  500.0f, -500.0f,

    -500.0f, -500.0f, -500.0f,
    -500.0f, -500.0f,  500.0f,
    500.0f, -500.0f, -500.0f,
    500.0f, -500.0f, -500.0f,
    -500.0f, -500.0f,  500.0f,
    500.0f, -500.0f,  500.0f
};


// Initialize geometry for the sun (quad)
float sunVertices[] = {
    // positions
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};

bool GLWidget::m_transparent = false;

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    m_currentFoV(60.0f),
    m_program(0),
    m_skyboxProgram(0),
    m_sunProgram(0),
    grid_program(0),
    m_sphereProgram(0),
    rayProgram(0),
    lastPos(0, 0)

{
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;


    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setAlphaBufferSize(8);
    format.setVersion(4, 0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);



    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

GLWidget::~GLWidget()
{
    cleanup();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(800, 800);
}

QSize GLWidget::sizeHint() const
{
    return QSize(800, 800);
}


void GLWidget::cleanup()
{
    makeCurrent();
    delete swefluid;
    delete m_program;
    delete grid_program;
    delete m_skyboxProgram;
    delete m_sunProgram;
    delete m_sphereProgram;
    delete rayProgram;
    m_sphereProgram = 0;
    m_skyboxProgram = 0;
    m_sunProgram = 0;
    m_program = 0;
    grid_program = 0;
    rayProgram = 0;
    doneCurrent();
}

void GLWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, m_transparent ? 0 : 1);

    //FLUID
    m_program = new QOpenGLShaderProgram;
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Define the sun's position
    QVector3D sunPosition = QVector3D(50, 100, 50);


    if (!m_program->link())
        close();
    if (!m_program->bind())
        close();

    m_mvp_matrix_loc = m_program->uniformLocation("mvp_matrix");
    m_normal_matrix_loc = m_program->uniformLocation("normal_matrix");
    m_light_pos_loc = m_program->uniformLocation("light_position");

    // Set the light position to the sun's position
    m_program->setUniformValue(m_light_pos_loc, sunPosition);

    m_program->release();



    //TEST GRID
    grid_program = new QOpenGLShaderProgram;
    if (!grid_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grid_vshader.glsl"))
        close();
    if (!grid_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grid_fshader.glsl"))
        close();

    if (!grid_program->link())
        close();
    if (!grid_program->bind())
        close();

    grid_mvp_matrix_loc = grid_program->uniformLocation("mvp_matrix");
    grid_normal_matrix_loc = grid_program->uniformLocation("normal_matrix");
    grid_light_pos_loc = grid_program->uniformLocation("light_position");

    // Set the light position to the sun's position
    grid_program->setUniformValue(grid_light_pos_loc, sunPosition);

    grid_program->release();

    grid = new GridGeometry(200,200);

    swefluid = new SWEFluid(200, 200);

    camera= new Camera({100,20,100});
    m_view =  camera->GetViewMatrix();

    // Skybox Shader Program
    m_skyboxProgram = new QOpenGLShaderProgram;
    m_skyboxProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/skybox_vshader.glsl");
    m_skyboxProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/skybox_fshader.glsl");
    m_skyboxProgram->link();

    m_sphereProgram = new QOpenGLShaderProgram;
    m_sphereProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/sphere_vshader.glsl");

    m_sphereProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/sphere_fshader.glsl");

    if (!m_sphereProgram->link())
        close();
    if (!m_sphereProgram->bind())
        close();

    sphere_mvp_matrix_loc = m_sphereProgram->uniformLocation("mvp_matrix");
    sphere_normal_matrix_loc = m_sphereProgram->uniformLocation("normal_matrix");
    sphere_light_pos_loc = m_sphereProgram->uniformLocation("light_position");

    // Set the light position to the sun's position
    m_sphereProgram->setUniformValue(sphere_light_pos_loc, sunPosition);




    m_sphereProgram->release();

    // Sun Shader Program
    m_sunProgram = new QOpenGLShaderProgram;
    m_sunProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/sun_vshader.glsl");
    m_sunProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/sun_fshader.glsl");
    m_sunProgram->link();

    // Sun geometry setup
    sunVAO.create();
    sunVAO.bind();
    sunVBO.create();
    sunVBO.bind();
    sunVBO.allocate(sunVertices, sizeof(sunVertices));
    // Setup vertex attribute pointers for sun
    sunVAO.release();

    // Skybox geometry setup
    skyboxVAO.create();
    skyboxVAO.bind();
    skyboxVBO.create();
    skyboxVBO.bind();
    skyboxVBO.allocate(skyboxVertices, sizeof(skyboxVertices));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    skyboxVAO.release();


    rayProgram = new QOpenGLShaderProgram(this);

    // Load and compile the vertex shader
    if (!rayProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/ray_vshader.glsl")) {
        qDebug() << "Error compiling vertex shader:" << rayProgram->log();
        return;
    }

    // Load and compile the fragment shader
    if (!rayProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/ray_fshader.glsl")) {
        qDebug() << "Error compiling fragment shader:" << rayProgram->log();
        return;
    }

    // Link the shaders
    if (!rayProgram->link()) {
        qDebug() << "Error linking shader program:" << rayProgram->log();
        return;
    }


}



void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_view = camera->GetViewMatrix();

    // Configuration de la matrice de transformation
    m_model.setToIdentity();
    m_model.rotate(180 / 16.0f, 1, 0, 0);
    m_model.rotate(180 / 16.0f, 0, 1, 0);
    m_model.rotate(0 / 16.0f, 0, 0, 1);

    m_program->bind();

    // Mise à jour de la matrice MVP (Model-View-Projection)
    m_program->setUniformValue(m_mvp_matrix_loc, m_projection * m_view * m_model);
    QMatrix3x3 normal_matrix = m_model.normalMatrix();
    m_program->setUniformValue(m_normal_matrix_loc, normal_matrix);

    // Dessin de la grille
    swefluid->drawGridGeometry(m_program);

    m_program->release();

    updateSimulation();

    //render grid

    grid_program->bind();

    grid_program->setUniformValue(grid_mvp_matrix_loc, m_projection * m_view * m_model);
    QMatrix3x3 grid_normal_matrix = m_model.normalMatrix();
    grid_program->setUniformValue(grid_normal_matrix_loc, grid_normal_matrix);

    // Dessin de la grille
    grid->drawGridGeometry(grid_program);

    grid_program->release();


    m_sphereProgram->bind();

    m_sphereProgram->setUniformValue(sphere_mvp_matrix_loc, m_projection * m_view * m_model);
    QMatrix3x3 sphere_normal_matrix = m_model.normalMatrix();
    m_sphereProgram->setUniformValue(sphere_normal_matrix_loc, sphere_normal_matrix);

    for(int i = 0; i < spheres.size() ; i++){

        spheres[i]->UpdateParticles(0.03f);
        spheres[i]->render(m_sphereProgram);


    }
    m_sphereProgram->release();


    // Render Skybox
    m_skyboxProgram->bind();
    skyboxVAO.bind();
    QMatrix4x4 viewMatrixNoTranslation = m_view;
    viewMatrixNoTranslation.setColumn(3, QVector4D(0, 0, 0, 1));
    QMatrix4x4 mvpMatrixSkybox = m_projection * viewMatrixNoTranslation;
    m_skyboxProgram->setUniformValue("mvp_matrix", mvpMatrixSkybox);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(skyboxVertices) / (3 * sizeof(float))); // Using the size of skyboxVertices array
    skyboxVAO.release();
    m_skyboxProgram->release();

    // Render Sun
    m_sunProgram->bind();
    sunVAO.bind();
    QMatrix4x4 modelMatrixSun;
    modelMatrixSun.translate(50, 10, 50);
    modelMatrixSun.scale(5);
    QMatrix4x4 mvpMatrixSun = m_projection * m_view * modelMatrixSun;
    m_sunProgram->setUniformValue("mvp_matrix", mvpMatrixSun);
    QVector3D sunColor(1.0, 1.0, 0.8); // Soft yellow-white
    m_sunProgram->setUniformValue("lightColor", sunColor);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(sunVertices) / (3 * sizeof(float))); // Using the size of sunVertices array
    sunVAO.release();
    m_sunProgram->release();


    if (drawRay) {
        glLineWidth(100.0f);
        // Set up the program for drawing the ray
        rayProgram->bind(); // Assuming you have a separate shader program for the ray

        // Set the MVP matrix for the ray
        rayProgram->setUniformValue("mvp_matrix", m_projection * m_view * m_model);

        // Define vertices for the ray line
        GLfloat rayVertices[] = {
            rayOrigin.x(), rayOrigin.y(), rayOrigin.z(),
            rayEnd.x(), rayEnd.y(), rayEnd.z()
        };

        // Create a temporary VBO
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), rayVertices, GL_STATIC_DRAW);

        // Position attribute
        GLint posAttrib = rayProgram->attributeLocation("position"); // Replace 'position' with your actual attribute name in shader
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

        glDrawArrays(GL_LINES, 0, 2); // Draw the line

        // Clean up
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &VBO);

        rayProgram->release();
        glLineWidth(1.0f);
    }

    update();

}

void GLWidget::updateSimulation() {

    swefluid->ShallowWaterStep(0.0001f);
    swefluid->updateVertexBuffer();

    update(); // Request to redraw the widget
}

void GLWidget::resizeGL(int w, int h) {
    if (h == 0) h = 1; // Prévenir la division par zéro
    m_projection.setToIdentity();
    m_projection.perspective(m_currentFoV, float(w) / float(h), 0.1f, 1000.0f);
}

void GLWidget::setFoV(float fov)
{
    m_currentFoV = fov;
    updateProjectionMatrix();
}

void GLWidget::updateProjectionMatrix()
{
    m_projection.setToIdentity();
    m_projection.perspective(m_currentFoV, float(width()) / float(height()),   0.1f, 1000.0f);
    update();
}


void GLWidget::wheelEvent(QWheelEvent *event) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        float zoomSpeed = 0.1f; // Adjust as needed
        camera->ProcessKeyboard(numSteps.y() > 0 ? Qt::Key_Up : Qt::Key_Down, zoomSpeed);
    }
    update();
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Retrieve the size of the OpenGL context
        QSize viewportSize = this->size();
        float screenWidth = viewportSize.width();
        float screenHeight = viewportSize.height();

        // Convert the mouse click position to normalized device coordinates (NDC)
        float x = (2.0f * event->x()) / screenWidth - 1.0f;
        float y = 1.0f - (2.0f * event->y()) / screenHeight;

        // Convert NDC to clip coordinates
        QVector4D rayClip = QVector4D(x, y, -1.0f, 1.0f);

        // Convert clip coordinates to eye coordinates
        QMatrix4x4 inverseProjectionMatrix = m_projection.inverted();
        QVector4D rayEye = inverseProjectionMatrix * rayClip;
        rayEye = QVector4D(rayEye.x(), rayEye.y(), -1.0f, 0.0f);

        // Convert to world coordinates
        QMatrix4x4 inverseViewMatrix = camera->GetViewMatrix().inverted();
        QVector3D rayWorld = inverseViewMatrix.map(rayEye.toVector3D()).normalized();

        // Calculate the ray's origin and both directions
        QVector3D rayOrigin = camera->Position;
        QVector3D forwardRayDirection = rayWorld;
        QVector3D backwardRayDirection = -rayWorld;

        // Set the ray's end points for both directions
        float rayLength = 100.0f; // Adjust the length of the ray as needed
        QVector3D forwardRayEnd = rayOrigin + forwardRayDirection * rayLength;
        QVector3D backwardRayEnd = rayOrigin + backwardRayDirection * rayLength;

        drawRay = true;

        // Intersection test for the forward ray
        QVector3D forwardHitPoint;
        if (grid->intersectsRay(rayOrigin, forwardRayDirection, rayLength, forwardHitPoint)) {
            // Sphere* newSphere = new Sphere(grid,{ forwardHitPoint.x(), forwardHitPoint.y() + 30, forwardHitPoint.z()},0.5f,12);
            // spheres.push_back(newSphere);
        }

        rayOrigin = backwardRayEnd;
        QVector3D backwardHitPoint;
        if (grid->intersectsRay(rayOrigin, backwardRayDirection, rayLength, backwardHitPoint)) {
            // Sphere* newSphere = new Sphere(grid,{ backwardHitPoint.x(), backwardHitPoint.y() + 30, backwardHitPoint.z()},0.5f,12);
            // spheres.push_back(newSphere);
        }

        rayEnd = forwardRayEnd;
    }


}




void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    QPoint currentPos = event->pos();

    if (!lastPos.isNull()) {
        // Calculate the difference in position
        float deltaX = currentPos.x() - lastPos.x();
        float deltaY = currentPos.y() - lastPos.y();

        const float sensitivity = 2.0f; // Adjust sensitivity as needed

        // Update camera based on the delta movement
        camera->ProcessMouseMovement( deltaX* sensitivity, (-deltaY) * sensitivity);
    }

    lastPos = currentPos;
    update();
}


void GLWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_T) {
        srand(time(NULL));

        int numberOfCircles = 5;
        int borderThickness = 1;

        for (int i = 0; i < numberOfCircles; ++i) {
            int centerX = rand() % 150+20;
            int centerY = rand() % 150+20;
            int diameter = 10;
            int radius = diameter / 2;

            for (int x = centerX - radius; x <= centerX + radius; ++x) {
                for (int y = centerY - radius; y <= centerY + radius; ++y) {
                    int dx = x - centerX;
                    int dy = y - centerY;
                    int distanceSquared = dx * dx + dy * dy;
                    if (distanceSquared <= radius * radius && distanceSquared >= (radius - borderThickness) * (radius - borderThickness)) {
                        swefluid->setWaterHeightAt(x, y, 10);  // Set height to 10
                    }
                }
            }
        }

        swefluid->updateVertexBuffer();
        //   update();
    }

    if (event->key() == Qt::Key_P) {
        Sphere* newSphere = new Sphere(grid,{ float(100), 50, float(100)},0.5f,12);
        spheres.push_back(newSphere);

    }

    camera->ProcessKeyboard(event->key() ,1.0f);

    update();



}
