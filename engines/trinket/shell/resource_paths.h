/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_RESOURCE_PATHS_H
#define TRINKET_SHELL_RESOURCE_PATHS_H

#include "common/array.h"
#include "trinket/core/object.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace Shell {

/** Ordered virtual search paths over a borrowed INX/BUL provider. */
class ResourcePaths : public OM::ResourceProvider {
public:
	explicit ResourcePaths(const OM::ResourceProvider &provider) : _provider(provider) {}
	void addPath(const Common::String &path);
	void removeAllPaths() { _paths.clear(); }
	void use(const Common::String &directory);
	Common::String resolve(const Common::String &name) const;
	Common::String resourcePath(const Common::String &id, const Common::String &extension) const;
	Common::SeekableReadStream *open(const Common::String &name) const override;

private:
	const OM::ResourceProvider &_provider;
	Common::Array<Common::String> _paths;
	Common::String _directory;
	static Common::String normalize(const Common::String &name);
	Common::String find(const Common::String &name, Common::SeekableReadStream *&stream) const;
};

/** RResourcePort borrows the application's cache/module controller. */
class ResourcePort : public Core::ScriptableObject {
public:
	class Host {
	public:
		virtual ~Host() {}
		virtual bool resourceCommand(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) = 0;
	};
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		explicit Factory(Host &host) : _host(host) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &) override { return new ResourcePort(_host); }

	private:
		Host &_host;
	};
	explicit ResourcePort(Host &host) : _host(host) {}
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override {
		return _host.resourceCommand(method, arguments, result);
	}

private:
	Host &_host;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
