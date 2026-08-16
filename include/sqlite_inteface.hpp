#pragma once

#include <string>
#include <vector>
#include <sqlite3.h>

struct Mod {
    int id;
    std::string name;
    std::string origin;
    std::string path;
};

class StorageManager {
private:
    struct sqlite3 *db;

public:
    StorageManager(const std::string& dbPath);
    ~StorageManager();

    void addMod(const Mod& mod);
    std::vector<Mod> selectPreset(const std::string& name);
    //void createPreset(const std::vector<Mod>& )
};
