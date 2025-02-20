#pragma once

#include "./vk_common.hh"
#include "gpu/shader.hh"

namespace zod {

enum ShaderStage {
  Vertex,
  Fragment,
  Compute,
};

class VKShader : public GPUShader {
public:
  VKShader(GPUShaderCreateInfo);

public:
  auto bind() -> void override;
  auto unbind() -> void override;
  auto dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void override;
  auto uniform_float(const std::string&, const f32*, usize /* size */)
      -> void override;
  auto uniform_uint(const std::string&, const u32*, usize /* size */)
      -> void override;
  auto uniform_int(const std::string&, const i32*, usize /* size */)
      -> void override;

public:
  auto get_shader_stages() const
      -> const std::vector<VkPipelineShaderStageCreateInfo>& {
    return m_stages;
  }

private:
  auto init_vertex_and_fragment_shader(const char* /* vertex_source */,
                                       const char* /* fragment_source */)
      -> void override;
  auto init_compute_shader(const char* /* source */) -> void override;
  auto compile(GPUShaderCreateInfo) -> void override;

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
