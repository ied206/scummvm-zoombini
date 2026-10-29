/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef TRINKET_RR1CLOUD9_OPTIONS_H
#define TRINKET_RR1CLOUD9_OPTIONS_H

#include "trinket/ompp/sound.h"
#include "trinket/rr1cloud9/world.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

/** RR1's modal settings, activity selection and earned progress panels. */
class Options {
public:
	enum Request {
		kNone,
		kClose,
		kSignin,
		kQuit,
		kCredits,
		kMovie,
		kLocation,
		kPrint,
	};
	/** Configure the options interface for the player, location and practice state. */
	Options(TrinketEngine &vm, World &world, const Common::String &name, int location, bool practice);
	/** Stop option prompts and remove its panel and text sprites. */
	~Options();
	/** Load and display the main options panel. */
	bool enter();
	/** Advance and draw the active options panel at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time);
	/** Handle options input and return a requested game transition. */
	Request event(const Common::Event &event);
	/** Return the location selected by an options navigation request. */
	int destination() const { return _destination; }
	/** Return the story movie selected for replay, or minus one when none is selected. */
	int movie() const { return _movie; }
	/** Return whether options changed the current activity's difficulty. */
	bool changedCurrentLevel() const { return _changedCurrent; }
	/** Stop the current spoken options prompt. */
	void stopSound() { _voice.stop(); }
	/** Map a movie-menu index to its authored video filename. */
	static const char *movieFile(int index);

private:
	/** Resource path for r1mvop1.bik. */
	static constexpr const char *kR1mvop1BikPath = "r1mvop1.bik";
	/** Resource path for r1mvop2.bik. */
	static constexpr const char *kR1mvop2BikPath = "r1mvop2.bik";
	/** Resource path for rr1mvcq.bik. */
	static constexpr const char *kRr1mvcqBikPath = "rr1mvcq.bik";
	/** Resource path for r1mvdemo.bik. */
	static constexpr const char *kR1mvdemoBikPath = "r1mvdemo.bik";
	/** Resource path for r1mvcl1.bik. */
	static constexpr const char *kR1mvcl1BikPath = "r1mvcl1.bik";
	/** Resource path for r1mvclosing.bik. */
	static constexpr const char *kR1mvclosingBikPath = "r1mvclosing.bik";
	/** Script property name for center. */
	static constexpr const char *kPropertyCenter = "center";
	/** World-state key for color. */
	static constexpr const char *kColorKey = "color";
	/** World-state key for is Background Music Off. */
	static constexpr const char *kIsBackgroundMusicOffKey = "isBackgroundMusicOff";
	/** World-state key for is Printer Off. */
	static constexpr const char *kIsPrinterOffKey = "isPrinterOff";
	/** World-state key for movie:%d. */
	static constexpr const char *kMovieSignedIndexKey = "movie:%d";
	/** World-state key for level. */
	static constexpr const char *kLevelKey = "Level";
	/** World-state key for fixed Level. */
	static constexpr const char *kFixedLevelKey = "FixedLevel";
	/** Format for script names or values: %sStats:%d:. */
	static constexpr const char *kNameStatsSignedIndexFormat = "%sStats:%d:";
	/** World-state key for correct. */
	static constexpr const char *kCorrectKey = "correct";
	/** World-state key for total. */
	static constexpr const char *kTotalKey = "total";
	/** World-state key for r1 CN3 Num Frames. */
	static constexpr const char *kR1CN3NumFramesKey = "R1CN3NumFrames";
	/** World-state key for history Count. */
	static constexpr const char *kHistoryCountKey = "HistoryCount";
	/** World-state key for streak. */
	static constexpr const char *kStreakKey = "Streak";
	/** World-state key for streak Kind. */
	static constexpr const char *kStreakKindKey = "StreakKind";
	/** World-state key for assessment Count. */
	static constexpr const char *kAssessmentCountKey = "AssessmentCount";

	/** Main options and assessment text font. */
	static constexpr const char *kFont = "common/1024.nfnt";
	/** Small options and assessment text font. */
	static constexpr const char *kSmallFont = "common/1018.nfnt";
	/** Options animation path indexed by library and resource ID. */
	static constexpr const char *kAnimationFormat = "%s/%d.ao";
	/** Library containing the map close button. */
	static constexpr const char *kMapLibrary = "r1map";
	/** Shared library alias for options artwork. */
	static constexpr const char *kCommonLibrary = "common";
	/** Options speech path indexed by resource ID. */
	static constexpr const char *kSpeechFormat = "comspch/%d.wav";
	/** Background of the options confirmation dialog. */
	static constexpr const char *kDialogBackground = "common/1905.ao";
	/** Prompt animation for restarting the story. */
	static constexpr const char *kRestartPrompt = "common/6140.ao";
	/** Accept button in the options confirmation dialog. */
	static constexpr const char *kDialogAccept = "common/1861.ao";
	/** Cancel button in the options confirmation dialog. */
	static constexpr const char *kDialogCancel = "common/1862.ao";
	/** Sound played for an options control click. */
	static constexpr const char *kClickSound = "common/1801.wav";

	enum Panel {
		kMain,
		kLevels,
		kReport,
		kMovies,
	};
	/** Engine supplying option resources and audio services. */
	TrinketEngine &_vm;
	/** Saved story and activity values modified by options. */
	World &_world;
	/** Resource-path resolver for options images, fonts and prompts. */
	Shell::ResourcePaths _paths;
	/** Sprites for the active options panel. */
	Shell::Scene _scene;
	/** Sprites for level-change and other options confirmations. */
	Shell::Scene _dialog;
	/** Playback state for spoken options prompts. */
	OM::Sound _voice;
	/** Playback state for control feedback sounds. */
	OM::Sound _effect;
	/** FontMan-owned font used for main options labels. */
	const Graphics::Font *_font = nullptr;
	/** FontMan-owned font used for smaller options labels. */
	const Graphics::Font *_smallFont = nullptr;
	/** Text objects created for the current panel. */
	Common::Array<Shell::TextObject *> _texts;
	/** Current player name displayed by options. */
	Common::String _name;
	/** Story location active when options were opened. */
	int _location;
	/** Location selected by the most recent navigation request. */
	int _destination = 0;
	/** Movie-menu selection awaiting playback. */
	int _movie = -1;
	/** Activity row awaiting level-change confirmation. */
	int _pendingRow = -1;
	/** Difficulty awaiting confirmation for the selected activity. */
	int _pendingLevel = 0;
	/** Whether options were opened from a practice activity. */
	bool _practice;
	/** Whether the current page needs to reload after a confirmed difficulty change. */
	bool _changedCurrent = false;
	/** Options panel currently displayed. */
	Panel _panel = kMain;
	struct Control {
		/** Sprite displaying this options control. */
		Shell::Sprite *sprite;
		/** One-based frame used while the control is not pressed. */
		uint rest;
		/** One-based frame used for pressed feedback. */
		uint pressed;
	};
	/** Button sprites with their rest and pressed frame selections. */
	Common::Array<Control> _controls;
	/** Game locations associated with options activity rows. */
	static const int kLocations[8];
	/** Authored activity names associated with options rows. */
	static const char *const kActivities[8];
	/** Build the saved-property prefix for an activity row. */
	static Common::String prefix(int row);
	/** Remove text objects created for the previous panel. */
	void clearText();
	/** Build and display the requested options panel. */
	bool show(Panel panel);
	/** Create an options sprite with an action and optional position. */
	Shell::Sprite *add(int resource, int action = 0, int x = -1, int y = -1);
	/** Draw a text object in the supplied rectangle using the selected font size. */
	void text(const Common::String &value, const Common::Rect &rect, bool small = false);
	/** Play a spoken options prompt by resource ID. */
	void say(int resource);
	/** Register a control sprite and its rest and pressed frames. */
	void control(Shell::Sprite *sprite, uint rest, uint pressed);
	/** Apply a confirmed difficulty change to an activity row. */
	void setLevel(int row, int level);
	/** Translate a control action into panel changes or a game request. */
	Request activate(int action);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
