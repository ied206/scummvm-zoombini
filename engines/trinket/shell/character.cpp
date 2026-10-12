/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/character.h"

#include "common/ptr.h"
#include "trinket/ompp/xml.h"

namespace Trinket {
namespace Shell {

Character::Character(Scene &scene, Audio::Mixer &mixer, const OM::ResourceProvider &provider, Random &random, const Resources &resources) : _scene(scene), _provider(provider), _random(random), _resources(resources), _voice(mixer) {
	addEvent("finished");
}

Character::~Character() {
	_listener = nullptr;
	clearTrinkets();
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
}

Sprite *Character::loadAnimation(uint resource, int depth, bool registered) {
	if (!resource)
		return nullptr;
	Sprite *sprite = _scene.add(_resources.characterResource(resource, Resources::kAnimation), z + depth);
	if (!sprite)
		return nullptr;
	sprite->managed = true;
	sprite->enabled = false;
	sprite->movable = false;
	sprite->setFrame(1);
	if (registered && _settle) {
		sprite->x = _settle->x;
		sprite->y = _settle->y;
	}
	addTrinket(*sprite);
	_owned.push_back(sprite);
	return sprite;
}

void Character::remove(Sprite *sprite) {
	for (uint i = 0; i < _owned.size(); i++) {
		if (_owned[i] != sprite)
			continue;
		_owned.remove_at(i);
		removeTrinket(*sprite);
		_scene.remove(sprite);
		return;
	}
}

bool Character::create(uint settle, int depth, Common::Point position) {
	z = depth;
	_settle = loadAnimation(settle, 0, false);
	if (settle && !_settle)
		return false;
	if (_settle) {
		if (position.x != 11111)
			_settle->x = position.x;
		if (position.y != 11111)
			_settle->y = position.y;
		x = _settle->x;
		y = _settle->y;
	}
	return true;
}

Common::Rect Character::bounds() const {
	if (!_settle)
		return Common::Rect();
	return Common::Rect(_settle->x, _settle->y, _settle->x + _settle->width(), _settle->y + _settle->height());
}

bool Character::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	bool value;
	if (property == "registerFidgetsToSettle")
		value = _registerFidgets;
	else if (property == "registerSpeechToSettle")
		value = _registerSpeech;
	else if (property == "registerAnimsToSettle")
		value = _registerAnims;
	else if (property == "loadSpeechAnimsInBkgd")
		value = _loadSpeechInBackground;
	else if (property == "loadFidgetsInBkgd")
		value = _loadFidgetsInBackground;
	else
		return MultiTrinket::get(property, arguments, result);
	result = Core::ScriptValue(static_cast<int32>(value));
	return true;
}

bool Character::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "registerFidgetsToSettle")
		_registerFidgets = value.integer() != 0;
	else if (property == "registerSpeechToSettle")
		_registerSpeech = value.integer() != 0;
	else if (property == "registerAnimsToSettle")
		_registerAnims = value.integer() != 0;
	else if (property == "loadSpeechAnimsInBkgd")
		_loadSpeechInBackground = value.integer() != 0;
	else if (property == "loadFidgetsInBkgd")
		_loadFidgetsInBackground = value.integer() != 0;
	else if (property == "visible")
		setVisible(value.integer() != 0);
	else if (property == "x" || property == "y" || property == "z") {
		int dx = property == "x" ? value.integer() - x : 0;
		int dy = property == "y" ? value.integer() - y : 0;
		int dz = property == "z" ? value.integer() - z : 0;
		for (uint i = 0; i < _owned.size(); i++) {
			_owned[i]->x += dx;
			_owned[i]->y += dy;
			if (dz)
				_scene.setDepth(_owned[i], _owned[i]->z + dz);
		}
		return MultiTrinket::set(property, arguments, value);
	} else
		return MultiTrinket::set(property, arguments, value);
	return true;
}

void Character::schedule(FidgetGroup &group) {
	group.deadline = ticks(_time) + 60 * (group.minimum + _random.draw() % (group.maximum - group.minimum));
}

bool Character::addFidgetGroup(uint groupTag, uint minimum, uint maximum, bool repeat) {
	if (maximum <= minimum)
		return false;
	FidgetGroup group;
	group.tag = groupTag;
	group.minimum = minimum;
	group.maximum = maximum;
	group.repeat = repeat;
	schedule(group);
	_groups.push_back(group);
	return true;
}

bool Character::addFidget(uint groupTag, uint resource) {
	for (uint i = 0; i < _groups.size(); i++) {
		FidgetGroup &group = _groups[i];
		if (group.tag != groupTag)
			continue;
		Sprite *sprite = loadAnimation(resource, 1, _registerFidgets);
		if (!sprite)
			return false;
		sprite->visible = false;
		sprite->setPauseListener(&_fidgetLink);
		group.available.push_back(group.sprites.size());
		group.sprites.push_back(sprite);
		group.registered.push_back(_registerFidgets);
		return true;
	}
	return false;
}

void Character::finishFidget() {
	if (!_fidget)
		return;
	_fidget->stopAnimation();
	_fidget = nullptr;
	schedule(_groups[_fidgetGroup]);
	_fidgetGroup = -1;
	if (_settle && _fidgetRegistered)
		_settle->visible = visible && !isBusy();
}

void Character::updateFidgets() {
	for (uint i = 0; i < _groups.size(); i++) {
		FidgetGroup &group = _groups[i];
		if (static_cast<int32>(ticks(_time) - group.deadline) < 0)
			continue;
		if (!visible || isBusy() || _fidget || _paused)
			schedule(group);
		else if (!group.available.empty()) {
			uint choice = _random.draw() % group.available.size();
			_fidget = group.sprites[group.available[choice]];
			_fidgetGroup = i;
			_fidgetRegistered = group.registered[group.available[choice]];
			if (_fidgetRegistered && _settle)
				_settle->visible = false;
			_fidget->startAnimation(1);
			_fidget->setPauseListener(&_fidgetLink);
			_fidget->visible = true;
			if (group.repeat) {
				group.available.remove_at(choice);
				if (group.available.empty())
					for (uint j = 0; j < group.sprites.size(); j++)
						group.available.push_back(j);
			}
		}
		break;
	}
}

void Character::clearSpeechParts() {
	for (uint i = 0; i < _parts.size(); i++)
		remove(_parts[i].sprite);
	_parts.clear();
}

bool Character::setSpeechAnims(const Core::ScriptArguments &arguments) {
	if (arguments.size() % 3 || _mode == kPartsSpeech02 || _mode == kChoreography03)
		return false;
	clearSpeechParts();
	for (uint i = 0; i < arguments.size(); i += 3) {
		SpeechPart part;
		part.resource = arguments[i].integer();
		part.enabled = arguments[i + 2].integer() != 0;
		part.sprite = loadAnimation(part.resource, arguments[i + 1].integer(), _registerSpeech);
		if (!part.sprite)
			return false;
		part.sprite->visible = false;
		_parts.push_back(part);
	}
	return true;
}

bool Character::playSpeechParts(uint resource) {
	if (_parts.empty())
		return false;
	Sprite *leader = _parts[0].sprite;
	for (uint cel = 1; cel < leader->asset->celCount(); cel++) {
		const Common::Array<Core::Animation::Step> &sequence = leader->asset->sequence(cel);
		if (2 < sequence.size() && static_cast<uint16>(sequence[2].duration) == resource) {
			for (uint i = 0; i < _parts.size(); i++) {
				Sprite *sprite = _parts[i].sprite;
				if (!sprite->startAnimation(i == 0 ? 1 : 0, cel))
					return false;
				sprite->setFrame(1);
				sprite->pauseAnimation(false);
				sprite->visible = visible;
			}
			leader->setPauseListener(&_speechLink);
			_mode = kPartsSpeech02;
			if (_settle)
				_settle->visible = false;
			return true;
		}
	}
	for (uint i = 0; i < _parts.size(); i++) {
		Sprite *sprite = _parts[i].sprite;
		if (!sprite->startAnimation(i == 0 ? 1 : 0))
			return false;
		const Common::Array<Core::Animation::Step> &source = sprite->asset->sequence(0);
		if (source.size() < 2)
			return false;
		Common::Array<Core::Animation::Step> steps;
		Core::Animation::Step command = {0, 0, -4, 0};
		steps.push_back(command);
		Core::Animation::Step base = source[1];
		base.picture = 0;
		base.duration = 0;
		steps.push_back(base);
		if (!i) {
			command.picture = -101;
			command.duration = resource;
			steps.push_back(command);
		}
		Common::ScopedPtr<Common::SeekableReadStream> lips(_provider.open(_resources.characterResource(resource, Resources::kLips, i + 1)));
		if (lips) {
			if (lips->size() < 6)
				return false;
			bool bigEndian = 512 <= lips->readUint16LE();
			lips->seek(4);
			uint count = bigEndian ? lips->readUint16BE() : lips->readUint16LE();
			if (lips->size() - 6 < count * 6)
				return false;
			for (uint frame = 0; frame < count; frame++) {
				uint index = bigEndian ? lips->readUint16BE() : lips->readUint16LE();
				lips->skip(4);
				if (sprite->getNumberOfFrames() <= index)
					index = 0;
				if (source.size() <= index + 1 || source[index + 1].picture < 0)
					return false;
				Core::Animation::Step step = source[index + 1];
				step.duration = 0;
				steps.push_back(step);
			}
		} else
			steps.push_back(base);
		steps.push_back(base);
		command.picture = -2;
		command.duration = 0;
		steps.push_back(command);
		command.picture = -1;
		steps.push_back(command);
		if (!sprite->replaceSequence(steps))
			return false;
		sprite->pauseAnimation(false);
		sprite->visible = visible;
	}
	leader->setPauseListener(&_speechLink);
	_mode = kPartsSpeech02;
	if (_settle)
		_settle->visible = false;
	return true;
}

bool Character::playChoreography(uint resource) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_provider.open(_resources.characterResource(resource, Resources::kChoreography)));
	if (!stream)
		return false;
	OM::XMLParser parser;
	if (!parser.loadStream(stream.release()) || !parser.parse())
		error("Could not load character choreography");
	const Common::Array<OM::XMLParser::Element> &elements = parser.elements();
	if (elements.empty() || elements[0].path != "CHOR")
		return false;
	uint fps = atoi(elements[0].attributes.getValOrDefault("FPS").c_str());
	if (!fps || 1000 < fps)
		error("Invalid character choreography FPS");
	_chorInterval = 1000 / fps;
	uint sound = 0;
	for (uint i = 1; i < elements.size(); i++) {
		if (elements[i].path == "CHOR/WAVESOUND")
			sound = atoi(elements[i].attributes.getValOrDefault("ID").c_str());
		if (elements[i].path != "CHOR/ANIM")
			continue;
		uint id = atoi(elements[i].attributes.getValOrDefault("ID").c_str());
		ChorPart part;
		for (uint j = 0; j < _parts.size(); j++)
			if (_parts[j].resource == id) {
				part.sprite = _parts[j].sprite;
				break;
			}
		if (!part.sprite)
			part.sprite = loadAnimation(id, 0, false);
		if (!part.sprite)
			continue;
		Common::String frames = parser.get("CHOR/ANIM/SEQ/FRAMES");
		for (uint j = i + 1; j < elements.size() && static_cast<int>(i) <= elements[j].parent; j++)
			if (elements[j].path == "CHOR/ANIM/SEQ/FRAMES")
				frames = elements[j].value;
		const char *cursor = frames.c_str();
		while (*cursor) {
			while (*cursor && Common::isSpace(*cursor))
				cursor += 1;
			if (!*cursor)
				break;
			char *end = nullptr;
			long frame = strtol(cursor, &end, 10);
			if (cursor == end)
				error("Invalid character choreography frame");
			part.frames.push_back(frame);
			cursor = end;
		}
		part.sprite->visible = visible;
		if (!part.frames.empty())
			part.sprite->setFrame(part.frames[0]);
		_chor.push_back(part);
	}
	_voice.play(_provider, _resources.characterResource(sound, Resources::kSound), Audio::Mixer::kSpeechSoundType);
	_chorFrame = 1;
	_chorDeadline = _time + _chorInterval;
	_mode = kChoreography03;
	if (_settle)
		_settle->visible = false;
	return true;
}

void Character::clearChoreography() {
	for (uint i = 0; i < _chor.size(); i++) {
		Sprite *sprite = _chor[i].sprite;
		bool retained = false;
		for (uint j = 0; j < _parts.size(); j++)
			retained |= _parts[j].sprite == sprite;
		if (retained)
			sprite->visible = false;
		else
			remove(sprite);
	}
	_chor.clear();
}

void Character::restoreSettle() {
	if (_settle)
		_settle->visible = visible;
	if (!_keepVisible)
		setVisible(false);
}

void Character::stop() {
	_listener = nullptr;
	if (!isBusy())
		return;
	if (_animation) {
		remove(_animation);
		_animation = nullptr;
	}
	for (uint i = 0; i < _parts.size(); i++) {
		_parts[i].sprite->stopAnimation();
		_parts[i].sprite->setFrame(1);
		_parts[i].sprite->visible = false;
	}
	clearChoreography();
	_voice.stop();
	_mode = kIdle00;
	restoreSettle();
}

void Character::finish() {
	if (_animation) {
		remove(_animation);
		_animation = nullptr;
	}
	for (uint i = 0; i < _parts.size(); i++) {
		_parts[i].sprite->pauseAnimation(true);
		_parts[i].sprite->visible = false;
	}
	clearChoreography();
	_mode = kIdle00;
	restoreSettle();
	Core::ScriptableObject::Lifetime lifetime(*this);
	emitEvent("finished");
	if (!lifetime.isAlive())
		return;
	complete();
	if (lifetime.isAlive() && _listener)
		_listener->characterFinished(*this);
}

bool Character::PauseLink::animationPaused() {
	if (_fidget)
		_owner.finishFidget();
	else
		_owner.finish();
	// Completion can replace the playback or dispose either owner.
	return false;
}

void Character::playSpeech(uint resource, CompletionListener *listener) {
	_time = callTime();
	if (isBusy()) {
		if (listener)
			listener->characterFinished(*this);
		return;
	}
	finishFidget();
	_listener = listener;
	_keepVisible = true;
	_speechResource = resource;
	if (playChoreography(resource) || playSpeechParts(resource))
		return;
	_mode = kDirectSpeech01;
	if (!_voice.play(_provider, _resources.characterResource(resource, Resources::kSound), Audio::Mixer::kSpeechSoundType)) {
		_mode = kIdle00;
		if (_listener)
			_listener->characterFinished(*this);
	}
}

void Character::playAnim(uint resource, int repeat, bool keepVisible, CompletionListener *listener) {
	_time = callTime();
	if (isBusy()) {
		if (listener)
			listener->characterFinished(*this);
		return;
	}
	finishFidget();
	_listener = listener;
	_keepVisible = keepVisible;
	_animation = loadAnimation(resource, 0, _registerAnims);
	if (!_animation) {
		if (_listener)
			_listener->characterFinished(*this);
		return;
	}
	_animation->startAnimation(repeat);
	_animation->setPauseListener(&_speechLink);
	_animation->visible = visible;
	_mode = kExplicitAnimation04;
	if (_settle)
		_settle->visible = false;
}

void Character::setVisible(bool shown) {
	if (visible == shown)
		return;
	finishFidget();
	visible = shown;
	if (_settle)
		_settle->visible = shown && (_mode == kIdle00 || _mode == kDirectSpeech01);
	if (_animation)
		_animation->visible = shown;
	if (_mode == kPartsSpeech02)
		for (uint i = 0; i < _parts.size(); i++)
			_parts[i].sprite->visible = shown;
	if (_mode == kChoreography03)
		for (uint i = 0; i < _chor.size(); i++)
			_chor[i].sprite->visible = shown;
}

void Character::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	_paused = paused;
	if (paused)
		_pauseTime = time;
	else if (_mode == kChoreography03)
		_chorDeadline += time - _pauseTime;
	_voice.pause(paused);
	if (_animation)
		_animation->pauseAnimation(paused);
	if (_fidget)
		_fidget->pauseAnimation(paused);
	if (_mode == kPartsSpeech02)
		for (uint i = 0; i < _parts.size(); i++)
			_parts[i].sprite->pauseAnimation(paused);
}

void Character::update(uint32 time, Common::Array<uint16> &sounds) {
	_time = time;
	Core::ScriptableObject::Lifetime lifetime(*this);
	if (_mode != kChoreography03)
		updateFidgets();
	else if (!_paused && static_cast<int32>(time - _chorDeadline) >= 0) {
		for (uint i = 0; i < _chor.size(); i++) {
			ChorPart &part = _chor[i];
			if (part.frames.size() <= _chorFrame)
				continue;
			int frame = part.frames[_chorFrame];
			part.sprite->visible = frame != -1 && visible;
			if (frame != -1)
				part.sprite->setFrame(frame + 1);
		}
		_chorFrame += 1;
		_chorDeadline += _chorInterval;
	}
	if (!_paused && (_mode == kDirectSpeech01 || _mode == kChoreography03) && !_voice.isPlaying()) {
		finish();
		if (!lifetime.isAlive())
			return;
	}
	Common::Array<Sprite *> current = _owned;
	for (uint i = 0; i < current.size(); i++) {
		Sprite *sprite = _scene.resolveSprite(current[i]);
		if (!sprite)
			continue;
		Common::Array<uint16> childSounds;
		sprite->update(time, childSounds);
		if (!lifetime.isAlive())
			return;
		for (uint j = 0; j < childSounds.size(); j++) {
			if (_mode == kPartsSpeech02 && childSounds[j] == _speechResource)
				_voice.play(_provider, _resources.characterResource(childSounds[j], Resources::kSound), Audio::Mixer::kSpeechSoundType);
			else
				sounds.push_back(childSounds[j]);
		}
	}
}

bool Character::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	_time = callTime();
	if (method == "addFidgetGroup" && (arguments.size() == 3 || arguments.size() == 4))
		return addFidgetGroup(arguments[0].integer(), arguments[1].integer(), arguments[2].integer(),
							  arguments.size() == 3 || arguments[3].integer() != 0);
	if (method == "addFidget" && arguments.size() == 2)
		return addFidget(arguments[0].integer(), arguments[1].integer());
	if (method == "setSpeechAnims")
		return setSpeechAnims(arguments);
	if (method == "playSpeech" && !arguments.empty() && arguments.size() <= 3) {
		playSpeech(arguments[0].integer());
		return true;
	}
	if (method == "playAnim" && !arguments.empty() && arguments.size() <= 3) {
		playAnim(arguments[0].integer(), 1 < arguments.size() ? arguments[1].integer() : 1,
				 arguments.size() < 3 || arguments[2].integer() != 0);
		return true;
	}
	if (arguments.empty() && (method == "stop" || method == "pause" || method == "resume")) {
		if (method == "stop")
			stop();
		else
			pause(method == "pause", _time);
		return true;
	}
	return MultiTrinket::play(method, arguments, result);
}

} // namespace Shell
} // namespace Trinket
