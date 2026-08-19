#include "file_manager.hpp"
#include "sqlite_interface.hpp"
#include <filesystem>
#include <format>

FileManager::FileManager() {
    if (!fs::exists(base_mod_path)) {
        fs::create_directory(base_mod_path);
    }

    load_last_preset_from_cache();
}

FileManager::~FileManager() {
    send_preset_to_cache();
}

fs::path FileManager::save_mod_to_archive(std::string& name, fs::path& absolute_path) {
    // TODO: handle mod names conflict or mod update
    fs::path mod_dir = base_mod_path / name.data();
    fs::create_directory(mod_dir);
    // TODO: add option to bundle multiple files into 1 mod
    fs::path mod_path = mod_dir / "0.vpk";
    fs::copy(absolute_path, mod_path, fs::copy_options::overwrite_existing);
    return mod_path;
}

inline void clean_addon_dir(const fs::path& addon_folder_path) {
    fs::remove_all(addon_folder_path);
    fs::create_directory(addon_folder_path);
}


void FileManager::load_preset_to_citadel(const std::vector<Mod>& preset) {
    clean_addon_dir(addons_folder);
    int prefix = 0;
    std::string filename;
    for (const auto& mod: preset) {
        filename = std::format("pak{:02d}_dir.vpk", prefix);
        fs::copy(mod.path, addons_folder / filename);
        prefix++;
    }
}