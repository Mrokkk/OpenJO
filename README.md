# OpenJO

OpenJO is my attempt to improve Jedi Outcast SP for Linux. I don't plan to support MacOSX and Windows, however original OpenJK support for those operating systems is left for now, so building should work. What would not work is creating PK3 with assets, which is done by `build.sh`.

## Changes

Apart from OpenJK changes described in the [CHANGELOG.md](https://github.com/Mrokkk/OpenJO/blob/master/CHANGELOG.md), it adds following:

### Shooting changes

* missiles are faster and have size so that it would be easier to hit an enemy (and so it's easier to hit player as well)
* better effects of weapons (mostly Bryar, E11, Bowcaster, Flechette)
* vanilla version can be enabled back by disabling "Improved Weapons" in setup/other menu or by `cg_improvedWeapons` cvar

### Other gameplay and visuals changes

* add more friction to player movement to decrease the sliding on the floor
* by default disable the ability to knock down Jedi by jumping on their head; this can be enabled back with the `g_jediKnockDown` cheat available in setup/other menu
* HUD and crosshair have correct aspect ratio
* lightsaber ignites faster
* configurable dynamic glow
* configurable option to draw health bars over enemy Jedi
* sabers clash is less dazzling
* bring back shining game title in menu
* use fonts from jk2mv

### Menu changes

* new game
  * add map selection
* controls/weapons
  * slightly decrease font size of "stun baton/lightsaber" controls option
* controls/mouse joystick
  * hide joystick settings if joystick is disabled
* setup/video
  * add support for multiple widescreen resolutions, including native resolution
  * add anti-aliasing selection
  * add texture compression on/off switch
  * add dynamic glow switch
  * video driver popup is now centered and uses smaller font
  * warning about video reset is also centered and better aligned
* setup/options
  * add crosshair selection with displayed image of crosshair
  * add dynamic crosshair switch
  * add show FPS switch
  * add draw health bars switch
  * move language-related settings to the setup/language
* setup/other - new submenu with following options
  * enable/disable developer mode (non-persistent)
  * enable/disable cheats (non-persistent)
  * show/skip opening logos

### UI scripting changes

* new keywords:
  * `globalDefinitions` - block with global constants; constants can be referenced by adding $ prefix
  * `assetGlobalDef.(in)activeButtonColor` - set color of active/inactive button
  * `assetGlobalDef.itemClickSound` - set default click sound for button
  * `menuDef.include` - include given file
  * `menuDef.layoutDef` - automatic horizontal/vertical layout group; layout group is defined with a layoutDef inside a menuDef; items are defined inside scope of layout; layouts can be stacked (e.g. horizontal one inside vertical one)
  * `menuDef.submenuGroupsDef` - define groups related to submenus
  * `scheme` - keyword which can be added directly after `itemDef` to mark given item definition as a base for other items
  * `itemDef.glow` - path to a shader used for glowing part of button shown when focused
  * `itemDef.clickSound` - override default click sound for given item
  * `itemDef.pos` - set only x and y part of rect
  * `itemDef.size` - set only w and h part of rect
* itemDef inheritance - itemDef can inherit from other itemDef by `itemDef : <base>`
* UI script commands:
  * `setActiveButton <group> <button>` - set `button` as active while other items from <group> are marked as inactive
  * `setFocus <non-existent item>` - clears focus
  * `showSubmenu <submenu>` - show given submenu; this works by hiding all other groups defined in submenuGroupsDef except for `submenu`

## License

OpenJO is licensed under GPLv2 as free software. You are free to use, modify and redistribute OpenJO following the terms in [LICENSE.txt](https://github.com/Mrokkk/OpenJO/blob/master/LICENSE.txt)

## Building

`./build.sh --install-dir <install dir with Jedi Outcast base> --build-type Release` run

This will build OpenJO in the `build` directory, install it to given dir and run it
