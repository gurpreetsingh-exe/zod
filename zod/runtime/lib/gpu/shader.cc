#include "gpu/shader.hh"
#include "core/span.hh"

namespace zod {

// TODO: remove this & use from core
auto read_file(const fs::path& path) -> String {
  if (not fs::exists(path)) {
    eprintln("`{}` does not exist", path.string());
  }

  FILE* f = std::fopen(path.c_str(), "r");
  if (not f) {
    eprintln("cannot open `{}`\n  {}", path.c_str(), std::strerror(errno));
  }

  if (std::fseek(f, 0L, SEEK_END)) {
    eprintln("{}", std::strerror(errno));
  }

  auto file_size = std::ftell(f);
  if (file_size < 0) {
    eprintln("cannot get file size\n  {}", std::strerror(errno));
  }

  std::rewind(f);
  auto size = zod::usize(file_size);
  auto content = String();
  content.resize(size);
  auto read_bytes = std::fread(content.data(), 1, size, f);
  std::fclose(f);
  if (read_bytes != size) {
    int e = std::feof(f);
    if (e) {
      eprintln("unexpected end of file");
    } else {
      e = std::ferror(f);
      eprintln("cannot read file\n  {}", std::strerror(e));
    }
  }

  return content;
}

auto GPUShaderCreateInfo::vertex_in(VertexIn in) -> GPUShaderCreateInfo& {
  inputs.push_back(in);
  return *this;
}

auto GPUShaderCreateInfo::fragment_out(FragmentOut out)
    -> GPUShaderCreateInfo& {
  outputs.push_back(out);
  return *this;
}

auto GPUShaderCreateInfo::varying(Varying varying) -> GPUShaderCreateInfo& {
  varyings.push_back(varying);
  return *this;
}

auto GPUShaderCreateInfo::vertex_source(const char* src)
    -> GPUShaderCreateInfo& {
  m_shader_bits |= GPU_VERTEX_SHADER_BIT;
  m_vertex_source = src;
  return *this;
}

auto GPUShaderCreateInfo::fragment_source(const char* src)
    -> GPUShaderCreateInfo& {
  m_shader_bits |= GPU_FRAGMENT_SHADER_BIT;
  m_fragment_source = src;
  return *this;
}

auto GPUShaderCreateInfo::compute_source(const char* src)
    -> GPUShaderCreateInfo& {
  m_shader_bits |= GPU_COMPUTE_SHADER_BIT;
  m_compute_source = src;
  return *this;
}

auto GPUShaderCreateInfo::get_vertex_source() const -> const char* {
  return m_vertex_generated.c_str();
}

auto GPUShaderCreateInfo::get_fragment_source() const -> const char* {
  return m_fragment_generated.c_str();
}

auto GPUShaderCreateInfo::get_compute_source() const -> const char* {
  return m_compute_generated.c_str();
}

auto preprocess(const char* source) -> String {
  usize i = 0;
  auto generated = String();

  auto lines = Vector<Span<const char>>();
  auto begin = i;
  while (source[i]) {
    if (source[i] == '\n') {
      const auto line = Span(&source[begin], i - begin);
      lines.push_back(line);
      begin = i + 1;
    }
    i += 1;
  }

  for (const auto l : lines) {
    const auto line = String(l.data(), l.data() + l.size());
    if (line.starts_with("#include")) {
      auto b = line.find("\"");
      if (b != String::npos) {
        auto e = line.rfind("\"");
        if (e == String::npos) {
          eprintln("closing \" not found");
        }
        auto file = line.substr(b + 1, e - b - 1);
        auto path = fs::path(GPU_SHADERS_DIR) / file;
        auto content = read_file(path);
        content = preprocess(content.c_str());
        generated += content + "\n";
      }
    } else {
      generated += line + "\n";
    }
  }

  return generated;
}

auto GPUShaderCreateInfo::generate() -> void {
  const auto version = 460;
  const auto version_string = fmt::format("#version {}", version);
  if (m_shader_bits == (GPU_VERTEX_SHADER_BIT | GPU_FRAGMENT_SHADER_BIT)) {
    m_vertex_generated += version_string + "\n";
    m_vertex_generated += preprocess(m_vertex_source);

    m_fragment_generated += version_string + "\n";
    m_fragment_generated += preprocess(m_fragment_source);
  } else {
    m_compute_generated += version_string + "\n";
    m_compute_generated += m_compute_source;
  }
}

auto ShaderLibrary::add(const String& name, SharedPtr<GPUShader> shader)
    -> void {
  if (m_shaders.contains(name)) {
    fmt::println("shader `{}` already present", name);
  }
  m_shaders.insert({ name, shader });
}

auto ShaderLibrary::get(const String& name) -> SharedPtr<GPUShader> {
  ZASSERT(m_shaders.contains(name), "shader `{}` not found", name);
  return m_shaders[name];
}

} // namespace zod
