#include "cc/renderer/camera/camera.hpp"
#include "pipeline/pipeline.hpp"

namespace cc{


// template<typename T>
// concept renderable = requires(T a, const gfx::Pipeline& pipeline){ {
//     {a.draw(pipeline)} -> std::same_as<void>;
// };
//
class Renderer {
    public:
    Renderer() = default;
    ~Renderer() = default;



    void Begin(const Camera& camera )noexcept;
    void End() noexcept;


    // void SetViewport(u32 x, u32 y, u32 width, u32 height) noexcept;


    private:

};




}

