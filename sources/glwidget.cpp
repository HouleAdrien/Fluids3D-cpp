#include "../headers/glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>

bool GLWidget::m_transparent = false;

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    m_currentFoV(60.0f),
    m_program(0)
{
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    if (m_transparent) {
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        setFormat(fmt);
    }
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
    if (m_program == nullptr)
        return;
    makeCurrent();
    delete maillage;
    delete m_program;
    m_program = 0;
    doneCurrent();
}

void GLWidget::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, m_transparent ? 0 : 1);

    m_program = new QOpenGLShaderProgram;
    // Compile vertex shader
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    m_program->bindAttributeLocation("vertex", 0);
    m_program->bindAttributeLocation("normal", 1);

    // Link shader pipeline
    if (!m_program->link())
        close();

    // Bind shader pipeline for use
    if (!m_program->bind())
        close();

    m_mvp_matrix_loc = m_program->uniformLocation("mvp_matrix");
    m_normal_matrix_loc = m_program->uniformLocation("normal_matrix");
    m_light_pos_loc = m_program->uniformLocation("light_position");

    maillage = new Maillage;

    // Our camera never changes in this example.
    m_view.setToIdentity();
    m_view.translate(-5.11658, -1.17886, -12.3675);

    // Light position is fixed.
    m_program->setUniformValue(m_light_pos_loc, QVector3D(0, 0, 70));

    m_program->release();


}



void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);

    m_model.setToIdentity();
    m_model.rotate( (180 / 16.0f), 1, 0, 0);
    m_model.rotate(180 / 16.0f, 0, 1, 0);
    m_model.rotate(0 / 16.0f, 0, 0, 1);

    m_program->bind();

    // Set modelview-projection matrix
    m_program->setUniformValue(m_mvp_matrix_loc, m_projection * m_view * m_model);
    QMatrix3x3 normal_matrix = m_model.normalMatrix();

    // Set normal matrix
    m_program->setUniformValue(m_normal_matrix_loc, normal_matrix);

    maillage->drawGridGeometry(m_program);

    m_program->release();
}

void GLWidget::resizeGL(int w, int h)
{
    if (h == 0) h = 1; // Prevent division by zero
    m_projection.setToIdentity();
    m_projection.perspective(m_currentFoV, float(w) / float(h),  0.000001f, 100000.0f);
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
