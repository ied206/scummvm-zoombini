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

/** RBinkMovie owns decoder state; its host controls the exclusive window task. */
class Movie : public Core::Trinket {
public:
	class Host {
	public:
		virtual ~Host() {}
		virtual void movieCreated(Movie &movie) = 0;
		virtual void movieDestroyed(Movie &movie) = 0;
		virtual void moviePlaying(Movie &movie, bool playing) = 0;
	};
	Movie(Host &host, const Common::String &filename, Common::Point position);
	~Movie() override;
	bool load();
	void update();
	void draw(OM::Canvas &canvas);
	bool isRunning() const { return _running; }
	int width() override { return _decoder.getWidth(); }
	int height() override { return _decoder.getHeight(); }
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	Core::ScriptableObject *clone() const override;
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		explicit Factory(Host &host) : _host(host) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override;

	private:
		Host &_host;
	};

private:
	Host &_host;
	Common::String _filename;
	Video::BinkDecoder _decoder;
	Graphics::Surface _frame;
	Common::Array<int> _signals;
	uint _nextSignal = 0;
	bool _running = false, _freezeWhenDone = false, _atEnd = false;
	void start();
	void stop(bool notify);
	void addSignal(int frame);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
