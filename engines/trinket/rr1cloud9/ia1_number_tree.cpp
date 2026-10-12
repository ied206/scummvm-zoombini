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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "trinket/rr1cloud9/ia1_number_tree.h"

#include "common/tokenizer.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

NumberTree::NumberTree(PageScene &host) : _host(host), _paths(host._vm.resources()) {}

NumberTree::~NumberTree() {
	delete _targetText;
	for (auto &side : _equations)
		for (auto *value : side)
			delete value;
}

int NumberTree::number(const Common::String &key) const { return _host._page->number(key); }

int NumberTree::indexed(const char *key, int index) const { return number(Common::String::format("%s.%d", key, index)); }

void NumberTree::say(const char *actor, const Common::String &key) {
	_host.delaySpeech(1);
	_host.say(actor, number(key));
	_host.delaySpeech(1);
}

void NumberTree::sound(const char *key) { _host.playSounds({static_cast<uint16>(number(key))}); }

Shell::Sprite *NumberTree::sprite(const Common::String &key, int depth) {
	return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
}

Shell::TextObject *NumberTree::text(int x, int y, bool large) {
	Shell::TextObject *result = new Shell::TextObject(_paths, large ? _largeFont : _font);
	result->create({Core::ScriptValue::fromText("0"), Core::ScriptValue(x), Core::ScriptValue(y), Core::ScriptValue(1)});
	result->set("color", {Core::ScriptValue(large ? 0 : 65025), Core::ScriptValue(0)}, Core::ScriptValue(large ? 65025 : 0));
	return result;
}

bool NumberTree::enter() {
	Common::ScopedPtr<Common::SeekableReadStream> font(_host._vm.resources().open("common/1018.nfnt"));
	Common::ScopedPtr<Common::SeekableReadStream> large(_host._vm.resources().open("common/1036.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	_largeFont.reset(large ? OM::NFNTFont::load(*large) : nullptr);
	if (!_font || !_largeFont || !_data.loadStream(_host._vm.resources().open("r1ia1.xml")) || !_data.parse() ||
		!_host.actor("squirrel", "kSquirrel"))
		return false;
	_level = CLIP(_host._world.get("R1IA1Level"), 1, 3);
	_dataset = _host._world.get("R1IA1Dataset");
	_set = CLIP(_host._world.get("R1IA1Set"), 1, 3);
	_rounds = _host._practice ? 0 : _host._world.get("R1IA1Rounds");
	_showMarble = !_host._practice && _host._world.get("R1IA1ShowMarble");
	if (!datasetCount())
		return false;
	if (_dataset < 1 || datasetCount() < _dataset)
		chooseDataset();
	_trunk = sprite("kTreeTrunkID", number("kTreeTrunkZ"));
	_treeMove = sprite("kTreeMoveID", number("kTreeMoveZ"));
	_nut = sprite("kNutID", number("kRewardZ"));
	_marble = sprite("kMarbleID", number("kRewardZ"));
	_eat = sprite("kSamEatID", number("kSamZ"));
	for (int side = 0; side < 2; side++) {
		_levers[side] = sprite(Common::String::format("kLeverID.%d", side + 1), number("kLeverZ"));
		for (int i = 0; i < 7; i++) {
			Common::String suffix = Common::String::format(".%d.%d", side + 1, i + 1);
			_equations[side][i] = text(number("kValueTextX" + suffix), number("kValueTextY" + suffix));
		}
	}
	_targetText = text(number("kTargetNumTextX"), number("kTargetNumTextY"), true);
	if (!_trunk || !_treeMove || !_nut || !_marble || !_eat || !_levers[0] || !_levers[1])
		return false;
	_treeMove->visible = _nut->visible = _eat->visible = false;
	_nut->x = number("kNutX");
	_nut->y = number("kNutY");
	_marble->x = number("kMarbleX");
	_marble->y = number("kMarbleY");
	_marble->visible = _showMarble;
	if (_host._practice && _host._chester)
		_host._chester->visible = false;
	if (!setup(false))
		return false;
	if (_host._practice) {
		help();
	} else if (_host._world.get("R1IA1State") < 2) {
		entrySpeech(1, number("kMaxEntrySpchs") - 2);
		if (_level == 1) {
			_levers[0]->setFrame(5);
			_levers[1]->setFrame(5);
			_phase = kIntroSpeech;
		} else
			entrySpeech(number("kMaxEntrySpchs") - 1, number("kMaxEntrySpchs"));
		_host._world.set("R1IA1State", 2);
	} else {
		say("squirrel", Common::String::format("kReentrySpchID.1.%d", _host._vm.rnd().number(1, 3)));
		if (_host._vm.rnd().number(1, 2) == 1) {
			say("reader", "kReentrySpchID.2.1");
			say("sam", "kReentrySpchID.2.2");
		}
	}
	if (_subtract) {
		say("squirrel", "kResetSpchID.4.1");
		say("squirrel", "kResetSpchID.4.2");
	}
	return true;
}

void NumberTree::entrySpeech(int first, int last) {
	for (int i = first; i <= last; i++) {
		Common::String suffix = Common::String::format(".%d", i);
		say(_host._page->text("kEnterCharID" + suffix).c_str(), "kEnterSpchID" + suffix);
	}
}

int NumberTree::datasetCount() const {
	// The US script declares 17 level-three datasets, but its XML contains only 16.
	int count = 0;
	for (int i = 1; i <= indexed("kMaxDataSets", _level); i++) {
		for (int set = 1; set <= number("kNumSets"); set++)
			if (_data.get(Common::String::format("DataSet/Level_%d/WordList_%d/Set_%d", _level, i, set)).empty())
				return count;
		count += 1;
	}
	return count;
}

void NumberTree::chooseDataset() {
	_dataset = _host._vm.rnd().number(1, MAX(1, datasetCount()));
	_set = _host._vm.rnd().number(1, number("kNumSets"));
}

void NumberTree::persist() {
	if (_host._practice)
		return;
	_host._world.set("R1IA1Level", _level);
	_host._world.set("R1IA1Dataset", _dataset);
	_host._world.set("R1IA1Set", _set);
	_host._world.set("R1IA1Rounds", _rounds);
	_host._world.set("R1IA1ShowMarble", _showMarble);
}

bool NumberTree::setup(bool announce) {
	Common::StringTokenizer tokens(_data.get(Common::String::format("DataSet/Level_%d/WordList_%d/Set_%d", _level, _dataset, _set)));
	Common::Array<int> values;
	while (!tokens.empty()) {
		Common::String token = tokens.nextToken();
		for (char c : token)
			if (!Common::isDigit(c))
				return false;
		values.push_back(atoi(token.c_str()));
	}
	if (values.size() != static_cast<uint>(indexed("kNumValues", _level)) || values[0] < 1 || 30 < values[0])
		return false;
	_subtract = values.back() == 0;
	if (_subtract)
		values.pop_back();
	for (uint i = 1; i < values.size(); i++)
		if (values[i] < 1 || 10 < values[i])
			return false;
	for (auto &cluster : _clusters)
		_host._scene.remove(cluster.sprite);
	_clusters.clear();
	_target = values[0];
	_dragged = -1;
	_imperfect = false;
	_removed[0] = _removed[1] = -1;
	_initial[0] = _initial[1] = 0;
	_heardCorrect[0] = _heardCorrect[1] = false;
	Common::Array<uint> homes, left, right;
	for (uint i = 1; i < values.size(); i++) {
		Cluster cluster;
		cluster.value = values[i];
		cluster.sprite = sprite(Common::String::format("kAnswerID.%d", cluster.value), number("kAnswerZ"));
		if (!cluster.sprite)
			return false;
		int home = _host.chooseAction(homes, values.size() - 1);
		cluster.home = Common::Point(indexed("kAnswerX", home), indexed("kAnswerY", home));
		if (_subtract) {
			cluster.slot = _host.chooseAction(i <= 3 ? left : right, 3) - 1;
			if (3 < i)
				cluster.slot += 3;
			_initial[cluster.slot / 3] += cluster.value;
		}
		_clusters.push_back(cluster);
	}
	for (int side = 0; side < 2; side++) {
		_tilt[side] = _subtract ? 3 : 1;
		_levers[side]->setFrame(_subtract ? 5 : 1);
		_levers[side]->visible = true;
	}
	for (auto &cluster : _clusters) {
		Common::Point point = position(cluster);
		cluster.sprite->x = point.x;
		cluster.sprite->y = point.y;
	}
	_targetText->set("text", {}, Core::ScriptValue::fromText(Common::String::format("%d", _target)));
	equations();
	_phase = kReady;
	_idle = _host._time;
	if (_subtract && announce) {
		say("squirrel", "kResetSpchID.4.1");
		say("squirrel", "kResetSpchID.4.2");
	}
	persist();
	return true;
}

int NumberTree::sum(int side) const {
	int total = 0;
	for (const auto &cluster : _clusters)
		if (0 <= cluster.slot && cluster.slot / 3 == side)
			total += cluster.value;
	return total;
}

int NumberTree::occupant(int slot) const {
	for (uint i = 0; i < _clusters.size(); i++)
		if (_clusters[i].slot == slot)
			return i;
	return -1;
}

Common::Rect NumberTree::slotRect(int slot) const {
	int x = indexed("kContainerX", slot + 1);
	int y = number(Common::String::format("kContainerY.%d.%d", _tilt[slot / 3], slot + 1));
	return Common::Rect(x, y, x + number("kContainerW"), y + number("kContainerH"));
}

Common::Point NumberTree::position(const Cluster &cluster) const {
	if (cluster.slot < 0)
		return cluster.home;
	Common::Rect rect = slotRect(cluster.slot);
	return Common::Point(rect.left + (rect.width() - cluster.sprite->width()) / 2, rect.top + (rect.height() - cluster.sprite->height()) / 2);
}

void NumberTree::equations() {
	for (int side = 0; side < 2; side++) {
		Common::String terms[7] = {
			"0",
			"+",
			"0",
			"+",
			"0",
			"=",
			Common::String::format("%d", sum(side)),
		};
		if (_subtract) {
			terms[0] = Common::String::format("%d", _initial[side]);
			terms[1] = "-";
			terms[2] = 0 <= _removed[side] ? Common::String::format("%d", _clusters[_removed[side]].value) : "_";
			terms[3] = "=";
			terms[4] = Common::String::format("%d", 0 <= _removed[side] ? sum(side) : _target);
			terms[5] = terms[6] = "";
		} else {
			int used = 0;
			for (int i = 0; i < 3; i++) {
				int answer = occupant(side * 3 + i);
				if (0 <= answer) {
					int at = _level == 1 ? used : i;
					terms[at * 2] = Common::String::format("%d", _clusters[answer].value);
					used += 1;
				}
			}
			if (_level == 1 && used < 3) {
				terms[4] = terms[2];
				terms[2] = terms[0];
				terms[0] = terms[1] = "";
			}
		}
		for (int i = 0; i < 7; i++)
			_equations[side][i]->set("text", {}, Core::ScriptValue::fromText(terms[i]));
	}
}

void NumberTree::moveBranches(int side, bool restoring) {
	_changedSide = side;
	for (int i = 0; i < 2; i++) {
		_oldTilt[i] = _tilt[i];
		_tilt[i] = 2;
		if (sum(i) < _target)
			_tilt[i] = 1;
		else if (_target < sum(i))
			_tilt[i] = 3;
		if (_tilt[i] != 2)
			_heardCorrect[i] = false;
	}
	for (auto &cluster : _clusters) {
		cluster.from = Common::Point(cluster.sprite->x, cluster.sprite->y);
		cluster.to = position(cluster);
	}
	_start = _host._time;
	_branchSound = false;
	_phase = restoring ? kRestoring : kTilting;
	equations();
}

bool NumberTree::advanceBranches(uint32 elapsed, bool finish) {
	if (!finish && elapsed < 100)
		return false;
	elapsed = elapsed < 100 ? 0 : elapsed - 100;
	bool done = true;
	static const int start[3][3] = {{1, 7, 7}, {1, 3, 5}, {1, 3, 5}};
	static const int end[3][3] = {{1, 9, 9}, {3, 3, 7}, {5, 5, 5}};
	for (int side = 0; side < 2; side++) {
		if (_oldTilt[side] == _tilt[side])
			continue;
		if (!_branchSound) {
			sound("kBranchMoveSFX");
			_branchSound = true;
		}
		int from = start[_tilt[side] - 1][_oldTilt[side] - 1], to = end[_tilt[side] - 1][_oldTilt[side] - 1];
		uint frame = elapsed * MAX<uint>(1, _levers[side]->asset->fps()) / 1000;
		_levers[side]->setFrame(finish ? to : MIN<int>(to, from + frame));
		if (!finish && frame <= static_cast<uint>(to - from))
			done = false;
	}
	for (uint i = 0; i < _clusters.size(); i++) {
		auto &cluster = _clusters[i];
		if (static_cast<int>(i) == _dragged || cluster.slot < 0 || _oldTilt[cluster.slot / 3] == _tilt[cluster.slot / 3])
			continue;
		uint fps = cluster.slot % 3 == 0 ? 34 : 42;
		int step = finish ? 10 : MIN<uint>(10, elapsed * fps / 1000);
		cluster.sprite->x = cluster.from.x + (cluster.to.x - cluster.from.x) * step / 10;
		cluster.sprite->y = cluster.from.y + (cluster.to.y - cluster.from.y) * step / 10;
		if (step < 10)
			done = false;
	}
	return done;
}

void NumberTree::checkAnswer() {
	if (sum(0) == _target && sum(1) == _target) {
		solved();
		return;
	}
	for (int side = 0; side < 2; side++) {
		if (sum(side) == _target && !_heardCorrect[side]) {
			sound("kCorrectSFX");
			_heardCorrect[side] = true;
		}
	}
	int count = 0, total = sum(_changedSide);
	for (int i = 0; i < 3; i++)
		if (0 <= occupant(_changedSide * 3 + i))
			count += 1;
	bool wrong = total != _target && ((_level == 1 && 2 <= count) || count == 3 || (2 <= count && _target < total));
	if (_subtract)
		wrong = total != _target;
	if (wrong) {
		_imperfect = true;
		sound("kWrongSfxID");
		if (_subtract) {
			say("squirrel", Common::String::format("kOopsSpchID.%d", _host._vm.rnd().number(1, 4)));
			_phase = kWrong;
			_start = _host._time;
			return;
		}
		if (_host._vm.rnd().number(1, 2) == 2)
			say("squirrel", Common::String::format("kWrongSpchID.%d", _host._vm.rnd().number(1, 2) + (total < _target ? 2 : 0)));
	}
	_phase = kReady;
}

void NumberTree::sayNumber(int value) {
	_host.delaySpeech(1);
	_host.say("squirrel", value ? number("kEqSpchID") + value - 1 : number("kZeroSpchID"));
	_host.delaySpeech(1);
}

void NumberTree::readEquation(int side) {
	if (_subtract) {
		sayNumber(_initial[side]);
		say("squirrel", "kTypeSpeech.2");
		if (0 <= _removed[side])
			sayNumber(_clusters[_removed[side]].value);
		else
			_host.queueSound(number("kBlankSFX"));
	} else {
		int count = 0;
		for (int i = 0; i < 3; i++) {
			int answer = occupant(side * 3 + i);
			if (_level == 1 && answer < 0)
				continue;
			if (count)
				say("squirrel", "kTypeSpeech.1");
			sayNumber(answer < 0 ? 0 : _clusters[answer].value);
			count += 1;
		}
		if (!count)
			sayNumber(0);
	}
	say("squirrel", "kTypeSpeech.3");
	sayNumber(_subtract && _removed[side] < 0 ? _target : sum(side));
}

void NumberTree::solved() {
	if (0 <= _dragged) {
		auto &cluster = _clusters[_dragged];
		cluster.sprite->x = cluster.home.x;
		cluster.sprite->y = cluster.home.y;
		_host._scene.setDepth(cluster.sprite, number("kAnswerZ"));
		_dragged = -1;
	}
	_phase = kReward;
	_rounds += 1;
	sound("kCorrectSFX");
	say("squirrel", Common::String::format("kCharCongratSpchID.%d", _host._vm.rnd().number(1, 5)));
	if (!_host._practice) {
		if (_rounds % 3 == 0) {
			say("squirrel", _showMarble ? "kRewardSpchID.3" : "kRewardSpchID.2");
			_showMarble = true;
			_marble->visible = true;
			sound("kMarbleSFX");
		} else {
			_nut->visible = true;
			say("squirrel", "kRewardSpchID.1");
		}
	}
	readEquation(0);
	readEquation(1);
	if (!_host._practice) {
		World &world = _host._world;
		int outcome = _imperfect ? 3 : 1;
		int streak = world.get("R1IA1StreakKind") == outcome ? world.get("R1IA1Streak") + 1 : 1;
		int threshold = _imperfect ? 5 : 3;
		world.recordAssessment("R1IA1", _level, outcome, 3, 5, streak);
		world.set("R1IA1StreakKind", outcome);
		world.set("R1IA1Streak", streak == threshold ? 0 : streak);
		if (threshold <= streak && !world.get("R1IA1FixedLevel"))
			_level = CLIP(_level + (_imperfect ? -1 : 1), 1, 3);
	}
	// Commit the next problem with the reward; reloading cannot replay a solved round.
	chooseDataset();
	persist();
	say("squirrel", "kResetSpchID.1.1");
}

void NumberTree::drop() {
	if (_phase == kTilting) {
		advanceBranches(0, true);
		checkAnswer();
		if (_dragged < 0)
			return;
		_host.interrupt();
	}
	Cluster &cluster = _clusters[_dragged];
	int best = -1, area = 0;
	Common::Rect bounds(cluster.sprite->x, cluster.sprite->y, cluster.sprite->x + cluster.sprite->width(), cluster.sprite->y + cluster.sprite->height());
	for (int i = 0; i < 6; i++) {
		Common::Rect overlap = slotRect(i);
		if (!overlap.intersects(bounds))
			continue;
		overlap.clip(bounds);
		int size = overlap.width() * overlap.height();
		if (area < size) {
			area = size;
			best = i;
		}
	}
	int side = _changedSide;
	if (0 <= best) {
		side = best / 3;
		int old = occupant(best);
		if (0 <= old) {
			int empty = -1;
			for (int i = side * 3; i < side * 3 + 3 && empty < 0; i++)
				if (occupant(i) < 0)
					empty = i;
			if (0 <= empty)
				best = empty;
			else {
				_clusters[old].slot = -1;
				_clusters[old].sprite->x = _clusters[old].home.x;
				_clusters[old].sprite->y = _clusters[old].home.y;
			}
		}
		cluster.slot = best;
	}
	_host._scene.setDepth(cluster.sprite, number("kAnswerZ"));
	Common::Point destination = position(cluster);
	cluster.sprite->x = destination.x;
	cluster.sprite->y = destination.y;
	_dragged = -1;
	sound(0 <= best ? "kPutClusterTreeSFX" : "kClusterSnapHomeSFX");
	moveBranches(side);
}

bool NumberTree::event(const Common::Event &event) {
	if (_dragged < 0)
		return false;
	Shell::Sprite *sprite = _clusters[_dragged].sprite;
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONUP) {
		sprite->x = event.mouse.x - _dragOffset.x;
		sprite->y = event.mouse.y - _dragOffset.y;
		if (event.type == Common::EVENT_LBUTTONUP)
			drop();
		return true;
	}
	return event.type == Common::EVENT_LBUTTONDOWN;
}

void NumberTree::help() {
	if (_host._vm.rnd().number(1, 2) == 1)
		say("squirrel", "kInstructionSpchID.1");
	if (1 < _level && _instruction == 1)
		_instruction += 1;
	say("squirrel", Common::String::format("kInstructionSpchID.%d", _instruction + 1));
	_instruction = _instruction % (_host._practice ? 3 : 4) + 1;
}

bool NumberTree::click(Common::Point point) {
	_idle = _host._time;
	if (_showMarble && _marble->covers(point)) {
		_showMarble = _marble->visible = false;
		Common::String item = Common::String::format("item:%d", number("kINV1Marble2"));
		_host._world.set(item, _host._world.get(item) + 1);
		_host.putAway();
		persist();
		say("reader", "kMarbleSpchID");
		return true;
	}
	if (_nut->visible && _nut->covers(point)) {
		_nut->visible = false;
		if (_firstNut)
			say("squirrel", "kEatNutSpchID.1");
		_firstNut = false;
		_host._actors["sam"]->setVisible(false);
		_eat->startAnimation();
		_eat->visible = true;
		_phase = kEating;
		return true;
	}
	for (int i = static_cast<int>(_clusters.size()) - 1; 0 <= i; i--) {
		Cluster &cluster = _clusters[i];
		if (_subtract) {
			if (cluster.slot < 0 || sum(cluster.slot / 3) == _target || !slotRect(cluster.slot).contains(point))
				continue;
			int side = cluster.slot / 3;
			_restoreSlot = cluster.slot;
			_removed[side] = i;
			cluster.slot = -1;
			Common::Point origin(indexed("kSubContainerX", side + 1), indexed("kSubContainerY", side + 1));
			cluster.home = Common::Point(origin.x + (number("kContainerW") - cluster.sprite->width()) / 2,
										 origin.y + (number("kContainerH") - cluster.sprite->height()) / 2);
			cluster.sprite->x = cluster.home.x;
			cluster.sprite->y = cluster.home.y;
			moveBranches(side);
		} else {
			if (!cluster.sprite->covers(point))
				continue;
			_host.interrupt();
			bool removed = 0 <= cluster.slot;
			_changedSide = removed ? cluster.slot / 3 : 0;
			cluster.slot = -1;
			_dragged = i;
			_dragOffset = Common::Point(point.x - cluster.sprite->x, point.y - cluster.sprite->y);
			_host._scene.setDepth(cluster.sprite, 150);
			equations();
			if (removed)
				moveBranches(_changedSide);
		}
		sound("kPickupClusterSFX");
		return true;
	}
	if (_host._actors["squirrel"]->bounds().contains(point)) {
		help();
		return true;
	}
	if (_host._actors["paige"]->bounds().contains(point)) {
		say("paige", Common::String::format("kHelpSpchID.%d", _host._vm.rnd().number(1, _subtract ? 6 : 5)));
		return true;
	}
	if (!_host._practice && (_host._actors["reader"]->bounds().contains(point) || _host._actors["sam"]->bounds().contains(point))) {
		switch (_host._vm.rnd().number(1, 4)) {
		case 1:
			say("sam", "kIdleSpchIDs.3.1");
			say("reader", "kIdleSpchIDs.3.2");
			break;
		case 2:
			say("reader", "kMainSpchID.1");
			say("squirrel", "kIdleSpchIDs.2.1");
			say("squirrel", "kIdleSpchIDs.2.2");
			break;
		case 3:
			say("reader", "kMainSpchID.2");
			say("squirrel", "kMainSpchID.3");
			say("squirrel", "kResetSpchID.3.3");
			break;
		case 4:
			say("sam", "kMainSpchID.4");
			say("squirrel", "kMainSpchID.5");
			break;
		}
		return true;
	}
	if (Common::Rect(330, 220, 390, 275).contains(point)) {
		sayNumber(_target);
		return true;
	}
	for (int side = 0; side < 2; side++) {
		int x = indexed("kEqHPL", side + 1), y = indexed("kEqHPT", side + 1);
		if (Common::Rect(x, y, x + number("kEqHPW"), y + number("kEqHPH")).contains(point)) {
			readEquation(side);
			return true;
		}
	}
	return false;
}

void NumberTree::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kIntroSpeech) {
		_phase = kIntroLeft;
		_start = time;
	} else if (_phase == kIntroLeft || _phase == kIntroRight) {
		int side = _phase == kIntroLeft ? 0 : 1;
		uint frame = (time - _start) * MAX<uint>(1, _levers[side]->asset->fps()) / 1000;
		_levers[side]->setFrame(5 + MIN<uint>(4, frame));
		if (5 <= frame) {
			_start = time;
			if (!side)
				_phase = kIntroRight;
			else {
				_levers[0]->setFrame(1);
				_levers[1]->setFrame(1);
				entrySpeech(number("kMaxEntrySpchs") - 1, number("kMaxEntrySpchs"));
				_phase = kReady;
			}
		}
	} else if (_phase == kTilting || _phase == kRestoring) {
		if (advanceBranches(time - _start)) {
			if (_phase == kRestoring)
				_phase = kReady;
			else
				checkAnswer();
		}
	} else if (_phase == kWrong && 500 <= time - _start) {
		_clusters[_removed[_changedSide]].slot = _restoreSlot;
		auto &cluster = _clusters[_removed[_changedSide]];
		Common::Point point = position(cluster);
		cluster.sprite->x = point.x;
		cluster.sprite->y = point.y;
		_removed[_changedSide] = -1;
		moveBranches(_changedSide, true);
	} else if (_phase == kReward) {
		for (auto &cluster : _clusters)
			cluster.sprite->visible = false;
		_trunk->visible = _levers[0]->visible = _levers[1]->visible = false;
		_treeMove->startAnimation();
		_treeMove->visible = true;
		_phase = kNewTree;
	} else if (_phase == kNewTree && !_treeMove->isRunning()) {
		_treeMove->visible = false;
		_trunk->visible = true;
		if (!setup())
			warning("RR1: cannot load IA1 level %d dataset %d set %d", _level, _dataset, _set);
		sound("kPresentNewClustersSFX");
		if (_rounds < 7)
			say("squirrel", Common::String::format("kResetSpchID.1.%d", _host._vm.rnd().number(2, 5)));
		if (_rounds == 1 || _rounds == 2) {
			int group = _rounds + 1;
			say("reader", Common::String::format("kResetSpchID.%d.1", group));
			for (int i = 2; i <= (_rounds == 1 ? 4 : 3); i++)
				say("squirrel", Common::String::format("kResetSpchID.%d.%d", group, i));
		}
	} else if (_phase == kEating && !_eat->isRunning()) {
		_eat->visible = false;
		_host._actors["sam"]->setVisible(true);
		say("sam", Common::String::format("kEatNutSpchID.%d", _host._vm.rnd().number(2, 5)));
		_phase = kReady;
	} else if (!_host._practice && _phase == kReady && _dragged < 0 && 60000 <= time - _idle) {
		int group = _host._vm.rnd().number(1, 3);
		say(group == 3 ? "sam" : "squirrel", Common::String::format("kIdleSpchIDs.%d.1", group));
		say(group == 3 ? "reader" : "squirrel", Common::String::format("kIdleSpchIDs.%d.2", group));
		_idle = time;
	}
}

void NumberTree::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	_host._scene.drawSprites(canvas, INT_MIN, 10);
	if (_phase != kNewTree) {
		_targetText->draw(canvas);
		for (auto &side : _equations)
			for (auto *value : side)
				value->draw(canvas);
	}
	_host._scene.drawSprites(canvas, 11, INT_MAX);
}

void NumberTree::skipSpeech() {
	_host.interrupt();
	if (_phase == kIntroSpeech || _phase == kIntroLeft || _phase == kIntroRight) {
		_levers[0]->setFrame(1);
		_levers[1]->setFrame(1);
		_phase = kReady;
	}
}

void NumberTree::pause(bool paused) {
	if (paused) {
		_pausedSprites.clear();
		for (auto *sprite : {_treeMove, _eat}) {
			if (sprite && sprite->isRunning()) {
				_pausedSprites.push_back(sprite);
				sprite->pauseAnimation(true);
			}
		}
	} else {
		for (auto *sprite : _pausedSprites)
			sprite->pauseAnimation(false);
		_pausedSprites.clear();
	}
}

} // namespace RR1Cloud9
} // namespace Trinket
