/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_BARN_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_BARN_H

#include "common/ptr.h"
#include "trinket/shell/hotspot.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Cyclic trait dominance and four-child matching, with one parent per pen. */
class BarnRules {
public:
	struct Parent {
		int32 traits[3];
		uint genes[2], side;
		int tag;
		bool paired;
	};
	struct Pair {
		uint parent[2];
	};
	struct Input {
		int32 traits[3] = {
			-1,
			-1,
			-1,
		};
		int tag = -1;
	};
	class Generator {
	public:
		virtual ~Generator() {}
		virtual bool barnParentGenerated(uint index) = 0;
	};
	enum Placement {
		kUnavailable00 = 0,
		kSingle01 = 1,
		kWrong02 = 2,
		kCorrect03 = 3,
	};
	BarnRules();
	bool generate(uint level, uint party, Shell::Random &random, const World::Token *tokens = nullptr, const int *tags = nullptr,
				  Generator *generator = nullptr);
	bool generatePuzzle(uint pairs, uint trait, bool showGenes, uint limit, Shell::Random &random, const Common::Array<Input> &inputs,
						Generator *generator = nullptr);
	uint level() const { return _level; }
	uint pairs() const { return _pairs; }
	uint party() const { return _pairs * 2; }
	uint trait() const { return _trait; }
	bool showGenes() const { return _showGenes; }
	uint chances() const { return _limit - _wrong; }
	uint wrong() const { return _wrong; }
	uint limit() const { return _limit; }
	uint completed() const { return _completed; }
	const Parent &parent(uint index) const { return _parents[index]; }
	uint target(uint index) const { return _target[index]; }
	int selected(uint side) const { return _selected[side]; }
	Placement place(uint index);
	bool remove(uint index);
	bool restore(uint index);
	bool commitPair();
	void offspring(int first, int second, int values[4]) const;
	bool matches(uint first, uint second) const;
	bool findAnswer(Common::Array<Pair> &plan) const;
	static uint dominant(uint first, uint second);
	static Common::Point home(uint index);

private:
	uint _level = 1, _pairs = 0, _limit = 0, _wrong = 0, _completed = 0, _trait = 1;
	bool _showGenes = true;
	Parent _parents[12];
	uint _choices[2][6], _target[4] = {};
	int _selected[2] = {
		-1,
		-1,
	};
	void chooseTarget();
};

/** Native Barn parent slots, genes, target reels and offspring/departure tasks. */
class BarnMachine : public Core::ScriptableObject, private BarnRules::Generator {
public:
	BarnMachine(TrinketEngine &vm, Shell::Scene &scene, int depth);
	~BarnMachine() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void event(const Common::Event &event, uint32 time);
	void update(uint32 time);
	void pause(bool paused, uint32 time);
	bool overDraggable(Common::Point point) const;
	bool isDragging() const { return 0 <= _selected; }
	bool hasResourceError() const { return !_valid; }
	bool pollSound(uint16 &sound);
	const BarnRules &rules() const { return _rules; }
	uint drained() const { return _drained; }
	Common::String debugAnswer() const;

private:
	enum PageResourceId : int16 {
		kResSound21052_StoneOpen = 21052,
		kResSound21065_StoneClose = 21065,
		kResSound21081_WrongPair = 21081,
		kResSound21093_CorrectPair = 21093,
		kResSound21094_ParentRemoved = 21094,
		kResSound21096_Machine = 21096,
		kResSound21097_ParentExit = 21097,
		kResSound21099_StoneMove = 21099,
		kResSound21100_GeneReveal = 21100,
		kResSound21101_GeneClose = 21101,
		kResSound2726_CommonGo = 2726,
	};
	static constexpr const char *kAnimation = "/z3a7/%u.ao";
	static constexpr const char *kCDAnimation = "/z3a7cd/%u.ao";
	static constexpr const char *kSound = "/z3a7/%u.wav";
	enum State {
		kSpinning00 = 0,
		kPlaying01 = 1,
		kGenes02 = 2,
		kPreview03 = 3,
		kCelebrating04 = 4,
		kExitDelay05 = 5,
		kExiting06 = 6,
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	BarnRules _rules;
	Common::Array<BarnRules::Input> _inputs;
	Common::Array<Shell::Sprite *> _owned, _pausedSprites;
	Common::Array<uint16> _sounds;
	int _depth, _debugMode = 0, _solvedTags[2] = {};
	Common::String _grabCursor = "0", _spotCursor = "0";
	bool _paused = false, _valid = true, _correct = false, _exhausted = false, _generated = false, _enabled = true, _spinWaiting = false;
	uint _level = 1, _step = 0, _drained = 0, _waterFrame = 1;
	uint32 _time = 0, _stateTime = 0, _pauseTime = 0;
	State _state = kPlaying01;
	int _selected = -1, _previousSlot = -1;
	Common::Point _grab, _pickup;
	AttributeCharacter *_parents[12] = {};
	Shell::Sprite *_stones[2] = {}, *_genes[2][2] = {}, *_geneFaces[2][2] = {}, *_target[4] = {}, *_reels[4] = {}, *_children[4] = {};
	Shell::Sprite *_water = nullptr, *_celebration = nullptr, *_emphasis[2] = {};
	uint32 _geneTime[2] = {};
	bool _geneOpening[2] = {}, _geneStarted[2][2] = {};
	OM::Sound _reelSound;
	bool setup(uint pairs, uint trait, bool showGenes, uint limit);
	void clearPuzzle();
	void removeSprite(Shell::Sprite *sprite);
	Shell::Sprite *add(uint id, int depth, bool cd = false);
	bool barnParentGenerated(uint index) override;
	int hitParent(Common::Point point) const;
	void pick(uint index, Common::Point point);
	void drop();
	void forceDrop();
	void updateState();
	void startSpin();
	void startPreview();
	void showChild(uint index);
	void clearChildren();
	void updateGenes(uint side, bool opening, bool animated = true);
	void commitPair();
	void drainWater();
	void playEffect(uint id) { _sounds.push_back(id); }
	static Common::Point stonePosition(uint side);
	static Common::Rect stoneBounds(uint side);
	static Common::Rect penBounds(uint side);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
