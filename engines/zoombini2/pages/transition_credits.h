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

#ifndef ZOOMBINI2_PAGES_TRANSITION_CREDITS_H
#define ZOOMBINI2_PAGES_TRANSITION_CREDITS_H

#include "zoombini2/pages/transition_base.h"

namespace Zoombini2 {

class BitBlock;

/**
 * End credits presentation with an initial pause, vertical scroll, and end hold.
 * Dismissing it or reaching the end exits the game.
 * It owns the dispatched screen rather than a modal overlay on another page.
 */
class CreditsPage : public TransitionPage {
public:
	CreditsPage(Zoombini2Engine *engine);
	~CreditsPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

	// Credits uses conditional drawing via _redrawNeeded and never needs screen clear.
	// The default false from Page is correct, so no override needed.

private:
	uint32 _endTime;
	float _scrollY;
	int _maxScrollY;
	bool _scrollActive;
	bool _redrawNeeded;
	uint32 _lastUpdateTime;
	bool _initialWait;
	BitBlock *_background;

	int _musicId;
	bool _finished;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_CREDITS_H
