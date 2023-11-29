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

        setTextureHeightMapsFromPaths(":/Images/grass.jpg", ":/Images/rockGrass.jpeg", ":/Images/rock.png", ":/Images/snow.jpg");


    initGridGeometry();
}

GridGeometry::~GridGeometry() {
    arrayBuf.destroy();
    indexBuf.destroy();

    delete heightMapTexture;
    delete gTextureHeight0;
    delete gTextureHeight1;
    delete gTextureHeight2;
    delete gTextureHeight3;
}

void GridGeometry::setHeightMap(const QImage& image) {
    heightMapTexture = new QOpenGLTexture(image);
}


void GridGeometry::setTextureHeightMaps(const QImage& image0, const QImage& image1, const QImage& image2, const QImage& image3) {
    // Assuming gTextureHeight0, gTextureHeight1, gTextureHeight2, and gTextureHeight3 are QOpenGLTexture pointers in your class

    if (!image0.isNull()) {
        if (gTextureHeight0) {
            delete gTextureHeight0;
        }
        gTextureHeight0 = new QOpenGLTexture(image0);
    }

    if (!image1.isNull()) {
        if (gTextureHeight1) {
            delete gTextureHeight1;
        }
        gTextureHeight1 = new QOpenGLTexture(image1);
    }

    if (!image2.isNull()) {
        if (gTextureHeight2) {
            delete gTextureHeight2;
        }
        gTextureHeight2 = new QOpenGLTexture(image2);
    }

    if (!image3.isNull()) {
        if (gTextureHeight3) {
            delete gTextureHeight3;
        }
        gTextureHeight3 = new QOpenGLTexture(image3);
    }
}

void GridGeometry::setTextureHeightMapsFromPaths(const QString& path0, const QString& path1, const QString& path2, const QString& path3) {
    QImage image0(path0);
    QImage image1(path1);
    QImage image2(path2);
    QImage image3(path3);

    setTextureHeightMaps(image0, image1, image2, image3);
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

    if (gTextureHeight0) {
        gTextureHeight0->bind();
        program->setUniformValue("gTextureHeight0", 1); // Assuming the texture is bound to GL_TEXTURE1
    }

    if (gTextureHeight1) {
        gTextureHeight1->bind();
        program->setUniformValue("gTextureHeight1", 2); // Assuming the texture is bound to GL_TEXTURE2
    }

    if (gTextureHeight2) {
        gTextureHeight2->bind();
        program->setUniformValue("gTextureHeight2", 3); // Assuming the texture is bound to GL_TEXTURE3
    }

    if (gTextureHeight3) {
        gTextureHeight3->bind();
        program->setUniformValue("gTextureHeight3", 4); // Assuming the texture is bound to GL_TEXTURE4
    }

    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}
