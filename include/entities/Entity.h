#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "core/Position.h"
#include "graphics/Animation.h"
#include "core/IRenderable.h"
#include "core/IDamageable.h"

// Forward declaration của Dungeon để tránh phụ thuộc vòng tròn (Circular Dependency)
class Dungeon;
class Player;
class CombatSystem;
class SaveLoadManager;

/**
 * @brief Lớp trừu tượng Entity (Abstract Base Class 1)
 * Đại diện cho mọi sinh vật sống tồn tại trên bản đồ (Người chơi, Quái vật, NPC).
 * 
 * ĐẶC ĐIỂM OOP:
 * 1. Tính trừu tượng (Abstraction):
 *    - Chứa phương thức thuần ảo: `virtual void act(Dungeon& dungeon) = 0;`
 *    - Không thể khởi tạo trực tiếp đối tượng Entity; buộc các lớp con phải tự định nghĩa
 *      hành vi lượt đi của riêng mình (Player nhận phím bấm, Monster dùng thuật toán AI).
 * 
 * 2. Tính đóng gói (Encapsulation) & Lớp bạn (Friend Class - Slide 60-61 Chương 3):
 *    - Các thuộc tính cốt lõi (hp, atk, def, pos, anim) được bảo vệ ở mức `protected`.
 *    - Cho phép `friend class CombatSystem;` và `friend class SaveLoadManager;` truy cập
 *      trực tiếp các chỉ số nội bộ phục vụ tính toán sát thương và lưu trữ file an toàn.
 * 
 * 3. Đa kế thừa & Kế thừa kim cương (Diamond Problem - Slide 46-47 Chương 5):
 *    - Entity kế thừa đồng thời từ 2 giao diện: `IRenderable` và `IDamageable`.
 *    - Cả 2 giao diện đều kế thừa ảo (`virtual public IGameObject`), giúp Entity chỉ có
 *      duy nhất 1 phiên bản của `instanceId`, giải quyết dứt điểm xung đột lưỡng nghĩa.
 * 
 * 4. Virtual Destructor:
 *    - Bắt buộc phải có `virtual ~Entity()` để đảm bảo khi hủy đối tượng con thông qua con trỏ
 *      lớp cha (`std::unique_ptr<Entity>`), destructor của lớp con được gọi đúng cách (tránh rò rỉ bộ nhớ).
 */
class Entity : public IRenderable, public IDamageable {
protected:
    // Khai báo Lớp bạn (Friend Class - Slide 60-61 Chương 3)
    friend class CombatSystem;
    friend class SaveLoadManager;

    std::string name;
    Position pos;
    Vector2 visualPos;          // Tọa độ hiển thị mượt mà (world pixel)
    float moveLerpSpeed = 20.0f; // Tốc độ trượt nội suy giữa các ô
    int hp;
    int maxHp;
    int attack;
    int defense;
    bool alive;

    // Con trỏ không sở hữu (non-owning) trỏ đến animation đang hoạt động
    // Animation thực sự được sở hữu bởi std::map trong lớp con (Monster, Player)
    Animation* currentAnim = nullptr;

public:
    Entity(const std::string& name, const Position& pos, int hp, int attack, int defense);
    virtual ~Entity() override = default;

    // Phương thức thuần ảo (Pure Virtual Function) -> Biến Entity thành Abstract Class
    // Mỗi lượt: Entity nhận sân khấu (Dungeon), đối thủ (Player) và nhật ký chiến đấu
    virtual void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) = 0;

    // Cập nhật frame và vẽ thực thể lên màn hình (Override từ IRenderable)
    virtual void update(float deltaTime);
    void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const override;

    // Các phương thức nghiệp vụ xử lý trạng thái (Override từ IDamageable)
    void takeDamage(int amount) override;
    void heal(int amount) override;
    bool isAlive() const override { return alive; }

    // Getters & Setters
    const std::string& getName() const { return name; }
    const Position& getPosition() const { return pos; }
    const Vector2& getVisualPosition() const { return visualPos; }
    void resetVisualPosition();
    void setPosition(const Position& newPos, bool snapVisual = false) { 
        pos = newPos; 
        if (snapVisual) resetVisualPosition();
    }

    int getHp() const { return hp; }
    int getMaxHp() const { return maxHp; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }

    float getMoveLerpSpeed() const { return moveLerpSpeed; }
    void setMoveLerpSpeed(float speed) { moveLerpSpeed = speed; }

    // Quản lý animation
    void setAnimation(std::unique_ptr<Animation> anim);
    Animation* getAnimation() { return currentAnim; }
    const Animation* getAnimation() const { return currentAnim; }

    // Nạp chồng toán tử in thông tin thực thể
    friend std::ostream& operator<<(std::ostream& os, const Entity& entity) {
        os << "[" << entity.name << "] HP: " << entity.hp << "/" << entity.maxHp 
           << " | ATK: " << entity.attack << " | DEF: " << entity.defense 
           << " | Pos: " << entity.pos;
        return os;
    }
};

#endif // ENTITY_H
