#include "../headers/glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()


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
    grid_program(0)

{
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    if (m_transparent) {
        QSurfaceFormat format;
        format.setAlphaBufferSize(8);
        format.setVersion(3, 3); // Set to OpenGL version 3.3
        format.setProfile(QSurfaceFormat::CoreProfile); // Using Core profile
        QSurfaceFormat::setDefaultFormat(format);

    }
      setFocusPolicy(Qt::StrongFocus);
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
    m_skyboxProgram = 0;
    m_sunProgram = 0;
    m_program = 0;
    grid_program = 0;
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

    m_program->bindAttributeLocation("fluid_vertex", 0);
    m_program->bindAttributeLocation("fluid_normal", 1);

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

    grid_program->bindAttributeLocation("vertex", 0);
    grid_program->bindAttributeLocation("normal", 1);

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

    grid = new GridGeometry();

    swefluid = new SWEFluid(100, 100);

    m_view.setToIdentity();
    m_view.translate(-50, 5, -60);
    m_view.rotate(45 / 16.0f, 1, 0, 0);

    // Skybox Shader Program
    m_skyboxProgram = new QOpenGLShaderProgram;
    m_skyboxProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/skybox_vshader.glsl");
    m_skyboxProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/skybox_fshader.glsl");
    m_skyboxProgram->link();

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
}


void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    m_projection.perspective(m_currentFoV, float(width()) / float(height()),  0.000001f, 100000.0f);
    update();
}


void GLWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        m_view.translate(0, 0, numSteps.y() * 1.0f); // Adjust 0.1f to control the speed of zoom
    }
    update();
}



void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_last_position = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_last_position.x();
    int dy = event->y() - m_last_position.y();

    if (event->buttons() & Qt::LeftButton) {
        float scaleFactor = 0.01;
        m_view.translate(-dx * scaleFactor, dy * scaleFactor, 0.0f);
    } else if (event->buttons() & Qt::RightButton) {
        float angleFactor = 0.5;
        m_view.rotate(dy * angleFactor, 1, 0, 0);
        m_view.rotate(dx * angleFactor, 0, 1, 0);
    }
    m_last_position = event->pos();
    update();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_T) {
        srand(time(NULL));

        int numberOfCircles = 5;
        int borderThickness = 1;

        for (int i = 0; i < numberOfCircles; ++i) {
            int centerX = rand() % 80+20;
            int centerY = rand() % 80+20;
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
        update();
    }
}
