MODULE := engines/zoombini2

MODULE_OBJS := \
	dialogs.o \
	graphics.o \
	pages/dialog_help.o \
	metaengine.o \
	path.o \
	sidebar.o \
	sound.o \
	state.o \
	ui.o \
	pages/page_base.o \
	pages/transition_video.o \
	pages/transition_title.o \
	pages/shelter_zombiniville.o \
	pages/transition_maptrans.o \
	pages/transition_maptrans_route.o \
	pages/puzzle_base.o \
	pages/puzzle_aquacube.o \
	pages/shelter_booliewood.o \
	pages/shelter_booliewood_final.o \
	pages/puzzle_boolies.o \
	pages/puzzle_cheznorf.o \
	pages/puzzle_crazyturtle.o \
	pages/puzzle_magicwall.o \
	pages/puzzle_mysticmarsh.o \
	pages/puzzle_snowboard.o \
	pages/puzzle_waterslide.o \
	pages/puzzle_walloffleens.o \
	pages/interactive_worldmap.o \
	pages/save_file_list.o \
	pages/interactive_menuscreen.o \
	pages/shelter_rescue.o \
	pages/transition_credits.o \
	zoombini2.o

MODULE_DIRS += \
	engines/zoombini2 \
	engines/zoombini2/pages

# This module can be built as a plugin
ifeq ($(ENABLE_ZOOMBINI2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
