#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& db_path) : db_path_(db_path), db_(nullptr) {
}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool DatabaseManager::initialize() {
    std::lock_guard<std::mutex> lock(db_mutex_);

    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << "\n";
        return false;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS detections (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            class_name TEXT NOT NULL,
            confidence REAL NOT NULL,
            x REAL,
            y REAL,
            w REAL,
            h REAL,
            timestamp TEXT NOT NULL,
            image_data TEXT
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

int DatabaseManager::saveDetection(const std::string& class_name, double confidence,
                                   double x, double y, double w, double h,
                                   const std::string& timestamp, const std::string& image_data) {
    std::lock_guard<std::mutex> lock(db_mutex_);

    const char* sql = "INSERT INTO detections (class_name, confidence, x, y, w, h, timestamp, image_data) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return -1;
    }

    sqlite3_bind_text(stmt, 1, class_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, confidence);
    sqlite3_bind_double(stmt, 3, x);
    sqlite3_bind_double(stmt, 4, y);
    sqlite3_bind_double(stmt, 5, w);
    sqlite3_bind_double(stmt, 6, h);
    sqlite3_bind_text(stmt, 7, timestamp.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, image_data.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db_) << "\n";
        sqlite3_finalize(stmt);
        return -1;
    }

    int last_id = static_cast<int>(sqlite3_last_insert_rowid(db_));
    sqlite3_finalize(stmt);
    return last_id;
}

std::vector<json> DatabaseManager::getAllDetections() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<json> results;

    const char* sql = "SELECT id, class_name, confidence, x, y, w, h, timestamp, image_data FROM detections;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
        return results;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json row;
        row["id"] = sqlite3_column_int(stmt, 0);
        
        const unsigned char* class_name = sqlite3_column_text(stmt, 1);
        if (class_name) row["class_name"] = reinterpret_cast<const char*>(class_name);
        
        row["confidence"] = sqlite3_column_double(stmt, 2);
        row["x"] = sqlite3_column_double(stmt, 3);
        row["y"] = sqlite3_column_double(stmt, 4);
        row["w"] = sqlite3_column_double(stmt, 5);
        row["h"] = sqlite3_column_double(stmt, 6);
        
        const unsigned char* timestamp = sqlite3_column_text(stmt, 7);
        if (timestamp) row["timestamp"] = reinterpret_cast<const char*>(timestamp);
        
        const unsigned char* image_data = sqlite3_column_text(stmt, 8);
        if (image_data) row["image_data"] = reinterpret_cast<const char*>(image_data);

        results.push_back(row);
    }

    sqlite3_finalize(stmt);
    return results;
}
