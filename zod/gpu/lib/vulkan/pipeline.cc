#include "vulkan/pipeline.hh"
#include "vulkan/backend.hh"

namespace zod {

static auto to_vk(GPUPrimitive prim) -> VkPrimitiveTopology {
  switch (prim) {
    case GPUPrimitive::Points:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case GPUPrimitive::Lines:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case GPUPrimitive::LineStrip:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case GPUPrimitive::Triangles:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case GPUPrimitive::TriangleStrip:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case GPUPrimitive::TriangleFan:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
  }
}

static auto to_vk(GPUPrimitiveDrawMode mode) -> VkPolygonMode {
  switch (mode) {
    case GPUPrimitiveDrawMode::Fill:
      return VK_POLYGON_MODE_FILL;
    case GPUPrimitiveDrawMode::Line:
      return VK_POLYGON_MODE_LINE;
    case GPUPrimitiveDrawMode::Point:
      return VK_POLYGON_MODE_POINT;
  }
}

static auto to_vk(GPUCullMode mode) -> VkCullModeFlags {
  switch (mode) {
    case GPUCullMode::None:
      return VK_CULL_MODE_NONE;
    case GPUCullMode::Front:
      return VK_CULL_MODE_FRONT_BIT;
    case GPUCullMode::Back:
      return VK_CULL_MODE_BACK_BIT;
    case GPUCullMode::FrontAndBack:
      return VK_CULL_MODE_FRONT_AND_BACK;
  }
}

VKPipeline::VKPipeline(PipelineSpec spec) : GPUPipeline(spec) {
  clear();
  m_shader_stages =
      static_pointer_cast<VKShader>(m_shader)->get_shader_stages();
  set_input_topology(to_vk(spec.primitive));
  set_polygon_mode(to_vk(spec.draw_mode));
  set_cull_mode(to_vk(spec.cull_mode));
  set_multisampling_none();

  // TODO: remove
  m_color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  m_color_blend_attachment.blendEnable = VK_FALSE;

  m_color_attachment_format = VK_FORMAT_B8G8R8A8_UNORM;
  m_render_info.colorAttachmentCount = 1;
  m_render_info.pColorAttachmentFormats = &m_color_attachment_format;
  m_render_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
}

auto VKPipeline::clear() -> void {
  m_input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
  };
  m_rasterizer = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
  };
  m_color_blend_attachment = {};
  m_multisampling = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
  };
  m_pipeline_layout = {};
  m_depth_stencil = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
  };
  m_render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
  m_shader_stages.clear();
}

auto VKPipeline::set_input_topology(VkPrimitiveTopology topology) -> void {
  m_input_assembly.topology = topology;
  m_input_assembly.primitiveRestartEnable = VK_FALSE;
}

auto VKPipeline::set_polygon_mode(VkPolygonMode mode) -> void {
  m_rasterizer.polygonMode = mode;
  m_rasterizer.lineWidth = 1.f;
}

auto VKPipeline::set_cull_mode(VkCullModeFlags mode) -> void {
  m_rasterizer.cullMode = mode;
  m_rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

void VKPipeline::set_multisampling_none() {
  m_multisampling.sampleShadingEnable = VK_FALSE;
  m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_multisampling.minSampleShading = 1.0f;
  m_multisampling.pSampleMask = nullptr;
  m_multisampling.alphaToCoverageEnable = VK_FALSE;
  m_multisampling.alphaToOneEnable = VK_FALSE;
}

auto VKPipeline::make() -> VkPipeline {
  VkPipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.pNext = nullptr;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineColorBlendStateCreateInfo color_blending = {};
  color_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.pNext = nullptr;

  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &m_color_blend_attachment;

  VkPipelineVertexInputStateCreateInfo _vertex_input_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
  };
  pipeline_info.pNext = &m_render_info;
  pipeline_info.stageCount = static_cast<u32>(m_shader_stages.size());
  pipeline_info.pStages = m_shader_stages.data();
  pipeline_info.pVertexInputState = &_vertex_input_info;
  pipeline_info.pInputAssemblyState = &m_input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &m_rasterizer;
  pipeline_info.pMultisampleState = &m_multisampling;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDepthStencilState = &m_depth_stencil;
  pipeline_info.layout = m_pipeline_layout;

  VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT,
                             VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamic_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
  };
  dynamic_info.pDynamicStates = &state[0];
  dynamic_info.dynamicStateCount = 2;

  pipeline_info.pDynamicState = &dynamic_info;

  VkPipeline pipeline;
  auto device = VKBackend::get().device.vk_device();
  VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                     nullptr, &pipeline));

  return pipeline;
}

} // namespace zod
