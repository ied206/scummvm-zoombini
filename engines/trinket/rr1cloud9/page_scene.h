/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRINKET_RR1CLOUD9_PAGE_SCENE_H
#define TRINKET_RR1CLOUD9_PAGE_SCENE_H

#include "trinket/rr1cloud9/page.h"
#include "trinket/rr1cloud9/world.h"
#include "trinket/shell/character.h"
#include "trinket/shell/text_object.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

class SkyRide;
class RainGame;
class Catapult;
class SheepCrossing;
class RockHead;
class RainbowSlide;
class Gumball;
class Sandwich;
class NumberTree;
class WordPots;
class CloudFloors;
class Gallery;
class Flipbook;
class MusicPatterns;

class PageScene : private Shell::Character::Resources {
public:
	PageScene(TrinketEngine &vm, Shell::Scene &scene, World &world);
	~PageScene() { clear(); }
	void clear();
	void checkpoint() const;
	bool enter(const Page &page, const Common::String &name, bool practice);
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	void playSounds(const Common::Array<uint16> &sounds);
	void pause(bool paused, uint32 time);
	Common::String click(Common::Point point);
	bool event(const Common::Event &event);
	void setMousePosition(Common::Point point) { _mouse = point; }
	void updateNavigationCursor();
	void releaseNavigationCursor();
	bool active() const { return !_name.empty(); }
	bool blocksMap() const;
	bool wantsQueenMovie() const { return !_queenArriving && _queenPhase == kQueenMovie; }
	bool wantsQueenExit() const { return !_queenArriving && _queenPhase == kQueenExitReady; }
	bool wantsInventorMovie() const { return _inventorPhase == kInventorMovie; }
	void queenMovieFinished();
	bool wantsGalleryMovie() const;
	int galleryDestination() const;
	Common::String destination() const;

private:
	friend class RainGame;
	RainGame *_rainGame = nullptr;
	friend class SkyRide;
	SkyRide *_skyRide = nullptr;
	friend class Catapult;
	Catapult *_catapult = nullptr;
	friend class SheepCrossing;
	SheepCrossing *_sheepCrossing = nullptr;
	friend class RockHead;
	RockHead *_rockHead = nullptr;
	friend class RainbowSlide;
	RainbowSlide *_rainbowSlide = nullptr;
	friend class MusicPatterns;
	MusicPatterns *_musicPatterns = nullptr;
	friend class Flipbook;
	Flipbook *_flipbook = nullptr;
	friend class Gallery;
	Gallery *_gallery = nullptr;
	friend class CloudFloors;
	CloudFloors *_cloudFloors = nullptr;
	friend class Gumball;
	friend class Sandwich;
	friend class WordPots;
	WordPots *_wordPots = nullptr;
	friend class NumberTree;
	NumberTree *_numberTree = nullptr;
	Gumball *_gumball = nullptr;
	Sandwich *_sandwich = nullptr;
	struct Speech {
		Common::String actor;
		uint resource;
		bool together;
		uint delay;
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	World &_world;
	const Page *_page = nullptr;
	Common::String _name;
	Common::Point _mouse = Common::Point(-1, -1);
	Graphics::Surface _navigationCursors[8];
	int _navigationCursor = -1;
	Common::HashMap<Common::String, Shell::Character *> _actors;
	Common::Array<Speech> _speech;
	Common::ScopedPtr<OM::Sound> _queuedSound;
	Common::Array<OM::Sound *> _sounds;
	Shell::Character *_speaking = nullptr;
	Shell::Character *_speakingAlso = nullptr;
	Shell::Sprite *_bell = nullptr, *_cloud = nullptr;
	Shell::Sprite *_gate = nullptr;
	uint _openingGate = 0;
	uint32 _gateStart = 0;
	bool _gateDelayStarted = false;
	bool _placeBell = false;
	Shell::Sprite *_chester = nullptr, *_chestLid = nullptr, *_chestTray = nullptr, *_dragged = nullptr;
	Shell::Sprite *_chestMotion = nullptr, *_chestButton = nullptr;
	enum ChestPhase {
		kChestClosed,
		kChestCharacter,
		kChestOpening,
		kChestOpen,
		kChestClosing,
	};
	ChestPhase _chestPhase = kChestClosed;
	bool _restoreChester = true;
	uint32 _highlightDue = 0;
	Common::Array<Shell::Sprite *> _items;
	Shell::ResourcePaths _chestPaths;
	Common::SharedPtr<OM::NFNTFont> _chestFont;
	Common::Array<Shell::TextObject *> _itemCounts;
	bool _putAwayPending = false, _putAwayActive = false;
	Common::Point _dragOffset, _dragHome;
	Common::Point _chestOffset, _chestDragPoint;
	bool _draggingChest = false;
	bool _chestOpen = false;
	uint _nextSpeech = 0;
	uint32 _speechDelayStart = 0;
	bool _speechDelayStarted = false;
	uint _arrivalIdleIndex = 0;
	bool _clearCloud = false, _cloudMoving = false;
	uint _cloudSpeechIndex = 0;
	uint32 _cloudStart = 0, _pauseStart = 0;
	Common::Point _cloudOrigin;
	bool _collectBell = false, _practice = false, _paused = false;
	uint32 _time = 0;
	enum QueenPhase {
		kQueenIdle,
		kQueenGreeting,
		kQueenWelcome,
		kQueenMovie,
		kQueenAfterMovie,
		kQueenExitSpeech,
		kQueenExitReady,
	};
	QueenPhase _queenPhase = kQueenIdle;
	uint _queenSongIndex = 0;
	bool _queenSongStarted = false;
	uint _queenClicks = 0, _mainClicks = 0, _paigeClicks = 0;
	bool _queenArriving = false;
	Shell::Sprite *_queenAmbient = nullptr;
	uint _queenAmbientChoice = 0;
	Shell::Sprite *_ambient = nullptr;
	OM::Sound *_ambientSound = nullptr;
	Common::Array<uint> _ambientChoices, _idleChoices;
	uint32 _ambientStart = 0, _idleStart = 0;
	bool _ambientWaiting = false, _idleWaiting = false, _hubStarted = false;
	bool enterHub();
	void updateHub(uint32 time);
	void hubIdle(uint32 time);
	Common::String hubClick(Common::Point point);
	Common::String groveClick(Common::Point point);
	uint chooseAction(Common::Array<uint> &choices, uint count = 3);
	enum InventorPhase {
		kInventorIdle,
		kInventorIntro,
		kInventorChestWait,
		kInventorHoop,
		kInventorBands,
		kInventorPillow,
		kInventorCloseWait,
		kInventorOutro,
		kInventorDeliveryWait,
		kInventorDeliverySpeech,
		kInventorMoreItems,
		kInventorMovie,
	};
	InventorPhase _inventorPhase = kInventorIdle;
	Shell::Sprite *_parts[3] = {}, *_highlight = nullptr;
	uint _irvinClicks = 0, _gearPlays = 0;
	uint32 _inventorStart = 0;
	bool _inventorTimerStarted = false;
	bool enterInventor();
	void inventorSpeech();
	void inventorGroup(const char *prefix, const char *actors);
	void inventorHint(uint item);
	void inventorDrop(uint item);
	void inventorMoreItems();
	void updateInventor(uint32 time);
	void updateInventorAmbient(uint32 time);
	void inventorIdle(uint32 time);
	Common::String inventorClick(Common::Point point);
	static const char *partName(uint item);
	bool chestContains(Common::Point point) const;
	void moveChest(Common::Point delta);
	Common::String characterResource(uint resource, Kind kind, uint part = 1) const override;
	bool actor(const Common::String &name, const Common::String &prefix, bool body = true);
	void say(const Common::String &actor, uint resource);
	void delaySpeech(uint milliseconds);
	void queueSound(uint resource);
	void arrivalSpeaker(uint group = 0);
	void arrivalIdle(uint32 time);
	void speeches(const Common::String &actors, const Common::String &resources, uint count);
	void interrupt();
	void completeSpeech();
	void openChest(bool animateCharacter = false);
	void closeChest(bool restoreCharacter = true);
	void finishChest();
	void updateChest(uint32 time);
	void drawInventoryCounts(OM::Canvas &canvas);
	void drawPage(OM::Canvas &canvas);
	void putAway() { _putAwayPending = true; }
	bool sceneBusy() const;
	bool chestBusy() const { return _chestPhase == kChestCharacter || _chestPhase == kChestOpening || _chestPhase == kChestClosing; }
	Common::String gateClick(Common::Point point);
	bool enterQueen();
	void updateQueenAmbient(uint32 time);
	void queenGroup(uint group, const char *actors);
	void queenReentry(int state, int reentry, bool fromCatapult);
	void queenWelcome();
	void updateQueen();
	void queenIdle(uint32 time);
	Common::String queenClick(Common::Point point);
	void music(uint resource);
	bool hasItem(const Common::String &name, const Common::String &delivered = Common::String()) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
