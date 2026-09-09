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
class ZoombiniGraphics;
class ZoombiniState;

/**
 * Zoombiniville is the starting shelter, where a party of 16 is assembled.
 *
 * Four feature stations assemble a party of 16 before route departure.
 */
class Zombiniville : public ShelterPage {
public:
	/** Construct the starting shelter for @p engine. */
	Zombiniville(Zoombini2Engine *engine);
	/** Release station animations, graphics, and entrance paths. */
	~Zombiniville() override;

	/** Load the shelter and initialize an empty boarding party. */
	void init() override;
	/** Advance feature controls and active entrance paths. */
	void update() override;
	/** Draw the shelter, feature stations, and boarding party. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Dispatch a click to a feature station or party action. */
	void handleClick(const Common::Point &pos) override;
	/** Return whether a full party of 16 can leave the shelter. */
	bool canUseGoButton() const override;

private:
	/** Starting-shelter background. */
	BitBlock *_background;

	/** Feature selection controls indexed by feature and value. */
	Animation *_featureButtons[4][5];
	/** Control that selects a random valid feature combination. */
	Animation *_quickFillButton;
	/** Control that fills the remaining party with valid Zoombinis. */
	Animation *_batchFillButton;
	/** Control that starts the route when the party is full. */
	Animation *_goButton;

	/** Large animation cells used for the feature preview. */
	ZoombiniGraphics *_bigZombGfx;
	/** Small animation cells used in the boarding area. */
	ZoombiniGraphics *_littleZombGfx;
	/** Font used for the generated Zoombini name. */
	BitmapFont *_nameFont;

	/** Geometry and current value for one feature station. */
	struct FeatureStation {
		/** Hit-test areas for the station's five values. */
		Common::Rect buttonRects[5];
		/** Draw positions for the station's five controls. */
		Common::Point drawPos[5];
		/** Currently selected feature value. */
		int selectedValue;
	};
	/** Four stations corresponding to the four visible features. */
	FeatureStation _stations[4];
	/** Counts of each feature value already present in the party. */
	int _featureCounts[4][6];

	/** Quick Fill control hit-test area. */
	Common::Rect _quickFillRect;
	/** Batch Fill control hit-test area. */
	Common::Rect _batchFillRect;
	/** Go control hit-test area. */
	Common::Rect _goRect;

	/** Owned Zoombini states currently assembled for departure. */
	Common::Array<ZoombiniState *> _boardingZoombinis;
	/** Entrance path corresponding to each boarding Zoombini. */
	Common::Array<PathObject *> _entrancePaths;

	/** Shelter music handle. */
	int _musicId;
	/** Sound played when a feature value is selected. */
	int _sndFeatureSelect;
	/** Sound played by Quick Fill. */
	int _sndQuickFill;
	/** Sound played by Batch Fill. */
	int _sndBatchFill;
	/** Sound played when a valid Zoombini joins the party. */
	int _sndValidZoombini;
	/** Sound played when the selected combination cannot join. */
	int _sndWrongZoombini;

	/** Generated name displayed for the current feature selection. */
	Common::String _currentName;

	/** Create one Zoombini from the current feature selection when valid. */
	bool createZoombini(bool allowConcurrentEntrances);
	/** Return whether the current feature selection satisfies party limits. */
	bool canCreateSelectedZoombini() const;
	/** Return whether any boarding Zoombini is still entering. */
	bool hasActiveEntrance() const;
	/** Return whether the supplied feature combination fits the current party. */
	bool passesPackFeatureLimits(byte featureA, byte featureB, byte featureC, byte featureD) const;
	/** Select a random feature combination. */
	void randomizeSelectedFeatures();
	/** Reset all four stations to their first values. */
	void resetSelectedFeatures();
	/** Recount feature values in the current boarding party. */
	void refreshFeatureCounts();
	/** Generate the displayed name for the current selection. */
	Common::String generateName();
	/** Create an entrance path ending at @p destination. */
	PathObject *createEntrancePath(const Common::Point &destination) const;
	/** Initialize station and action-control hit-test geometry. */
	void setupFeatureRects();
	/** Return the boarding-area position for @p index. */
	static Common::Point getSlotPosition(uint index);
	/** Draw one animation frame with the supplied alpha lookup table. */
	static void drawAnimationFrame(Graphics::ManagedSurface *screen, const Animation *animation, int frameIndex, int x, int y,
								   const byte alphaLUT[256][256]);
	/** Draw one Zoombini at boarding cell @p cellIndex. */
	void drawZoombini(Graphics::ManagedSurface *screen, const ZoombiniState &zoombini, int cellIndex) const;
	/** Draw all Zoombinis currently in the boarding area. */
	void drawBoardingZoombinis(Graphics::ManagedSurface *screen) const;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H
