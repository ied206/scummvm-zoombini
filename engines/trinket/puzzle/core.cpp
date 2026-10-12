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

#include "trinket/puzzle/core.h"

namespace Trinket {
namespace Puzzle {

bool Controller::areAnswersUsed() const {
	for (uint i = 0; i < answers.size(); i++) {
		if (answers[i] && !answers[i]->isUsed())
			return false;
	}
	return true;
}

bool Controller::areContainersFull() const {
	for (uint i = 0; i < containers.size(); i++) {
		if (containers[i] && !containers[i]->isFull())
			return false;
	}
	return true;
}

bool Controller::allSolved(const Common::Array<Container *> &group) {
	for (uint i = 0; i < group.size(); i++) {
		if (group[i] && !group[i]->isSolved())
			return false;
	}
	return true;
}

bool Controller::isSolved() const {
	for (uint i = 0; i < solvedGroups.size(); i++) {
		if (allSolved(solvedGroups[i]))
			return true;
	}
	return allSolved(containers);
}

} // namespace Puzzle
} // namespace Trinket
