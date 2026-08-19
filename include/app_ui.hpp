#pragma once

#include "imgui.h"
#include "sqlite_interface.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <functional>

class AppUI {
public:
    AppUI();
    
    // Dedicated load/update function controlled by the Manager
    void update_state(const Preset& preset, const std::vector<Mod>& p_mods, const std::vector<Mod>& a_mods);
    void render();

    // Callbacks (Events emitted to the Manager)
    std::function<void(const std::string&, const std::filesystem::path&)> on_import_mod;
    std::function<void(const std::string&, const std::vector<Mod>&)> on_save_preset;
    std::function<void(const std::vector<Mod>&)> on_preset_reordered;

private:
    void apply_statlocker_style();
    
    // Internal State
    Preset current_preset;
    std::vector<Mod> preset_mods;
    std::vector<Mod> all_mods;

    bool view_all_mods = false;
    bool is_dirty = false;

    // UI Input State
    char new_preset_name[256] = "";
    bool show_new_preset_popup = false;
    bool show_file_manager = false;
    bool show_add_mod_popup = false;
    std::filesystem::path current_browse_path;
    std::filesystem::path selected_file_path;
    char new_mod_name[256] = "";

    void render_preset_view();
    void render_all_mods_view();
    void render_popups();
    void render_file_browser();
    
    bool has_mod(int mod_id);
};