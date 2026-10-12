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
		virtual ~Resources() {}
		virtual Common::String characterResource(uint resource, Kind kind, uint part = 1) const = 0;
	};
	class CompletionListener {
	public:
		virtual ~CompletionListener() {}
		virtual void characterFinished(Character &character) = 0;
	};
	Character(Scene &scene, Audio::Mixer &mixer, const OM::ResourceProvider &provider, Random &random, const Resources &resources);
	~Character() override;
	bool create(uint settle, int depth, Common::Point position = Common::Point(11111, 11111));
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void update(uint32 time, Common::Array<uint16> &sounds);
	void playSpeech(uint resource, CompletionListener *listener = nullptr);
	void playAnim(uint resource, int repeat = 1, bool keepVisible = true, CompletionListener *listener = nullptr);
	void removeCompletionListener(CompletionListener &listener) {
		if (_listener == &listener)
			_listener = nullptr;
	}
	void stop();
	void pause(bool paused, uint32 time);
	void setVisible(bool shown);
	Common::Rect bounds() const;
	int width() override { return bounds().width(); }
	int height() override { return bounds().height(); }
	bool isBusy() const { return _mode != kIdle00; }

private:
	enum Mode {
		kIdle00 = 0,
		kDirectSpeech01 = 1,
		kPartsSpeech02 = 2,
		kChoreography03 = 3,
		kExplicitAnimation04 = 4,
	};
	class PauseLink : public Core::AnimationPlayer::PauseListener {
	public:
		PauseLink(Character &owner, bool fidget) : _owner(owner), _fidget(fidget) {}
		bool animationPaused() override;

	private:
		Character &_owner;
		bool _fidget;
	};
	struct FidgetGroup {
		uint tag = 0, minimum = 0, maximum = 0;
		uint32 deadline = 0;
		bool repeat = true;
		Common::Array<Sprite *> sprites;
		Common::Array<bool> registered;
		Common::Array<uint> available;
	};
	struct SpeechPart {
		uint resource = 0;
		Sprite *sprite = nullptr;
		bool enabled = true;
	};
	struct ChorPart {
		Sprite *sprite = nullptr;
		Common::Array<int> frames;
	};
	Scene &_scene;
	const OM::ResourceProvider &_provider;
	Random &_random;
	const Resources &_resources;
	OM::Sound _voice;
	Common::Array<Sprite *> _owned;
	Common::Array<FidgetGroup> _groups;
	Common::Array<SpeechPart> _parts;
	Common::Array<ChorPart> _chor;
	Sprite *_settle = nullptr, *_animation = nullptr, *_fidget = nullptr;
	int _fidgetGroup = -1;
	bool _fidgetRegistered = false;
	uint _speechResource = 0;
	uint32 _time = 0, _pauseTime = 0, _chorDeadline = 0, _chorInterval = 0, _chorFrame = 0;
	bool _paused = false, _keepVisible = true;
	bool _registerFidgets = true, _registerSpeech = true, _registerAnims = false;
	bool _loadSpeechInBackground = true, _loadFidgetsInBackground = true;
	Mode _mode = kIdle00;
	CompletionListener *_listener = nullptr;
	PauseLink _fidgetLink{*this, true}, _speechLink{*this, false};
	Sprite *loadAnimation(uint resource, int depth, bool registered);
	void remove(Sprite *sprite);
	void schedule(FidgetGroup &group);
	void finishFidget();
	void finish();
	void restoreSettle();
	bool addFidgetGroup(uint groupTag, uint minimum, uint maximum, bool repeat);
	bool addFidget(uint groupTag, uint resource);
	bool setSpeechAnims(const Core::ScriptArguments &arguments);
	bool playSpeechParts(uint resource);
	bool playChoreography(uint resource);
	void clearSpeechParts();
	void clearChoreography();
	void updateFidgets();
	static uint32 ticks(uint32 time) { return static_cast<uint64>(time) * 60 / 1000; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
