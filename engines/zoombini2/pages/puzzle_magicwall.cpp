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

#include "zoombini2/pages/puzzle_magicwall.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

bool MagicWallMaze::readCount(Common::SeekableReadStream &stream, int &count, int maximum) {
	if (stream.size() - stream.pos() < 2)
		return false;
	count = stream.readSint16LE();
	return !stream.err() && 0 <= count && count <= maximum;
}

bool MagicWallMaze::readLayout(Common::SeekableReadStream &stream, Layout &layout) {
	int count;
	if (!readCount(stream, layout.difficulty, 4) || !readCount(stream, layout.weight, 32767) || !readCount(stream, count, 10) || count == 0)
		return false;
	for (int i = 0; i < count; i++) {
		if (stream.size() - stream.pos() < 4)
			return false;
		const int x = stream.readSint16LE();
		const int y = stream.readSint16LE();
		layout.points.push_back(Common::Point32(x, y));
	}
	for (int group = 0; group < 3; group++) {
		int variants;
		if (!readCount(stream, variants, 32767) || variants == 0)
			return false;
		for (int v = 0; v < variants; v++) {
			int rules;
			if (!readCount(stream, rules, 5))
				return false;
			Variant variant;
			for (int r = 0; r < rules; r++) {
				int edges;
				if (!readCount(stream, edges, 100))
					return false;
				Rule rule;
				for (int e = 0; e < edges; e++) {
					if (stream.size() - stream.pos() < 6)
						return false;
					Edge edge;
					edge.from = stream.readSint16LE();
					edge.to = stream.readSint16LE();
					const int kind = stream.readSint16LE();
					if (edge.from < 0 || count <= edge.from || edge.to < 0 || count <= edge.to || kind < 0 || 1 < kind)
						return false;
					edge.swap = kind == 1;
					rule.push_back(edge);
				}
				variant.push_back(rule);
			}
			layout.groups[group].push_back(variant);
		}
	}
	return !stream.err();
}

bool MagicWallMaze::load(Common::SeekableReadStream &stream) {
	_layouts.clear();
	_layoutIndex = -1;
	int count;
	if (!readCount(stream, count, 32767))
		return false;
	for (int i = 0; i < count; i++) {
		Layout layout;
		if (!readLayout(stream, layout)) {
			warning("MagicWall: incomplete layout %d; retaining %u complete layouts", i, _layouts.size());
			break;
		}
		_layouts.push_back(layout);
	}
	return !_layouts.empty();
}

bool MagicWallMaze::isIdentity(const Common::Array<int> &positions) {
	for (uint i = 0; i < positions.size(); i++) {
		if (positions[i] != static_cast<int>(i))
			return false;
	}
	return true;
}

void MagicWallMaze::applyRule(const Rule &rule, Common::Array<int> &positions, bool simultaneous) {
	bool consumed[10] = {};
	for (const Edge &edge : rule) {
		int from = -1;
		int to = -1;
		for (uint i = 0; i < positions.size(); i++) {
			if (positions[i] == edge.from && (!simultaneous || !consumed[i])) {
				from = i;
				if (simultaneous)
					consumed[i] = true;
			}
			if (edge.swap && positions[i] == edge.to && (!simultaneous || !consumed[i])) {
				to = i;
				if (simultaneous)
					consumed[i] = true;
			}
		}
		if (0 <= from)
			positions[from] = edge.to;
		if (edge.swap && 0 <= to)
			positions[to] = edge.from;
	}
}

void MagicWallMaze::scramble(Random &random) {
	_generationSequence.clear();
	int count;
	do {
		count = random.getRandomNumber(6);
	} while (count < 4);
	for (int i = 0; i < count; i++) {
		const int rule = random.getRandomNumber(_rules.size() - 1);
		_generationSequence.push_back(rule);
		applyRule(_rules[rule], _positions, false);
	}
}

int MagicWallMaze::solve(const Common::Array<int> &positions, int depth) const {
	if (isIdentity(positions))
		return 1;
	for (const Rule &rule : _rules) {
		Common::Array<int> next = positions;
		applyRule(rule, next, true);
		if (isIdentity(next))
			return 2;
		if (depth <= 4 && solve(next, depth + 1) == 2)
			return 2;
	}
	return 0;
}

bool MagicWallMaze::generate(int difficulty, Random &random) {
	_generationSequence.clear();
	_initialPositions.clear();
	int totalWeight = 0;
	for (const Layout &layout : _layouts) {
		if (layout.difficulty == difficulty)
			totalWeight += layout.weight;
	}
	if (totalWeight == 0)
		return false;
	int choice = random.getRandomNumber(totalWeight - 1);
	for (uint i = 0; i < _layouts.size(); i++) {
		if (_layouts[i].difficulty != difficulty)
			continue;
		if (choice < _layouts[i].weight) {
			_layoutIndex = i;
			break;
		}
		choice -= _layouts[i].weight;
	}
	const Layout &selected = layout();
	_rules.clear();
	for (int group = 0; group < 3; group++) {
		const Common::Array<Variant> &variants = selected.groups[group];
		const int variantIndex = random.getRandomNumber(variants.size() - 1);
		const Variant &variant = variants[variantIndex];
		if (!variant.empty() && !variant[0].empty()) {
			for (const Rule &rule : variant) {
				if (!rule.empty())
					_rules.push_back(rule);
			}
		}
	}
	if (_rules.empty() || 5 < _rules.size())
		return false;
	for (Rule &rule : _rules) {
		if (random.getRandomNumber(2) == 0) {
			for (Edge &edge : rule)
				SWAP(edge.from, edge.to);
		}
	}
	_positions.resize(selected.points.size());
	Common::Array<int> candidate;
	Common::Array<int> candidateSequence;
	for (int attempt = 0; attempt < 6; attempt++) {
		for (uint i = 0; i < _positions.size(); i++)
			_positions[i] = i;
		scramble(random);
		const int result = solve(_positions, 0);
		if (result == 0 || result == 2) {
			candidate = _positions;
			candidateSequence = _generationSequence;
		}
		if (result == 0)
			break;
	}
	if (!candidate.empty()) {
		_positions = candidate;
		_generationSequence = candidateSequence;
	}
	_initialPositions = _positions;
	return true;
}

void MagicWallMaze::apply(int ruleIndex) {
	if (0 <= ruleIndex && ruleIndex < static_cast<int>(_rules.size()))
		applyRule(_rules[ruleIndex], _positions, true);
}

bool MagicWallMaze::matched(int color) const {
	return static_cast<int>(_positions.size()) <= color || _positions[color] == color;
}

bool MagicWallMaze::gateMatched(int gate) const {
	return matched(gate) && matched(gate + 4) && matched(gate + 8);
}

bool MagicWallMaze::debugSolve(const Common::Array<int> &positions, int depth, int &budget, Common::Array<int> &sequence) const {
	if (isIdentity(positions))
		return true;
	if (!depth || budget <= 0)
		return false;
	budget -= 1;
	for (uint rule = 0; rule < _rules.size(); rule++) {
		Common::Array<int> next = positions;
		applyRule(_rules[rule], next, true);
		if (next == positions)
			continue;
		sequence.push_back(rule);
		if (debugSolve(next, depth - 1, budget, sequence))
			return true;
		sequence.pop_back();
	}
	return false;
}

bool MagicWallMaze::debugSolution(Common::Array<int> &sequence) const {
	sequence.clear();
	int budget = 200000;
	for (int depth = 0; depth <= 8 && 0 < budget; depth++)
		if (debugSolve(_positions, depth, budget, sequence))
			return true;
	return false;
}

constexpr const char *PuzzleMagicWall::kLayoutPath;
constexpr const char *PuzzleMagicWall::kMusicPath;
constexpr const char *PuzzleMagicWall::kDotFormat;
constexpr const char *PuzzleMagicWall::kBugFormat;
constexpr const char *PuzzleMagicWall::kDirectionFormat;
constexpr const char *PuzzleMagicWall::kLightFormat;
constexpr const char *PuzzleMagicWall::kTabletPath;
constexpr const char *PuzzleMagicWall::kTabletDotPath;
constexpr const char *PuzzleMagicWall::kLeverPath;
constexpr const char *PuzzleMagicWall::kLeverGlowPath;
constexpr const char *PuzzleMagicWall::kGateFormat;
constexpr const char *PuzzleMagicWall::kGateBackFormat;
constexpr const char *PuzzleMagicWall::kExitFormat;
constexpr const char *PuzzleMagicWall::kMoveFormat;
constexpr const char *PuzzleMagicWall::kBugLoopPath;
constexpr const char *PuzzleMagicWall::kGateSoundPath;
constexpr const char *PuzzleMagicWall::kLeverSoundPath;
constexpr const char *PuzzleMagicWall::kRetreatSpeechPath;
constexpr const char *PuzzleMagicWall::kPerfectSpeechFormat;
constexpr const char *PuzzleMagicWall::kColors[11];
constexpr Common::Point32 PuzzleMagicWall::kTabletPositions[5];
constexpr Common::Point32 PuzzleMagicWall::kGatePositions[4];
constexpr Common::Point32 PuzzleMagicWall::kRosterPositions[8];
constexpr Common::Point32 PuzzleMagicWall::kLightPositions[10];

PuzzleMagicWall::PuzzleMagicWall(Zoombini2Engine *vm) : PuzzleBase(vm, kPageMagicWall) {}

PuzzleMagicWall::~PuzzleMagicWall() {
	_vm->setHoverCursorActive(false);
	for (int i = 0; i < 10; i++)
		delete _beetles[i].path;
	for (int i = 0; i < 4; i++)
		delete _gateAnimations[i];
	delete _leverAnimation;
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->unload(_bugSound);
		sound->unload(_gateSound);
		sound->unload(_leverSound);
	}
	for (ZoombiniRunner *runner : _puzzleZoombinis)
		runner->clearMovement();
	finishPuzzleRoster(_vm->_state->_rescue1Storage);
}

void PuzzleMagicWall::loadResources() {
	Gfx *gfx = _vm->_gfx;
	for (int i = 0; i < 10; i++) {
		gfx->loadPageRleBlock(Common::String::format(kDotFormat, kColors[i]));
		gfx->loadPageRleBlock(Common::String::format(kBugFormat, kColors[i]));
	}
	for (int i = 0; i < 11; i++)
		gfx->loadPageRleBlock(Common::String::format(kLightFormat, kColors[i]));
	for (int i = 0; i < 8; i++)
		gfx->loadPageRleBlock(Common::String::format(kDirectionFormat, i + 1));
	gfx->loadPageRleBlock(kTabletPath);
	gfx->loadPageRleBlock(kTabletDotPath);
	gfx->loadPageBitBlock(kLeverGlowPath);
	PageLayer *foreground = gfx->getPageLayerStack()->getLayer(1);
	for (int i = 0; i < 4; i++) {
		gfx->loadPageBitBlock(Common::String::format(kGateBackFormat, 'A' + i));
		_gateAnimations[i] = new Animation(_vm);
		_gateAnimations[i]->loadFromFile(Common::Path(Common::String::format(kGateFormat, 'A' + i)));
		AnimationRunner *runner = foreground->createAnimationRunner(kGatePositions[i], AnimationRunnerMode::kPlayOnce00);
		_gateRunners[i] = runner;
		runner->setAnimation(_gateAnimations[i]);
		for (int frame = 0; frame < 5; frame++)
			runner->addTimedFrame(frame, 50);
		runner->addTimedFrame(5, 2000);
		for (int frame = 4; 0 <= frame; frame--)
			runner->addTimedFrame(frame, 50);
		runner->setCompletionCallback(gateDone, this);
	}
	_leverAnimation = new Animation(_vm);
	_leverAnimation->loadFromFile(Common::Path(kLeverPath));
	_leverRunner = gfx->getPageLayerStack()->getLayer(0)->createAnimationRunner(Common::Point32(279, 362), AnimationRunnerMode::kPlayOnceAndHide02);
	_leverRunner->setAnimation(_leverAnimation);
	static constexpr int kLeverFrames[5] = {
		0,
		1,
		2,
		1,
		0,
	};
	for (int frame : kLeverFrames)
		_leverRunner->addTimedFrame(frame, 100);
	if (SoundManager *sound = _vm->getSoundManager()) {
		_bugSound = sound->load(false, Common::Path(kBugLoopPath), false);
		_gateSound = sound->load(false, Common::Path(kGateSoundPath), false);
		_leverSound = sound->load(false, Common::Path(kLeverSoundPath), false);
	}
}

void PuzzleMagicWall::init() {
	PuzzleBase::init();
	loadResources();
	startPageMusic(Common::Path(kMusicPath));
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *runner = _puzzleZoombinis[i];
		runner->clearMovement();
		runner->setDefaultAnimation(_zoombiniAnimation);
		runner->resetAnimation();
		runner->setPosition(kRosterPositions[MIN<uint>(i, 7)]);
		runner->_inputEnabled = false;
		runner->_hidden = false;
		runner->_puzzleStatus = 0;
	}
	Common::SeekableReadStream *stream = _vm->openResourceFile(kLayoutPath);
	const bool loaded = stream && _maze.load(*stream);
	delete stream;
	if (!loaded)
		error("MagicWall: no bounded puzzle layouts in %s", kLayoutPath);
	newPuzzle();
}

void PuzzleMagicWall::newPuzzle() {
	_ready = _maze.generate(_puzzleLevel, *_vm->_rnd);
	if (!_ready)
		error("MagicWall: no usable layout for difficulty %d", _puzzleLevel);
	for (uint i = 0; i < _maze.positions().size(); i++) {
		delete _beetles[i].path;
		_beetles[i].path = nullptr;
		_beetles[i].position = dotPosition(_maze.positions()[i]) + Common::Point32(5, 8);
		_beetles[i].direction = 0;
	}
	_movingRule = -1;
	_hoveredTablet = -1;
	debug(1, "MagicWall: phase=%d difficulty=%d layout=%d points=%u tablets=%u", static_cast<int>(_phase), _puzzleLevel,
		  _maze.layoutIndex(), _maze.positions().size(), _maze.rules().size());
	for (uint i = 0; i < _maze.positions().size(); i++)
		debug(2, "MagicWall: beetle %u at %d", i, _maze.positions()[i]);
	for (uint i = 0; i < _maze.rules().size(); i++) {
		for (const MagicWallMaze::Edge &edge : _maze.rules()[i])
			debug(2, "MagicWall: rule %u edge %d %d %d", i, edge.from, edge.to, edge.swap);
	}
}

Common::Point32 PuzzleMagicWall::dotPosition(int index) const {
	const Common::Point32 &point = _maze.layout().points[index];
	return Common::Point32(300 + 40 * point.x, -15 + 40 * point.y);
}

bool PuzzleMagicWall::beetlesMoving() const {
	for (int i = 0; i < 10; i++) {
		if (_beetles[i].path)
			return true;
	}
	return false;
}

bool PuzzleMagicWall::runnersMoving() const {
	for (const ZoombiniRunner *runner : _puzzleZoombinis) {
		if (runner->_movementPath)
			return true;
	}
	return false;
}

bool PuzzleMagicWall::gatesActive() const {
	for (int i = 0; i < 4; i++) {
		if (_gateRunners[i] && _gateRunners[i]->isActive())
			return true;
	}
	return false;
}

int PuzzleMagicWall::directionIndex(const Common::Point32 &from, const Common::Point32 &to) {
	const double dx = to.x - from.x;
	const double dy = to.y - from.y;
	if (dx == 0 && dy == 0)
		return 0;
	int angle = static_cast<int>(acos(dx / sqrt(dx * dx + dy * dy)) * 180.0 * 0.31831926);
	if (from.y < to.y)
		angle = -angle;
	return (2 - angle / 45 + 8) % 8;
}

void PuzzleMagicWall::startRule(int index) {
	if (!_ready || _phase == Phase::kFinished || _nextPuzzlePending || gatesActive() || runnersMoving() || beetlesMoving())
		return;
	const Common::Array<int> before = _maze.positions();
	_maze.apply(index);
	const uint32 now = _vm->getGameTickCount();
	for (uint i = 0; i < before.size(); i++) {
		if (before[i] == _maze.positions()[i])
			continue;
		Beetle &beetle = _beetles[i];
		const Common::Point32 end = dotPosition(_maze.positions()[i]);
		const Common::Point32 third((end.x - beetle.position.x) / 3, (end.y - beetle.position.y) / 3);
		beetle.path = new PathObject(_vm);
		beetle.path->appendSegment(beetle.position, beetle.position + third, end - third, end, 10, 0);
		beetle.path->start(now);
		beetle.direction = directionIndex(beetle.position, end);
	}
	_movingRule = index;
	if (SoundManager *sound = _vm->getSoundManager())
		sound->playLoop(_bugSound);
	debug(2, "MagicWall: tablet=%d", index);
}

void PuzzleMagicWall::startRunnerPath(int index, int gate, bool exit) {
	if (index < 0 || 8 <= index || static_cast<int>(_puzzleZoombinis.size()) <= index || _exited[index])
		return;
	const Common::Path path(Common::String::format(exit ? kExitFormat : kMoveFormat, gate + 1));
	PathObject *movement = PathObject::loadFromPAT(_vm, path);
	if (!movement)
		error("MagicWall: required roster path is unavailable");
	movement->setStepValueForAllSegments(4);
	ZoombiniRunner *runner = _puzzleZoombinis[index];
	runner->resetAnimation();
	runner->startMovement(movement, _vm->getGameTickCount());
	runner->startDirectionTrackedAnimation(_vm->getGameTickCount());
	if (exit)
		runner->_puzzleStatus = 1;
}

void PuzzleMagicWall::submit() {
	if (!_ready || gatesActive() || runnersMoving() || beetlesMoving() || _phase == Phase::kFinished)
		return;
	_leverRunner->start(_vm->getGameTickCount());
	SoundManager *sound = _vm->getSoundManager();
	if (sound)
		sound->play(_leverSound);
	bool accepted = false;
	for (int gate = 0; gate < 4; gate++) {
		if (!_maze.gateMatched(gate))
			continue;
		const int index = _exited[gate] ? gate + 4 : gate;
		if (static_cast<int>(_puzzleZoombinis.size()) <= index || _exited[index])
			continue;
		startRunnerPath(index, gate, true);
		if (index < 4)
			startRunnerPath(index + 4, gate, false);
		_gateRunners[gate]->start(_vm->getGameTickCount());
		if (sound)
			sound->play(_gateSound);
		accepted = true;
	}
	if (accepted) {
		_nextPuzzlePending = _phase == Phase::kFirstBoard;
		if (_phase == Phase::kSecondBoard)
			_phase = Phase::kFinished;
		_hoveredTablet = -1;
	}
	debug(1, "MagicWall: submit phase=%d accepted=%d", static_cast<int>(_phase), accepted);
}

void PuzzleMagicWall::gateDone(void *context, AnimationRunner *runner) {
	(void)runner;
	PuzzleMagicWall *page = static_cast<PuzzleMagicWall *>(context);
	page->_canDepart = true;
	if (page->_phase == Phase::kFinished)
		page->_vm->restartGoBlink();
}

void PuzzleMagicWall::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	if (_speechPending) {
		SoundManager *sound = _vm->getSoundManager();
		if (!sound || !sound->hasPendingSpeech()) {
			_speechPending = false;
			_vm->_mapTransitionSourcePageId = kPageMagicWall;
			_vm->requestPageChange(kPageMapTrans);
		}
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *runner = _puzzleZoombinis[i];
		if (runner->_movementPath) {
			if (runner->_movementPath->finished) {
				runner->clearMovement();
				runner->resetAnimation();
				if (runner->_screenPos.y < 330) {
					runner->_hidden = true;
					if (i < 8)
						_exited[i] = true;
				}
			} else {
				runner->advanceMovement(now);
			}
		}
		runner->updateAnimation(now);
	}
	if (gatesActive() || runnersMoving()) {
		_vm->setHoverCursorActive(false);
		return;
	}
	if (_nextPuzzlePending) {
		_phase = Phase::kSecondBoard;
		_nextPuzzlePending = false;
		newPuzzle();
	}
	const bool wasMoving = beetlesMoving();
	for (int i = 0; i < 10; i++) {
		Beetle &beetle = _beetles[i];
		if (beetle.path && !beetle.path->advance(now, beetle.position)) {
			delete beetle.path;
			beetle.path = nullptr;
			if (SoundManager *sound = _vm->getSoundManager())
				sound->stopAll(_bugSound);
		}
	}
	if (wasMoving && !beetlesMoving())
		_movingRule = -1;
	_hoveredTablet = tabletAt(_pointer);
	_vm->setHoverCursorActive(_phase != Phase::kFinished && !_nextPuzzlePending && !beetlesMoving() && (0 <= _hoveredTablet || inside(_pointer, 279, 341, 40, 70)));
	if (0 <= _hoveredTablet || beetlesMoving()) {
		_ripplePhase += 7;
		if (255 < _ripplePhase)
			_ripplePhase = 0;
	}
}

void PuzzleMagicWall::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

Common::Point32 PuzzleMagicWall::tabletPoint(int tablet, int point, int inset) const {
	const Common::Point32 &p = _maze.layout().points[point];
	const int firstX = _maze.layout().points[0].x;
	const Common::Point32 origin = kTabletPositions[tablet];
	return Common::Point32(origin.x + 3 * p.x + inset,
						   origin.y + 3 * p.y + inset - static_cast<int>(3 * (p.x - firstX) * -0.5));
}

void PuzzleMagicWall::drawRipple(ManagedSurface32 *screen, Common::Point32 from, const Common::Point32 &to, int phase) {
	const int dx = ABS(to.x - from.x);
	const int dy = ABS(to.y - from.y);
	const int sx = from.x < to.x ? 1 : -1;
	const int sy = from.y < to.y ? 1 : -1;
	const int steps = MAX(dx, dy);
	int errorTerm = steps / 2;
	for (int step = 0; step < steps; step++) {
		phase -= 3;
		if (phase < 0)
			phase = 255;
		if (dy <= dx) {
			errorTerm += dy;
			if (dx <= errorTerm) {
				errorTerm -= dx;
				from.y += sy;
			}
			from.x += sx;
		} else {
			errorTerm += dx;
			if (dy <= errorTerm) {
				errorTerm -= dy;
				from.x += sx;
			}
			from.y += sy;
		}
		const uint32 intensity = 255 - phase % 255;
		const uint32 color = screen->format.RGBToColor(intensity, intensity, intensity);
		_vm->_gfx->fillRect(screen, Common::Rect32(from.x, from.y, from.x + 1, from.y + 1), color);
	}
}

void PuzzleMagicWall::drawTablets(ManagedSurface32 *screen) {
	for (uint i = 0; i < _maze.rules().size(); i++) {
		_vm->_gfx->drawPageRleBlock(screen, kTabletPath, kTabletPositions[i] - Common::Point32(5, 5));
		if (_nextPuzzlePending || _phase == Phase::kFinished)
			continue;
		for (const MagicWallMaze::Edge &edge : _maze.rules()[i])
			drawRipple(screen, tabletPoint(i, edge.from, 12), tabletPoint(i, edge.to, 12), 255);
		for (uint point = 0; point < _maze.layout().points.size(); point++)
			_vm->_gfx->drawPageRleBlock(screen, kTabletDotPath, tabletPoint(i, point, 10));
	}
}

void PuzzleMagicWall::drawConnections(ManagedSurface32 *screen, int index) {
	if (index < 0 || static_cast<int>(_maze.rules().size()) <= index)
		return;
	for (const MagicWallMaze::Edge &edge : _maze.rules()[index]) {
		Common::Point32 from = dotPosition(edge.from) + Common::Point32(25, 25);
		Common::Point32 to = dotPosition(edge.to) + Common::Point32(25, 25);
		if (!edge.swap) {
			drawRipple(screen, from, to, _ripplePhase);
			continue;
		}
		drawRipple(screen, to, from, _ripplePhase);
		if (from.x == to.x) {
			from.x += 10;
			to.x += 10;
		} else if (from.y == to.y) {
			from.y += 10;
			to.y += 10;
		} else {
			const float initialToX = to.x;
			const float initialToY = to.y;
			float fx = from.x;
			float fy = from.y;
			float tx = to.x;
			float ty = to.y;
			const float stepY = -((ty - fy) / (tx - fx)) * 0.01f;
			do {
				fx += 0.01f;
				fy += stepY;
				tx += 0.01f;
				ty += stepY;
			} while (sqrt((initialToX - tx) * (initialToX - tx) + (initialToY - ty) * (initialToY - ty)) < 10.0);
			from = Common::Point32(static_cast<int>(fx), static_cast<int>(fy));
			to = Common::Point32(static_cast<int>(tx), static_cast<int>(ty));
		}
		drawRipple(screen, from, to, _ripplePhase);
	}
}

void PuzzleMagicWall::drawBeetles(ManagedSurface32 *screen) {
	if (_phase == Phase::kFinished || _nextPuzzlePending)
		return;
	for (uint i = 0; i < _maze.positions().size(); i++) {
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kBugFormat, kColors[i]), _beetles[i].position);
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kDirectionFormat, _beetles[i].direction + 1), _beetles[i].position);
	}
}

void PuzzleMagicWall::onRenderContent(ManagedSurface32 *screen) {
	if (!_ready)
		return;
	if (_phase != Phase::kFinished) {
		drawTablets(screen);
		if (!_nextPuzzlePending)
			drawConnections(screen, beetlesMoving() ? _movingRule : _hoveredTablet);
		for (uint i = 0; i < _maze.positions().size(); i++) {
			const Common::Point32 pos = dotPosition(i);
			if (!_backgroundPath.empty())
				_vm->_gfx->drawPageBitBlockSubRect(screen, _backgroundPath, pos, Common::Rect(pos.x, pos.y, pos.x + 50, pos.y + 50));
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kDotFormat, kColors[i]), dotPosition(i));
		}
		drawBeetles(screen);
	}
	bool allMatched = true;
	for (uint i = 0; i < _maze.positions().size(); i++) {
		const bool lit = _phase != Phase::kFinished && _maze.matched(i);
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kLightFormat, kColors[lit ? i : 10]),
									kLightPositions[i] - Common::Point32(6, 5));
		if (!_maze.matched(i))
			allMatched = false;
	}
	if (allMatched && _phase != Phase::kFinished && !_leverRunner->isActive())
		_vm->_gfx->drawPageBitBlock(screen, kLeverGlowPath, Common::Point32(279, 362));
}

void PuzzleMagicWall::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleMagicWall::onActorsRendered() {
	for (ZoombiniRunner *runner : _puzzleZoombinis)
		runner->advanceAnimationAfterDraw();
}

void PuzzleMagicWall::onRenderForeground(ManagedSurface32 *screen) {
	for (int i = 0; i < 4; i++) {
		if (_gateRunners[i]->isActive())
			_vm->_gfx->drawPageBitBlock(screen, Common::String::format(kGateBackFormat, 'A' + i), kGatePositions[i]);
	}
	_vm->_gfx->getPageLayerStack()->getLayer(1)->drawAndUpdate(screen);
	if (_ready && !_nextPuzzlePending && _phase != Phase::kFinished && gatesActive())
		drawBeetles(screen);
}

bool PuzzleMagicWall::inside(const Common::Point &pos, int x, int y, int width, int height) {
	return x < pos.x && pos.x < x + width && y < pos.y && pos.y < y + height;
}

int PuzzleMagicWall::tabletAt(const Common::Point &pos) const {
	if (!_ready || _phase == Phase::kFinished || _nextPuzzlePending)
		return -1;
	for (uint i = 0; i < _maze.rules().size(); i++) {
		if (inside(pos, kTabletPositions[i].x, kTabletPositions[i].y, 50, 50))
			return i;
	}
	return -1;
}

EventHandleResult PuzzleMagicWall::onLButtonDown(const Common::Point &pos) {
	_pointer = pos;
	_buttonArmed = tabletAt(pos) != -1 || inside(pos, 279, 341, 40, 70);
	return _buttonArmed ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

EventHandleResult PuzzleMagicWall::onLButtonUp(const Common::Point &pos) {
	_pointer = pos;
	if (!_buttonArmed)
		return EventHandleResult::kPassthrough;
	_buttonArmed = false;
	const int tablet = tabletAt(pos);
	if (0 <= tablet)
		startRule(tablet);
	else if (inside(pos, 279, 341, 40, 70))
		submit();
	return EventHandleResult::kConsumed;
}

EventHandleResult PuzzleMagicWall::onMouseMove(const Common::Point &pos) {
	_pointer = pos;
	return EventHandleResult::kPassthrough;
}

bool PuzzleMagicWall::blocksSidebarInteraction() const {
	return gatesActive() || runnersMoving() || beetlesMoving() || _speechPending;
}

bool PuzzleMagicWall::canUseGoButton() const {
	return _canDepart && !blocksSidebarInteraction();
}

bool PuzzleMagicWall::onGoButtonPressed() {
	if (_speechPending)
		return false;
	if (!_vm->_isSavedGame)
		return true;
	int remaining = 0;
	for (const ZoombiniRunner *runner : _puzzleZoombinis) {
		if (runner->_puzzleStatus == 0)
			remaining += 1;
	}
	Common::String path;
	if (4 <= remaining)
		path = kRetreatSpeechPath;
	else if (remaining == 0)
		path = Common::String::format(kPerfectSpeechFormat, _vm->_rnd->getRandomNumber(4) + 1);
	else
		return true;
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return true;
	sound->queueSpeech(Common::Path(path));
	_speechPending = true;
	return false;
}

Common::String PuzzleMagicWall::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	if (!_ready)
		return answer + "Board resources are unavailable.\n";
	answer += "Goal: each beetle occupies the dot of its own color.\n";
	answer += "Generation scramble from that goal (sequential construction rules):";
	for (int tablet : _maze.generationSequence())
		answer += Common::String::format(" %d", tablet + 1);
	answer += "\nThis is construction history, not a solution. Player presses use simultaneous moves; reversing this list is not guaranteed to solve.\n";
	for (uint color = 0; color < _maze.positions().size(); color++)
		answer += Common::String::format("%s beetle: initial dot %d; current dot %d; target dot %u\n", kColors[color],
										 _maze.initialPositions()[color] + 1, _maze.positions()[color] + 1, color + 1);
	answer += "Solution search from the current position:\n";
	Common::Array<int> sequence;
	if (_maze.debugSolution(sequence)) {
		answer += "Tablets (left to right), then pull the lever:";
		for (int tablet : sequence)
			answer += Common::String::format(" %d", tablet + 1);
		answer += "\n";
	} else {
		answer += "No full solution found within 8 presses / 200000 search nodes. Tablet mappings:\n";
		for (uint rule = 0; rule < _maze.rules().size(); rule++)
			for (const MagicWallMaze::Edge &edge : _maze.rules()[rule]) {
				const char *arrow = "->";
				if (edge.swap)
					arrow = "<->";
				answer += Common::String::format("  Tablet %u: dot %d %s dot %d\n", rule + 1, edge.from + 1, arrow, edge.to + 1);
			}
	}
	return answer;
}

Common::String PuzzleMagicWall::debugGetChanceDetails() const {
	return Common::String::format("Tablet presses and rejected submissions are unlimited. Accepted rounds: %d/2.\n", static_cast<int>(_phase));
}

} // End of namespace Zoombini2
