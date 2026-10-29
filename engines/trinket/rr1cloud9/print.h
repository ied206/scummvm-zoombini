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
	/** Export the supplied print image as a numbered BMP in the screenshot directory. */
	static bool save(const Graphics::Surface &image);
	/** Render a flipbook contact sheet and export it as a print image. */
	static bool book(const Artwork &artwork, const OM::ResourceProvider &resources, int book);
private:
	/** Printed page filename indexed by target and sequence number. */
	static constexpr const char *kFileNameFormat = "%s-print-%04u.bmp";
	/** ScummVM directory setting used for printed pages. */
	static constexpr const char *kScreenshotPathOption = "screenshotpath";


};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
