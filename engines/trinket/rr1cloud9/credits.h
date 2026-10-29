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
 */

#ifndef TRINKET_RR1CLOUD9_CREDITS_H
#define TRINKET_RR1CLOUD9_CREDITS_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

/** Authored XML credits, shared by the ending and the options panel. */
class Credits {
public:
	/** Connect scrolling credits to the engine's resources and audio. */
	explicit Credits(TrinketEngine &vm);
	/** Stop credit speech and remove the credit sprites. */
	~Credits();
	/** Build the credits and optionally select the ending presentation. */
	bool enter(bool ending);
	/** Advance and draw the scrolling credits at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time);
	/** Dispatch pointer input to credit-screen controls. */
	int event(const Common::Event &event) { return _scene.event(event); }

private:
	/** Script property name for center. */
	static constexpr const char *kPropertyCenter = "center";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** XML element or attribute name for root Element/Section. */
	static constexpr const char *kXMLRootElementSection = "Root_Element/Section";
	/** XML element or attribute name for title. */
	static constexpr const char *kXMLTitle = "title";
	/** XML element or attribute name for root Element/Section/member. */
	static constexpr const char *kXMLRootElementSectionMember = "Root_Element/Section/member";
	/** XML element or attribute name for name. */
	static constexpr const char *kXMLName = "name";
	/** XML element or attribute name for root Element/case1. */
	static constexpr const char *kXMLRootElementCase1 = "Root_Element/case1";
	/** XML element or attribute name for root Element/case1/text. */
	static constexpr const char *kXMLRootElementCase1Text = "Root_Element/case1/text";
	/** XML element or attribute name for root Element/case1/imgpair. */
	static constexpr const char *kXMLRootElementCase1Imgpair = "Root_Element/case1/imgpair";

	/** Font used for credits text. */
	static constexpr const char *kFont = "common/1014.nfnt";
	/** Credits text and image layout. */
	static constexpr const char *kCreditsXML = "r1credits.xml";
	/** Background animation for credits. */
	static constexpr const char *kBackground = "common/6550.ao";
	/** Button returning to sign-in after the ending. */
	static constexpr const char *kSigninButton = "common/6501.ao";
	/** Button quitting after the ending. */
	static constexpr const char *kQuitButton = "common/6502.ao";
	/** Close control in credits. */
	static constexpr const char *kCloseButton = "common/6503.ao";
	/** Credits image path indexed by resource ID. */
	static constexpr const char *kImageFormat = "common/%d.ao";
	/** Background music for credits. */
	static constexpr const char *kMusic = "common/1050.wav";
	/** Speech played after the ending credits. */
	static constexpr const char *kEndingSpeech = "comspch/1913.wav";

	struct Row {
		/** Text objects displayed on this credit row. */
		Common::Array<Shell::TextObject *> text;
		/** Animation sprites displayed on this credit row. */
		Common::Array<Shell::Sprite *> images;
		/** Vertical row offset within the full credit sequence, in pixels. */
		int offset = 0;
		/** Vertical space reserved for the row in pixels. */
		int height = 24;
	};
	/** Engine supplying credit resources and audio services. */
	TrinketEngine &_vm;
	/** Playback state for credit or ending speech. */
	OM::Sound _voice;
	/** Resource-path resolver for credit fonts and animations. */
	Shell::ResourcePaths _paths;
	/** Sprites used for credit-screen controls and background. */
	Shell::Scene _scene;
	/** Animation sprites positioned within the scrolling content. */
	Shell::Scene _images;
	/** Font used for credit text rows. */
	Common::SharedPtr<OM::NFNTFont> _font;
	/** Credit rows in scrolling order. */
	Common::Array<Row> _rows;
	/** Intermediate canvas used to clip credit content to its viewport. */
	OM::Canvas _contents{640, 480};
	/** Current vertical scroll distance in pixels. */
	int _scroll = 0;
	/** Total scrolling content length in pixels. */
	int _length = 0;
	/** Millisecond deadline for the next scrolling step. */
	uint32 _due = 0;
	/** Whether the scrolling presentation has begun. */
	bool _started = false;
	/** Append a credit row with the supplied pixel height. */
	void addRow(int height = 24);
	/** Add credit text at a horizontal position with optional centering. */
	void addText(const Common::String &value, int x = 120, bool center = false);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
