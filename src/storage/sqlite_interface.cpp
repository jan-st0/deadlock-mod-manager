#include "sqlite3.h"
#include <iostream>
#include <string>
#include "sqlite_interface.hpp"
#include <string_view>
#include <stdexcept>
#include <vector>

StorageManager::StorageManager(const std::string& path): dbPath(path), db(nullptr) {
    
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::runtime_error("Failed to open DB");
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

void StorageManager::create_preset(std::string_view name) {
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql::create_preset.data(),
                static_cast<int>(sql::create_preset.size()), 
                &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }
    
    sqlite3_bind_text(stmt, 1, name.data(), static_cast<int>(name.size()), SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << "\n";
    }
    
    sqlite3_finalize(stmt);

}

void StorageManager::create_mods_in_batch(const std::vector<Mod>& mods) {
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

    for (const Mod& mod: mods) {
        sqlite3_bind_text(stmt, 1, mod.name.c_str(), static_cast<int>(mod.name.size()), SQLITE_STATIC);

        sqlite3_bind_text(stmt, 1, mod.origin.c_str(), static_cast<int>(mod.origin.size()), SQLITE_STATIC);
        
        sqlite3_bind_text(stmt, 1, mod.path.c_str(), static_cast<int>(mod.path.size()), SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
                    std::cerr << "Failed inserting record: " << sqlite3_errmsg(db) << '\n';
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
