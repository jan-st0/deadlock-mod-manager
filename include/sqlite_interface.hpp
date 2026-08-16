#pragma once

#include <string>
#include <vector>
#include <sqlite3.h>
#include <string_view>

struct Mod {
    int id;
    std::string name;
    std::string origin;
    std::string path;
};

namespace sql {
    using query = std::string_view;
    inline constexpr query schema = R"sql(
CREATE TABLE Mods (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    origin TEXT NOT NULL,
    path TEXT
);

CREATE TABLE Presets (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE Preset_Link (
    preset_id INTEGER NOT NULL,
    mod_id INTEGER NOT NULL,
    load_order INTEGER DEFAULT 0,
    PRIMARY KEY (preset_id, mod_id),
    FOREIGN KEY (preset_id) REFERENCES Presets(id) ON DELETE CASCADE,
    FOREIGN KEY (mod_id) REFERENCES Mods(id) ON DELETE CASCADE
) WITHOUT ROWID;

)sql";
    
    inline constexpr query create_preset = R"sql(
INSERT INTO Presets (name) VALUES(?);
)sql";

    inline constexpr query add_mod = R"sql(
INSERT INTO Mods (name, origin, path) VALUES(?, ?, ?);
)sql";
}

class StorageManager {
private:
    struct sqlite3 *db;
    std::string_view dbPath;

public:
    StorageManager(const std::string& dbPath);
    ~StorageManager();

    void addMod(const Mod& mod);
    std::vector<Mod> selectPreset(const std::string& name);
    void initialize_schema();
    void create_preset(std::string_view name);
    void create_mods_in_batch(const std::vector<Mod>& mods);
};
