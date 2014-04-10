#include "AbilityValues.h"

#include <math.h>

#include "Engine.h"
#include "ActorPlayer.h"
#include "PlayerBon.h"
#include "Utils.h"
#include "Properties.h"

int AbilityValues::getVal(const AbilityId ability,
                          const bool IS_AFFECTED_BY_PROPS,
                          Actor& actor) const {
  int val = abilityList[ability];

  if(IS_AFFECTED_BY_PROPS) {
    val += actor.getPropHandler().getAbilityMod(ability);
  }

  if(&actor == eng->player) {
    const int HP_PCT  = (actor.getHp() * 100) / actor.getHpMax(true);

    switch(ability) {
      case ability_searching: {
        val += 8;
        if(PlayerBon::hasTrait(Trait::observant))   val += 4;
        if(PlayerBon::hasTrait(Trait::perceptive))  val += 4;
      } break;

      case ability_accuracyMelee: {
        val += 45;
        if(PlayerBon::hasTrait(Trait::adeptMeleeFighter))   val += 10;
        if(PlayerBon::hasTrait(Trait::expertMeleeFighter))  val += 10;
        if(PlayerBon::hasTrait(Trait::masterMeleeFighter))  val += 10;
        if(PlayerBon::hasTrait(Trait::perseverant) && HP_PCT <= 25) val += 30;
      } break;

      case ability_accuracyRanged: {
        val += 50;
        if(PlayerBon::hasTrait(Trait::adeptMarksman))   val += 10;
        if(PlayerBon::hasTrait(Trait::expertMarksman))  val += 10;
        if(PlayerBon::hasTrait(Trait::masterMarksman))  val += 10;
        if(PlayerBon::hasTrait(Trait::perseverant) && HP_PCT <= 25) val += 30;
      } break;

      case ability_dodgeTrap: {
        val += 5;
        if(PlayerBon::hasTrait(Trait::dexterous)) val += 20;
        if(PlayerBon::hasTrait(Trait::lithe))     val += 20;
      } break;

      case ability_dodgeAttack: {
        val += 10;
        if(PlayerBon::hasTrait(Trait::dexterous)) val += 20;
        if(PlayerBon::hasTrait(Trait::lithe))     val += 20;
        if(PlayerBon::hasTrait(Trait::perseverant) && HP_PCT <= 25) val += 50;
      } break;

      case ability_stealth: {
        val += 10;
        if(PlayerBon::hasTrait(Trait::stealthy))      val += 50;
        if(PlayerBon::hasTrait(Trait::imperceptible)) val += 30;
      } break;

      case ability_empty:
      case endOfAbilityId: {} break;
    }

    if(ability == ability_searching) {
      val = max(val, 1);
    } else if(ability == ability_dodgeAttack) {
      val = min(val, 95);
    }
  }

  val = max(0, val);

  return val;
}

AbilityRollResult AbilityRoll::roll(const int TOTAL_SKILL_VALUE) const {
  const int ROLL = Rnd::percentile();

  const int successCriticalLimit  = int(ceil(float(TOTAL_SKILL_VALUE) / 20.0));
  const int successBigLimit       = int(ceil(float(TOTAL_SKILL_VALUE) / 5.0));
  const int successNormalLimit    = int(ceil(float(TOTAL_SKILL_VALUE) * 4.0 / 5.0));
  const int successSmallLimit     = TOTAL_SKILL_VALUE;
  const int failSmallLimit        = 2 * TOTAL_SKILL_VALUE - successNormalLimit;
  const int failNormalLimit       = 2 * TOTAL_SKILL_VALUE - successBigLimit;
  const int failBigLimit          = 98;

  if(ROLL <= successCriticalLimit)  return successCritical;
  if(ROLL <= successBigLimit)     return successBig;
  if(ROLL <= successNormalLimit)    return successNormal;
  if(ROLL <= successSmallLimit)   return successSmall;
  if(ROLL <= failSmallLimit)      return failSmall;
  if(ROLL <= failNormalLimit)     return failNormal;
  if(ROLL <= failBigLimit)      return failBig;

  return failCritical;

  /* Example:
  -----------
  Ability = 50

  Roll:
  1  -   3: Critical  success
  4  -  10: Big       success
  11 -  40: Normal    success
  41 -  50: Small     Success
  51 -  60: Small     Fail
  61 -  90: Normal    Fail
  91 -  98: Big       Fail
  99 - 100: Critical  Fail */
}

void AbilityValues::reset() {
  for(unsigned int i = 0; i < endOfAbilityId; i++) {
    abilityList[i] = 0;
  }
}

void AbilityValues::setVal(const AbilityId ability, const int VAL) {
  abilityList[ability] = VAL;
}

void AbilityValues::changeVal(const AbilityId ability, const int CHANGE) {
  abilityList[ability] += CHANGE;
}

