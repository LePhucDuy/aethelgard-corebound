#include "graphics/TextureManager.h"

TextureManager::~TextureManager() {
    unloadAll();
}

TextureManager& TextureManager::getInstance() {
    static TextureManager instance;
    return instance;
}

bool TextureManager::load(const std::string& id, const std::string& filePath) {
    // Nếu đã nạp rồi thì không nạp lại
    if (textures.find(id) != textures.end()) {
        return true;
    }

    Texture2D tex = LoadTexture(filePath.c_str());
    if (tex.id == 0) {
        std::cerr << "[TextureManager Error] Khong the nap file: " << filePath << std::endl;
        return false;
    }

    textures[id] = tex;
    std::cout << "[TextureManager] Da nap texture '" << id << "' tu: " << filePath 
              << " (" << tex.width << "x" << tex.height << ")" << std::endl;
    return true;
}

const Texture2D& TextureManager::get(const std::string& id) const {
    auto it = textures.find(id);
    if (it != textures.end()) {
        return it->second;
    }

    static Texture2D emptyTexture = { 0, 0, 0, 1, 7 };
    std::cerr << "[TextureManager Warning] Khong tim thay texture ID: " << id << std::endl;
    return emptyTexture;
}

bool TextureManager::has(const std::string& id) const {
    return textures.find(id) != textures.end();
}

void TextureManager::unloadAll() {
    for (auto& pair : textures) {
        if (pair.second.id != 0) {
            UnloadTexture(pair.second);
        }
    }
    textures.clear();
}
