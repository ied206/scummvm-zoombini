/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_CORRAL_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_CORRAL_H

#include "common/ptr.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Snozzleberry preferences, projector charges and overlapping light regions. */
class CorralRules {
public:
	struct Fruit {
		uint traits[3];
		bool consumed;
		int projector;
		Common::Point supply;
	};
	struct Zerble {
		uint feet, tail;
		bool fed;
		int slot;
		Common::Point position;
	};
	struct Feeding {
		uint fruit, zerble;
	};
	class Generator {
	public:
		virtual ~Generator() {}
		/** Create the character here to preserve its timer RNG consumption. */
		virtual bool corralGenerated(uint index) = 0;
	};
	CorralRules();
	bool generate(uint level, uint party, Shell::Random &random, Generator *generator = nullptr);
	uint level() const { return _level; }
	uint party() const { return _party; }
	int turns() const { return _turns; }
	void setTurns(int count) { _turns = count; }
	uint projectorCount() const { return _level == 1 ? 2 : 3; }
	uint axis(uint projector) const { return _axes[projector]; }
	int projectedFruit(uint projector) const { return _projectors[projector]; }
	const Fruit &fruit(uint index) const { return _fruits[index]; }
	const Zerble &zerble(uint index) const { return _zerbles[index]; }
	bool project(uint fruit, uint projector, Shell::Random &random);
	bool remove(uint fruit);
	bool matches(uint fruit, uint zerble) const;
	bool feed(uint fruit, uint zerble);
	uint fruitRemaining() const;
	uint fedCount() const;
	uint region(uint zerble) const;
	void findAnswer(Common::Array<Feeding> &plan) const;
	static Common::Point slotPosition(uint slot);

private:
	uint _level = 1, _party = 0, _axes[3] = {};
	int _turns = 0;
	int _projectors[3] = {
		-1,
		-1,
		-1,
	};
	Fruit _fruits[12];
	Zerble _zerbles[12];
	bool relocate(Shell::Random &random);
	void allowedSlots(uint zerble, Common::Array<uint> &slots) const;
};

/** Native Corral fruit, projector, herd and feeding callbacks. */
class CorralMachine : public Shell::MultiTrinket, private CorralRules::Generator {
public:
	CorralMachine(TrinketEngine &vm, Shell::Scene &scene);
	~CorralMachine() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void event(const Common::Event &event, uint32 time);
	void update(uint32 time);
	void pause(bool paused, uint32 time);
	bool overDraggable(Common::Point point) const;
	bool isDragging() const { return 0 <= _selected; }
	bool hasResourceError() const { return !_valid; }
	bool feedingPending() const { return 0 <= _feeding; }
	bool pollSound(uint16 &sound);
	const CorralRules &rules() const { return _rules; }
	Common::String debugAnswer() const;

private:
	enum PageResourceId : int16 {
		kResSound20112_RejectEarly = 20112,
		kResSound20113_RejectLate = 20113,
		kResSound20114_FruitPickup = 20114,
		kResSound20115_ProjectorReady = 20115,
		kResSound20116_FruitDrop = 20116,
		kResSound20117_TrayHover = 20117,
		kResSound20400_Projection = 20400,
		kResSound2726_CommonGo = 2726,
	};
	static constexpr const char *kAnimation = "/z3a6/%u.ao";
	static constexpr const char *kBeamAnimation = "/z3a6cd/20103.ao";
	static constexpr const char *kCommonAnimation = "/z3common/%u.ao";
	struct FruitView {
		Shell::Sprite *body, *leaf;
		Common::Point position, home, bodyOffset, leafOffset;
		Common::Rect bounds;
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	CorralRules _rules;
	Common::Array<Shell::Sprite *> _owned;
	Common::Array<uint16> _sounds;
	bool _paused = false, _valid = true, _demonstrating = false, _correct = false;
	uint _level = 1, _fruitRemaining = 0, _zerblesRemaining = 0;
	uint32 _time = 0, _blinkDeadline = 0;
	uint _blinkCount = 0;
	int _selected = -1, _feeding = -1, _feedingFruit = -1, _lastExited = -1, _hover = -1, _hoverTray = -1;
	Common::Point _grab, _pickup;
	Common::Rect _hoverBounds;
	FruitView _fruits[12] = {};
	AttributeCharacter *_zerbles[12] = {};
	Shell::Sprite *_beams = nullptr, *_projectors[3] = {}, *_trays[3] = {}, *_clues[3][5] = {}, *_feedback[3] = {}, *_sniff[3] = {};
	bool _exiting[12] = {}, _moving[12] = {};
	bool setup(uint level, uint party);
	void clearSprites();
	void removeSprite(Shell::Sprite *sprite);
	Shell::Sprite *add(uint id, int depth, bool common = false);
	bool corralGenerated(uint index) override;
	void positionFruit(uint index, Common::Point position);
	Common::Rect fruitBounds(uint index) const;
	int hitFruit(Common::Point point) const;
	void drop();
	void updateProjectors();
	void updateBlink();
	void updateHover();
	void stopHover();
	void moveZerbles();
	void beginFeeding(uint fruit, uint zerble);
	void finishFeeding();
	void updateFeedback(Common::Array<uint16> &sounds);
	void playEffect(uint id) { _sounds.push_back(id); }
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
