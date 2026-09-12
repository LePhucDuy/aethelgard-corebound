#ifndef DAMAGE_POPUP_H
#define DAMAGE_POPUP_H

#include <string>
#include "raylib.h"

/**
 * @brief Đối tượng hiển thị số sát thương nổi (Floating Damage Text)
 * Được quản lý bởi cấu trúc mảng động tự viết `DynamicArray<DamagePopup>`.
 */
struct DamagePopup {
    std::string text;
    float x;
    float y;
    Color color;
    float lifetime;
    float maxLifetime;

    DamagePopup()
        : text(""), x(0.0f), y(0.0f), color(YELLOW), lifetime(0.8f), maxLifetime(0.8f) {}

    DamagePopup(const std::string& text, float x, float y, Color color = YELLOW, float duration = 0.8f)
        : text(text), x(x), y(y), color(color), lifetime(duration), maxLifetime(duration) {}

    void update(float dt) {
        y -= 38.0f * dt; // Bay bổng lên trên
        lifetime -= dt;
    }

    bool isAlive() const {
        return lifetime > 0.0f;
    }

    float getAlpha() const {
        if (maxLifetime <= 0.0f) return 1.0f;
        float ratio = lifetime / maxLifetime;
        return ratio > 1.0f ? 1.0f : (ratio < 0.0f ? 0.0f : ratio);
    }
};

#endif // DAMAGE_POPUP_H

