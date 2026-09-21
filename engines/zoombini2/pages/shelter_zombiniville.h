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

#ifndef ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H
#define ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "zoombini2/pages/shelter_base.h"
#include "zoombini2/state.h"

namespace Zoombini2 {

class AnimationRunner;
class Animation;
class PathObject;
class ZoombiniAnimation;
class ZoombiniRunner;

/**
 * Zoombiniville is the starting shelter, where a party of 16 is assembled.
 *
 * Four feature stations assemble a party of 16 before route departure.
 */
class ShelterZombiniville : public ShelterBase {
public:
	/** Construct the starting shelter for @p vm. */
	ShelterZombiniville(Zoombini2Engine *vm);
	/** Release station animations and graphics. */
	~ShelterZombiniville() override;

	/** Load the shelter and initialize an empty boarding party. */
	void init() override;
	/** Advance feature controls, entrance paths, and concurrent return paths. */
	void onUpdate() override;
	/** Draw the shelter, feature stations, and boarding party. */
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Draw and advance the held Zoombini above the shared sidebar. */
	void renderDragOverlay(ManagedSurface32 *screen, bool advanceState) override;
	/** Refresh button availability and restart the runner under the final frame pointer. */
	void onPostRender() override;
	/** Dispatch a click to a feature station or party action. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Finish dragging the held boarding Zoombini. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Move the held boarding Zoombini with the pointer. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Return the held Zoombini, or the full party when enhanced Shift+Delete is pressed. */
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Return whether a full party of 16 can leave the shelter. */
	bool canUseGoButton() const override;
	/** Complete all active returns before the Map control saves or leaves the page. */
	void onMapButtonPressed() override;

private:
	/** Number of fixed slots in the Zombiniville boarding area. */
	static constexpr int kBoardingSlotCount = 16;
	/** Resource path. */
	static constexpr const char *kBackgroundPath = "#bmp/zombiniville/zoombiniville";
	/** Resource path. */
	static constexpr const char *kAreaMaskPath = "bmp/zombiniville/area.bmt";
	/** Resource path format. */
	static constexpr const char *kFeatureAnimationFormat = "bmp/zombiniville/pikaroll/z1pi%d%d.an";
	/** Resource path. */
	static constexpr const char *kQuickFillButtonPath = "bmp/zombiniville/BUMPER-1.AN";
	/** Resource path. */
	static constexpr const char *kBatchFillButtonPath = "bmp/zombiniville/bumper-16.an";
	/** Resource path. */
	static constexpr const char *kCreateButtonPath = "bmp/zombiniville/bumper-Valid.an";
	/** Resource path. */
	static constexpr const char *kBigZombAnimationPath = "bmp/zombiniville/BigZomb/BigZomb.anm";
	/** Resource path. */
	static constexpr const char *kLittleZombAnimationPath = "bmp/zombis/littleZomb.anm";
	/** Resource path. */
	static constexpr const char *kPickupZombAnimationPath = "bmp/zombis/pris/pris.anm";
	/** Resource path. */
	static constexpr const char *kIdleZombAnimationPath = "bmp/zombis/attente2/attenteZomb2.anm";
	/** Resource path. */
	static constexpr const char *kMusicPath = "#sounds/music/ZMR-PickerScreen.wav";
	/** Resource path. */
	static constexpr const char *kFeatureSelectSoundPath = "sounds/fx/Z-BS11.wav";
	/** Resource path. */
	static constexpr const char *kQuickFillSoundPath = "sounds/fx/Z-BS12.wav";
	/** Resource path. */
	static constexpr const char *kBatchFillSoundPath = "sounds/fx/Z-BS13.wav";
	/** Resource path. */
	static constexpr const char *kValidZoombiniSoundPath = "sounds/fx/Z-BS14.wav";
	/** Resource path. */
	static constexpr const char *kWrongZoombiniSoundPath = "sounds/fx/WrongZ.wav";

	/** Feature selection controls indexed by feature and value. */
	Animation *_featureButtons[ZmbTrait::kTraitCount][ZmbTrait::kTraitValueCount] = {};
	/** Play-once hover overlays indexed by feature and value. */
	AnimationRunner *_featureButtonRunners[ZmbTrait::kTraitCount][ZmbTrait::kTraitValueCount] = {};
	/** Control that selects a random valid feature combination. */
	Animation *_quickFillButton = nullptr;
	/** Hover and pointer-exit sequence for Quick Fill. */
	AnimationRunner *_quickFillButtonRunner = nullptr;
	/** Control that fills the remaining party with valid Zoombinis. */
	Animation *_batchFillButton = nullptr;
	/** Hover and pointer-exit sequence for Batch Fill. */
	AnimationRunner *_batchFillButtonRunner = nullptr;
	/** Control that creates the currently selected Zoombini. */
	Animation *_createButton = nullptr;
	/** Hover and pointer-exit sequence for Create Selected. */
	AnimationRunner *_createButtonRunner = nullptr;

	/** Borrowed immutable large-sprite grid retained in the engine cache. */
	const ZoombiniAnimation *_bigZombAnimation = nullptr;
	/** Borrowed immutable small-sprite grid retained in the engine cache. */
	const ZoombiniAnimation *_littleZombAnimation = nullptr;
	/** Borrowed immutable pickup-sprite grid retained in the engine cache. */
	const ZoombiniAnimation *_pickupZombAnimation = nullptr;
	/** Borrowed immutable random-idle sprite grid retained in the engine cache. */
	const ZoombiniAnimation *_idleZombAnimation = nullptr;

	/** Geometry and current value for one feature station. */
	struct FeatureStation {
		/** 32-bit hit-test areas for the station's five values. */
		Common::Rect32 buttonRects[ZmbTrait::kTraitValueCount];
		/** Draw positions for the station's five controls. */
		Common::Point32 drawPos[ZmbTrait::kTraitValueCount];
		/** Currently selected feature value. */
		int selectedValue = 0;
	};
	/** Four stations corresponding to the four visible features. */
	FeatureStation _stations[ZmbTrait::kTraitCount] = {};
	/** Counts of each feature value already present in the party. */
	int _featureCounts[ZmbTrait::kTraitCount][ZmbTrait::kTraitValueCount + 1] = {};

	/** 32-bit Quick Fill control hit-test area. */
	Common::Rect32 _quickFillRect = Common::Rect32();
	/** 32-bit Batch Fill control hit-test area. */
	Common::Rect32 _batchFillRect = Common::Rect32();
	/** 32-bit Create Selected control hit-test area. */
	Common::Rect32 _createRect = Common::Rect32();

	/** Zoombini states assigned to departure slots. */
	Common::Array<ZoombiniRunner *> _boardingZoombinis;
	/** Independent occupancy retained for one original Zombiniville slot. */
	struct BoardingSlotState {
		bool occupied = false;
	};
	/** Sixteen fixed boarding slots whose occupancy is independent of roster order. */
	BoardingSlotState _boardingSlots[kBoardingSlotCount] = {};
	/** Zoombinis currently following their leftward return paths. */
	Common::Array<ZoombiniRunner *> _departingZoombinis;

	/** Sound played when a feature value is selected. */
	int _sndFeatureSelect = -1;
	/** Sound played by Quick Fill. */
	int _sndQuickFill = -1;
	/** Sound played by Batch Fill. */
	int _sndBatchFill = -1;
	/** Sound played when a valid Zoombini joins the party. */
	int _sndValidZoombini = -1;
	/** Sound played when the selected combination cannot join. */
	int _sndWrongZoombini = -1;

	/** Generated name displayed for the current feature selection. */
	Common::String _currentName;

	/** Create one Zoombini from the current feature selection when valid. */
	bool createZoombini(bool allowConcurrentEntrances);
	/** Return whether the current feature selection satisfies party limits. */
	bool canCreateSelectedZoombini() const;
	/** Return whether any boarding Zoombini is still entering. */
	bool hasActiveEntrance() const;
	/** Return whether any boarding Zoombini is returning to the entrance. */
	bool hasActiveReturns() const;
	/** Return whether @p zoombini is following a return path. */
	bool isZoombiniReturning(const ZoombiniRunner *zoombini) const;
	/** Return whether the supplied trait combination fits the current party. */
	bool passesPackTraitLimits(const ZmbTrait &traits) const;
	/** Select a random feature combination. */
	void randomizeSelectedFeatures();
	/** Reset all four stations to their first values. */
	void resetSelectedFeatures();
	/** Recount feature values in the current boarding party. */
	void refreshFeatureCounts();
	/** Generate the displayed name for the current selection. */
	Common::String generateName();
	/** Create an entrance path ending at @p dest. */
	PathObject *createEntrancePath(const Common::Point32 &dest) const;
	/** Create a return path from @p start to the left-side entrance. */
	PathObject *createReturnPath(const Common::Point32 &start) const;
	/** Return the first free fixed boarding slot, or -1 when all slots are occupied. */
	int findFirstFreeSlot() const;
	/** Restore the four feature stations from @p zoombini. */
	void restoreSelectedFeatures(const ZoombiniRunner &zoombini);
	/** Begin returning @p zoombini and optionally restore its traits to the picker. */
	void sendZoombiniOff(ZoombiniRunner *zoombini, bool restoreFeatures);
	/** Begin returning every boarding Zoombini without changing the picker traits. */
	void sendAllZoombinisOff();
	/** Remove and release @p zoombini after its return path has completed. */
	void finishSendingZoombiniOff(ZoombiniRunner *zoombini);
	/** Complete and release every Zoombini still following a return path. */
	void finishAllZoombiniReturns();
	/** Initialize station and action-control hit-test geometry. */
	void setupFeatureRects();
	/** Reconstruct the page-authored hover timing tables for all 23 controls. */
	void setupHoverRunners();
	/** Draw active hover overlays and advance their pointer-exit sequences. */
	void drawHoverRunners(ManagedSurface32 *screen);
	/** Apply availability gates and reset each runner under the final frame pointer. */
	void updateHoverRunners();
	/** Return the boarding-area pos for @p index. */
	static Common::Point32 getSlotPosition(uint index);
	/** Build the stable-Y actor order without the held Zoombini. */
	void buildBoardingZoombiniDrawOrder(Common::Array<uint> &order) const;
	/** Draw non-held Zoombinis currently in the boarding area. */
	void drawBoardingZoombinis(ManagedSurface32 *screen) const;
	/** Return the currently dragged boarding Zoombini, or nullptr. */
	ZoombiniRunner *getDraggedZoombini() const;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_ZOMBINIVILLE_H
