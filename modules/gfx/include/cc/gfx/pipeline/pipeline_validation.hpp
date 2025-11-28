#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

//NOTE: Enable/disable validation at compile time
#ifndef CC_GFX_ENABLE_VALIDATION
#define CC_GFX_ENABLE_VALIDATION 1
#endif

class VertexLayout;
class Shader;
class DescriptorSetLayout;

//NOTE: Validation helpers (debug only, no-ops if disabled)
void ValidateVertexLayoutAgainstShader(const VertexLayout* layout, const Shader* shader);
void ValidateDescriptorSetLayoutAgainstShader(const DescriptorSetLayout* layout, const Shader* shader);

} // namespace cc::gfx
