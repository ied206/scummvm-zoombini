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

#ifndef ZOOMBINI2_UI_H
#define ZOOMBINI2_UI_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/path.h"
#include "common/str.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;

/**
 * UIButton — clickable button with normal/hover/disabled states.
 *
 * Original: Button struct (96 bytes) at unk_48AA68.
 * Init: Button__LoadImages_418F90
 * Draw/HitTest: Button__DrawAndHitTest_4191C0
 *
 * Layout (96 bytes):
 *   +4:  byte enabled
 *   +5:  byte forceShowDisabled
 *   +8:  DWORD x
 *   +12: DWORD y
 *   +16: DWORD width
 *   +20: DWORD height
 *   +24: char* pathNormal
 *   +28: char* pathHover
 *   +32: char* pathDisabled
 *   +36: char* maskPathNormal
 *   +40: char* maskPathHover
 *   +44: char* maskPathDisabled
 *   +48: DWORD wasHovering
 *   +52: byte hasMask
 *   +56: BitBlock* normalBB
 *   +60: BitBlock* hoverBB
 *   +64: BitBlock* disabledBB
 *   +68: RleBlock* normalRle
 *   +72: RleBlock* hoverRle
 *   +76: RleBlock* disabledRle
 *   +80: RleBlock* clipOverlay
 *   +84: DWORD* clipVarPtr
 *   +88: DWORD clipOffsetX
 *   +92: DWORD clipOffsetY
 */
class UIButton {
public:
	UIButton();
	~UIButton();

	/**
	 * Load button images. If maskPaths are provided, uses RleBlock.
	 * @param normalPath  Path to normal state image (without .bmp)
	 * @param hoverPath   Path to hover state image (without .bmp), or empty
	 * @param disabledPath Path to disabled state image, or empty
	 */
	bool loadImages(const Common::Path &normalPath,
	                const Common::Path &hoverPath = Common::Path(),
	                const Common::Path &disabledPath = Common::Path());

	/**
	 * Load button images with alpha masks (for RLE sprites).
	 */
	bool loadImagesWithMask(const Common::Path &normalPath, const Common::Path &normalMask,
	                        const Common::Path &hoverPath = Common::Path(),
	                        const Common::Path &hoverMask = Common::Path(),
	                        const Common::Path &disabledPath = Common::Path(),
	                        const Common::Path &disabledMask = Common::Path());

	/**
	 * Set button position and dimensions.
	 */
	void setRect(int x, int y, int width, int height);
	void setRect(const Common::Rect &rect);

	/**
	 * Set enabled state.
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }
	bool isEnabled() const { return _enabled; }

	/**
	 * Draw button and perform hit test.
	 * @param dst     Target surface
	 * @param mouseX  Current mouse X
	 * @param mouseY  Current mouse Y
	 * @param alphaLUT  Alpha blend lookup table
	 * @return 0 = no hover, 1 = new hover, 2 = continued hover
	 */
	int drawAndHitTest(Graphics::ManagedSurface *dst, int mouseX, int mouseY,
	                   const byte alphaLUT[256][256]);

	/**
	 * Check if point is within button bounds.
	 */
	bool containsPoint(int x, int y) const;
	bool containsPoint(const Common::Point &pt) const { return containsPoint(pt.x, pt.y); }

	const Common::Rect &getRect() const { return _rect; }
	bool wasHovering() const { return _wasHovering; }
	bool isHovering() const { return _isHovering; }

private:
	Common::Rect _rect;
	bool _enabled;
	bool _hasMask;
	bool _wasHovering;
	bool _isHovering;

	// BitBlock versions (no mask)
	BitBlock *_normalBB;
	BitBlock *_hoverBB;
	BitBlock *_disabledBB;

	// RleBlock versions (with mask)
	RleBlock *_normalRle;
	RleBlock *_hoverRle;
	RleBlock *_disabledRle;
};

/**
 * Menu button IDs corresponding to original button array.
 * 7 buttons at unk_47A850 (Z2-U) / unk_48AA68 (Z2-K).
 * Positions verified from Z2-U binary data.
 */
enum MenuButtonId {
	kMenuButtonNext     = 0,  // ArrowUP: scroll up (613, 350, 46, 50)
	kMenuButtonPrev     = 1,  // ArrowDOWN: scroll down (613, 416, 46, 50)
	kMenuButtonStart    = 2,  // Start: play selected save (27, 561, 145, 39)
	kMenuButtonOptions  = 3,  // Options: open options (175, 561, 145, 39)
	kMenuButtonNew      = 4,  // New: create new party (321, 561, 145, 39)
	kMenuButtonTraining = 5,  // Entrainement: training mode (468, 561, 145, 39)
	kMenuButtonQuit     = 6,  // Quitter: quit game (613, 561, 145, 39)
	kMenuButtonCount    = 7
};

/**
 * BitmapFont — bitmap-based font for UI text rendering.
 *
 * Original functions:
 *   BitmapFont__AllocAndLoad_4646A0 - Allocates glyph array
 *   BitmapFont__LoadGlyphs_464430   - Loads from "bmp/typo/bmt" + alpha
 *   BitmapFont__SetActive_464750    - Sets active font (dword_4E05AC)
 *   BitmapFont__DrawString_464B60   - Draws text with character mapping
 *
 * Character mapping (from DrawString_464B60):
 *   A-Z: indices 0-25
 *   a-z: indices 26-51
 *   0-9: indices 52-61
 *   Special chars (.,:;/()-+=@&#'?!*_): indices 62-80
 *   Space: advance 10 pixels
 *
 * Global active font: dword_4E05AC
 * Global glyph count: dword_4E08D0
 */
class BitmapFont {
public:
	static const int kNumGlyphs = 81;   // A-Z(26) + a-z(26) + 0-9(10) + special(19)
	static const int kSpaceWidth = 10;  // Pixels to advance for space

	BitmapFont();
	~BitmapFont();

	/**
	 * Load font glyphs from bitmap files.
	 * Original: BitmapFont__LoadGlyphs_464430(glyphs, r, g, b)
	 * @param basePath Base path without extension (e.g., "bmp/typo")
	 * @param r Red component (0-255) for glyph color
	 * @param g Green component (0-255) for glyph color
	 * @param b Blue component (0-255) for glyph color
	 * @return true on success
	 */
	bool load(const Common::Path &basePath, byte r, byte g, byte b);

	/**
	 * Draw text string to surface.
	 * @param dst       Target surface
	 * @param x         X position
	 * @param y         Y position
	 * @param text      Text to draw
	 * @param alphaLUT  Alpha blend lookup table
	 * @return Width of drawn text in pixels
	 */
	int drawString(Graphics::ManagedSurface *dst, int x, int y,
	               const Common::String &text, const byte alphaLUT[256][256]) const;

	/**
	 * Calculate width of text string without drawing.
	 * @param text Text to measure
	 * @return Width in pixels
	 */
	int getStringWidth(const Common::String &text) const;

	/**
	 * Get glyph index for character.
	 * @param c Character to look up
	 * @return Glyph index, or -1 if not found
	 */
	static int charToGlyphIndex(char c);

	bool isLoaded() const { return _loaded; }

private:
	bool _loaded;
	BitBlock *_glyphs[kNumGlyphs];  // Individual character glyphs (color + alpha)
};

enum VolumePanelResult {
	kVolumePanelOpen,
	kVolumePanelChanged,
	kVolumePanelApply,
	kVolumePanelCancel
};

/**
 * Volume sliders for music, sound effects, and speech.
 *
 * Slider changes are previewed while the panel remains open. Apply commits
 * all three values. Cancel restores the values captured when the panel opened.
 */
class VolumePanel {
public:
	static const int kSliderMinX = 350;    // Minimum X for slider (button_x + 193)
	static const int kSliderMaxX = 638;    // Maximum X (350 + 288)
	static const int kSliderRange = 288;   // Pixel range for slider

	// Label button positions.
	static const int kLabelX = 157;
	static const int kLabelW = 520;
	static const int kLabelH = 64;
	static const int kMusicLabelY = 224;
	static const int kSfxLabelY = 286;
	static const int kSpeechLabelY = 351;

	// Gauge draw Y positions (button_y + clipOffsetY)
	static const int kMusicGaugeY = 240;   // 224 + 16
	static const int kSfxGaugeY = 305;     // 286 + 19
	static const int kSpeechGaugeY = 374;  // 351 + 23

	VolumePanel();
	~VolumePanel();

	/**
	 * Initialize the panel resources.
	 * @return true on success
	 */
	bool init();

	/**
	 * Draw volume panel and handle input.
	 * @param dst       Target surface
	 * @param mouseX    Mouse X position
	 * @param mouseY    Mouse Y position
	 * @param mouseDown Whether the mouse button is held
	 * @param mouseClicked Whether the mouse button was pressed this frame
	 * @param alphaLUT  Alpha blend lookup table
	 * @return The current interaction result
	 */
	VolumePanelResult drawAndHandleInput(Graphics::ManagedSurface *dst, int mouseX, int mouseY,
	                                     bool mouseDown, bool mouseClicked,
	                                     const byte alphaLUT[256][256]);

	/**
	 * Get current volume values (0-100).
	 */
	int getMusicVolume() const { return _musicVolume; }
	int getSfxVolume() const { return _sfxVolume; }
	int getSpeechVolume() const { return _speechVolume; }
	int getInitialMusicVolume() const { return _initialMusicVolume; }
	int getInitialSfxVolume() const { return _initialSfxVolume; }
	int getInitialSpeechVolume() const { return _initialSpeechVolume; }

	/**
	 * Set volume values (0-100).
	 */
	void setMusicVolume(int vol);
	void setSfxVolume(int vol);
	void setSpeechVolume(int vol);
	void setInitialVolumes(int music, int sfx, int speech);

	/**
	 * Convert between pixel position and volume percentage.
	 */
	static int pixelToVolume(int x);
	static int volumeToPixel(int volume);

private:
	int _musicVolume;
	int _sfxVolume;
	int _speechVolume;
	int _initialMusicVolume;
	int _initialSfxVolume;
	int _initialSpeechVolume;

	int _musicSliderX;
	int _sfxSliderX;
	int _speechSliderX;

	int _activeSlider;  // -1 = none, 0 = music, 1 = sfx, 2 = speech

	RleBlock *_gaugeImage;
	UIButton _sliderLabels[3];  // Music, SFX, Speech label buttons
	UIButton _okButton;
	UIButton _noButton;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_UI_H
