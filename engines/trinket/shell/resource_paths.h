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
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "trinket/core/object.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace Shell {

/**
 * Resolve resource names using the MPS working directory and ordered search paths.
 * Lookup starts in the current virtual directory, or at the provider root for names beginning with '/'.
 * Unresolved names then try search paths in insertion order.
 * These are resource paths, not host filesystem directories.
 */
class ResourcePaths : public OM::ResourceProvider {
public:
	/** Create a path resolver over the supplied resource provider. */
	explicit ResourcePaths(const OM::ResourceProvider &provider) : _provider(provider) {}
	/** Append a virtual search path used when the initial lookup fails. */
	void addPath(const Common::String &path);
	/** Remove fallback paths and invalidate successful request resolutions. */
	void removeAllPaths() {
		_paths.clear();
		_resolved.clear();
	}
	/** Change the virtual working directory independently of the search-path list. */
	void use(const Common::String &directory);
	/** Return the matched provider-relative name, or an empty string if lookup fails. */
	Common::String resolve(const Common::String &name) const;
	/**
	 * Append the extension if needed and return the resolved name anchored with '/'.
	 * The anchor lets consumers reopen it through this resolver without adding the working directory again.
	 * @return An anchored resource path, or an empty string if lookup fails.
	 */
	Common::String resourcePath(const Common::String &id, const Common::String &extension) const;
	/** Open through the current directory and ordered fallback candidates. */
	Common::SeekableReadStream *open(const Common::String &name) const override;
	/** Resolve availability without requesting payload bytes from indexed providers. */
	bool contains(const Common::String &name) const override { return !resolve(name).empty(); }
	/** Anchor the actual source identity so directory changes cannot alias cached assets. */
	Common::String cacheKey(const Common::String &name) const override;

private:
	/** Resource provider queried after resolving virtual paths. */
	const OM::ResourceProvider &_provider;
	/** Ordered normalized fallback directories. */
	Common::Array<Common::String> _paths;
	/** Current virtual working directory, without a leading slash. */
	Common::String _directory;
	/** Successful request resolutions within the current directory/search-path configuration. */
	mutable Common::HashMap<Common::String, Common::String> _resolved;
	/** Fold case and path separators without changing root anchoring. */
	static Common::String normalize(const Common::String &name);
	/** Resolve candidates in order, optionally opening the successful resource. */
	Common::String find(const Common::String &name, Common::SeekableReadStream *&stream, bool openStream) const;
	/** Check or open one candidate according to the requested lookup mode. */
	bool findCandidate(const Common::String &name, Common::SeekableReadStream *&stream, bool openStream) const;
};

/** Forward scripted resource commands to the application. */
class ResourcePort : public Core::ScriptableObject {
public:
	class Host {
	public:
		/** Allow destruction through the resource-command interface. */
		virtual ~Host() {}
		/** Execute a scripted resource command and write its result. */
		virtual bool resourceCommand(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) = 0;
	};
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		/** Connect resource-port creation to its command host. */
		explicit Factory(Host &host) : _host(host) {}
		/** Create a script object that forwards resource commands to the host. */
		Core::ScriptableObject *create(const Core::ScriptArguments &) override { return new ResourcePort(_host); }

	private:
		/** Application interface that handles resource commands. */
		Host &_host;
	};
	/** Connect the script resource port to its command host. */
	explicit ResourcePort(Host &host) : _host(host) {}
	/** Forward the script method and arguments to the resource-command host. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override {
		return _host.resourceCommand(method, sargs, result);
	}

private:
	/** Application interface that executes resource commands. */
	Host &_host;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
