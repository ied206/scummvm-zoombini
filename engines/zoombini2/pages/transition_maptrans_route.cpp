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

#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

int MapTransition::getDestPage(int source, int route, int rescuedBoolies) {
	switch (source) {
	case kPageZombiniville:
		return kPageCrazyTurtle;
	case kPageCrazyTurtle:
		return kPageWaterslide;
	case kPageWaterslide:
		return kPageAquacube;
	case kPageAquacube:
		return kPageRescue1;
	case kPageRescue1:
		return route == 1 ? kPageMagicWall : kPageMysticMarsh;
	case kPageMysticMarsh:
		return kPageWallOfFleens;
	case kPageMagicWall:
		return kPageChezNorf;
	case kPageWallOfFleens:
	case kPageChezNorf:
		return kPageRescue2;
	case kPageRescue2:
		return kPageSnowboard;
	case kPageSnowboard:
		return kPageBoolies;
	case kPageBoolies:
		return rescuedBoolies < 400 ? kPageBooliewood : kPageFinal;
	default:
		return kPageNone;
	}
}

} // End of namespace Zoombini2
