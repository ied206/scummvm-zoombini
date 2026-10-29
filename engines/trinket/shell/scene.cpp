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

#include "trinket/shell/scene.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "image/jpeg.h"
#include "trinket/core/background.h"

namespace Trinket {
namespace Shell {

constexpr const char *Scene::kZName;

constexpr const char *Sprite::kEventHit;
constexpr const char *Sprite::kEventAnimDone;
constexpr const char *Sprite::kEventPaused;
constexpr const char *Sprite::kEventStartTracking;
constexpr const char *Sprite::kEventEndTracking;
constexpr const char *Sprite::kPropertyUserData;
constexpr const char *Sprite::kPropertyEnabled;
constexpr const char *Sprite::kPropertyIsSolid;
constexpr const char *Sprite::kPropertyBoxTouchy;
constexpr const char *Sprite::kPropertyTracking;
constexpr const char *Sprite::kPropertyClickSoundID;
constexpr const char *Sprite::kPropertyFlyOnSoundID;
constexpr const char *Sprite::kPropertyPlayDelay;
constexpr const char *Sprite::kPropertyFrame;
constexpr const char *Sprite::kPropertyFrameCount;
constexpr const char *Sprite::kPropertyPauseOnLastFrame;
constexpr const char *Sprite::kPropertyClickControllerFXID;
constexpr const char *Sprite::kPropertyFlyOnControllerFXID;
constexpr const char *Sprite::kPropertyCorral;
constexpr const char *Sprite::kPropertyRepeatCount;
constexpr const char *Sprite::kPropertyZ;
constexpr const char *Sprite::kMethodSetFlyOffRect;
constexpr const char *Sprite::kMethodDefault;
constexpr const char *Sprite::kMethodForever;
constexpr const char *Sprite::kMethodPlay;
constexpr const char *Sprite::kMethodResume;
constexpr const char *Sprite::kMethodPause;
constexpr const char *Sprite::k1Name;

constexpr const char *Scene::kBackgroundExtension;
constexpr const char *Scene::kAnimationExtension;
constexpr const char *Scene::kPaletteExtension;

Scene::~Scene() {
	clear();
}

Sprite::Sprite() {
	addEvent(kEventHit);
	addEvent(kEventAnimDone);
	addEvent(kEventPaused);
	addEvent(kEventStartTracking);
	addEvent(kEventEndTracking);
	_player.setPauseListener(this);
}

bool Sprite::animationPaused() {
	Core::ScriptableObject::Lifetime lifetime(*this);
	emitEvent(kEventPaused);
	return lifetime.isAlive();
}

void Sprite::dispose() {
	if (_scene)
		_scene->remove(this);
	else
		delete this;
}

Sprite *Scene::resolveSprite(const Core::ScriptableObject *object) const {
	for (uint i = 0; i < _sprites.size(); i++) {
		if (_sprites[i] == object)
			return _sprites[i];
	}
	return nullptr;
}

bool Sprite::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyUserData))
		result = Core::ScriptValue::fromText(_userData);
	else if (property.equals(kPropertyEnabled))
		result = Core::ScriptValue(static_cast<int32>(enabled));
	else if (property.equals(kPropertyIsSolid))
		result = Core::ScriptValue(static_cast<int32>(solid));
	else if (property.equals(kPropertyBoxTouchy))
		result = Core::ScriptValue(static_cast<int32>(boxTouchy));
	else if (property.equals(kPropertyTracking))
		result = Core::ScriptValue(static_cast<int32>(tracking));
	else if (property.equals(kPropertyClickSoundID))
		result = Core::ScriptValue::fromText(_clickSoundID);
	else if (property.equals(kPropertyFlyOnSoundID))
		result = Core::ScriptValue::fromText(_flyOnSoundID);
	else if (property.equals(kPropertyPlayDelay))
		result = Core::ScriptValue(static_cast<int32>(_playDelay));
	else if (property.equals(kPropertyFrame))
		result = Core::ScriptValue(static_cast<int32>(getFrame()));
	else if (property.equals(kPropertyFrameCount))
		result = Core::ScriptValue(static_cast<int32>(getNumberOfFrames()));
	else if (property.equals(kPropertyPauseOnLastFrame))
		result = Core::ScriptValue(static_cast<int32>(_pauseOnLastFrame));
	else
		return Core::Trinket::get(property, sargs, result);
	return true;
}

bool Sprite::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	if (property.equals(kPropertyUserData))
		_userData = value.toText();
	else if (property.equals(kPropertyEnabled))
		enabled = value.toInt32() != 0;
	else if (property.equals(kPropertyIsSolid))
		solid = value.toInt32() != 0;
	else if (property.equals(kPropertyBoxTouchy))
		boxTouchy = value.toInt32() != 0;
	else if (property.equals(kPropertyTracking))
		tracking = value.toInt32() != 0;
	else if (property.equals(kPropertyClickSoundID))
		_clickSoundID = value.toText();
	else if (property.equals(kPropertyFlyOnSoundID))
		_flyOnSoundID = value.toText();
	else if (property.equals(kPropertyClickControllerFXID))
		_clickControllerFXID = value.toInt32();
	else if (property.equals(kPropertyFlyOnControllerFXID))
		_flyOnControllerFXID = value.toInt32();
	else if (property.equals(kPropertyPlayDelay))
		_playDelay = value.toInt32();
	else if (property.equals(kPropertyCorral) && sargs.size() == 3)
		corral = Common::Rect(sargs[0].toInt32(), sargs[1].toInt32(), sargs[2].toInt32(), value.toInt32());
	else if (property.equals(kPropertyFrame))
		setFrame(value.toInt32());
	else if (property.equals(kPropertyPauseOnLastFrame))
		setPauseOnLastFrame(value.toInt32() != 0);
	else if (property.equals(kPropertyRepeatCount))
		setRepeat(value.toInt32());
	else if (property.equals(kPropertyZ) && _scene)
		_scene->setDepth(this, value.toInt32());
	else
		return Core::Trinket::set(property, sargs, value);
	return true;
}

bool Sprite::startAnimation(int repeat, uint sequence) {
	if (!asset || !_player.start(*asset, sequence, repeat))
		return false;
	if (_animated) {
		x -= _animOffset.x;
		y -= _animOffset.y;
	}
	_animated = true;
	_animOffset = _player.offset();
	x += _animOffset.x;
	y += _animOffset.y;
	frame = _player.picture();
	return true;
}

bool Sprite::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (button && method.equals(kMethodSetFlyOffRect) && sargs.size() == 4) {
		hitRect = Common::Rect(sargs[0].toInt32(), sargs[1].toInt32(), sargs[2].toInt32(), sargs[3].toInt32());
		return true;
	}
	if (!sargs.empty())
		return Core::Trinket::play(method, sargs, result);
	if (button && method.equals(kMethodDefault)) {
		buttonPressed(callTime(), true);
		return true;
	}
	if (method.equals(kMethodForever)) {
		if (!_animated && !startAnimation())
			return false;
		setRepeat(-1);
		playAnimation();
		return true;
	}
	if (method.equals(kMethodPlay) || method.equals(kMethodDefault) || method.equals(kMethodResume)) {
		if (!_animated && !startAnimation())
			return false;
		playAnimation();
		return true;
	}
	if (method.equals(kMethodPause)) {
		pauseAnimation(true);
		return true;
	}
	return Core::Trinket::play(method, sargs, result);
}

void Sprite::playAnimation() {
	_scriptPlaying = true;
	_explicitlyPaused = false;
	_player.activate();
}

void Sprite::stopAnimation() {
	_scriptPlaying = false;
	_player.stop();
	visible = false;
}

void Sprite::syncAnimation() {
	Common::Point offset = _player.offset();
	x += offset.x - _animOffset.x;
	y += offset.y - _animOffset.y;
	_animOffset = offset;
	frame = _player.picture();
}

void Sprite::setFrame(int number) {
	if (number <= 0)
		return;
	if (!_animated && !startAnimation())
		return;
	_player.setFrame(number);
	syncAnimation();
}

bool Sprite::replaceSequence(const Common::Array<Core::Animation::Step> &steps) {
	if ((!_animated && !startAnimation()) || !_player.replaceSequence(steps))
		return false;
	syncAnimation();
	return true;
}

bool Sprite::reverseSequence() {
	if (!_animated && !startAnimation())
		return false;
	uint count = _player.getNumberOfFrames();
	const Common::Array<Core::Animation::Step> &source = _player.sequence();
	if (!count || source.size() <= count)
		return false;
	Common::Array<Core::Animation::Step> reversed;
	Core::Animation::Step command = {Common::Point(), -4, 0};
	reversed.push_back(command);
	// Shell reverses raw entries, retaining their coordinate offsets.
	for (uint i = count; 0 < i; i--) {
		Core::Animation::Step step = source[i];
		step.duration = 0;
		reversed.push_back(step);
	}
	command.picture = -2;
	reversed.push_back(command);
	command.picture = -1;
	reversed.push_back(command);
	if (!_player.replaceSequence(reversed))
		return false;
	syncAnimation();
	return true;
}

void Sprite::setPauseOnLastFrame(bool pauseAtEnd) {
	if (!_animated && !startAnimation())
		return;
	_pauseOnLastFrame = pauseAtEnd;
	const Common::Array<Core::Animation::Step> &steps = _player.sequence();
	uint end = 0;
	while (end < steps.size() && steps[end].picture != -1)
		end += 1;
	if (end == steps.size() || end <= 3)
		return;
	uint position = end - 3;
	if (pauseAtEnd && steps[position].picture != -4) {
		Core::Animation::Step stop = {Common::Point(), -4, 0};
		_player.insertSequenceEntry(position + 1, stop);
	} else if (!pauseAtEnd && steps[position].picture == -4) {
		_player.removeSequenceEntry(position);
	}
	setFrame(1);
}

void Scene::update(uint32 time, Common::Array<uint16> &sounds) {
	_updating = true;
	_nextUpdate = 0;
	while (_nextUpdate < _sprites.size()) {
		Sprite &sprite = *_sprites[_nextUpdate];
		_nextUpdate += 1;
		if (sprite.managed)
			continue;
		sprite.update(time, sounds);
	}
	_updating = false;
}

void Sprite::update(uint32 time, Common::Array<uint16> &sounds) {
	if (_syntheticPress && _playDelay < time - _pressTime) {
		_syntheticPress = false;
		pressed = false;
	}
	if (_updateListener)
		_updateListener->spriteUpdating(*this, time);
	if (!_animated)
		return;
	if (!_player.update(time, sounds))
		return;
	syncAnimation();
	if (!_player.isVisible())
		visible = false;
	if (_scriptPlaying && !_explicitlyPaused && !_player.isRunning()) {
		_scriptPlaying = false;
		Core::ScriptableObject::Lifetime lifetime(*this);
		AnimationListener *listener = _animListener;
		_animListener = nullptr;
		if (listener)
			listener->animationFinished(*this);
		if (!lifetime.isAlive())
			return;
		complete();
		if (lifetime.isAlive())
			emitEvent(kEventAnimDone);
	}
}

void Scene::remove(Sprite *sprite) {
	for (uint i = 0; i < _sprites.size(); i++) {
		if (_sprites[i] == sprite) {
			_sprites.remove_at(i);
			if (_updating && i < _nextUpdate)
				_nextUpdate -= 1;
			delete sprite;
			return;
		}
	}
}

void Scene::insert(Sprite *sprite) {
	uint i = 0;
	while (i < _sprites.size() && _sprites[i]->z <= sprite->z)
		i += 1;
	_sprites.insert_at(i, sprite);
	// Insertion at the pending entry is visited in this update, including a new tail.
	if (_updating && i < _nextUpdate)
		_nextUpdate += 1;
}

void Scene::setDepth(Sprite *sprite, int z) {
	if (sprite->z == z)
		return;
	for (uint i = 0; i < _sprites.size(); i++) {
		if (_sprites[i] != sprite)
			continue;
		_sprites.remove_at(i);
		if (_updating && i < _nextUpdate)
			_nextUpdate -= 1;
		sprite->Core::Trinket::set(kZName, Core::ScriptArguments(), Core::ScriptValue(static_cast<int32>(z)));
		insert(sprite);
		return;
	}
}

bool Sprite::cachePicture() {
	if (button) {
		uint state = 0;
		if (!enabled)
			state = disabledFrame;
		else if (pressed)
			state = 1;
		else if (flyButton && hovered)
			state = hoverFrame;
		// Button states are logical frames, including their authored coordinate offsets.
		setFrame(state + 1);
	}
	if (static_cast<int>(frame) == _cachedFrame)
		return _picture != nullptr;
	_cachedFrame = frame;
	_picture = asset->picture(frame);
	return _picture != nullptr;
}

void Scene::clear() {
	if (!_assets.empty() || !_backgrounds.empty())
		debug(2, "Trinket scene cache: %s", cacheDescription().c_str());
	_pressedAction = 0;
	_pressFeedback = 0;
	_scrollX = 0;
	for (uint i = 0; i < _sprites.size(); i++)
		delete _sprites[i];
	_sprites.clear();
	_nextUpdate = 0;
	for (auto it = _assets.begin(); it != _assets.end(); ++it)
		delete it->_value;
	_assets.clear();
	_background.reset();
	_backgrounds.clear();
	_resources.clear();
}

bool Scene::setBackground(const Common::String &path) {
	Common::String key = _resources.cacheKey(path);
	if (key.empty())
		return false;
	auto found = _backgrounds.find(key);
	if (found != _backgrounds.end()) {
		_background = found->_value;
		_backgroundHits += 1;
		return true;
	}
	Common::ScopedPtr<Common::SeekableReadStream> stream(_resources.open(path));
	if (!stream)
		return false;
	_resources.release(path);
	key = _resources.cacheKey(path);
	found = _backgrounds.find(key);
	if (found != _backgrounds.end()) {
		_background = found->_value;
		_backgroundHits += 1;
		return true;
	}
	Common::SharedPtr<Background> background(new Background());
	if (key.hasSuffix(kBackgroundExtension)) {
		if (!Core::Background16::decode(*stream, background->surface))
			return false;
	} else {
		Image::JPEGDecoder decoder;
		decoder.setOutputPixelFormat(Graphics::PixelFormat::createFormatRGBA32());
		if (!decoder.loadStream(*stream))
			return false;
		background->surface.copyFrom(*decoder.getSurface());
	}
	_backgrounds[key] = background;
	_background = background;
	_bgDecodes += 1;
	return true;
}

const Core::Animation *Scene::loadAnimation(const Common::String &path) {
	Common::String sourceKey = _resources.cacheKey(path);
	if (sourceKey.empty())
		return nullptr;
	Common::String palettePath = path;
	palettePath.toLowercase();
	palettePath.replace('\\', '/');
	if (palettePath.hasSuffix(kAnimationExtension))
		palettePath = palettePath.substr(0, palettePath.size() - 3) + kPaletteExtension;
	else
		palettePath.clear();
	Common::String paletteKey;
	if (!palettePath.empty())
		paletteKey = _resources.cacheKey(palettePath);
	Common::String key = sourceKey + "|" + paletteKey;
	auto found = _assets.find(key);
	if (found != _assets.end())
		return found->_value;
	Common::ScopedPtr<Common::SeekableReadStream> stream(_resources.open(path));
	if (!stream)
		return nullptr;
	Common::ScopedPtr<Common::SeekableReadStream> palette;
	if (!palettePath.empty())
		palette.reset(_resources.open(palettePath));
	// Resolve each source independently: a common AO and RGB can live in different libraries.
	sourceKey = _resources.cacheKey(path);
	if (!palettePath.empty())
		paletteKey = _resources.cacheKey(palettePath);
	key = sourceKey + "|" + paletteKey;
	// Streams retain their bytes while parsing; do not retain failed decodes in the byte cache.
	_resources.release(path);
	_resources.release(palettePath);
	found = _assets.find(key);
	if (found != _assets.end())
		return found->_value;
	Common::ScopedPtr<Core::Animation> asset(new Core::Animation());
	if (!asset->load(*stream) || (palette && !asset->loadPalette(*palette)))
		return nullptr;
	_assets[key] = asset.release();
	return _assets[key];
}

Sprite *Scene::add(const Common::String &path, int z, int action, bool button) {
	const Core::Animation *asset = loadAnimation(path);
	if (!asset)
		return nullptr;
	Sprite *sprite = new Sprite();
	sprite->_scene = this;
	sprite->asset = asset;
	sprite->x = asset->origin().x;
	sprite->y = asset->origin().y;
	sprite->z = z;
	sprite->action = action;
	sprite->button = button;
	insert(sprite);
	return sprite;
}

int Scene::event(const Common::Event &event, int minimumZ, int maximumZ) {
	_pressFeedback = 0;
	if (event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP && event.type != Common::EVENT_MOUSEMOVE)
		return 0;
	int hit = hitTest(event.mouse, minimumZ, maximumZ);
	bool actionOnMouseDown = false;
	for (uint i = 0; hit && i < _sprites.size(); i++) {
		if (_sprites[i]->action == hit) {
			actionOnMouseDown = _sprites[i]->actionOnMouseDown;
			break;
		}
	}
	int action = 0;
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		_pressedAction = hit;
		if (actionOnMouseDown)
			action = hit;
	}
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_pressedAction == hit && !actionOnMouseDown)
			action = hit;
		_pressedAction = 0;
	}
	for (uint i = 0; i < _sprites.size(); i++) {
		Sprite &sprite = *_sprites[i];
		bool pressed = _pressedAction && sprite.action == _pressedAction && hit == _pressedAction;
		if (sprite.button && pressed && (!sprite.pressed || event.type == Common::EVENT_LBUTTONDOWN))
			_pressFeedback = sprite.action;
		sprite.pressed = pressed;
		sprite.hovered = !_pressedAction && sprite.action && sprite.action == hit;
	}
	return action;
}

void Scene::draw(OM::Canvas &canvas) {
	drawBackground(canvas);
	drawSprites(canvas, INT_MIN, INT_MAX);
}

void Scene::drawBackground(OM::Canvas &canvas) {
	canvas.surface().fillRect(Common::Rect(canvas.surface().w, canvas.surface().h), 0);
	if (_background)
		canvas.blit(_background->surface, -_scrollX, 0);
}

void Scene::drawSprites(OM::Canvas &canvas, int minimumZ, int maximumZ) {
	for (uint i = 0; i < _sprites.size(); i++) {
		Sprite &sprite = *_sprites[i];
		if (!sprite.visible || sprite.z < minimumZ || maximumZ < sprite.z)
			continue;
		if (sprite.cachePicture()) {
			int x = sprite.mirrored ? 2 * sprite.mirrorX - sprite.x - sprite._picture->w : sprite.x;
			x -= sprite.scrolling ? _scrollX : 0;
			sprite.bounds = Common::Rect(x, sprite.y, x + sprite._picture->w, sprite.y + sprite._picture->h);
			canvas.blit(*sprite._picture, x, sprite.y, sprite.mirrored, sprite.colorRemap, &sprite.colorRemapRegions);
		}
	}
}

Sprite *Scene::hitSprite(Common::Point point, int minimumZ, int maximumZ, bool requireAction) const {
	for (int i = static_cast<int>(_sprites.size()) - 1; 0 <= i; i--) {
		Sprite &sprite = *_sprites[i];
		if (sprite.z < minimumZ || maximumZ < sprite.z || (requireAction && !sprite.action))
			continue;
		if (sprite.covers(point))
			return &sprite;
	}
	return nullptr;
}

int Scene::hitTest(Common::Point point, int minimumZ, int maximumZ) const {
	Sprite *sprite = hitSprite(point, minimumZ, maximumZ, true);
	return sprite ? sprite->action : 0;
}

bool Sprite::covers(Common::Point point) {
	if (!visible || !enabled || !asset || !cachePicture())
		return false;
	if (!hitRect.isEmpty())
		return hitRect.contains(point);
	int left = mirrored ? 2 * mirrorX - x - _picture->w : x;
	left -= scrolling && _scene ? _scene->_scrollX : 0;
	bounds = Common::Rect(left, y, left + _picture->w, y + _picture->h);
	if (!bounds.contains(point))
		return false;
	if (solid || boxTouchy)
		return true;
	byte a;
	byte r;
	byte g;
	byte b;
	int column = mirrored ? _picture->w - 1 - (point.x - left) : point.x - left;
	uint32 pixel = *static_cast<const uint32 *>(_picture->getBasePtr(column, point.y - y));
	_picture->format.colorToARGB(pixel, a, r, g, b);
	return a != 0;
}

void Sprite::buttonPressed(uint32 time, bool synthetic) {
	pressed = true;
	_pressTime = time;
	_syntheticPress = synthetic;
	_clickPending = true;
	_pendingSound = _clickSoundID;
}

void Sprite::flyOn() {
	hovered = true;
	if (flyButton && !_flyOnSoundID.equals(k1Name)) {
		_pendingSound = _flyOnSoundID;
		_clickPending = true;
	}
}

bool Sprite::pollClickSound(Common::String &resource) {
	if (!_clickPending)
		return false;
	_clickPending = false;
	resource = _pendingSound;
	return !resource.equals(k1Name);
}

Core::ScriptableObject *Sprite::clone() const {
	if (!_scene)
		return nullptr;
	Sprite *copy = new Sprite();
	copy->_scene = _scene;
	copy->asset = asset;
	copy->colorRemap = colorRemap;
	copy->colorRemapRegions = colorRemapRegions;
	copy->x = x;
	copy->y = y;
	copy->z = z;
	copy->tag = tag;
	copy->visible = visible;
	copy->enabled = enabled;
	copy->movable = movable;
	copy->spotCursor = spotCursor;
	copy->grabCursor = grabCursor;
	copy->button = button;
	copy->flyButton = flyButton;
	copy->solid = solid;
	copy->boxTouchy = boxTouchy;
	copy->tracking = tracking;
	copy->corral = corral;
	copy->hitRect = hitRect;
	copy->_userData = _userData;
	copy->_clickSoundID = _clickSoundID;
	copy->_flyOnSoundID = _flyOnSoundID;
	copy->_clickControllerFXID = _clickControllerFXID;
	copy->_flyOnControllerFXID = _flyOnControllerFXID;
	copy->_playDelay = _playDelay;
	copy->_player = _player;
	copy->_player.setPauseListener(copy);
	copy->_animated = _animated;
	copy->_animOffset = _animOffset;
	copy->frame = frame;
	copy->_pauseOnLastFrame = _pauseOnLastFrame;
	copyEventsTo(*copy);
	_scene->insert(copy);
	return copy;
}

void Scene::uncacheUnusedResources() {
	for (auto it = _assets.begin(); it != _assets.end();) {
		bool used = false;
		for (const Sprite *sprite : _sprites)
			used = used || sprite->asset == it->_value;
		if (!used) {
			delete it->_value;
			auto unused = it++;
			_assets.erase(unused);
		} else {
			++it;
		}
	}
	for (auto it = _backgrounds.begin(); it != _backgrounds.end();) {
		if (it->_value.unique()) {
			auto unused = it++;
			_backgrounds.erase(unused);
		} else {
			++it;
		}
	}
	_resources.clear();
}

Common::String Scene::cacheDescription() const {
	uint64 sourceBytes = 0;
	uint64 pictureBytes = 0;
	uint64 decodes = 0;
	uint64 hits = 0;
	uint64 bgBytes = 0;
	for (const auto &entry : _assets) {
		sourceBytes += entry._value->sourceBytes();
		pictureBytes += entry._value->pictureBytes();
		decodes += entry._value->decodeCount();
		hits += entry._value->pictureHits();
	}
	for (const auto &entry : _backgrounds)
		bgBytes += static_cast<uint64>(entry._value->surface.pitch) * entry._value->surface.h;
	const OM::ResourceCache::Statistics &stats = _resources.statistics();
	return Common::String::format("assets=%u source=%" PRIu64 " pixels=%" PRIu64 " raw=%" PRIu64
								  " decodes=%" PRIu64 " pictureHits=%" PRIu64 " requests=%" PRIu64 " reads=%" PRIu64 " byteHits=%" PRIu64
								  " backgrounds=%u backgroundBytes=%" PRIu64 " backgroundDecodes=%" PRIu64 " backgroundHits=%" PRIu64,
								  _assets.size(), sourceBytes, pictureBytes, _resources.bytes(), decodes, hits, stats.requests, stats.reads, stats.hits,
								  _backgrounds.size(), bgBytes, _bgDecodes, _backgroundHits);
}

} // namespace Shell
} // namespace Trinket
