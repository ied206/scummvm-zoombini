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

#include "trinket/rr1cloud9/rr1cloud9.h"

#include "trinket/rr1cloud9/game.h"
#include "trinket/rr1cloud9/resources.h"
#include "trinket/rr1cloud9/savefiles.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

static OM::ResourceProvider *createResources() { return new Resources(); }

static bool loadResources(OM::ResourceProvider &resources, Common::Path &path) {
	return static_cast<Resources &>(resources).load(path);
}

const Subengine &getSubengine() {
	static const RuntimeVersions game = {
		"RR1 Cloud Nine Windows, 2002 CD",
		{"ompp32x.dll", nullptr, {}},
		{"trinketcore.dll", nullptr, {}},
		{"shelldll.dll", nullptr, {}},
		{"PuzzleEngine.dll", nullptr, {}},
		{"RR1.EXE", nullptr, {}}};
	static const RuntimeVersions launcher = {
		"RR1 Cloud Nine TLC launcher, 2002 CD",
		{"ompp32x.tlc", "4.01d", {2002, 2, 5, 11, 58, 47}},
		{"TLCLLTrinketcore.dll", "1.00a", {2002, 2, 11, 18, 29, 12}},
		{"TLCLLShelldll.dll", nullptr, {}},
		{nullptr, nullptr, {}},
		{"TLCLauncher.exe", nullptr, {}}};
	static const Subengine subengine = {
		&Game::create, &createResources, &loadResources,
		"Reader Rabbit 1st Grade: Capers on Cloud Nine!", &SaveFiles::create, game, launcher, "sku381489-CD", 1, 640, 480};
	return subengine;
}

} // namespace RR1Cloud9
} // namespace Trinket
