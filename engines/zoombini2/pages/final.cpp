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

#include "zoombini2/pages/final.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// FinalPage — final celebration (page ID 23).
// Original: Final__Init_418740 (0x418740), object size 0x54 (84 bytes).
//
// Resources from IDA decompilation:
//   Background: "bmp/final/big BOOL.bb" via Scene__LoadBackground
//   Overlay: "bmp/final/thefullbigbool.bb" drawn at (225, 34)
//   Dancing boolies: "bmp/final/dancing_boolie.an" (10 frames 0-9, 100ms each)
//     Positions: [(100, 530), (300, 530), (400, 530)] from dword_48A7F8/FC
//   Boolance: "bmp/final/booldance.an" (9 frames 0-8, 200ms each) at (151, 31)
//   Fireworks: "bmp/final/FWBLUE.an", "FWRED.an", "FWGREEN.an" (36 frames, 40ms)
//     Position: all at (50, 50)
//   Firefly anims: "bmp/aquacube/fla*.an" (reused from Aquacube puzzle)
//   Ambient sounds: zbv425.wav + blw22.1-6.wav (random interval)
//   Music: "sounds/music/Booliewood_Finale.wav" (looped)
//   Speech: "fin11"
//
// After init, transitions to Credits page (16). In original engine, Final and
// Credits coexist — Credits__TickHandler uses Final's scene for animations.
// ============================================================================

// Dancing boolie positions (from dword_48A7F8/FC arrays in IDA at 0x418740)
// Original values: [(100, 530), (300, 530), (400, 530)]
static const int kBoolieX[] = {100, 300, 400};
static const int kBoolieY[] = {530, 530, 530};

FinalPage::FinalPage(Zoombini2Engine *engine)
	: Page(engine), _background(nullptr), _fullBigBool(nullptr),
	  _dancingBoolie(nullptr), _boolDance(nullptr),
	  _fwBlue(nullptr), _fwRed(nullptr), _fwGreen(nullptr),
	  _animFrame(0), _lastFrameTime(0), _startTime(0), _finished(false), _speechPlayed(false),
	  _musicId(-1), _nextAmbientTime(0) {
	_pageId = kPageFinal;
	for (int i = 0; i < kAmbientSoundCount; i++)
		_ambientSoundIds[i] = -1;
}

FinalPage::~FinalPage() {
	SoundManager *sm = _engine->getSoundManager();
	if (_musicId >= 0) {
		sm->stop(_musicId);
		sm->unload(_musicId);
	}
	// Unload ambient sounds
	for (int i = 0; i < kAmbientSoundCount; i++) {
		if (_ambientSoundIds[i] >= 0)
			sm->unload(_ambientSoundIds[i]);
	}
	delete _background;
	delete _fullBigBool;
	delete _dancingBoolie;
	delete _boolDance;
	delete _fwBlue;
	delete _fwRed;
	delete _fwGreen;
}

void FinalPage::init() {
	debug(1, "FinalPage::init");

	// Load background: "bmp/final/big BOOL" (.bb)
	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/final/big BOOL"))) {
		warning("FinalPage: Failed to load background");
	}

	// Load overlay: "bmp/final/thefullbigbool" (.bb)
	_fullBigBool = new BitBlock();
	if (!_fullBigBool->load(Common::Path("bmp/final/thefullbigbool"))) {
		warning("FinalPage: Failed to load fullbigbool overlay");
	}

	// Load animations
	_dancingBoolie = new Animation();
	_dancingBoolie->loadFromFile(Common::Path("bmp/final/dancing_boolie.an"));

	_boolDance = new Animation();
	_boolDance->loadFromFile(Common::Path("bmp/final/booldance.an"));

	_fwBlue = new Animation();
	_fwBlue->loadFromFile(Common::Path("bmp/final/FWBLUE.an"));

	_fwRed = new Animation();
	_fwRed->loadFromFile(Common::Path("bmp/final/FWRED.an"));

	_fwGreen = new Animation();
	_fwGreen->loadFromFile(Common::Path("bmp/final/FWGREEN.an"));

	_animFrame = 0;
	_startTime = _engine->getGameTickCount();
	_lastFrameTime = _startTime;
	_finished = false;
	_speechPlayed = false;

	// BGM: Booliewood_Finale.wav (IDA: aSoundsMusicBoo_0 at Final__Init_418740)
	SoundManager *sm = _engine->getSoundManager();
	_musicId = sm->load(true, Common::Path("sounds/music/Booliewood_Finale.wav"), true);
	if (_musicId >= 0) {
		sm->playLoop(_musicId);
		sm->setVolume(_musicId, sm->_volumeMusic);
	}

	// Play victory speech "fin11"
	int speechId = sm->load(false, Common::Path("sounds/speech/fin11.wav"), false);
	if (speechId >= 0) {
		sm->play(speechId);
		_speechPlayed = true;
	}

	// Load ambient sounds (IDA: this+0 to this+6)
	// zbv425.wav (index 0) + blw22.1-6.wav (indices 1-6)
	_ambientSoundIds[0] = sm->load(true, Common::Path("sounds/zbv425.wav"), false);
	for (int i = 1; i < kAmbientSoundCount; i++) {
		Common::String filename = Common::String::format("sounds/blw22.%d.wav", i);
		_ambientSoundIds[i] = sm->load(true, Common::Path(filename), false);
	}

	// Schedule first ambient sound (IDA: 125 * (rand() % 10 + 10) * 8 = 10000-19000ms)
	scheduleNextAmbient();
}

void FinalPage::update() {
	uint32 now = _engine->getGameTickCount();

	// Advance animation frame (100ms per frame for dancing boolies)
	if (now - _lastFrameTime >= 100) {
		_animFrame++;
		_lastFrameTime = now;
	}

	// Check ambient sound timer
	if (_nextAmbientTime > 0 && now >= _nextAmbientTime) {
		playRandomAmbient();
		scheduleNextAmbient();
	}

	// Auto-transition to credits after 15 seconds
	if (now - _startTime > 15000 && !_finished) {
		_finished = true;
		_engine->requestPageChange(kPageCredits);
	}
}

void FinalPage::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	// Draw fullbigbool overlay at (225, 34)
	if (_fullBigBool) {
		_fullBigBool->drawToSurface(screen, 225, 34);
	}

	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw boolance at (151, 31) — 9 frames, 200ms each
	if (_boolDance && _boolDance->getFrameCount() > 0) {
		int boolFrame = (_animFrame / 2) % _boolDance->getFrameCount();
		const RleBlock *frame = _boolDance->getFrame(boolFrame);
		if (frame)
			frame->drawToScreen(screen, 151, 31, lut);
	}

	// Draw 3 dancing boolies — 10 frames, 100ms each
	if (_dancingBoolie && _dancingBoolie->getFrameCount() > 0) {
		for (int i = 0; i < 3; i++) {
			int boolieFrame = (_animFrame + i * 3) % _dancingBoolie->getFrameCount();
			const RleBlock *frame = _dancingBoolie->getFrame(boolieFrame);
			if (frame)
				frame->drawToScreen(screen, kBoolieX[i], kBoolieY[i], lut);
		}
	}

	// Draw fireworks — 36 frames, 40ms each (roughly every other main frame)
	// Original creates all at (50, 50) via Scene__CreateAnimElement, but the scene system
	// may reposition them. Using spread positions for visual variety until verified.
	// TODO: Verify original firework positions/movement behavior
	int fwFrame = (_animFrame * 100 / 40); // Scale to 40ms timing
	if (_fwBlue && _fwBlue->getFrameCount() > 0) {
		int f = fwFrame % _fwBlue->getFrameCount();
		const RleBlock *frame = _fwBlue->getFrame(f);
		if (frame)
			frame->drawToScreen(screen, 50, 50, lut);
	}

	if (_fwRed && _fwRed->getFrameCount() > 0) {
		int f = (fwFrame + 12) % _fwRed->getFrameCount();
		const RleBlock *frame = _fwRed->getFrame(f);
		if (frame)
			frame->drawToScreen(screen, 350, 50, lut);  // Spread for variety
	}

	if (_fwGreen && _fwGreen->getFrameCount() > 0) {
		int f = (fwFrame + 24) % _fwGreen->getFrameCount();
		const RleBlock *frame = _fwGreen->getFrame(f);
		if (frame)
			frame->drawToScreen(screen, 600, 50, lut);  // Spread for variety
	}
}

void FinalPage::handleClick(const Common::Point &pos) {
	// Skip to credits on click
	if (!_finished) {
		_finished = true;
		_engine->requestPageChange(kPageCredits);
	}
}

void FinalPage::scheduleNextAmbient() {
	// IDA: 125 * (rand() % 10 + 10) * 8 = 10000-19000ms interval
	Common::RandomSource *rnd = _engine->getRandom();
	int delay = 125 * ((rnd->getRandomNumber(9) + 10)) * 8;
	_nextAmbientTime = _engine->getGameTickCount() + delay;
}

void FinalPage::playRandomAmbient() {
	// Pick a random ambient sound and play it
	SoundManager *sm = _engine->getSoundManager();
	Common::RandomSource *rnd = _engine->getRandom();
	int idx = rnd->getRandomNumber(kAmbientSoundCount - 1);
	if (_ambientSoundIds[idx] >= 0) {
		sm->play(_ambientSoundIds[idx]);
	}
}

} // End of namespace Zoombini2
