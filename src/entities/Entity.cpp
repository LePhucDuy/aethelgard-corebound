#include "entities/Entity.h"
#include "core/Constants.h"
#include <algorithm>

Entity::Entity(const std::string& name, const Position& pos, int hp, int attack, int defense)
    : name(name), pos(pos), hp(hp), maxHp(hp), attack(attack), defense(defense), alive(true), currentAnim(nullptr) {}

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
}

void Entity::render(float scale, Vector2 offset) const {
    if (!currentAnim) return;

    float fWidth = (float)currentAnim->getFrameWidth() * scale;
    float fHeight = (float)currentAnim->getFrameHeight() * scale;

    // QUY ƯỚC MỚI: pos = ô WALKABLE (mặt cỏ / bệ đá) mà thực thể đang đứng.
    // Chân (đáy sprite) chạm mép dưới ô đó: pos.y * TILE_SIZE - fHeight + TILE_SIZE.
    Vector2 screenPos = {
        (float)(pos.x * Constants::TILE_SIZE) + ((float)Constants::TILE_SIZE - fWidth) / 2.0f + offset.x,
        (float)(pos.y * Constants::TILE_SIZE) - fHeight + (float)Constants::TILE_SIZE + offset.y
    };

    currentAnim->draw(screenPos, scale);
}

void Entity::setAnimation(std::unique_ptr<Animation> anim) {
    currentAnim = anim.release();  // Giải phóng quyền sở hữu từ unique_ptr, gán raw pointer
}
