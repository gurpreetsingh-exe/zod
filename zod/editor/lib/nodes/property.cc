#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "property.hh"

namespace zod {

static constexpr int padding = 70;

Property::Property(Property&& prop)
    : name(prop.name), type(prop.type), subtype(prop.subtype) {
  switch (prop.type) {
    case PROP_STRING: {
      s = prop.s;
      prop.s = nullptr;
    } break;
    case PROP_INT: {
      i = prop.i;
    } break;
    case PROP_FLOAT: {
      f = prop.f;
    } break;
    case PROP_VEC3: {
      v3 = prop.v3;
    } break;
  }
}

static auto DragScalar(const char* label, ImGuiDataType data_type, void* p_data,
                       float v_speed, const void* p_min, const void* p_max,
                       const char* format, ImGuiSliderFlags flags, ImVec4 color)
    -> bool {
  auto* window = ImGui::GetCurrentWindow();
  if (window->SkipItems) {
    return false;
  }

  auto& g = *GImGui;
  auto* list = window->DrawList;
  auto& style = g.Style;
  auto id = window->GetID(label);
  auto w = ImGui::CalcItemWidth();

  auto label_size = ImGui::CalcTextSize(label, NULL, true);
  auto frame_bb =
      ImRect(window->DC.CursorPos,
             window->DC.CursorPos +
                 ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
  auto total_bb =
      ImRect(frame_bb.Min,
             frame_bb.Max + ImVec2(label_size.x > 0.0f
                                       ? style.ItemInnerSpacing.x + label_size.x
                                       : 0.0f,
                                   0.0f));

  auto temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
  ImGui::ItemSize(total_bb, style.FramePadding.y);
  if (not ImGui::ItemAdd(total_bb, id, &frame_bb,
                         temp_input_allowed ? ImGuiItemFlags_Inputable : 0)) {
    return false;
  }

  if (not format) {
    format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;
  }

  auto hovered = ImGui::ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
  auto temp_input_is_active =
      temp_input_allowed and ImGui::TempInputIsActive(id);
  if (not temp_input_is_active) {
    auto clicked =
        hovered and ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);
    auto double_clicked = (hovered and g.IO.MouseClickedCount[0] == 2 and
                           ImGui::TestKeyOwner(ImGuiKey_MouseLeft, id));
    auto make_active = (clicked or double_clicked or g.NavActivateId == id);
    if (make_active and (clicked or double_clicked)) {
      ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
    }
    if (make_active and temp_input_allowed) {
      if ((clicked and g.IO.KeyCtrl) or double_clicked or
          (g.NavActivateId == id and
           (g.NavActivateFlags & ImGuiActivateFlags_PreferInput))) {
        temp_input_is_active = true;
      }
    }

    if (g.IO.ConfigDragClickToInputText and temp_input_allowed and
        not temp_input_is_active) {
      if (g.ActiveId == id and hovered and g.IO.MouseReleased[0] and
          not ImGui::IsMouseDragPastThreshold(0,
                                              g.IO.MouseDragThreshold * 0.5f)) {
        g.NavActivateId = id;
        g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
        temp_input_is_active = true;
      }
    }

    if (make_active and not temp_input_is_active) {
      ImGui::SetActiveID(id, window);
      ImGui::SetFocusID(id, window);
      ImGui::FocusWindow(window);
      g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }
  }

  if (temp_input_is_active) {
    auto is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0 and
                          (p_min == NULL or p_max == NULL or
                           ImGui::DataTypeCompare(data_type, p_min, p_max) < 0);
    return ImGui::TempInputScalar(frame_bb, id, label, data_type, p_data,
                                  format, is_clamp_input ? p_min : NULL,
                                  is_clamp_input ? p_max : NULL);
  }

  auto frame_col = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive
                                      : hovered        ? ImGuiCol_FrameBgHovered
                                                       : ImGuiCol_FrameBg);
  ImGui::RenderNavHighlight(frame_bb, id);

  list->AddRectFilled(frame_bb.Min, frame_bb.Max, frame_col,
                      style.FrameRounding);
  auto border_size = g.Style.FrameBorderSize;
  if (border_size > 0.0f) {
    list->AddRect(frame_bb.Min + ImVec2(1, 1), frame_bb.Max + ImVec2(1, 1),
                  ImGui::GetColorU32(ImGuiCol_BorderShadow),
                  style.FrameRounding, 0, border_size);
    list->AddRect(frame_bb.Min, frame_bb.Max,
                  ImGui::GetColorU32(ImGuiCol_Border), style.FrameRounding, 0,
                  border_size);
  }

  auto value_changed = ImGui::DragBehavior(id, data_type, p_data, v_speed,
                                           p_min, p_max, format, flags);
  if (value_changed) {
    ImGui::MarkItemEdited(id);
  }

  char value_buf[64];
  const char* value_buf_end =
      value_buf + ImGui::DataTypeFormatString(value_buf,
                                              IM_ARRAYSIZE(value_buf),
                                              data_type, p_data, format);
  ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end,
                           NULL, ImVec2(0.5f, 0.5f));

  if (label_size.x > 0.0f) {
    ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                             frame_bb.Min.y + style.FramePadding.y),
                      label);
  }

  ImGui::PushStyleColor(ImGuiCol_Button, color);
  list->AddRectFilled(frame_bb.Min,
                      ImVec2(frame_bb.Min.x + 6.0f, frame_bb.Max.y),
                      ImGui::GetColorU32(ImGuiCol_Button), style.FrameRounding,
                      ImDrawFlags_RoundCornersLeft);

  ImGui::PopStyleColor();

  return value_changed;
}

auto DragFloat3(const char* name, f32* value) -> bool {
  auto needs_update = false;
  ImGui::BeginGroup();
  ImGui::AlignTextToFramePadding();
  ImGui::Text(name);
  ImGui::SameLine(padding);
  ImGui::PushID(name);
  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
  needs_update =
      DragScalar("##X", ImGuiDataType_Float, &value[0], 1.0f, nullptr, nullptr,
                 nullptr, 0, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
  ImGui::PopItemWidth();
  ImGui::SameLine();
  needs_update |=
      DragScalar("##Y", ImGuiDataType_Float, &value[1], 1.0f, nullptr, nullptr,
                 nullptr, 0, ImVec4(0.1f, 0.8f, 0.1f, 1.0f));
  ImGui::PopItemWidth();
  ImGui::SameLine();
  needs_update |=
      DragScalar("##Z", ImGuiDataType_Float, &value[2], 1.0f, nullptr, nullptr,
                 nullptr, 0, ImVec4(0.1f, 0.3f, 0.8f, 1.0f));
  ImGui::PopItemWidth();
  ImGui::PopStyleVar();
  ImGui::PopID();
  ImGui::EndGroup();
  return needs_update;
}

auto Property::draw() -> bool {
  switch (type) {
    case PROP_STRING: {
      ImGui::BeginGroup();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(name);
      ImGui::SameLine(padding);
      needs_update = ImGui::InputText("##", s, 64);
      if (subtype == PROP_SUBTYPE_FILEPATH) {
        ImGui::SameLine();
        ImGui::Button("Open");
      }
      ImGui::EndGroup();
    } break;
    case PROP_INT: {
      needs_update = ImGui::DragInt(name, &i);
    } break;
    case PROP_FLOAT: {
      needs_update = ImGui::DragFloat(name, &f);
    } break;
    case PROP_VEC3: {
      needs_update = DragFloat3(name, &v3[0]);
    } break;
  }
  return needs_update;
}

Property::~Property() {
  switch (type) {
    case PROP_STRING: {
      delete[] s;
    } break;
  }
}

} // namespace zod
