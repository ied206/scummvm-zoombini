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

#ifndef TRINKET_PUZZLE_CORE_H
#define TRINKET_PUZZLE_CORE_H

#include "common/array.h"

namespace Trinket {

/** Shared PuzzleEngine.dll family; game-specific activity rules belong to subengines. */
namespace Puzzle {

/** Non-owning interfaces: scene objects retain the lifetime of answers and containers. */
class Answer {
public:
	virtual ~Answer() {}
	virtual bool isUsed() const = 0;
};

class Container {
public:
	virtual ~Container() {}
	virtual bool isFull() const = 0;
	virtual bool isSolved() const = 0;
};

/**
 * Shared PuzzleEngine completion predicates, independent of a game's puzzle rules.
 * Each puzzle instance has its own controller; this is not an engine-wide singleton.
 */
class Controller {
public:
	Common::Array<Answer *> answers;
	Common::Array<Container *> containers;
	Common::Array<Common::Array<Container *>> solvedGroups;
	bool areAnswersUsed() const;
	bool areContainersFull() const;
	bool isSolved() const;

private:
	static bool allSolved(const Common::Array<Container *> &group);
};

} // End of namespace Puzzle
} // End of namespace Trinket

#endif
