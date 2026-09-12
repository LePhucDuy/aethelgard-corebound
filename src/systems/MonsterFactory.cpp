#include "systems/MonsterFactory.h"
#include "entities/BoarKing.h"
#include <cstdlib>

std::unique_ptr<Monster> MonsterFactory::create(MonsterType type, const Position& pos) {
    switch (type) {
        case MonsterType::BOAR:
            return std::make_unique<Boar>(pos);
        case MonsterType::SMALL_BEE:
            return std::make_unique<SmallBee>(pos);
        case MonsterType::SNAIL:
            return std::make_unique<Snail>(pos);
        case MonsterType::BOAR_KING:
            return std::make_unique<BoarKing>(pos);
        default:
            return std::make_unique<Boar>(pos);
    }
}

std::unique_ptr<Monster> MonsterFactory::createRandom(const Position& pos) {
    int roll = std::rand() % 3;
    switch (roll) {
        case 0:  return std::make_unique<Boar>(pos);
        case 1:  return std::make_unique<SmallBee>(pos);
        case 2:  return std::make_unique<Snail>(pos);
        default: return std::make_unique<Boar>(pos);
    }
}
