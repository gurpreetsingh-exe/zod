#pragma once

#include "../shader.hh"
#include "vk_common.hh"

namespace zod {

enum ShaderStage {
  Vertex,
  Fragment,
  Compute,
};

class VKShader : public GPUShader {
public:
  VKShader(std::string /* name */);

public:
  auto init_vertex_shader(const char* /* source */) -> void override;
  auto init_fragment_shader(const char* /* source */) -> void override;
  auto init_compute_shader(const char* /* source */) -> void override;
  auto compile() -> void override;
  auto bind() -> void override;
  auto unbind() -> void override;
  auto dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void override;
  auto uniform_float(const std::string&, const f32*, usize /* size */)
      -> void override;
  auto uniform_uint(const std::string&, const u32*, usize /* size */)
      -> void override;
  auto uniform_int(const std::string&, const i32*, usize /* size */)
      -> void override;

private:
  auto create_shader_module(ShaderStage, const char* /* source */)
      -> VkShaderModule;
  auto create_shader_stage(VkShaderModule, VkShaderStageFlagBits) -> void;

private:
  VkShaderModule m_vert = VK_NULL_HANDLE;
  VkShaderModule m_frag = VK_NULL_HANDLE;
  VkShaderModule m_comp = VK_NULL_HANDLE;
  std::vector<VkPipelineShaderStageCreateInfo> m_stages = {};
};

} // namespace zod
