#include "sqlite3.h"
#include <iostream>
#include <optional>
#include <string>
#include "sqlite_interface.hpp"
#include <string_view>
#include <stdexcept>
#include <utility>
#include <vector>

StorageManager::StorageManager(const std::string& path): db_path(path), db(nullptr) {
    
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open DB");
    }
}

StorageManager::~StorageManager() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}


void StorageManager::initialize_schema() {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql::schema.data(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to initialize db with error: " << err_msg << "\n";
        sqlite3_free(err_msg);
    }
}

std::optional<Preset> StorageManager::create_preset(std::string_view name) {
    sqlite3_stmt *stmt = nullptr;
    Preset preset;
    preset.name = name;
    if (sqlite3_prepare_v2(db, sql::create_preset.data(),
                static_cast<int>(sql::create_preset.size()), 
                &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, name.data(), static_cast<int>(name.size()), SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << "\n";
    }
    preset.id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    return preset;
}


// TODO: handle db exceptions better
void StorageManager::create_mods_in_batch(std::vector<Mod>& mods) {
    if (mods.empty()) return;

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql::add_mod.data(), 
                static_cast<int>(sql::add_mod.size()), 
                &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << '\n';
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return;
    }

    for (auto& mod: mods) {
        sqlite3_bind_text(stmt, 1, mod.name.c_str(), static_cast<int>(mod.name.size()), SQLITE_STATIC);

        sqlite3_bind_text(stmt, 2, mod.origin.c_str(), static_cast<int>(mod.origin.size()), SQLITE_STATIC);
        
        sqlite3_bind_text(stmt, 3, mod.path.c_str(), static_cast<int>(mod.path.size()), SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
                    std::cerr << "Failed inserting record: " << sqlite3_errmsg(db) << '\n';
                    sqlite3_finalize(stmt);
                    sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
                    return;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        mod.id = static_cast<int>(sqlite3_last_insert_rowid(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
}

void StorageManager::create_mod(Mod& mod) {
    sqlite3_stmt *stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, sql::add_mod.data(), 
                static_cast<int>(sql::add_mod.size()), 
                &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << '\n';
        return;
    }

    sqlite3_bind_text(stmt, 1, mod.name.c_str(), static_cast<int>(mod.name.size()), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, mod.origin.c_str(), static_cast<int>(mod.origin.size()), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, mod.path.c_str(), static_cast<int>(mod.path.size()), SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed inserting record: " << sqlite3_errmsg(db) << '\n';
        sqlite3_finalize(stmt);
        return;
    }
    
    mod.id = static_cast<int>(sqlite3_last_insert_rowid(db));
    
    sqlite3_finalize(stmt);
}

void StorageManager::link_mods_to_preset(const std::vector<Mod>& mods, const Preset& preset) {
    if (mods.empty()) return;

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql::insert_preset_link.data(), 
                static_cast<int>(sql::insert_preset_link.size()), 
                &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare error: " << sqlite3_errmsg(db) << '\n';
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    int increment = 0;
    for (const auto& mod: mods) {
        sqlite3_bind_int(stmt, 1, preset.id);
        sqlite3_bind_int(stmt, 2, mod.id);
        sqlite3_bind_int(stmt, 3, increment);
        increment++;

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Step error linking mod " << mod.id << ": " << sqlite3_errmsg(db) << '\n';
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
}

std::vector<Mod> StorageManager::select_preset_by_name(const std::string& name) {
    std::vector<Mod> mods;
    sqlite3_stmt *stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, sql::select_preset.data(),
                static_cast<int>(sql::select_preset.size()),
                &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << '\n';
        return mods;
    }

    sqlite3_bind_text(stmt, 1, name.data(), static_cast<int>(name.size()), SQLITE_STATIC);

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Mod mod;
        mod.id = sqlite3_column_int(stmt, 0); if (const unsigned char* name_txt = sqlite3_column_text(stmt, 1)) {
            mod.name = reinterpret_cast<const char*>(name_txt);
        }
        
        if (const unsigned char* origin_txt = sqlite3_column_text(stmt, 2)) {
            mod.origin = reinterpret_cast<const char*>(origin_txt);
        }
        
        if (const unsigned char* path_txt = sqlite3_column_text(stmt, 3)) {
            mod.path = reinterpret_cast<const char*>(path_txt);
        }
        
        mods.push_back(std::move(mod));
        
    }
    sqlite3_finalize(stmt);

    return mods;
}

std::vector<Mod> StorageManager::select_preset(const Preset& preset) {
    std::vector<Mod> mods;
    sqlite3_stmt *stmt = nullptr;
    
    const char* query_data = sql::select_preset.data();
    int query_size = static_cast<int>(sql::select_preset.size());
    
    if (sqlite3_prepare_v2(db, query_data, query_size, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << '\n';
        return mods;
    }

    sqlite3_bind_text(stmt, 1, preset.name.data(), static_cast<int>(preset.name.size()), SQLITE_STATIC);

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        Mod mod;
        mod.id = sqlite3_column_int(stmt, 0); 
        
        if (const unsigned char* name_txt = sqlite3_column_text(stmt, 1)) {
            mod.name = reinterpret_cast<const char*>(name_txt);
        }
        
        if (const unsigned char* origin_txt = sqlite3_column_text(stmt, 2)) {
            mod.origin = reinterpret_cast<const char*>(origin_txt);
        }
        
        if (const unsigned char* path_txt = sqlite3_column_text(stmt, 3)) {
            mod.path = reinterpret_cast<const char*>(path_txt);
        }
        
        mods.push_back(std::move(mod));
    }
    sqlite3_finalize(stmt);

    return mods;

}
