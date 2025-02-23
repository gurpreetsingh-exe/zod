#pragma once

#include "./vk_common.hh"
#include "gpu/pipeline.hh"

namespace zod {

class VKPipeline : public GPUPipeline {
public:
  VKPipeline(PipelineSpec);

private:
  auto clear() -> void;
  auto make() -> VkPipeline;
  auto set_input_topology(VkPrimitiveTopology) -> void;
  auto set_polygon_mode(VkPolygonMode) -> void;
  auto set_cull_mode(VkCullModeFlags) -> void;
  auto set_multisampling_none() -> void;

private:
  Vector<VkPipelineShaderStageCreateInfo> m_shader_stages;

  VkPipelineInputAssemblyStateCreateInfo m_input_assembly;
  VkPipelineRasterizationStateCreateInfo m_rasterizer;
  VkPipelineColorBlendAttachmentState m_color_blend_attachment;
  VkPipelineMultisampleStateCreateInfo m_multisampling;
  VkPipelineLayout m_pipeline_layout;
  VkPipelineDepthStencilStateCreateInfo m_depth_stencil;
  VkPipelineRenderingCreateInfoKHR m_render_info;
  VkFormat m_color_attachment_format;
};

} // namespace zod
