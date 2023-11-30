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
        qWarning() << "No image found for gradient.";
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

        gTextureHeight0 = new QOpenGLTexture(image0);
        gTextureHeight1 = new QOpenGLTexture(image1);
        gTextureHeight2 = new QOpenGLTexture(image2);
        gTextureHeight3 = new QOpenGLTexture(image3);

}

void GridGeometry::setTextureHeightMapsFromPaths(const QString& path0, const QString& path1, const QString& path2, const QString& path3) {
    QImage image0(path0);
    QImage image1(path1);
    QImage image2(path2);
    QImage image3(path3);

   if(image0.isNull() ){
        qWarning() << "No image found for rockGrass.";
        return;
    }
   if(image1.isNull() ){
        qWarning() << "No image found for grass.";
        return;
   }
   if(image2.isNull() ){
        qWarning() << "No image found for rock.";
        return;
   }
   if(image3.isNull() ){
        qWarning() << "No image found for snow.";
        return;
   }

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
    auto checkGLError = [this](const char* action) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qWarning() << "OpenGL error after" << action << ":" << err;
        }
    };

    // Texture binding and checking
    auto bindTextureWithCheck = [&](QOpenGLTexture* texture, GLenum textureUnit, const char* uniformName, int unitIndex) {
        if (texture) {
            glActiveTexture(textureUnit);
            texture->bind();
            program->setUniformValue(uniformName, unitIndex);
            checkGLError(uniformName);
        }
    };

    bindTextureWithCheck(heightMapTexture, GL_TEXTURE0, "heightMap", 0);
    program->setUniformValue("maxHeight", maxHeight);

    bindTextureWithCheck(gTextureHeight0, GL_TEXTURE1, "gTextureHeight0", 1);
    bindTextureWithCheck(gTextureHeight1, GL_TEXTURE2, "gTextureHeight1", 2);
    bindTextureWithCheck(gTextureHeight2, GL_TEXTURE3, "gTextureHeight2", 3);
    bindTextureWithCheck(gTextureHeight3, GL_TEXTURE4, "gTextureHeight3", 4);

    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}
