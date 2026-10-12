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

#include "trinket/rr1cloud9/print.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/translation.h"
#include "image/bmp.h"

namespace Trinket {
namespace RR1Cloud9 {

bool Print::save(const Graphics::Surface &image) {
	Common::Path directory = ConfMan.getPath("screenshotpath");
	Common::String name;
	Common::Path path;
	for (uint i = 1; i <= 10000; i++) {
		name = Common::String::format("%s-print-%04u.bmp", ConfMan.getActiveDomainName().c_str(), i);
		path = directory.join(name);
		if (!Common::FSNode(path).exists())
			break;
		name.clear();
	}
	Common::DumpFile output;
	bool ok = !name.empty() && output.open(path);
	if (ok)
		ok = Image::writeBMP(output, image) && output.flush() && !output.err();
	output.close();
	if (ok)
		g_system->displayMessageOnOSD(Common::U32String::format(_("Saved print image '%s'."), path.toString(Common::Path::kNativeSeparator).c_str()));
	else
		g_system->displayMessageOnOSD(_("Could not save the print image."));
	return ok;
}

bool Print::book(const Artwork &artwork, const OM::ResourceProvider &resources, int book) {
	if (book < 1 || Artwork::kBooks < book)
		return false;
	static constexpr int columns = 3, margin = 20;
	Graphics::Surface sheet;
	sheet.create(columns * (Artwork::kWidth + margin) + margin,
				 (Artwork::kFrames / columns) * (Artwork::kHeight + margin) + margin, Graphics::PixelFormat::createFormatRGBA32());
	sheet.fillRect(Common::Rect(sheet.w, sheet.h), sheet.format.RGBToColor(255, 255, 255));
	bool ok = true;
	for (int i = 0; i < Artwork::kFrames; i++) {
		Common::Array<uint16> pixels;
		if (!artwork.image(resources, book, i + 1, pixels)) {
			ok = false;
			break;
		}
		Graphics::Surface frame;
		Artwork::render(pixels, frame);
		sheet.copyRectToSurface(frame, margin + (i % columns) * (Artwork::kWidth + margin),
								margin + (i / columns) * (Artwork::kHeight + margin), Common::Rect(frame.w, frame.h));
		frame.free();
	}
	if (ok)
		ok = save(sheet);
	else
		g_system->displayMessageOnOSD(_("Could not save the print image."));
	sheet.free();
	return ok;
}
} // namespace RR1Cloud9
} // namespace Trinket
