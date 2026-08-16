#include "file_manager.hpp"
#include "sqlite_interface.hpp"
#include <filesystem>

FileManager::FileManager() {
    if (!fs::exists(base_mod_path)) {
        fs::create_directory(base_mod_path);
    }

    load_last_preset_from_cache();
}

FileManager::~FileManager() {
    send_preset_to_cache();
}

void FileManager::copy_mod(std::string_view name, fs::path absolute_path) {
    // TODO: handle mod names conflict or mod update
    fs::path mod_dir = base_mod_path / name.data();
    fs::create_directory(mod_dir);
    // TODO: add option to bundle multiple files into 1 mod
    fs::copy(absolute_path, mod_dir / "0.vpk", fs::copy_options::overwrite_existing);
    
}

void FileManager::execute_preset(const Preset& preset, const StorageManager& db_cursor) {
    std::vector<Mod> mods = db_cursor.select_preset_by_name()
}