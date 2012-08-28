#include "ItemDrop.h"

#include "Engine.h"
#include "ConstDungeonSettings.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"

#include <algorithm>

void ItemDrop::dropAllCharactersItems(Actor* actor, bool died) {
	(void)died;
	actor->getInventory()->dropAllNonIntrinsic(actor->pos, true, eng);
}

void ItemDrop::dropItemFromInventory(Actor* actorDropping, const int ELEMENT) {
	Inventory* inventory = actorDropping->getInventory();
	Item* item = inventory->getItemInElement(ELEMENT);

	if(item != NULL) {
		const string itemRef = eng->itemData->itemInterfaceName(item, true);

		inventory->removeItemInElementWithoutDeletingInstance(ELEMENT);

		eng->itemDrop->dropItemOnMap(actorDropping->pos, &item);

		//Messages
		const Actor* const curActor = eng->gameTime->getCurrentActor();
		if(curActor == eng->player) {
			eng->log->clearLog();
			eng->log->addMessage("I drop " + itemRef + ".");
		} else {
			bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
			eng->mapTests->makeVisionBlockerArray(blockers);
			if(eng->player->checkIfSeeActor(*curActor, blockers)) {
				eng->log->addMessage("I see " + curActor->getNameThe() + " throw " + itemRef + ".");
			}
		}

		//End turn
		eng->gameTime->letNextAct();

	}
}

void ItemDrop::dropItemOnMap(const coord pos, Item** item) {
	//If target cell is bottomless, just destroy the item
	if(eng->map->featuresStatic[pos.x][pos.y]->isBottomless()) {
		delete *item;
		return;
	}

	//Make a vector of all cells on map with no blocking feature
	bool freeCellArray[MAP_X_CELLS][MAP_Y_CELLS];
	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			freeCellArray[x][y] = eng->map->featuresStatic[x][y]->canHaveItem();
		}
	}
	vector<coord> freeCells;
	eng->mapTests->makeMapVectorFromArray(freeCellArray, freeCells);

	//Sort the vector according to distance to origin
	IsCloserToOrigin isCloserToOrigin(pos, eng);
	sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);

	int curX, curY, stackX, stackY;
	const bool ITEM_STACKS = (*item)->getInstanceDefinition().isStackable;
	int ii = 0;
	Item* stackItem;
	const unsigned int vectorSize = freeCells.size();
	for(unsigned int i = 0; i < vectorSize; i++) {
		//First look in all cells that has distance to origin equal to cell i
		//to try and merge the item if it stacks
		if(ITEM_STACKS) {
			//While ii cell is not further away than i cell
			while(isCloserToOrigin(freeCells.at(i), freeCells.at(ii)) == false) {
				stackX = freeCells.at(ii).x;
				stackY = freeCells.at(ii).y;
				stackItem = eng->map->items[stackX][stackY];
				if(stackItem != NULL) {
					if(stackItem->getInstanceDefinition().devName == (*item)->getInstanceDefinition().devName) {
						stackItem->numberOfItems += (*item)->numberOfItems;
						delete(*item);
						*item = NULL;
						i = 999999;
						break;
					}
				}
				ii++;
			}
		} else {
			(*item)->appplyDropEffects();
		}

		if(*item == NULL) {
			break;
		}

		curX = freeCells.at(i).x;
		curY = freeCells.at(i).y;
		if(eng->map->items[curX][curY] == NULL) {
			eng->map->items[curX][curY] = *item;
			if(eng->player->pos == coord(curX, curY)) {
				if(curX != pos.x || curY != pos.y) {
					eng->log->addMessage("I feel something by my feet.");
				}
			}

			i = 999999;
		}

		if(i == vectorSize - 1) {
			delete *item;
			*item = NULL;
		}
	}
}

