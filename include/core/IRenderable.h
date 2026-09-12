#ifndef I_RENDERABLE_H
#define I_RENDERABLE_H

#include "core/IGameObject.h"
#include <raylib.h>

/**
 * @brief Giao diện trừu tượng IRenderable (Interface / Abstract Class)
 * Đại diện cho các đối tượng có khả năng kết xuất đồ họa lên màn hình.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 5 & 6:
 * - Kế thừa ảo từ IGameObject (`virtual public IGameObject`).
 * - Chứa phương thức thuần ảo: `virtual void render(...) const = 0;`
 */
class IRenderable : virtual public IGameObject {
public:
    IRenderable() : IGameObject("Renderable") {}
    virtual ~IRenderable() override = default;

    // Phương thức thuần ảo kết xuất hình ảnh
    virtual void render(float scale = 2.0f, Vector2 offset = {0.0f, 0.0f}) const = 0;
};

#endif // I_RENDERABLE_H

