#pragma once

#include <filesystem>
#include <string_view>
#include "sqlite_interface.hpp"
#include <fstream>

namespace fs = std::filesystem;


static void read_line_from_file(std::ifstream& file, std::string& line) {
    std::getline(file, line);
    if (!line.empty() && line.back() == '\r') {
            line.pop_back();
    }
}

class FileManager{
private:
    fs::path base_mod_path = "./mods"; 
    fs::path addons_folder = "C:/Steam/steamapps/common/Deadlock/game/citadel"; 
    fs::path cache_file = "./last_preset.txt";
    Preset cache_preset;

    void load_last_preset_from_cache() {
        std::ifstream file(cache_file);
        std::string line;
        read_line_from_file(file, line);
        cache_preset.id = std::stoi(line);
        read_line_from_file(file, line);
        cache_preset.name = line;
        file.close();
    };

    void send_preset_to_cache() {
        std::ofstream file(cache_file, std::ios::out | std::ios::trunc);
        file << cache_preset.id << "\n";
        file << cache_preset.name << "\n";
        file.close();
    }
public:
    FileManager();
    ~FileManager();
    void copy_mod(std::string_view name, fs::path absolute_path);
    void execute_preset(const Preset& preset, const StorageManager& db_cursor);

    void set_addons_folder(fs::path new_addons) { addons_folder = std::move(new_addons); }

    const Preset& get_last_preset() const { return cache_preset;}

};
