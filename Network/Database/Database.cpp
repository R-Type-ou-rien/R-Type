#include "Database.hpp"

#include <iostream>

#include "sqlite3.h"

Database::Database(const std::string& filename) {
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[DB] Impossible d'ouvrir la DB " << filename << "\n";
        std::cerr << "[DB] Message : " << sqlite3_errmsg(db) << "\n";
        bConnected = false;
    } else {
        std::cout << "[DB] Base de donnees connectee : " << filename << "\n";
        bConnected = true;
        _initialize();
    }
}

Database::~Database() {
    if (bConnected) {
        sqlite3_close(db);
    }
}

void Database::_initialize() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS Users ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Username TEXT UNIQUE NOT NULL,"
        "Password TEXT NOT NULL,"
        "Token TEXT,"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "[DB] Erreur Init : " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

bool Database::RegisterUser(std::string username, std::string password) {
    std::string sql = "INSERT INTO Users (Username, Password) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (result == SQLITE_DONE);
}

int Database::LoginUser(std::string username, std::string password) {
    std::string sql = "SELECT ID FROM Users WHERE Username = ? AND Password = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    int userID = -1;

    if (result == SQLITE_ROW) {
        userID = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return userID;
}

void Database::SaveToken(int userID, std::string token) {
    std::string sql = "UPDATE Users SET Token = ? WHERE ID = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, userID);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int Database::GetUserByToken(std::string token) {
    std::string sql = "SELECT ID FROM Users WHERE Token = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    int userID = -1;

    if (result == SQLITE_ROW)
        userID = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return userID;
}

std::string Database::GetNameById(int userId) {
    std::string sql = "SELECT USERNAME FROM Users WHERE ID = ?;";
    sqlite3_stmt* stmt;
    std::string username = "";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return "";

    sqlite3_bind_int(stmt, 1, userId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        if (result) {
            username = std::string(result);
        }
    }

    sqlite3_finalize(stmt);

    return username;
}

std::string Database::GetTokenById(int userId) {
    std::string sql = "SELECT Token FROM Users WHERE ID = ?;";
    sqlite3_stmt* stmt;
    std::string token = "";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return "";

    sqlite3_bind_int(stmt, 1, userId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        if (result) {
            token = std::string(result);
        }
    }

    sqlite3_finalize(stmt);

    return token;
}
