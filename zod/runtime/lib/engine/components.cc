#include "engine/components.hh"

#include <yaml-cpp/yaml.h>

namespace YAML {

template <>
struct convert<zod::vec3> {
  static Node encode(const zod::vec3& v) {
    Node node;
    node.push_back(v.x);
    node.push_back(v.y);
    node.push_back(v.z);
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, zod::vec3& v) {
    if (not node.IsSequence() or node.size() != 3) {
      return false;
    }

    v.x = node[0].as<zod::f32>();
    v.y = node[1].as<zod::f32>();
    v.z = node[2].as<zod::f32>();
    return true;
  }
};

template <>
struct convert<zod::vec4> {
  static Node encode(const zod::vec4& v) {
    Node node;
    node.push_back(v.x);
    node.push_back(v.y);
    node.push_back(v.z);
    node.push_back(v.w);
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, zod::vec4& v) {
    if (not node.IsSequence() or node.size() != 4) {
      return false;
    }

    v.x = node[0].as<zod::f32>();
    v.y = node[1].as<zod::f32>();
    v.z = node[2].as<zod::f32>();
    v.w = node[3].as<zod::f32>();
    return true;
  }
};

template <>
struct convert<zod::mat4> {
  static Node encode(const zod::mat4& mat) {
    Node node;
    node.push_back(mat[0]);
    node.push_back(mat[1]);
    node.push_back(mat[2]);
    node.push_back(mat[3]);
    node.SetStyle(EmitterStyle::Flow);
    return node;
  }

  static bool decode(const Node& node, zod::mat4& mat) {
    if (not node.IsSequence() or node.size() != 4) {
      return false;
    }

    mat[0] = node[0].as<zod::vec4>();
    mat[1] = node[1].as<zod::vec4>();
    mat[2] = node[2].as<zod::vec4>();
    mat[3] = node[3].as<zod::vec4>();
    return true;
  }
};

} // namespace YAML

namespace zod {

auto operator<<(YAML::Emitter& out, const vec3& v) -> YAML::Emitter& {
  out << YAML::Flow;
  out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
  return out;
}

template <typename T>
static auto write_field(YAML::Emitter& out, const String& key, const T& value)
    -> void {
  out << YAML::Key << key << YAML::Value << value;
}

/// IdentifierComponent
IdentifierComponent::IdentifierComponent(const String& name) {
  auto sz = name.size() >= 64L ? 64L : name.size();
  std::memcpy(&identifier, name.c_str(), sz);
}

auto IdentifierComponent::serialize(YAML::Emitter& out) -> void {
  out << YAML::BeginMap;
  const auto name = String(identifier);
  write_field(out, "Identifier", name);
  out << YAML::EndMap;
}

auto IdentifierComponent::deserialize(YAML::Node node) -> void {
  auto name = node["Identifier"].as<String>();
  *this = IdentifierComponent(name);
}

/// UUIDComponent
UUIDComponent::UUIDComponent(YAML::Node node) { deserialize(node); }

auto UUIDComponent::serialize(YAML::Emitter& out) -> void {
  out << YAML::BeginMap;
  write_field(out, "UUID", id.to_string());
  out << YAML::EndMap;
}

auto UUIDComponent::deserialize(YAML::Node node) -> void {
  id = UUID(node["UUID"].as<String>());
}

/// TransformComponent
auto TransformComponent::operator*() -> mat4 {
  auto tmat = translate(mat4(1.0f), position);
  auto rmat = toMat4(quat(radians(rotation)));
  auto smat = glm::scale(mat4(1.0f), scale);
  return tmat * rmat * smat;
}

auto TransformComponent::serialize(YAML::Emitter& out) -> void {
  out << YAML::BeginMap;
  write_field(out, "Position", position);
  write_field(out, "Rotation", rotation);
  write_field(out, "Scale", scale);
  out << YAML::EndMap;
}

auto TransformComponent::deserialize(YAML::Node node) -> void {
  position = node["Position"].as<vec3>();
  rotation = node["Rotation"].as<vec3>();
  scale = node["Scale"].as<vec3>();
}

auto CameraComponent::serialize(YAML::Emitter& out) -> void {
  out << YAML::BeginMap;
  out << YAML::Key << "PerspectiveCamera" << YAML::Value << YAML::BeginMap;
  write_field(out, "Fov", camera->get_fov());
  auto [near, far] = camera->get_clipping();
  write_field(out, "ClipNear", near);
  write_field(out, "ClipFar", far);
  out << YAML::EndMap;
}

auto CameraComponent::deserialize(YAML::Node node) -> void {
  const auto& cnode = node["PerspectiveCamera"];
  camera->set_fov(cnode["Fov"].as<f32>());
  camera->set_clipping(cnode["ClipNear"].as<f32>(), cnode["ClipFar"].as<f32>());
  camera->update_matrix();
}

// SkyboxComponent
auto SkyboxComponent::serialize(YAML::Emitter& out) -> void {
  out << YAML::BeginMap;
  if (env.mode == LightingMode::SolidColor) {
    out << YAML::Key << "SolidColor" << YAML::Value << YAML::BeginMap;
    write_field(out, "Color", env.color.v3);
  } else if (env.mode == LightingMode::Texture) {
    out << YAML::Key << "Texture" << YAML::Value << YAML::BeginMap;
    write_field(out, "HDRI", env.hdr.s);
  } else {
    PANIC();
  }
  out << YAML::EndMap;
}

auto SkyboxComponent::deserialize(YAML::Node node) -> void {
  if (node["SolidColor"]) {
    env.mode = LightingMode::SolidColor;
    const auto& solid_color = node["SolidColor"];
    env.color.v3 = solid_color["Color"].as<vec3>();
  } else if (node["Texture"]) {
    env.mode = LightingMode::Texture;
    const auto& hdri = node["Texture"];
    std::memcpy(env.hdr.s, hdri["HDRI"].as<String>().c_str(),
                STRING_PROP_MAX_SIZE);
  } else {
    PANIC("invalid field in SkyboxComponent");
  }
}

// LightComponent
auto LightComponent::serialize(YAML::Emitter& out) -> void {
  out << YAML::BeginMap;
  // TODO: sun light
  ZASSERT(kind == LightKind::Point);
  out << YAML::Key << "Point" << YAML::Value << YAML::BeginMap;
  write_field(out, "RadiusInner", a);
  write_field(out, "RadiusOuter", b);
  write_field(out, "Intensity", intensity);
  out << YAML::EndMap;
}

auto LightComponent::deserialize(YAML::Node node) -> void {
  // TODO: sun light
  ZASSERT(node["Point"]);
  kind = LightKind::Point;
  const auto& point_light = node["Point"];
  a = point_light["RadiusInner"].as<f32>();
  b = point_light["RadiusOuter"].as<f32>();
  intensity = point_light["Intensity"].as<f32>();
}

} // namespace zod
