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

#include "common/debug.h"

#include "zoombini2/pages/zombiniville.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// Zombiniville — home hub (page ID 0).
// Original: Zombiniville__Init_43DD60 (0x43DD60), object size 0x1F8 (504 bytes).
//
// Screen layout (800×600):
//   Feature 3 buttons along top
//   Feature 4 buttons on left side
//   Feature 2 buttons on right side
//   Feature 1 buttons along bottom
//   BigZomb preview in center
//   Quick Fill / Batch Fill / Go buttons at bottom
// ============================================================================

Zombiniville::Zombiniville(Zoombini2Engine *engine)
	: Page(engine), _background(nullptr), _bigZombGfx(nullptr),
	  _bumper1(nullptr), _bumperValid(nullptr),
	  _sndFeatureSelect(-1), _sndFeatureClick2(-1),
	  _sndFeatureClick3(-1), _sndFeatureClick4(-1),
	  _sndWrongZoombini(-1),
	  _phase(0), _lastTick(0) {
	_pageId = kPageZombiniville;
	for (int i = 0; i < kNumFeatures; i++) {
		for (int j = 0; j < kNumFeatureValues; j++) {
			_stations[i].buttonRects[j] = Common::Rect();
			_stations[i].drawPos[j] = Common::Point();
			_featureButtons[i][j] = nullptr;
		}
		_stations[i].selectedValue = 0;
	}
}

Zombiniville::~Zombiniville() {
	delete _background;
	delete _bigZombGfx;
	delete _bumper1;
	delete _bumperValid;
	for (int i = 0; i < kNumFeatures; i++) {
		for (int j = 0; j < kNumFeatureValues; j++)
			delete _featureButtons[i][j];
	}
}

void Zombiniville::setupFeatureRects() {
	// Exact positions from IDA decompilation of Zombiniville__Init_43DD60.
	// All coordinates are native 800×600.

	// Feature 1 (bottom) — hair buttons (z1pi1{1-5})
	_stations[0].drawPos[0] = Common::Point(237, 345);
	_stations[0].drawPos[1] = Common::Point(292, 351);
	_stations[0].drawPos[2] = Common::Point(398, 355);
	_stations[0].drawPos[3] = Common::Point(346, 349);
	_stations[0].drawPos[4] = Common::Point(438, 369);
	_stations[0].buttonRects[0] = Common::Rect(238, 349, 286, 390);
	_stations[0].buttonRects[1] = Common::Rect(291, 352, 344, 396);
	_stations[0].buttonRects[2] = Common::Rect(398, 359, 433, 403);
	_stations[0].buttonRects[3] = Common::Rect(349, 353, 394, 398);
	_stations[0].buttonRects[4] = Common::Rect(438, 365, 478, 406);

	// Feature 2 (right) — eyes buttons (z1pi2{1-5})
	_stations[1].drawPos[0] = Common::Point(534, 126);
	_stations[1].drawPos[1] = Common::Point(529, 167);
	_stations[1].drawPos[2] = Common::Point(526, 214);
	_stations[1].drawPos[3] = Common::Point(522, 255);
	_stations[1].drawPos[4] = Common::Point(518, 296);
	_stations[1].buttonRects[0] = Common::Rect(534, 118, 570, 151);
	_stations[1].buttonRects[1] = Common::Rect(529, 159, 564, 197);
	_stations[1].buttonRects[2] = Common::Rect(522, 207, 561, 247);
	_stations[1].buttonRects[3] = Common::Rect(518, 255, 557, 298);
	_stations[1].buttonRects[4] = Common::Rect(511, 303, 552, 341);

	// Feature 3 (top) — nose buttons (z1pi3{1-5})
	_stations[2].drawPos[0] = Common::Point(229, 41);
	_stations[2].drawPos[1] = Common::Point(286, 45);
	_stations[2].drawPos[2] = Common::Point(404, 46);
	_stations[2].drawPos[3] = Common::Point(345, 44);
	_stations[2].drawPos[4] = Common::Point(453, 45);
	_stations[2].buttonRects[0] = Common::Rect(226, 40, 276, 85);
	_stations[2].buttonRects[1] = Common::Rect(284, 42, 335, 87);
	_stations[2].buttonRects[2] = Common::Rect(403, 44, 448, 85);
	_stations[2].buttonRects[3] = Common::Rect(345, 43, 396, 87);
	_stations[2].buttonRects[4] = Common::Rect(454, 44, 501, 85);

	// Feature 4 (left) — feet buttons (z1pi4{1-5})
	_stations[3].drawPos[0] = Common::Point(162, 155);
	_stations[3].drawPos[1] = Common::Point(165, 109);
	_stations[3].drawPos[2] = Common::Point(163, 195);
	_stations[3].drawPos[3] = Common::Point(157, 233);
	_stations[3].drawPos[4] = Common::Point(157, 275);
	_stations[3].buttonRects[0] = Common::Rect(164, 153, 207, 186);
	_stations[3].buttonRects[1] = Common::Rect(162, 112, 203, 147);
	_stations[3].buttonRects[2] = Common::Rect(166, 190, 209, 223);
	_stations[3].buttonRects[3] = Common::Rect(166, 228, 212, 262);
	_stations[3].buttonRects[4] = Common::Rect(167, 271, 213, 304);

	// Action buttons — QuickFill, BatchFill, Go
	_quickFillRect = Common::Rect(135, 399, 211, 448);
	_batchFillRect = Common::Rect(239, 416, 318, 477);
	_goRect = Common::Rect(404, 438, 491, 500);
}

void Zombiniville::init() {
	debug(1, "Zombiniville::init");

	// Load background — "#bmp/zombiniville/zoombiniville.bmp" (CD-only resource)
	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/zombiniville/zoombiniville"))) {
		warning("Zombiniville: Failed to load background");
	}

	// Load feature button animations
	// Original: "./bmp/zombiniville/pikaroll/z1pi{group}{value}.bmp" → cached as .an
	for (int grp = 0; grp < kNumFeatures; grp++) {
		for (int val = 0; val < kNumFeatureValues; val++) {
			Common::String path = Common::String::format(
				"bmp/zombiniville/pikaroll/z1pi%d%d.an", grp + 1, val + 1);
			_featureButtons[grp][val] = new Animation();
			if (!_featureButtons[grp][val]->loadFromFile(Common::Path(path))) {
				debug(1, "Zombiniville: Failed to load %s", path.c_str());
			}
		}
	}

	// Load BigZomb preview
	_bigZombGfx = new ZoombiniGfx();
	if (!_bigZombGfx->loadFromFile(Common::Path("bmp/zombiniville/BigZomb/BigZomb.anm"))) {
		warning("Zombiniville: Failed to load BigZomb.anm");
	}

	// Load bumper indicators
	_bumper1 = new Animation();
	_bumper1->loadFromFile(Common::Path("bmp/zombiniville/BUMPER-1.AN"));

	_bumperValid = new Animation();
	_bumperValid->loadFromFile(Common::Path("bmp/zombiniville/bumper-Valid.an"));

	setupFeatureRects();

	// Reset feature selections
	for (int i = 0; i < kNumFeatures; i++) {
		_stations[i].selectedValue = 0;
		_engine->_selectedFeatures[i] = -1;
	}

	_phase = 0;
	_lastTick = _engine->getGameTickCount();

	// Play picker screen music
	int musicId = _engine->getSoundManager()->load(true, Common::Path("sounds/music/ZMR-PickerScreen.wav"), true);
	if (musicId >= 0)
		_engine->getSoundManager()->play(musicId);

	// Load sound effects (original: g_sndFeatureSelect etc.)
	SoundManager *sm = _engine->getSoundManager();
	_sndFeatureSelect = sm->load(false, Common::Path("sounds/fx/Z-BS11.wav"), false);
	_sndFeatureClick2 = sm->load(false, Common::Path("sounds/fx/Z-BS12.wav"), false);
	_sndFeatureClick3 = sm->load(false, Common::Path("sounds/fx/Z-BS13.wav"), false);
	_sndFeatureClick4 = sm->load(false, Common::Path("sounds/fx/Z-BS14.wav"), false);
	_sndWrongZoombini = sm->load(false, Common::Path("sounds/fx/wrongz.wav"), false);
}

void Zombiniville::createZoombini() {
	// Create a new zoombini with currently selected features
	bool allSelected = true;
	for (int i = 0; i < kNumFeatures; i++) {
		if (_stations[i].selectedValue == 0) {
			allSelected = false;
			break;
		}
	}

	if (!allSelected) {
		// Randomize any unset features
		Common::RandomSource *rnd = _engine->getRandom();
		for (int i = 0; i < kNumFeatures; i++) {
			if (_stations[i].selectedValue == 0) {
				_stations[i].selectedValue = rnd->getRandomNumber(4) + 1;
				_engine->_selectedFeatures[i] = _stations[i].selectedValue;
			}
		}
	}

	Zoombini *z = new Zoombini();
	z->setFeatures(
		_stations[0].selectedValue,
		_stations[1].selectedValue,
		_stations[2].selectedValue,
		_stations[3].selectedValue
	);
	z->computeHash();

	_engine->_globalZoombinis.push_back(z);
	_boardingZoombinis.push_back(z);

	debug(2, "Zombiniville: Created zoombini %u (features %d/%d/%d/%d)",
		z->_featureHash,
		_stations[0].selectedValue, _stations[1].selectedValue,
		_stations[2].selectedValue, _stations[3].selectedValue);

	// Reset feature selections after creating
	for (int i = 0; i < kNumFeatures; i++) {
		_stations[i].selectedValue = 0;
		_engine->_selectedFeatures[i] = -1;
	}
}

void Zombiniville::update() {
	// Phase 0: Waiting for zoombini creation
	// When 16 zoombinis are created, advance to departure
	if (_phase == 0 && (int)_boardingZoombinis.size() >= kMaxPackSize) {
		_phase = 2;
		debug(1, "Zombiniville: Pack full (%d), departing", (int)_boardingZoombinis.size());
		_engine->_routeDirection = 1; // Default left route
		_engine->requestPageChange(kPageMapTrans);
	}
}

void Zombiniville::draw(Graphics::ManagedSurface *screen) {
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw feature station buttons (4 groups × 5 values)
	// Original: each feature button is an Animation with selected/unselected states
	for (int grp = 0; grp < kNumFeatures; grp++) {
		for (int val = 0; val < kNumFeatureValues; val++) {
			Animation *anim = _featureButtons[grp][val];
			if (!anim || anim->getFrameCount() == 0)
				continue;

			// Frame 0 = normal, frame 1 = highlighted/selected (if available)
			int frameIdx = (_stations[grp].selectedValue == val + 1 && anim->getFrameCount() > 1) ? 1 : 0;
			const RleBlock *frame = anim->getFrame(frameIdx);
			if (frame) {
				Common::Point p = _stations[grp].drawPos[val];
				frame->drawToScreen(screen, p.x, p.y, lut);
			}
		}
	}

	// Draw BigZomb preview at center — original position (300, 110)
	// Drawing order: base body, feature1, feature3, feature4, feature2 (frontmost)
	if (_bigZombGfx) {
		// Cell 15844 = base body (index into 3000-cell array, mapped: 15844/16 = 990 rem 4)
		// Original addresses are byte offsets into the object, divide by 16 for cell index
		static const int kBaseCell = 15844 / 16;    // ~990
		static const int kFeat1Base = 15940 / 16;   // ~996
		static const int kFeat2Base = 16036 / 16;   // ~1002
		static const int kFeat3Base = 16132 / 16;   // ~1008
		static const int kFeat4Base = 16228 / 16;   // ~1014

		int bx = 300, by = 110;

		// Base body (always drawn)
		if (_bigZombGfx->getFrameCount(kBaseCell) > 0) {
			const RleBlock *frame = _bigZombGfx->getFrame(kBaseCell, 0);
			if (frame)
				frame->drawToScreen(screen, bx, by, lut);
		}

		// Feature overlays — draw order: 1, 3, 4, 2
		int featureOrder[] = {0, 2, 3, 1};
		int cellBases[] = {kFeat1Base, kFeat2Base, kFeat3Base, kFeat4Base};

		for (int i = 0; i < 4; i++) {
			int feat = featureOrder[i];
			int selVal = _stations[feat].selectedValue;
			if (selVal >= 1 && selVal <= 5) {
				int cell = cellBases[feat] + selVal;
				if (_bigZombGfx->getFrameCount(cell) > 0) {
					const RleBlock *frame = _bigZombGfx->getFrame(cell, 0);
					if (frame)
						frame->drawToScreen(screen, bx, by, lut);
				}
			}
		}
	}

	// Draw bumper indicators
	// bumper-1 shown when 1 zoombini is created, bumper-Valid when pack is valid
	int packCount = (int)_boardingZoombinis.size();
	if (packCount > 0 && packCount < kMaxPackSize && _bumper1 && _bumper1->getFrameCount() > 0) {
		const RleBlock *frame = _bumper1->getFrame(0);
		if (frame)
			frame->drawToScreen(screen, 10, 560, lut);
	}
	if (packCount >= kMaxPackSize && _bumperValid && _bumperValid->getFrameCount() > 0) {
		const RleBlock *frame = _bumperValid->getFrame(0);
		if (frame)
			frame->drawToScreen(screen, 10, 560, lut);
	}
}

void Zombiniville::handleClick(const Common::Point &pos) {
	// Check feature station buttons
	for (int feat = 0; feat < kNumFeatures; feat++) {
		for (int val = 0; val < kNumFeatureValues; val++) {
			if (_stations[feat].buttonRects[val].contains(pos)) {
				_stations[feat].selectedValue = val + 1;
				_engine->_selectedFeatures[feat] = val + 1;

				// Play feature click sound (original: 4 different sounds for features 1-4)
				int sndId = -1;
				switch (feat) {
				case 0: sndId = _sndFeatureSelect; break;
				case 1: sndId = _sndFeatureClick2; break;
				case 2: sndId = _sndFeatureClick3; break;
				case 3: sndId = _sndFeatureClick4; break;
				}
				if (sndId >= 0)
					_engine->getSoundManager()->play(sndId);

				debug(2, "Zombiniville: Feature %d set to %d", feat, val + 1);
				return;
			}
		}
	}

	// Check Go button — original: RouteButton3_Go_43C630
	if (_goRect.contains(pos)) {
		if ((int)_boardingZoombinis.size() < kMaxPackSize) {
			createZoombini();
		}
		if ((int)_boardingZoombinis.size() >= kMaxPackSize) {
			_phase = 2;
			_engine->_routeDirection = 1;
			_engine->requestPageChange(kPageMapTrans);
		}
		return;
	}

	// Quick Fill — create one random zoombini
	// Original: RouteButton1_QuickFill_43D100
	if (_quickFillRect.contains(pos)) {
		if ((int)_boardingZoombinis.size() < kMaxPackSize) {
			// Randomize all features
			Common::RandomSource *rnd = _engine->getRandom();
			for (int i = 0; i < kNumFeatures; i++) {
				_stations[i].selectedValue = rnd->getRandomNumber(4) + 1;
				_engine->_selectedFeatures[i] = _stations[i].selectedValue;
			}
			createZoombini();
		}
		return;
	}

	// Batch Fill — fill remaining slots with random zoombinis
	// Original: RouteButton2_BatchFill_43D1C0
	if (_batchFillRect.contains(pos)) {
		Common::RandomSource *rnd = _engine->getRandom();
		while ((int)_boardingZoombinis.size() < kMaxPackSize) {
			for (int i = 0; i < kNumFeatures; i++) {
				_stations[i].selectedValue = rnd->getRandomNumber(4) + 1;
				_engine->_selectedFeatures[i] = _stations[i].selectedValue;
			}
			createZoombini();
		}
		return;
	}
}

} // End of namespace Zoombini2
