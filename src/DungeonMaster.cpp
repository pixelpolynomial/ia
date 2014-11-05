#include "DungeonMaster.h"

#include <vector>

#include "Highscore.h"
#include "TextFormatting.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "CharacterLines.h"
#include "Log.h"
#include "SdlWrapper.h"
#include "Map.h"

using namespace std;

namespace DungeonMaster {

namespace {

int       xpForLvl_[PLAYER_MAX_CLVL + 1];
int       clvl_  = 0;
int       xp_    = 0;
TimeData  timeStarted_;

void playerGainLvl() {
  if(Map::player->isAlive()) {
    clvl_++;

    Log::addMsg("Welcome to level " + toStr(clvl_) + "!", clrGreen,
                false, true);

    CreateCharacter::pickNewTrait(false);

    Map::player->restoreHp(999, false);
    Map::player->changeMaxHp(HP_PER_LVL, true);
    Map::player->changeMaxSpi(SPI_PER_LVL, true);
  }
}

void initXpArray() {
  xpForLvl_[0] = 0;
  xpForLvl_[1] = 0;
  for(int lvl = 2; lvl <= PLAYER_MAX_CLVL; lvl++) {
    xpForLvl_[lvl] = xpForLvl_[lvl - 1] + (100 * lvl);
  }
}

} //namespace

void init() {
  clvl_ = 1;
  xp_   = 0;
  initXpArray();
}

void storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(clvl_));
  lines.push_back(toStr(xp_));
  lines.push_back(toStr(timeStarted_.year_));
  lines.push_back(toStr(timeStarted_.month_));
  lines.push_back(toStr(timeStarted_.day_));
  lines.push_back(toStr(timeStarted_.hour_));
  lines.push_back(toStr(timeStarted_.minute_));
  lines.push_back(toStr(timeStarted_.second_));
}

void setupFromSaveLines(vector<string>& lines) {
  clvl_ = toInt(lines.front());
  lines.erase(begin(lines));
  xp_ = toInt(lines.front());
  lines.erase(begin(lines));
  timeStarted_.year_ = toInt(lines.front());
  lines.erase(begin(lines));
  timeStarted_.month_ = toInt(lines.front());
  lines.erase(begin(lines));
  timeStarted_.day_ = toInt(lines.front());
  lines.erase(begin(lines));
  timeStarted_.hour_ = toInt(lines.front());
  lines.erase(begin(lines));
  timeStarted_.minute_ = toInt(lines.front());
  lines.erase(begin(lines));
  timeStarted_.second_ = toInt(lines.front());
  lines.erase(begin(lines));
}

int getCLvl()             {return clvl_;}
int getXp()               {return xp_;}
TimeData getTimeStarted() {return timeStarted_;}

int getMonTotXpWorth(const ActorDataT& d) {
  //K regulates player XP rate, higher -> more XP per monster
  const double K          = 0.6;
  const double HP         = d.hp;
  const double SPEED      = double(d.speed);
  const double SPEED_MAX  = double(ActorSpeed::END);
  const double SHOCK      = double(d.monShockLvl);
  const double SHOCK_MAX  = double(MonShockLvl::END);

  const double SPEED_FACTOR   = (1.0 + ((SPEED / SPEED_MAX) * 0.50));
  const double SHOCK_FACTOR   = (1.0 + ((SHOCK / SHOCK_MAX) * 0.75));
  const double UNIQUE_FACTOR  = d.isUnique ? 2.0 : 1.0;

  return ceil(K * HP * SPEED_FACTOR * SHOCK_FACTOR * UNIQUE_FACTOR);
}

void playerGainXp(const int XP_GAINED) {
  if(Map::player->isAlive()) {
    for(int i = 0; i < XP_GAINED; ++i) {
      xp_++;
      if(clvl_ < PLAYER_MAX_CLVL) {
        if(xp_ >= xpForLvl_[clvl_ + 1]) {
          playerGainLvl();
        }
      }
    }
  }
}

int getXpToNextLvl() {
  if(clvl_ == PLAYER_MAX_CLVL) {return -1;}
  return xpForLvl_[clvl_ + 1] - xp_;
}

void playerLoseXpPercent(const int PERCENT) {
  xp_ = (xp_ * (100 - PERCENT)) / 100;
}

void winGame() {
  HighScore::onGameOver(true);

  const string winMsg =
    "As I touch the crystal, there is a jolt of electricity. A surreal glow "
    "suddenly illuminates the area. I feel as if I have stirred something. "
    "I notice a dark figure observing me from the edge of the light. It is "
    "the shape of a human. The figure approaches me, but still no light falls "
    "on it as it enters. There is no doubt in my mind concerning the nature "
    "of this entity; It is the Faceless God who dwells in the depths of the "
    "earth, it is the Crawling Chaos - NYARLATHOTEP! I panic. Why is it I "
    "find myself here, stumbling around in darkness? The being beckons me to "
    "gaze into the stone. In the divine radiance I see visions beyond "
    "eternity, visions of unreal reality, visions of the brightest light of "
    "day and the darkest night of madness. The torrent of revelations does "
    "not seem unwelcome - I feel as if under a spell. There is only onward "
    "now. I demand to attain more! So I make a pact with the Fiend......... "
    "I now harness the shadows that stride from world to world to sow death "
    "and madness. The destinies of all things on earth, living and dead, are "
    "mine.";

  vector<string> winMsgLines;
  TextFormatting::lineToLines(winMsg, 68, winMsgLines);

  Render::coverPanel(Panel::screen);
  Render::updateScreen();

  const int Y0 = 2;
  const unsigned int NR_OF_WIN_MESSAGE_LINES = winMsgLines.size();
  const int DELAY_BETWEEN_LINES = 40;
  SdlWrapper::sleep(DELAY_BETWEEN_LINES);
  for(unsigned int i = 0; i < NR_OF_WIN_MESSAGE_LINES; ++i) {
    for(unsigned int ii = 0; ii <= i; ii++) {
      Render::drawTextCentered(winMsgLines.at(ii), Panel::screen,
                               Pos(MAP_W_HALF, Y0 + ii),
                               clrMsgBad, clrBlack, true);
      if(i == ii && ii == NR_OF_WIN_MESSAGE_LINES - 1) {
        const string CMD_LABEL =
          "[space/esc] to record high-score and return to main menu";
        Render::drawTextCentered(
          CMD_LABEL, Panel::screen,
          Pos(MAP_W_HALF, Y0 + NR_OF_WIN_MESSAGE_LINES + 2),
          clrWhite, clrBlack, true);
      }
    }
    Render::updateScreen();
    SdlWrapper::sleep(DELAY_BETWEEN_LINES);
  }

  Query::waitForEscOrSpace();
}

void onMonKilled(Actor& actor) {
  ActorDataT& d = actor.getData();

  d.nrKills += 1;

  if(d.hp >= 3) {
    if(Map::player->obsessions[int(Obsession::sadism)]) {
      Map::player->shock_ = max(0.0, Map::player->shock_ - 3.0);
    }
  }

  const int XP_WORTH_TOT  = getMonTotXpWorth(d);
  Mon* const mon  = static_cast<Mon*>(&actor);
  const int XP_GAINED     = mon->hasGivenXpForSpotting_ ?
                            XP_WORTH_TOT / 2 : XP_WORTH_TOT;
  playerGainXp(XP_GAINED);
}

void onMonSpotted(Actor& actor) {
  Mon* const mon = static_cast<Mon*>(&actor);
  if(!mon->hasGivenXpForSpotting_) {
    mon->hasGivenXpForSpotting_ = true;
    playerGainXp(getMonTotXpWorth(mon->getData()) / 2);
  }
}

void setTimeStartedToNow() {
  timeStarted_ = Utils::getCurTime();
}

} //DungeonMaster
