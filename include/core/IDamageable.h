#ifndef I_DAMAGEABLE_H
#define I_DAMAGEABLE_H

#include "core/IGameObject.h"

/**
 * @brief Giao diện trừu tượng IDamageable (Interface / Abstract Class)
 * Đại diện cho các đối tượng có sinh lực, chịu sát thương và hồi phục.
 * 
 * TIÊU CHÍ OOP CHƯƠNG 5 & 6:
 * - Kế thừa ảo từ IGameObject (`virtual public IGameObject`).
 * - Chứa các phương thức thuần ảo: `takeDamage`, `heal`, `isAlive`.
 */
class IDamageable : virtual public IGameObject {
public:
    IDamageable() : IGameObject("Damageable") {}
    virtual ~IDamageable() override = default;

    // Các phương thức thuần ảo nghiệp vụ sinh lực
    virtual void takeDamage(int amount) = 0;
    virtual void heal(int amount) = 0;
    virtual bool isAlive() const = 0;
};

#endif // I_DAMAGEABLE_H

