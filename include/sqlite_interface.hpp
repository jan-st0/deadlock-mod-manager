#pragma once

#include <optional>
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

struct Preset {
    int id;
    std::string name;
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

    inline constexpr query insert_preset_link = R"sql(
INSERT INTO Preset_Link (preset_id, mod_id, load_order) VALUES(?, ?, ?);
)sql";

    inline constexpr query select_preset = R"sql(
SELECT m.id, m.name, m.origin, m.path
FROM Mods m
JOIN Preset_Link pl ON m.id = pl.mod_id
JOIN Presets p ON pl.preset_id = p.id
WHERE p.name = ?
ORDER BY pl.load_order ASC;
)sql";

}

class StorageManager {
private:
    struct sqlite3 *db;

    //May be used in later versions or for diagnostics
    std::string_view dbPath;

public:
    StorageManager(const std::string& dbPath);
    ~StorageManager();

    std::vector<Mod> select_preset_by_name(const std::string& name);
    void initialize_schema();
    std::optional<Preset> create_preset(std::string_view name);
    // returns updated mod vector with actual id for each mod
    void create_mods_in_batch(std::vector<Mod>& mods);
    void link_mods_to_preset(const std::vector<Mod>& mods, const Preset& preset);
};
