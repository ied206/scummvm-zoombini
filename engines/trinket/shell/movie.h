/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_MOVIE_H
#define TRINKET_SHELL_MOVIE_H

#include "trinket/core/trinket.h"
#include "trinket/ompp/canvas.h"
#include "video/bink_decoder.h"

namespace Trinket {
namespace Shell {

/** Play a Bink movie while the host coordinates its exclusive window task. */
class Movie : public Core::Trinket {
public:
	class Host {
	public:
		/** Allow destruction through the movie-host interface. */
		virtual ~Host() {}
		/** Register a newly created movie with its scene or application. */
		virtual void movieCreated(Movie &movie) = 0;
		/** Remove a movie that is being destroyed from host tracking. */
		virtual void movieDestroyed(Movie &movie) = 0;
		/** Notify the host when movie playback starts or stops. */
		virtual void moviePlaying(Movie &movie, bool playing) = 0;
	};
	/** Configure a movie file and its initial position in game pixels. */
	Movie(Host &host, const Common::String &filename, Common::Point position);
	/** Stop playback, unregister the movie and release its decoded frame. */
	~Movie() override;
	/** Open the configured Bink file and prepare its decoder. */
	bool load();
	/** Decode due frames and emit frame-signal or playback-completion events. */
	void update();
	/** Draw the most recently decoded frame at the movie's game-pixel position. */
	void draw(OM::Canvas &canvas);
	/** Return whether movie playback is active. */
	bool isRunning() const { return _running; }
	/** Return the decoded video's width in pixels. */
	int width() override { return _decoder.getWidth(); }
	/** Return the decoded video's height in pixels. */
	int height() override { return _decoder.getHeight(); }
	/** Read the movie's frame count, playback rate or inherited geometry. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Dispatch movie start, stop, release and frame-signal script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Create another movie with the same file, placement and signal settings. */
	Core::ScriptableObject *clone() const override;
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		/** Connect scripted movie creation to its host. */
		explicit Factory(Host &host) : _host(host) {}
		/** Create a movie from a filename and optional position arguments. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override;

	private:
		/** Scene or application notified of each created movie's playback state. */
		Host &_host;
	};

private:
	/** Script event name for finished. */
	static constexpr const char *kEventFinished = "finished";
	/** Script event name for freeze. */
	static constexpr const char *kEventFreeze = "freeze";
	/** Script event name for signal. */
	static constexpr const char *kEventSignal = "signal";
	/** Script property name for frames. */
	static constexpr const char *kPropertyFrames = "frames";
	/** Script property name for rate. */
	static constexpr const char *kPropertyRate = "rate";
	/** Script method name for start. */
	static constexpr const char *kMethodStart = "start";
	/** Script method name for default. */
	static constexpr const char *kMethodDefault = "default";
	/** Script method name for stop. */
	static constexpr const char *kMethodStop = "stop";
	/** Script method name for release. */
	static constexpr const char *kMethodRelease = "release";
	/** Script method name for add Signals. */
	static constexpr const char *kMethodAddSignals = "addSignals";

	/** Scene or application coordinating this movie's playback. */
	Host &_host;
	/** Movie file opened by the decoder. */
	Common::String _filename;
	/** Bink decoder supplying video frames and movie audio. */
	Video::BinkDecoder _decoder;
	/** Most recently decoded frame converted to RGBA pixels. */
	Graphics::Surface _frame;
	/** Sorted one-based frame numbers that trigger script signal events. */
	Common::Array<int> _signals;
	/** Index of the next frame signal not yet emitted. */
	uint _nextSignal = 0;
	/** Whether the decoder is currently advancing playback. */
	bool _running = false;
	/** Whether reaching the final frame emits freeze instead of stopping. */
	bool _freezeWhenDone = false;
	/** Whether playback has reached its stopping frame. */
	bool _atEnd = false;
	/** Start or restart movie playback and reset frame-signal delivery. */
	void start();
	/** Stop playback and optionally emit completion notifications. */
	void stop(bool notify);
	/** Insert a one-based frame signal without duplicating an existing signal. */
	void addSignal(int frame);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
