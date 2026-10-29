/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_CHARACTER_H
#define TRINKET_SHELL_CHARACTER_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/multitrinket.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace Shell {

/** Settled character, fidget groups and speech animation parts. */
class Character : public MultiTrinket {
public:
	class Resources {
	public:
		enum Kind {
			kAnimation,
			kSound,
			kLips,
			kChoreography,
		};
		/** Allow destruction through the character-resource interface. */
		virtual ~Resources() {}
		/** Resolve a character resource ID, kind and optional part to its virtual path. */
		virtual Common::String characterResource(uint resource, Kind kind, uint part = 1) const = 0;
	};
	class CompletionListener {
	public:
		/** Allow destruction through the character-completion interface. */
		virtual ~CompletionListener() {}
		/** Notify the caller that a character speech or animation operation finished. */
		virtual void characterFinished(Character &character) = 0;
	};
	/** Connect a character's sprites, speech, resources and random idle behavior. */
	Character(Scene &scene, Audio::Mixer &mixer, const OM::ResourceProvider &provider, Random &random, const Resources &resources);
	/** Stop speech and remove all character animation sprites. */
	~Character() override;
	/** Load the resting animation at a drawing depth and optional game-pixel position. */
	bool create(uint settle, int depth, Common::Point position = Common::Point(11111, 11111));
	/** Read character animation policies and inherited object properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign character loading, registration or inherited object properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch character speech, gestures and idle-animation setup commands. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Advance speech choreography and fidgets at a millisecond timestamp. */
	void update(uint32 time, Common::Array<uint16> &sounds);
	/** Begin a speech resource and notify the optional listener on completion. */
	void playSpeech(uint resource, CompletionListener *listener = nullptr);
	/** Play a character gesture with the supplied repeat and final-visibility settings. */
	void playAnim(uint resource, int repeat = 1, bool keepVisible = true, CompletionListener *listener = nullptr);
	/** Detach the supplied completion listener if it is still registered. */
	void removeCompletionListener(CompletionListener &listener) {
		if (_listener == &listener)
			_listener = nullptr;
	}
	/** Stop speech and gestures and restore the resting appearance. */
	void stop();
	/** Pause or resume character playback while adjusting millisecond deadlines. */
	void pause(bool paused, uint32 time);
	/** Show or hide all sprites that contribute to the character. */
	void setVisible(bool shown);
	/** Return the combined character bounds in game pixels. */
	Common::Rect bounds() const;
	/** Return the current character bounds' width in game pixels. */
	int width() override { return bounds().width(); }
	/** Return the current character bounds' height in game pixels. */
	int height() override { return bounds().height(); }
	/** Return whether a speech or gesture operation is active. */
	bool isBusy() const { return _mode != kIdle00; }

private:
	/** Script event name for finished. */
	static constexpr const char *kEventFinished = "finished";
	/** Script property name for register Fidgets To Settle. */
	static constexpr const char *kPropertyRegisterFidgetsToSettle = "registerFidgetsToSettle";
	/** Script property name for register Speech To Settle. */
	static constexpr const char *kPropertyRegisterSpeechToSettle = "registerSpeechToSettle";
	/** Script property name for register Anims To Settle. */
	static constexpr const char *kPropertyRegisterAnimsToSettle = "registerAnimsToSettle";
	/** Script property name for load Speech Anims In Bkgd. */
	static constexpr const char *kPropertyLoadSpeechAnimsInBkgd = "loadSpeechAnimsInBkgd";
	/** Script property name for load Fidgets In Bkgd. */
	static constexpr const char *kPropertyLoadFidgetsInBkgd = "loadFidgetsInBkgd";
	/** Script property name for visible. */
	static constexpr const char *kPropertyVisible = "visible";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Script property name for y. */
	static constexpr const char *kPropertyY = "y";
	/** Script property name for z. */
	static constexpr const char *kPropertyZ = "z";
	/** Script identifier for cHOR. */
	static constexpr const char *kCHORName = "CHOR";
	/** Script identifier for fPS. */
	static constexpr const char *kFPSName = "FPS";
	/** XML element or attribute name for cHOR/WAVESOUND. */
	static constexpr const char *kXMLCHORWAVESOUND = "CHOR/WAVESOUND";
	/** Script identifier for iD. */
	static constexpr const char *kIDName = "ID";
	/** Script identifier for cHOR/ANIM. */
	static constexpr const char *kCHORANIMName = "CHOR/ANIM";
	/** Script property name for cHOR/ANIM/SEQ/FRAMES. */
	static constexpr const char *kPropertyCHORANIMSEQFRAMES = "CHOR/ANIM/SEQ/FRAMES";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Script method name for set Speech Anims. */
	static constexpr const char *kMethodSetSpeechAnims = "setSpeechAnims";
	/** Script method name for play Speech. */
	static constexpr const char *kMethodPlaySpeech = "playSpeech";
	/** Script method name for play Anim. */
	static constexpr const char *kMethodPlayAnim = "playAnim";
	/** Script method name for stop. */
	static constexpr const char *kMethodStop = "stop";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";

	enum Mode {
		kIdle00 = 0,
		kDirectSpeech01 = 1,
		kPartsSpeech02 = 2,
		kChoreography03 = 3,
		kExplicitAnimation04 = 4,
	};
	class PauseLink : public Core::AnimationPlayer::PauseListener {
	public:
		/** Connect an animation pause marker to fidget or speech completion handling. */
		PauseLink(Character &owner, bool fidget) : _owner(owner), _fidget(fidget) {}
		/** Handle the character animation's pause marker and report whether playback remains valid. */
		bool animationPaused() override;

	private:
		/** Character whose fidget or speech operation receives the callback. */
		Character &_owner;
		/** Whether this callback ends a fidget instead of a speech animation. */
		bool _fidget;
	};
	struct FidgetGroup {
		/** Script tag identifying this group of idle animations. */
		uint tag = 0;
		/** Inclusive minimum number of seconds between fidget attempts. */
		uint minimum = 0;
		/** Exclusive maximum number of seconds between fidget attempts. */
		uint maximum = 0;
		/** Next scheduled attempt in sixty-per-second ticks. */
		uint32 deadline = 0;
		/** Whether completed choices return to the group's available pool. */
		bool repeat = true;
		/** Fidget variants available in this group. */
		Common::Array<Sprite *> sprites;
		/** Whether each corresponding variant hides the resting sprite. */
		Common::Array<bool> hideSettle;
		/** Indices of variants still available for selection. */
		Common::Array<uint> available;
	};
	struct SpeechPart {
		/** Resource ID of this speech animation part. */
		uint resource = 0;
		/** Sprite displaying this speech part. */
		Sprite *sprite = nullptr;
		/** Whether this part participates in speech playback. */
		bool enabled = true;
	};
	struct ChorPart {
		/** Sprite controlled by this choreography track. */
		Sprite *sprite = nullptr;
		/** Frame selections for successive choreography steps. */
		Common::Array<int> frames;
	};
	/** Scene containing the character's resting, fidget and speech sprites. */
	Scene &_scene;
	/** Provider used to open character animation, speech and choreography data. */
	const OM::ResourceProvider &_provider;
	/** Shared random stream used to schedule and choose fidgets. */
	Random &_random;
	/** Game-specific mapping from character IDs to resource paths. */
	const Resources &_resources;
	/** Sound playback state for the current spoken line. */
	OM::Sound _voice;
	/** Character sprites removed when their operation or the character ends. */
	Common::Array<Sprite *> _owned;
	/** Idle-animation groups with their schedules and selection pools. */
	Common::Array<FidgetGroup> _groups;
	/** Speech animation parts and their enabled states. */
	Common::Array<SpeechPart> _parts;
	/** Per-sprite frame tracks for the current speech choreography. */
	Common::Array<ChorPart> _chor;
	/** Resting sprite shown between speech, gestures and fidgets. */
	Sprite *_settle = nullptr;
	/** Sprite used by the current scripted gesture. */
	Sprite *_anim = nullptr;
	/** Idle-animation sprite currently playing. */
	Sprite *_fidget = nullptr;
	/** Index of the active fidget group, or minus one when idle. */
	int _fidgetGroup = -1;
	/** Whether the current fidget temporarily hides the resting sprite. */
	bool _fidgetHidesSettle = false;
	/** Resource ID of the current spoken line. */
	uint _speechRes = 0;
	/** Most recent character update time in milliseconds. */
	uint32 _time = 0;
	/** Millisecond timestamp when playback was paused. */
	uint32 _pauseTime = 0;
	/** Millisecond deadline for the next choreography frame. */
	uint32 _chorDeadline = 0;
	/** Milliseconds between choreography frames. */
	uint32 _chorInterval = 0;
	/** Index of the next frame selection in the choreography tracks. */
	uint32 _chorFrame = 0;
	/** Whether character playback and fidget advancement are paused. */
	bool _paused = false;
	/** Whether the scripted gesture remains visible when playback finishes. */
	bool _keepVisible = true;
	/** Whether fidget sprites are registered in the scene. */
	bool _regFidgets = true;
	/** Whether speech sprites are registered in the scene. */
	bool _regSpeech = true;
	/** Whether scripted gesture sprites are registered in the scene. */
	bool _regAnims = false;
	/** Stored script preference for background loading of speech assets. */
	bool _loadSpeechBg = true;
	/** Stored script preference for background loading of fidget assets. */
	bool _loadFidgetsBg = true;
	/** Current idle, speech or gesture operation. */
	Mode _mode = kIdle00;
	/** Callback notified when the current speech or gesture completes. */
	CompletionListener *_listener = nullptr;
	/** Pause-marker callback that restores the character after a fidget. */
	PauseLink _fidgetLink{*this, true};
	/** Pause-marker callback that completes the active speech animation. */
	PauseLink _speechLink{*this, false};
	/** Load a character animation sprite at a depth with optional scene registration. */
	Sprite *loadAnimation(uint resource, int depth, bool registered);
	/** Remove a character sprite from tracking and the scene. */
	void remove(Sprite *sprite);
	/** Draw and assign the next fidget deadline using the group's delay limits. */
	void schedule(FidgetGroup &group);
	/** End the current fidget and restore the resting sprite. */
	void finishFidget();
	/** End the active speech or gesture and notify its completion listener. */
	void finish();
	/** Restore resting-sprite visibility after a temporary animation. */
	void restoreSettle();
	/** Create an idle-animation group with second-based delay limits. */
	bool addFidgetGroup(uint groupTag, uint minimum, uint maximum, bool repeat);
	/** Add a resource variant and resting-sprite visibility policy to a fidget group. */
	bool addFidget(uint groupTag, uint resource, bool hideSettle);
	/** Replace the speech-part sprites described by the script arguments. */
	bool setSpeechAnims(const Core::ScriptArguments &sargs);
	/** Load and start the animation parts for a spoken resource. */
	bool playSpeechParts(uint resource);
	/** Load the per-part frame tracks for a spoken resource. */
	bool playChoreography(uint resource);
	/** Remove the current speech-part sprites. */
	void clearSpeechParts();
	/** Clear per-part frame tracks for the previous spoken line. */
	void clearChoreography();
	/** Start due idle animations when the character's current operation permits them. */
	void updateFidgets();
	/** Convert a millisecond timestamp to sixty-per-second character ticks. */
	static uint32 ticks(uint32 time) { return static_cast<uint64>(time) * 60 / 1000; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
