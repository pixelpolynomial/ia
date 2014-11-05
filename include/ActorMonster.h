#ifndef MONSTER_H
#define MONSTER_H

#include "CmnData.h"

#include "Actor.h"
#include "Sound.h"
#include "Spells.h"

struct BestAttack {
  BestAttack() : weapon(nullptr), isMelee(true) {}

  Wpn* weapon;
  bool isMelee;
};

struct AttackOpport {
  AttackOpport() :
    isTimeToReload(false), isMelee(true) {
    weapons.clear();
  }

  AttackOpport(const AttackOpport& other) :
    weapons(other.weapons), isTimeToReload(other.isTimeToReload),
    isMelee(other.isMelee) {}

  AttackOpport& operator=(const AttackOpport& other) {
    weapons = other.weapons;
    isTimeToReload = other.isTimeToReload;
    isMelee = other.isMelee;

    return *this;
  }

  std::vector<Wpn*> weapons;
  bool isTimeToReload;
  bool isMelee;
};

class Wpn;

class Mon: public Actor {
public:
  Mon();
  virtual ~Mon();

  virtual void place_() override {}

  void moveDir(Dir dir);

  AttackOpport getAttackOpport(Actor& defender);
  BestAttack getBestAttack(const AttackOpport& attackOpport);
  bool tryAttack(Actor& defender);

  virtual void mkStartItems() override = 0;

  void hearSound(const Snd& snd);

  void becomeAware(const bool IS_FROM_SEEING);

  void playerBecomeAwareOfMe(const int DURATION_FACTOR = 1);

  void onActorTurn() override;
  virtual bool onActorTurn_() {return false;}
  virtual void onStdTurn() override {}

  virtual std::string getAggroPhraseMonSeen() const {
    return data_->aggroTextMonSeen;
  }
  virtual std::string getAggroPhraseMonHidden() const {
    return data_->aggroTextMonHidden;
  }
  virtual SfxId getAggroSfxMonSeen() const {
    return data_->aggroSfxMonSeen;
  }
  virtual SfxId getAggroSfxMonHidden() const {
    return data_->aggroSfxMonHidden;
  }

  void speakPhrase();

  bool isLeaderOf(const Actor& actor)       const override;
  bool isActorMyLeader(const Actor& actor)  const override;

  int                 awareCounter_, playerAwareOfMeCounter_;
  bool                isMsgMonInViewPrinted;
  Dir                 lastDirTravelled_;
  std::vector<Spell*> spellsKnown;
  int                 spellCoolDownCur;
  bool                isRoamingAllowed_;
  bool                isStealth;
  Actor*              leader;
  Actor*              target;
  bool                waiting_;
  double              shockCausedCur_;
  bool                hasGivenXpForSpotting_;

protected:
  virtual void hit_(int& dmg) override;
};

class Rat: public Mon {
public:
  Rat() : Mon() {}
  ~Rat() {}
  virtual void mkStartItems() override;
};

class RatThing: public Rat {
public:
  RatThing() : Rat() {}
  ~RatThing() {}
  void mkStartItems() override;
};

class BrownJenkin: public RatThing {
public:
  BrownJenkin() : RatThing() {}
  ~BrownJenkin() {}
};

class Spider: public Mon {
public:
  Spider() : Mon() {}
  virtual ~Spider() {}
  bool onActorTurn_() override;
};

class GreenSpider: public Spider {
public:
  GreenSpider() : Spider() {}
  ~GreenSpider() {}
  void mkStartItems() override;
};

class WhiteSpider: public Spider {
public:
  WhiteSpider() : Spider() {}
  ~WhiteSpider() {}
  void mkStartItems() override;
};

class RedSpider: public Spider {
public:
  RedSpider() : Spider() {}
  ~RedSpider() {}
  void mkStartItems() override;
};

class ShadowSpider: public Spider {
public:
  ShadowSpider() : Spider() {}
  ~ShadowSpider() {}
  void mkStartItems() override;
};

class LengSpider: public Spider {
public:
  LengSpider() : Spider() {}
  ~LengSpider() {}
  void mkStartItems() override;
};

class Zombie: public Mon {
public:
  Zombie() : Mon() {
    deadTurnCounter = 0;
    hasResurrected = false;
  }
  virtual ~Zombie() {}
  virtual bool onActorTurn_() override;
  void die_() override;
protected:
  bool tryResurrect();
  int deadTurnCounter;
  bool hasResurrected;
};

class ZombieClaw: public Zombie {
public:
  ZombieClaw() : Zombie() {}
  ~ZombieClaw() {}
  void mkStartItems() override;
};

class ZombieAxe: public Zombie {
public:
  ZombieAxe() : Zombie() {}
  ~ZombieAxe() {}
  void mkStartItems() override;
};

class BloatedZombie: public Zombie {
public:
  BloatedZombie() : Zombie() {}
  ~BloatedZombie() {}

  void mkStartItems() override;
};

class MajorClaphamLee: public ZombieClaw {
public:
  MajorClaphamLee() :
    ZombieClaw(), hasSummonedTombLegions(false) {
  }
  ~MajorClaphamLee() {}

  bool onActorTurn_() override;
private:
  bool hasSummonedTombLegions;
};

class DeanHalsey: public ZombieClaw {
public:
  DeanHalsey() : ZombieClaw() {}
  ~DeanHalsey() {}
};

class KeziahMason: public Mon {
public:
  KeziahMason() : Mon(), hasSummonedJenkin(false) {}
  ~KeziahMason() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
private:
  bool hasSummonedJenkin;
};

class LengElder: public Mon {
public:
  LengElder() : Mon() {}
  ~LengElder() {}
  void onStdTurn() override;
  void mkStartItems()   override;
private:
  bool  hasGivenItemToPlayer_;
  int   nrTurnsToHostile_;
};

class Cultist: public Mon {
public:
  Cultist() : Mon() {}

  virtual void mkStartItems() override;

  static std::string getCultistPhrase();

  std::string getAggroPhraseMonSeen() const {
    return getNameThe() + ": " + getCultistPhrase();
  }
  std::string getAggroPhraseMonHidden() const {
    return "Voice: " + getCultistPhrase();
  }

  virtual ~Cultist() {}
};

class CultistTeslaCannon: public Cultist {
public:
  CultistTeslaCannon() : Cultist() {}
  ~CultistTeslaCannon() {}
  void mkStartItems() override;
};

class CultistSpikeGun: public Cultist {
public:
  CultistSpikeGun() : Cultist() {}
  ~CultistSpikeGun() {}
  void mkStartItems() override;
};

class CultistPriest: public Cultist {
public:
  CultistPriest() : Cultist() {}
  ~CultistPriest() {}
  void mkStartItems() override;
};

class LordOfShadows: public Mon {
public:
  LordOfShadows() : Mon() {}
  ~LordOfShadows() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class LordOfSpiders: public Mon {
public:
  LordOfSpiders() : Mon() {}
  ~LordOfSpiders() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class LordOfSpirits: public Mon {
public:
  LordOfSpirits() : Mon() {}
  ~LordOfSpirits() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class LordOfPestilence: public Mon {
public:
  LordOfPestilence() : Mon() {}
  ~LordOfPestilence() {}
  bool onActorTurn_() override;
  void mkStartItems() override;
};

class FireHound: public Mon {
public:
  FireHound() : Mon() {}
  ~FireHound() {}
  void mkStartItems() override;
};

class FrostHound: public Mon {
public:
  FrostHound() : Mon() {}
  ~FrostHound() {}
  void mkStartItems() override;
};

class Zuul: public Mon {
public:
  Zuul() : Mon() {}
  ~Zuul() {}

  void place_() override;

  void mkStartItems() override;
};

class Ghost: public Mon {
public:
  Ghost() : Mon() {}
  ~Ghost() {}
  bool onActorTurn_() override;
  virtual void mkStartItems() override;
};

class Phantasm: public Ghost {
public:
  Phantasm() : Ghost() {}
  ~Phantasm() {}
  void mkStartItems() override;
};

class Wraith: public Ghost {
public:
  Wraith() : Ghost() {}
  ~Wraith() {}
  void mkStartItems() override;
};

class GiantBat: public Mon {
public:
  GiantBat() : Mon() {}
  ~GiantBat() {}
  void mkStartItems() override;
};

class Byakhee: public GiantBat {
public:
  Byakhee() : GiantBat() {}
  ~Byakhee() {}
  void mkStartItems() override;
};

class GiantMantis: public Mon {
public:
  GiantMantis() : Mon() {}
  ~GiantMantis() {}
  void mkStartItems() override;
};

class Chthonian: public Mon {
public:
  Chthonian() : Mon() {}
  ~Chthonian() {}
  void mkStartItems() override;
};

class HuntingHorror: public GiantBat {
public:
  HuntingHorror() : GiantBat() {}
  ~HuntingHorror() {}
  void mkStartItems() override;
};

class Wolf: public Mon {
public:
  Wolf() : Mon() {}
  ~Wolf() {}
  void mkStartItems() override;
};

class MiGo: public Mon {
public:
  MiGo() : Mon() {}
  ~MiGo() {}
  void mkStartItems() override;
};

class FlyingPolyp: public Mon {
public:
  FlyingPolyp() : Mon() {}
  ~FlyingPolyp() {}
  void mkStartItems() override;
};

class Ghoul: public Mon {
public:
  Ghoul() : Mon() {}
  ~Ghoul() {}
  virtual void mkStartItems() override;
};

class DeepOne: public Mon {
public:
  DeepOne() : Mon() {}
  ~DeepOne() {}
  void mkStartItems() override;
};

class Mummy: public Mon {
public:
  Mummy() : Mon() {}
  ~Mummy() {}
  virtual void mkStartItems() override;
};

class MummyUnique: public Mummy {
public:
  MummyUnique() : Mummy() {}
  ~MummyUnique() {}
  void mkStartItems() override;
};

class Khephren: public MummyUnique {
public:
  Khephren() : MummyUnique() {}
  ~Khephren() {}

  bool onActorTurn_() override;
private:
  bool hasSummonedLocusts;
};

class Shadow: public Mon {
public:
  Shadow() : Mon() {}
  ~Shadow() {}

  virtual void mkStartItems() override;
};

class WormMass: public Mon {
public:
  WormMass() : Mon(), chanceToSpawnNew(12) {}
  ~WormMass() {}
  bool onActorTurn_() override;
  virtual void mkStartItems() override;
private:
  int chanceToSpawnNew;
};

class GiantLocust: public Mon {
public:
  GiantLocust() : Mon(), chanceToSpawnNew(5) {}
  ~GiantLocust() {}
  bool onActorTurn_() override;
  virtual void mkStartItems() override;
private:
  int chanceToSpawnNew;
};

class Vortex: public Mon {
public:
  Vortex() : Mon(), pullCooldown(0) {}
  virtual ~Vortex() {}

  bool onActorTurn_() override;

  virtual void mkStartItems() = 0;
  virtual void die_() = 0;
private:
  int pullCooldown;
};

class DustVortex: public Vortex {
public:
  DustVortex() : Vortex() {}
  ~DustVortex() {}
  void mkStartItems() override;
  void die_();
};

class FireVortex: public Vortex {
public:
  FireVortex() : Vortex() {}
  ~FireVortex() {}
  void mkStartItems() override;
  void die_();
};

class FrostVortex: public Vortex {
public:
  FrostVortex() : Vortex() {}
  ~FrostVortex() {}
  void mkStartItems() override;
  void die_();
};

class Ooze: public Mon {
public:
  Ooze() : Mon() {}
  ~Ooze() {}
  virtual void onStdTurn() override;
  virtual void mkStartItems() = 0;
};

class OozeBlack: public Ooze {
public:
  OozeBlack() : Ooze() {}
  ~OozeBlack() {}
  void mkStartItems() override;
};

class OozeClear: public Ooze {
public:
  OozeClear() : Ooze() {}
  ~OozeClear() {}
  void mkStartItems() override;
};

class OozePutrid: public Ooze {
public:
  OozePutrid() : Ooze() {}
  ~OozePutrid() {}
  void mkStartItems() override;
};

class OozePoison: public Ooze {
public:
  OozePoison() : Ooze() {}
  ~OozePoison() {}
  void mkStartItems() override;
};

class ColourOOSpace: public Ooze {
public:
  ColourOOSpace() : Ooze(),
    curColor(clrMagentaLgt) {}
  ~ColourOOSpace() {}
//  bool onActorTurn_() override;
  void onStdTurn() override;
  void mkStartItems() override;
  const Clr& getClr();
private:
  Clr curColor;
};

#endif
