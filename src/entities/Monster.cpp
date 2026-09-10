#include "entities/Monster.h"

Monster::Monster(const std::string& name, const Position& pos, int hp, int attack, int defense, 
                 int expReward, int goldReward)
    : Entity(name, pos, hp, attack, defense),
      expReward(expReward), goldReward(goldReward) {}
