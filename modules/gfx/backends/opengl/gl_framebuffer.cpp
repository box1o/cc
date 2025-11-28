#include "gl_framebuffer.hpp"
#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>
#include <glad/glad.h>
#include <vector>

namespace cc::gfx {

void OpenGLFramebufferImpl::AttachTexture(unsigned int attachmentPoint, Texture* texture) const {
    if (texture == nullptr) return;

    const u32 textureHandle = texture->GetHandle();

    if (texture->GetType() == TextureType::TextureCube) {
        glNamedFramebufferTextureLayer(handle_, attachmentPoint, textureHandle, 0, 0);
    } else {
        glNamedFramebufferTexture(handle_, attachmentPoint, textureHandle, 0);
    }
}

OpenGLFramebufferImpl::OpenGLFramebufferImpl(
    u32 width,
    u32 height,
    const std::unordered_map<u32, Texture*>& colorAttachments,
    Texture* depthAttachment
)
    : width_(width), height_(height) {

    glCreateFramebuffers(1, &handle_);
    if (handle_ == 0) {
        log::Critical("Failed to create OpenGL framebuffer");
        return;
    }

    std::vector<unsigned int> drawBuffers;
    for (const auto& [index, texture] : colorAttachments) {
        const unsigned int attachmentPoint = GL_COLOR_ATTACHMENT0 + index;
        AttachTexture(attachmentPoint, texture);
        drawBuffers.push_back(attachmentPoint);
    }

    if (! drawBuffers.empty()) {
        glNamedFramebufferDrawBuffers(handle_, static_cast<int>(drawBuffers. size()), drawBuffers.data());
    } else {
        glNamedFramebufferDrawBuffer(handle_, GL_NONE);
        glNamedFramebufferReadBuffer(handle_, GL_NONE);
    }

    if (depthAttachment != nullptr) {
        TextureFormat format = depthAttachment->GetFormat();
        if (format == TextureFormat::Depth24Stencil8 || format == TextureFormat::Depth32FStencil8) {
            AttachTexture(GL_DEPTH_STENCIL_ATTACHMENT, depthAttachment);
        } else {
            AttachTexture(GL_DEPTH_ATTACHMENT, depthAttachment);
        }
    }

    if (! CheckComplete()) {
        log::Error("Framebuffer is incomplete");
    }

    log::Trace("OpenGL Framebuffer created (handle={}, {}x{})", handle_, width_, height_);
}

OpenGLFramebufferImpl::~OpenGLFramebufferImpl() {
    if (handle_ != 0) {
        glDeleteFramebuffers(1, &handle_);
        log::Trace("OpenGL Framebuffer destroyed (handle={})", handle_);
        handle_ = 0;
    }
}

void OpenGLFramebufferImpl::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
}

void OpenGLFramebufferImpl::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool OpenGLFramebufferImpl::CheckComplete() const {
    const unsigned int status = glCheckNamedFramebufferStatus(handle_, GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char* errorMsg = "Unknown error";
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED: errorMsg = "Framebuffer undefined"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errorMsg = "Incomplete attachment"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorMsg = "Missing attachment"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: errorMsg = "Incomplete draw buffer"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: errorMsg = "Incomplete read buffer"; break;
            case GL_FRAMEBUFFER_UNSUPPORTED: errorMsg = "Unsupported framebuffer configuration"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: errorMsg = "Incomplete multisample"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: errorMsg = "Incomplete layer targets"; break;
        }
        log::Error("Framebuffer incomplete: {}", errorMsg);
        return false;
    }

    return true;
}

OpenGLFramebuffer::OpenGLFramebuffer(
    u32 width,
    u32 height,
    scope<FramebufferImpl> impl,
    std::unordered_map<u32, Texture*> colorTextures,
    Texture* depthTexture
)
    : width_(width)
    , height_(height)
    , impl_(std::move(impl))
    , colorTextures_(std::move(colorTextures))
    , depthTexture_(depthTexture) {}

OpenGLFramebuffer::~OpenGLFramebuffer() = default;

void OpenGLFramebuffer::Bind() const {
    impl_->Bind();
}

void OpenGLFramebuffer::Unbind() const {
    impl_->Unbind();
}

Texture* OpenGLFramebuffer::GetColorTexture(u32 index) const {
    auto it = colorTextures_.find(index);
    if (it == colorTextures_.end()) {
        return nullptr;
    }
    return it->second;
}

Texture* OpenGLFramebuffer::GetDepthTexture() const {
    return depthTexture_;
}

scope<Framebuffer> CreateOpenGLFramebuffer(
    Device*,
    u32 width,
    u32 height,
    const std::unordered_map<u32, Texture*>& colorAttachments,
    Texture* depthAttachment
) {
    auto impl = scope<FramebufferImpl>(new OpenGLFramebufferImpl(width, height, colorAttachments, depthAttachment));
    return scope<Framebuffer>(new OpenGLFramebuffer(width, height, std::move(impl), colorAttachments, depthAttachment));
}

class OpenGLDefaultFramebuffer final : public Framebuffer {
public:
    OpenGLDefaultFramebuffer(u32 width, u32 height) : width_(width), height_(height) {}
    ~OpenGLDefaultFramebuffer() override = default;

    u32 GetWidth() const override { return width_; }
    u32 GetHeight() const override { return height_; }
    u32 GetHandle() const override { return 0; }

    void Bind() const override { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
    void Unbind() const override { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    Texture* GetColorTexture(u32) const override { return nullptr; }
    Texture* GetDepthTexture() const override { return nullptr; }

private:
    u32 width_;
    u32 height_;
};

scope<Framebuffer> CreateOpenGLDefaultFramebuffer(Device*, u32 width, u32 height) {
    return scope<Framebuffer>(new OpenGLDefaultFramebuffer(width, height));
}

} // namespace cc::gfx
