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

#ifndef ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H
#define ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class BitBlock;
class Animation;
class BitmapFont;
class PathObject;
class ZoombiniGfx;
class Zoombini;

/**
 * Zoombiniville is the starting shelter, where a party of 16 is assembled.
 *
 * Four feature stations assemble a party of 16 before route departure.
 */
class Zombiniville : public ShelterPage {
public:
	Zombiniville(Zoombini2Engine *engine);
	~Zombiniville() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;
	bool canUseGoButton() const override;

private:
	BitBlock *_background;

	// Feature station buttons: 4 groups with 5 values each.
	// Loaded from bmp/zombiniville/pikaroll/z1pi{group}{value}.an
	Animation *_featureButtons[4][5];
	Animation *_quickFillButton;
	Animation *_batchFillButton;
	Animation *_goButton;

	ZoombiniGfx *_bigZombGfx;
	ZoombiniGfx *_littleZombGfx;
	BitmapFont *_nameFont;

	// Feature station button layouts
	struct FeatureStation {
		Common::Rect buttonRects[5];
		Common::Point drawPos[5];
		int selectedValue;
	};
	FeatureStation _stations[4];
	int _featureCounts[4][6];

	// Quick Fill, Batch Fill, and Go buttons.
	Common::Rect _quickFillRect;
	Common::Rect _batchFillRect;
	Common::Rect _goRect;

	// Zoombini slots in the boarding area
	Common::Array<Zoombini *> _boardingZoombinis;
	Common::Array<PathObject *> _entrancePaths;

	// Sound effect handles
	int _musicId;
	int _sndFeatureSelect;
	int _sndQuickFill;
	int _sndBatchFill;
	int _sndValidZoombini;
	int _sndWrongZoombini;

	Common::String _currentName;

	bool createZoombini(bool allowConcurrentEntrances);
	bool canCreateSelectedZoombini() const;
	bool hasActiveEntrance() const;
	bool passesPackFeatureLimits(byte featureA, byte featureB, byte featureC, byte featureD) const;
	void randomizeSelectedFeatures();
	void resetSelectedFeatures();
	void refreshFeatureCounts();
	Common::String generateName();
	PathObject *createEntrancePath(const Common::Point &destination) const;
	void setupFeatureRects();
	static Common::Point getSlotPosition(uint index);
	static void drawAnimationFrame(Graphics::ManagedSurface *screen, const Animation *animation, int frameIndex, int x, int y,
			const byte alphaLUT[256][256]);
	void drawZoombini(Graphics::ManagedSurface *screen, const Zoombini &zoombini, int cellIndex) const;
	void drawBoardingZoombinis(Graphics::ManagedSurface *screen) const;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H
