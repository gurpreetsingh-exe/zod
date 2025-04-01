#include <imgui.h>

#include "content_browser.hh"
#include "editor.hh"

namespace zod {

ContentBrowser::ContentBrowser()
    : SPanel("Content Browser", shared<OrthographicCamera>(64.0f, 64.0f)),
      m_base_directory(Editor::get().get_project()->assets_directory()),
      m_current_directory(m_base_directory) {}

auto ContentBrowser::update() -> void {
  if (m_current_directory != m_base_directory) {
    if (ImGui::Button("<-")) {
      m_current_directory = m_current_directory.parent_path();
    }
  }

  static auto padding = 16.0f;
  static auto thumbnail_size = 128.0f;
  auto cell_size = thumbnail_size + padding;

  auto panel_width = ImGui::GetContentRegionAvail().x;
  auto column_count = static_cast<i32>(panel_width / cell_size);
  if (column_count < 1) {
    column_count = 1;
  }

  ImGui::Columns(column_count, 0, false);

  for (auto& dir : fs::directory_iterator(m_current_directory)) {
    const auto& path = dir.path();
    auto filename_string = path.filename().string();

    ImGui::PushID(filename_string.c_str());
    // Ref<Texture2D> icon =
    //     dir.is_directory() ? m_DirectoryIcon : m_FileIcon;
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    // ImGui::ImageButton((ImTextureID)icon->GetRendererID(),
    //                    { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
    ImGui::Button(filename_string.c_str(), { thumbnail_size, thumbnail_size });

    if (ImGui::BeginDragDropSource()) {
      auto relative_path = path;
      const char* item_path = relative_path.c_str();
      ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path,
                                (strlen(item_path) + 1) * sizeof(char));
      ImGui::EndDragDropSource();
    }

    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered() and
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      if (dir.is_directory())
        m_current_directory /= path.filename();
    }
    ImGui::TextWrapped(filename_string.c_str());

    ImGui::NextColumn();

    ImGui::PopID();
  }

  ImGui::Columns(1);

  // ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
  // ImGui::SliderFloat("Padding", &padding, 0, 32);

  // TODO: status bar
}

} // namespace zod
