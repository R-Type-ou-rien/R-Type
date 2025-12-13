#pragma once

#include <string>

#include "sqlite3.h"

class Database {
   private:
    sqlite3* db = nullptr;
    bool bConnected = false;

   public:
    Database(const std::string& filename);
    ~Database();

   private:
    void _initialize();

   public:
    bool RegisterUser(std::string username, std::string password);
    int LoginUser(std::string username, std::string password);
    void SaveToken(int userID, std::string token);
    int GetUserByToken(std::string token);
    void UpdateScore(int userID, int newScore);
    std::string GetNameById(int userId);
    std::string GetTokenById(int userId);
};