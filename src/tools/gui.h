
#ifndef GUI_H
#define GUI_H
#include <memory>
#include <string>
#include "imgui.h"

namespace Gui {
    bool StatefulButton(std::string const &label, bool *state, const ImVec2 &size = ImVec2(0,0));
    bool StatefulButton(std::string const &label, const std::shared_ptr<bool> &state, const ImVec2 &size = ImVec2(0,0));
}
#endif //GUI_H
