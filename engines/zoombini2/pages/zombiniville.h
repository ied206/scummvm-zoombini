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

#ifndef ZOOMBINI2_PAGES_ZOMBINIVILLE_H
#define ZOOMBINI2_PAGES_ZOMBINIVILLE_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/page.h"

namespace Zoombini2 {

class BitBlock;
class Animation;
class ZoombiniGfx;
class Zoombini;

/**
 * Zombiniville — home hub (page ID 0).
 * Object size: 0x1F8 (504 bytes).
 * Init: Zombiniville__Init_43DD60
 *
 * Features: 4 feature stations (hair, eyes, nose, feet) with 5 buttons each,
 * route buttons (left/right), BigZomb preview, zoombini creation.
 */
class Zombiniville : public Page {
public:
	Zombiniville(Zoombini2Engine *engine);
	~Zombiniville() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	BitBlock *_background;

	// Feature station buttons — 4 groups × 5 values = 20 Animation sprites
	// Loaded from bmp/zombiniville/pikaroll/z1pi{group}{value}.an
	Animation *_featureButtons[4][5];

	// BigZomb preview — loaded from bmp/zombiniville/BigZomb/BigZomb.anm
	ZoombiniGfx *_bigZombGfx;

	// Bumper indicators
	Animation *_bumper1;
	Animation *_bumperValid;

	// Feature station button layouts
	struct FeatureStation {
		Common::Rect buttonRects[5];    // Click rectangles (from SetRect)
		Common::Point drawPos[5];       // Draw positions (from CreateAnimElement)
		int selectedValue;              // Currently selected value (1-5, 0=none)
	};
	FeatureStation _stations[4]; // One per feature type

	// Action buttons — QuickFill, BatchFill, Go
	Common::Rect _quickFillRect;
	Common::Rect _batchFillRect;
	Common::Rect _goRect;

	// Zoombini slots in the boarding area
	Common::Array<Zoombini *> _boardingZoombinis;

	// Sound effect handles
	int _sndFeatureSelect;   // "sounds/fx/Z-BS11.wav"
	int _sndFeatureClick2;   // "sounds/fx/Z-BS12.wav"
	int _sndFeatureClick3;   // "sounds/fx/Z-BS13.wav"
	int _sndFeatureClick4;   // "sounds/fx/Z-BS14.wav"
	int _sndWrongZoombini;   // "sounds/fx/wrongz"

	// State
	int _phase;  // 0=selecting features, 1=waiting, 2=departing
	uint32 _lastTick;

	void createZoombini();
	void setupFeatureRects();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_ZOMBINIVILLE_H
