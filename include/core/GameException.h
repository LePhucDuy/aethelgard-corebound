#ifndef GAME_EXCEPTION_H
#define GAME_EXCEPTION_H

#include <stdexcept>
#include <string>

/**
 * @brief Lớp cơ sở ngoại lệ của trò chơi (C++ Exception Handling - Kế thừa từ std::runtime_error)
 */
class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Ngoại lệ phát sinh khi tệp dữ liệu lưu game bị hỏng, rỗng hoặc sai định dạng
 */
class SaveLoadException : public GameException {
public:
    explicit SaveLoadException(const std::string& message)
        : GameException("[SaveLoadException] " + message) {}
};

/**
 * @brief Ngoại lệ phát sinh khi không tìm thấy hoặc lỗi tài nguyên game
 */
class ResourceLoadException : public GameException {
public:
    explicit ResourceLoadException(const std::string& message)
        : GameException("[ResourceLoadException] " + message) {}
};

#endif // GAME_EXCEPTION_H

