#include "ItemData.h"

#include <iostream>
#include <climits>

#include "Colors.h"
#include "Item.h"
#include "CmnData.h"
#include "ActorData.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ActorPlayer.h"
#include "Sound.h"
#include "ItemDevice.h"
#include "Map.h"

using namespace std;

ItemDataT::ItemDataT(const ItemId id_) :
  id(id_),
  itemValue(ItemValue::normal),
  itemWeight(itemWeight_none),
  spawnStdRange(Range(1, INT_MAX)),
  maxStackAtSpawn(1),
  chanceToIncludeInSpawnList(100),
  isStackable(true),
  isIdentified(true),
  isTried(false),
  baseName(),
  glyph('X'),
  clr(clrWhite),
  tile(TileId::empty),
  mainAttMode(MainAttMode::none),
  isExplosive(false),
  isScroll(false),
  isPotion(false),
  isDevice(false),
  isArmor(false),
  isHeadwear(false),
  isIntrinsic(false),
  isAmmo(false),
  isAmmoClip(false),
  isMedicalBag(false),
  spellCastFromScroll(SpellId::END),
  landOnHardSurfaceSoundMsg("I hear a thudding sound."),
  landOnHardSurfaceSfx(),
  melee(MeleeItemData()),
  ranged(RangedItemData()),
  armor(ArmorItemData()) {

  baseDescr.resize(0);
  nativeRooms.resize(0);
  featuresCanBeFoundIn.resize(0);
}

ItemDataT::MeleeItemData::MeleeItemData() :
  isMeleeWpn(false),
  dmg(pair<int, int>(0, 0)),
  hitChanceMod(0),
  attMsgs(ItemAttMsgs()),
  propApplied(nullptr),
  dmgType(DmgType::physical),
  isKnockback(false),
  hitSmallSfx(SfxId::END),
  hitMediumSfx(SfxId::END),
  hitHardSfx(SfxId::END),
  missSfx(SfxId::END) {}

ItemDataT::MeleeItemData::~MeleeItemData() {
  if(propApplied) {delete propApplied;}
}

ItemDataT::RangedItemData::RangedItemData() :
  isRangedWpn(false),
  isThrowingWpn(false),
  isMachineGun(false),
  isShotgun(false),
  ammoContainedInClip(0),
  dmg(DiceParam()),
  throwDmg(DiceParam()),
  hitChanceMod(0),
  throwHitChanceMod(0),
  isKnockback(false),
  dmgInfoOverride(""),
  ammoItemId(ItemId::empty),
  dmgType(DmgType::physical),
  hasInfiniteAmmo(false),
  missileGlyph('/'),
  missileTile(TileId::projectileStdFrontSlash),
  missileClr(clrWhite),
  missileLeavesTrail(false),
  missileLeavesSmoke(false),
  attMsgs(ItemAttMsgs()),
  sndMsg(""),
  sndVol(SndVol::low),
  makesRicochetSnd(false),
  attSfx(SfxId::END),
  reloadSfx(SfxId::END),
  propApplied(nullptr) {}

ItemDataT::RangedItemData::~RangedItemData() {
  if(propApplied) {delete propApplied;}
}

ItemDataT::ArmorItemData::ArmorItemData() :
  absorptionPoints(0),
  dmgToDurabilityFactor(0.0) {}

namespace ItemData {

ItemDataT* data[int(ItemId::END)];

namespace {

void addFeatureFoundIn(ItemDataT& data, const FeatureId featureId,
                       const int CHANCE_TO_INCL = 100) {
  data.featuresCanBeFoundIn.push_back(pair<FeatureId, int>(featureId, CHANCE_TO_INCL));
}

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void resetData(ItemDataT& d, ItemType const itemType) {
  switch(itemType) {
    case ItemType::general: {
      d = ItemDataT(d.id);
    } break;

    case ItemType::meleeWpn: {
      resetData(d, ItemType::general);
      d.isStackable = false;
      d.itemWeight = itemWeight_medium;
      d.glyph = '(';
      d.clr = clrWhite;
      d.mainAttMode = MainAttMode::melee;
      d.melee.isMeleeWpn = true;
      d.melee.hitSmallSfx = SfxId::hitSmall;
      d.melee.hitMediumSfx = SfxId::hitMedium;
      d.melee.hitHardSfx = SfxId::hitHard;
    } break;

    case ItemType::meleeWpnIntr: {
      resetData(d, ItemType::meleeWpn);
      d.isIntrinsic = true;
      d.spawnStdRange = Range(-1, -1);
      d.melee.hitSmallSfx = SfxId::hitSmall;
      d.melee.hitMediumSfx = SfxId::hitMedium;
      d.melee.hitHardSfx = SfxId::hitHard;
      d.melee.missSfx = SfxId::missMedium;
    } break;

    case ItemType::rangedWpn: {
      resetData(d, ItemType::general);
      d.isStackable = false;
      d.itemWeight = itemWeight_medium;
      d.glyph = '}';
      d.clr = clrWhite;
      d.melee.isMeleeWpn = true;
      d.melee.dmg = pair<int, int>(1, 6);
      d.mainAttMode = MainAttMode::ranged;
      d.ranged.isRangedWpn = true;
      d.ranged.missileGlyph = '/';
      d.ranged.missileClr = clrWhite;
      d.spawnStdRange.upper = FIRST_CAVERN_LVL - 1;
      d.melee.hitSmallSfx = SfxId::hitSmall;
      d.melee.hitMediumSfx = SfxId::hitMedium;
      d.melee.hitHardSfx = SfxId::hitHard;
      d.melee.missSfx = SfxId::missMedium;
      d.ranged.sndVol = SndVol::high;
    } break;

    case ItemType::rangedWpnIntr: {
      resetData(d, ItemType::rangedWpn);
      d.isIntrinsic = true;
      d.ranged.hasInfiniteAmmo = true;
      d.spawnStdRange = Range(-1, -1);
      d.melee.isMeleeWpn = false;
      d.ranged.missileGlyph = '*';
      d.ranged.sndVol = SndVol::low;
    } break;

    case ItemType::throwingWpn: {
      resetData(d, ItemType::general);
      d.itemWeight = itemWeight_extraLight;
      d.isStackable = true;
      d.ranged.isThrowingWpn = true;
      d.spawnStdRange.upper = FIRST_CAVERN_LVL - 1;
      d.ranged.sndVol = SndVol::low;
    } break;

    case ItemType::ammo: {
      resetData(d, ItemType::general);
      d.itemWeight = itemWeight_extraLight;
      d.glyph = '{';
      d.clr = clrWhite;
      d.tile = TileId::ammo;
      d.isAmmo = true;
      d.spawnStdRange.upper = FIRST_CAVERN_LVL - 1;
    } break;

    case ItemType::ammoClip: {
      resetData(d, ItemType::ammo);
      d.itemWeight = itemWeight_light;
      d.isStackable = false;
      d.isAmmoClip = true;
      d.spawnStdRange.upper = FIRST_CAVERN_LVL - 1;
    } break;

    case ItemType::scroll: {
      resetData(d, ItemType::general);
      d.itemValue = ItemValue::minorTreasure;
      d.chanceToIncludeInSpawnList = 40;
      d.itemWeight = itemWeight_none;
      d.isIdentified = false;
      d.glyph = '?';
      d.clr = clrWhite;
      d.tile = TileId::scroll;
      d.isScroll = true;
      d.maxStackAtSpawn = 1;
      d.landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, FeatureId::chest);
      addFeatureFoundIn(d, FeatureId::tomb);
      addFeatureFoundIn(d, FeatureId::cabinet, 25);
      addFeatureFoundIn(d, FeatureId::cocoon, 25);
      ScrollNameHandling::setFalseScrollName(d);
    } break;

    case ItemType::potion: {
      resetData(d, ItemType::general);
      d.itemValue = ItemValue::minorTreasure;
      d.chanceToIncludeInSpawnList = 55;
      d.itemWeight = itemWeight_light;
      d.isIdentified = false;
      d.glyph = '!';
      d.tile = TileId::potion;
      d.isPotion = true;
      d.ranged.isThrowingWpn = true;
      d.ranged.throwHitChanceMod = 15;
      d.ranged.throwDmg = DiceParam(1, 3, 0);
      d.maxStackAtSpawn = 2;
      d.landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, FeatureId::chest);
      addFeatureFoundIn(d, FeatureId::tomb);
      addFeatureFoundIn(d, FeatureId::cabinet, 25);
      addFeatureFoundIn(d, FeatureId::cocoon, 25);
      PotionNameHandling::setClrAndFalseName(d);
    } break;

    case ItemType::device: {
      resetData(d, ItemType::general);
      d.isDevice = true;
      d.chanceToIncludeInSpawnList = 12;
      d.itemWeight = itemWeight_light;
      d.isIdentified = true;
      d.glyph = '~';
      d.tile = TileId::device1;
      d.isStackable = false;
      d.landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
      d.landOnHardSurfaceSfx = SfxId::metalClank;
    } break;

    case ItemType::armor: {
      resetData(d, ItemType::general);
      d.itemWeight = itemWeight_heavy;
      d.glyph = '[';
      d.tile = TileId::armor;
      d.isArmor = true;
      d.isStackable = false;
    } break;

    case ItemType::explosive: {
      resetData(d, ItemType::general);
      d.itemWeight = itemWeight_light;
      d.isExplosive = true;
      d.glyph = '-';
      d.maxStackAtSpawn = 2;
      d.landOnHardSurfaceSoundMsg = "";
    } break;

    default: {} break;
  }
}

void setDmgFromMonsterData(ItemDataT& itemData, const ActorDataT& actorData)  {
  itemData.melee.dmg        = pair<int, int>(1, actorData.dmgMelee);
  itemData.ranged.dmg       = DiceParam(1, actorData.dmgRanged, 0);
  itemData.ranged.throwDmg  = DiceParam(1, actorData.dmgRanged, 0);
}

//------------------------------- LIST OF ITEMS
void initDataList() {
  ItemDataT* d = nullptr;

  d = new ItemDataT(ItemId::trapezohedron);
  resetData(*d, ItemType::general);
  d->baseName = ItemName("Shining Trapezohedron", "Shining Trapezohedrons",
                         "The Shining Trapezohedron");
  d->spawnStdRange = Range(-1, -1);
  d->isStackable = false;
  d->glyph = '*';
  d->clr = clrRedLgt;
  d->tile = TileId::trapezohedron;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::sawedOff);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Sawed-off Shotgun", "Sawed-off shotguns",
                         "a Sawed-off Shotgun");
  d->baseDescr = {"Compared to a standard shotgun, the sawed-off has a shorter "
                  "effective range. At close range it is more devastating however. It "
                  "holds two barrels, and needs to be reloaded after both are discharged"
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::shotgun;
  d->ranged.isShotgun = true;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a shotgun");
  d->ranged.dmg = DiceParam(8, 3);
  d->ranged.ammoItemId = ItemId::shotgunShell;
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a shotgun");
  d->ranged.sndMsg = "I hear a shotgun blast.";
  d->ranged.attSfx = SfxId::shotgunSawedOffFire;
  d->ranged.makesRicochetSnd = true;
  d->ranged.reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pumpShotgun);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Pump Shotgun", "Pump shotguns", "a Pump Shotgun");
  d->baseDescr = {"A pump-action shotgun has a handgrip that can be pumped back and "
                  "forth in order to eject a spent round of ammunition and to chamber a "
                  "fresh one. It has a single barrel above a tube magazine into which "
                  "shells are inserted. The magazine has a capacity of 8 shells."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::shotgun;
  d->ranged.isShotgun = true;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a shotgun");
  d->ranged.dmg = DiceParam(6, 3);
  d->ranged.ammoItemId = ItemId::shotgunShell;
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a shotgun");
  d->ranged.sndMsg = "I hear a shotgun blast.";
  d->ranged.attSfx = SfxId::shotgunPumpFire ;
  d->ranged.makesRicochetSnd = true;
  d->ranged.reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::shotgunShell);
  resetData(*d, ItemType::ammo);
  d->baseName = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
  d->baseDescr = {"A cartridge designed to be fired from a shotgun"};
  d->maxStackAtSpawn = 10;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::incinerator);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Incinerator", "Incinerators", "an Incinerator");
  d->baseDescr = {"This hellish, experimental weapon launches an explosive fireball. "
                  "Best used with extreme caution."
                 };
  d->itemWeight = itemWeight_heavy;
  d->tile = TileId::incinerator;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with an Incinerator");
  d->ranged.dmg = DiceParam(1, 3);
  d->ranged.dmgInfoOverride = "*";
  d->ranged.ammoItemId = ItemId::incineratorCartridge;
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires an incinerator");
  d->ranged.sndMsg = "I hear the blast of a launched missile.";
  d->ranged.missileGlyph = '*';
  d->ranged.missileClr = clrRedLgt;
  d->spawnStdRange.lower = 5;
  addFeatureFoundIn(*d, FeatureId::chest, 25);
  addFeatureFoundIn(*d, FeatureId::cabinet, 25);
  addFeatureFoundIn(*d, FeatureId::cocoon, 25);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::incineratorCartridge);
  resetData(*d, ItemType::ammoClip);
  d->baseName = ItemName("Incinerator Cartridge", "Incinerator Cartridges",
                         "an Incinerator Cartridge");
  d->baseDescr = {"Ammunition designed for Incinerators."};
  d->itemWeight = itemWeight_light;
  d->ranged.ammoContainedInClip = 3;
  d->spawnStdRange.lower = 5;
  d->maxStackAtSpawn = 1;
  addFeatureFoundIn(*d, FeatureId::chest, 25);
  addFeatureFoundIn(*d, FeatureId::cabinet, 25);
  addFeatureFoundIn(*d, FeatureId::cocoon, 25);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::machineGun);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Tommy Gun", "Tommy Guns", "a Tommy Gun");
  d->baseDescr = {"\"Tommy Gun\" is a nickname for the Thompson submachine gun - an "
                  "automatic firearm with a drum magazine and verical foregrip. It fires "
                  ".45 ACP ammunition. The drum magazine has a capacity of 50 rounds."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::tommyGun;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Tommy Gun");
  d->ranged.isMachineGun = true;
  d->ranged.dmg = DiceParam(2, 2, 2);
  d->ranged.hitChanceMod = -10;
  d->ranged.ammoItemId = ItemId::drumOfBullets;
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a Tommy Gun");
  d->ranged.sndMsg = "I hear the burst of a machine gun.";
  d->ranged.attSfx = SfxId::machineGunFire;
  d->ranged.makesRicochetSnd = true;
  d->ranged.reloadSfx = SfxId::machineGunReload;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::drumOfBullets);
  resetData(*d, ItemType::ammoClip);
  d->baseName = ItemName("Drum of .45 ACP", "Drums of .45 ACP",
                         "a Drum of .45 ACP");
  d->baseDescr = {"Ammunition used by Tommy Guns."};
  d->ranged.ammoContainedInClip = 50;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pistol);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("M1911 Colt", "M1911 Colt", "an M1911 Colt");
  d->baseDescr = {"A semi-automatic, magazine-fed pistol chambered for the .45 ACP "
                  "cartridge."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::pistol;
  d->ranged.dmg = DiceParam(1, 8, 4);
  d->ranged.ammoItemId = ItemId::pistolClip;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a pistol");
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a pistol");
  d->ranged.sndMsg = "I hear a pistol being fired.";
  d->ranged.attSfx = SfxId::pistolFire;
  d->ranged.makesRicochetSnd = true;
  d->ranged.reloadSfx = SfxId::pistolReload;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pistolClip);
  resetData(*d, ItemType::ammoClip);
  d->baseName = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges",
                         "a .45ACP Colt cartridge");
  d->baseDescr = {"Ammunition used by Colt pistols."};
  d->ranged.ammoContainedInClip = 7;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::flareGun);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Flare Gun", "Flare Gun", "a Flare Gun");
  d->baseDescr = {"Launches flares. Not designed to function as a weapon."};
  d->itemWeight = itemWeight_light;
  d->tile = TileId::flareGun;
  d->ranged.dmg = DiceParam(1, 3, 0);
  d->ranged.dmgInfoOverride = "*";
  d->ranged.ammoItemId = ItemId::flare;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a flare gun");
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a flare gun");
  d->ranged.sndMsg = "I hear a flare gun being fired.";
  d->ranged.propApplied = new PropFlared(PropTurns::std);
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::teslaCannon);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Tesla Cannon", "Tesla Cannons", "a Tesla Cannon");
  d->baseDescr = {"A strange weapon seemingly based on technology pioneered by "
                  "Nikola Tesla. It fires electric bolts."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::teslaCannon;
  d->clr = clrYellow;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Tesla Cannon");
  d->ranged.isMachineGun = true;
  d->ranged.hitChanceMod = -15;
  d->ranged.dmg = DiceParam(2, 3, 3);
  d->ranged.dmgType = DmgType::electric;
  d->ranged.ammoItemId = ItemId::teslaCanister;
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a Tesla Cannon");
  d->ranged.sndMsg = "I hear loud electric crackle.";
  d->ranged.missileGlyph = '*';
  d->ranged.missileClr = clrYellow;
  d->spawnStdRange.lower = 7;
  addFeatureFoundIn(*d, FeatureId::chest, 50);
  addFeatureFoundIn(*d, FeatureId::cabinet, 50);
  addFeatureFoundIn(*d, FeatureId::cocoon, 50);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::teslaCanister);
  resetData(*d, ItemType::ammoClip);
  d->baseName = ItemName("Nuclear battery", "Nuclear batteries",
                         "a Nuclear battery");
  d->baseDescr = {"Ammunition used by Tesla Cannons."};
  d->clr = clrYellow;
  d->ranged.ammoContainedInClip = 30;
  d->spawnStdRange.lower = 6;
  addFeatureFoundIn(*d, FeatureId::chest, 50);
  addFeatureFoundIn(*d, FeatureId::cabinet, 50);
  addFeatureFoundIn(*d, FeatureId::cocoon, 50);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::spikeGun);
  resetData(*d, ItemType::rangedWpn);
  d->baseName = ItemName("Spike Gun", "Spike Guns", "a Spike Gun");
  d->baseDescr = {"A very strange and crude weapon capable of launching iron spikes "
                  "with enough force to pierce flesh (or even rock). It seems almost to "
                  "be deliberately designed for cruelty, rather than pure stopping "
                  "power."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::tommyGun;
  d->clr = clrBlueLgt;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Spike Gun");
  d->ranged.isMachineGun = false;
  d->ranged.hitChanceMod = 0;
  d->ranged.dmg = DiceParam(1, 7, 0);
  d->ranged.dmgType = DmgType::physical;
  d->ranged.isKnockback = true;
  d->ranged.ammoItemId = ItemId::ironSpike;
  d->ranged.attMsgs = ItemAttMsgs("fire", "fires a Spike Gun");
  d->ranged.sndMsg = "I hear a very crude weapon being fired.";
  d->ranged.makesRicochetSnd = true;
  d->ranged.missileGlyph = '/';
  d->ranged.missileClr = clrGray;
  d->spawnStdRange.lower = 4;
  d->ranged.sndVol = SndVol::low;
  addFeatureFoundIn(*d, FeatureId::chest, 50);
  addFeatureFoundIn(*d, FeatureId::cabinet, 50);
  addFeatureFoundIn(*d, FeatureId::cocoon, 50);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::dynamite);
  resetData(*d, ItemType::explosive);
  d->baseName = ItemName("Dynamite", "Sticks of Dynamite", "a Stick of Dynamite");
  d->baseDescr = {"An explosive material based on nitroglycerin. The name comes from the "
                  "ancient Greek word dynamis, meaning \"power\"."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::dynamite;
  d->clr = clrRedLgt;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::flare);
  resetData(*d, ItemType::explosive);
  d->baseName = ItemName("Flare", "Flares", "a Flare");
  d->baseDescr = {"A type of pyrotechnic that produces a brilliant light or intense "
                  "heat without an explosion."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::flare;
  d->clr = clrGray;
  d->isAmmo = true;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::molotov);
  resetData(*d, ItemType::explosive);
  d->baseName = ItemName("Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail");
  d->baseDescr = {"An improvised incendiary weapon made of a glass bottle containing "
                  "flammable liquid and some cloth for ignition. In action, the cloth "
                  "is lit and the bottle hurled at a target, causing an immediate "
                  "fireball followed by a raging fire."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::molotov;
  d->clr = clrWhite;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::smokeGrenade);
  resetData(*d, ItemType::explosive);
  d->baseName = ItemName("Smoke Grenade", "Smoke Grenades", "a Smoke Grenade");
  d->baseDescr = {"A sheet steel cylinder with emission holes releasing smoke when the "
                  "grenade is ignited. Their primary use is to create smoke screens for "
                  "concealment. The fumes produced can harm the eyes, throat and lungs "
                  "- so it is recommended to wear a protective mask."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::flare;
  d->clr = clrGreen;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::throwingKnife);
  resetData(*d, ItemType::throwingWpn);
  d->baseName = ItemName("Throwing Knife", "Throwing Knives", "a Throwing Knife");
  d->baseDescr = {"A knife specially designed and weighted so that it can be thrown "
                  "effectively."
                 };
  d->itemWeight = itemWeight_extraLight;
  d->tile = TileId::dagger;
  d->glyph = '/';
  d->clr = clrWhite;
  d->ranged.throwHitChanceMod = 0;
  d->ranged.throwDmg = DiceParam(2, 4);
  d->maxStackAtSpawn = 8;
  d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
  d->landOnHardSurfaceSfx = SfxId::metalClank;
  d->mainAttMode = MainAttMode::thrown;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::rock);
  resetData(*d, ItemType::throwingWpn);
  d->baseName = ItemName("Rock", "Rocks", "a Rock");
  d->baseDescr = {"Although not a very impressive weapon, with skill they can be used "
                  "with some result. Shooting them with a sling instead of throwing "
                  "will increase the range and cause more damage."
                 };
  d->itemWeight = itemWeight_extraLight;
  d->tile = TileId::rock;
  d->glyph = '*';
  d->clr = clrGray;
  d->ranged.throwHitChanceMod = 10;
  d->ranged.throwDmg = DiceParam(1, 3);
  d->maxStackAtSpawn = 6;
  d->mainAttMode = MainAttMode::thrown;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::dagger);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Dagger", "Daggers", "a Dagger");
  d->baseDescr = {"Commonly associated with deception, stealth, and treachery. Many "
                  "assassinations have been carried out with the use of a dagger.",

                  "Attacking an unaware opponent with a dagger does 3x damage (instead "
                  "of the normal 1,5x damage from stealth attacks).",

                  "Melee attacks with daggers are silent."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::dagger;
  d->melee.attMsgs = ItemAttMsgs("stab", "stabs me with a Dagger");
  d->melee.dmg = pair<int, int>(1, 4);
  d->melee.hitChanceMod = 20;
  d->melee.hitMediumSfx = SfxId::hitSharp;
  d->melee.hitHardSfx = SfxId::hitSharp;
  d->melee.missSfx = SfxId::missLight;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::tomb);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::hatchet);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Hatchet", "Hatchets", "a Hatchet");
  d->baseDescr = {"A small axe with a short handle. Hatchets are reliable weapons - "
                  "they are easy to use, and cause decent damage for their low weight. "
                  "They can also serve well as thrown weapons.",

                  "Melee attacks with hatchets are silent."
                 };
  d->itemWeight = itemWeight_light;
  d->tile = TileId::axe;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Hatchet");
  d->melee.dmg = pair<int, int>(1, 5);
  d->melee.hitChanceMod = 15;
  d->ranged.throwHitChanceMod = -5;
  d->ranged.throwDmg = DiceParam(1, 10);
  d->ranged.isThrowingWpn = false;
  d->melee.hitMediumSfx = SfxId::hitSharp;
  d->melee.hitHardSfx = SfxId::hitSharp;
  d->melee.missSfx = SfxId::missLight;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::club);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Club", "Clubs", "a Club");
  d->baseDescr = {"Wielded since prehistoric times.",

                  "Melee attacks with clubs are noisy."
                 };
  d->spawnStdRange = Range(FIRST_CAVERN_LVL, INT_MAX);
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::club;
  d->clr = clrBrown;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Club");
  d->melee.dmg = pair<int, int>(2, 3);
  d->melee.hitChanceMod = 10;
  d->melee.missSfx = SfxId::missMedium;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::hammer);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Hammer", "Hammers", "a Hammer");
  d->baseDescr = {"Typically used for construction, but can be quite devastating when "
                  "wielded as a weapon.",

                  "Melee attacks with hammers are noisy."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::hammer;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Hammer");
  d->melee.dmg = pair<int, int>(2, 4);
  d->melee.hitChanceMod = 5;
  d->melee.missSfx = SfxId::missMedium;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::machete);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Machete", "Machetes", "a Machete");
  d->baseDescr = {"A large cleaver-like knife. It serves well both as a cutting tool "
                  "and weapon.",

                  "Melee attacks with machetes are noisy."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::machete;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Machete");
  d->melee.dmg = pair<int, int>(2, 5);
  d->melee.hitChanceMod = 0;
  d->melee.hitSmallSfx = SfxId::hitSharp;
  d->melee.hitMediumSfx = SfxId::hitSharp;
  d->melee.missSfx = SfxId::missMedium;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::axe);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Axe", "Axes", "an Axe");
  d->baseDescr = {"A tool intended for felling trees, splitting timber, etc. Used as a "
                  "weapon it can deliver devastating blows, although it requires some "
                  "skill to use effectively. Also effective for breaching wooden doors.",

                  "Melee attacks with axes are noisy."
                 };
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::axe;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with an axe");
  d->melee.dmg = pair<int, int>(2, 6);
  d->melee.hitChanceMod = -5;
  d->melee.hitSmallSfx = SfxId::hitSmall;
  d->melee.hitMediumSfx = SfxId::hitMedium;
  d->melee.hitHardSfx = SfxId::hitHard;
  d->melee.missSfx = SfxId::missMedium;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::tomb);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pitchFork);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Pitchfork", "Pitchforks", "a Pitchfork");
  d->itemWeight = itemWeight_heavy;
  d->tile = TileId::pitchfork;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Pitchfork");
  d->melee.dmg = pair<int, int>(3, 4);
  d->melee.hitChanceMod = -5;
  d->melee.isKnockback = true;
  d->melee.hitSmallSfx = SfxId::hitSharp;
  d->melee.hitMediumSfx = SfxId::hitSharp;
  d->melee.missSfx = SfxId::missHeavy;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::sledgeHammer);
  resetData(*d, ItemType::meleeWpn);
  d->baseName = ItemName("Sledgehammer", "Sledgehammers", "a Sledgehammer");
  d->itemWeight = itemWeight_heavy;
  d->tile = TileId::sledgeHammer;
  d->melee.attMsgs = ItemAttMsgs("strike", "strikes me with a Sledgehammer");
  d->melee.dmg = pair<int, int>(3, 5);
  d->melee.hitChanceMod = -10;
  d->melee.isKnockback = true;
  d->melee.missSfx = SfxId::missHeavy;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ironSpike);
  resetData(*d, ItemType::throwingWpn);
  d->baseName = ItemName("Iron Spike", "Iron Spikes", "an Iron Spike");
  d->isAmmo = true;
  d->itemWeight = itemWeight_extraLight;
  d->tile = TileId::ironSpike;
  d->isStackable = true;
  d->clr = clrGray;
  d->glyph = '/';
  d->ranged.throwHitChanceMod = -5;
  d->ranged.throwDmg = DiceParam(1, 3);
  d->maxStackAtSpawn = 12;
  d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
  d->landOnHardSurfaceSfx = SfxId::metalClank;
  d->mainAttMode = MainAttMode::thrown;
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::playerKick);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("kick", "");
  d->melee.hitChanceMod = 20;
  d->melee.dmg = pair<int, int>(1, 3);
  d->melee.isKnockback = true;
  d->melee.missSfx = SfxId::missMedium;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::playerStomp);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("stomp", "");
  d->melee.hitChanceMod = 20;
  d->melee.dmg = pair<int, int>(1, 3);
  d->melee.isKnockback = false;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::playerPunch);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("punch", "");
  d->melee.hitChanceMod = 25;
  d->melee.dmg = pair<int, int>(1, 2);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zombieClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zombie]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zombieClawDiseased);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zombie]);
  d->melee.propApplied = new PropInfected(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zombieAxe);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "chops me with a rusty axe");
  d->melee.hitSmallSfx = SfxId::hitSmall;
  d->melee.hitMediumSfx = SfxId::hitMedium;
  d->melee.hitHardSfx = SfxId::hitHard;
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zombieAxe]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::bloatedZombiePunch);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "mauls me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::bloatedZombie]);
  d->melee.isKnockback = true;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::bloatedZombieSpit);
  resetData(*d, ItemType::rangedWpnIntr);
  d->ranged.attMsgs = ItemAttMsgs("", "spits acid pus at me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::bloatedZombie]);
  d->ranged.sndMsg = "I hear spitting.";
  d->ranged.missileClr = clrGreenLgt;
  d->ranged.dmgType = DmgType::acid;
  d->ranged.missileGlyph = '*';
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ratBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::rat]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ratBiteDiseased);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::rat]);
  d->melee.propApplied = new PropInfected(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ratThingBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::ratThing]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::wormMassBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::wormMass]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::wolfBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::wolf]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::greenSpiderBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::greenSpider]);
  d->melee.propApplied = new PropBlind(PropTurns::specific, 4);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::whiteSpiderBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::whiteSpider]);
  d->melee.propApplied = new PropParalyzed(PropTurns::specific, 2);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::redSpiderBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::redSpider]);
  d->melee.propApplied = new PropWeakened(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::shadowSpiderBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::shadowSpider]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::lengSpiderBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(
    *d, ActorData::data[ActorId::lengSpider]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::fireHoundBreath);
  resetData(*d, ItemType::rangedWpnIntr);
  d->ranged.attMsgs = ItemAttMsgs("", "breaths fire at me");
  d->ranged.sndMsg = "I hear a burst of flames.";
  setDmgFromMonsterData(*d, ActorData::data[ActorId::fireHound]);
  d->ranged.propApplied = new PropBurning(PropTurns::std);
  d->ranged.missileClr = clrRedLgt;
  d->ranged.missileGlyph = '*';
  d->ranged.missileLeavesTrail = true;
  d->ranged.missileLeavesSmoke = true;
  d->ranged.dmgType = DmgType::fire;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::fireHoundBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::fireHound]);
  d->melee.dmgType = DmgType::fire;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::frostHoundBreath);
  resetData(*d, ItemType::rangedWpnIntr);
  d->ranged.attMsgs = ItemAttMsgs("", "breaths frost at me");
  d->ranged.sndMsg = "I hear a chilling sound.";
  setDmgFromMonsterData(*d, ActorData::data[ActorId::frostHound]);
  d->ranged.missileClr = clrBlueLgt;
  d->ranged.missileGlyph = '*';
  d->ranged.missileLeavesTrail = true;
  d->ranged.missileLeavesSmoke = true;
  d->ranged.dmgType = DmgType::cold;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::frostHoundBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::frostHound]);
  d->melee.dmgType = DmgType::cold;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zuulBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zuul]);
  d->melee.dmgType = DmgType::physical;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::dustVortexEngulf);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::dustVortex]);
  d->melee.propApplied = new PropBlind(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::fireVortexEngulf);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::fireVortex]);
  d->melee.propApplied = new PropBurning(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::frostVortexEngulf);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::frostVortex]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ghostClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::ghost]);
  d->melee.propApplied = new PropTerrified(PropTurns::specific, 4);
  d->melee.dmgType = DmgType::spirit;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::phantasmSickle);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "slices me with a sickle");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::phantasm]);
  d->melee.propApplied = new PropTerrified(PropTurns::specific, 4);
  d->melee.dmgType = DmgType::spirit;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::wraithClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::wraith]);
  d->melee.propApplied = new PropTerrified(PropTurns::specific, 4);
  d->melee.dmgType = DmgType::spirit;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::giantBatBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::giantBat]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::miGoElectricGun);
  resetData(*d, ItemType::rangedWpnIntr);
  d->ranged.missileLeavesTrail = true;
  d->ranged.missileClr = clrYellow;
  d->ranged.missileGlyph = '/';
  d->ranged.attMsgs = ItemAttMsgs("", "fires an electric gun");
  d->ranged.dmgType = DmgType::electric;
  d->ranged.propApplied = new PropParalyzed(PropTurns::specific, 2);
  d->ranged.sndMsg = "I hear a bolt of electricity.";
  setDmgFromMonsterData(*d, ActorData::data[ActorId::miGo]);
  d->ranged.sndVol = SndVol::high;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::polypTentacle);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "grips me with a tentacle");
  d->melee.propApplied = new PropParalyzed(PropTurns::specific, 1);
  setDmgFromMonsterData(*d, ActorData::data[ActorId::flyingPolyp]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ghoulClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::ghoul]);
  d->melee.propApplied = new PropInfected(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::shadowClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::shadow]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::byakheeClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::byakhee]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::giantMantisClaw);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::giantMantis]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::giantLocustBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::locust]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::mummyMaul);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "mauls me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::mummy]);
  d->melee.propApplied = new PropCursed(PropTurns::std);
  d->melee.isKnockback = true;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deepOneJavelinAtt);
  resetData(*d, ItemType::rangedWpnIntr);
  d->ranged.attMsgs = ItemAttMsgs("", "throws a Javelin at me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::deepOne]);
  d->ranged.sndMsg = "";
  d->ranged.missileClr = clrBrown;
  d->ranged.missileGlyph = '/';
  d->ranged.sndVol = SndVol::low;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deepOneSpearAtt);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "thrusts a spear at me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::deepOne]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozeBlackSpewPus);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "spews pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozeBlack]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozeClearSpewPus);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "spews pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozeClear]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozePutridSpewPus);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "spews infected pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozePutrid]);
  d->melee.propApplied = new PropInfected(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozePoisonSpewPus);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "spews poisonous pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozePoison]);
  d->melee.propApplied = new PropPoisoned(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::colourOOSpaceTouch);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "touches me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::colourOOSpace]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::chthonianBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "strikes me with a tentacle");
  d->melee.isKnockback = true;
  setDmgFromMonsterData(*d, ActorData::data[ActorId::chthonian]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::huntingHorrorBite);
  resetData(*d, ItemType::meleeWpnIntr);
  d->melee.attMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::huntingHorror]);
  d->melee.propApplied = new PropParalyzed(PropTurns::std);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorLeatherJacket);
  resetData(*d, ItemType::armor);
  d->baseName = ItemName("Leather Jacket", "", "a Leather Jacket");
  d->itemWeight = itemWeight_light;
  d->clr = clrBrown;
  d->spawnStdRange.lower = 1;
  d->armor.absorptionPoints = 1;
  d->armor.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(*d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorIronSuit);
  resetData(*d, ItemType::armor);
  d->baseName = ItemName("Iron Suit", "", "an Iron Suit");
  d->itemWeight = itemWeight_heavy;
  d->clr = clrWhite;
  d->spawnStdRange.lower = 2;
  d->armor.absorptionPoints = 4;
  d->armor.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a crashing sound.";
  addFeatureFoundIn(*d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorFlackJacket);
  resetData(*d, ItemType::armor);
  d->baseName = ItemName("Flak Jacket", "", "a Flak Jacket");
  d->itemWeight = itemWeight_medium;
  d->clr = clrGreen;
  d->spawnStdRange.lower = 3;
  d->armor.absorptionPoints = 3;
  d->armor.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a thudding sound.";
  addFeatureFoundIn(*d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorAsbSuit);
  resetData(*d, ItemType::armor);
  d->baseName = ItemName("Asbestos Suit", "", "an Asbestos Suit");
  d->itemWeight = itemWeight_medium;
  d->clr = clrRedLgt;
  d->spawnStdRange.lower = 3;
  d->armor.absorptionPoints = 1;
  d->armor.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::chest);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorHeavyCoat);
  resetData(*d, ItemType::armor);
  d->baseName = ItemName("Heavy Coat", "", "a Heavy Coat");
  d->itemWeight = itemWeight_medium;
  d->clr = clrBlueLgt;
  d->spawnStdRange.lower = 3;
  d->armor.absorptionPoints = 1;
  d->armor.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(*d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::gasMask);
  resetData(*d, ItemType::general);
  d->baseName = ItemName("Gas Mask", "", "A Gas Mask");
  d->isStackable = false;
  d->clr = clrBrown;
  d->tile = TileId::mask;
  d->glyph = '[';
  d->isHeadwear = true;
  d->spawnStdRange = Range(1, LAST_ROOM_AND_CORRIDOR_LVL);
  d->chanceToIncludeInSpawnList = 50;
  d->itemWeight = itemWeight_light;
  d->landOnHardSurfaceSoundMsg = "";
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::hideousMask);
  resetData(*d, ItemType::general);
  d->baseName = ItemName("Hideous Mask", "", "The Hideous Mask");
  d->isStackable = false;
  d->clr = clrMagenta;
  d->tile = TileId::mask;
  d->glyph = '[';
  d->isHeadwear = true;
  d->spawnStdRange = Range(-1, -1);
  d->chanceToIncludeInSpawnList = 0;
  d->itemWeight = itemWeight_light;
  d->landOnHardSurfaceSoundMsg = "";
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollMayhem);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::mayhem;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollTelep);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::teleport;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollPestilence);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::pestilence;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollSlowMon);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::slowEnemies;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollTerrifyMon);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::terrifyEnemies;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollParalMon);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::paralyzeEnemies;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDetItems);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detItems;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDetTraps);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detTraps;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollBless);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::bless;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDarkbolt);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::darkbolt;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollAzaWrath);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::azathothsWrath;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollOpening);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::opening;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollSacrLife);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::sacrLife;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollSacrSpi);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::sacrSpi;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDetMon);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detMon;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollElemRes);
  resetData(*d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::elemRes;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionVitality);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionSpirit);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionBlindness);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionFrenzy);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionFortitude);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionParalyze);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionRElec);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionConf);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionPoison);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionInsight);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionClairv);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionRFire);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionAntidote);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionDescent);
  resetData(*d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceSentry);
  resetData(*d, ItemType::device);
  d->baseName = ItemName("Sentry Device", "Sentry Devices", "a Sentry Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(*d, FeatureId::chest, 10);
  addFeatureFoundIn(*d, FeatureId::tomb, 10);
  addFeatureFoundIn(*d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceRepeller);
  resetData(*d, ItemType::device);
  d->baseName = ItemName("Repeller Device", "Repeller Devices",
                         "a Repeller Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(*d, FeatureId::chest, 10);
  addFeatureFoundIn(*d, FeatureId::tomb, 10);
  addFeatureFoundIn(*d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceRejuvenator);
  resetData(*d, ItemType::device);
  d->baseName = ItemName("Rejuvenator Device", "Rejuvenator Devices",
                         "a Rejuvenator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(*d, FeatureId::chest, 10);
  addFeatureFoundIn(*d, FeatureId::tomb, 10);
  addFeatureFoundIn(*d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceTranslocator);
  resetData(*d, ItemType::device);
  d->baseName = ItemName("Translocator Device", "Translocator Devices",
                         "a Translocator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(*d, FeatureId::chest, 10);
  addFeatureFoundIn(*d, FeatureId::tomb, 10);
  addFeatureFoundIn(*d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::electricLantern);
  resetData(*d, ItemType::device);
  d->baseName = ItemName("Electric Lantern", "Electric Lanterns",
                         "an Electric Lantern");
  d->spawnStdRange = Range(1, 10);
  d->chanceToIncludeInSpawnList = 50;
  d->isIdentified = true;
  d->tile = TileId::electricLantern;
  d->clr = clrYellow;
  addFeatureFoundIn(*d, FeatureId::chest);
  addFeatureFoundIn(*d, FeatureId::cabinet);
  addFeatureFoundIn(*d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::medicalBag);
  resetData(*d, ItemType::general);
  d->isMedicalBag = true;
  d->baseName = ItemName("Medical Bag", "Medical Bags", "a Medical Bag");
  d->itemValue = ItemValue::normal;
  d->itemWeight = itemWeight_medium;
  d->spawnStdRange = Range(1, LAST_ROOM_AND_CORRIDOR_LVL);
  d->isStackable = false;
  d->glyph = '~';
  d->clr = clrBrownDrk;
  d->tile = TileId::medicalBag;
  data[int(d->id)] = d;
}

} //namespace

void init() {
  initDataList();
}

void cleanup() {
  for(size_t i = 1; i < int(ItemId::END); ++i) delete data[i];
}


void storeToSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::END); ++i) {
    lines.push_back(data[i]->isIdentified ? "1" : "0");

    if(data[i]->isScroll) {
      lines.push_back(data[i]->isTried ? "1" : "0");
    }
  }
}

void setupFromSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::END); ++i) {
    data[i]->isIdentified = lines.front() == "0" ? false : true;
    lines.erase(begin(lines));

    if(data[i]->isScroll) {
      data[i]->isTried = lines.front() == "0" ? false : true;
      lines.erase(begin(lines));
    }
  }
}

//TODO Remove this function
bool isWpnStronger(const ItemDataT& data1, const ItemDataT& data2,
                   const bool IS_MELEE) {
  (void)data1;
  (void)data2;
  (void)IS_MELEE;

  return false;
}

} //ItemData
