MODULE := engines/zoombini2

MODULE_OBJS := \
	zoombini2.o \
	dialogs.o \
	graphics.o \
	metaengine.o \
	random.o \
	scripts.o \
	sound.o \
	state.o \
	pages/dialog_help.o \
	pages/dialog_msgbox.o \
	pages/interactive_base.o \
	pages/interactive_map.o \
	pages/interactive_menu.o \
	pages/page_base.o \
	pages/puzzle_base.o \
	pages/puzzle_aquacube.o \
	pages/puzzle_boolies.o \
	pages/puzzle_cheznorf.o \
	pages/puzzle_crazyturtle.o \
	pages/puzzle_magicwall.o \
	pages/puzzle_mysticmarsh.o \
	pages/puzzle_snowboard.o \
	pages/puzzle_walloffleens.o \
	pages/puzzle_waterslide.o \
	pages/save_file_list.o \
	pages/shelter_base.o \
	pages/shelter_booliewood.o \
	pages/shelter_final.o \
	pages/shelter_rescue1.o \
	pages/shelter_rescue2.o \
	pages/shelter_zombiniville.o \
	pages/transition_credits.o \
	pages/transition_maptrans.o \
	pages/transition_title.o \
	pages/transition_video.o

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
