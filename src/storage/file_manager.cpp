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

void FileManager::create_mod_from_path(std::string& name, fs::path& absolute_path, StorageManager& db_cursor) {
    // TODO: handle mod names conflict or mod update
    fs::path mod_dir = base_mod_path / name.data();
    fs::create_directory(mod_dir);
    // TODO: add option to bundle multiple files into 1 mod
    fs::path mod_path = mod_dir / "0.vpk";
    fs::copy(absolute_path, mod_path, fs::copy_options::overwrite_existing);
    Mod mod = {.name = name, .origin = "", .path = mod_path};
    db_cursor.create_mod(mod);
}

inline void clean_addon_dir(const fs::path& addon_folder_path) {
    fs::remove_all(addon_folder_path);
    fs::create_directory(addon_folder_path);
}

void FileManager::load_preset_to_citadel(const Preset& preset, StorageManager& db_cursor) {
    std::vector<Mod> mods = db_cursor.select_preset(preset);
    clean_addon_dir(addons_folder);
    int prefix = 0;
    std::string filename;
    for (const auto& mod: mods) {
        filename = std::format("pak{:02d}_dir.vpk", prefix);
        fs::copy(mod.path, addons_folder / filename);
        prefix++;
    }
}