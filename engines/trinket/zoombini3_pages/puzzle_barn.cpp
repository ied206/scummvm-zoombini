/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_pages/puzzle_barn.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "trinket/shell/character_action.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

BarnRules::BarnRules() = default;

Common::Point BarnRules::home(uint index) {
	static constexpr int x[2][6] = {{496, 614, 436, 553, 658, 479}, {70, 154, 136, 81, 57, 174}};
	static constexpr int y[2][6] = {{117, 180, 124, 160, 145, 180}, {401, 389, 469, 503, 555, 540}};
	return index < 12 ? Common::Point(x[index % 2][index / 2], y[index % 2][index / 2]) : Common::Point();
}

bool BarnRules::generate(uint level, uint party, Shell::Random &random, const World::Token *tokens, const int *tags, Generator *generator) {
	if (level < 1 || 3 < level || party < 2 || 12 < party)
		return false;
	Common::Array<Input> inputs;
	for (uint i = 0; i < party; i++) {
		Input input;
		input.tag = tags ? tags[i] : -1;
		for (uint a = 0; a < 3; a++)
			input.traits[a] = tokens ? tokens[i].attributes[a] : -1;
		inputs.push_back(input);
	}
	static constexpr uint limits[6][3] = {{1, 1, 1}, {1, 1, 1}, {2, 2, 1}, {4, 4, 3}, {6, 6, 5}, {8, 8, 6}};
	return generatePuzzle(party / 2, level == 3 ? 2 : 1, level == 1, limits[party / 2 - 1][level - 1], random, inputs, generator);
}

bool BarnRules::generatePuzzle(uint pairs, uint trait, bool showGenes, uint limit, Shell::Random &random, const Common::Array<Input> &inputs,
							   Generator *generator) {
	if (pairs < 1 || 6 < pairs || trait < 1 || 2 < trait || limit < 1)
		return false;
	_level = trait == 2 ? 3 : (showGenes ? 1 : 2);
	_trait = trait;
	_showGenes = showGenes;
	_pairs = pairs;
	_wrong = _completed = 0;
	_selected[0] = _selected[1] = -1;
	_limit = limit;
	for (uint side = 0; side < 2; side++)
		for (uint i = 0; i < _pairs; i++)
			_choices[side][i] = random.draw() % (_pairs - i);
	for (uint i = 0; i < this->party(); i++) {
		Parent &parent = _parents[i];
		parent.side = i % 2;
		parent.paired = false;
		parent.tag = i < inputs.size() ? inputs[i].tag : -1;
		for (uint a = 0; a < 3; a++) {
			int count = a ? 3 : 1;
			int value = i < inputs.size() ? inputs[i].traits[a] : -1;
			parent.traits[a] = 0 <= value && value < count ? value : random.draw() % count;
		}
		if (generator) {
			if (!generator->barnParentGenerated(i))
				return false;
		} else {
			random.draw();
			random.draw();
		}
		parent.genes[0] = parent.traits[_trait];
		uint recessive = parent.genes[0] + random.draw() % 2;
		if (2 < recessive)
			recessive = 0;
		if (recessive == parent.genes[0]) {
			recessive = parent.genes[0] + random.draw() % 2;
			if (2 < recessive)
				recessive = 0;
		}
		parent.genes[1] = recessive;
	}
	chooseTarget();
	return true;
}

uint BarnRules::dominant(uint first, uint second) {
	if (first == second)
		return first;
	if ((first + 1) % 3 == second)
		return first;
	return second;
}

void BarnRules::offspring(int first, int second, int values[4]) const {
	for (uint i = 0; i < 4; i++) {
		if (first < 0 && second < 0)
			values[i] = -1;
		else if (first < 0)
			values[i] = _parents[second].genes[i / 2];
		else if (second < 0)
			values[i] = _parents[first].genes[i % 2];
		else
			values[i] = dominant(_parents[first].genes[i % 2], _parents[second].genes[i / 2]);
	}
}

void BarnRules::chooseTarget() {
	if (_pairs <= _completed)
		return;
	int parents[2] = {
		-1,
		-1,
	};
	for (uint side = 0; side < 2; side++) {
		uint offset = _choices[side][_completed];
		for (uint i = side; i < party(); i += 2) {
			if (_parents[i].paired)
				continue;
			if (!offset) {
				parents[side] = i;
				break;
			}
			offset -= 1;
		}
	}
	int values[4];
	offspring(parents[0], parents[1], values);
	for (uint i = 0; i < 4; i++)
		_target[i] = values[i];
}

bool BarnRules::matches(uint first, uint second) const {
	if (party() <= first || party() <= second || _parents[first].paired || _parents[second].paired || _parents[first].side == _parents[second].side)
		return false;
	int values[4];
	offspring(first, second, values);
	uint counts[3] = {}, wanted[3] = {};
	for (uint i = 0; i < 4; i++) {
		counts[values[i]] += 1;
		wanted[_target[i]] += 1;
	}
	return counts[0] == wanted[0] && counts[1] == wanted[1] && counts[2] == wanted[2];
}

BarnRules::Placement BarnRules::place(uint index) {
	if (party() <= index || _parents[index].paired || !chances() || 0 <= _selected[_parents[index].side])
		return kUnavailable00;
	_selected[_parents[index].side] = index;
	if (_selected[0] < 0 || _selected[1] < 0)
		return kSingle01;
	if (matches(_selected[0], _selected[1]))
		return kCorrect03;
	_wrong += 1;
	return kWrong02;
}

bool BarnRules::remove(uint index) {
	if (party() <= index || _selected[_parents[index].side] != static_cast<int>(index))
		return false;
	_selected[_parents[index].side] = -1;
	return true;
}

bool BarnRules::restore(uint index) {
	if (party() <= index || _parents[index].paired || 0 <= _selected[_parents[index].side])
		return false;
	_selected[_parents[index].side] = index;
	return true;
}

bool BarnRules::commitPair() {
	if (_selected[0] < 0 || _selected[1] < 0 || !matches(_selected[0], _selected[1]))
		return false;
	_parents[_selected[0]].paired = _parents[_selected[1]].paired = true;
	_selected[0] = _selected[1] = -1;
	_completed += 1;
	chooseTarget();
	return true;
}

bool BarnRules::findAnswer(Common::Array<Pair> &plan) const {
	plan.clear();
	BarnRules trial = *this;
	trial._selected[0] = trial._selected[1] = -1;
	while (trial._completed < trial._pairs) {
		if (!trial.chances())
			return false;
		bool found = false;
		for (uint first = 0; first < party() && !found; first += 2) {
			for (uint second = 1; second < party(); second += 2) {
				if (!trial.matches(first, second))
					continue;
				Pair pair = {{first, second}};
				plan.push_back(pair);
				trial._selected[0] = first;
				trial._selected[1] = second;
				trial.commitPair();
				found = true;
				break;
			}
		}
		if (!found) {
			plan.clear();
			return false;
		}
	}
	return true;
}

constexpr const char *BarnMachine::kAnimation;
constexpr const char *BarnMachine::kCDAnimation;
constexpr const char *BarnMachine::kSound;

BarnMachine::BarnMachine(TrinketEngine &vm, Shell::Scene &scene, int depth) : _vm(vm), _scene(scene), _depth(depth), _reelSound(vm.mixer()) {
	const char *events[] = {
		"dropped",
		"pickedUp",
		"solved",
		"wrong",
		"outOfTurns",
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
}

BarnMachine::~BarnMachine() {
	clearPuzzle();
}

void BarnMachine::clearPuzzle() {
	_reelSound.stop();
	for (uint i = 0; i < ARRAYSIZE(_parents); i++) {
		delete _parents[i];
		_parents[i] = nullptr;
	}
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
	_owned.clear();
	_pausedSprites.clear();
	for (uint side = 0; side < 2; side++)
		for (uint i = 0; i < 2; i++)
			_geneFaces[side][i] = nullptr;
	for (uint i = 0; i < 4; i++)
		_children[i] = nullptr;
	_generated = false;
}

void BarnMachine::removeSprite(Shell::Sprite *sprite) {
	if (!sprite)
		return;
	for (uint i = 0; i < _pausedSprites.size(); i++)
		if (_pausedSprites[i] == sprite) {
			_pausedSprites.remove_at(i);
			break;
		}
	for (uint i = 0; i < _owned.size(); i++)
		if (_owned[i] == sprite) {
			_owned.remove_at(i);
			break;
		}
	_scene.remove(sprite);
}

Shell::Sprite *BarnMachine::add(uint id, int depth, bool cd) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(cd ? kCDAnimation : kAnimation, id), _depth + depth);
	if (!sprite) {
		_valid = false;
		warning("Barn animation %u could not load", id);
		return nullptr;
	}
	sprite->setFrame(1);
	sprite->enabled = false;
	_owned.push_back(sprite);
	return sprite;
}

bool BarnMachine::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "solvedZerbleTag" && arguments.size() == 1) {
		int index = arguments[0].integer();
		result = Core::ScriptValue(index < 0 || 1 < index ? 0 : _solvedTags[index]);
		return true;
	}
	if (arguments.empty() && property == "debugMode")
		result = Core::ScriptValue(_debugMode);
	else if (arguments.empty() && property == "pieceGrabAOCursorID")
		result = Core::ScriptValue(_grabCursor);
	else if (arguments.empty() && property == "pieceSpotAOCursorID")
		result = Core::ScriptValue(_spotCursor);
	else
		return ScriptableObject::get(property, arguments, result);
	return true;
}

bool BarnMachine::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "nextZerbleTrait" && arguments.size() == 2) {
		int index = arguments[0].integer(), attribute = arguments[1].integer();
		if (0 <= index && static_cast<uint>(index) < _inputs.size() && 0 <= attribute && attribute < 3)
			_inputs[index].traits[attribute] = value.integer();
		return true;
	}
	if (!arguments.empty())
		return ScriptableObject::set(property, arguments, value);
	if (property == "debugMode")
		_debugMode = value.integer();
	else if (property == "enabled")
		_enabled = value.integer() != 0;
	else if (property == "pieceGrabAOCursorID")
		_grabCursor = value.text();
	else if (property == "pieceSpotAOCursorID")
		_spotCursor = value.text();
	else
		return ScriptableObject::set(property, arguments, value);
	return true;
}

bool BarnMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	_time = callTime();
	if (method == "generatePuzzle" && arguments.size() == 4)
		return setup(arguments[0].integer(), arguments[1].integer(), arguments[2].integer() != 0, arguments[3].integer());
	if (method == "addNextZerble" && arguments.size() == 1) {
		BarnRules::Input input;
		input.tag = arguments[0].integer();
		uint at = 0;
		while (at < _inputs.size() && _inputs[at].tag < input.tag)
			at += 1;
		_inputs.insert_at(at, input);
		return true;
	}
	if (!arguments.empty())
		return ScriptableObject::play(method, arguments, result);
	if (method == "pause" || method == "resume") {
		pause(method == "pause", _time);
		return true;
	}
	if (method == "forceDrop") {
		forceDrop();
		return true;
	}
	if (method == "clearPuzzle") {
		clearPuzzle();
		return true;
	}
	if (method == "interrupt") {
		if (_state == kExitDelay05)
			_stateTime -= 700;
		return true;
	}
	return ScriptableObject::play(method, arguments, result);
}

bool BarnMachine::barnParentGenerated(uint index) {
	_parents[index] = new AttributeCharacter(_vm, _scene);
	if (!_parents[index]->create(1, _rules.parent(index).traits, 0, BarnRules::home(index), 1, _time))
		return false;
	_parents[index]->setDepthFromY(true);
	return true;
}

Common::Point BarnMachine::stonePosition(uint side) { return side ? Common::Point(290, 436) : Common::Point(529, 234); }

Common::Rect BarnMachine::stoneBounds(uint side) { return side ? Common::Rect(256, 381, 326, 471) : Common::Rect(494, 209, 574, 249); }

Common::Rect BarnMachine::penBounds(uint side) { return side ? Common::Rect(0, 332, 214, 600) : Common::Rect(391, 95, 690, 185); }

bool BarnMachine::setup(uint pairs, uint trait, bool showGenes, uint limit) {
	clearPuzzle();
	_valid = true;
	_selected = _previousSlot = -1;
	_correct = _exhausted = false;
	_drained = 0;
	_waterFrame = 1;
	memset(_geneOpening, 0, sizeof(_geneOpening));
	memset(_geneStarted, 0, sizeof(_geneStarted));
	if (!_rules.generatePuzzle(pairs, trait, showGenes, limit, _vm.rnd(), _inputs, this))
		return false;
	_inputs.clear();
	_level = _rules.level();
	_stones[0] = add(21050, 0);
	_stones[1] = add(21051, 0);
	_water = add(21083, 0, true);
	_celebration = add(21085, 1, true);
	if (!_valid)
		return false;
	_celebration->visible = false;
	static constexpr int geneX[2][2] = {{417, 587}, {325, 315}}, geneY[2][2] = {{245, 245}, {316, 488}};
	for (uint side = 0; side < 2; side++) {
		for (uint i = 0; i < 2; i++) {
			_genes[side][i] = add(side ? 21065 : 21052, 0);
			if (!_valid)
				return false;
			_genes[side][i]->x = geneX[side][i];
			_genes[side][i]->y = geneY[side][i];
		}
	}
	static constexpr int targetX[] = {
		174,
		216,
		257,
		296,
	},
						 targetY[] = {
							 212,
							 185,
							 159,
							 137,
						 };
	for (uint side = 0; side < 2; side++) {
		_emphasis[side] = add(21095 + side, 2);
		if (!_valid)
			return false;
		_emphasis[side]->visible = false;
	}
	static constexpr uint reels[] = {
		21087,
		21090,
		21091,
		21092,
	};
	for (uint i = 0; i < 4; i++) {
		_target[i] = add(_level == 3 ? 21089 : 21088, 0);
		_reels[i] = add(reels[i], 1);
		if (!_valid)
			return false;
		_target[i]->setFrame(_rules.target(i) + 1);
		_target[i]->x = targetX[i] - _target[i]->width() / 2;
		_target[i]->y = targetY[i] - _target[i]->height() / 2;
	}
	startSpin();
	_generated = true;
	debug(1, "Barn ready: level %u, party %u, pairs %u, chances %u", _level, _rules.party(), _rules.pairs(), _rules.chances());
	return true;
}

void BarnMachine::startSpin() {
	_state = kSpinning00;
	_spinWaiting = true;
	_stateTime = _time;
	_step = 0;
	clearChildren();
	static constexpr int targetX[] = {
		174,
		216,
		257,
		296,
	},
						 targetY[] = {
							 212,
							 185,
							 159,
							 137,
						 };
	for (uint i = 0; i < 4; i++) {
		_target[i]->setFrame(_rules.target(i) + 1);
		_target[i]->x = targetX[i] - _target[i]->width() / 2;
		_target[i]->y = targetY[i] - _target[i]->height() / 2;
		_reels[i]->visible = true;
		_reels[i]->startAnimation(-1);
	}
	_reelSound.play(_vm.resources(), Common::String::format(kSound, static_cast<uint>(kResSound21096_Machine)), Audio::Mixer::kSFXSoundType, 1);
}

int BarnMachine::hitParent(Common::Point point) const {
	for (int i = _rules.party() - 1; 0 <= i; i--)
		if (!_rules.parent(i).paired && _parents[i]->bounds().contains(point))
			return i;
	return -1;
}

void BarnMachine::pick(uint index, Common::Point point) {
	_state = kPlaying01;
	_correct = false;
	_selected = index;
	_pickup = _parents[index]->position();
	_grab = point - _pickup;
	_previousSlot = _rules.selected(_rules.parent(index).side) == static_cast<int>(index) ? _rules.parent(index).side : -1;
	if (0 <= _previousSlot) {
		if (0 <= _rules.selected(0) && 0 <= _rules.selected(1) && _drained < _rules.wrong())
			drainWater();
		_rules.remove(index);
		updateGenes(_previousSlot, false);
		clearChildren();
	}
	_parents[index]->setDepth(200);
	playEffect(kResSound21099_StoneMove);
	emitEvent("pickedUp");
}

void BarnMachine::drainWater() {
	_drained = _rules.wrong();
	uint intervals = _water->getNumberOfFrames() - 1;
	uint count = MIN(_rules.limit(), intervals), step = intervals / count, remainder = intervals % count;
	_waterFrame = 1;
	for (uint i = 0; i < _drained; i++)
		_waterFrame += step + (count - i <= remainder ? 1 : 0);
	_water->pauseAnimation(false);
	playEffect(kResSound21094_ParentRemoved);
}

void BarnMachine::drop() {
	uint index = _selected, side = _rules.parent(index).side;
	bool exhaustedOnRemoval = 0 <= _previousSlot && !_rules.chances();
	if (exhaustedOnRemoval) {
		_exhausted = true;
		emitEvent("outOfTurns");
	}
	_selected = -1;
	_parents[index]->setDepth(1);
	_stones[side]->setFrame(1);
	BarnRules::Placement result = BarnRules::kUnavailable00;
	if (_parents[index]->bounds().intersects(stoneBounds(side)))
		result = _rules.place(index);
	if (result != BarnRules::kUnavailable00) {
		_parents[index]->setPosition(stonePosition(side));
		_correct = result == BarnRules::kCorrect03;
		if (result == BarnRules::kWrong02)
			emitEvent("wrong");
		playEffect(kResSound21100_GeneReveal);
		if (_rules.showGenes()) {
			updateGenes(side, true);
			_state = kGenes02;
			_stateTime = _time;
		} else
			startPreview();
		debug(1, "Barn parent %u placed on stone %u: %s, chances %u", index + 1, side + 1,
			  result == BarnRules::kSingle01 ? "single" : _correct ? "correct"
																   : "wrong",
			  _rules.chances());
	} else {
		Common::Point point = _parents[index]->position();
		Common::Rect pen = penBounds(side);
		if (_parents[index]->bounds().intersects(pen))
			_parents[index]->setPosition(Common::Point(CLIP<int>(point.x, pen.left, pen.right), CLIP<int>(point.y, pen.top, pen.bottom)));
		else {
			_parents[index]->setPosition(_pickup);
			if (0 <= _previousSlot) {
				_rules.restore(index);
				updateGenes(side, true, false);
				clearChildren();
				if (0 <= _rules.selected(0) && 0 <= _rules.selected(1))
					for (uint i = 0; i < 4; i++)
						showChild(i);
				for (uint i = 0; i < 2; i++)
					_emphasis[i]->visible = false;
			}
		}
		playEffect(kResSound21101_GeneClose);
	}
	emitEvent("dropped");
}

void BarnMachine::updateGenes(uint side, bool opening, bool animated) {
	if (_level != 1)
		return;
	_geneTime[side] = _time;
	_geneOpening[side] = opening;
	for (uint i = 0; i < 2; i++) {
		_geneStarted[side][i] = false;
		if (!opening) {
			_genes[side][i]->startAnimation(1);
			// startAnimation restores the source sequence; select the reverse afterward.
			_genes[side][i]->reverseSequence();
			_genes[side][i]->pauseAnimation(false);
			if (_geneFaces[side][i]) {
				_geneFaces[side][i]->reverseSequence();
				_geneFaces[side][i]->pauseAnimation(false);
			}
			_geneStarted[side][i] = true;
			continue;
		}
		if (_geneFaces[side][i])
			removeSprite(_geneFaces[side][i]);
		const BarnRules::Parent &parent = _rules.parent(_rules.selected(side));
		uint base = side ? 21066 : 21053;
		_geneFaces[side][i] = add(base + parent.genes[i] + (i && parent.genes[0] != parent.genes[1] ? 3 : 0), 1);
		if (!_geneFaces[side][i])
			continue;
		_geneFaces[side][i]->x += side ? (i ? -10 : 0) : (i ? 170 : 0);
		_geneFaces[side][i]->y += side && i ? 172 : 0;
		_genes[side][i]->startAnimation(1);
		_genes[side][i]->setFrame(1);
		if (!animated) {
			_genes[side][i]->setFrame(_genes[side][i]->getNumberOfFrames());
			_geneFaces[side][i]->setFrame(_geneFaces[side][i]->getNumberOfFrames());
			_geneStarted[side][i] = true;
		}
	}
	if (animated)
		playEffect(opening ? kResSound21052_StoneOpen : kResSound21065_StoneClose);
}

void BarnMachine::clearChildren() {
	for (uint side = 0; side < 2; side++)
		_emphasis[side]->visible = false;
	for (uint i = 0; i < 4; i++) {
		if (_children[i])
			removeSprite(_children[i]);
		_children[i] = nullptr;
	}
}

void BarnMachine::startPreview() {
	clearChildren();
	if (_rules.selected(0) < 0 || _rules.selected(1) < 0) {
		_state = kPlaying01;
		return;
	}
	_state = kPreview03;
	_stateTime = _time - 400;
	_step = 0;
	playEffect(_correct ? kResSound21093_CorrectPair : kResSound21081_WrongPair);
}

void BarnMachine::showChild(uint index) {
	int values[4];
	_rules.offspring(_rules.selected(0), _rules.selected(1), values);
	if (values[index] < 0)
		return;
	_children[index] = add(_rules.trait() == 2 ? 21082 : 21081, 1);
	if (!_children[index])
		return;
	_children[index]->setFrame(values[index] + 1);
	static constexpr int x[] = {
		473,
		585,
		480,
		596,
	},
						 y[] = {
							 392,
							 392,
							 490,
							 490,
						 };
	_children[index]->x += x[index] - 50;
	_children[index]->y += y[index] - 50 + (_rules.trait() == 2 ? (index < 2 ? 8 : 10) : 0);
	if (_rules.showGenes()) {
		for (uint side = 0; side < 2; side++) {
			uint gene = side ? index / 2 : index % 2;
			_emphasis[side]->startAnimation(1);
			_emphasis[side]->x = _genes[side][gene]->x;
			_emphasis[side]->y = _genes[side][gene]->y;
			_emphasis[side]->visible = true;
		}
	}
}

void BarnMachine::commitPair() {
	int first = _rules.selected(0), second = _rules.selected(1);
	if (first < 0 || second < 0)
		return;
	int chosen[] = {
		first,
		second,
	};
	for (uint side = 0; side < 2; side++) {
		AttributeCharacter &parent = *_parents[chosen[side]];
		if (side) {
			parent.addWalkMove(Common::Point(300, 307));
			parent.addWalkMove(Common::Point(450, 200));
			parent.addWalkMove(Common::Point(500, 200));
		}
		parent.addWalkMove(side ? Common::Point(750, 233) : Common::Point(760, 260));
		parent.walk(true, _time);
		updateGenes(side, false);
		_solvedTags[side] = _rules.parent(chosen[side]).tag;
	}
	_rules.commitPair();
	emitEvent("solved");
	clearChildren();
	_state = kExiting06;
	if (_rules.completed() < _rules.pairs())
		startSpin();
	debug(1, "Barn pair accepted: parents %u and %u, completed %u of %u", first + 1, second + 1, _rules.completed(), _rules.pairs());
}

void BarnMachine::updateState() {
	for (uint i = 0; i < _rules.party(); i++)
		if (_rules.parent(i).paired && !_parents[i]->isWalking())
			_parents[i]->setVisible(false, _time);
	if (_state == kSpinning00) {
		if (_spinWaiting && _stateTime + 500 <= _time) {
			_spinWaiting = false;
			_stateTime = _time;
			playEffect(kResSound21097_ParentExit);
		} else if (!_spinWaiting && _stateTime + 400 <= _time) {
			_reels[_step]->visible = false;
			_reels[_step]->pauseAnimation(true);
			_step += 1;
			_stateTime = _time;
			if (_step == 4) {
				_reelSound.stop();
				_state = kPlaying01;
			} else
				playEffect(kResSound21097_ParentExit);
		}
	}
	for (uint side = 0; side < 2; side++) {
		if (!_geneOpening[side])
			continue;
		for (uint i = 0; i < 2; i++) {
			if (_geneStarted[side][i] || _time < _geneTime[side] + 500 * i)
				continue;
			_geneStarted[side][i] = true;
			_genes[side][i]->pauseAnimation(false);
			if (_geneFaces[side][i]) {
				_geneFaces[side][i]->setRepeat(1);
				_geneFaces[side][i]->pauseAnimation(false);
			}
		}
	}
	if (_state == kGenes02) {
		bool ready = true;
		for (uint side = 0; side < 2; side++)
			if (0 <= _rules.selected(side) && (!_geneStarted[side][1] || _genes[side][1]->isRunning()))
				ready = false;
		if (ready)
			startPreview();
	} else if (_state == kPreview03 && _stateTime + 400 <= _time) {
		_stateTime = _time;
		if (_step < 4) {
			showChild(_step);
			_step += 1;
		} else if (_correct) {
			_celebration->visible = true;
			_celebration->startAnimation(1);
			_state = kCelebrating04;
		} else
			_state = kPlaying01;
	} else if (_state == kCelebrating04 && !_celebration->isRunning()) {
		_celebration->visible = false;
		_state = kExitDelay05;
		_stateTime = _time;
		playEffect(kResSound2726_CommonGo);
	} else if (_state == kExitDelay05 && _stateTime + 700 <= _time)
		commitPair();
	else if (_state == kExiting06) {
		bool walking = false;
		for (uint i = 0; i < _rules.party(); i++)
			if (_rules.parent(i).paired && _parents[i]->isWalking())
				walking = true;
		if (!walking) {
			for (uint i = 0; i < _rules.party(); i++)
				if (_rules.parent(i).paired)
					_parents[i]->setVisible(false, _time);
			if (_rules.completed() == _rules.pairs())
				_state = kPlaying01;
			else
				startSpin();
		}
	}
	if (_waterFrame <= _water->getFrame())
		_water->setFrame(_waterFrame);
}

void BarnMachine::forceDrop() {
	if (_selected < 0)
		return;
	uint index = _selected, side = _rules.parent(index).side;
	_parents[index]->setPosition(_pickup);
	_parents[index]->setDepth(1);
	if (0 <= _previousSlot) {
		_rules.restore(index);
		updateGenes(side, true, false);
		clearChildren();
		if (0 <= _rules.selected(0) && 0 <= _rules.selected(1))
			for (uint i = 0; i < 4; i++)
				showChild(i);
		for (uint i = 0; i < 2; i++)
			_emphasis[i]->visible = false;
	}
	_stones[side]->setFrame(1);
	_selected = _previousSlot = -1;
}

void BarnMachine::event(const Common::Event &event, uint32 time) {
	_time = time;
	if (!_generated || _paused || !_enabled)
		return;
	if (_exhausted || (_state != kPlaying01 && !((_state == kGenes02 || _state == kPreview03) && !_correct)))
		return;
	if (event.type == Common::EVENT_MOUSEMOVE && 0 <= _selected) {
		_parents[_selected]->setPosition(event.mouse - _grab);
		uint side = _rules.parent(_selected).side;
		_stones[side]->setFrame(_rules.chances() && _rules.selected(side) < 0 && _parents[_selected]->bounds().intersects(stoneBounds(side)) ? 2 : 1);
	}
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (0 <= _selected)
			drop();
		else {
			int index = hitParent(event.mouse);
			if (0 <= index && (_rules.chances() || _rules.selected(_rules.parent(index).side) == index))
				pick(index, event.mouse);
		}
	}
	if (event.type == Common::EVENT_LBUTTONUP && 0 <= _selected && _parents[_selected]->position() != _pickup)
		drop();
	return;
}

void BarnMachine::update(uint32 time) {
	_time = time;
	if (!_generated || _paused)
		return;
	Common::Array<uint16> sounds;
	for (uint i = 0; i < _rules.party(); i++)
		_parents[i]->update(time, sounds);
	for (uint i = 0; i < sounds.size(); i++)
		playEffect(sounds[i]);
	updateState();
}

void BarnMachine::pause(bool paused, uint32 time) {
	if (!_generated || _paused == paused)
		return;
	_paused = paused;
	for (uint i = 0; i < _rules.party(); i++)
		_parents[i]->pause(paused, time);
	_reelSound.pause(paused);
	if (paused) {
		_pauseTime = time;
		Common::Array<Shell::Sprite *> sprites;
		sprites.push_back(_water);
		sprites.push_back(_celebration);
		for (uint i = 0; i < 4; i++)
			sprites.push_back(_reels[i]);
		for (uint side = 0; side < 2; side++)
			for (uint i = 0; i < 2; i++) {
				sprites.push_back(_genes[side][i]);
				if (_geneFaces[side][i])
					sprites.push_back(_geneFaces[side][i]);
			}
		for (uint side = 0; side < 2; side++)
			sprites.push_back(_emphasis[side]);
		for (uint i = 0; i < sprites.size(); i++)
			if (sprites[i]->isRunning()) {
				_pausedSprites.push_back(sprites[i]);
				sprites[i]->pauseAnimation(true);
			}
	} else {
		_stateTime += time - _pauseTime;
		for (uint side = 0; side < 2; side++)
			_geneTime[side] += time - _pauseTime;
		for (uint i = 0; i < _pausedSprites.size(); i++)
			_pausedSprites[i]->pauseAnimation(false);
		_pausedSprites.clear();
	}
}

bool BarnMachine::overDraggable(Common::Point point) const {
	if (!_generated || !_enabled)
		return false;
	return !_exhausted && (_state == kPlaying01 || ((_state == kGenes02 || _state == kPreview03) && !_correct)) && 0 <= hitParent(point);
}

bool BarnMachine::pollSound(uint16 &sound) {
	if (_sounds.empty())
		return false;
	sound = _sounds[0];
	_sounds.remove_at(0);
	return true;
}

Common::String BarnMachine::debugAnswer() const {
	Common::String result = Common::String::format("Barn level %u: target %s [%u,%u,%u,%u], dominance 0>1>2>0.\n", _level,
												   _level == 3 ? "tail" : "feet", _rules.target(0), _rules.target(1), _rules.target(2), _rules.target(3));
	if (_state != kPlaying01)
		return result + "Wait for machine, genes, offspring or departing parents.\n";
	Common::Array<BarnRules::Pair> plan;
	if (!_rules.findAnswer(plan))
		return result + "No remaining completion with the current water budget.\n";
	if (plan.empty())
		return result + "All pairs completed.\n";
	bool mismatchFound = false;
	for (uint first = 0; first < _rules.party() && !mismatchFound; first += 2) {
		for (uint second = 1; second < _rules.party(); second += 2) {
			if (_rules.parent(first).paired || _rules.parent(second).paired || _rules.matches(first, second))
				continue;
			uint indices[] = {
				first,
				second,
			};
			for (uint side = 0; side < 2; side++) {
				uint index = indices[side];
				Common::Rect bounds = _parents[index]->bounds();
				Common::Point center(bounds.left + bounds.width() / 2, bounds.top + bounds.height() / 2);
				Common::Point drop = stonePosition(side) + center - _parents[index]->position();
				result += Common::String::format("Mismatch example parent %u: drag (%d,%d) to (%d,%d)\n", index + 1, center.x, center.y, drop.x, drop.y);
			}
			mismatchFound = true;
			break;
		}
	}
	for (uint side = 0; side < 2; side++) {
		uint index = plan[0].parent[side];
		if (0 <= _rules.selected(side) && _rules.selected(side) != static_cast<int>(index)) {
			uint selected = _rules.selected(side);
			Common::Rect bounds = _parents[selected]->bounds();
			Common::Point home = BarnRules::home(selected);
			result += Common::String::format("Return parent %u: drag (%d,%d) to (%d,%d)\n", selected + 1, bounds.left + bounds.width() / 2,
											 bounds.top + bounds.height() / 2, home.x, home.y);
		}
		if (_rules.selected(side) == static_cast<int>(index))
			continue;
		Common::Rect bounds = _parents[index]->bounds();
		Common::Point target = stonePosition(side);
		Common::Point offset(bounds.left + bounds.width() / 2 - _parents[index]->position().x, bounds.top + bounds.height() / 2 - _parents[index]->position().y);
		result += Common::String::format("Parent %u: drag (%d,%d) to (%d,%d), stone %u, genes %u/%u\n", index + 1,
										 bounds.left + bounds.width() / 2, bounds.top + bounds.height() / 2, target.x + offset.x, target.y + offset.y, side + 1,
										 _rules.parent(index).genes[0], _rules.parent(index).genes[1]);
	}
	return result;
}

} // namespace Zoombini3
} // namespace Trinket
