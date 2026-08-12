MODULE := engines/zoombini2

MODULE_OBJS := \
	game_state.o \
	gfx.o \
	help_screen.o \
	metaengine.o \
	path.o \
	sidebar.o \
	sound.o \
	ui.o \
	pages/page.o \
	pages/video.o \
	pages/title.o \
	pages/zombiniville.o \
	pages/maptrans.o \
	pages/puzzle.o \
	pages/aquacube.o \
	pages/booliewood.o \
	pages/boolies.o \
	pages/cheznorf.o \
	pages/crazyturtle.o \
	pages/magicwall.o \
	pages/mysticmarsh.o \
	pages/snowboard.o \
	pages/waterslide.o \
	pages/walloffleens.o \
	pages/worldmap.o \
	pages/save_file_list.o \
	pages/menuscreen.o \
	pages/rescue.o \
	pages/credits.o \
	pages/final.o \
	zoombini.o \
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
