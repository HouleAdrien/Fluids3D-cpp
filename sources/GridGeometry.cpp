#include "../headers/GridGeometry.h"
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <vector>


GridGeometry::GridGeometry() : indexBuf(QOpenGLBuffer::IndexBuffer) {
    initializeOpenGLFunctions();

// ":/Images/iceland_heightmap.png"
    QImage _image(":/Images/iceland_heightmap.png");
    
    if(_image.isNull()){
        qWarning() << "No image found.";
    }else{
        image = _image;
        gridWidth = image.width();
        gridDepth = image.height();
        data = image.bits();
    }


    arrayBuf.create();
    indexBuf.create();

    initGridGeometry();
}

GridGeometry::~GridGeometry() {
    arrayBuf.destroy();
    indexBuf.destroy();
}

void GridGeometry::initGridGeometry() {

    // vertex generation
    std::vector<GridVertexData> vertices;
    int nChannels = image.depth() / 8;

    float yScale = 64.0f / 256.0f, yShift = 16.0f;  // apply a scale+shift to the height data
     QVector3D normal(0.0f, 1.0f, 0.0f); // Normal pointing upwards


    for(unsigned int i = 0; i < gridDepth; i++)
    {
        for(unsigned int j = 0; j < gridWidth; j++)
        {
            // retrieve texel for (i,j) tex coord
            unsigned char* texel = data + (j + gridWidth * i) * nChannels;
            // raw height at coordinate
            unsigned char y = texel[0];

            // vertex
            // vertices.push_back( -gridDepth/2.0f + i );        // v.x
            // vertices.push_back( (int)y * yScale - yShift); // v.y
            // vertices.push_back( -gridWidth/2.0f + j/ );        // v.z

            vertices.push_back({QVector3D(-gridDepth/2.0f + i , (int)y * yScale - yShift, -gridWidth/2.0f + j), normal, QVector2D()});
        }
    }



    // index generation
    std::vector<GLushort> indices;
    for(unsigned int i = 0; i < gridDepth-1; i++)       // for each row a.k.a. each strip
    {
        for(unsigned int j = 0; j < gridWidth; j++)      // for each column
        {
            for(unsigned int k = 0; k < 2; k++)      // for each side of the strip
            {
                indices.push_back(j + gridWidth * (i + k));
            }
        }
    }

    // // Example indices for GL_TRIANGLE_STRIP
    // std::vector<GLushort> indices;
    // for (int z = 0; z < gridDepth - 1; ++z) {
    //     if (z > 0)
    //         indices.push_back(z * gridWidth); // Degenerate index

    //     for (int x = 0; x < gridWidth; ++x) {
    //         indices.push_back(z * gridWidth + x);
    //         indices.push_back((z + 1) * gridWidth + x);
    //     }

    //     if (z < gridDepth - 2)
    //         indices.push_back((z + 1) * gridWidth + (gridWidth - 1)); // Degenerate index
    // }

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

    glDrawElements(GL_TRIANGLE_STRIP, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}
