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
    if (m_program == nullptr)
        return;
    makeCurrent();
    delete swefluid;
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

    swefluid=  new SWEFluid(100,100);

    // Our camera never changes in this example.
    m_view.setToIdentity();
    m_view.translate(-50, 5, -60); // Reculer et élever la caméra
    m_view.rotate(45/ 16.0f, 1, 0, 0);

    // Light position is fixed.
    m_program->setUniformValue(m_light_pos_loc, QVector3D(50, 10, 50));

    m_program->release();


}


void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

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
    update();

}

void GLWidget::updateSimulation() {

    swefluid->ShallowWaterStep(0.00001f);
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
        int centerX = 50;
        int centerY = 50;
        int radius = 6 / 2;  // Diameter is 20, so radius is 10
        int borderThickness = 1;  // Thickness of the border, adjust as needed

        // Iterate over a square that bounds the circle
        for (int x = centerX - radius; x <= centerX + radius; ++x) {
            for (int y = centerY - radius; y <= centerY + radius; ++y) {
                // Calculate distance from the center
                int dx = x - centerX;
                int dy = y - centerY;
                int distanceSquared = dx * dx + dy * dy;

                // Check if point is on the border of the circle
                if (distanceSquared <= radius * radius && distanceSquared >= (radius - borderThickness) * (radius - borderThickness)) {
                    // Point is on the border, set the water height
                    swefluid->setWaterHeightAt(x, y, 10);  // Set height to 10
                }
            }
        }

        swefluid->updateVertexBuffer();
        update();
    }
}
