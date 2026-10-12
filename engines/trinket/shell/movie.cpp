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

Movie::Movie(Host &host, const Common::String &filename, Common::Point position) : _host(host), _filename(filename) {
	x = position.x;
	y = position.y;
	movable = false;
	addEvent("finished");
	addEvent("freeze");
	addEvent("signal");
	_host.movieCreated(*this);
}

Movie::~Movie() {
	bindEvent("finished", nullptr);
	bindEvent("freeze", nullptr);
	bindEvent("signal", nullptr);
	stop(false);
	_host.movieDestroyed(*this);
	_frame.free();
}

bool Movie::load() {
	return _decoder.loadFile(Common::Path(_filename));
}

Core::ScriptableObject *Movie::Factory::create(const Core::ScriptArguments &arguments) {
	if (arguments.empty() || 3 < arguments.size())
		return nullptr;
	Common::Point position;
	if (1 < arguments.size())
		position.x = arguments[1].integer();
	if (2 < arguments.size())
		position.y = arguments[2].integer();
	Common::ScopedPtr<Movie> movie(new Movie(_host, arguments[0].text(), position));
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
			emitEvent("finished");
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
			emitEvent("freeze");
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
			emitEvent("freeze");
		else
			stop(true);
		return;
	}
	int nextFrame = _decoder.getCurFrame() + 2;
	if (_nextSignal < _signals.size() && _signals[_nextSignal] == nextFrame) {
		_nextSignal += 1;
		emitEvent("signal");
	}
}

void Movie::draw(OM::Canvas &canvas) {
	if (_frame.getPixels())
		canvas.blit(_frame, x, y);
}

bool Movie::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "frames" && arguments.empty())
		result = Core::ScriptValue(static_cast<int32>(_decoder.getFrameCount()));
	else if (property == "rate" && arguments.empty())
		result = Core::ScriptValue(static_cast<int32>(_decoder.getFrameRate().toInt()));
	else
		return Core::Trinket::get(property, arguments, result);
	return true;
}

bool Movie::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if ((method == "start" || method == "default") && arguments.empty()) {
		start();
		return true;
	}
	if ((method == "stop" || method == "release") && arguments.empty()) {
		Core::ScriptableObject::Lifetime lifetime(*this);
		stop(true);
		if (lifetime.isAlive() && method == "release")
			_decoder.close();
		return true;
	}
	if (method == "addSignals") {
		for (uint i = 0; i < arguments.size(); i++) {
			int frame = arguments[i].integer();
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
	return Core::Trinket::play(method, arguments, result);
}

} // namespace Shell
} // namespace Trinket
