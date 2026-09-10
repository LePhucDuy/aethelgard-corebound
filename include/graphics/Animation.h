#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <raylib.h>

/**
 * @brief Lớp Animation đóng gói logic hoạt họa cắt frame từ Spritesheet.
 * 
 * Tính năng:
 * - Cắt frame tự động theo chiều ngang của spritesheet.
 * - Hỗ trợ lật mặt trái / phải (Flip horizontal) theo hướng di chuyển.
 * - Hỗ trợ chế độ lặp (Looping cho Idle, Run, Fly) hoặc chạy 1 lần (One-shot cho Attack, Dead, Hit).
 */
class Animation {
private:
    std::string textureId;
    int totalFrames;
    int frameWidth;
    int frameHeight;
    float frameDuration; // Thời gian mỗi frame (giây)
    float timer;
    int currentFrame;
    bool isFacingRight;
    bool isLoop;
    bool isFinished;

public:
    Animation();
    Animation(const std::string& textureId, int totalFrames, int frameWidth, int frameHeight, 
              float frameDuration = 0.12f, bool isLoop = true);

    void update(float deltaTime);
    void draw(Vector2 position, float scale = 2.0f, Color tint = WHITE) const;

    void reset();
    void setFacingRight(bool right);
    bool getFacingRight() const;
    bool hasFinished() const;

    void setTextureId(const std::string& id, int frames, int width, int height, 
                      float duration = 0.12f, bool loop = true);
    int getCurrentFrame() const;
    int getFrameWidth() const { return frameWidth; }
    int getFrameHeight() const { return frameHeight; }
};

#endif // ANIMATION_H
