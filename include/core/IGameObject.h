#ifndef I_GAME_OBJECT_H
#define I_GAME_OBJECT_H

#include <string>

/**
 * @brief Lớp cơ sở ảo trừu tượng gốc (Virtual Base Class)
 * Đại diện cho mọi thực thể và đối tượng tồn tại trong thế giới game Aethelgard.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 5 (Slide 46-47: Kế thừa kim cương - Diamond Problem):
 * - Khi Entity kế thừa đồng thời từ 2 giao diện IRenderable và IDamageable,
 *   cả 2 giao diện này cùng kế thừa ảo (`virtual public`) từ IGameObject.
 * - Giúp Entity chỉ sở hữu DUY NHẤT 1 bản sao của instanceId và các thuộc tính gốc,
 *   giải quyết triệt để vấn đề nhập nhằng dữ liệu (Ambiguity) trong đa kế thừa C++.
 */
class IGameObject {
private:
    static int nextInstanceId; // Thuộc tính tĩnh đếm số lượng thực thể được tạo ra
protected:
    int instanceId;            // Mã định danh duy nhất của thực thể
    std::string tag;           // Thẻ định danh loại thực thể

public:
    IGameObject() : instanceId(++nextInstanceId), tag("GameObject") {}
    explicit IGameObject(const std::string& tag) : instanceId(++nextInstanceId), tag(tag) {}
    virtual ~IGameObject() = default;

    int getInstanceId() const { return instanceId; }
    const std::string& getTag() const { return tag; }
    void setTag(const std::string& newTag) { tag = newTag; }
};

#endif // I_GAME_OBJECT_H

