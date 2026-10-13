MODULE := engines/trinket

MODULE_OBJS := \
	metaengine.o \
	trinket.o \
	dialogs.o \
	compression/lzo1x.o \
	ompp/canvas.o \
	ompp/font.o \
	ompp/resource_cache.o \
	ompp/sound.o \
	ompp/xml.o \
	puzzle/core.o \
	shell/resource_paths.o \
	shell/random.o \
	shell/music.o \
	launcher/application.o \
	launcher/registry.o \
	launcher/version.o \
	shell/savefiles.o \
	shell/scene.o \
	shell/scene_port.o \
	shell/scroller.o \
	shell/action.o \
	shell/animation_action.o \
	shell/character.o \
	shell/character_action.o \
	shell/sound_action.o \
	shell/xml_file.o \
	shell/multitrinket.o \
	shell/text.o \
	shell/text_object.o \
	shell/credits.o \
	core/animation.o \
	core/background.o \
	core/object.o \
	core/trinket.o \
	core/script_runtime.o \
	core/script.o

ifdef ENABLE_ZOOMBINI3
MODULE_OBJS += \
	zoombini3.o \
	shell/resource.o \
	shell/movie.o \
	zoombini3_pages/puzzle_catapult.o \
	zoombini3_character.o \
	zoombini3_attrib_ch_walk_action.o \
	zoombini3_radio.o \
	zoombini3_dialogs.o \
	zoombini3_saves.o \
	zoombini3_savefiles.o \
	zoombini3_world.o \
	zoombini3_world_port.o \
	zoombini3_game.o \
	zoombini3_script_session.o \
	zoombini3_pages/puzzle_glyph.o \
	zoombini3_pages/puzzle_planetarium.o \
	zoombini3_pages/puzzle_greenhouse.o \
	zoombini3_pages/puzzle_garden.o \
	zoombini3_pages/puzzle_corral.o \
	zoombini3_pages/puzzle_barn.o \
	zoombini3_slider.o \
	zoombini3_checkbox.o \
	zoombini3_options.o \
	zoombini3_quit.o \
	zoombini3_pages/interactive_ziv.o \
	zoombini3_version.o
endif

ifdef ENABLE_RR1CLOUD9
MODULE_OBJS += \
	compression/nc1.o \
	rr1cloud9/rr1cloud9.o \
	rr1cloud9/resources.o \
	rr1cloud9/savefiles.o \
	rr1cloud9/page.o \
	rr1cloud9/page_scene.o \
	rr1cloud9/cn2_queen.o \
	rr1cloud9/in4_hub.o \
	rr1cloud9/ln2_inventor.o \
	rr1cloud9/la1_gumball.o \
	rr1cloud9/ia1_number_tree.o \
	rr1cloud9/ia3_word_pots.o \
	rr1cloud9/ca1_cloud_floors.o \
	rr1cloud9/artwork.o \
	rr1cloud9/ca2_flipbook.o \
	rr1cloud9/la2_music_patterns.o \
	rr1cloud9/cn3_gallery.o \
	rr1cloud9/cn4_rainbow_slide.o \
	rr1cloud9/ig1_rock_head.o \
	rr1cloud9/cg2_sheep_crossing.o \
	rr1cloud9/lg2_catapult.o \
	rr1cloud9/rss_sky_ride.o \
	rr1cloud9/la3_rain_game.o \
	rr1cloud9/credits.o \
	rr1cloud9/options.o \
	rr1cloud9/print.o \
	rr1cloud9/signin.o \
	rr1cloud9/ia2_sandwich.o \
	rr1cloud9/world.o \
	rr1cloud9/game.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_TRINKET), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
