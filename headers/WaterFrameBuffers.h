#ifndef WATERFRAMEBUFFERS_H
#define WATERFRAMEBUFFERS_H

#include <QOpenGLFunctions>

class WaterFrameBuffers : protected QOpenGLFunctions {
public:
    WaterFrameBuffers();
    ~WaterFrameBuffers();

    void initialize();
    void cleanUp();

    void bindReflectionFrameBuffer();
    void bindRefractionFrameBuffer();
    void unbindCurrentFrameBuffer();

    GLuint getReflectionTexture() const;
    GLuint getRefractionTexture() const;
    GLuint getRefractionDepthTexture() const;
    GLuint getReflectionFrameBuffer() const { return reflectionFrameBuffer; }
    GLuint getRefractionFrameBuffer() const { return refractionFrameBuffer; }
    const int REFLECTION_WIDTH = 320;
    const int REFLECTION_HEIGHT = 180;
    const int REFRACTION_WIDTH = 1280;
    const int REFRACTION_HEIGHT = 720;

private:

    GLuint reflectionFrameBuffer;
    GLuint reflectionTexture;
    GLuint reflectionDepthBuffer;

    GLuint refractionFrameBuffer;
    GLuint refractionTexture;
    GLuint refractionDepthTexture;

    GLuint createFrameBuffer();
    GLuint createTextureAttachment(int width, int height);
    GLuint createDepthBufferAttachment(int width, int height);
};

#endif // WATERFRAMEBUFFERS_H
