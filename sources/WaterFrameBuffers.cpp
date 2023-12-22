#include "../headers/WaterFrameBuffers.h"
#include <QOpenGLFramebufferObject>

WaterFrameBuffers::WaterFrameBuffers() {
    initializeOpenGLFunctions();
}

WaterFrameBuffers::~WaterFrameBuffers() {
    cleanUp();
}

void WaterFrameBuffers::initialize() {
    // Initialize reflection framebuffer
    reflectionFrameBuffer = createFrameBuffer();
    reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    
    // Initialize refraction framebuffer
    refractionFrameBuffer = createFrameBuffer();
    refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
    refractionDepthTexture = createDepthBufferAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WaterFrameBuffers::cleanUp() {
    glDeleteFramebuffers(1, &reflectionFrameBuffer);
    glDeleteTextures(1, &reflectionTexture);
    glDeleteRenderbuffers(1, &reflectionDepthBuffer);
    
    glDeleteFramebuffers(1, &refractionFrameBuffer);
    glDeleteTextures(1, &refractionTexture);
    glDeleteTextures(1, &refractionDepthTexture);
}

void WaterFrameBuffers::bindReflectionFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFrameBuffer);
    glViewport(0, 0, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

void WaterFrameBuffers::bindRefractionFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, refractionFrameBuffer);
    glViewport(0, 0, REFRACTION_WIDTH, REFRACTION_HEIGHT);
}

void WaterFrameBuffers::unbindCurrentFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint WaterFrameBuffers::getReflectionTexture() const {
    return reflectionTexture;
}

GLuint WaterFrameBuffers::getRefractionTexture() const {
    return refractionTexture;
}

GLuint WaterFrameBuffers::getRefractionDepthTexture() const {
    return refractionDepthTexture;
}

GLuint WaterFrameBuffers::createFrameBuffer() {
    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    return frameBuffer;
}

GLuint WaterFrameBuffers::createTextureAttachment(int width, int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    return texture;
}

GLuint WaterFrameBuffers::createDepthBufferAttachment(int width, int height) {
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    return depthBuffer;
}
