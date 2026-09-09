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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_UI_H
#define ZOOMBINI2_UI_H

#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;

/**
 * Owns the normal, highlighted, and disabled images for one button.
 *
 * A button may use an uncompressed @ref BitBlock or an @ref RleBlock for each
 * state. Its draw call also performs the frame's hit test and reports whether
 * the pointer has just entered or remains inside the enabled button.
 */
class UIButton {
public:
	/** Construct an enabled button with an empty rectangle and no images. */
	UIButton();
	/** Release every image owned by the button. */
	~UIButton();

	/**
	 * Load normal and optional alternate images without separate alpha masks.
	 *
	 * Each non-empty path is tried as a BitBlock and then as an RLE block.
	 * The normal state is required; missing highlighted or disabled states are
	 * non-fatal and fall back during drawing.
	 */
	bool loadImages(const Common::Path &normalPath, const Common::Path &hoverPath = Common::Path(),
					const Common::Path &disabledPath = Common::Path());

	/** Load normal and optional alternate color-and-alpha BMP pairs. */
	bool loadImagesWithMask(const Common::Path &normalPath, const Common::Path &normalMask,
							const Common::Path &hoverPath = Common::Path(), const Common::Path &hoverMask = Common::Path(),
							const Common::Path &disabledPath = Common::Path(), const Common::Path &disabledMask = Common::Path());

	/** Set the button rectangle from a position and size. */
	void setRect(int x, int y, int width, int height);
	/** Replace the button rectangle with @p rect. */
	void setRect(const Common::Rect &rect);
	/** Enable or disable pointer interaction. */
	void setEnabled(bool enabled) { _enabled = enabled; }
	/** Return whether pointer interaction is enabled. */
	bool isEnabled() const { return _enabled; }

	/**
	 * Draw the state selected by @p mouseX and @p mouseY.
	 *
	 * @return Zero when not hovered, one when newly hovered, or two when the
	 * pointer remains over the button from the preceding draw.
	 */
	int drawAndHitTest(Graphics::ManagedSurface *dst, int mouseX, int mouseY, const byte alphaLUT[256][256]);

	/** Return whether the coordinates are inside the button rectangle. */
	bool containsPoint(int x, int y) const;
	/** Return whether @p point is inside the button rectangle. */
	bool containsPoint(const Common::Point &point) const { return containsPoint(point.x, point.y); }

	/** Return the button rectangle. */
	const Common::Rect &getRect() const { return _rect; }
	/** Return whether the pointer was over the button during the preceding draw. */
	bool wasHovering() const { return _wasHovering; }
	/** Return whether the pointer was over the button during the current draw. */
	bool isHovering() const { return _isHovering; }

private:
	/** Button position and hit-test bounds. */
	Common::Rect _rect;
	/** Whether the button responds to pointer input. */
	bool _enabled;
	/** Whether the loaded image set was requested with separate masks. */
	bool _hasMask;
	/** Hover state retained from the preceding draw. */
	bool _wasHovering;
	/** Hover state computed during the current draw. */
	bool _isHovering;

	/** Owned uncompressed normal-state image. */
	BitBlock *_normalBB;
	/** Owned uncompressed highlighted-state image. */
	BitBlock *_hoverBB;
	/** Owned uncompressed disabled-state image. */
	BitBlock *_disabledBB;
	/** Owned RLE normal-state image. */
	RleBlock *_normalRle;
	/** Owned RLE highlighted-state image. */
	RleBlock *_hoverRle;
	/** Owned RLE disabled-state image. */
	RleBlock *_disabledRle;
};

/** Button indices used by the seven controls on the sign-in screen. */
enum MenuButtonId {
	kMenuButtonNext = 0,     ///< Scroll toward the preceding visible profile rows.
	kMenuButtonPrev = 1,     ///< Scroll toward the following visible profile rows.
	kMenuButtonStart = 2,    ///< Start the selected saved adventure.
	kMenuButtonOptions = 3,  ///< Open the volume panel.
	kMenuButtonNew = 4,      ///< Begin entry of a new profile name.
	kMenuButtonTraining = 5, ///< Open the practice map.
	kMenuButtonQuit = 6,     ///< Request exit from the sign-in screen.
	kMenuButtonCount = 7     ///< Number of sign-in buttons.
};

/**
 * Owns and draws the 81 glyphs extracted from one bitmap-font strip.
 *
 * The supported glyph sequence contains uppercase letters, lowercase letters,
 * digits, and nineteen punctuation characters. Spaces and unsupported bytes
 * advance by @ref BitmapFont::kSpaceWidth without drawing.
 */
class BitmapFont {
public:
	/** Number of glyphs in the fixed font-strip mapping. */
	static const int kNumGlyphs = 81;
	/** Horizontal advance used for spaces and unsupported characters. */
	static const int kSpaceWidth = 10;

	/** Construct an unloaded font. */
	BitmapFont();
	/** Release every extracted glyph bitmap. */
	~BitmapFont();

	/** Load a BMT color-and-alpha pair and color its extracted glyphs. */
	bool load(const Common::Path &basePath, byte red, byte green, byte blue);
	/** Draw @p text and return its horizontal pixel advance. */
	int drawString(Graphics::ManagedSurface *dst, int x, int y, const Common::String &text, const byte alphaLUT[256][256]) const;
	/** Return the horizontal pixel advance for @p text without drawing it. */
	int getStringWidth(const Common::String &text) const;
	/** Return the glyph index for @p character, or -1 when it is unsupported. */
	static int charToGlyphIndex(char character);
	/** Return whether glyph extraction completed. */
	bool isLoaded() const { return _loaded; }

private:
	/** Whether the font strip has been processed. */
	bool _loaded;
	/** Owned glyph bitmaps in character-mapping order. */
	BitBlock *_glyphs[kNumGlyphs];
};

/** Result of one @ref VolumePanel input-and-draw pass. */
enum VolumePanelResult {
	kVolumePanelOpen,    ///< The panel remains open without a new volume change.
	kVolumePanelChanged, ///< At least one preview volume changed during this pass.
	kVolumePanelApply,   ///< The player accepted the preview values.
	kVolumePanelCancel   ///< The player cancelled the preview values.
};

/**
 * Owns the music, sound-effect, and speech sliders shared by map and menu pages.
 *
 * Slider changes are previewed while the panel remains open. Callers decide
 * whether an apply result commits the values or a cancel result restores the
 * initial values captured by @ref VolumePanel::setInitialVolumes.
 */
class VolumePanel {
public:
	/** Leftmost selectable gauge coordinate. */
	static const int kSliderMinX = 350;
	/** Rightmost selectable gauge coordinate. */
	static const int kSliderMaxX = 638;
	/** Number of pixels in the selectable gauge interval. */
	static const int kSliderRange = 288;

	/** Shared X coordinate of each slider label button. */
	static const int kLabelX = 157;
	/** Shared slider label width. */
	static const int kLabelW = 520;
	/** Shared slider label height. */
	static const int kLabelH = 64;
	/** Music slider label Y coordinate. */
	static const int kMusicLabelY = 224;
	/** Sound-effect slider label Y coordinate. */
	static const int kSfxLabelY = 286;
	/** Speech slider label Y coordinate. */
	static const int kSpeechLabelY = 351;

	/** Music gauge Y coordinate. */
	static const int kMusicGaugeY = 240;
	/** Sound-effect gauge Y coordinate. */
	static const int kSfxGaugeY = 305;
	/** Speech gauge Y coordinate. */
	static const int kSpeechGaugeY = 374;

	/** Construct a panel with all current and initial volumes set to 100 percent. */
	VolumePanel();
	/** Release the owned gauge image. */
	~VolumePanel();

	/** Load gauge and button resources. */
	bool init();

	/**
	 * Process slider and completion-button input while drawing the panel.
	 *
	 * @return The action or preview-change state observed during this pass.
	 */
	VolumePanelResult drawAndHandleInput(Graphics::ManagedSurface *dst, int mouseX, int mouseY, bool mouseDown, bool mouseClicked,
										 const byte alphaLUT[256][256]);

	/** Return the current music volume percentage. */
	int getMusicVolume() const { return _musicVolume; }
	/** Return the current sound-effect volume percentage. */
	int getSfxVolume() const { return _sfxVolume; }
	/** Return the current speech volume percentage. */
	int getSpeechVolume() const { return _speechVolume; }
	/** Return the initial music volume percentage. */
	int getInitialMusicVolume() const { return _initialMusicVolume; }
	/** Return the initial sound-effect volume percentage. */
	int getInitialSfxVolume() const { return _initialSfxVolume; }
	/** Return the initial speech volume percentage. */
	int getInitialSpeechVolume() const { return _initialSpeechVolume; }

	/** Clamp and assign the current music volume. */
	void setMusicVolume(int volume);
	/** Clamp and assign the current sound-effect volume. */
	void setSfxVolume(int volume);
	/** Clamp and assign the current speech volume. */
	void setSpeechVolume(int volume);
	/** Assign current values and capture them as the panel's cancellation baseline. */
	void setInitialVolumes(int music, int sfx, int speech);

	/** Clamp @p x to the gauge interval and convert it to a percentage. */
	static int pixelToVolume(int x);
	/** Convert @p volume from a percentage to a gauge X coordinate. */
	static int volumeToPixel(int volume);

private:
	/** Current music volume percentage. */
	int _musicVolume;
	/** Current sound-effect volume percentage. */
	int _sfxVolume;
	/** Current speech volume percentage. */
	int _speechVolume;
	/** Music volume restored when the caller cancels. */
	int _initialMusicVolume;
	/** Sound-effect volume restored when the caller cancels. */
	int _initialSfxVolume;
	/** Speech volume restored when the caller cancels. */
	int _initialSpeechVolume;

	/** Current music gauge endpoint. */
	int _musicSliderX;
	/** Current sound-effect gauge endpoint. */
	int _sfxSliderX;
	/** Current speech gauge endpoint. */
	int _speechSliderX;
	/** Dragged slider index, or -1 when no slider is captured. */
	int _activeSlider;

	/** Owned gauge sprite shared by the three slider rows. */
	RleBlock *_gaugeImage;
	/** Music, sound-effect, and speech label buttons. */
	UIButton _sliderLabels[3];
	/** Apply button. */
	UIButton _okButton;
	/** Cancel button. */
	UIButton _noButton;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_UI_H
