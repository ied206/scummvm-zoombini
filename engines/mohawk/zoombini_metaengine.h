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

#ifndef MOHAWK_ZOOMBINI_METAENGINE_H
#define MOHAWK_ZOOMBINI_METAENGINE_H

#include "common/config-manager.h"

namespace Mohawk {

class MohawkMetaEngine_Zoombini {
public:
	constexpr static const char *kOptionBrightenPalette = "brighten_palette";
	constexpr static const char *kOptionOriginalPRNG = "original_prng";
	constexpr static const char *kOptionFixHotelMidiBGM = "fix_hotel_midi_bgm";
	constexpr static const char *kOptionFixAudioPops = "fix_audio_pops";
	constexpr static const char *kOptionEnhancedKbdShortcuts = "enhanced_kbd_shortcuts";
	
	static void registerDefaultSettings();
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_METAENGINE_H
