#include "app_ui.hpp"
#include <iostream>
#include <algorithm>

AppUI::AppUI() {
    apply_statlocker_style();
    current_browse_path = std::filesystem::current_path();
}

bool AppUI::has_mod(int mod_id) {
    return std::any_of(preset_mods.begin(), preset_mods.end(), 
        [mod_id](const Mod& m) { return m.id == mod_id; });
}

void AppUI::update_state(const Preset& preset, const std::vector<Mod>& p_mods, const std::vector<Mod>& a_mods) {
    current_preset = preset;
    preset_mods = p_mods;
    all_mods = a_mods;
    is_dirty = false;
}

void AppUI::render() {
    // Main Window
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
    
    ImGui::Begin("Deadlock Mod Manager", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Header
    ImGui::TextColored(ImVec4(0.87f, 0.58f, 0.18f, 1.0f), "DEADLOCK MOD MANAGER");
    ImGui::SameLine();
    ImGui::Text(" | Current Preset: %s", current_preset.name.empty() ? "None" : current_preset.name.c_str());
    ImGui::Separator();

    // Top Toolbar
    if (ImGui::Button(view_all_mods ? "View Preset Mods" : "View All Mods")) {
        view_all_mods = !view_all_mods;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Add New Mod (.vpk)")) {
        show_file_manager = true;
    }

    if (view_all_mods && is_dirty) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("Create New Preset from Selection")) {
            show_new_preset_popup = true;
        }
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    // Main Content Area
    if (view_all_mods) {
        render_all_mods_view();
    } else {
        render_preset_view();
    }

    render_popups();
    render_file_browser();

    ImGui::End();
}

void AppUI::render_preset_view() {
    ImGui::Text("Drag and drop to change load order.");
    ImGui::BeginChild("PresetList", ImVec2(0, 0), true);

    for (size_t i = 0; i < preset_mods.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        
        // Render item
        ImGui::Selectable(preset_mods[i].name.c_str(), false, ImGuiSelectableFlags_AllowItemOverlap);
        
        // Drag Source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload("MOD_ORDER", &i, sizeof(size_t));
            ImGui::Text("Moving %s", preset_mods[i].name.c_str());
            ImGui::EndDragDropSource();
        }
        
        // Drag Target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MOD_ORDER")) {
                size_t payload_n = *(const size_t*)payload->Data;
                Mod tmp = preset_mods[payload_n];
                preset_mods.erase(preset_mods.begin() + payload_n);
                
                size_t insert_idx = i;
                if (payload_n < i) insert_idx = i;
                
                preset_mods.insert(preset_mods.begin() + insert_idx, tmp);
                is_dirty = true;

                if (on_preset_reordered) {
                    on_preset_reordered(preset_mods);
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void AppUI::render_all_mods_view() {
    ImGui::Text("Select mods to include in a new preset configuration.");
    ImGui::BeginChild("AllModsList", ImVec2(0, 0), true);

    for (auto& mod : all_mods) {
        bool active = has_mod(mod.id);
        if (ImGui::Checkbox(mod.name.c_str(), &active)) {
            if (active) {
                preset_mods.push_back(mod);
            } else {
                preset_mods.erase(std::remove_if(preset_mods.begin(), preset_mods.end(),
                    [&](const Mod& m) { return m.id == mod.id; }), preset_mods.end());
            }
            is_dirty = true;
        }
        ImGui::SameLine(300);
        ImGui::TextDisabled("%s", mod.path.c_str());
    }
    ImGui::EndChild();
}

void AppUI::render_file_browser() {
    if (!show_file_manager) return;
    
    ImGui::OpenPopup("Select .vpk File");
    if (ImGui::BeginPopupModal("Select .vpk File", &show_file_manager, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Current Path: %s", current_browse_path.string().c_str());
        if (ImGui::Button(".. (Up)")) {
            current_browse_path = current_browse_path.parent_path();
        }
        
        ImGui::BeginChild("FileTree", ImVec2(500, 300), true);
        try {
            for (const auto& entry : std::filesystem::directory_iterator(current_browse_path)) {
                auto path = entry.path();
                auto filename = path.filename().string();
                
                if (entry.is_directory()) {
                    if (ImGui::Selectable((std::string("[DIR] ") + filename).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (ImGui::IsMouseDoubleClicked(0)) current_browse_path = path;
                    }
                } else if (path.extension() == ".vpk") {
                    if (ImGui::Selectable(filename.c_str())) {
                        selected_file_path = std::filesystem::absolute(path);
                        show_file_manager = false;
                        show_add_mod_popup = true;
                    }
                }
            }
        } catch (...) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Permission denied or invalid directory.");
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

void AppUI::render_popups() {
    // Add Mod Popup
    if (show_add_mod_popup) {
        ImGui::OpenPopup("Name Your Mod");
        if (ImGui::BeginPopupModal("Name Your Mod", &show_add_mod_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Selected File:");
            ImGui::TextDisabled("%s", selected_file_path.string().c_str());
            ImGui::Spacing();
            
            ImGui::InputText("Mod Name", new_mod_name, IM_ARRAYSIZE(new_mod_name));
            
            if (ImGui::Button("Import Mod", ImVec2(120, 0))) {
                std::string mod_name(new_mod_name);
                if (!mod_name.empty()) {
                    if (on_import_mod) {
                        on_import_mod(mod_name, selected_file_path);
                    }
                    show_add_mod_popup = false;
                    memset(new_mod_name, 0, sizeof(new_mod_name));
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { show_add_mod_popup = false; }
            ImGui::EndPopup();
        }
    }

    // New Preset Popup
    if (show_new_preset_popup) {
        ImGui::OpenPopup("Save New Preset");
        if (ImGui::BeginPopupModal("Save New Preset", &show_new_preset_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Preset Name", new_preset_name, IM_ARRAYSIZE(new_preset_name));
            
            if (ImGui::Button("Save", ImVec2(120, 0))) {
                std::string pname(new_preset_name);
                if (!pname.empty()) {
                    if (on_save_preset) {
                        on_save_preset(pname, preset_mods);
                    }
                    show_new_preset_popup = false;
                    memset(new_preset_name, 0, sizeof(new_preset_name));
                    view_all_mods = false; 
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { show_new_preset_popup = false; }
            ImGui::EndPopup();
        }
    }
}

void AppUI::apply_statlocker_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Deep dark grays and blacks with a hint of purple/blue
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.09f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
    
    // Statlocker Amber / Orange accents
    colors[ImGuiCol_Button]                 = ImVec4(0.85f, 0.55f, 0.10f, 0.80f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.95f, 0.65f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.75f, 0.45f, 0.05f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.28f, 0.30f, 0.33f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.35f, 0.38f, 0.42f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.85f, 0.55f, 0.10f, 1.00f);

    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.WindowBorderSize  = 1.0f;
}