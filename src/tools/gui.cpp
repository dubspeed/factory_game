#include "gui.h"
#include "imgui.h"

bool Gui::StatefulButton(std::string const &label, bool *state, const ImVec2 &size) {
    if (*state) {
        ImGui::BeginDisabled(true);
        ImGui::Button(label.c_str(), size);
        ImGui::EndDisabled();
    } else {
        if (ImGui::Button(label.c_str(), size)) {
            *state = true;
            return true;
        }
    }
    return false;
}

bool Gui::StatefulButton(std::string const &label, const std::shared_ptr<bool> &state, const ImVec2 &size) {
    return StatefulButton(label, state.get(), size);
}

