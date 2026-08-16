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
