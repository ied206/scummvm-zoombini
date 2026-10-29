/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/movie.h"

#include "common/ptr.h"

namespace Trinket {
namespace Shell {

constexpr const char *Movie::kEventFinished;
constexpr const char *Movie::kEventFreeze;
constexpr const char *Movie::kEventSignal;
constexpr const char *Movie::kPropertyFrames;
constexpr const char *Movie::kPropertyRate;
constexpr const char *Movie::kMethodStart;
constexpr const char *Movie::kMethodDefault;
constexpr const char *Movie::kMethodStop;
constexpr const char *Movie::kMethodRelease;
constexpr const char *Movie::kMethodAddSignals;

Movie::Movie(Host &host, const Common::String &filename, Common::Point position) : _host(host), _filename(filename) {
	x = position.x;
	y = position.y;
	movable = false;
	addEvent(kEventFinished);
	addEvent(kEventFreeze);
	addEvent(kEventSignal);
	_host.movieCreated(*this);
}

Movie::~Movie() {
	bindEvent(kEventFinished, nullptr);
	bindEvent(kEventFreeze, nullptr);
	bindEvent(kEventSignal, nullptr);
	stop(false);
	_host.movieDestroyed(*this);
	_frame.free();
}

bool Movie::load() {
	return _decoder.loadFile(Common::Path(_filename));
}

Core::ScriptableObject *Movie::Factory::create(const Core::ScriptArguments &sargs) {
	if (sargs.empty() || 3 < sargs.size())
		return nullptr;
	Common::Point position;
	if (1 < sargs.size())
		position.x = sargs[1].toInt32();
	if (2 < sargs.size())
		position.y = sargs[2].toInt32();
	Common::ScopedPtr<Movie> movie(new Movie(_host, sargs[0].toText(), position));
	return movie->load() ? movie.release() : nullptr;
}

Core::ScriptableObject *Movie::clone() const {
	Common::ScopedPtr<Movie> movie(new Movie(_host, _filename, Common::Point(x, y)));
	if (!movie->load())
		return nullptr;
	movie->_freezeWhenDone = _freezeWhenDone;
	movie->_signals = _signals;
	movie->z = z;
	return movie.release();
}

void Movie::start() {
	if (_running || !_decoder.isVideoLoaded())
		return;
	_nextSignal = 0;
	_running = true;
	_host.moviePlaying(*this, true);
	_decoder.start();
}

void Movie::stop(bool notify) {
	if (!_running)
		return;
	_running = false;
	_decoder.stop();
	_host.moviePlaying(*this, false);
	if (notify) {
		Core::ScriptableObject::Lifetime lifetime(*this);
		complete();
		if (lifetime.isAlive())
			emitEvent(kEventFinished);
	}
}

void Movie::addSignal(int frame) {
	uint i = 0;
	while (i < _signals.size() && _signals[i] < frame)
		i += 1;
	if (i == _signals.size() || _signals[i] != frame)
		_signals.insert_at(i, frame);
}

void Movie::update() {
	if (!_running)
		return;
	if (_atEnd) {
		if (_freezeWhenDone)
			emitEvent(kEventFreeze);
		else
			stop(true);
		return;
	}
	if (!_decoder.needsUpdate())
		return;
	const Graphics::Surface *frame = _decoder.decodeNextFrame();
	if (frame) {
		Common::ScopedPtr<Graphics::Surface> converted(frame->convertTo(Graphics::PixelFormat::createFormatRGBA32()));
		_frame.free();
		_frame.copyFrom(*converted);
		converted->free();
	}
	// Native FrameNum is one-based and stops on Frames - 1, before advancing.
	_atEnd = _decoder.endOfVideo() || _decoder.getCurFrame() + 2 >= static_cast<int>(_decoder.getFrameCount());
	if (_atEnd) {
		if (_freezeWhenDone)
			emitEvent(kEventFreeze);
		else
			stop(true);
		return;
	}
	int nextFrame = _decoder.getCurFrame() + 2;
	if (_nextSignal < _signals.size() && _signals[_nextSignal] == nextFrame) {
		_nextSignal += 1;
		emitEvent(kEventSignal);
	}
}

void Movie::draw(OM::Canvas &canvas) {
	if (_frame.getPixels())
		canvas.blit(_frame, x, y);
}

bool Movie::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyFrames) && sargs.empty())
		result = Core::ScriptValue(static_cast<int32>(_decoder.getFrameCount()));
	else if (property.equals(kPropertyRate) && sargs.empty())
		result = Core::ScriptValue(static_cast<int32>(_decoder.getFrameRate().toInt()));
	else
		return Core::Trinket::get(property, sargs, result);
	return true;
}

bool Movie::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if ((method.equals(kMethodStart) || method.equals(kMethodDefault)) && sargs.empty()) {
		start();
		return true;
	}
	if ((method.equals(kMethodStop) || method.equals(kMethodRelease)) && sargs.empty()) {
		Core::ScriptableObject::Lifetime lifetime(*this);
		stop(true);
		if (lifetime.isAlive() && method.equals(kMethodRelease))
			_decoder.close();
		return true;
	}
	if (method.equals(kMethodAddSignals)) {
		for (uint i = 0; i < sargs.size(); i++) {
			int frame = sargs[i].toInt32();
			if (frame == -2) {
				_freezeWhenDone = true;
				break;
			}
			if (frame == -1) {
				for (uint j = 0; j < _decoder.getFrameCount(); j++)
					addSignal(j);
				break;
			}
			addSignal(frame <= 0 ? 0 : frame - 1);
		}
		return true;
	}
	return Core::Trinket::play(method, sargs, result);
}

} // namespace Shell
} // namespace Trinket
