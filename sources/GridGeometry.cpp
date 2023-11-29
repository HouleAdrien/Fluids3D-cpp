#include "../headers/GridGeometry.h"
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <vector>


GridGeometry::GridGeometry(int _gridWidth,int _gridDepth) : indexBuf(QOpenGLBuffer::IndexBuffer) {
    initializeOpenGLFunctions();

    gridWidth = _gridWidth;
    gridDepth = _gridDepth;

    QImage _image(":/Images/Untitled.jpeg");
    
    if(_image.isNull()){
        qWarning() << "No image found.";
    }else{
        setHeightMap(_image);
    }


    arrayBuf.create();
    indexBuf.create();

    initGridGeometry();
}

GridGeometry::~GridGeometry() {
    arrayBuf.destroy();
    indexBuf.destroy();
}

void GridGeometry::setHeightMap(const QImage& image) {
    heightMapTexture = new QOpenGLTexture(image);
}

void GridGeometry::initGridGeometry() {
    std::vector<GridVertexData> vertices;
    QVector3D normal(0.0f, 1.0f, 0.0f); // Normal pointing upwards

    for (int x = 0; x < gridWidth; ++x) {
        for (int z = 0; z < gridDepth; ++z) {
            QVector2D texCoord(static_cast<float>(x) / gridWidth, static_cast<float>(z) / gridDepth);
            vertices.push_back({QVector3D(x, -5.0f, z), normal, texCoord});
        }
    }

    // Example indices for GL_TRIANGLE_STRIP
    std::vector<GLushort> indices;
    for (int z = 0; z < gridDepth - 1; ++z) {
        if (z > 0)
            indices.push_back(z * gridWidth); // Degenerate index

        for (int x = 0; x < gridWidth; ++x) {
            indices.push_back(z * gridWidth + x);
            indices.push_back((z + 1) * gridWidth + x);
        }

        if (z < gridDepth - 2)
            indices.push_back((z + 1) * gridWidth + (gridWidth - 1)); // Degenerate index
    }

    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(GridVertexData));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size()) * sizeof(GLushort));
}

void GridGeometry::drawGridGeometry(QOpenGLShaderProgram* program) {
    arrayBuf.bind();
    indexBuf.bind();

    quintptr offset = 0;

    // Position attribute
    int vertexLocation = program->attributeLocation("vertex");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(GridVertexData));

    offset += sizeof(QVector3D);

    // Normal attribute
    int normalLocation = program->attributeLocation("normal");
    program->enableAttributeArray(normalLocation);
    program->setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, sizeof(GridVertexData));

    offset += sizeof(QVector3D); // Offset for the normal vector

    // Texture coordinate attribute (if needed)
    int texcoordLocation = program->attributeLocation("texCoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(GridVertexData));

    if (heightMapTexture) {
        heightMapTexture->bind();
        program->setUniformValue("heightMap", 0); // Assuming the texture is bound to GL_TEXTURE0
        program->setUniformValue("maxHeight", maxHeight);
    }

    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}
