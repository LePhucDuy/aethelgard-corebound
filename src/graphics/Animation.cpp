#include "graphics/Animation.h"
#include "graphics/TextureManager.h"

Animation::Animation() 
    : textureId(""), totalFrames(1), frameWidth(0), frameHeight(0),
      frameDuration(0.12f), timer(0.0f), currentFrame(0),
      isFacingRight(true), isLoop(true), isFinished(false) {}

Animation::Animation(const std::string& textureId, int totalFrames, int frameWidth, int frameHeight, 
                     float frameDuration, bool isLoop)
    : textureId(textureId), totalFrames(totalFrames), frameWidth(frameWidth), frameHeight(frameHeight),
      frameDuration(frameDuration), timer(0.0f), currentFrame(0),
      isFacingRight(true), isLoop(isLoop), isFinished(false) {}

void Animation::update(float deltaTime) {
    if (isFinished && !isLoop) return;

    timer += deltaTime;
    if (timer >= frameDuration) {
        timer = 0.0f;
        currentFrame++;
        if (currentFrame >= totalFrames) {
            if (isLoop) {
                currentFrame = 0;
            } else {
                currentFrame = totalFrames - 1;
                isFinished = true;
            }
        }
    }
}

void Animation::draw(Vector2 position, float scale, Color tint) const {
    const Texture2D& tex = TextureManager::getInstance().get(textureId);
    if (tex.id == 0) return;

    // Cắt frame hiện tại
    // Nếu quay sang trái, gán width âm để lật ảnh (Sprite Flip)
    Rectangle sourceRec = {
        (float)(currentFrame * frameWidth),
        0.0f,
        isFacingRight ? (float)frameWidth : -(float)frameWidth,
        (float)frameHeight
    };

    Rectangle destRec = {
        position.x,
        position.y,
        (float)frameWidth * scale,
        (float)frameHeight * scale
    };

    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(tex, sourceRec, destRec, origin, 0.0f, tint);
}

void Animation::reset() {
    currentFrame = 0;
    timer = 0.0f;
    isFinished = false;
}

void Animation::setFacingRight(bool right) {
    isFacingRight = right;
}

bool Animation::getFacingRight() const {
    return isFacingRight;
}

bool Animation::hasFinished() const {
    return isFinished;
}

void Animation::setTextureId(const std::string& id, int frames, int width, int height, 
                            float duration, bool loop) {
    textureId = id;
    totalFrames = frames;
    frameWidth = width;
    frameHeight = height;
    frameDuration = duration;
    isLoop = loop;
    reset();
}

int Animation::getCurrentFrame() const {
    return currentFrame;
}
