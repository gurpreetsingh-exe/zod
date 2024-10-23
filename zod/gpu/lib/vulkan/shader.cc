#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include "vulkan/backend.hh"
#include "vulkan/shader.hh"

#define FAILWITH(msg)                                                          \
  do {                                                                         \
    fmt::println(stderr, "{}", msg);                                           \
    fmt::println(stderr, "{}", glslang_shader_get_info_log(shader));           \
    fmt::println(stderr, "{}", glslang_shader_get_info_debug_log(shader));     \
  } while (0)

namespace zod {

static auto glslang_get_shader_stage(ShaderStage stage) -> glslang_stage_t {
  switch (stage) {
    case ShaderStage::Vertex:
      return GLSLANG_STAGE_VERTEX;
    case ShaderStage::Fragment:
      return GLSLANG_STAGE_FRAGMENT;
    case ShaderStage::Compute:
      return GLSLANG_STAGE_COMPUTE;
    default:
      UNREACHABLE();
  }
}

auto compile_glsl_to_spirv(ShaderStage stage0, const char* source)
    -> std::vector<u32> {
  const auto stage = glslang_get_shader_stage(stage0);
  const auto input = glslang_input_t {
    .language = GLSLANG_SOURCE_GLSL,
    .stage = stage,
    .client = GLSLANG_CLIENT_VULKAN,
    .client_version = GLSLANG_TARGET_VULKAN_1_2,
    .target_language = GLSLANG_TARGET_SPV,
    .target_language_version = GLSLANG_TARGET_SPV_1_5,
    .code = source,
    .default_version = 100,
    .default_profile = GLSLANG_NO_PROFILE,
    .force_default_version_and_profile = false,
    .forward_compatible = false,
    .messages = GLSLANG_MSG_DEFAULT_BIT,
    .resource = glslang_default_resource(),
  };

  auto* shader = glslang_shader_create(&input);
  if (not glslang_shader_preprocess(shader, &input)) {
    FAILWITH("GLSL preprocessing failed");
    glslang_shader_delete(shader);
    return {};
  }

  if (not glslang_shader_parse(shader, &input)) {
    FAILWITH("GLSL parsing failed");
    glslang_shader_delete(shader);
    return {};
  }

  auto* program = glslang_program_create();
  glslang_program_add_shader(program, shader);
  if (not glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT |
                                            GLSLANG_MSG_VULKAN_RULES_BIT)) {
    FAILWITH("GLSL linking failed");
    glslang_program_delete(program);
    glslang_shader_delete(shader);
    return {};
  }

  glslang_program_SPIRV_generate(program, stage);
  auto size = glslang_program_SPIRV_get_size(program);
  auto binary = std::vector<u32>(size);
  glslang_program_SPIRV_get(program, binary.data());
  if (auto messages = glslang_program_SPIRV_get_messages(program)) {
    fmt::println("{}", messages);
  }

  glslang_program_delete(program);
  glslang_shader_delete(shader);
  return binary;
}

VKShader::VKShader(std::string name) : GPUShader(std::move(name)) {}

auto VKShader::init_vertex_shader(const char* source) -> void {
  m_vert = create_shader_module(ShaderStage::Vertex, source);
}

auto VKShader::init_fragment_shader(const char* source) -> void {
  m_frag = create_shader_module(ShaderStage::Fragment, source);
}

auto VKShader::init_compute_shader(const char* source) -> void {
  m_comp = create_shader_module(ShaderStage::Compute, source);
}

auto VKShader::compile() -> void {
  if (m_comp) {
    ZASSERT(m_vert == VK_NULL_HANDLE and m_frag == VK_NULL_HANDLE);
    TODO();
  } else {
    create_shader_stage(m_vert, VK_SHADER_STAGE_VERTEX_BIT);
    create_shader_stage(m_frag, VK_SHADER_STAGE_FRAGMENT_BIT);
  }
}

auto VKShader::bind() -> void {}
auto VKShader::unbind() -> void {}
auto VKShader::dispatch(u32 /* x */, u32 /* y */, u32 /* z */) -> void {}
auto VKShader::uniform_float(const std::string&, const f32*, usize /* size */)
    -> void {}
auto VKShader::uniform_uint(const std::string&, const u32*, usize /* size */)
    -> void {}
auto VKShader::uniform_int(const std::string&, const i32*, usize /* size */)
    -> void {}

auto VKShader::create_shader_module(ShaderStage stage, const char* source)
    -> VkShaderModule {
  auto spirv = compile_glsl_to_spirv(stage, source);
  auto info = VkShaderModuleCreateInfo {};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = spirv.size() * sizeof(u32);
  info.pCode = spirv.data();

  auto vk_device = VKBackend::get().device.vk_device();
  VkShaderModule module;
  VK_CHECK(vkCreateShaderModule(vk_device, &info, nullptr, &module));
  return module;
}

auto VKShader::create_shader_stage(VkShaderModule module,
                                   VkShaderStageFlagBits stage) -> void {
  VkPipelineShaderStageCreateInfo info {};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.stage = stage;
  info.module = module;
  info.pName = name.c_str();
  m_stages.push_back(info);
}

} // namespace zod
