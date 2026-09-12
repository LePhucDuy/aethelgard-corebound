#include "entities/Entity.h"
#include "core/Constants.h"
#include <algorithm>

Entity::Entity(const std::string& name, const Position& pos, int hp, int attack, int defense)
    : IGameObject(name),
      name(name), pos(pos),
      visualPos{ (float)(pos.x * Constants::TILE_SIZE), (float)(pos.y * Constants::TILE_SIZE) },
      moveLerpSpeed(20.0f),
      hp(hp), maxHp(hp), attack(attack), defense(defense), alive(true), currentAnim(nullptr) {}

void Entity::resetVisualPosition() {
    visualPos = { (float)(pos.x * Constants::TILE_SIZE), (float)(pos.y * Constants::TILE_SIZE) };
}

void Entity::takeDamage(int amount) {
    // Sát thương thực tế = lượng dame trừ đi chỉ số phòng thủ (tối thiểu chịu 1 sát thương)
    int actualDamage = std::max(1, amount - defense);
    hp -= actualDamage;

    if (hp <= 0) {
        hp = 0;
        alive = false;
    }
}

void Entity::heal(int amount) {
    if (!alive) return;
    hp = std::min(maxHp, hp + amount);
}

void Entity::update(float deltaTime) {
    if (currentAnim) {
        currentAnim->update(deltaTime);
    }

    // Nội suy mượt mà tọa độ hiển thị (Visual LERP) theo thời gian thực độc lập với FPS
    float targetX = (float)(pos.x * Constants::TILE_SIZE);
    float targetY = (float)(pos.y * Constants::TILE_SIZE);
    float t = 1.0f - std::exp(-moveLerpSpeed * deltaTime);
    visualPos.x += (targetX - visualPos.x) * t;
    visualPos.y += (targetY - visualPos.y) * t;

    if (std::abs(visualPos.x - targetX) < 0.25f) visualPos.x = targetX;
    if (std::abs(visualPos.y - targetY) < 0.25f) visualPos.y = targetY;
}

void Entity::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    float fWidth = (float)currentAnim->getFrameWidth() * scale;
    float fHeight = (float)currentAnim->getFrameHeight() * scale;

    // QUY ƯỚC MỚI: neo chân theo tọa độ visualPos lướt mượt
    constexpr float FOOT_SINK = 6.0f;
    Vector2 screenPos = {
        visualPos.x + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        visualPos.y + FOOT_SINK - fHeight + offset.y
    };

    currentAnim->draw(screenPos, scale);
}

void Entity::setAnimation(std::unique_ptr<Animation> anim) {
    currentAnim = anim.release();  // Giải phóng quyền sở hữu từ unique_ptr, gán raw pointer
}
