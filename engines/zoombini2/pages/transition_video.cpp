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

#include "common/debug.h"

#include "audio/mixer.h"

#include "video/bink_decoder.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_video.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *TransitionVideo::kLogoTlcPath;
constexpr const char *TransitionVideo::kLogoTlcHalfPath;
constexpr const char *TransitionVideo::kLogoPolygonPath;
constexpr const char *TransitionVideo::kCutsceneFirstPath;
constexpr const char *TransitionVideo::kCutsceneFirstHalfPath;
constexpr const char *TransitionVideo::kCutsceneSecondPath;
constexpr const char *TransitionVideo::kCutsceneSecondHalfPath;
constexpr const char *TransitionVideo::kCutsceneThirdPath;
constexpr const char *TransitionVideo::kCutsceneThirdHalfPath;
constexpr const char *TransitionVideo::kLogoArisuPath;

TransitionVideo::TransitionVideo(Zoombini2Engine *vm, PageId pageId)
	: TransitionBase(vm) {
	_pageId = pageId;

	switch (pageId) {
	case kPageLogoTLC:
		_videoPath = selectMoviePath(kLogoTlcPath, kLogoTlcHalfPath);
		_nextPageId = kPageLogoPolygon;
		break;
	case kPageLogoPolygon:
		_videoPath = Common::Path(kLogoPolygonPath);
		_nextPageId = kPageTitleScreen;
		break;
	case kPageCutsceneFirst:
		_videoPath = selectMoviePath(kCutsceneFirstPath, kCutsceneFirstHalfPath);
		_nextPageId = kPageZombiniville;
		break;
	case kPageCutsceneSecond:
		_videoPath = selectMoviePath(kCutsceneSecondPath, kCutsceneSecondHalfPath);
		_nextPageId = kPageRescue1;
		break;
	case kPageCutsceneThird:
		_videoPath = selectMoviePath(kCutsceneThirdPath, kCutsceneThirdHalfPath);
		_nextPageId = kPageRescue2;
		break;
	case kPageLogoArisuMedia:
		_videoPath = Common::Path(kLogoArisuPath);
		_nextPageId = kPageLogoTLC;
		break;
	default:
		warning("TransitionVideo: Unknown page %d", static_cast<int>(pageId));
		break;
	}
}

Common::Path TransitionVideo::selectMoviePath(const char *fullSizePath, const char *halfSizePath) const {
	if (_vm->hasResource(fullSizePath))
		return Common::Path(fullSizePath);

	return Common::Path(halfSizePath);
}

TransitionVideo::~TransitionVideo() {
	if (_decoder) {
		_decoder->close();
		delete _decoder;
	}
	delete _lastFrame;
}

void TransitionVideo::init() {
	debug(1, "TransitionVideo::init - %s", _videoPath.toString('/').c_str());

	_decoder = new Video::BinkDecoder();
	_decoder->setSoundType(Audio::Mixer::kMusicSoundType);

	Common::SeekableReadStream *stream = _vm->openResourceFile(_videoPath.toString('/'));
	if (!stream || !_decoder->loadStream(stream)) {
		delete stream;
		warning("TransitionVideo: Failed to load %s", _videoPath.toString('/').c_str());
		// Skip to next page if video is missing
		_vm->requestPageChange(_nextPageId);
		return;
	}

	_decoder->start();
	_started = true;
}

void TransitionVideo::onUpdate() {
	if (!_started || !_decoder)
		return;

	if (_decoder->endOfVideo()) {
		_vm->requestPageChange(_nextPageId);
		return;
	}

	if (_decoder->needsUpdate()) {
		const Graphics::Surface *frame = _decoder->decodeNextFrame();
		if (frame) {
			const Size32 frameSize(frame->w, frame->h);
			const Size32 frameOffset = (ManagedSurface32::kScreenSize - frameSize) / 2;
			_framePos = Common::Point32(frameOffset.width, frameOffset.height);

			// Cache the decoded frame in screen format
			if (!_lastFrame || _lastFrame->w != frameSize.width || _lastFrame->h != frameSize.height) {
				delete _lastFrame;
				_lastFrame = _vm->_gfx->createSurface(frameSize);
			}
			_lastFrame->blitFrom(*frame);
		}
	}
}

void TransitionVideo::onRenderContent(ManagedSurface32 *screen) {
	if (_lastFrame)
		screen->blitFrom(*_lastFrame, _framePos);
}

EventHandleResult TransitionVideo::skip() {
	if (!_started)
		return EventHandleResult::kPassthrough;
	_vm->requestPageChange(_nextPageId);
	return EventHandleResult::kConsumed;
}

EventHandleResult TransitionVideo::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	return skip();
}

EventHandleResult TransitionVideo::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)key;
	(void)repeat;
	return skip();
}

} // End of namespace Zoombini2
