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

#include "trinket/rr1cloud9/page_scene.h"

#include "graphics/cursorman.h"
#include "trinket/rr1cloud9/ca1_cloud_floors.h"
#include "trinket/rr1cloud9/ca2_flipbook.h"
#include "trinket/rr1cloud9/cg2_sheep_crossing.h"
#include "trinket/rr1cloud9/cn3_gallery.h"
#include "trinket/rr1cloud9/cn4_rainbow_slide.h"
#include "trinket/rr1cloud9/ia1_number_tree.h"
#include "trinket/rr1cloud9/ia2_sandwich.h"
#include "trinket/rr1cloud9/ia3_word_pots.h"
#include "trinket/rr1cloud9/ig1_rock_head.h"
#include "trinket/rr1cloud9/la1_gumball.h"
#include "trinket/rr1cloud9/la2_music_patterns.h"
#include "trinket/rr1cloud9/la3_rain_game.h"
#include "trinket/rr1cloud9/lg2_catapult.h"
#include "trinket/rr1cloud9/rss_sky_ride.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *PageScene::kQueenName;
constexpr const char *PageScene::kQueenKey;
constexpr const char *PageScene::kR1CN2StateKey;
constexpr const char *PageScene::kTalkedToInventorKey;
constexpr const char *PageScene::kRBandsName;
constexpr const char *PageScene::kHulaHoopName;
constexpr const char *PageScene::kPillowName;
constexpr const char *PageScene::kINV1Key;
constexpr const char *PageScene::kItemSignedIndexKey;
constexpr const char *PageScene::kR1LN2Key;
constexpr const char *PageScene::kDeliveredKey;
constexpr const char *PageScene::kR1CN2ReentryKey;
constexpr const char *PageScene::kPreviousLocationKey;
constexpr const char *PageScene::kR1CN2CatapultReentryKey;
constexpr const char *PageScene::kR1CN3FromStreetKey;
constexpr const char *PageScene::kLitterIDKey;
constexpr const char *PageScene::kLitterZKey;
constexpr const char *PageScene::kSignIDKey;
constexpr const char *PageScene::kSignZKey;
constexpr const char *PageScene::kAmbientIDs1Key;
constexpr const char *PageScene::kAmbientZKey;
constexpr const char *PageScene::kReaderName;
constexpr const char *PageScene::kReaderWalkInIDKey;
constexpr const char *PageScene::kSamName;
constexpr const char *PageScene::kSamWalkInIDKey;
constexpr const char *PageScene::kR1CN2WelcomePendingKey;
constexpr const char *PageScene::kR1CN2ExitPendingKey;
constexpr const char *PageScene::kByeIDKey;
constexpr const char *PageScene::kEnterSpchIDsKey;
constexpr const char *PageScene::kDelayTimeKey;
constexpr const char *PageScene::kAmbientIDsUnsignedIndexKey;
constexpr const char *PageScene::kReEnterSpchIDsUnsignedIndexUnsignedIndexKey;
constexpr const char *PageScene::kRsrName;
constexpr const char *PageScene::kReEnterSpchIDsSignedIndexKey;
constexpr const char *PageScene::kQrqName;
constexpr const char *PageScene::kQsrsrsrsrsqName;
constexpr const char *PageScene::kQrName;
constexpr const char *PageScene::kQrsName;
constexpr const char *PageScene::kR1LN2HulaHoopDeliveredKey;
constexpr const char *PageScene::kR1LN2RBandsDeliveredKey;
constexpr const char *PageScene::kR1LN2PillowDeliveredKey;
constexpr const char *PageScene::kReEnterSpchIDs111Key;
constexpr const char *PageScene::kReEnterSpchIDs11UnsignedIndexKey;
constexpr const char *PageScene::kReEnterSpchIDs115Key;
constexpr const char *PageScene::kReEnterSpchIDs12UnsignedIndexKey;
constexpr const char *PageScene::kReEnterSpchIDs124Key;
constexpr const char *PageScene::kQqsrqName;
constexpr const char *PageScene::kQqrName;
constexpr const char *PageScene::kWelcomeCharID1Key;
constexpr const char *PageScene::kWelcomeSpchID1Key;
constexpr const char *PageScene::kNumWelcomeSpch1Key;
constexpr const char *PageScene::kWelcomeCharID2Key;
constexpr const char *PageScene::kWelcomeSpchID2Key;
constexpr const char *PageScene::kNumWelcomeSpch2Key;
constexpr const char *PageScene::kSongIntroIDKey;
constexpr const char *PageScene::kGIdleHelpTimeName;
constexpr const char *PageScene::kIdleSpchIDsUnsignedIndexKey;
constexpr const char *PageScene::k1Name;
constexpr const char *PageScene::k2Name;
constexpr const char *PageScene::k3Name;
constexpr const char *PageScene::kBackgroundMusicIDs2Key;
constexpr const char *PageScene::kWelcomeCharID4Key;
constexpr const char *PageScene::kWelcomeSpchID4Key;
constexpr const char *PageScene::kNumWelcomeSpch4Key;
constexpr const char *PageScene::kWelcomeCharID5Key;
constexpr const char *PageScene::kWelcomeSpchID5Key;
constexpr const char *PageScene::kNumWelcomeSpch5Key;
constexpr const char *PageScene::kR1CN2SignClickCountKey;
constexpr const char *PageScene::kNavLocCN3SignKey;
constexpr const char *PageScene::kNumNavHotspotsSignedIndexKey;
constexpr const char *PageScene::kSignedIndexSignedIndexFormat;
constexpr const char *PageScene::kNavHSX1Key;
constexpr const char *PageScene::kNavHSY1Key;
constexpr const char *PageScene::kNavHSX2Key;
constexpr const char *PageScene::kNavHSY2Key;
constexpr const char *PageScene::kHandledName;
constexpr const char *PageScene::kInstrSpchIDs2UnsignedIndexKey;
constexpr const char *PageScene::kInstrSpchIDs3Key;
constexpr const char *PageScene::kWhistleKey;
constexpr const char *PageScene::kInstrSpchIDsUnsignedIndex1Key;
constexpr const char *PageScene::kInstrSpchIDsUnsignedIndex2Key;
constexpr const char *PageScene::kInstrSpchIDs43Key;
constexpr const char *PageScene::kTalkedToRockHeadKey;
constexpr const char *PageScene::kLadderKey;
constexpr const char *PageScene::kR1IG1LadderHereKey;
constexpr const char *PageScene::kInstrSpchIDs51Key;
constexpr const char *PageScene::kInstrSpchIDs52Key;
constexpr const char *PageScene::kWagonKey;
constexpr const char *PageScene::kR1CN4WagonHereKey;
constexpr const char *PageScene::kInstrSpchIDs73Key;
constexpr const char *PageScene::kInstrSpchIDs74Key;
constexpr const char *PageScene::kInstrSpchIDsUnsignedIndex3Key;
constexpr const char *PageScene::kPaigeName;
constexpr const char *PageScene::kHelpSpchIDsUnsignedIndexKey;
constexpr const char *PageScene::kMainSpchIDsUnsignedIndexKey;
constexpr const char *PageScene::kMainSpchIDsUnsignedIndex1Key;
constexpr const char *PageScene::kMainSpchIDsUnsignedIndex2Key;
constexpr const char *PageScene::kItemUnsignedIndexKey;
constexpr const char *PageScene::kAmbientIDKey;
constexpr const char *PageScene::kEnterSpchIDs1Key;
constexpr const char *PageScene::kEnterSpchIDs2Key;
constexpr const char *PageScene::kEnterSpchIDs3Key;
constexpr const char *PageScene::kReEnterSpchIDsA1Key;
constexpr const char *PageScene::kReEnterSpchIDsA2Key;
constexpr const char *PageScene::kReEnterSpchIDs11Key;
constexpr const char *PageScene::kReEnterSpchIDs12Key;
constexpr const char *PageScene::kReEnterSpchIDs13Key;
constexpr const char *PageScene::kReEnterSpchIDs2Key;
constexpr const char *PageScene::kReEnterSpchIDs4Key;
constexpr const char *PageScene::kAmbientVolumeKey;
constexpr const char *PageScene::kPaigeHelpID1Key;
constexpr const char *PageScene::kMaxPaigeIndexKey;
constexpr const char *PageScene::kPaigeHelpID21Key;
constexpr const char *PageScene::kPaigeHelpIDKey;
constexpr const char *PageScene::kMaxMainCharIndexKey;
constexpr const char *PageScene::kMainSpchIDUnsignedIndexKey;
constexpr const char *PageScene::kMainCharID1Key;
constexpr const char *PageScene::kMainCharID2Key;
constexpr const char *PageScene::kIrvinName;
constexpr const char *PageScene::kNameUnsignedIndexFormat;
constexpr const char *PageScene::kIrvinKey;
constexpr const char *PageScene::kRubberBandsKey;
constexpr const char *PageScene::kConstantPrefix;
constexpr const char *PageScene::kIDName;
constexpr const char *PageScene::kZName;
constexpr const char *PageScene::kR1LN2StateKey;
constexpr const char *PageScene::kR1LN2ReentryKey;
constexpr const char *PageScene::kR1LN2CatapultReentryKey;
constexpr const char *PageScene::kR1LN2IntroPendingKey;
constexpr const char *PageScene::kR1LN2MoviePendingKey;
constexpr const char *PageScene::kR1LN2DeliveryPendingKey;
constexpr const char *PageScene::kReEnterSpchIDs10Key;
constexpr const char *PageScene::kIriName;
constexpr const char *PageScene::kReEnterSpchIDs1Key;
constexpr const char *PageScene::kRriName;
constexpr const char *PageScene::kSiName;
constexpr const char *PageScene::kReEnterSpchIDs3Key;
constexpr const char *PageScene::kSiriName;
constexpr const char *PageScene::kReEnterSpchIDs7Key;
constexpr const char *PageScene::kReEnterSpchIDs8Key;
constexpr const char *PageScene::kSiiName;
constexpr const char *PageScene::kIrName;
constexpr const char *PageScene::kReEnterSpchIDsUnsignedIndexKey;
constexpr const char *PageScene::kIirName;
constexpr const char *PageScene::kRiName;
constexpr const char *PageScene::kInitialInstrSpchIDs1Key;
constexpr const char *PageScene::kIirsiriisirirsrName;
constexpr const char *PageScene::kInitialInstrSpchIDs2Key;
constexpr const char *PageScene::kIiisiName;
constexpr const char *PageScene::kInitialInstrSpchIDs27Key;
constexpr const char *PageScene::kInitialInstrSpchIDs26Key;
constexpr const char *PageScene::kInitialInstrSpchIDs28Key;
constexpr const char *PageScene::kInstrSpchIDs5Key;
constexpr const char *PageScene::kIiName;
constexpr const char *PageScene::kRisName;
constexpr const char *PageScene::kInstrSpchIDsUnsignedIndexKey;
constexpr const char *PageScene::kCollectiblesUnsignedIndexFormat;
constexpr const char *PageScene::kInventoryImageField;
constexpr const char *PageScene::kInventoryXField;
constexpr const char *PageScene::kInventoryYField;
constexpr const char *PageScene::kChesterYOffsetKey;
constexpr const char *PageScene::kInitialInstrSpchIDs3UnsignedIndexKey;
constexpr const char *PageScene::kShowInventoryOutlinesKey;
constexpr const char *PageScene::kInitialInstrSpchIDs36Key;
constexpr const char *PageScene::kBandsName;
constexpr const char *PageScene::kHoopName;
constexpr const char *PageScene::kCharIDName;
constexpr const char *PageScene::kSpchIDName;
constexpr const char *PageScene::kNumKey;
constexpr const char *PageScene::kSpchName;
constexpr const char *PageScene::kItemsCharIDs2Key;
constexpr const char *PageScene::kItemsSpchIDs2Key;
constexpr const char *PageScene::kNumItemSpch2Key;
constexpr const char *PageScene::kItemsCharIDs11Key;
constexpr const char *PageScene::kItemsSpchIDs11Key;
constexpr const char *PageScene::kItemsCharIDs15Key;
constexpr const char *PageScene::kItemsSpchIDs15Key;
constexpr const char *PageScene::k1UnsignedIndexFormat;
constexpr const char *PageScene::kItemsCharIDsKey;
constexpr const char *PageScene::kItemsSpchIDsKey;
constexpr const char *PageScene::kNoUseCharIDKey;
constexpr const char *PageScene::kNoUseSpchIDKey;
constexpr const char *PageScene::kIdleSpchIDs3Key;
constexpr const char *PageScene::kIdleSpchIDs1Key;
constexpr const char *PageScene::kRsName;
constexpr const char *PageScene::kIdleSpchIDs2Key;
constexpr const char *PageScene::kSrsName;
constexpr const char *PageScene::kHelpSpchIDs2Key;
constexpr const char *PageScene::kHelpSpchIDs1Key;
constexpr const char *PageScene::kSrName;
constexpr const char *PageScene::kSetIDName;
constexpr const char *PageScene::kLipsIDName;
constexpr const char *PageScene::kPropertyRegisterFidgetsToSettle;
constexpr const char *PageScene::kPropertyRegisterSpeechToSettle;
constexpr const char *PageScene::kPropertyLoadSpeechAnimsInBkgd;
constexpr const char *PageScene::kBodyIDName;
constexpr const char *PageScene::kMethodSetSpeechAnims;
constexpr const char *PageScene::kMethodAddFidgetGroup;
constexpr const char *PageScene::kSpeakKey;
constexpr const char *PageScene::kFidg1IDName;
constexpr const char *PageScene::kMethodAddFidget;
constexpr const char *PageScene::kNameFidgUnsignedIndexIDFormat;
constexpr const char *PageScene::kMaxSpeakSpchsKey;
constexpr const char *PageScene::kUnsignedIndexFormat;
constexpr const char *PageScene::kMaxSpeakIdleSpchsKey;
constexpr const char *PageScene::kSpeakName;
constexpr const char *PageScene::kSpeakIdleSpchsIDKey;
constexpr const char *PageScene::kMaxIdleIndexKey;
constexpr const char *PageScene::kMaxIdleIndexSpeechesKey;
constexpr const char *PageScene::kIdleCharIDKey;
constexpr const char *PageScene::kIdleSpchIDKey;
constexpr const char *PageScene::kR1CN1BellClickKey;
constexpr const char *PageScene::kR1CN1BellPendingKey;
constexpr const char *PageScene::kItem4Key;
constexpr const char *PageScene::kReaderKey;
constexpr const char *PageScene::kSamKey;
constexpr const char *PageScene::kPaigeKey;
constexpr const char *PageScene::kGChesterFidgIDName;
constexpr const char *PageScene::kChesterZKey;
constexpr const char *PageScene::kGClosedChesterZName;
constexpr const char *PageScene::kEnterCharIDKey;
constexpr const char *PageScene::kEnterSpchIDKey;
constexpr const char *PageScene::kR1CG1StateKey;
constexpr const char *PageScene::kOpenGatesIDKey;
constexpr const char *PageScene::kFenceZKey;
constexpr const char *PageScene::kUsedBellIDKey;
constexpr const char *PageScene::kBellZKey;
constexpr const char *PageScene::kR1CG1OpeningKey;
constexpr const char *PageScene::kMaxEntrySpchsKey;
constexpr const char *PageScene::kSignedIndexFormat;
constexpr const char *PageScene::kReEnterCharIDsKey;
constexpr const char *PageScene::kReEnterSpchIDsKey;
constexpr const char *PageScene::kReEnterMaxSpchsKey;
constexpr const char *PageScene::kBellIDKey;
constexpr const char *PageScene::kCloudMaskIDKey;
constexpr const char *PageScene::kCloudMaskZKey;
constexpr const char *PageScene::kR1CN1StateKey;
constexpr const char *PageScene::kReEnterSpchIDs21Key;
constexpr const char *PageScene::kRingBellIDKey;
constexpr const char *PageScene::kGateOpenCharIDKey;
constexpr const char *PageScene::kGateOpenSpchIDKey;
constexpr const char *PageScene::kCloudSFXIDKey;
constexpr const char *PageScene::kTargetMoveFramesKey;
constexpr const char *PageScene::kTargetMoveFPSKey;
constexpr const char *PageScene::kTargetMoveOffsetXKey;
constexpr const char *PageScene::kTargetMoveOffsetYKey;
constexpr const char *PageScene::kMapName;
constexpr const char *PageScene::kOutlineSpchCharUnsignedIndexKey;
constexpr const char *PageScene::kOutlineSpchIDUnsignedIndexKey;
constexpr const char *PageScene::kBellHSX1Key;
constexpr const char *PageScene::kBellHSY1Key;
constexpr const char *PageScene::kBellHSX2Key;
constexpr const char *PageScene::kBellHSY2Key;
constexpr const char *PageScene::kBellSpchCharIDsKey;
constexpr const char *PageScene::kBellSpchIDsKey;
constexpr const char *PageScene::kMaxBellSpchsKey;
constexpr const char *PageScene::kHelpSpchCharIDsKey;
constexpr const char *PageScene::kHelpSpchIDsKey;
constexpr const char *PageScene::kMainSpchCharIDsKey;
constexpr const char *PageScene::kMainSpchIDsKey;
constexpr const char *PageScene::kMaxMainSpchsKey;
constexpr const char *PageScene::kDontExitSpchCharIDsKey;
constexpr const char *PageScene::kDontExitSpchIDsKey;
constexpr const char *PageScene::kBellCharIDKey;
constexpr const char *PageScene::kBellSpchIDKey;
constexpr const char *PageScene::kR1CG1PaigeClicksKey;
constexpr const char *PageScene::kR1CG1MainClicksKey;
constexpr const char *PageScene::kMaxHelpSpchsKey;
constexpr const char *PageScene::kSignHSX1Key;
constexpr const char *PageScene::kSignHSY1Key;
constexpr const char *PageScene::kSignHSX2Key;
constexpr const char *PageScene::kSignHSY2Key;
constexpr const char *PageScene::kBigSignCharIDs1Key;
constexpr const char *PageScene::kBigSignIDs1Key;
constexpr const char *PageScene::kR1CG1BigSignClicksKey;
constexpr const char *PageScene::kBigSignCharIDs2Key;
constexpr const char *PageScene::kBigSignIDs2Key;
constexpr const char *PageScene::kSmSignCharIDs1Key;
constexpr const char *PageScene::kSmSignIDs1Key;
constexpr const char *PageScene::kMaxSmSignSpchs1Key;
constexpr const char *PageScene::kSmSignCharIDsKey;
constexpr const char *PageScene::kSmSignIDsKey;
constexpr const char *PageScene::kMaxSmSignSpchsKey;
constexpr const char *PageScene::kGChesterOpenIDName;
constexpr const char *PageScene::kDrawNumbersUnsignedIndexFormat;
constexpr const char *PageScene::kPropertyColor;
constexpr const char *PageScene::k4Name;
constexpr const char *PageScene::kNavHotspotZKey;

constexpr const char *PageScene::kMapButton;
constexpr const char *PageScene::kChestFont;
constexpr const char *PageScene::kChestClosedLid;
constexpr const char *PageScene::kChestButton;
constexpr const char *PageScene::kChestOpenLid;
constexpr const char *PageScene::kChestTray;
constexpr const char *PageScene::kNavigationCursorFormat;
constexpr const char *PageScene::kInventoryAnimationFormat;
constexpr const char *PageScene::kCharacterResourceFormat;
constexpr const char *PageScene::kAnimationExtension;
constexpr const char *PageScene::kSoundExtension;
constexpr const char *PageScene::kLipsExtension;
constexpr const char *PageScene::kBodyExtension;
constexpr const char *PageScene::kSpeechLibrarySuffix;
constexpr const char *PageScene::kEffectsLibrarySuffix;
constexpr const char *PageScene::kMusicLibrarySuffix;
constexpr const char *PageScene::kWordLibrarySuffix;
constexpr const char *PageScene::kFirstSpeechLibrarySuffix;
constexpr const char *PageScene::kSecondSpeechLibrarySuffix;
constexpr const char *PageScene::kCommonLibrary;
constexpr const char *PageScene::kPageMusicFormat;

OM::Canvas::ColorRemap PageScene::marbleColor(uint color) {
	static constexpr OM::Canvas::ColorRemap colors[] = {
		OM::Canvas::ColorRemap::kNone,          // Red.
		OM::Canvas::ColorRemap::kOrangeToWhite, // Orange.
		OM::Canvas::ColorRemap::kNone,          // Yellow.
		OM::Canvas::ColorRemap::kGreenToCyan,   // Green.
		OM::Canvas::ColorRemap::kNone,          // Blue.
	};
	if (ARRAYSIZE(colors) <= color)
		return OM::Canvas::ColorRemap::kNone;
	return colors[color];
}

PageScene::PageScene(TrinketEngine &vm, Shell::Scene &scene, World &world) : _vm(vm), _scene(scene), _world(world), _chestPaths(vm.resources()) {}

void PageScene::clear() {
	releaseNavigationCursor();
	for (auto &cursor : _navCursors)
		cursor.free();
	delete _rainGame;
	_rainGame = nullptr;
	delete _skyRide;
	_skyRide = nullptr;
	delete _catapult;
	_catapult = nullptr;
	delete _sheepCrossing;
	_sheepCrossing = nullptr;
	delete _rockHead;
	_rockHead = nullptr;
	delete _rainbowSlide;
	_rainbowSlide = nullptr;
	delete _musicPatterns;
	_musicPatterns = nullptr;
	interrupt();
	delete _flipbook;
	_flipbook = nullptr;
	delete _gallery;
	_gallery = nullptr;
	delete _cloudFloors;
	_cloudFloors = nullptr;
	delete _gumball;
	_gumball = nullptr;
	delete _wordPots;
	_wordPots = nullptr;
	delete _numberTree;
	_numberTree = nullptr;
	delete _sandwich;
	_sandwich = nullptr;
	for (auto &actor : _actors)
		delete actor._value;
	_actors.clear();
	for (auto *sound : _sounds)
		delete sound;
	_sounds.clear();
	delete _ambientSound;
	_ambientSound = nullptr;
	_ambient = nullptr;
	_ambientChoices.clear();
	_idleChoices.clear();
	_ambientWaiting = _idleWaiting = _hubStarted = false;
	_inventorPhase = kInventorIdle;
	_highlight = nullptr;
	for (auto &part : _parts)
		part = nullptr;
	_irvinClicks = _gearPlays = 0;
	_inventorTimerStarted = false;
	_name.clear();
	_page = nullptr;
	_bell = _cloud = nullptr;
	_gate = nullptr;
	_openingGate = 0;
	_gateDelayStarted = false;
	_chester = _chestLid = _chestTray = _dragged = nullptr;
	_chestMotion = _chestButton = nullptr;
	_chestPhase = kChestClosed;
	_draggingChest = false;
	_chestOffset = Common::Point();
	_items.clear();
	for (auto *label : _itemCounts)
		delete label;
	_itemCounts.clear();
	_putAwayPending = _putAwayActive = false;
	_chestOpen = false;
	_paused = false;
	_queenPhase = kQueenIdle;
	_queenSongStarted = false;
	_queenClicks = _mainClicks = _paigeClicks = 0;
	_queenArriving = false;
	_queenAmbient = nullptr;
	_queenAmbientChoice = 0;
	_arrivalIdleIdx = 0;
}

Common::String PageScene::characterResource(uint resource, Kind kind, uint part) const {
	const char *extension;
	if (kind == kAnimation)
		extension = kAnimationExtension;
	else if (kind == kSound)
		extension = kSoundExtension;
	else if (kind == kLips)
		extension = kLipsExtension;
	else
		extension = kBodyExtension;
	Common::String filename = Common::String::format(kCharacterResourceFormat, resource, extension);
	if (kind == kLips && part != 1)
		return Common::String();
	Common::Array<Common::String> libraries = {_name, _name + kSpeechLibrarySuffix, _name + kEffectsLibrarySuffix, Common::String(kCommonLibrary)};
	if (_name.equals(Page::kPageLA2))
		libraries.insert_at(3, _name + kMusicLibrarySuffix);
	if (_name.equals(Page::kPageIA2) || _name.equals(Page::kPageIA3) || _name.equals(Page::kPageCA1))
		libraries.insert_at(2, _name + kWordLibrarySuffix);
	if (_name.equals(Page::kPageLA1)) {
		libraries.insert_at(1, _name + kFirstSpeechLibrarySuffix);
		libraries.insert_at(2, _name + kSecondSpeechLibrarySuffix);
		libraries.insert_at(3, _name + kWordLibrarySuffix);
	}
	for (const Common::String &library : libraries) {
		Common::String path = library + "/" + filename;
		Common::ScopedPtr<Common::SeekableReadStream> stream(_vm.resources().open(path));
		if (stream)
			return path;
	}
	return _name + "/" + filename;
}

bool PageScene::actor(const Common::String &name, const Common::String &prefix, bool body) {
	uint settle = _page->number(prefix + kSetIDName);
	if (!settle)
		settle = _page->number(prefix + kLipsIDName);
	if (!settle)
		return false;
	Common::ScopedPtr<Shell::Character> character(new Shell::Character(_scene, _vm.mixer(), _vm.resources(), *_vm._rnd, *this));
	if (!character->create(settle, _page->number(prefix + kZName)))
		return false;
	character->movable = false;
	Core::ScriptValue result;
	character->set(kPropertyRegisterFidgetsToSettle, {}, Core::ScriptValue(0));
	character->set(kPropertyRegisterSpeechToSettle, {}, Core::ScriptValue(0));
	character->set(kPropertyLoadSpeechAnimsInBkgd, {}, Core::ScriptValue(0));
	int lips = _page->number(prefix + kLipsIDName);
	if (lips) {
		Core::ScriptArguments parts = {Core::ScriptValue(lips), Core::ScriptValue(1), Core::ScriptValue(1)};
		int bodyID = body ? _page->number(prefix + kBodyIDName) : 0;
		if (bodyID) {
			parts.push_back(Core::ScriptValue(bodyID));
			parts.push_back(Core::ScriptValue(0));
			parts.push_back(Core::ScriptValue(0));
		}
		character->play(kMethodSetSpeechAnims, parts, result);
	}
	character->play(kMethodAddFidgetGroup, {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	int blink = prefix.equals(kSpeakKey) || prefix.equals(kQueenKey) ? _page->number(prefix + kFidg1IDName) : settle;
	character->play(kMethodAddFidget, {Core::ScriptValue(1), Core::ScriptValue(blink)}, result);
	if (body) {
		bool queen = prefix.equals(kQueenKey);
		character->play(kMethodAddFidgetGroup, {Core::ScriptValue(2), Core::ScriptValue(queen ? 5 : 15), Core::ScriptValue(queen ? 10 : 30)}, result);
		for (uint i = queen ? 2 : 1; i <= 2; i++) {
			int id = _page->number(Common::String::format(kNameFidgUnsignedIndexIDFormat, prefix.c_str(), i));
			if (id)
				character->play(kMethodAddFidget, {Core::ScriptValue(2), Core::ScriptValue(id)}, result);
		}
	}
	_actors[name] = character.release();
	return true;
}

void PageScene::say(const Common::String &actorName, uint resource) {
	if (resource && _actors.contains(actorName)) {
		if (_name.equals(Page::kPageCN2) || _name.equals(Page::kPageCN1) || _name.equals(Page::kPageLA1))
			_idleWaiting = false;
		Speech speech = {actorName, resource, false, 0};
		_speech.push_back(speech);
	}
}

void PageScene::delaySpeech(uint milliseconds) {
	Speech speech = {Common::String(), 0, false, milliseconds};
	_speech.push_back(speech);
}

void PageScene::queueSound(uint resource) {
	Speech speech = {Common::String(), resource, false, 0};
	_speech.push_back(speech);
}

void PageScene::arrivalSpeaker(uint group) {
	if (!group)
		group = _vm._rnd->getRandomNumberRng(1, _page->number(kMaxSpeakSpchsKey));
	Common::String suffix = Common::String::format(kUnsignedIndexFormat, group);
	for (uint i = 1; i <= static_cast<uint>(_page->number(kMaxSpeakIdleSpchsKey + suffix)); i++)
		say(kSpeakName, _page->number(kSpeakIdleSpchsIDKey + suffix + Common::String::format(kUnsignedIndexFormat, i)));
}

void PageScene::arrivalIdle(uint32 time) {
	if (_chestPhase != kChestClosed || _putAwayPending || _putAwayActive)
		return;
	if (!_idleWaiting) {
		_idleWaiting = true;
		_idleStart = time;
	}
	if (time - _idleStart < 7000)
		return;
	_idleWaiting = false;
	_arrivalIdleIdx = _arrivalIdleIdx % MAX(1, _page->number(kMaxIdleIndexKey) * 3) + 1;
	arrivalSpeaker();
	if (_arrivalIdleIdx % 3 == 2) {
		Common::String group = Common::String::format(kUnsignedIndexFormat, static_cast<uint>(_vm._rnd->getRandomNumberRng(1, 2)));
		for (uint i = 1; i <= static_cast<uint>(_page->number(kMaxIdleIndexSpeechesKey + group)); i++) {
			delaySpeech(1500);
			Common::String suffix = group + Common::String::format(kUnsignedIndexFormat, i);
			say(_page->text(kIdleCharIDKey + suffix), _page->number(kIdleSpchIDKey + suffix));
		}
	}
}

void PageScene::speeches(const Common::String &actors, const Common::String &resources, uint count) {
	for (uint i = 1; i <= count; i++) {
		Common::String suffix = Common::String::format(kUnsignedIndexFormat, i);
		say(_page->text(actors + suffix), _page->number(resources + suffix));
	}
}

void PageScene::completeSpeech() {
	if (_placeBell) {
		if (_bell)
			_bell->visible = true;
		_placeBell = false;
	}
	if (_collectBell) {
		_world.set(kR1CN1BellClickKey, 1);
		_world.set(kR1CN1BellPendingKey, 0);
		_world.set(kItem4Key, MIN(25, _world.get(kItem4Key) + 1));
		if (_bell)
			_bell->visible = false;
		_collectBell = false;
		putAway();
	}
}

void PageScene::interrupt() {
	_queuedSound.reset();
	if (_speaking)
		_speaking->stop();
	if (_speakingAlso)
		_speakingAlso->stop();
	_speaking = nullptr;
	_speakingAlso = nullptr;
	_speech.clear();
	_nextSpeech = 0;
	_speechDelayStarted = false;
	if (_name.equals(Page::kPageCN1) || _name.equals(Page::kPageLA1))
		_idleWaiting = false;
	if (_clearCloud && _cloud)
		_cloud->visible = false;
	_clearCloud = _cloudMoving = false;
	completeSpeech();
}

bool PageScene::enter(const Page &page, const Common::String &name, bool practice) {
	clear();
	_page = &page;
	_name = name;
	_practice = practice;
	if (name.equals(Page::kPageLA3)) {
		if (!actor(kReaderName, kReaderKey) || !actor(kSamName, kSamKey) || !actor(kPaigeName, kPaigeKey, false) ||
			!_scene.add(kMapButton, 100, 1, true))
			return false;
		_rainGame = new RainGame(*this);
		return _rainGame->enter();
	}
	if (name.equals(Page::kPageRSS)) {
		_skyRide = new SkyRide(*this);
		return _skyRide->enter();
	}
	if (name.equals(Page::kPageIG1)) {
		if (!_scene.add(kMapButton, 100, 1, true))
			return false;
		_rockHead = new RockHead(*this);
		return _rockHead->enter();
	}
	if (!name.equals(Page::kPageCN1) && !name.equals(Page::kPageCG1) && !name.equals(Page::kPageCN2) && !name.equals(Page::kPageIN4) &&
		!name.equals(Page::kPageIN1) && !name.equals(Page::kPageLN2) && !name.equals(Page::kPageLA1) && !name.equals(Page::kPageIA2) &&
		!name.equals(Page::kPageIA1) && !name.equals(Page::kPageIA3) && !name.equals(Page::kPageCA1) && !name.equals(Page::kPageCN3) &&
		!name.equals(Page::kPageCA2) && !name.equals(Page::kPageLA2) && !name.equals(Page::kPageCN4) && !name.equals(Page::kPageCG2) &&
		!name.equals(Page::kPageLG2))
		return true;
	if (!actor(kReaderName, kReaderKey) || !actor(kSamName, kSamKey) || !actor(kPaigeName, kPaigeKey, false))
		return false;
	if (!name.equals(Page::kPageCA2)) {
		int chesterDepth;
		if (name.equals(Page::kPageCN3))
			chesterDepth = page.number(kChesterZKey);
		else
			chesterDepth = page.number(kGClosedChesterZName);
		_chester = _scene.add(characterResource(page.number(kGChesterFidgIDName), kAnimation), chesterDepth);
	}
	if (!_scene.add(kMapButton, 100, 1, true))
		return false;
	if (name.equals(Page::kPageLG2)) {
		_catapult = new Catapult(*this);
		return _catapult->enter();
	}
	if (name.equals(Page::kPageCG2)) {
		_sheepCrossing = new SheepCrossing(*this);
		return _sheepCrossing->enter();
	}
	if (name.equals(Page::kPageCN4)) {
		_rainbowSlide = new RainbowSlide(*this);
		return _rainbowSlide->enter();
	}
	if (name.equals(Page::kPageLA1)) {
		_gumball = new Gumball(*this);
		return _gumball->enter();
	}
	if (name.equals(Page::kPageLA2)) {
		_musicPatterns = new MusicPatterns(*this);
		return _musicPatterns->enter();
	}
	if (name.equals(Page::kPageIA3)) {
		_wordPots = new WordPots(*this);
		return _wordPots->enter();
	}
	if (name.equals(Page::kPageCA2)) {
		_flipbook = new Flipbook(*this);
		return _flipbook->enter();
	}
	if (name.equals(Page::kPageCN3)) {
		_gallery = new Gallery(*this);
		return _gallery->enter();
	}
	if (name.equals(Page::kPageCA1)) {
		_cloudFloors = new CloudFloors(*this);
		return _cloudFloors->enter();
	}
	if (name.equals(Page::kPageIA1)) {
		_numberTree = new NumberTree(*this);
		return _numberTree->enter();
	}
	if (name.equals(Page::kPageIA2)) {
		_sandwich = new Sandwich(*this);
		return _sandwich->enter();
	}
	if (name.equals(Page::kPageCN2))
		return enterQueen();
	if (name.equals(Page::kPageIN4))
		return enterHub();
	if (name.equals(Page::kPageLN2))
		return enterInventor();
	if (name.equals(Page::kPageIN1)) {
		if (!_world.visits(name))
			speeches(kEnterCharIDKey, kEnterSpchIDKey, 2);
		return true;
	}
	if (name.equals(Page::kPageCG1)) {
		int state = MAX(1, _world.get(kR1CG1StateKey));
		if (state == 1 && _world.get(kItem4Key))
			state = 2;
		_world.set(kR1CG1StateKey, state);
		_gate = _scene.add(characterResource(page.number(kOpenGatesIDKey), kAnimation), page.number(kFenceZKey));
		_bell = _scene.add(characterResource(page.number(kUsedBellIDKey), kAnimation), page.number(kBellZKey));
		if (!_gate || !_bell)
			return false;
		_gate->setFrame(1);
		_bell->visible = state == 3;
		if (state == 3)
			_gate->setFrame(_gate->getNumberOfFrames());
		if (_world.get(kR1CG1OpeningKey))
			_openingGate = 3;
		else if (!_world.visits(name))
			speeches(kEnterCharIDKey, kEnterSpchIDKey, page.number(kMaxEntrySpchsKey));
		else if (state < 3) {
			Common::String suffix = Common::String::format(kSignedIndexFormat, state);
			speeches(kReEnterCharIDsKey + suffix, kReEnterSpchIDsKey + suffix, page.number(kReEnterMaxSpchsKey + suffix));
		}
		return true;
	}
	if (!actor(kSpeakName, kSpeakKey, false))
		return false;
	_bell = _scene.add(characterResource(page.number(kBellIDKey), kAnimation), page.number(kBellZKey));
	_cloud = _scene.add(characterResource(page.number(kCloudMaskIDKey), kAnimation), page.number(kCloudMaskZKey));
	if (!_bell || !_cloud)
		return false;
	_bell->visible = !_world.get(kR1CN1BellClickKey);
	_cloud->visible = !_world.visits(Page::kPageCN2);
	if (_world.get(kR1CN1BellPendingKey)) {
		_collectBell = true;
		delaySpeech(450);
	} else if (!_world.visits(name)) {
		speeches(kEnterCharIDKey, kEnterSpchIDKey, page.number(kMaxEntrySpchsKey));
		delaySpeech(500);
		arrivalSpeaker(1);
	} else {
		if (_world.visits(name) == 1)
			say(kReaderName, page.number(kReEnterSpchIDs11Key));
		if (_world.visits(Page::kPageCN2) && _world.get(kR1CN1StateKey) < 3) {
			_world.set(kR1CN1StateKey, 3);
			_cloud->visible = true;
			_clearCloud = true;
			_cloudSpeechIdx = _speech.size();
			_cloudOrigin = Common::Point(_cloud->x, _cloud->y);
			say(kSamName, page.number(kReEnterSpchIDs21Key));
		}
	}
	return true;
}

void PageScene::pause(bool paused, uint32 time) {
	releaseNavigationCursor();
	if (_paused == paused)
		return;
	if (paused) {
		_draggingChest = false;
		_pauseStart = time;
	} else {
		if (_speechDelayStarted)
			_speechDelayStart += time - _pauseStart;
		if (_cloudMoving)
			_cloudStart += time - _pauseStart;
		if (_gateDelayStarted)
			_gateStart += time - _pauseStart;
		if (_ambientWaiting)
			_ambientStart += time - _pauseStart;
		if (_idleWaiting)
			_idleStart += time - _pauseStart;
		if (_inventorTimerStarted)
			_inventorStart += time - _pauseStart;
		if (_gumball)
			_gumball->resume(time - _pauseStart);
		if (_sandwich)
			_sandwich->resume(time - _pauseStart);
		if (_numberTree)
			_numberTree->resume(time - _pauseStart);
		if (_wordPots)
			_wordPots->resume(time - _pauseStart);
		if (_flipbook)
			_flipbook->resume(time);
		if (_gallery)
			_gallery->resume(time);
		if (_cloudFloors)
			_cloudFloors->resume(time - _pauseStart);
		if (_musicPatterns)
			_musicPatterns->resume(time - _pauseStart);
	}
	if (_skyRide)
		_skyRide->pause(paused, time - _pauseStart);
	if (_rainGame)
		_rainGame->pause(paused, time - _pauseStart);
	if (_catapult)
		_catapult->pause(paused, time - _pauseStart);
	if (_sheepCrossing)
		_sheepCrossing->pause(paused, time - _pauseStart);
	if (_rockHead && !paused)
		_rockHead->resume(time - _pauseStart);
	if (_rainbowSlide)
		_rainbowSlide->pause(paused, time - _pauseStart);
	_paused = paused;
	if (_musicPatterns)
		_musicPatterns->pause(paused);
	if (_gumball)
		_gumball->pause(paused);
	if (_numberTree)
		_numberTree->pause(paused);
	if (_wordPots)
		_wordPots->pause(paused);
	if (_flipbook)
		_flipbook->pause(paused);
	if (_gallery)
		_gallery->pause(paused);
	if (_cloudFloors)
		_cloudFloors->pause(paused);
	for (auto &actor : _actors)
		actor._value->pause(paused, time);
	for (auto *sound : _sounds)
		sound->pause(paused);
	if (_queuedSound)
		_queuedSound->pause(paused);
	if (_gate)
		_gate->pauseAnimation(paused);
	if (_ambient)
		_ambient->pauseAnimation(paused);
	if (_queenAmbient)
		_queenAmbient->pauseAnimation(paused);
	if (_ambientSound)
		_ambientSound->pause(paused);
	for (auto *sprite : {_chester, _chestMotion, _chestLid}) {
		if (sprite)
			sprite->pauseAnimation(paused);
	}
	if (!paused && _highlight)
		_highlightDue += time - _pauseStart;
}

void PageScene::playSounds(const Common::Array<uint16> &cues) {
	for (uint id : cues) {
		OM::Sound *sound = new OM::Sound(_vm.mixer());
		sound->play(_vm.resources(), characterResource(id, kSound), Audio::Mixer::kSFXSoundType);
		_sounds.push_back(sound);
	}
}

void PageScene::update(uint32 time) {
	if (_paused)
		return;
	_time = time;
	updateChest(time);
	if (_rainGame)
		_rainGame->advance(time);
	if (_skyRide)
		_skyRide->update(time);
	if (_catapult)
		_catapult->advance(time);
	if (_flipbook)
		_flipbook->advance(time);
	if (_gallery)
		_gallery->advance(time);
	if (_cloudFloors)
		_cloudFloors->advance(time);
	if (_wordPots)
		_wordPots->advance(time);
	if (_musicPatterns)
		_musicPatterns->advance(time);
	if (_name.equals(Page::kPageIN4))
		updateHub(time);
	if (_name.equals(Page::kPageLN2))
		updateInventorAmbient(time);
	if (_name.equals(Page::kPageCN2))
		updateQueenAmbient(time);
	Common::Array<uint16> cues;
	for (auto &actor : _actors)
		actor._value->update(time, cues);
	playSounds(cues);
	if (_queenArriving) {
		if (_actors[kReaderName]->isBusy() || _actors[kSamName]->isBusy())
			return;
		_queenArriving = false;
	}
	for (uint i = 0; i < _sounds.size();) {
		if (_sounds[i]->isPlaying())
			i += 1;
		else {
			delete _sounds[i];
			_sounds.remove_at(i);
		}
	}
	if ((_queuedSound && _queuedSound->isPlaying()) || (_speaking && _speaking->isBusy()) || (_speakingAlso && _speakingAlso->isBusy()))
		return;
	_queuedSound.reset();
	_speaking = nullptr;
	_speakingAlso = nullptr;
	if (chestBusy())
		return;
	if (_rainGame)
		_rainGame->update(time);
	if (_name.equals(Page::kPageCN2))
		updateQueen();
	if (_name.equals(Page::kPageLN2))
		updateInventor(time);
	if (_gumball)
		_gumball->update(time);
	if (_sandwich)
		_sandwich->update(time);
	if (_numberTree)
		_numberTree->update(time);
	if (_wordPots)
		_wordPots->update(time);
	if (_cloudFloors)
		_cloudFloors->update(time);
	if (_flipbook)
		_flipbook->update(time);
	if (_gallery)
		_gallery->update(time);
	if (_musicPatterns)
		_musicPatterns->update(time);
	if (_rainbowSlide)
		_rainbowSlide->update(time);
	if (_rockHead)
		_rockHead->update(time);
	if (_sheepCrossing)
		_sheepCrossing->update(time);
	if (_catapult)
		_catapult->update(time);
	if (_putAwayPending && !sceneBusy() && _nextSpeech >= _speech.size()) {
		_putAwayPending = false;
		if (_chestPhase == kChestClosed) {
			_putAwayActive = true;
			openChest();
		}
		return;
	}
	if (_openingGate == 3 || _openingGate == 4) {
		if (!_gateDelayStarted) {
			_gateStart = time;
			_gateDelayStarted = true;
		}
		if (time - _gateStart < (_openingGate == 3 ? 1500U : 500U))
			return;
		_gateDelayStarted = false;
		if (_openingGate == 3) {
			_world.set(kR1CG1StateKey, 3);
			_world.set(kR1CG1OpeningKey, 0);
			_actors[kReaderName]->setCallTime(time);
			_actors[kReaderName]->playAnim(_page->number(kRingBellIDKey), 1, false);
			_openingGate = 1;
		} else {
			_openingGate = 0;
			_placeBell = true;
			speeches(kGateOpenCharIDKey, kGateOpenSpchIDKey, 2);
		}
	}
	if (_openingGate == 1) {
		if (_actors[kReaderName]->isBusy())
			return;
		_gate->startAnimation(1);
		_openingGate = 2;
		return;
	}
	if (_openingGate == 2) {
		if (_gate->isRunning())
			return;
		_gate->setFrame(_gate->getNumberOfFrames());
		_openingGate = 4;
		return;
	}
	if (_clearCloud && _nextSpeech == _cloudSpeechIdx) {
		if (!_cloudMoving) {
			_cloudStart = time;
			_cloudMoving = true;
			OM::Sound *sound = new OM::Sound(_vm.mixer());
			sound->play(_vm.resources(), characterResource(_page->number(kCloudSFXIDKey), kSound), Audio::Mixer::kSFXSoundType);
			_sounds.push_back(sound);
		}
		uint frames = MAX(1, _page->number(kTargetMoveFramesKey));
		uint frame = MIN<uint>(frames, static_cast<uint64>(time - _cloudStart) * _page->number(kTargetMoveFPSKey) / 1000);
		_cloud->x = _cloudOrigin.x + _page->number(kTargetMoveOffsetXKey) * frame / frames;
		_cloud->y = _cloudOrigin.y + _page->number(kTargetMoveOffsetYKey) * frame / frames;
		if (frame < frames)
			return;
		_cloud->visible = false;
		_clearCloud = _cloudMoving = false;
	}
	if (_nextSpeech < _speech.size()) {
		const Speech &speech = _speech[_nextSpeech];
		if (speech.delay) {
			if (!_speechDelayStarted) {
				_speechDelayStarted = true;
				_speechDelayStart = time;
			}
			if (time - _speechDelayStart < speech.delay)
				return;
			_speechDelayStarted = false;
			_nextSpeech += 1;
			return;
		}
		if (speech.actor.empty()) {
			_queuedSound.reset(new OM::Sound(_vm.mixer()));
			_queuedSound->play(_vm.resources(), characterResource(speech.resource, kSound), Audio::Mixer::kSFXSoundType);
			_nextSpeech += 1;
			return;
		}
		_speaking = _actors[speech.actor];
		_nextSpeech += 1;
		_speaking->setCallTime(time);
		_speaking->playSpeech(speech.resource);
		if (speech.together && _nextSpeech < _speech.size()) {
			const Speech &other = _speech[_nextSpeech];
			_nextSpeech += 1;
			_speakingAlso = _actors[other.actor];
			_speakingAlso->setCallTime(time);
			_speakingAlso->playSpeech(other.resource);
		}
	} else {
		completeSpeech();
		if (_name.equals(Page::kPageCN1))
			arrivalIdle(time);
		if (_name.equals(Page::kPageCN2))
			queenIdle(time);
		if (_name.equals(Page::kPageIN4))
			hubIdle(time);
		if (_name.equals(Page::kPageLN2))
			inventorIdle(time);
	}
}

Common::String PageScene::click(Common::Point point) {
	if (chestBusy() || _putAwayPending || _putAwayActive || _queenArriving)
		return kHandledName;
	if (_rainGame) {
		Shell::Sprite *map = _scene.hitSprite(point, 100, 100, true);
		if (map && map->action == 1 && !_rainGame->ending())
			return kMapName;
		_rainGame->click(point);
		return kHandledName;
	}
	if (_skyRide)
		return kHandledName;
	if (_catapult && _catapult->busy()) {
		interrupt();
		return kHandledName;
	}
	if (_sheepCrossing && _sheepCrossing->busy()) {
		interrupt();
		return kHandledName;
	}
	if (_rockHead && _rockHead->busy()) {
		interrupt();
		return kHandledName;
	}
	if (_rainbowSlide && _rainbowSlide->busy()) {
		interrupt();
		return kHandledName;
	}
	if (_musicPatterns && _musicPatterns->busy()) {
		_musicPatterns->skipSpeech();
		return kHandledName;
	}
	if (_flipbook && _flipbook->busy())
		return kHandledName;
	if (_gallery && _gallery->busy()) {
		interrupt();
		return kHandledName;
	}
	if (_cloudFloors && _cloudFloors->busy()) {
		interrupt();
		return kHandledName;
	}
	if (_wordPots && _wordPots->busy()) {
		_wordPots->skipSpeech();
		return kHandledName;
	}
	if (_numberTree && _numberTree->busy()) {
		_numberTree->skipSpeech();
		return kHandledName;
	}
	if (_sandwich && _sandwich->busy()) {
		_sandwich->skipSpeech();
		return kHandledName;
	}
	if (_gumball && _gumball->busy()) {
		_gumball->skipSpeech();
		return kHandledName;
	}
	if (!_name.equals(Page::kPageCN1) && !_name.equals(Page::kPageCG1) && !_name.equals(Page::kPageCN2) &&
		!_name.equals(Page::kPageIN4) && !_name.equals(Page::kPageIN1) && !_name.equals(Page::kPageLN2) &&
		!_gumball && !_sandwich && !_numberTree && !_wordPots && !_cloudFloors && !_gallery && !_flipbook && !_musicPatterns && !_rainbowSlide && !_rockHead && !_sheepCrossing && !_catapult)
		return Common::String();
	if (_inventorPhase != kInventorIdle) {
		if (_inventorPhase == kInventorIntro || _inventorPhase == kInventorDeliverySpeech || _inventorPhase == kInventorMoreItems)
			interrupt();
		return kHandledName;
	}
	if (_queenPhase != kQueenIdle) {
		interrupt();
		return kHandledName;
	}
	if (_openingGate)
		return kHandledName;
	if (_chestOpen) {
		for (Shell::Sprite *item : _items) {
			if (!item->action || !item->covers(point))
				continue;
			if (item->action < 0) {
				interrupt();
				uint id = -item->action;
				say(_page->text(Common::String::format(kOutlineSpchCharUnsignedIndexKey, id)), _page->number(Common::String::format(kOutlineSpchIDUnsignedIndexKey, id)));
				return kHandledName;
			}
			_dragged = item;
			_dragOffset = point - Common::Point(item->x, item->y);
			_dragHome = Common::Point(item->x, item->y);
			return kHandledName;
		}
		if (!chestContains(point) || (_chestButton && _chestButton->covers(point))) {
			playSounds({7002});
			closeChest();
		} else {
			_draggingChest = true;
			_chestDragPoint = point;
		}
		return kHandledName;
	}
	if (!_practice && _chester && _chester->covers(point)) {
		interrupt();
		openChest(true);
		return kHandledName;
	}
	Shell::Sprite *sprite = _scene.hitSprite(point, 100, 100, true);
	if (sprite && sprite->action == 1)
		return kMapName;
	interrupt();
	if (_catapult)
		return _catapult->click(point) ? kHandledName : Common::String();
	if (_sheepCrossing)
		return _sheepCrossing->click(point) ? kHandledName : Common::String();
	if (_rockHead)
		return _rockHead->click(point) ? kHandledName : Common::String();
	if (_rainbowSlide)
		return _rainbowSlide->click(point) ? kHandledName : Common::String();
	if (_musicPatterns)
		return _musicPatterns->click(point) ? kHandledName : Common::String();
	if (_flipbook)
		return _flipbook->click(point) ? kHandledName : Common::String();
	if (_gallery)
		return _gallery->click(point) ? kHandledName : Common::String();
	if (_numberTree)
		return _numberTree->click(point) ? kHandledName : Common::String();
	if (_cloudFloors)
		return _cloudFloors->click(point) ? kHandledName : Common::String();
	if (_wordPots)
		return _wordPots->click(point) ? kHandledName : Common::String();
	if (_sandwich)
		return _sandwich->click(point) ? kHandledName : Common::String();
	if (_gumball)
		return _gumball->click(point) ? kHandledName : Common::String();
	if (_name.equals(Page::kPageCG1))
		return gateClick(point);
	if (_name.equals(Page::kPageCN2))
		return queenClick(point);
	if (_name.equals(Page::kPageIN4))
		return hubClick(point);
	if (_name.equals(Page::kPageIN1))
		return groveClick(point);
	if (_name.equals(Page::kPageLN2))
		return inventorClick(point);
	if (_bell && _bell->visible &&
		Common::Rect(_page->number(kBellHSX1Key), _page->number(kBellHSY1Key), _page->number(kBellHSX2Key), _page->number(kBellHSY2Key)).contains(point)) {
		Common::String group = _world.visits(Page::kPageCG1) ? k2Name : k1Name;
		_world.set(kR1CN1BellClickKey, 1);
		_world.set(kR1CN1BellPendingKey, 1);
		playSounds({1802});
		delaySpeech(450);
		speeches(kBellSpchCharIDsKey + group, kBellSpchIDsKey + group, _page->number(kMaxBellSpchsKey + group));
		delaySpeech(450);
		_collectBell = true;
		return kHandledName;
	}
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key.equals(kSpeakName)) {
			arrivalSpeaker();
		} else if (actor._key.equals(kPaigeName)) {
			Common::String group = _world.get(kR1CN1BellClickKey) ? k2Name : k1Name;
			speeches(kHelpSpchCharIDsKey + group, kHelpSpchIDsKey + group, 2);
		} else {
			uint choice = _world.get(kR1CN1BellClickKey) ? _vm._rnd->getRandomNumberRng(2, 3) : 1;
			Common::String group = Common::String::format(kUnsignedIndexFormat, choice);
			speeches(kMainSpchCharIDsKey + group, kMainSpchIDsKey + group, _page->number(kMaxMainSpchsKey + group));
		}
		return kHandledName;
	}
	if (!_practice && !_world.visits(Page::kPageCN2) && 350 <= point.x) {
		uint choice = _vm._rnd->getRandomNumberRng(1, 2);
		Common::String suffix = Common::String::format(kUnsignedIndexFormat, choice);
		say(_page->text(kDontExitSpchCharIDsKey + suffix), _page->number(kDontExitSpchIDsKey + suffix));
		return kHandledName;
	}
	return Common::String();
}

Common::String PageScene::gateClick(Common::Point point) {
	int state = _world.get(kR1CG1StateKey);
	if (_bell && _bell->visible && _bell->covers(point)) {
		say(_page->text(kBellCharIDKey), _page->number(kBellSpchIDKey));
		return kHandledName;
	}
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		bool paige = actor._key.equals(kPaigeName);
		Common::String key = paige ? kR1CG1PaigeClicksKey : kR1CG1MainClicksKey;
		int count = _world.get(key);
		int choice;
		if (count) {
			if (paige)
				choice = _vm._rnd->getRandomNumberRng(4, 5);
			else
				choice = 4;
		} else {
			choice = state;
		}
		Common::String suffix = Common::String::format(kSignedIndexFormat, choice);
		speeches((paige ? kHelpSpchCharIDsKey : kMainSpchCharIDsKey) + suffix,
				 (paige ? kHelpSpchIDsKey : kMainSpchIDsKey) + suffix, _page->number((paige ? kMaxHelpSpchsKey : kMaxMainSpchsKey) + suffix));
		_world.set(key, count + 1);
		return kHandledName;
	}
	for (uint i = 1; i <= 2; i++) {
		Common::String suffix = Common::String::format(kUnsignedIndexFormat, i);
		Common::Rect rect(_page->number(kSignHSX1Key + suffix), _page->number(kSignHSY1Key + suffix),
						  _page->number(kSignHSX2Key + suffix), _page->number(kSignHSY2Key + suffix));
		if (!rect.contains(point))
			continue;
		if (i == 1) {
			say(_page->text(kBigSignCharIDs1Key), _page->number(kBigSignIDs1Key));
			if (!_world.get(kR1CG1BigSignClicksKey))
				say(_page->text(kBigSignCharIDs2Key), _page->number(kBigSignIDs2Key));
			_world.set(kR1CG1BigSignClicksKey, _world.get(kR1CG1BigSignClicksKey) + 1);
		} else {
			speeches(kSmSignCharIDs1Key, kSmSignIDs1Key, _page->number(kMaxSmSignSpchs1Key));
			if (state < 3) {
				Common::String group = Common::String::format(kSignedIndexFormat, state == 1 ? _vm._rnd->getRandomNumberRng(2, 3) : 4);
				speeches(kSmSignCharIDsKey + group, kSmSignIDsKey + group, _page->number(kMaxSmSignSpchsKey + group));
			}
		}
		return kHandledName;
	}
	if (!_practice && state < 3) {
		for (const auto &exit : _page->exits()) {
			if (exit.destination.equals(Page::kPageCN1) || !exit.rect.contains(point))
				continue;
			Common::String suffix = Common::String::format(kSignedIndexFormat, _vm._rnd->getRandomNumberRng(1, 2));
			say(_page->text(kDontExitSpchCharIDsKey + suffix), _page->number(kDontExitSpchIDsKey + suffix));
			return kHandledName;
		}
	}
	return Common::String();
}

void PageScene::openChest(bool animateCharacter) {
	if (_chestPhase != kChestClosed && _chestPhase != kChestCharacter)
		return;
	if (_name.equals(Page::kPageCN2))
		_idleWaiting = false;
	if (animateCharacter) {
		_chestMotion = _scene.add(characterResource(_page->number(kGChesterOpenIDName), kAnimation), _page->number(kGClosedChesterZName));
		if (_chestMotion) {
			_chestMotion->startAnimation(1);
			_chestPhase = kChestCharacter;
			if (_chester)
				_chester->visible = false;
			return;
		}
	}
	_chestLid = _scene.add(kChestClosedLid, 211);
	_chestTray = _scene.add(kChestTray, 200);
	_chestButton = _scene.add(kChestButton, 221);
	if (!_chestLid || !_chestTray) {
		finishChest();
		return;
	}
	_chestPhase = kChestOpening;
	_restoreChester = true;
	_chestLid->startAnimation(1);
	playSounds({7001});
	int offset = _page->number(kChesterYOffsetKey);
	_chestLid->y -= offset;
	_chestTray->y -= offset;
	if (_chestButton) {
		_chestButton->y -= offset;
		_chestButton->visible = false;
	}
	if (_chester)
		_chester->visible = false;
	if (!_chestFont)
		_chestFont = _vm.fonts().load(kChestFont);
	for (uint id = 1; id <= 17; id++) {
		int count = _world.get(Common::String::format(kItemUnsignedIndexKey, id));
		if (!count)
			continue;
		Common::String entry = Common::String::format(kCollectiblesUnsignedIndexFormat, id);
		int resource = _page->number(entry + kInventoryImageField);
		if (!resource)
			continue;
		Shell::Sprite *item = _scene.add(Common::String::format(kInventoryAnimationFormat, resource), 202, id);
		if (!item)
			continue;
		if (10 <= id && id <= 14)
			item->colorRemap = marbleColor(id - 10);
		item->x = _page->number(entry + kInventoryXField);
		item->y = _page->number(entry + kInventoryYField) - offset;
		item->boxTouchy = id == 2 || id == 6 || id == 8;
		if (1 < count) {
			Shell::Sprite *underneath = _scene.add(Common::String::format(kInventoryAnimationFormat, resource), 201);
			if (underneath) {
				underneath->colorRemap = item->colorRemap;
				underneath->x = item->x;
				underneath->y = item->y;
				_items.push_back(underneath);
			}
		}
		_items.push_back(item);
		if (1 < count && _chestFont && _page->number(Common::String::format(kDrawNumbersUnsignedIndexFormat, id))) {
			Shell::TextObject *label = new Shell::TextObject(_chestPaths, _vm.fonts(), _chestFont);
			label->create({Core::ScriptValue::fromText(Common::String::format("%d", count)), Core::ScriptValue(item->x + 10),
						   Core::ScriptValue(item->y + 10), Core::ScriptValue(30), Core::ScriptValue(14), Core::ScriptValue(0)});
			label->set(kPropertyColor, {Core::ScriptValue(500), Core::ScriptValue(450)}, Core::ScriptValue(7500));
			label->tag = id;
			_itemCounts.push_back(label);
		}
	}
	if (_world.get(kShowInventoryOutlinesKey)) {
		for (uint id = 1; id <= 3; id++) {
			if (_world.get(Common::String::format(kItemUnsignedIndexKey, id)) || _world.get(Common::String(kR1LN2Key) + partName(id) + kDeliveredKey))
				continue;
			Common::String entry = Common::String::format(kCollectiblesUnsignedIndexFormat, id);
			Shell::Sprite *outline = _scene.add(Common::String::format(kInventoryAnimationFormat, _page->number(entry + k4Name)), 202, -static_cast<int>(id));
			if (outline) {
				outline->x = _page->number(entry + kInventoryXField);
				outline->y = _page->number(entry + kInventoryYField) - offset;
				outline->boxTouchy = true;
				_items.push_back(outline);
			}
		}
	}
	Common::Point position = _chestOffset;
	_chestOffset = Common::Point();
	moveChest(position);
}

bool PageScene::chestContains(Common::Point point) const {
	int offset = _page->number(kChesterYOffsetKey);
	return Common::Rect(297, 29 - offset, 618, 312 - offset).contains(point - _chestOffset);
}

void PageScene::moveChest(Common::Point delta) {
	_chestOffset += delta;
	for (auto *layer : {_chestLid, _chestTray, _chestButton}) {
		if (layer) {
			layer->x += delta.x;
			layer->y += delta.y;
		}
	}
	for (auto *item : _items) {
		item->x += delta.x;
		item->y += delta.y;
	}
	for (auto *label : _itemCounts) {
		label->x += delta.x;
		label->y += delta.y;
	}
}

void PageScene::closeChest(bool restoreCharacter) {
	if (_chestPhase != kChestOpen)
		return;
	_restoreChester = restoreCharacter;
	_draggingChest = false;
	_dragged = nullptr;
	_chestOpen = false;
	_chestPhase = kChestClosing;
	if (_chestButton)
		_chestButton->visible = false;
	_scene.remove(_chestLid);
	_chestLid = _scene.add(kChestOpenLid, 211);
	if (!_chestLid) {
		finishChest();
		return;
	}
	_chestLid->startAnimation(1);
	_chestLid->x += _chestOffset.x;
	_chestLid->y += _chestOffset.y - _page->number(kChesterYOffsetKey);
	playSounds({7003});
}

void PageScene::finishChest() {
	_dragged = nullptr;
	_draggingChest = false;
	for (Shell::Sprite *item : _items)
		_scene.remove(item);
	_items.clear();
	for (auto *label : _itemCounts)
		delete label;
	_itemCounts.clear();
	if (_chestLid)
		_scene.remove(_chestLid);
	if (_chestTray)
		_scene.remove(_chestTray);
	if (_chestButton)
		_scene.remove(_chestButton);
	_chestButton = nullptr;
	_chestLid = _chestTray = nullptr;
	_chestOpen = false;
	_chestPhase = kChestClosed;
	_putAwayActive = false;
	if (_chester && _restoreChester)
		_chester->visible = true;
}

void PageScene::updateChest(uint32 time) {
	if (_chestPhase == kChestCharacter && !_chestMotion->isRunning()) {
		_scene.remove(_chestMotion);
		_chestMotion = nullptr;
		openChest();
	} else if (_chestPhase == kChestOpening && !_chestLid->isRunning()) {
		_chestLid->setFrame(6);
		_chestPhase = kChestOpen;
		_chestOpen = true;
		if (_chestButton)
			_chestButton->visible = true;
	} else if (_chestPhase == kChestClosing && !_chestLid->isRunning()) {
		finishChest();
	} else if (_chestPhase == kChestOpen && _putAwayActive) {
		closeChest();
	}
	if (_highlight && static_cast<int32>(time - _highlightDue) >= 0) {
		_highlight->setFrame(_highlight->getFrame() == 1 ? 2 : 1);
		_highlightDue = time + 100;
	}
}

void PageScene::drawInventoryCounts(OM::Canvas &canvas) {
	if (!_chestOpen)
		return;
	for (auto *label : _itemCounts) {
		if (!_dragged || _dragged->action != label->tag)
			label->draw(canvas);
	}
}

bool PageScene::event(const Common::Event &event) {
	if (chestBusy() || _putAwayPending || _putAwayActive)
		return true;
	if (_draggingChest) {
		if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONUP) {
			moveChest(event.mouse - _chestDragPoint);
			_chestDragPoint = event.mouse;
			if (event.type == Common::EVENT_LBUTTONUP)
				_draggingChest = false;
		}
		return true;
	}
	if (_rainGame && _rainGame->event(event))
		return true;
	if (_musicPatterns && !_chestOpen && _musicPatterns->event(event))
		return true;
	if (_flipbook && _flipbook->event(event))
		return true;
	if (_gallery && !_chestOpen)
		_gallery->event(event);
	if (_cloudFloors && !_chestOpen && _cloudFloors->event(event))
		return true;
	if (_wordPots && !_chestOpen && _wordPots->event(event))
		return true;
	if (_numberTree && !_chestOpen && _numberTree->event(event))
		return true;
	if (_sandwich && !_chestOpen && _sandwich->event(event))
		return true;
	if (_gumball && !_chestOpen && event.type == Common::EVENT_MOUSEMOVE)
		_gumball->hover(event.mouse);
	if (!_dragged)
		return false;
	if (event.type == Common::EVENT_MOUSEMOVE) {
		_dragged->x = event.mouse.x - _dragOffset.x;
		_dragged->y = event.mouse.y - _dragOffset.y;
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_catapult && !chestContains(event.mouse) && _catapult->donate(_dragged->action))
			return true;
		if (_sheepCrossing && !chestContains(event.mouse) && _sheepCrossing->donate(_dragged->action))
			return true;
		if (_rockHead && !chestContains(event.mouse) && _rockHead->donate(_dragged->action))
			return true;
		if (_rainbowSlide && !chestContains(event.mouse) && _rainbowSlide->donate(_dragged->action))
			return true;
		if (_musicPatterns && !chestContains(event.mouse) && _musicPatterns->donate(_dragged->action))
			return true;
		if (_name.equals(Page::kPageLN2) && !chestContains(event.mouse)) {
			uint item = _dragged->action;
			_dragged->x = _dragHome.x;
			_dragged->y = _dragHome.y;
			_dragged = nullptr;
			inventorDrop(item);
			return true;
		}
		if (_name.equals(Page::kPageCG1) && _dragged->action == 4 && !chestContains(event.mouse)) {
			_world.set(kItem4Key, MAX(0, _world.get(kItem4Key) - 1));
			closeChest();
			interrupt();
			_world.set(kR1CG1OpeningKey, 1);
			_openingGate = 3;
			_gateDelayStarted = false;
			return true;
		}
		_dragged->x = _dragHome.x;
		_dragged->y = _dragHome.y;
		_dragged = nullptr;
	}
	return true;
}

bool PageScene::blocksMap() const {
	return chestBusy() || _putAwayPending || _putAwayActive || sceneBusy();
}

void PageScene::releaseNavigationCursor() {
	if (0 <= _navCursor) {
		CursorMan.popCursor();
		_navCursor = -1;
	}
}

void PageScene::updateNavigationCursor() {
	releaseNavigationCursor();
	if (!_page || _paused || _practice || _chestOpen || _dragged || _draggingChest || _openingGate || blocksMap())
		return;
	if (_flipbook && _flipbook->hasCustomCursor())
		return;
	if (_cloudFloors && _cloudFloors->hasCustomCursor())
		return;
	if (_scene.hitSprite(_mouse, _page->number(kNavHotspotZKey) + 1, INT_MAX, true))
		return;
	for (const auto &actor : _actors) {
		if (actor._value->visible && actor._value->bounds().contains(_mouse))
			return;
	}
	for (const auto &exit : _page->exits()) {
		if (!exit.rect.contains(_mouse) || exit.cursor < 4066 || 4073 < exit.cursor)
			continue;
		int index = exit.cursor - 4066;
		Graphics::Surface &cursor = _navCursors[index];
		if (!cursor.getPixels()) {
			Common::String path = Common::String::format(kNavigationCursorFormat, exit.cursor);
			const Core::Animation *animation = _scene.loadAnimation(path);
			if (!animation || !animation->picture(0))
				return;
			cursor.copyFrom(*animation->picture(0));
		}
		static const Common::Point hotspots[] = {
			Common::Point(16, 1),
			Common::Point(30, 13),
			Common::Point(1, 13),
			Common::Point(17, 31),
			Common::Point(3, 1),
			Common::Point(28, 1),
			Common::Point(1, 28),
			Common::Point(30, 28),
		};
		CursorMan.pushCursor(cursor, hotspots[index].x, hotspots[index].y, 0);
		_navCursor = index;
		return;
	}
}

bool PageScene::sceneBusy() const {
	if (_queenArriving)
		return true;
	if (_rainGame)
		return _rainGame->ending();
	if (_skyRide)
		return true;
	if (_catapult && _catapult->busy())
		return true;
	if (_sheepCrossing && _sheepCrossing->busy())
		return true;
	if (_rockHead && _rockHead->busy())
		return true;
	if (_rainbowSlide && _rainbowSlide->busy())
		return true;
	if (_musicPatterns && _musicPatterns->busy())
		return true;
	return _queenPhase != kQueenIdle || _inventorPhase != kInventorIdle || (_gumball && _gumball->busy()) || (_sandwich && _sandwich->busy()) ||
		   (_numberTree && _numberTree->busy()) || (_wordPots && _wordPots->busy()) || (_cloudFloors && _cloudFloors->busy()) || (_gallery && _gallery->busy()) || (_flipbook && _flipbook->busy());
}

Common::String PageScene::destination() const {
	if (_rainGame)
		return _rainGame->destination();
	if (_skyRide)
		return _skyRide->destination();
	if (_catapult)
		return _catapult->destination();
	if (_sheepCrossing)
		return _sheepCrossing->destination();
	return _rainbowSlide ? _rainbowSlide->destination() : Common::String();
}

bool PageScene::wantsGalleryMovie() const { return _gallery && _gallery->wantsMovie(); }

int PageScene::galleryDestination() const { return _gallery ? _gallery->destination() : 0; }

void PageScene::checkpoint() const {
	if (_flipbook)
		_flipbook->checkpoint();
}

void PageScene::draw(OM::Canvas &canvas) {
	drawPage(canvas);
	drawInventoryCounts(canvas);
}

void PageScene::drawPage(OM::Canvas &canvas) {
	if (_musicPatterns) {
		_musicPatterns->draw(canvas);
		return;
	}
	if (_rainGame) {
		_rainGame->draw(canvas);
		return;
	}
	if (_flipbook) {
		_flipbook->draw(canvas);
		return;
	}
	if (_gallery) {
		_gallery->draw(canvas);
		return;
	}
	if (_cloudFloors) {
		_cloudFloors->draw(canvas);
		return;
	}
	if (_wordPots) {
		_wordPots->draw(canvas);
		return;
	}
	if (_numberTree) {
		_numberTree->draw(canvas);
		return;
	}
	if (_sandwich) {
		_sandwich->draw(canvas);
		return;
	}
	if (!_gumball) {
		_scene.draw(canvas);
		return;
	}
	_scene.drawBackground(canvas);
	_scene.drawSprites(canvas, INT_MIN, 100);
	_gumball->draw(canvas);
	_scene.drawSprites(canvas, 101, INT_MAX);
}

} // namespace RR1Cloud9
} // namespace Trinket
