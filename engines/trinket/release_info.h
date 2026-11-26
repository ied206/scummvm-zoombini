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

#ifndef TRINKET_RELEASE_INFO_H
#define TRINKET_RELEASE_INFO_H

#include "common/language.h"
#include "common/platform.h"
#include "common/str.h"

namespace Trinket {

struct TrinketGameDescription;

/** Runtime settings derived from the detected release. */
class ReleaseInfo {
public:
	/** Copy release data, using the configured game language when detection leaves it unknown. */
	explicit ReleaseInfo(const TrinketGameDescription &description);
	/** Return the detected platform for native saves, random numbers and cursors. */
	Common::Platform getPlatform() const { return _platform; }
	/** Return whether the release starts through ScummVM's TLC launcher by default. */
	bool usesTLCLauncher() const;
	/** Format the Z3 Options slot heading in the original bitmap font's encoding. */
	Common::String formatSaveSlotHeading(const Common::String &playerName) const;

private:
	/** Runtime feature bits from the detected game description. */
	const uint32 _features;
	/** Detected or configured language of the game's data. */
	const Common::Language _language;
	/** Platform of the original game executable. */
	const Common::Platform _platform;
};

} // End of namespace Trinket

#endif

