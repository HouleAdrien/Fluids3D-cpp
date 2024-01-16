#include "../headers/SWEFluid.h"
#include <QVector2D>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <vector>
#include <math.h>

SWEFluid::SWEFluid(GridGeometry* _grid,int _BaseWaterHeight) : indexBuf(QOpenGLBuffer::IndexBuffer) {
    initializeOpenGLFunctions();
    grid = _grid;
    gridDepth = grid->gridDepth;
    gridWidth = grid->gridWidth;
    BaseWaterHeight =_BaseWaterHeight;
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


    std::vector<GLushort> indices;
    vertexIndexMap = std::vector<std::vector<int>>(gridWidth, std::vector<int>(gridDepth, -1));
    int vertexCounter = 0;

    for (int x = 0; x < gridWidth; ++x) {
        for (int z = 0; z < gridDepth; ++z) {
            float terrainHeight = grid->getHeightAtPosition(x, z);
            if (terrainHeight <= BaseWaterHeight) {
                VertexData vertex;
                vertex.position = QVector3D(x, terrainHeight, z);
                vertex.normal = normal;
                vertex.waterHeight =  BaseWaterHeight - terrainHeight;
                vertex.velocity = QVector2D(0, 0); // 2D velocity (vx, vz)
                vertex.groundHeight = terrainHeight;
                vertex.fluidHeight = vertex.waterHeight+ vertex.groundHeight;
                vertex.texCoord = QVector2D(static_cast<float>(x) / gridWidth, static_cast<float>(z) / gridDepth);
                vertex.isBorder = false;
                if (x > 0 && grid->getHeightAtPosition(x - 1, z) > BaseWaterHeight) vertex.isBorder = true;
                if (x < gridWidth - 1 && grid->getHeightAtPosition(x + 1, z) > BaseWaterHeight) vertex.isBorder = true;
                if (z > 0 && grid->getHeightAtPosition(x, z - 1) > BaseWaterHeight) vertex.isBorder = true;
                if (z < gridDepth - 1 && grid->getHeightAtPosition(x, z + 1) > BaseWaterHeight) vertex.isBorder = true;

                vertices.push_back(vertex);


                vertexIndexMap[x][z] = vertexCounter++;
            }
                VertexData vertex;
                vertex.normal = normal;
                vertex.waterHeight =  BaseWaterHeight ;
                vertex.velocity = QVector2D(0, 0); // 2D velocity (vx, vz)
                vertex.groundHeight = 0;
                vertex.isBorder=false;
                vertex.fluidHeight = vertex.waterHeight+ vertex.groundHeight;
                vertex.position = QVector3D(x, vertex.fluidHeight, z);
                vertex.texCoord = QVector2D(static_cast<float>(x) / gridWidth, static_cast<float>(z) / gridDepth);
                globalGridInfos.push_back(vertex);

        }
    }

    // Now create indices
    for (int x = 0; x < gridWidth - 1; ++x) {
        for (int z = 0; z < gridDepth - 1; ++z) {
            // Get the indices of the four corners of the current grid cell
            int topLeft = vertexIndexMap[x][z];
            int topRight = vertexIndexMap[x + 1][z];
            int bottomLeft = vertexIndexMap[x][z + 1];
            int bottomRight = vertexIndexMap[x + 1][z + 1];

            // Create two triangles if all four corners have vertices
            if (topLeft != -1 && topRight != -1 && bottomLeft != -1 && bottomRight != -1) {
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }


    for (auto& vertex : vertices) {
        vertex.position.setY(vertex.fluidHeight);
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

    offset += sizeof(bool);

    int isborderLocation = program->attributeLocation("isBorder");
    program->enableAttributeArray(isborderLocation);
    program->setAttributeBuffer(isborderLocation, GL_FLOAT, offset, 1, sizeof(VertexData));


    // Draw the geometry
    glDrawElements(GL_TRIANGLES, indexBuf.size() / sizeof(GLushort), GL_UNSIGNED_SHORT, nullptr);

    arrayBuf.release();
    indexBuf.release();
}

void SWEFluid::ShallowWaterStep(float dt) {

    std::vector<VertexData> tempVertices = globalGridInfos;

    // Steps 1, 2, and 3: Advect for water height and velocities
    Advect(tempVertices, globalGridInfos, dt);

    // Steps 4, 5, and 6: Update velocities and water height
    for (size_t i = 0; i < globalGridInfos.size(); ++i) {
        if (globalGridInfos[i].velocity.x() == 0 && globalGridInfos[i].velocity.y() == 0) {
            // Maintain existing water height if velocity is zero
            tempVertices[i].waterHeight = globalGridInfos[i].waterHeight;
        }
        globalGridInfos[i].velocity = tempVertices[i].velocity;
        globalGridInfos[i].waterHeight = tempVertices[i].waterHeight;
    }

    // Step 7: Update height
    Update_height(dt);

    // Step 8: Calculate total fluid height
    for (auto& vertex : globalGridInfos) {
        vertex.fluidHeight = vertex.waterHeight + vertex.groundHeight;
    }

    // Step 9: Update velocities
    Update_velocities(dt);

    // Update position based on fluid height
    for (auto& vertex : globalGridInfos) {
        vertex.position.setY(vertex.fluidHeight);
    }


    computeNormal();

    for (int x = 0; x < gridWidth; ++x) {
        for (int z = 0; z < gridDepth; ++z) {
            if( vertexIndexMap[x][z] != -1){
                if(!vertices[vertexIndexMap[x][z]].isBorder){
                vertices[vertexIndexMap[x][z]] =  globalGridInfos[x * gridDepth + z];
                float groundHeight =  grid->getHeightAtPosition(x,z);
                if(vertices[vertexIndexMap[x][z]].waterHeight <groundHeight){
                        vertices[vertexIndexMap[x][z]].waterHeight = groundHeight;
                        vertices[vertexIndexMap[x][z]].fluidHeight = groundHeight;
                        vertices[vertexIndexMap[x][z]].position.setY(groundHeight+0.1f);
                    }
                }
            }
        }
    }
}


void SWEFluid::Advect(std::vector<VertexData>& deplacement, const std::vector<VertexData>& source, float dt) {
    for (size_t j = 1; j < gridDepth-1; ++j) {
        for (size_t i = 1; i < gridWidth-1; ++i) {
            float x = i - dt * source[j * gridWidth + i].velocity.x();
            float z = j - dt * source[j * gridWidth + i].velocity.y(); // Use 'z' for the z-axis velocity

            // Interpolate and assign new values
            deplacement[j * gridWidth + i].waterHeight = InterpolateWaterH(source, x, z); // Interpolate using 'z' for the z-axis
            deplacement[j * gridWidth + i].velocity = InterpolateVelocity(source, x, z); // Interpolate velocity using 'z' for the z-axis
        }
    }
}

void SWEFluid::Update_height(float dt) {
    float div;

    for (size_t j = 1; j < gridDepth - 1; ++j) {
        for (size_t i = 1; i < gridWidth - 1; ++i) {
            auto& vertex = globalGridInfos[j * gridWidth + i];

            // Je fais valeur a gauche et a droite car meilleur résulat mais ça pourrait être valeur +1 et actuel

            float dVx = (globalGridInfos[j * gridWidth + (i + 1)].velocity.x() - globalGridInfos[j * gridWidth + (i -1)].velocity.x()) / (2  /* * Dx */);
            float dVz = (globalGridInfos[(j + 1) * gridWidth + i].velocity.y() - globalGridInfos[(j - 1) * gridWidth + i].velocity.y()) / (2 /*  * Dy */);

            div = dt * (dVx + dVz);

            // La nouvelle hauteur c'est l'ancienne hauteur moins la hauteur * le divergent
            vertex.waterHeight =vertex.waterHeight - (  vertex.waterHeight * (div));

        }
    }
}

void SWEFluid::Update_velocities(float dt) {
    float dampingFactor = 1;
    float xGrad;
    float zGrad;

    for (size_t j = 0; j < gridDepth; ++j) {
        for (size_t i = 0; i < gridWidth; ++i) {
            auto& vertex = globalGridInfos[j * gridWidth + i];

            size_t left = (i == 0) ? 0 : i - 1;
            size_t right = (i == gridWidth - 1) ? i : i + 1;
            size_t down = (j == 0) ? 0 : j - 1;
            size_t up = (j == gridDepth - 1) ? j : j + 1;

            xGrad = (globalGridInfos[j * gridWidth + right].waterHeight - globalGridInfos[j * gridWidth + left].waterHeight) / (2 /* * Dx */);
            zGrad = (globalGridInfos[up * gridWidth + i].waterHeight - globalGridInfos[down * gridWidth + i].waterHeight) / (2 /* * Dy */);

            vertex.velocity.setX((vertex.velocity.x() + (gravity * xGrad))*dampingFactor);
            vertex.velocity.setY((vertex.velocity.y() + (gravity * zGrad))*dampingFactor);
        }
    }
}


float SWEFluid::InterpolateWaterH(const std::vector<VertexData>& source, float x, float z) {
    // Clamp indices to within the grid boundaries
    int x0 = std::max(0, std::min(static_cast<int>(floor(x)), gridWidth - 1));
    int x1 = std::max(0, std::min(x0 + 1, gridWidth - 1));
    int z0 = std::max(0, std::min(static_cast<int>(floor(z)), gridDepth - 1));
    int z1 = std::max(0, std::min(z0 + 1, gridDepth - 1));

    // Retrieve water heights at the corners
    float q00 = source[z0 * gridWidth + x0].waterHeight;
    float q10 = source[z0 * gridWidth + x1].waterHeight;
    float q01 = source[z1 * gridWidth + x0].waterHeight;
    float q11 = source[z1 * gridWidth + x1].waterHeight;

    // Bilinear interpolation
    float r0 = ((x1 - x) * q00) + ((x - x0) * q10);
    float r1 = ((x1 - x) * q01) + ((x - x0) * q11);
    return ((z1 - z) * r0) + ((z - z0) * r1);
}


QVector2D SWEFluid::InterpolateVelocity(const std::vector<VertexData>& source, float x, float z) {
    // Clamp indices to within the grid boundaries
    int x0 = std::max(0, std::min(static_cast<int>(floor(x)), gridWidth - 1));
    int x1 = std::max(0, std::min(x0 + 1, gridWidth - 1));
    int z0 = std::max(0, std::min(static_cast<int>(floor(z)), gridDepth - 1));
    int z1 = std::max(0, std::min(z0 + 1, gridDepth - 1));

    QVector2D v00 = source[z0 * gridWidth + x0].velocity;
    QVector2D v10 = source[z0 * gridWidth + x1].velocity;
    QVector2D v01 = source[z1 * gridWidth + x0].velocity;
    QVector2D v11 = source[z1 * gridWidth + x1].velocity;

    QVector2D r0 = ((x1 - x) * v00) + ((x - x0) * v10);
    QVector2D r1 = ((x1 - x) * v01) + ((x - x0) * v11);

    return ((z1 - z) * r0) + ((z - z0) * r1);
}



void SWEFluid::computeNormal() {
    for (int x = 1; x < gridWidth-1; ++x) {
        for (int z =1; z < gridDepth-1; ++z) {

            float heightLeft = getWaterHeight(x - 1, z);
            float heightRight = getWaterHeight(x + 1, z);
            float heightDown = getWaterHeight(x, z + 1);
            float heightUp = getWaterHeight(x+1, z + 1);

            QVector3D normal = calculateNormal(heightLeft, heightRight, heightDown, heightUp);

            int index = z * gridWidth + x;
            if (index >= 0 && index < globalGridInfos.size()) {
                globalGridInfos[index].normal = normal;
            }
        }
    }
}

QVector3D SWEFluid::calculateNormal(float heightLeft, float heightRight, float heightDown, float heightUp) {
    QVector3D rightVector(1.0f, heightRight - heightLeft, 0.0f);
    QVector3D upVector(0.0f, heightUp - heightDown, 1.0f);
    QVector3D normal = QVector3D::crossProduct(upVector, rightVector).normalized();

    return normal;
}


void SWEFluid::updateVertexBuffer() {
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), static_cast<int>(vertices.size()) * sizeof(VertexData));
    arrayBuf.release();
}

float  SWEFluid::getWaterHeight(float x, float z){
    // Clamp x and z to within the grid boundaries
    int gridX = std::max(1, std::min(static_cast<int>(round(x)), gridWidth - 1));
    int gridZ = std::max(1, std::min(static_cast<int>(round(z)), gridDepth - 1));

    // Calculate the index in the globalGridInfos vector
    int index = gridZ * gridWidth + gridX;
    if(vertexIndexMap[x][z] != -1){

        return globalGridInfos[index].waterHeight;
    }
    else{
        return BaseWaterHeight;
    }

}


QVector2D  SWEFluid::getWaterVelocity(float x, float z){

    int gridX = std::max(1, std::min(static_cast<int>(round(x)), gridWidth - 1));
    int gridZ = std::max(1, std::min(static_cast<int>(round(z)), gridDepth - 1));
    if(vertexIndexMap[gridX][gridZ] != -1)
    {
        return vertices[vertexIndexMap[gridX][gridZ]].velocity;
    }else{
        return QVector2D();
    }
}



void SWEFluid::CreateInitialWave(Border border) {
    // Define the initial wave height
    const float initialWaveHeight = 15.0f;

    // Depending on the border, initialize the wave
    switch (border) {
    case Border::South:
        setSemiCircularWave(1, 100, 20,initialWaveHeight);
        break;
    case Border::East:
         setSemiCircularWave(100,1, 20,initialWaveHeight);
        break;
    case Border::West:
        setSemiCircularWave(100,198, 20,initialWaveHeight);
        break;
    case Border::North:
        setSemiCircularWave(198, 100, 20,initialWaveHeight);
        break;
    }
}

void SWEFluid::setSemiCircularWave(int centerX, int centerZ, double radius, double waveHeight) {
    for (int x = 1; x < gridWidth-1; x++) {
        for (int z = 1; z < gridDepth-1; z++) {
            double distance = sqrt(pow(x - centerX, 2) + pow(z - centerZ, 2));
            if (distance <= radius && z <= centerZ) {
                setWaterHeightAt(x, z, waveHeight);
            }
        }
    }
}


void SWEFluid::setWaterHeightAt(int x, int z, double height) {
    if (x >= 0 && x < gridWidth && z >= 0 && z < gridDepth) {
        int index = z * gridWidth + x;

        globalGridInfos[index].waterHeight += height;

        globalGridInfos[index].position.setY(vertices[index].groundHeight + height);
    }
}

