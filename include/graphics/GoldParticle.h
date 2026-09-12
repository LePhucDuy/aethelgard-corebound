#ifndef GOLD_PARTICLE_H
#define GOLD_PARTICLE_H

#include <raylib.h>
#include <cmath>
#include <cstdlib>

/**
 * @brief Cấu trúc GoldParticle quản lý hiệu ứng đồng tiền vàng rơi ra khi quái vật tử trận.
 * 
 * ĐẶC ĐIỂM VẬT LÝ & HIỆU ỨNG:
 * 1. Giai đoạn 1 (Parabolic Burst & Bounce):
 *    - Đồng tiền văng lên theo hình nón parabol với vận tốc ban đầu ngẫu nhiên.
 *    - Rơi chịu ảnh hưởng gia tốc trọng trường g = 700 px/s^2.
 *    - Chạm sàn nảy nhẹ giảm chấn 1-2 lần.
 * 2. Giai đoạn 2 (Magnet Attraction):
 *    - Sau khoảng thời gian trễ ngắn (0.35s - 0.5s), đồng tiền tự động gia tốc lướt về phía Player.
 * 3. Giai đoạn 3 (Collection):
 *    - Khi chạm vào bán kính nhân vật (< 24px), đồng tiền được hấp thu vào túi đồ của Player,
 *      sinh số tiền vàng nổi lấp lánh trên màn hình.
 */
struct GoldParticle {
    Vector2 pos;        // Vị trí thực tế trên bản đồ (world pixels)
    Vector2 vel;        // Vận tốc chuyển động (pixels/giây)
    float groundY;      // Cao độ mặt sàn tiếp đất
    int bounceCount;    // Đếm số lần nảy tiếp đất
    float lifeTimer;    // Thời gian đã tồn tại
    float attractDelay; // Độ trễ trước khi bị người chơi hút về
    int value;          // Lượng vàng của đồng xu này
    bool collected;     // true: đã chạm người chơi và được thu thập
    float rotation;     // Góc quay của sprite đồng xu

    GoldParticle()
        : pos{ 0, 0 }, vel{ 0, 0 }, groundY(0), bounceCount(0),
          lifeTimer(0.0f), attractDelay(0.35f), value(1),
          collected(false), rotation(0.0f) {}

    GoldParticle(float startX, float startY, float targetGroundY, int val = 1)
        : pos{ startX, startY },
          groundY(targetGroundY),
          bounceCount(0),
          lifeTimer(0.0f),
          attractDelay(0.35f + (float)(std::rand() % 25) * 0.01f),
          value(val),
          collected(false),
          rotation(0.0f) {
        // Vận tốc văng ban đầu: góc bay ngẫu nhiên trong khoảng -55 đến -125 độ (hướng lên trên)
        float angle = -60.0f - (float)(std::rand() % 60);
        float speed = 150.0f + (float)(std::rand() % 140);
        float rad = angle * (3.14159265f / 180.0f);
        vel.x = std::cos(rad) * speed;
        vel.y = std::sin(rad) * speed;
    }

    void update(float dt, Vector2 playerCenter) {
        if (collected) return;
        lifeTimer += dt;
        rotation += 400.0f * dt; // Xoay tròn lấp lánh khi bay

        if (lifeTimer < attractDelay) {
            // Giai đoạn 1: Bay parabol chịu trọng lực và nảy trên sàn
            vel.y += 650.0f * dt;
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;

            if (pos.y >= groundY && vel.y > 0.0f) {
                pos.y = groundY;
                if (bounceCount < 2) {
                    bounceCount++;
                    vel.y = -vel.y * 0.42f; // Nảy lên với hệ số đàn hồi
                    vel.x *= 0.65f;
                } else {
                    vel.x = 0.0f;
                    vel.y = 0.0f;
                }
            }
        } else {
            // Giai đoạn 2: Bị hút mạnh mẽ về phía người chơi (Magnet Attract)
            float dx = playerCenter.x - pos.x;
            float dy = playerCenter.y - pos.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < 24.0f) {
                collected = true;
            } else {
                // Tốc độ lướt gia tốc dần
                float flySpeed = 480.0f + (lifeTimer - attractDelay) * 450.0f;
                if (flySpeed > 950.0f) flySpeed = 950.0f;
                pos.x += (dx / dist) * flySpeed * dt;
                pos.y += (dy / dist) * flySpeed * dt;
            }
        }
    }
};

#endif // GOLD_PARTICLE_H

