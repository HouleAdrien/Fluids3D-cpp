#include "../headers/SWEFluid.h"
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <vector>

SWEFluid::SWEFluid(int _gridWidth, int _gridDepth) : indexBuf(QOpenGLBuffer::IndexBuffer) {
    initializeOpenGLFunctions();
    gridDepth = _gridDepth;
    gridWidth = _gridWidth;
    Dx = 1.0f / (_gridWidth - 1);
    Dy = 1.0f / (_gridDepth - 1);

    arrayBuf.create();
    indexBuf.create();

    initGridGeometry();
}

SWEFluid::~SWEFluid() {
    arrayBuf.destroy();
    indexBuf.destroy();
}

void SWEFluid::initGridGeometry() {
    QVector3D normal(0.0f, 1.0f, 0.0f); // Normal pointing upwards

    for (int x = 0; x < gridWidth; ++x) {
        for (int z = 0; z < gridDepth; ++z) {
            VertexData vertex;
            vertex.position = QVector3D(x, 0.0f, z);
            vertex.normal = normal;
            vertex.waterHeight = 0;
            vertex.velocity = QVector2D(0, 0); // 2D velocity (vx, vz)
            vertex.groundHeight = 0;
            vertex.fluidHeight = 0;

            vertices.push_back(vertex);
        }
    }

    std::vector<GLushort> indices;
    for (int z = 0; z < gridDepth - 1; ++z) {
        if (z > 0)
            indices.push_back(z * gridWidth);

        for (int x = 0; x < gridWidth; ++x) {
            indices.push_back(z * gridWidth + x);
            indices.push_back((z + 1) * gridWidth + x);
        }

        if (z < gridDepth - 2)
            indices.push_back((z + 1) * gridWidth + (gridWidth - 1));
    }

    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(VertexData));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size()) * sizeof(GLushort));
}


void SWEFluid::drawGridGeometry(QOpenGLShaderProgram* program) {
    arrayBuf.bind();
    indexBuf.bind();

    quintptr offset = 0;

    // Position attribute
    int vertexLocation = program->attributeLocation("vertex");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    // Normal attribute
    int normalLocation = program->attributeLocation("normal");
    program->enableAttributeArray(normalLocation);
    program->setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    // Texture coordinate attribute (if needed)
    int texcoordLocation = program->attributeLocation("texCoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    offset += sizeof(QVector2D);

    // Water height attribute
    int waterHeightLocation = program->attributeLocation("waterHeight");
    program->enableAttributeArray(waterHeightLocation);
    program->setAttributeBuffer(waterHeightLocation, GL_FLOAT, offset, 1, sizeof(VertexData));

    offset += sizeof(float);

    // Velocity attribute (2D velocity)
    int velocityLocation = program->attributeLocation("velocity");
    program->enableAttributeArray(velocityLocation);
    program->setAttributeBuffer(velocityLocation, GL_FLOAT, offset, 2, sizeof(VertexData)); // 2D velocity (vx, vz)

    offset += sizeof(QVector2D); // 2D velocity (vx, vz)

    // Ground height attribute
    int groundHeightLocation = program->attributeLocation("groundHeight");
    program->enableAttributeArray(groundHeightLocation);
    program->setAttributeBuffer(groundHeightLocation, GL_FLOAT, offset, 1, sizeof(VertexData));

    offset += sizeof(float);

    // Fluid height attribute
    int fluidHeightLocation = program->attributeLocation("fluidHeight");
    program->enableAttributeArray(fluidHeightLocation);
    program->setAttributeBuffer(fluidHeightLocation, GL_FLOAT, offset, 1, sizeof(VertexData));

    // Draw the geometry
    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}

void SWEFluid::ShallowWaterStep(float dt) {
    std::vector<VertexData> tempVertices = vertices;

    // Étape 1: Advect pour la hauteur de l'eau
    Advect(tempVertices, vertices, dt);

    // Étape 2 et 3: Advect pour les vitesses
    // Cette implémentation suppose que Advect peut aussi gérer les vitesses
    Advect(tempVertices, vertices, dt);

    // Étape 4, 5, et 6: Mise à jour des vitesses et de la hauteur de l'eau
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i].velocity = tempVertices[i].velocity;
        vertices[i].waterHeight = tempVertices[i].waterHeight;
    }

    // Étape 7: Mise à jour de la hauteur
    Update_height(dt);

    // Étape 8: Calcul de la hauteur totale du fluide
    for (auto& vertex : vertices) {
        vertex.fluidHeight = vertex.waterHeight + vertex.groundHeight;
    }

    // Étape 9: Mise à jour des vitesses
    Update_velocities(dt);

    for (auto& vertex : vertices) {
        vertex.position.setY(vertex.groundHeight + vertex.waterHeight);
    }
}

void SWEFluid::Advect(std::vector<VertexData>& deplacement, const std::vector<VertexData>& source, float dt) {
    for (size_t j = 0; j < gridDepth; ++j) {
        for (size_t i = 0; i < gridWidth; ++i) {
            float x = i - dt * source[j * gridWidth + i].velocity.x();
            float z = j - dt * source[j * gridWidth + i].velocity.y(); // Use 'z' for the z-axis velocity

            // Interpolate and assign new values
            deplacement[j * gridWidth + i].waterHeight = Interpolate(source, x, z); // Interpolate using 'z' for the z-axis
            deplacement[j * gridWidth + i].velocity = InterpolateVelocity(source, x, z); // Interpolate velocity using 'z' for the z-axis
        }
    }
}
const float heightDampingFactor = 0.9f;
void SWEFluid::Update_height(float dt) {
    for (size_t j = 1; j < gridDepth - 1; ++j) {
        for (size_t i = 1; i < gridWidth - 1; ++i) {
            auto& vertex = vertices[j * gridWidth + i];
            float dVx = (vertices[j * gridWidth + (i + 1)].velocity.x() - vertex.velocity.x()) / Dx;
            float dVz = (vertices[(j + 1) * gridWidth + i].velocity.y() - vertex.velocity.y()) / Dy;

            vertex.waterHeight -= (dt * (dVx + dVz));
            vertex.waterHeight *=heightDampingFactor;
        }
    }
}
const float velocityDampingFactor = 0.9f;
void SWEFluid::Update_velocities(float dt) {
    for (size_t j = 1; j < gridDepth - 1; ++j) {
        for (size_t i = 1; i < gridWidth - 1; ++i) {
            auto& vertex = vertices[j * gridWidth + i];
            float dhX = (vertices[j * gridWidth + (i - 1)].waterHeight - vertex.waterHeight) / Dx;
            float dhZ = (vertex.waterHeight - vertices[(j - 1) * gridWidth + i].waterHeight) / Dy;
            vertex.velocity.setX(vertex.velocity.x() + gravity * dt * dhX);
            vertex.velocity.setY(vertex.velocity.y() + gravity * dt * dhZ);
        }
    }
}

void SWEFluid::updateVertexBuffer() {
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(VertexData));
    arrayBuf.release();
}

float SWEFluid::Interpolate(const std::vector<VertexData>& source, float x, float z) {
    // Trouver les indices des coins du carré contenant le point (x, z)
    int x0 = floor(x);
    int x1 = x0 + 1;
    int z0 = floor(z);
    int z1 = z0 + 1;

    // Vérifier les limites de la grille
    x0 = std::max(0, std::min(x0, gridWidth - 1));
    x1 = std::max(0, std::min(x1, gridWidth - 1));
    z0 = std::max(0, std::min(z0, gridDepth - 1));
    z1 = std::max(0, std::min(z1, gridDepth - 1));

    // Récupérer les hauteurs de l'eau aux coins
    float q00 = source[z0 * gridWidth + x0].waterHeight;
    float q10 = source[z0 * gridWidth + x1].waterHeight;
    float q01 = source[z1 * gridWidth + x0].waterHeight;
    float q11 = source[z1 * gridWidth + x1].waterHeight;

    // Interpolation bilinéaire
    float r0 = ((x1 - x) * q00) + ((x - x0) * q10);
    float r1 = ((x1 - x) * q01) + ((x - x0) * q11);
    return ((z1 - z) * r0) + ((z - z0) * r1);
}

QVector2D SWEFluid::InterpolateVelocity(const std::vector<VertexData>& source, float x, float z) {
    int x0 = floor(x);
    int x1 = x0 + 1;
    int z0 = floor(z);
    int z1 = z0 + 1;

    x0 = std::max(0, std::min(x0, gridWidth - 1));
    x1 = std::max(0, std::min(x1, gridWidth - 1));
    z0 = std::max(0, std::min(z0, gridDepth - 1));
    z1 = std::max(0, std::min(z1, gridDepth - 1));

    QVector2D v00 = source[z0 * gridWidth + x0].velocity;
    QVector2D v10 = source[z0 * gridWidth + x1].velocity;
    QVector2D v01 = source[z1 * gridWidth + x0].velocity;
    QVector2D v11 = source[z1 * gridWidth + x1].velocity;

    QVector2D r0 = ((x1 - x) * v00) + ((x - x0) * v10);
    QVector2D r1 = ((x1 - x) * v01) + ((x - x0) * v11);

    return ((z1 - z) * r0) + ((z - z0) * r1);
}

void SWEFluid::setWaterHeightAt(int x, int z, double height) {
    if (x >= 0 && x < gridWidth && z >= 0 && z < gridDepth) {
        int index = z * gridWidth + x;
        double deltaHeight = height - vertices[index].waterHeight;

        // Mettre à jour la hauteur de l'eau
        vertices[index].waterHeight = height;

        // Mettre à jour la position Y du vertex
        vertices[index].position.setY(vertices[index].groundHeight + height);
    }
}
