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
#include "common/file.h"

#include "zoombini2/ui.h"
#include "zoombini2/gfx.h"

namespace Zoombini2 {

// ============================================================================
// UIButton — clickable button with normal/hover/disabled states.
// Original: Button__LoadImages_418F90, Button__DrawAndHitTest_4191C0
// ============================================================================

UIButton::UIButton()
	: _rect(0, 0, 0, 0),
	  _enabled(true),
	  _hasMask(false),
	  _wasHovering(false),
	  _isHovering(false),
	  _normalBB(nullptr),
	  _hoverBB(nullptr),
	  _disabledBB(nullptr),
	  _normalRle(nullptr),
	  _hoverRle(nullptr),
	  _disabledRle(nullptr) {
}

UIButton::~UIButton() {
	delete _normalBB;
	delete _hoverBB;
	delete _disabledBB;
	delete _normalRle;
	delete _hoverRle;
	delete _disabledRle;
}

bool UIButton::loadImages(const Common::Path &normalPath,
                          const Common::Path &hoverPath,
                          const Common::Path &disabledPath) {
	_hasMask = false;

	// Helper lambda to try loading as BitBlock or RleBlock
	auto tryLoadImage = [](const Common::Path &basePath, BitBlock **bb, RleBlock **rle) -> bool {
		if (basePath.empty()) return true;  // Skip empty paths

		// First try .bb or .bmp (BitBlock format)
		*bb = new BitBlock();
		if ((*bb)->load(basePath)) {
			return true;
		}
		delete *bb;
		*bb = nullptr;

		// Try .rb (RleBlock format)
		Common::Path rbPath(basePath.toString() + ".rb");
		*rle = new RleBlock();
		if ((*rle)->loadFromFile(rbPath)) {
			return true;
		}
		delete *rle;
		*rle = nullptr;

		return false;
	};

	// Load normal state (required)
	if (!tryLoadImage(normalPath, &_normalBB, &_normalRle)) {
		warning("UIButton: Failed to load normal image: %s", normalPath.toString().c_str());
		return false;
	}

	// Set button size from image if not already set
	if (_rect.width() == 0) {
		if (_normalBB) {
			_rect.setWidth(_normalBB->getWidth());
			_rect.setHeight(_normalBB->getHeight());
		} else if (_normalRle) {
			_rect.setWidth(_normalRle->getWidth());
			_rect.setHeight(_normalRle->getHeight());
		}
	}

	// Load hover state (optional)
	if (!tryLoadImage(hoverPath, &_hoverBB, &_hoverRle)) {
		// Non-fatal for hover state
		debug(1, "UIButton: No hover image for: %s", hoverPath.toString().c_str());
	}

	// Load disabled state (optional)
	if (!tryLoadImage(disabledPath, &_disabledBB, &_disabledRle)) {
		// Non-fatal for disabled state
		debug(1, "UIButton: No disabled image for: %s", disabledPath.toString().c_str());
	}

	return _normalBB != nullptr || _normalRle != nullptr;
}

bool UIButton::loadImagesWithMask(const Common::Path &normalPath, const Common::Path &normalMask,
                                  const Common::Path &hoverPath, const Common::Path &hoverMask,
                                  const Common::Path &disabledPath, const Common::Path &disabledMask) {
	_hasMask = true;

	// Load normal state with mask (required)
	if (!normalPath.empty() && !normalMask.empty()) {
		_normalBB = new BitBlock();
		if (!_normalBB->loadFromBMPPair(normalPath, normalMask)) {
			warning("UIButton: Failed to load normal image with mask");
			delete _normalBB;
			_normalBB = nullptr;
			return false;
		}
		if (_rect.width() == 0) {
			_rect.setWidth(_normalBB->getWidth());
			_rect.setHeight(_normalBB->getHeight());
		}
	}

	// Load hover state with mask (optional)
	if (!hoverPath.empty() && !hoverMask.empty()) {
		_hoverBB = new BitBlock();
		if (!_hoverBB->loadFromBMPPair(hoverPath, hoverMask)) {
			warning("UIButton: Failed to load hover image with mask");
			delete _hoverBB;
			_hoverBB = nullptr;
		}
	}

	// Load disabled state with mask (optional)
	if (!disabledPath.empty() && !disabledMask.empty()) {
		_disabledBB = new BitBlock();
		if (!_disabledBB->loadFromBMPPair(disabledPath, disabledMask)) {
			warning("UIButton: Failed to load disabled image with mask");
			delete _disabledBB;
			_disabledBB = nullptr;
		}
	}

	return _normalBB != nullptr;
}

void UIButton::setRect(int x, int y, int width, int height) {
	_rect = Common::Rect(x, y, x + width, y + height);
}

void UIButton::setRect(const Common::Rect &rect) {
	_rect = rect;
}

bool UIButton::containsPoint(int x, int y) const {
	return _rect.contains(x, y);
}

int UIButton::drawAndHitTest(Graphics::ManagedSurface *dst, int mouseX, int mouseY,
                             const byte alphaLUT[256][256]) {
	// Original: Button__DrawAndHitTest_4191C0
	_wasHovering = _isHovering;
	_isHovering = false;

	// Hit test if enabled
	if (_enabled) {
		if (mouseX > _rect.left && mouseX < _rect.right &&
		    mouseY > _rect.top && mouseY < _rect.bottom) {
			_isHovering = true;
		}
	}

	// Select image to draw
	BitBlock *imgToDraw = nullptr;
	RleBlock *rleToDraw = nullptr;

	if (_enabled) {
		if (_isHovering && _hoverBB) {
			imgToDraw = _hoverBB;
		} else if (_isHovering && _hoverRle) {
			rleToDraw = _hoverRle;
		} else if (_normalBB) {
			imgToDraw = _normalBB;
		} else if (_normalRle) {
			rleToDraw = _normalRle;
		}
	} else {
		// Disabled state
		if (_disabledBB) {
			imgToDraw = _disabledBB;
		} else if (_disabledRle) {
			rleToDraw = _disabledRle;
		} else if (_normalBB) {
			// Fall back to normal but draw as disabled
			imgToDraw = _normalBB;
		} else if (_normalRle) {
			rleToDraw = _normalRle;
		}
	}

	// Draw
	if (imgToDraw) {
		if (imgToDraw->hasAlpha()) {
			imgToDraw->drawAlphaBlend(dst, _rect.left, _rect.top, alphaLUT);
		} else {
			imgToDraw->drawToSurface(dst, _rect.left, _rect.top);
		}
	} else if (rleToDraw) {
		rleToDraw->drawToScreen(dst, _rect.left, _rect.top, alphaLUT);
	}

	// Return hover state change
	// 0 = no hover, 1 = new hover (just started), 2 = continued hover
	if (_isHovering) {
		return _wasHovering ? 2 : 1;
	}
	return 0;
}

// ============================================================================
// BitmapFont — bitmap-based font for UI text rendering.
// Original: BitmapFont__LoadGlyphs_464430, BitmapFont__DrawString_464B60
// ============================================================================

// Special character mapping table (from BitmapFont__DrawString_464B60)
// Maps special characters to glyph indices 62-80
static const struct {
	char ch;
	int index;
} kSpecialChars[] = {
	{'.', 62}, {',', 63}, {';', 64}, {':', 65}, {'/', 66},
	{'(', 67}, {')', 68}, {'-', 69}, {'+', 70}, {'=', 71},
	{'@', 72}, {'&', 73}, {'#', 74}, {'\'', 75}, {'?', 76},
	{'!', 77}, {'*', 78}, {'_', 79}, {'"', 80},
	{0, -1}
};

BitmapFont::BitmapFont() : _loaded(false) {
	for (int i = 0; i < kNumGlyphs; i++) {
		_glyphs[i] = nullptr;
	}
}

BitmapFont::~BitmapFont() {
	for (int i = 0; i < kNumGlyphs; i++) {
		delete _glyphs[i];
	}
}

bool BitmapFont::load(const Common::Path &basePath, byte r, byte g, byte b) {
	// Original: BitmapFont__LoadGlyphs_464430
	// Loads alpha from "bmp/typo-A.bmt", fills color with (r, g, b).
	// The color BMP is not actually used for the font pixels in the original;
	// only the alpha channel matters. Pixels are filled with the given color.

	// BMT files are actually BMP files with a .bmt extension
	Common::Path alphaPath = basePath.getParent().appendComponent(
		basePath.baseName() + "-A.bmt");

	// Load alpha BMP into a BitBlock (we only need the alpha channel)
	BitBlock alphaBB;
	if (!alphaBB.loadFromBMPPair(
			basePath.append(".bmt"),
			alphaPath)) {
		warning("BitmapFont: Failed to load BMP pair from %s", basePath.toString().c_str());
		return false;
	}

	const byte *srcAlpha = alphaBB.getAlpha();
	if (!srcAlpha) {
		warning("BitmapFont: No alpha channel in font BMP");
		return false;
	}

	int width = alphaBB.getWidth();
	int height = alphaBB.getHeight();

	// Scan columns of the alpha map to find glyph boundaries.
	// A column is "blank" when every alpha pixel in that column is 0.
	int glyphIndex = 0;
	int col = 0;

	while (col < width && glyphIndex < kNumGlyphs) {
		// Skip blank columns
		while (col < width) {
			bool blank = true;
			for (int row = 0; row < height; row++) {
				if (srcAlpha[row * width + col] != 0) {
					blank = false;
					break;
				}
			}
			if (!blank)
				break;
			col++;
		}

		if (col >= width)
			break;

		// Found start of glyph — advance to the next blank column
		int startCol = col;
		while (col < width) {
			bool blank = true;
			for (int row = 0; row < height; row++) {
				if (srcAlpha[row * width + col] != 0) {
					blank = false;
					break;
				}
			}
			if (blank)
				break;
			col++;
		}

		// Original adds +2 padding to glyph width
		int glyphW = col - startCol + 2;

		// Create a BitBlock for this glyph with the extracted alpha
		// and the requested solid color.
		BitBlock *glyph = new BitBlock();
		glyph->createEmpty(glyphW, height, true);

		// Copy alpha sub-region row by row
		byte *dstAlpha = const_cast<byte *>(glyph->getAlpha());
		for (int row = 0; row < height; row++) {
			int copyW = col - startCol;  // actual data width (no padding)
			memcpy(dstAlpha + row * glyphW,
			       srcAlpha + row * width + startCol,
			       copyW);
			// Padding columns stay 0 (transparent) from createEmpty
		}

		// Fill pixel data with the specified (r, g, b) color
		byte *dstPixels = const_cast<byte *>(glyph->getPixels());
		int totalPixels = glyphW * height;
		for (int i = 0; i < totalPixels; i++) {
			dstPixels[i * 4 + 0] = r;
			dstPixels[i * 4 + 1] = g;
			dstPixels[i * 4 + 2] = b;
			dstPixels[i * 4 + 3] = 255;
		}

		_glyphs[glyphIndex] = glyph;
		glyphIndex++;
	}

	_loaded = true;
	debug(1, "BitmapFont: Loaded %d glyphs from %s (color %d,%d,%d)",
	      glyphIndex, basePath.toString().c_str(), r, g, b);

	return true;
}

int BitmapFont::charToGlyphIndex(char c) {
	// Original: BitmapFont__DrawString_464B60 character mapping
	if (c >= 'A' && c <= 'Z') {
		return c - 'A';  // 0-25
	}
	if (c >= 'a' && c <= 'z') {
		return c - 'a' + 26;  // 26-51
	}
	if (c >= '0' && c <= '9') {
		return c - '0' + 52;  // 52-61
	}

	// Check special characters
	for (int i = 0; kSpecialChars[i].ch != 0; i++) {
		if (kSpecialChars[i].ch == c) {
			return kSpecialChars[i].index;
		}
	}

	return -1;  // Not found (treat as space)
}

int BitmapFont::drawString(Graphics::ManagedSurface *dst, int x, int y,
                           const Common::String &text, const byte alphaLUT[256][256]) const {
	if (!_loaded) {
		return 0;
	}

	int curX = x;

	for (uint i = 0; i < text.size(); i++) {
		char c = text[i];

		if (c == ' ') {
			curX += kSpaceWidth;
			continue;
		}

		int glyphIdx = charToGlyphIndex(c);
		if (glyphIdx < 0 || glyphIdx >= kNumGlyphs || !_glyphs[glyphIdx]) {
			curX += kSpaceWidth;  // Unknown character, treat as space
			continue;
		}

		_glyphs[glyphIdx]->drawAlphaBlend(dst, curX, y, alphaLUT);
		curX += _glyphs[glyphIdx]->getWidth();
	}

	return curX - x;
}

int BitmapFont::getStringWidth(const Common::String &text) const {
	if (!_loaded) {
		return 0;
	}

	int width = 0;

	for (uint i = 0; i < text.size(); i++) {
		char c = text[i];

		if (c == ' ') {
			width += kSpaceWidth;
			continue;
		}

		int glyphIdx = charToGlyphIndex(c);
		if (glyphIdx < 0 || glyphIdx >= kNumGlyphs || !_glyphs[glyphIdx]) {
			width += kSpaceWidth;
			continue;
		}

		width += _glyphs[glyphIdx]->getWidth();
	}

	return width;
}

// ============================================================================
// VolumePanel
// ============================================================================

VolumePanel::VolumePanel()
	: _musicVolume(100),
	  _sfxVolume(100),
	  _speechVolume(100),
	  _initialMusicVolume(100),
	  _initialSfxVolume(100),
	  _initialSpeechVolume(100),
	  _musicSliderX(kSliderMaxX),
	  _sfxSliderX(kSliderMaxX),
	  _speechSliderX(kSliderMaxX),
	  _activeSlider(-1),
	  _gaugeImage(nullptr) {
}

VolumePanel::~VolumePanel() {
	delete _gaugeImage;
}

bool VolumePanel::init() {
	// Load the gauge image from its color and alpha pair.
	_gaugeImage = new RleBlock();
	if (!_gaugeImage->loadFromFile(Common::Path("bmp/menu/OPTION - Jauge.rb"))) {
		warning("VolumePanel: Failed to load gauge image");
		delete _gaugeImage;
		_gaugeImage = nullptr;
	}

	// Load the completion buttons.
	_okButton.setRect(294, 487, 76, 74);
	_okButton.loadImages(
		Common::Path("bmp/menu/MENU - Valid - OK"),
		Common::Path("bmp/menu/MENU - Valid - OK highlight"));

	_noButton.setRect(468, 487, 76, 74);
	_noButton.loadImages(
		Common::Path("bmp/menu/MENU - Valid - NO"),
		Common::Path("bmp/menu/MENU - Valid - NO highlight"));

	// Load the slider labels.
	// Music label
	_sliderLabels[0].setRect(kLabelX, kMusicLabelY, kLabelW, kLabelH);
	_sliderLabels[0].loadImages(
		Common::Path("bmp/menu/OPTION - Musique NORMAL"),
		Common::Path("bmp/menu/OPTION - Musique HIGHLIGHT"));

	// SFX label
	_sliderLabels[1].setRect(kLabelX, kSfxLabelY, kLabelW, kLabelH);
	_sliderLabels[1].loadImages(
		Common::Path("bmp/menu/OPTION - Bruitages NORMAL"),
		Common::Path("bmp/menu/OPTION - Bruitages HILITE"));

	// Speech label
	_sliderLabels[2].setRect(kLabelX, kSpeechLabelY, kLabelW, kLabelH);
	_sliderLabels[2].loadImages(
		Common::Path("bmp/menu/OPTION - Dialogues NORMAL"),
		Common::Path("bmp/menu/OPTION - Dialogues HILITE"));

	// Initialize slider positions from current volumes.
	_musicSliderX = volumeToPixel(_musicVolume);
	_sfxSliderX = volumeToPixel(_sfxVolume);
	_speechSliderX = volumeToPixel(_speechVolume);

	return true;
}

int VolumePanel::pixelToVolume(int x) {
	// Convert the 288-pixel gauge range to a percentage.
	int vol = (100 * (x - kSliderMinX)) / kSliderRange;
	if (vol < 0) vol = 0;
	if (vol > 100) vol = 100;
	return vol;
}

int VolumePanel::volumeToPixel(int volume) {
	return (kSliderRange * volume) / 100 + kSliderMinX;
}

void VolumePanel::setMusicVolume(int vol) {
	_musicVolume = CLIP(vol, 0, 100);
	_musicSliderX = volumeToPixel(_musicVolume);
}

void VolumePanel::setSfxVolume(int vol) {
	_sfxVolume = CLIP(vol, 0, 100);
	_sfxSliderX = volumeToPixel(_sfxVolume);
}

void VolumePanel::setSpeechVolume(int vol) {
	_speechVolume = CLIP(vol, 0, 100);
	_speechSliderX = volumeToPixel(_speechVolume);
}

void VolumePanel::setInitialVolumes(int music, int sfx, int speech) {
	setMusicVolume(music);
	setSfxVolume(sfx);
	setSpeechVolume(speech);
	_initialMusicVolume = _musicVolume;
	_initialSfxVolume = _sfxVolume;
	_initialSpeechVolume = _speechVolume;
}

VolumePanelResult VolumePanel::drawAndHandleInput(Graphics::ManagedSurface *dst,
                                                  int mouseX, int mouseY,
                                                  bool mouseDown, bool mouseClicked,
                                                  const byte alphaLUT[256][256]) {
	bool volumeChanged = false;

	// Handle slider input
	if (mouseDown) {
		// Check which slider is being dragged
		if (_activeSlider < 0) {
			// Check if clicking within slider label button areas
			if (kMusicLabelY <= mouseY && mouseY < kMusicLabelY + kLabelH) {
				_activeSlider = 0;  // Music
			} else if (kSfxLabelY <= mouseY && mouseY < kSfxLabelY + kLabelH) {
				_activeSlider = 1;  // SFX
			} else if (kSpeechLabelY <= mouseY && mouseY < kSpeechLabelY + kLabelH) {
				_activeSlider = 2;  // Speech
			}
		}

		// Update active slider position
		if (0 <= _activeSlider) {
			int newX = CLIP(mouseX, kSliderMinX, kSliderMaxX);
			int newVol = pixelToVolume(newX);

			switch (_activeSlider) {
			case 0:
				if (_musicVolume != newVol) {
					_musicVolume = newVol;
					_musicSliderX = newX;
					volumeChanged = true;
				}
				break;
			case 1:
				if (_sfxVolume != newVol) {
					_sfxVolume = newVol;
					_sfxSliderX = newX;
					volumeChanged = true;
				}
				break;
			case 2:
				if (_speechVolume != newVol) {
					_speechVolume = newVol;
					_speechSliderX = newX;
					volumeChanged = true;
				}
				break;
			}
		}
	} else {
		_activeSlider = -1;
	}

	// Draw slider label buttons (with hover state)
	for (int i = 0; i < 3; i++) {
		_sliderLabels[i].drawAndHitTest(dst, mouseX, mouseY, alphaLUT);
	}

	// Clip each gauge at its current slider position.
	if (_gaugeImage) {
		_gaugeImage->drawToScreenClipped(dst, kSliderMinX, kMusicGaugeY,
		                                 0, 0, _musicSliderX, 600, alphaLUT);
		_gaugeImage->drawToScreenClipped(dst, kSliderMinX, kSfxGaugeY,
		                                 0, 0, _sfxSliderX, 600, alphaLUT);
		_gaugeImage->drawToScreenClipped(dst, kSliderMinX, kSpeechGaugeY,
		                                 0, 0, _speechSliderX, 600, alphaLUT);
	}

	// Draw OK/NO buttons (with hover state)
	_okButton.drawAndHitTest(dst, mouseX, mouseY, alphaLUT);
	_noButton.drawAndHitTest(dst, mouseX, mouseY, alphaLUT);

	if (mouseClicked && _okButton.containsPoint(mouseX, mouseY))
		return kVolumePanelApply;
	if (mouseClicked && _noButton.containsPoint(mouseX, mouseY))
		return kVolumePanelCancel;
	return volumeChanged ? kVolumePanelChanged : kVolumePanelOpen;
}

} // End of namespace Zoombini2
