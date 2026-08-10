#============================================================================
# Copyright (C) 2013 - 2018, OpenJK contributors
#
# This file is part of the OpenJK source code.
#
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

# Make sure the user is not executing this script directly
if(NOT IN_OPENJO)
    message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT IN_OPENJO)

set(SHARED_COMMON_FILES
    ${SRC_DIR}/qcommon/q_assert.h
    ${SRC_DIR}/qcommon/q_assert.cpp
    ${SRC_DIR}/qcommon/q_color.h
    ${SRC_DIR}/qcommon/q_color.c
    ${SRC_DIR}/qcommon/q_math.h
    ${SRC_DIR}/qcommon/q_math.c
    ${SRC_DIR}/qcommon/q_string.h
    ${SRC_DIR}/qcommon/q_string.c
    ${SRC_DIR}/qcommon/q_platform.h
    ${SRC_DIR}/qcommon/safe/gsl.h
    ${SRC_DIR}/qcommon/safe/string.cpp
    ${SRC_DIR}/qcommon/safe/string.h
    ${SRC_DIR}/qcommon/safe/sscanf.h
    ${SRC_DIR}/qcommon/safe/limited_vector.h
    ${CMAKE_BINARY_DIR}/src/qcommon/q_version.h
)

#    Add Modular Renderer project
add_subdirectory("rd-vanilla")

#    Add Engine Project

# Libraries
if(WIN32)
    set(SP_ENGINE_LIBRARIES "winmm")
endif(WIN32)

# Defines
set(SP_ENGINE_DEFINES ${SHARED_DEFINES} "_JK2EXE") # it's called JK2EXE but it really just means Singleplayer Exe

set(SP_ENGINE_INCLUDE_DIRS
    ${SRC_DIR}
    ${GSL_DIR}
    ${CMAKE_BINARY_DIR}/src
)

# Dependencies

# OpenAL (is optionally included for Windows)
if(MSVC AND NOT WIN64)
    if(USE_INTERNAL_OPENAL)
        # There's a libs/OpenAL32.lib
        # (mrw) TODO Is this actually required on windows? It is loaded at runtime, should not be linked?
        find_library(OPENAL_LIBRARY NAMES OpenAL32 PATHS "${LIB_DIR}")
        if(NOT OPENAL_LIBRARY)
            message(FATAL_ERROR "USE_INTERNAL_OPENAL enabled, but lib/OpenAL32.lib not found!")
        endif(NOT OPENAL_LIBRARY)
        set(SP_ENGINE_LIBRARIES ${SP_ENGINE_LIBRARIES} ${OPENAL_LIBRARY})
    else(USE_INTERNAL_OPENAL)
        find_package(OpenAL REQUIRED)
        set(SP_ENGINE_INCLUDE_DIRS ${SP_ENGINE_INCLUDE_DIRS} ${OPENAL_INCLUDE_DIR})
        set(SP_ENGINE_LIBRARIES ${SP_ENGINE_LIBRARIES} ${OPENAL_LIBRARY})
    endif(USE_INTERNAL_OPENAL)

    set(SP_ENGINE_FILES ${SP_ENGINE_FILES}
        client/eax/eax.h
        client/eax/EaxMan.h
    )
endif()

if(USE_INTERNAL_SDL2)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(SP_ENGINE_LIBRARIES
            ${SP_ENGINE_LIBRARIES}
            ${LIB_DIR}/SDL2/lib/x86/SDL2main.lib
            ${LIB_DIR}/SDL2/lib/x86/SDL2.lib
        )
    else()
        set(SP_ENGINE_LIBRARIES
            ${SP_ENGINE_LIBRARIES}
            ${LIB_DIR}/SDL2/lib/x64/SDL2main.lib
            ${LIB_DIR}/SDL2/lib/x64/SDL2.lib
        )
    endif()

    set(SP_ENGINE_INCLUDE_DIRS
        ${SP_ENGINE_INCLUDE_DIRS}
        ${LIB_DIR}/SDL2/include
    )
else()
    find_package(SDL2 REQUIRED CONFIG)
    if (TARGET SDL2::SDL2)
        set(SDL2_INCLUDE_DIRS "")
        set(SDL2_LIBRARIES SDL2::SDL2)
    endif()
    string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
    set(SP_ENGINE_INCLUDE_DIRS ${SP_ENGINE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
    set(SP_ENGINE_LIBRARIES ${SP_ENGINE_LIBRARIES} ${SDL2_LIBRARIES})
endif()

# Source Files
set(SP_ENGINE_FILES ${SP_ENGINE_FILES}
    cgame/cg_public.h

    client/keys.h
    client/keycodes.h
    client/cl_cgame.cpp
    client/cl_cin.cpp
    client/cl_console.cpp
    client/cl_input.cpp
    client/cl_keys.cpp
    client/cl_main.cpp
    client/cl_mp3.cpp
    client/cl_parse.cpp
    client/cl_scrn.cpp
    client/cl_ui.cpp
    client/cl_mp3.h
    client/client.h
    client/client_ui.h
    client/snd_ambient.cpp
    client/snd_dma.cpp
    client/snd_mem.cpp
    client/snd_mix.cpp
    client/snd_music.cpp
    client/snd_ambient.h
    client/snd_local.h
    client/snd_music.h
    client/snd_public.h
    client/vmachine.cpp
    client/vmachine.h

    game/genericparser2.cpp
    game/channels.h
    game/anims.h
    game/animtable.cpp
    game/b_public.h
    game/bg_public.h
    game/bset.h
    game/bstate.h
    game/g_functions.h
    game/g_items.h
    game/g_local.h
    game/g_nav.h
    game/g_public.h
    game/g_shared.h
    game/genericparser2.h
    game/statindex.h
    game/teams.h
    game/weapons.h
    game/surfaceflags.h

    ghoul2/G2.h
    ghoul2/ghoul2_gore.h
    ghoul2/ghoul2_shared.h

    mp3code/cdct.c
    mp3code/csbt.c
    mp3code/csbtb.c
    mp3code/csbtl3.c
    mp3code/cup.c
    mp3code/cupini.c
    mp3code/cupl1.c
    mp3code/cupl3.c
    mp3code/cwin.c
    mp3code/cwinb.c
    mp3code/cwinm.c
    mp3code/hwin.c
    mp3code/l3dq.c
    mp3code/l3init.c
    mp3code/mdct.c
    mp3code/mhead.c
    mp3code/msis.c
    mp3code/towave.c
    mp3code/uph.c
    mp3code/upsf.c
    mp3code/wavep.c
    mp3code/config.h
    mp3code/htable.h
    mp3code/jdw.h
    mp3code/l3.h
    mp3code/mhead.h
    mp3code/mp3struct.h
    mp3code/port.h
    mp3code/small_header.h
    mp3code/tableawd.h

    qcommon/safe/files.cpp
    qcommon/safe/files.h
    qcommon/safe/memory.h
    qcommon/cm_load.cpp
    qcommon/cm_patch.cpp
    qcommon/cm_polylib.cpp
    qcommon/cm_test.cpp
    qcommon/cm_trace.cpp
    qcommon/cm_local.h
    qcommon/cm_patch.h
    qcommon/cm_polylib.h
    qcommon/cm_public.h
    qcommon/cmd.cpp
    qcommon/common.cpp
    qcommon/cvar.cpp
    qcommon/files.cpp
    qcommon/md4.cpp
    qcommon/msg.cpp
    qcommon/net_chan.cpp
    qcommon/q_shared.cpp
    qcommon/q_shared.h
    qcommon/sstring.h
    qcommon/strip.cpp
    qcommon/persistence.cpp
    qcommon/tags.h
    qcommon/z_memman_pc.cpp
    qcommon/qfiles.h
    qcommon/qcommon.h
    qcommon/stv_version.h
    qcommon/timing.h
    qcommon/MiniHeap.h
    qcommon/ojk_i_saved_game.h
    qcommon/ojk_saved_game.h
    qcommon/ojk_saved_game.cpp
    qcommon/ojk_saved_game_helper.h
    qcommon/ojk_saved_game_helper_fwd.h
    qcommon/ojk_scope_guard.h

    rd-common/mdx_format.h
    rd-common/tr_public.h
    rd-common/tr_types.h

    sdl/sdl_qgl.h
    sdl/sdl_sound.cpp
    sdl/sdl_input.cpp
    sdl/sdl_window.cpp
    sdl/sdl_icon.h

    server/exe_headers.cpp
    server/sv_ccmds.cpp
    server/sv_client.cpp
    server/sv_game.cpp
    server/sv_init.cpp
    server/sv_main.cpp
    server/sv_savegame.cpp
    server/sv_snapshot.cpp
    server/sv_world.cpp
    server/exe_headers.h
    server/server.h

    sys/sys_main.cpp
    sys/sys_event.cpp
    sys/sys_local.h
    sys/sys_public.h
    sys/sys_loadlib.h
    sys/con_local.h
    sys/con_log.cpp

    ui/ui_atoms.cpp
    ui/ui_connect.cpp
    ui/ui_layout.cpp
    ui/ui_define.cpp
    ui/ui_main.cpp
    ui/ui_shared.cpp
    ui/ui_syscalls.cpp
    ui/gameinfo.h
    ui/menudef.h
    ui/ui_define.h
    ui/ui_layout.h
    ui/ui_local.h
    ui/ui_public.h
    ui/ui_shared.h

    ${SHARED_COMMON_FILES}
)

if(WIN32)
    set(SP_ENGINE_FILES
        ${SP_ENGINE_FILES}
        sys/sys_win32.cpp
        sys/con_passive.cpp
        win32/resource.h
        win32/winquake.rc
        sys/win_manifest.manifest
    )
else()
    set(SP_ENGINE_FILES
        ${SP_ENGINE_FILES}
        sys/sys_unix.cpp
        sys/con_tty.cpp
    )

    if(DECODE_STACKTRACE)
        include(FindPkgConfig)
        pkg_check_modules(BACKTRACE libbacktrace)
        if(BACKTRACE_FOUND)
            list(APPEND SP_ENGINE_INCLUDE_DIRS ${BACKTRACE_INCLUDE_DIRS})
            list(APPEND SP_ENGINE_LIBRARIES ${BACKTRACE_LIBRARIES})
            set(SP_ENGINE_LIBRARY_DIRS  ${BACKTRACE_LIBRARY_DIRS})
            set(SHARED_DEFINES ${SHARED_DEFINES} "OPENJO_STACKTRACE")
        else()
            set(DECODE_STACKTRACE OFF)
        endif()
    endif()
endif()

# Transparently use our bundled minizip.
list(APPEND SP_ENGINE_INCLUDE_DIRS  ${MINIZIP_INCLUDE_DIRS})
list(APPEND SP_ENGINE_LIBRARIES     ${MINIZIP_LIBRARIES})

# Transparently use either bundled or system zlib.
list(APPEND SP_ENGINE_INCLUDE_DIRS  ${ZLIB_INCLUDE_DIR})
list(APPEND SP_ENGINE_LIBRARIES     ${ZLIB_LIBRARIES})

if(APPLE)
    set_source_files_properties(${SRC_DIR}/macosx/OpenJK.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(SP_ENGINE_FILES ${SP_ENGINE_FILES} ${SRC_DIR}/macosx/OpenJK.icns)
    set_source_files_properties(${SRC_DIR}/macosx/English.lproj/InfoPlist.strings PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj)
    set(SP_ENGINE_FILES ${SP_ENGINE_FILES} ${SRC_DIR}/macosx/English.lproj/InfoPlist.strings)
endif(APPLE)

if(WIN32)
    add_executable(${ENGINE_NAME} WIN32 ${SP_ENGINE_FILES})
    if(USE_INTERNAL_SDL2)
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(SP_ENGINE_EXTRA_INSTALL_FILES
                ${LIB_DIR}/SDL2/bin/x86/SDL2.dll
            )
        else()
            set(SP_ENGINE_EXTRA_INSTALL_FILES
                ${LIB_DIR}/SDL2/bin/x64/SDL2.dll
            )
        endif()
    endif()
elseif(APPLE)
    add_executable(${ENGINE_NAME} MACOSX_BUNDLE ${SP_ENGINE_FILES})
    set_target_properties(${ENGINE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${SRC_DIR}/macosx/OpenJKInfo.plist")
else()
    add_executable(${ENGINE_NAME} ${SP_ENGINE_FILES})
endif()

if(APPLE)
    install(CODE "
        include(BundleUtilities)
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        fixup_bundle(\"${CMAKE_BINARY_DIR}/${ENGINE_NAME}.app\" \"\" \"\")
        "
        COMPONENT Runtime)
    install(TARGETS ${ENGINE_NAME}
        BUNDLE
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        COMPONENT ${JK2SP_CLIENT_COMPONENT})
else()
    install(TARGETS ${ENGINE_NAME}
        RUNTIME
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        COMPONENT ${JK2SP_CLIENT_COMPONENT})
    if(SP_ENGINE_EXTRA_INSTALL_FILES)
        install(FILES ${SP_ENGINE_EXTRA_INSTALL_FILES}
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            COMPONENT ${JK2SP_CLIENT_COMPONENT})
    endif()
endif()

set_target_properties(${ENGINE_NAME} PROPERTIES COMPILE_DEFINITIONS "${SP_ENGINE_DEFINES}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${ENGINE_NAME} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})

target_include_directories(${ENGINE_NAME} PRIVATE "${SP_ENGINE_INCLUDE_DIRS}")
set_target_properties(${ENGINE_NAME} PROPERTIES PROJECT_LABEL "JK2 SP Client")
if(SP_ENGINE_LIBRARIES)
    if(SP_ENGINE_LIBRARY_DIRS)
        target_link_directories(${ENGINE_NAME} PRIVATE ${SP_ENGINE_LIBRARY_DIRS})
    endif()
    target_link_libraries(${ENGINE_NAME} ${SP_ENGINE_LIBRARIES})
endif(SP_ENGINE_LIBRARIES)

set(JK2SP_GAME_DEFINES ${SHARED_DEFINES} "JK2_MODE" "SP_GAME")

add_library(${GAME_NAME} SHARED
    game/animtable.cpp
    game/AI_Atst.cpp
    game/AI_Default.cpp
    game/AI_Droid.cpp
    game/AI_GalakMech.cpp
    game/AI_Grenadier.cpp
    game/AI_Howler.cpp
    game/AI_ImperialProbe.cpp
    game/AI_Interrogator.cpp
    game/AI_Jedi.cpp
    game/AI_Mark1.cpp
    game/AI_Mark2.cpp
    game/AI_MineMonster.cpp
    game/AI_Remote.cpp
    game/AI_Seeker.cpp
    game/AI_Sentry.cpp
    game/AI_Sniper.cpp
    game/AI_Stormtrooper.cpp
    game/AI_Utils.cpp
    game/G_Timer.cpp
    game/NPC.cpp
    game/NPC_behavior.cpp
    game/NPC_combat.cpp
    game/NPC_goal.cpp
    game/NPC_misc.cpp
    game/NPC_move.cpp
    game/NPC_reactions.cpp
    game/NPC_senses.cpp
    game/NPC_sounds.cpp
    game/NPC_spawn.cpp
    game/NPC_stats.cpp
    game/NPC_utils.cpp
    game/Q3_Interface.cpp
    game/Q3_Registers.cpp
    game/bg_misc.cpp
    game/bg_pangles.cpp
    game/bg_panimate.cpp
    game/bg_pmove.cpp
    game/bg_slidemove.cpp
    game/g_ICARUS.cpp
    game/g_active.cpp
    game/g_breakable.cpp
    game/g_camera.cpp
    game/g_client.cpp
    game/g_cmds.cpp
    game/g_combat.cpp
    game/g_functions.cpp
    game/g_fx.cpp
    game/g_inventory.cpp
    game/g_itemLoad.cpp
    game/g_items.cpp
    game/g_main.cpp
    game/g_mem.cpp
    game/g_misc.cpp
    game/g_misc_model.cpp
    game/g_missile.cpp
    game/g_mover.cpp
    game/g_nav.cpp
    game/g_navnew.cpp
    game/g_navigator.cpp
    game/g_object.cpp
    game/g_objectives.cpp
    game/g_ref.cpp
    game/g_roff.cpp
    game/g_savegame.cpp
    game/g_session.cpp
    game/g_spawn.cpp
    game/g_svcmds.cpp
    game/g_target.cpp
    game/g_trigger.cpp
    game/g_turret.cpp
    game/g_usable.cpp
    game/g_utils.cpp
    game/g_weapon.cpp
    game/g_weaponLoad.cpp
    game/genericparser2.cpp
    game/wp_atst.cpp
    game/wp_blaster_rifle.cpp
    game/wp_bot_laser.cpp
    game/wp_bowcaster.cpp
    game/wp_bryar_pistol.cpp
    game/wp_demp2.cpp
    game/wp_det_pack.cpp
    game/wp_disruptor.cpp
    game/wp_emplaced_gun.cpp
    game/wp_flechette.cpp
    game/wp_melee.cpp
    game/wp_repeater.cpp
    game/wp_rocket_launcher.cpp
    game/wp_saber.cpp
    game/wp_stun_baton.cpp
    game/wp_thermal.cpp
    game/wp_trip_mine.cpp
    game/Q3_Interface.h
    game/Q3_Registers.h
    game/ai.h
    game/anims.h
    game/b_local.h
    game/b_public.h
    game/bg_local.h
    game/bg_public.h
    game/bset.h
    game/bstate.h
    game/channels.h
    game/dmstates.h
    game/events.h
    game/fields.h
    game/g_functions.h
    game/g_headers.h
    game/g_icarus.h
    game/g_items.h
    game/g_local.h
    game/g_nav.h
    game/g_navigator.h
    game/g_public.h
    game/g_roff.h
    game/g_shared.h
    game/genericparser2.h
    game/npc_headers.h
    game/objectives.h
    game/say.h
    game/statindex.h
    game/surfaceflags.h
    game/teams.h
    game/weapons.h
    game/w_local.h
    game/wp_saber.h

    cgame/FX_ATSTMain.cpp
    cgame/FX_Blaster.cpp
    cgame/FX_Bowcaster.cpp
    cgame/FX_BryarPistol.cpp
    cgame/FX_DEMP2.cpp
    cgame/FX_Disruptor.cpp
    cgame/FX_Emplaced.cpp
    cgame/FX_Flechette.cpp
    cgame/FX_HeavyRepeater.cpp
    cgame/FX_RocketLauncher.cpp
    cgame/FxPrimitives.cpp
    cgame/FxPrimitives.h
    cgame/FxScheduler.cpp
    cgame/FxScheduler.h
    cgame/FxSystem.cpp
    cgame/FxSystem.h
    cgame/FxTemplate.cpp
    cgame/FxUtil.cpp
    cgame/FxUtil.h
    cgame/cg_camera.cpp
    cgame/cg_camera.h
    cgame/cg_consolecmds.cpp
    cgame/cg_credits.cpp
    cgame/cg_draw.cpp
    cgame/cg_drawtools.cpp
    cgame/cg_effects.cpp
    cgame/cg_ents.cpp
    cgame/cg_event.cpp
    cgame/cg_info.cpp
    cgame/cg_lights.cpp
    cgame/cg_local.h
    cgame/cg_localents.cpp
    cgame/cg_main.cpp
    cgame/cg_marks.cpp
    cgame/cg_media.h
    cgame/cg_players.cpp
    cgame/cg_playerstate.cpp
    cgame/cg_predict.cpp
    cgame/cg_public.h
    cgame/cg_scoreboard.cpp
    cgame/cg_servercmds.cpp
    cgame/cg_snapshot.cpp
    cgame/cg_syscalls.cpp
    cgame/cg_text.cpp
    cgame/cg_view.cpp
    cgame/cg_weapons.cpp
    cgame/strip_objectives.h

    ui/gameinfo.cpp
    ui/gameinfo.h

    qcommon/ojk_i_saved_game.h
    qcommon/ojk_saved_game_class_archivers.h
    qcommon/ojk_saved_game_helper.h
    qcommon/ojk_saved_game_helper_fwd.h
    qcommon/ojk_scope_guard.h
    qcommon/q_shared.cpp
    qcommon/q_shared.h
    qcommon/safe/files.cpp
    qcommon/safe/files.h
    qcommon/safe/memory.h
    qcommon/strippublic.h
    qcommon/tri_coll_test.cpp

    icarus/BlockStream.cpp
    icarus/Sequence.cpp
    icarus/Sequencer.cpp
    icarus/TaskManager.cpp
    icarus/Instance.cpp
    icarus/blockstream.h
    icarus/icarus.h
    icarus/interface.h
    icarus/interpreter.h
    icarus/sequence.h
    icarus/sequencer.h
    icarus/taskmanager.h
    icarus/instance.h
    icarus/tokenizer.h

    Rufl/hstring.cpp
    Rufl/hstring.h

    ${SHARED_COMMON_FILES}
)

if(WIN32)
    target_sources(${GAME_NAME} "win32/game.rc")
endif(WIN32)

if(NOT MSVC)
    # remove "lib" prefix for .so/.dylib files
    set_target_properties(${GAME_NAME} PROPERTIES PREFIX "")
endif()

if(WIN32)
    install(TARGETS ${GAME_NAME} RUNTIME DESTINATION "OpenJO")
elseif(APPLE)
    install(TARGETS ${GAME_NAME} LIBRARY DESTINATION "${ENGINE_NAME}.app/Contents/MacOS/OpenJK")
else()
    install(TARGETS ${GAME_NAME} LIBRARY DESTINATION "OpenJO")
endif()

set_target_properties(${GAME_NAME} PROPERTIES COMPILE_DEFINITIONS "${JK2SP_GAME_DEFINES}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${GAME_NAME} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})

target_include_directories(${GAME_NAME} PRIVATE
    ${SRC_DIR}
    ${GSL_DIR}
    ${CMAKE_BINARY_DIR}/src
)

set_target_properties(${GAME_NAME} PROPERTIES PROJECT_LABEL "JK2 SP Game Library")

if(WIN32)
    target_link_libraries(${GAME_NAME} winmm)
endif(WIN32)
