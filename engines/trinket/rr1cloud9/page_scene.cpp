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

PageScene::PageScene(TrinketEngine &vm, Shell::Scene &scene, World &world) : _vm(vm), _scene(scene), _world(world), _chestPaths(vm.resources()) {}

void PageScene::clear() {
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
	_arrivalIdleIndex = 0;
}

Common::String PageScene::characterResource(uint resource, Kind kind, uint part) const {
	const char *extension = kind == kAnimation ? "ao" : kind == kSound ? "wav"
													: kind == kLips    ? "lips"
																	   : "tc";
	Common::String filename = Common::String::format("%u.%s", resource, extension);
	if (kind == kLips && part != 1)
		return Common::String();
	Common::Array<Common::String> libraries = {_name, _name + "spch", _name + "sfx", Common::String("common")};
	if (_name == "r1la2")
		libraries.insert_at(3, _name + "tunz");
	if (_name == "r1ia2" || _name == "r1ia3" || _name == "r1ca1")
		libraries.insert_at(2, _name + "words");
	if (_name == "r1la1") {
		libraries.insert_at(1, _name + "spch1");
		libraries.insert_at(2, _name + "spch2");
		libraries.insert_at(3, _name + "words");
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
	uint settle = _page->number(prefix + "SetID");
	if (!settle)
		settle = _page->number(prefix + "LipsID");
	if (!settle)
		return false;
	Common::ScopedPtr<Shell::Character> character(new Shell::Character(_scene, _vm.mixer(), _vm.resources(), _vm.rnd(), *this));
	if (!character->create(settle, _page->number(prefix + "Z")))
		return false;
	character->movable = false;
	Core::ScriptValue result;
	character->set("registerFidgetsToSettle", {}, Core::ScriptValue(0));
	character->set("registerSpeechToSettle", {}, Core::ScriptValue(0));
	character->set("loadSpeechAnimsInBkgd", {}, Core::ScriptValue(0));
	int lips = _page->number(prefix + "LipsID");
	if (lips) {
		Core::ScriptArguments parts = {Core::ScriptValue(lips), Core::ScriptValue(1), Core::ScriptValue(1)};
		int bodyID = body ? _page->number(prefix + "BodyID") : 0;
		if (bodyID) {
			parts.push_back(Core::ScriptValue(bodyID));
			parts.push_back(Core::ScriptValue(0));
			parts.push_back(Core::ScriptValue(0));
		}
		character->play("setSpeechAnims", parts, result);
	}
	character->play("addFidgetGroup", {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	int blink = prefix == "kSpeak" || prefix == "kQueen" ? _page->number(prefix + "Fidg1ID") : settle;
	character->play("addFidget", {Core::ScriptValue(1), Core::ScriptValue(blink)}, result);
	if (body) {
		bool queen = prefix == "kQueen";
		character->play("addFidgetGroup", {Core::ScriptValue(2), Core::ScriptValue(queen ? 5 : 15), Core::ScriptValue(queen ? 10 : 30)}, result);
		for (uint i = queen ? 2 : 1; i <= 2; i++) {
			int id = _page->number(Common::String::format("%sFidg%uID", prefix.c_str(), i));
			if (id)
				character->play("addFidget", {Core::ScriptValue(2), Core::ScriptValue(id)}, result);
		}
	}
	_actors[name] = character.release();
	return true;
}

void PageScene::say(const Common::String &actorName, uint resource) {
	if (resource && _actors.contains(actorName)) {
		if (_name == "r1cn2" || _name == "r1cn1" || _name == "r1la1")
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
		group = _vm.rnd().number(1, _page->number("kMaxSpeakSpchs"));
	Common::String suffix = Common::String::format(".%u", group);
	for (uint i = 1; i <= static_cast<uint>(_page->number("kMaxSpeakIdleSpchs" + suffix)); i++)
		say("speak", _page->number("kSpeakIdleSpchsID" + suffix + Common::String::format(".%u", i)));
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
	_arrivalIdleIndex = _arrivalIdleIndex % MAX(1, _page->number("kMaxIdleIndex") * 3) + 1;
	arrivalSpeaker();
	if (_arrivalIdleIndex % 3 == 2) {
		Common::String group = Common::String::format(".%u", _vm.rnd().number(1, 2));
		for (uint i = 1; i <= static_cast<uint>(_page->number("kMaxIdleIndexSpeeches" + group)); i++) {
			delaySpeech(1500);
			Common::String suffix = group + Common::String::format(".%u", i);
			say(_page->text("kIdleCharID" + suffix), _page->number("kIdleSpchID" + suffix));
		}
	}
}

void PageScene::speeches(const Common::String &actors, const Common::String &resources, uint count) {
	for (uint i = 1; i <= count; i++) {
		Common::String suffix = Common::String::format(".%u", i);
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
		_world.set("R1CN1BellClick", 1);
		_world.set("R1CN1BellPending", 0);
		_world.set("item:4", MIN(25, _world.get("item:4") + 1));
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
	if (_name == "r1cn1" || _name == "r1la1")
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
	if (name == "r1la3") {
		if (!actor("reader", "kReader") || !actor("sam", "kSam") || !actor("paige", "kPaige", false) ||
			!_scene.add("r1map/1852.ao", 100, 1, true))
			return false;
		_rainGame = new RainGame(*this);
		return _rainGame->enter();
	}
	if (name == "r1rss") {
		_skyRide = new SkyRide(*this);
		return _skyRide->enter();
	}
	if (name == "r1ig1") {
		if (!_scene.add("r1map/1852.ao", 100, 1, true))
			return false;
		_rockHead = new RockHead(*this);
		return _rockHead->enter();
	}
	if (name != "r1cn1" && name != "r1cg1" && name != "r1cn2" && name != "r1in4" && name != "r1in1" && name != "r1ln2" &&
		name != "r1la1" && name != "r1ia2" && name != "r1ia1" && name != "r1ia3" && name != "r1ca1" && name != "r1cn3" && name != "r1ca2" && name != "r1la2" && name != "r1cn4" && name != "r1cg2" && name != "r1lg2")
		return true;
	if (!actor("reader", "kReader") || !actor("sam", "kSam") || !actor("paige", "kPaige", false))
		return false;
	if (name != "r1ca2")
		_chester = _scene.add(characterResource(page.number("gChesterFidgID"), kAnimation), name == "r1cn3" ? page.number("kChesterZ") : page.number("gClosedChesterZ"));
	if (!_scene.add("r1map/1852.ao", 100, 1, true))
		return false;
	if (name == "r1lg2") {
		_catapult = new Catapult(*this);
		return _catapult->enter();
	}
	if (name == "r1cg2") {
		_sheepCrossing = new SheepCrossing(*this);
		return _sheepCrossing->enter();
	}
	if (name == "r1cn4") {
		_rainbowSlide = new RainbowSlide(*this);
		return _rainbowSlide->enter();
	}
	if (name == "r1la1") {
		_gumball = new Gumball(*this);
		return _gumball->enter();
	}
	if (name == "r1la2") {
		_musicPatterns = new MusicPatterns(*this);
		return _musicPatterns->enter();
	}
	if (name == "r1ia3") {
		_wordPots = new WordPots(*this);
		return _wordPots->enter();
	}
	if (name == "r1ca2") {
		_flipbook = new Flipbook(*this);
		return _flipbook->enter();
	}
	if (name == "r1cn3") {
		_gallery = new Gallery(*this);
		return _gallery->enter();
	}
	if (name == "r1ca1") {
		_cloudFloors = new CloudFloors(*this);
		return _cloudFloors->enter();
	}
	if (name == "r1ia1") {
		_numberTree = new NumberTree(*this);
		return _numberTree->enter();
	}
	if (name == "r1ia2") {
		_sandwich = new Sandwich(*this);
		return _sandwich->enter();
	}
	if (name == "r1cn2")
		return enterQueen();
	if (name == "r1in4")
		return enterHub();
	if (name == "r1ln2")
		return enterInventor();
	if (name == "r1in1") {
		if (!_world.visits(name))
			speeches("kEnterCharID", "kEnterSpchID", 2);
		return true;
	}
	if (name == "r1cg1") {
		int state = MAX(1, _world.get("R1CG1State"));
		if (state == 1 && _world.get("item:4"))
			state = 2;
		_world.set("R1CG1State", state);
		_gate = _scene.add(characterResource(page.number("kOpenGatesID"), kAnimation), page.number("kFenceZ"));
		_bell = _scene.add(characterResource(page.number("kUsedBellID"), kAnimation), page.number("kBellZ"));
		if (!_gate || !_bell)
			return false;
		_gate->setFrame(1);
		_bell->visible = state == 3;
		if (state == 3)
			_gate->setFrame(_gate->getNumberOfFrames());
		if (_world.get("R1CG1Opening"))
			_openingGate = 3;
		else if (!_world.visits(name))
			speeches("kEnterCharID", "kEnterSpchID", page.number("kMaxEntrySpchs"));
		else if (state < 3) {
			Common::String suffix = Common::String::format(".%d", state);
			speeches("kReEnterCharIDs" + suffix, "kReEnterSpchIDs" + suffix, page.number("kReEnterMaxSpchs" + suffix));
		}
		return true;
	}
	if (!actor("speak", "kSpeak", false))
		return false;
	_bell = _scene.add(characterResource(page.number("kBellID"), kAnimation), page.number("kBellZ"));
	_cloud = _scene.add(characterResource(page.number("kCloudMaskID"), kAnimation), page.number("kCloudMaskZ"));
	if (!_bell || !_cloud)
		return false;
	_bell->visible = !_world.get("R1CN1BellClick");
	_cloud->visible = !_world.visits("r1cn2");
	if (_world.get("R1CN1BellPending")) {
		_collectBell = true;
		delaySpeech(450);
	} else if (!_world.visits(name)) {
		speeches("kEnterCharID", "kEnterSpchID", page.number("kMaxEntrySpchs"));
		delaySpeech(500);
		arrivalSpeaker(1);
	} else {
		if (_world.visits(name) == 1)
			say("reader", page.number("kReEnterSpchIDs.1.1"));
		if (_world.visits("r1cn2") && _world.get("R1CN1State") < 3) {
			_world.set("R1CN1State", 3);
			_cloud->visible = true;
			_clearCloud = true;
			_cloudSpeechIndex = _speech.size();
			_cloudOrigin = Common::Point(_cloud->x, _cloud->y);
			say("sam", page.number("kReEnterSpchIDs.2.1"));
		}
	}
	return true;
}

void PageScene::pause(bool paused, uint32 time) {
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
	for (auto *sprite : {_chester, _chestMotion, _chestLid})
		if (sprite)
			sprite->pauseAnimation(paused);
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
	if (_name == "r1in4")
		updateHub(time);
	if (_name == "r1ln2")
		updateInventorAmbient(time);
	if (_name == "r1cn2")
		updateQueenAmbient(time);
	Common::Array<uint16> cues;
	for (auto &actor : _actors)
		actor._value->update(time, cues);
	playSounds(cues);
	if (_queenArriving) {
		if (_actors["reader"]->isBusy() || _actors["sam"]->isBusy())
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
	if (_name == "r1cn2")
		updateQueen();
	if (_name == "r1ln2")
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
			_world.set("R1CG1State", 3);
			_world.set("R1CG1Opening", 0);
			_actors["reader"]->setCallTime(time);
			_actors["reader"]->playAnim(_page->number("kRingBellID"), 1, false);
			_openingGate = 1;
		} else {
			_openingGate = 0;
			_placeBell = true;
			speeches("kGateOpenCharID", "kGateOpenSpchID", 2);
		}
	}
	if (_openingGate == 1) {
		if (_actors["reader"]->isBusy())
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
	if (_clearCloud && _nextSpeech == _cloudSpeechIndex) {
		if (!_cloudMoving) {
			_cloudStart = time;
			_cloudMoving = true;
			OM::Sound *sound = new OM::Sound(_vm.mixer());
			sound->play(_vm.resources(), characterResource(_page->number("kCloudSFXID"), kSound), Audio::Mixer::kSFXSoundType);
			_sounds.push_back(sound);
		}
		uint frames = MAX(1, _page->number("kTargetMoveFrames"));
		uint frame = MIN<uint>(frames, static_cast<uint64>(time - _cloudStart) * _page->number("kTargetMoveFPS") / 1000);
		_cloud->x = _cloudOrigin.x + _page->number("kTargetMoveOffsetX") * frame / frames;
		_cloud->y = _cloudOrigin.y + _page->number("kTargetMoveOffsetY") * frame / frames;
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
		if (_name == "r1cn1")
			arrivalIdle(time);
		if (_name == "r1cn2")
			queenIdle(time);
		if (_name == "r1in4")
			hubIdle(time);
		if (_name == "r1ln2")
			inventorIdle(time);
	}
}

Common::String PageScene::click(Common::Point point) {
	if (chestBusy() || _putAwayPending || _putAwayActive || _queenArriving)
		return "handled";
	if (_rainGame) {
		Shell::Sprite *map = _scene.hitSprite(point, 100, 100, true);
		if (map && map->action == 1 && !_rainGame->ending())
			return "map";
		_rainGame->click(point);
		return "handled";
	}
	if (_skyRide)
		return "handled";
	if (_catapult && _catapult->busy()) {
		interrupt();
		return "handled";
	}
	if (_sheepCrossing && _sheepCrossing->busy()) {
		interrupt();
		return "handled";
	}
	if (_rockHead && _rockHead->busy()) {
		interrupt();
		return "handled";
	}
	if (_rainbowSlide && _rainbowSlide->busy()) {
		interrupt();
		return "handled";
	}
	if (_musicPatterns && _musicPatterns->busy()) {
		_musicPatterns->skipSpeech();
		return "handled";
	}
	if (_flipbook && _flipbook->busy())
		return "handled";
	if (_gallery && _gallery->busy()) {
		interrupt();
		return "handled";
	}
	if (_cloudFloors && _cloudFloors->busy()) {
		interrupt();
		return "handled";
	}
	if (_wordPots && _wordPots->busy()) {
		_wordPots->skipSpeech();
		return "handled";
	}
	if (_numberTree && _numberTree->busy()) {
		_numberTree->skipSpeech();
		return "handled";
	}
	if (_sandwich && _sandwich->busy()) {
		_sandwich->skipSpeech();
		return "handled";
	}
	if (_gumball && _gumball->busy()) {
		_gumball->skipSpeech();
		return "handled";
	}
	if (_name != "r1cn1" && _name != "r1cg1" && _name != "r1cn2" && _name != "r1in4" && _name != "r1in1" && _name != "r1ln2" &&
		!_gumball && !_sandwich && !_numberTree && !_wordPots && !_cloudFloors && !_gallery && !_flipbook && !_musicPatterns && !_rainbowSlide && !_rockHead && !_sheepCrossing && !_catapult)
		return Common::String();
	if (_inventorPhase != kInventorIdle) {
		if (_inventorPhase == kInventorIntro || _inventorPhase == kInventorDeliverySpeech || _inventorPhase == kInventorMoreItems)
			interrupt();
		return "handled";
	}
	if (_queenPhase != kQueenIdle) {
		interrupt();
		return "handled";
	}
	if (_openingGate)
		return "handled";
	if (_chestOpen) {
		for (Shell::Sprite *item : _items) {
			if (!item->action || !item->covers(point))
				continue;
			if (item->action < 0) {
				interrupt();
				uint id = -item->action;
				say(_page->text(Common::String::format("kOutlineSpchChar.%u", id)), _page->number(Common::String::format("kOutlineSpchID.%u", id)));
				return "handled";
			}
			_dragged = item;
			_dragOffset = point - Common::Point(item->x, item->y);
			_dragHome = Common::Point(item->x, item->y);
			return "handled";
		}
		if (!chestContains(point) || (_chestButton && _chestButton->covers(point))) {
			playSounds({7002});
			closeChest();
		} else {
			_draggingChest = true;
			_chestDragPoint = point;
		}
		return "handled";
	}
	if (!_practice && _chester && _chester->covers(point)) {
		interrupt();
		openChest(true);
		return "handled";
	}
	Shell::Sprite *sprite = _scene.hitSprite(point, 100, 100, true);
	if (sprite && sprite->action == 1)
		return "map";
	interrupt();
	if (_catapult)
		return _catapult->click(point) ? "handled" : Common::String();
	if (_sheepCrossing)
		return _sheepCrossing->click(point) ? "handled" : Common::String();
	if (_rockHead)
		return _rockHead->click(point) ? "handled" : Common::String();
	if (_rainbowSlide)
		return _rainbowSlide->click(point) ? "handled" : Common::String();
	if (_musicPatterns)
		return _musicPatterns->click(point) ? "handled" : Common::String();
	if (_flipbook)
		return _flipbook->click(point) ? "handled" : Common::String();
	if (_gallery)
		return _gallery->click(point) ? "handled" : Common::String();
	if (_numberTree)
		return _numberTree->click(point) ? "handled" : Common::String();
	if (_cloudFloors)
		return _cloudFloors->click(point) ? "handled" : Common::String();
	if (_wordPots)
		return _wordPots->click(point) ? "handled" : Common::String();
	if (_sandwich)
		return _sandwich->click(point) ? "handled" : Common::String();
	if (_gumball)
		return _gumball->click(point) ? "handled" : Common::String();
	if (_name == "r1cg1")
		return gateClick(point);
	if (_name == "r1cn2")
		return queenClick(point);
	if (_name == "r1in4")
		return hubClick(point);
	if (_name == "r1in1")
		return groveClick(point);
	if (_name == "r1ln2")
		return inventorClick(point);
	if (_bell && _bell->visible && Common::Rect(_page->number("kBellHSX1"), _page->number("kBellHSY1"), _page->number("kBellHSX2"), _page->number("kBellHSY2")).contains(point)) {
		Common::String group = _world.visits("r1cg1") ? ".2" : ".1";
		_world.set("R1CN1BellClick", 1);
		_world.set("R1CN1BellPending", 1);
		playSounds({1802});
		delaySpeech(450);
		speeches("kBellSpchCharIDs" + group, "kBellSpchIDs" + group, _page->number("kMaxBellSpchs" + group));
		delaySpeech(450);
		_collectBell = true;
		return "handled";
	}
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key == "speak") {
			arrivalSpeaker();
		} else if (actor._key == "paige") {
			Common::String group = _world.get("R1CN1BellClick") ? ".2" : ".1";
			speeches("kHelpSpchCharIDs" + group, "kHelpSpchIDs" + group, 2);
		} else {
			uint choice = _world.get("R1CN1BellClick") ? _vm.rnd().number(2, 3) : 1;
			Common::String group = Common::String::format(".%u", choice);
			speeches("kMainSpchCharIDs" + group, "kMainSpchIDs" + group, _page->number("kMaxMainSpchs" + group));
		}
		return "handled";
	}
	if (!_practice && !_world.visits("r1cn2") && 350 <= point.x) {
		uint choice = _vm.rnd().number(1, 2);
		Common::String suffix = Common::String::format(".%u", choice);
		say(_page->text("kDontExitSpchCharIDs" + suffix), _page->number("kDontExitSpchIDs" + suffix));
		return "handled";
	}
	return Common::String();
}

Common::String PageScene::gateClick(Common::Point point) {
	int state = _world.get("R1CG1State");
	if (_bell && _bell->visible && _bell->covers(point)) {
		say(_page->text("kBellCharID"), _page->number("kBellSpchID"));
		return "handled";
	}
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		bool paige = actor._key == "paige";
		Common::String key = paige ? "R1CG1PaigeClicks" : "R1CG1MainClicks";
		int count = _world.get(key);
		int choice = count ? (paige ? _vm.rnd().number(4, 5) : 4) : state;
		Common::String suffix = Common::String::format(".%d", choice);
		speeches((paige ? "kHelpSpchCharIDs" : "kMainSpchCharIDs") + suffix,
				 (paige ? "kHelpSpchIDs" : "kMainSpchIDs") + suffix, _page->number((paige ? "kMaxHelpSpchs" : "kMaxMainSpchs") + suffix));
		_world.set(key, count + 1);
		return "handled";
	}
	for (uint i = 1; i <= 2; i++) {
		Common::String suffix = Common::String::format(".%u", i);
		Common::Rect rect(_page->number("kSignHSX1" + suffix), _page->number("kSignHSY1" + suffix),
						  _page->number("kSignHSX2" + suffix), _page->number("kSignHSY2" + suffix));
		if (!rect.contains(point))
			continue;
		if (i == 1) {
			say(_page->text("kBigSignCharIDs.1"), _page->number("kBigSignIDs.1"));
			if (!_world.get("R1CG1BigSignClicks"))
				say(_page->text("kBigSignCharIDs.2"), _page->number("kBigSignIDs.2"));
			_world.set("R1CG1BigSignClicks", _world.get("R1CG1BigSignClicks") + 1);
		} else {
			speeches("kSmSignCharIDs.1", "kSmSignIDs.1", _page->number("kMaxSmSignSpchs.1"));
			if (state < 3) {
				Common::String group = Common::String::format(".%d", state == 1 ? _vm.rnd().number(2, 3) : 4);
				speeches("kSmSignCharIDs" + group, "kSmSignIDs" + group, _page->number("kMaxSmSignSpchs" + group));
			}
		}
		return "handled";
	}
	if (!_practice && state < 3) {
		for (const auto &exit : _page->exits()) {
			if (exit.destination == "r1cn1" || !exit.rect.contains(point))
				continue;
			Common::String suffix = Common::String::format(".%d", _vm.rnd().number(1, 2));
			say(_page->text("kDontExitSpchCharIDs" + suffix), _page->number("kDontExitSpchIDs" + suffix));
			return "handled";
		}
	}
	return Common::String();
}

void PageScene::openChest(bool animateCharacter) {
	if (_chestPhase != kChestClosed && _chestPhase != kChestCharacter)
		return;
	if (_name == "r1cn2")
		_idleWaiting = false;
	if (animateCharacter) {
		_chestMotion = _scene.add(characterResource(_page->number("gChesterOpenID"), kAnimation), _page->number("gClosedChesterZ"));
		if (_chestMotion) {
			_chestMotion->startAnimation(1);
			_chestPhase = kChestCharacter;
			if (_chester)
				_chester->visible = false;
			return;
		}
	}
	_chestLid = _scene.add("common/7001.ao", 211);
	_chestTray = _scene.add("common/7004.ao", 200);
	_chestButton = _scene.add("common/7002.ao", 221);
	if (!_chestLid || !_chestTray) {
		finishChest();
		return;
	}
	_chestPhase = kChestOpening;
	_restoreChester = true;
	_chestLid->startAnimation(1);
	playSounds({7001});
	int offset = _page->number("kChesterYOffset");
	_chestLid->y -= offset;
	_chestTray->y -= offset;
	if (_chestButton) {
		_chestButton->y -= offset;
		_chestButton->visible = false;
	}
	if (_chester)
		_chester->visible = false;
	if (!_chestFont) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_vm.resources().open("common/1018.nfnt"));
		if (stream)
			_chestFont.reset(OM::NFNTFont::load(*stream));
	}
	for (uint id = 1; id <= 17; id++) {
		int count = _world.get(Common::String::format("item:%u", id));
		if (!count)
			continue;
		Common::String entry = Common::String::format("collectibles.%u.", id);
		int resource = _page->number(entry + "1");
		if (!resource)
			continue;
		Shell::Sprite *item = _scene.add(Common::String::format("r1inventory/%d.ao", resource), 202, id);
		if (!item)
			continue;
		item->x = _page->number(entry + "2");
		item->y = _page->number(entry + "3") - offset;
		item->boxTouchy = id == 2 || id == 6 || id == 8;
		if (1 < count) {
			Shell::Sprite *underneath = _scene.add(Common::String::format("r1inventory/%d.ao", resource), 201);
			if (underneath) {
				underneath->x = item->x;
				underneath->y = item->y;
				_items.push_back(underneath);
			}
		}
		_items.push_back(item);
		if (1 < count && _chestFont && _page->number(Common::String::format("drawNumbers.%u", id))) {
			Shell::TextObject *label = new Shell::TextObject(_chestPaths, _chestFont);
			label->create({Core::ScriptValue::fromText(Common::String::format("%d", count)), Core::ScriptValue(item->x + 10),
						   Core::ScriptValue(item->y + 10), Core::ScriptValue(30), Core::ScriptValue(14), Core::ScriptValue(0)});
			label->set("color", {Core::ScriptValue(500), Core::ScriptValue(450)}, Core::ScriptValue(7500));
			label->tag = id;
			_itemCounts.push_back(label);
		}
	}
	if (_world.get("ShowInventoryOutlines")) {
		for (uint id = 1; id <= 3; id++) {
			if (_world.get(Common::String::format("item:%u", id)) || _world.get(Common::String("R1LN2") + partName(id) + "Delivered"))
				continue;
			Common::String entry = Common::String::format("collectibles.%u.", id);
			Shell::Sprite *outline = _scene.add(Common::String::format("r1inventory/%d.ao", _page->number(entry + "4")), 202, -static_cast<int>(id));
			if (outline) {
				outline->x = _page->number(entry + "2");
				outline->y = _page->number(entry + "3") - offset;
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
	int offset = _page->number("kChesterYOffset");
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
	_chestLid = _scene.add("common/7003.ao", 211);
	if (!_chestLid) {
		finishChest();
		return;
	}
	_chestLid->startAnimation(1);
	_chestLid->x += _chestOffset.x;
	_chestLid->y += _chestOffset.y - _page->number("kChesterYOffset");
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
	for (auto *label : _itemCounts)
		if (!_dragged || _dragged->action != label->tag)
			label->draw(canvas);
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
		if (_name == "r1ln2" && !chestContains(event.mouse)) {
			uint item = _dragged->action;
			_dragged->x = _dragHome.x;
			_dragged->y = _dragHome.y;
			_dragged = nullptr;
			inventorDrop(item);
			return true;
		}
		if (_name == "r1cg1" && _dragged->action == 4 && !chestContains(event.mouse)) {
			_world.set("item:4", MAX(0, _world.get("item:4") - 1));
			closeChest();
			interrupt();
			_world.set("R1CG1Opening", 1);
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
