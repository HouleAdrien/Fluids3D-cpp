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
    -250.0f,  250.0f, -250.0f,
    -250.0f, -250.0f, -250.0f,
    250.0f, -250.0f, -250.0f,
    250.0f, -250.0f, -250.0f,
    250.0f,  250.0f, -250.0f,
    -250.0f,  250.0f, -250.0f,

    -250.0f, -250.0f,  250.0f,
    -250.0f, -250.0f, -250.0f,
    -250.0f,  250.0f, -250.0f,
    -250.0f,  250.0f, -250.0f,
    -250.0f,  250.0f,  250.0f,
    -250.0f, -250.0f,  250.0f,

    250.0f, -250.0f, -250.0f,
    250.0f, -250.0f,  250.0f,
    250.0f,  250.0f,  250.0f,
    250.0f,  250.0f,  250.0f,
    250.0f,  250.0f, -250.0f,
    250.0f, -250.0f, -250.0f,

    -250.0f, -250.0f,  250.0f,
    -250.0f,  250.0f,  250.0f,
    250.0f,  250.0f,  250.0f,
    250.0f,  250.0f,  250.0f,
    250.0f, -250.0f,  250.0f,
    -250.0f, -250.0f,  250.0f,

    -250.0f,  250.0f, -250.0f,
    250.0f,  250.0f, -250.0f,
    250.0f,  250.0f,  250.0f,
    250.0f,  250.0f,  250.0f,
    -250.0f,  250.0f,  250.0f,
    -250.0f,  250.0f, -250.0f,

    -250.0f, -250.0f, -250.0f,
    -250.0f, -250.0f,  250.0f,
    250.0f, -250.0f,  250.0f,
    250.0f, -250.0f,  250.0f,
    250.0f, -250.0f, -250.0f,
    -250.0f, -250.0f, -250.0f

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
    format.setSamples(16);
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
    waterFrameBuffers->cleanUp();
    delete waterFrameBuffers;
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
    glEnable(GL_CLIP_DISTANCE0);
    //FLUID
    m_program = new QOpenGLShaderProgram;
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Define the sun's position
    QVector3D sunPosition = QVector3D(100, 200, 100);


    if (!m_program->link())
        close();
    if (!m_program->bind())
        close();

    m_mvp_matrix_loc = m_program->uniformLocation("mvp_matrix");
    m_normal_matrix_loc = m_program->uniformLocation("normal_matrix");
    m_model_matrix_loc = m_program->uniformLocation("model_matrix");
    m_light_pos_loc = m_program->uniformLocation("light_position");
    reflect_texture_loc = m_program->uniformLocation("reflectionTexture");
    refract_texture_loc = m_program->uniformLocation("refractionTexture");
    cameraposloc = m_program->uniformLocation("cameraPos");
    m_program->setUniformValue(reflect_texture_loc,0);
    m_program->setUniformValue(refract_texture_loc,1);

    QImage dudvmap(":/Images/waterDUDV.png");
    QOpenGLTexture* dudvmapTexture = new QOpenGLTexture(dudvmap);

    QImage normalMap(":/Images/normalMap.png");
    QOpenGLTexture* normalMapTexture = new QOpenGLTexture(normalMap);

    auto bindTexture= [&](QOpenGLTexture* texture, GLenum textureUnit, const char* uniformName, int unitIndex) {
        if (texture) {
            glActiveTexture(textureUnit);
            texture->bind();
            m_program->setUniformValue(uniformName, unitIndex);
        }
    };
    bindTexture(dudvmapTexture,GL_TEXTURE2,"dudvMap",2);
    bindTexture(normalMapTexture,GL_TEXTURE3,"normalMap",3);

    timeLocation = m_program->uniformLocation("time");


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
    grid_plane_loc = grid_program->uniformLocation("plane");
    // Set the light position to the sun's position
    grid_program->setUniformValue(grid_light_pos_loc, sunPosition);

    grid_program->release();

    grid = new GridGeometry(200,200,TerrainType::River);

    swefluid = new SWEFluid(grid,12);

    camera= new Camera({100,20,100});
    m_view =  camera->GetViewMatrix();

    initializeSkybox();

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
    sphere_plane_loc = m_sphereProgram->uniformLocation("plane");
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
    sunVAO.release();

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

    waterFrameBuffers = new WaterFrameBuffers();
    waterFrameBuffers->initialize();
    
}

void GLWidget::initializeSkybox()
{
    // Skybox Shader Program
    m_skyboxProgram = new QOpenGLShaderProgram;
    if (!m_skyboxProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/skybox_vshader.glsl"))
        close();
    if (!m_skyboxProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/skybox_fshader.glsl"))
        close();
    if (!m_skyboxProgram->link())
        close();

    // Noms des fichiers pour chaque face de la cubemap
    QStringList faceFilenames = {
        ":/Images/right.png",   // Face 1: Right
        ":/Images/left.png",    // Face 2: Left
        ":/Images/top.png",     // Face 3: Top
        ":/Images/bottom.png",  // Face 4: Bottom
        ":/Images/front.png",   // Face 5: Front
        ":/Images/back.png"     // Face 6: Back
    };

    // Création de la texture cubemap
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    // Charger chaque face de la cubemap
    for (int i = 0; i < faceFilenames.size(); ++i) {
        QImage faceImage(faceFilenames[i]);
        if (faceImage.isNull()) {
            qDebug() << "Failed to load cubemap face image:" << faceFilenames[i];
            return;
        }

        // Convertir l'image en format OpenGL approprié si nécessaire
        QImage glFormatImage = faceImage.convertToFormat(QImage::Format_RGB888);

        // Charger l'image dans la texture cubemap
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, glFormatImage.width(), glFormatImage.height(), 0,
            GL_RGB, GL_UNSIGNED_BYTE, glFormatImage.bits()
            );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_skyboxProgram->bind();
    glUniform1i(glGetUniformLocation(m_skyboxProgram->programId(), "skybox"), 0);
    m_skyboxProgram->release();

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




void GLWidget::paintGL() {

    waterFrameBuffers->bindReflectionFrameBuffer();
    float distance = 2 * (camera->Position.y() - swefluid->BaseWaterHeight);
    camera->Position = {camera->Position.x(),camera->Position.y() - distance,camera->Position.z()};
    camera->InvertPitch();
    m_view = camera->GetViewMatrix();

    RenderScene(false,{0,1,0,-static_cast<float>(swefluid->BaseWaterHeight)});

    camera->Position = {camera->Position.x(),camera->Position.y() + distance,camera->Position.z()};
    camera->InvertPitch();
    m_view = camera->GetViewMatrix();


    waterFrameBuffers->bindRefractionFrameBuffer();
    RenderScene(false,{0,-1,0,static_cast<float>(swefluid->BaseWaterHeight)});

    waterFrameBuffers->unbindCurrentFrameBuffer();


    // 2. Render the main scene
    QSize viewportSize = this->size();
    glViewport(0, 0, viewportSize.width(), viewportSize.height());
    RenderScene(true,{0,-1,0,10000});
}


void GLWidget::RenderScene(bool withWater,QVector4D clippingPlane){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_view = camera->GetViewMatrix();


    if(withWater){
        m_program->bind();

        // Mise à jour de la matrice MVP (Model-View-Projection)
        m_program->setUniformValue(m_mvp_matrix_loc, m_projection * m_view * m_model);
        QMatrix3x3 normal_matrix = m_model.normalMatrix();
        m_program->setUniformValue(m_normal_matrix_loc, normal_matrix);
        m_program->setUniformValue(m_model_matrix_loc,m_model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D ,waterFrameBuffers->getReflectionTexture());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D , waterFrameBuffers->getRefractionTexture());
        m_program->setUniformValue(cameraposloc, camera->Position);
        // Dessin de la grille
        swefluid->drawGridGeometry(m_program);

        m_program->release();
    }
    updateSimulation();

    //render grid

    grid_program->bind();

    grid_program->setUniformValue(grid_mvp_matrix_loc, m_projection * m_view * m_model);
    QMatrix3x3 grid_normal_matrix = m_model.normalMatrix();
    grid_program->setUniformValue(grid_normal_matrix_loc, grid_normal_matrix);
    grid_program->setUniformValue(grid_plane_loc,clippingPlane);
    // Dessin de la grille
    grid->drawGridGeometry(grid_program);

    grid_program->release();


    m_sphereProgram->bind();

    m_sphereProgram->setUniformValue(sphere_mvp_matrix_loc, m_projection * m_view * m_model);
    QMatrix3x3 sphere_normal_matrix = m_model.normalMatrix();
    m_sphereProgram->setUniformValue(sphere_normal_matrix_loc, sphere_normal_matrix);
    m_sphereProgram->setUniformValue(sphere_plane_loc,clippingPlane);
    updateSpheres(0.03f);
    m_sphereProgram->release();

    glDepthMask(GL_FALSE); // Disable writing to the depth buffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
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
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDepthMask(GL_TRUE);

    // Render Sun
    m_sunProgram->bind();
    sunVAO.bind();
    QMatrix4x4 mvpMatrixSun = m_projection * m_view;
    m_sunProgram->setUniformValue("mvp_matrix", mvpMatrixSun);
    QVector3D sunColor(1.5, 1.5, 1.2); // Intense yellow-white

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


void GLWidget::updateSpheres(float dt) {
    for (Sphere* sphere : spheres) {
        sphere->UpdateParticles(dt);
    }

    resolveSphereCollisions();

    for (Sphere* sphere : spheres) {
        sphere->render(m_sphereProgram);
    }
}

void GLWidget::resolveSphereCollisions() {
    const float radius = 0.5f;
    const float diameter = radius * 2.0f;

    // Assuming gridWidth and gridDepth are accessible here
    float maxX = grid->gridWidth - radius;
    float maxZ = grid->gridDepth - radius;
    float minX = 0 + radius;
    float minZ = 0 + radius;

    for (int i = 0; i < spheres.size(); ++i) {
        for (int j = i + 1; j < spheres.size(); ++j) {
            QVector3D& pos1 = spheres[i]->vertices[0].position;
            QVector3D& pos2 = spheres[j]->vertices[0].position;

            QVector3D delta = pos2 - pos1;
            float distance = delta.length();

            if (distance < diameter) {
                QVector3D midpoint = (pos1 + pos2) * 0.5f;
                QVector3D direction = delta.normalized();

                QVector3D iNewPos = midpoint - direction * radius;
                QVector3D jNewPos = midpoint + direction * radius;

                // Boundary check for iNewPos
                iNewPos.setX(qMax(minX, qMin(iNewPos.x(), maxX)));
                iNewPos.setZ(qMax(minZ, qMin(iNewPos.z(), maxZ)));

                // Boundary check for jNewPos
                jNewPos.setX(qMax(minX, qMin(jNewPos.x(), maxX)));
                jNewPos.setZ(qMax(minZ, qMin(jNewPos.z(), maxZ)));

                spheres[i]->vertices[0].position = iNewPos;
                spheres[j]->vertices[0].position = jNewPos;

                // Update all vertices of the moved spheres
                for(int k = 1; k < spheres[i]->vertices.size(); k++) {
                    spheres[i]->vertices[k].position = iNewPos + spheres[i]->vertices[k].offset;
                }
                for(int k = 1; k < spheres[j]->vertices.size(); k++) {
                    spheres[j]->vertices[k].position = jNewPos + spheres[j]->vertices[k].offset;
                }
            }
        }
    }
}


void GLWidget::updateSimulation() {

    swefluid->ShallowWaterStep(0.0005f);
    swefluid->updateVertexBuffer();

    time+= 0.1f;
    m_program->bind();
    m_program->setUniformValue(timeLocation, time);
    m_program->release();
    update(); // Request to redraw the widget
}

void GLWidget::resizeGL(int w, int h) {
    if (h == 0) h = 1; // Prévenir la division par zéro
    m_projection.setToIdentity();
    m_projection.perspective(m_currentFoV, float(w) / float(h), 0.01f, 10000.0f);
}

void GLWidget::setFoV(float fov)
{
    m_currentFoV = fov;
    updateProjectionMatrix();
}

void GLWidget::updateProjectionMatrix()
{
    m_projection.setToIdentity();
    m_projection.perspective(m_currentFoV, float(width()) / float(height()),   0.01f, 10000.0f);
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
             Sphere* newSphere = new Sphere(grid,{ forwardHitPoint.x(), forwardHitPoint.y() + 30, forwardHitPoint.z()},0.5f,12,swefluid);
             spheres.push_back(newSphere);
        }

        rayOrigin = backwardRayEnd;
        QVector3D backwardHitPoint;
        if (grid->intersectsRay(rayOrigin, backwardRayDirection, rayLength, backwardHitPoint)) {
             Sphere* newSphere = new Sphere(grid,{ backwardHitPoint.x(), backwardHitPoint.y() + 30, backwardHitPoint.z()},0.5f,12,swefluid);
             spheres.push_back(newSphere);
        }

        rayEnd = forwardRayEnd;
    }


}




void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::RightButton) {
        QPoint currentPos = event->pos();

        if (!lastPos.isNull()) {
             // Calculate the difference in position
             float deltaX = currentPos.x() - lastPos.x();
             float deltaY = currentPos.y() - lastPos.y();

             const float sensitivity = 2.0f; // Adjust sensitivity as needed

             // Update camera based on the delta movement
             camera->ProcessMouseMovement(deltaX * sensitivity, (-deltaY) * sensitivity);
        }

        lastPos = currentPos;
        update();
    }
}



void GLWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_T) {  
        swefluid->CreateInitialWave(Border::East);
        swefluid->updateVertexBuffer();
        //   update();
    }

    if (event->key() == Qt::Key_P) {
        Sphere* newSphere = new Sphere(grid,{ float(100), 50, float(100)},0.5f,12,swefluid);
        spheres.push_back(newSphere);

    }

    camera->ProcessKeyboard(event->key() ,1.0f);

    update();

}

void GLWidget::ChangeTerrain(int index){
    foreach (Sphere*sphere, spheres) {
        sphere->~Sphere();
    }
    spheres.clear();
    delete swefluid;
    delete grid;

    switch (index) {
    case 0:
        grid = new GridGeometry(200,200,TerrainType::River);
        swefluid = new SWEFluid(grid,12);
        break;
    case 1:
        grid = new GridGeometry(200,200,TerrainType::Island);
        swefluid = new SWEFluid(grid,12);
        break;
    case 2:
        grid = new GridGeometry(200,200,TerrainType::Canal);
        swefluid = new SWEFluid(grid,12);
        break;
    case 3:
        grid = new GridGeometry(200,200,TerrainType::None);
        swefluid = new SWEFluid(grid,12);
        break;
    }
}
