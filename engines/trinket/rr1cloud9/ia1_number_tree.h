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

#ifndef TRINKET_RR1CLOUD9_IA1_NUMBER_TREE_H
#define TRINKET_RR1CLOUD9_IA1_NUMBER_TREE_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** IA1: balance both fruit branches, or remove one cluster from each branch. */
class NumberTree {
public:
	explicit NumberTree(PageScene &host);
	~NumberTree();
	bool enter();
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	bool busy() const { return _phase != kReady || 0 <= _dragged; }
	void skipSpeech();
	void pause(bool paused);
	void resume(uint32 elapsed) {
		_start += elapsed;
		_idle += elapsed;
	}

private:
	enum Phase {
		kReady,
		kTilting,
		kWrong,
		kRestoring,
		kReward,
		kNewTree,
		kEating,
		kIntroSpeech,
		kIntroLeft,
		kIntroRight,
	};
	struct Cluster {
		Shell::Sprite *sprite = nullptr;
		Common::Point home, from, to;
		int value = 0, slot = -1;
	};
	PageScene &_host;
	Shell::ResourcePaths _paths;
	OM::XMLParser _data;
	Common::SharedPtr<OM::NFNTFont> _font, _largeFont;
	Shell::TextObject *_targetText = nullptr, *_equations[2][7] = {};
	Shell::Sprite *_trunk = nullptr, *_treeMove = nullptr, *_nut = nullptr, *_marble = nullptr, *_eat = nullptr;
	Shell::Sprite *_levers[2] = {};
	Common::Array<Shell::Sprite *> _pausedSprites;
	Common::Array<Cluster> _clusters;
	Common::Point _dragOffset;
	int _level = 1, _dataset = 1, _set = 1, _target = 0, _dragged = -1;
	int _tilt[2] = {}, _oldTilt[2] = {}, _initial[2] = {}, _removed[2] = {
															   -1,
															   -1,
														   };
	int _changedSide = 0, _restoreSlot = -1, _rounds = 0, _instruction = 1;
	bool _subtract = false, _imperfect = false, _showMarble = false, _firstNut = true;
	bool _heardCorrect[2] = {};
	bool _branchSound = false;
	uint32 _start = 0, _idle = 0;
	Phase _phase = kReady;
	int number(const Common::String &key) const;
	int indexed(const char *key, int index) const;
	Shell::Sprite *sprite(const Common::String &key, int depth);
	Shell::TextObject *text(int x, int y, bool large = false);
	void say(const char *actor, const Common::String &key);
	void sound(const char *key);
	void sayNumber(int value);
	void readEquation(int side);
	bool setup(bool announce = true);
	int datasetCount() const;
	void chooseDataset();
	void persist();
	int sum(int side) const;
	int occupant(int slot) const;
	Common::Rect slotRect(int slot) const;
	Common::Point position(const Cluster &cluster) const;
	void equations();
	void moveBranches(int side, bool restoring = false);
	bool advanceBranches(uint32 elapsed, bool finish = false);
	void entrySpeech(int first, int last);
	void checkAnswer();
	void solved();
	void drop();
	void help();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
