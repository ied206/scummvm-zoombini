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
#include "common/ptr.h"
#include "image/jpeg.h"
#include "trinket/core/background.h"

namespace Trinket {
namespace Shell {

Scene::~Scene() {
	clear();
}

Sprite::Sprite() {
	addEvent("hit");
	addEvent("animDone");
	addEvent("paused");
	addEvent("startTracking");
	addEvent("endTracking");
	_player.setPauseListener(this);
}

bool Sprite::animationPaused() {
	Core::ScriptableObject::Lifetime lifetime(*this);
	emitEvent("paused");
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

bool Sprite::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "userData")
		result = Core::ScriptValue::fromText(_userData);
	else if (property == "enabled")
		result = Core::ScriptValue(static_cast<int32>(enabled));
	else if (property == "isSolid")
		result = Core::ScriptValue(static_cast<int32>(solid));
	else if (property == "boxTouchy")
		result = Core::ScriptValue(static_cast<int32>(boxTouchy));
	else if (property == "tracking")
		result = Core::ScriptValue(static_cast<int32>(tracking));
	else if (property == "clickSoundID")
		result = Core::ScriptValue::fromText(_clickSoundID);
	else if (property == "flyOnSoundID")
		result = Core::ScriptValue::fromText(_flyOnSoundID);
	else if (property == "playDelay")
		result = Core::ScriptValue(static_cast<int32>(_playDelay));
	else if (property == "frame")
		result = Core::ScriptValue(static_cast<int32>(getFrame()));
	else if (property == "frameCount")
		result = Core::ScriptValue(static_cast<int32>(getNumberOfFrames()));
	else if (property == "pauseOnLastFrame")
		result = Core::ScriptValue(static_cast<int32>(_pauseOnLastFrame));
	else
		return Core::Trinket::get(property, arguments, result);
	return true;
}

bool Sprite::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "userData")
		_userData = value.text();
	else if (property == "enabled")
		enabled = value.integer() != 0;
	else if (property == "isSolid")
		solid = value.integer() != 0;
	else if (property == "boxTouchy")
		boxTouchy = value.integer() != 0;
	else if (property == "tracking")
		tracking = value.integer() != 0;
	else if (property == "clickSoundID")
		_clickSoundID = value.text();
	else if (property == "flyOnSoundID")
		_flyOnSoundID = value.text();
	else if (property == "clickControllerFXID")
		_clickControllerFXID = value.integer();
	else if (property == "flyOnControllerFXID")
		_flyOnControllerFXID = value.integer();
	else if (property == "playDelay")
		_playDelay = value.integer();
	else if (property == "corral" && arguments.size() == 3)
		corral = Common::Rect(arguments[0].integer(), arguments[1].integer(), arguments[2].integer(), value.integer());
	else if (property == "frame")
		setFrame(value.integer());
	else if (property == "pauseOnLastFrame")
		setPauseOnLastFrame(value.integer() != 0);
	else if (property == "repeatCount")
		setRepeat(value.integer());
	else if (property == "z" && _scene)
		_scene->setDepth(this, value.integer());
	else
		return Core::Trinket::set(property, arguments, value);
	return true;
}

bool Sprite::startAnimation(int repeat, uint sequence) {
	if (!asset || !_player.start(*asset, sequence, repeat))
		return false;
	if (_animated) {
		x -= _animationOffset.x;
		y -= _animationOffset.y;
	}
	_animated = true;
	_animationOffset = _player.offset();
	x += _animationOffset.x;
	y += _animationOffset.y;
	frame = _player.picture();
	return true;
}

bool Sprite::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (button && method == "setFlyOffRect" && arguments.size() == 4) {
		hitRect = Common::Rect(arguments[0].integer(), arguments[1].integer(), arguments[2].integer(), arguments[3].integer());
		return true;
	}
	if (!arguments.empty())
		return Core::Trinket::play(method, arguments, result);
	if (button && method == "default") {
		buttonPressed(callTime(), true);
		return true;
	}
	if (method == "forever") {
		if (!_animated && !startAnimation())
			return false;
		setRepeat(-1);
		playAnimation();
		return true;
	}
	if (method == "play" || method == "default" || method == "resume") {
		if (!_animated && !startAnimation())
			return false;
		playAnimation();
		return true;
	}
	if (method == "pause") {
		pauseAnimation(true);
		return true;
	}
	return Core::Trinket::play(method, arguments, result);
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
	x += offset.x - _animationOffset.x;
	y += offset.y - _animationOffset.y;
	_animationOffset = offset;
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
	Core::Animation::Step command = {0, 0, -4, 0};
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
		Core::Animation::Step stop = {0, 0, -4, 0};
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
		AnimationListener *listener = _animationListener;
		_animationListener = nullptr;
		if (listener)
			listener->animationFinished(*this);
		if (!lifetime.isAlive())
			return;
		complete();
		if (lifetime.isAlive())
			emitEvent("animDone");
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
		sprite->Core::Trinket::set("z", Core::ScriptArguments(), Core::ScriptValue(static_cast<int32>(z)));
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
		return _picture.getPixels() != nullptr;
	_cachedFrame = frame;
	if (!asset->decode(frame, _picture)) {
		_picture.free();
		return false;
	}
	return true;
}

void Scene::clear() {
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
	_background.free();
	_resources.clear();
}

bool Scene::setBackground(const Common::String &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_resources.open(path));
	if (!stream)
		return false;
	if (path.hasSuffix(".bk16"))
		return Core::Background16::decode(*stream, _background);
	Image::JPEGDecoder decoder;
	decoder.setOutputPixelFormat(Graphics::PixelFormat::createFormatRGBA32());
	if (!decoder.loadStream(*stream))
		return false;
	_background.copyFrom(*decoder.getSurface());
	return true;
}

Sprite *Scene::add(const Common::String &path, int z, int action, bool button) {
	Core::Animation *asset = nullptr;
	auto found = _assets.find(path);
	if (found != _assets.end()) {
		asset = found->_value;
	} else {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_resources.open(path));
		if (!stream)
			return nullptr;
		asset = new Core::Animation();
		if (!asset->load(*stream)) {
			delete asset;
			return nullptr;
		}
		if (path.hasSuffix(".ao")) {
			Common::ScopedPtr<Common::SeekableReadStream> palette(_resources.open(path.substr(0, path.size() - 3) + ".rgb"));
			if (palette && !asset->loadPalette(*palette)) {
				delete asset;
				return nullptr;
			}
		}
		_assets[path] = asset;
	}
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
	if (_background.getPixels())
		canvas.blit(_background, -_scrollX, 0);
}

void Scene::drawSprites(OM::Canvas &canvas, int minimumZ, int maximumZ) {
	for (uint i = 0; i < _sprites.size(); i++) {
		Sprite &sprite = *_sprites[i];
		if (!sprite.visible || sprite.z < minimumZ || maximumZ < sprite.z)
			continue;
		if (sprite.cachePicture()) {
			int x = sprite.mirrored ? 2 * sprite.mirrorX - sprite.x - sprite._picture.w : sprite.x;
			x -= sprite.scrolling ? _scrollX : 0;
			sprite.bounds = Common::Rect(x, sprite.y, x + sprite._picture.w, sprite.y + sprite._picture.h);
			canvas.blit(sprite._picture, x, sprite.y, sprite.mirrored);
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
	int left = mirrored ? 2 * mirrorX - x - _picture.w : x;
	left -= scrolling && _scene ? _scene->_scrollX : 0;
	bounds = Common::Rect(left, y, left + _picture.w, y + _picture.h);
	if (!bounds.contains(point))
		return false;
	if (solid || boxTouchy)
		return true;
	byte a, r, g, b;
	int column = mirrored ? _picture.w - 1 - (point.x - left) : point.x - left;
	uint32 pixel = *static_cast<const uint32 *>(_picture.getBasePtr(column, point.y - y));
	_picture.format.colorToARGB(pixel, a, r, g, b);
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
	if (flyButton && _flyOnSoundID != "-1") {
		_pendingSound = _flyOnSoundID;
		_clickPending = true;
	}
}

bool Sprite::pollClickSound(Common::String &resource) {
	if (!_clickPending)
		return false;
	_clickPending = false;
	resource = _pendingSound;
	return resource != "-1";
}

Core::ScriptableObject *Sprite::clone() const {
	if (!_scene)
		return nullptr;
	Sprite *copy = new Sprite();
	copy->_scene = _scene;
	copy->asset = asset;
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
	copy->_animationOffset = _animationOffset;
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
	_resources.clear();
}

} // namespace Shell
} // namespace Trinket
