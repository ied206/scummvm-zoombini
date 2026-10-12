/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef TRINKET_RR1CLOUD9_PRINT_H
#define TRINKET_RR1CLOUD9_PRINT_H

#include "trinket/rr1cloud9/artwork.h"

namespace Trinket {
namespace RR1Cloud9 {

/** Portable image output for the original report and picture-book print commands. */
class Print {
public:
	static bool save(const Graphics::Surface &image);
	static bool book(const Artwork &artwork, const OM::ResourceProvider &resources, int book);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
