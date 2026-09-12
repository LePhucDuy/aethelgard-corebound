#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "core/Position.h"
#include "graphics/Animation.h"

// Forward declaration của Dungeon để tránh phụ thuộc vòng tròn (Circular Dependency)
class Dungeon;
class Player;

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
 * 2. Tính đóng gói (Encapsulation):
 *    - Các thuộc tính cốt lõi (hp, atk, def, pos, anim) được bảo vệ ở mức `protected` hoặc `private`.
 *    - Truy xuất và tương tác qua các phương thức nghiệp vụ: `takeDamage()`, `heal()`, `isAlive()`.
 * 
 * 3. Quan hệ hợp thành (Composition):
 *    - Entity sở hữu trực tiếp `std::unique_ptr<Animation>`. Vòng đời của Animation gắn chặt
 *      với Entity; khi Entity bị hủy, Animation tự động được giải phóng khỏi bộ nhớ.
 * 
 * 4. Nạp chồng toán tử (Operator Overloading):
 *    - `operator<<`: Xuất trạng thái, tên, HP và tọa độ của thực thể ra stream.
 * 
 * 5. Virtual Destructor:
 *    - Bắt buộc phải có `virtual ~Entity()` để đảm bảo khi hủy đối tượng con thông qua con trỏ
 *      lớp cha (`std::unique_ptr<Entity>`), destructor của lớp con được gọi đúng cách (tránh rò rỉ bộ nhớ).
 */
class Entity {
protected:
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
    virtual ~Entity() = default;

    // Phương thức thuần ảo (Pure Virtual Function) -> Biến Entity thành Abstract Class
    // Mỗi lượt: Entity nhận sân khấu (Dungeon), đối thủ (Player) và nhật ký chiến đấu
    virtual void act(Dungeon& dungeon, Player& player, std::vector<std::string>& combatLog) = 0;

    // Cập nhật frame và vẽ thực thể lên màn hình
    virtual void update(float deltaTime);
    virtual void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const;

    // Các phương thức nghiệp vụ xử lý trạng thái
    virtual void takeDamage(int amount);
    virtual void heal(int amount);

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
    bool isAlive() const { return alive; }

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
