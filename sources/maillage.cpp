#include "../headers/maillage.h"
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <vector>

struct VertexData {
    QVector3D position;
    QVector3D normal;
    QVector2D texCoord;
};


Maillage::Maillage() : indexBuf(QOpenGLBuffer::IndexBuffer), gridWidth(10), gridHeight(10) {
    initializeOpenGLFunctions();

    arrayBuf.create();
    indexBuf.create();

    initGridGeometry();
}

Maillage::~Maillage() {
    arrayBuf.destroy();
    indexBuf.destroy();
}

void Maillage::initGridGeometry() {
    std::vector<VertexData> vertices;
    QVector3D normal(0.0f, 1.0f, 0.0f); // Normal pointing upwards

    for (int x = 0; x < gridWidth; ++x) {
        for (int z = 0; z < gridHeight; ++z) {
            vertices.push_back({QVector3D(x, 0.0f, z), normal, QVector2D()});
        }
    }

    // Example indices for GL_TRIANGLE_STRIP
    std::vector<GLushort> indices;
    for (int z = 0; z < gridHeight - 1; ++z) {
        if (z > 0)
            indices.push_back(z * gridWidth); // Degenerate index

        for (int x = 0; x < gridWidth; ++x) {
            indices.push_back(z * gridWidth + x);
            indices.push_back((z + 1) * gridWidth + x);
        }

        if (z < gridHeight - 2)
            indices.push_back((z + 1) * gridWidth + (gridWidth - 1)); // Degenerate index
    }

    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(VertexData));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size()) * sizeof(GLushort));
}


void Maillage::drawGridGeometry(QOpenGLShaderProgram* program) {
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

    offset += sizeof(QVector3D); // Offset for the normal vector

    // Texture coordinate attribute (if needed)
    int texcoordLocation = program->attributeLocation("texCoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}

