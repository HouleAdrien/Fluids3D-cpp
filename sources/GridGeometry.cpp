#include "../headers/GridGeometry.h"
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <vector>


GridGeometry::GridGeometry(int _gridWidth,int _gridDepth,TerrainType terrainType) : indexBuf(QOpenGLBuffer::IndexBuffer) {
    initializeOpenGLFunctions();

    gridWidth = _gridWidth;
    gridDepth = _gridDepth;
    switch (terrainType) {
    case TerrainType::River:{
        QImage image1(":/Images/river.jpeg");

        if(image1.isNull()){
            qWarning() << "No image found for gradient.";
        }else{
            setHeightMap(image1);
        }
        break;
          }
    case TerrainType::Island:{
        QImage image2(":/Images/island.png");

        if(image2.isNull()){
            qWarning() << "No image found for gradient.";
        }else{
            setHeightMap(image2);
        }
        break;
          }
    case TerrainType::Canal:{
        QImage image3(":/Images/canaux.jpg");

        if(image3.isNull()){
            qWarning() << "No image found for gradient.";
        }else{
            setHeightMap(image3);
        }
        break;
    }
    case TerrainType::None:
        QImage image4(":/Images/black.jpg");

        if(image4.isNull()){
            qWarning() << "No image found for gradient.";
        }else{
            setHeightMap(image4);
        }
        break;
    }



    arrayBuf.create();
    indexBuf.create();

    setTextureHeightMapsFromPaths(":/Images/sand.jpg", ":/Images/sand.jpg", ":/Images/rock.png", ":/Images/rock.png");


    initGridGeometry();
}

GridGeometry::~GridGeometry() {
    arrayBuf.destroy();
    indexBuf.destroy();

    delete gTextureHeight0;
    delete gTextureHeight1;
    delete gTextureHeight2;
    delete gTextureHeight3;
}

void GridGeometry::setHeightMap(const QImage& image) {
    heightMapTexture = image;
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

float GridGeometry::getHeightAtPosition(float x, float z) {
    // Ensure that x and z are within the bounds of the grid
    if (x < 0.0f || x >= gridWidth || z < 0.0f || z >= gridDepth) {
        return 0.0f; // Return a default height or an out-of-bounds value
    }

    // Normalize x and z to the range [0, 1] for texture sampling
    float texX = x / static_cast<float>(gridWidth);
    float texZ = z / static_cast<float>(gridDepth);

    QColor pixelColor = heightMapTexture.pixelColor(texX * heightMapTexture.width(), texZ * heightMapTexture.height());

    // Convert the sampled color to a floating-point height value
    float height = pixelColor.red();
    height /=255;
    height *= maxHeight;

    return height;
}

QVector2D GridGeometry::getGradientAtPosition(float x, float z) {
    // Step size for gradient calculation
    float stepSize = 1.0f;

    // Ensure that x and z are within the bounds of the grid
    if (x < 0.0f || x >= gridWidth || z < 0.0f || z >= gridDepth) {
        return QVector2D(0.0f, 0.0f); // Return zero gradient for out-of-bounds
    }

    // Calculate heights at neighboring positions
    float heightX1 = getHeightAtPosition(x + stepSize, z);
    float heightX2 = getHeightAtPosition(x - stepSize, z);
    float heightZ1 = getHeightAtPosition(x, z + stepSize);
    float heightZ2 = getHeightAtPosition(x, z - stepSize);

    // Calculate gradient
    float dX = (heightX1 - heightX2) / (2.0f * stepSize);
    float dZ = (heightZ1 - heightZ2) / (2.0f * stepSize);

    return QVector2D(dX, dZ);
}


bool GridGeometry::intersectsRay(const QVector3D &rayOrigin, const QVector3D &rayDirection, float rayLength, QVector3D &intersectionPoint) {
    float tolerance = 0.2f; // Tolerance for intersection
    float stepSize = 1.0f; // Tune this value as needed

    for (float t = 0.0f; t <= rayLength; t += stepSize) {
        QVector3D pointOnRay = rayOrigin + rayDirection.normalized() * t;
        float terrainHeight = getHeightAtPosition(pointOnRay.x(), pointOnRay.z());

        // Check if the ray's y-coordinate is within the tolerance range of the terrain height
        if (std::abs(pointOnRay.y() - terrainHeight) <= tolerance) {
            intersectionPoint = QVector3D(pointOnRay.x(), terrainHeight, pointOnRay.z());
            return true; // Ray intersects the terrain within the tolerance range
        }
    }

    return false; // No intersection found within the tolerance range
}



void GridGeometry::initGridGeometry() {
    QVector3D normal(0.0f, 1.0f, 0.0f); // Normal pointing upwards


    for (int z = 0; z < gridDepth; ++z) {
        for (int x = 0; x < gridWidth; ++x) {
            // Calculate texture coordinates based on original dimensions
            QVector2D texCoord(static_cast<float>(x) / gridWidth, static_cast<float>(z) / gridDepth);

            // Calculate height based on original dimensions
            float height = getHeightAtPosition(static_cast<float>(x) , static_cast<float>(z) );

            vertices.push_back({QVector3D(x , height, z ), normal, texCoord});
        }
    }


    for (int z = 0; z < gridDepth - 1; ++z) {
        for (int x = 0; x < gridWidth - 1; ++x) {
            // First triangle in the quad
            indices.push_back(z * gridWidth + x);
            indices.push_back((z + 1) * gridWidth + x);
            indices.push_back(z * gridWidth + x + 1);

            // Second triangle in the quad
            indices.push_back(z * gridWidth + x + 1);
            indices.push_back((z + 1) * gridWidth + x);
            indices.push_back((z + 1) * gridWidth + x + 1);
        }
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

    program->setUniformValue("maxHeight", maxHeight);

    bindTextureWithCheck(gTextureHeight0, GL_TEXTURE1, "gTextureHeight0", 1);
    bindTextureWithCheck(gTextureHeight1, GL_TEXTURE2, "gTextureHeight1", 2);
    bindTextureWithCheck(gTextureHeight2, GL_TEXTURE3, "gTextureHeight2", 3);
    bindTextureWithCheck(gTextureHeight3, GL_TEXTURE4, "gTextureHeight3", 4);

    glDrawElements(GL_TRIANGLES, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}


// Implement the getVertex and getIndex methods
QVector3D GridGeometry::getVertex(int index) const {
    if (index < 0 || index >= static_cast<int>(vertices.size())) {
        return QVector3D(); 
    }
    return vertices[index].position;
}

GLushort GridGeometry::getIndex(int index) const {
    if (index < 0 || index >= static_cast<int>(indices.size())) {
        return 0; 
    }
    return indices[index];
}
