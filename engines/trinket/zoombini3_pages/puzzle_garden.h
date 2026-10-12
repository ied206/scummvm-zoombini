/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_GARDEN_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_GARDEN_H

#include "common/ptr.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** The first accepted plants establish the trait assigned to each axis. */
class GardenRules {
public:
	enum Attribute {
		kFlower00 = 0,
		kStem01 = 1,
		kLeaf02 = 2,
		kRoot03 = 3,
	};
	struct Plant {
		uint trait[4];
		int hole;
	};
	struct Placement {
		uint plant, hole;
	};
	GardenRules();
	bool generate(uint level, uint party, Shell::Random &random);
	uint level() const { return _level; }
	uint party() const { return _party; }
	uint passed() const { return _passed; }
	uint chances() const { return _chances; }
	uint holeCount() const { return _level == 1 ? 4 : _level == 2 ? 16
																  : 64; }
	uint axis(uint index) const { return _axes[index]; }
	const Plant &plant(uint index) const { return _plants[index]; }
	uint population(uint hole) const { return _population[hole]; }
	uint16 allowed(uint hole) const { return _allowed[hole]; }
	bool accepts(uint plant, uint hole) const;
	bool place(uint plant, uint hole);

	/** Called after the rejected plant has returned to its source. */
	void reject();
	void setChances(uint count) { _chances = count; }
	bool findAnswer(Common::Array<Placement> &plan) const;
	static Common::Point holePosition(uint level, uint hole);

private:
	uint _level = 1, _party = 0, _passed = 0, _chances = 0, _axes[3] = {};
	uint16 _ignored = 0, _allowed[64] = {};
	uint _population[64] = {};
	Plant _plants[12] = {};
	uint16 value(uint plant) const;
};

/** Native plant containers, drag gestures, return curve and sun counter. */
class GardenMachine : public Shell::MultiTrinket {
public:
	GardenMachine(TrinketEngine &vm, Shell::Scene &scene);
	~GardenMachine() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void event(const Common::Event &event);
	void update(uint32 time);
	void pause(bool paused, uint32 time);
	bool overDraggable(Common::Point point) const;
	bool isDragging() const { return 0 <= _selected; }
	bool hasResourceError() const { return !_valid; }
	bool returnPending() const { return 0 <= _rejected; }
	bool pollSound(uint16 &sound);
	const GardenRules &rules() const { return _rules; }
	Common::String debugAnswer() const;

private:
	enum PageResourceId : int16 {
		kResSound18156_PlantPickup = 18156,
		kResSound18157_PlantDrop = 18157,
		kResSound18257_Return = 18257,
	};
	static constexpr const char *kAnimation = "/z3a5/%u.ao";
	struct PlantView {
		Shell::Sprite *parts[4];
		Common::Point position, home, offsets[4];
		Common::Rect bounds;
		int depth;
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	GardenRules _rules;
	Common::Array<Shell::Sprite *> _owned;
	Common::Array<uint16> _sounds;
	bool _paused = false, _valid = true, _returning = false;
	uint _level = 1, _party = 0;
	uint32 _time = 0, _pauseTime = 0, _returnTime = 0;
	int _selected = -1, _depth = 20, _rejected = -1;
	Common::Point _grab, _pickup, _returnStart;
	PlantView _plants[12] = {};
	Shell::Sprite *_holes[64] = {}, *_sun = nullptr;
	bool setup(uint level, uint party);
	void clearSprites();
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	void positionPlant(uint index, Common::Point position);
	int hitPlant(Common::Point point) const;
	int hitHole(const Common::Rect &plantBounds) const;
	void dropPlant();
	void plantAccepted(uint index, uint hole);
	void snapAnswer();
	void playEffect(uint id) { _sounds.push_back(id); }
	void updateSun();
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
