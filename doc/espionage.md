# Action Quake 2: Espionage
---

## Introduction
This is the main documentation for the Espionage game mode.  It will cover each mode, how to operate a server with this mode, and how to create your own scene files, or edit existing ones.

### Where did this come from?
A little history lesson: when the 1.52 code was leaked/released (depends on who you ask?), it was still very early in the lifecycle of the game.  The European scene decided to enhance this code into their own fork of the mod, which came to be known as [AQ2: The Next Generation (TNG)](https://aq2-tng.sourceforge.net/), and the North American scene did the same thing, this one by a team known as the [Action Quake Developers Team (AQDT)](https://assets.aq2world.com/archive/websites/aqdt.fear.net/).

Utilizing some ideas from [the Gangsters mod](https://assets.aq2world.com/archive/websites/www.planetquake.com/gangsters/) of Action Quake, as well as [Mr. Brian's CTB](https://assets.aq2world.com/archive/websites/gauntlet.telefragged.com/action/index.html), despite mentioning to the contrary that the mods were not related, as well as some ideas of their own, the AQDT created the Espionage mod.  On top of standard Deathmatch and Teamplay modes, Espionage mode was yet another way to experience Action Quake 2.  When players asked for a way to handle official matches, Tournament Edition was added, and Espionage Tournament Edition was what the modification was eventually named (**AQ2: ETE**).

### Great, but what _is_ Espionage?
Espionage incorporated several 'sub-modes' that were [teamplay-focused objective-based scenarios](https://assets.aq2world.com/archive/websites/aqdt.fear.net/aqdtdocs/esp.htm) and encouraged a bit of creativity from the author to create fun and engaging scenes to play in.  The original premise of the game was to act like the players were in a movie, so now they have a script!  [There were six new modes to play](https://wiki.aq2world.com/wiki/Game_Modes:Espionage):

1. Assassinate the Leader (ATL)
1. Escort the VIP (ETV)
1. Capture the Briefcase (CTB)
1. Capture and Hold (CNH)
1. One Flag Capture (OFC)
1. Find, Retrieve, Defend (FRD)

### How come it's taken so long to add these to TNG?
Sadly, the source code for ETE was never publicly released, as much as I pleaded and asked.  So there was only one true option left -- write it from scratch.  This is no small undertaking, but I think it's doable.

### Assassinate the Leader
The first sub-mode that we're looking at porting is Assassinate the Leader.  This is because the game flow is very straightforward and is almost exactly like normal teamplay as far as basic game logic is concerned, with a few major differences:

1. A player can `volunteer` to become a leader for their team.  There's only one leader per team.
1. The goal of each team is to frag the opposing team's leader, while protecting their own.
1. Configurable via server cvar, each leader can provided with `all weapons`, `all items`, or both.
1. Consistent respawning to keep everyone in the action.  Respawn timers are configurable.
1. In Matchmode, the Captain is also the Leader.

Teams only gain points and rounds end when a leader gets fragged.  Getting fragged in the defense of your leader is an honorable act and is awarded with extra points and recognition, as well is assaulting the opposing team's leader.

### Escort the VIP
The second sub-mode is Escort the VIP.  It's a mixture of Assassinate the Leader sprinkled in with some CTF, except there's no need to return the flag, merely touching the capture point is enough to win the round.
1. Team 1 is the only team that has a Leader.
2. Team 1's goal is to have their Leader touch the capture point (briefcase) on the map.  If this is achieved, they win the round.
3. Team 2's goal is to prevent Team 1's Leader from touching the capture point.  This can be achieved in two ways:
   1. Killing Team 1's Leader before he reaches the capture point
   2. Waiting for the `roundtimelimit` to expire (if set), causing a Team 2 win by default.  Suggested `roundtimelimit` for most maps is 2 minutes.
4. Halftime, if enabled and a `roundlimit` is set, will swap teams when half of the roundlimit is reached.  This is so that the experience is fair to both teams for the map.


### Sounds great, how do I enable Espionage mode on my servers?
There are some settings you need to setup to enable it:

* **esp** [0/1] - Activates Espionage mode.  Default 0.
* **esp_punish** [0/1/2] - Sets punishment mode for losing team when their leader dies.  This occurs instantly.  A value of 2 is only compatible with 2 Team Assassinate the Leader mode.  In 3 Team mode, only 0 and 1 are available.
    * 0 (default) is normal post-round teamplay with FF on.
    * 1 immediately kills all remaining members of the losing team.
    * 2 generates an invunerability shield on the remaining winning team members for the duration of post-round celebrations, so they can deal with the remaining losing team members in style
* **esp_showleader** [0/1] - GHUD setting, enabling this will show a marker over your team's leader at all times so that you can find him
    * 0 disables the indicator hovering over your leader
    * 1 enables the indicator hovering over your leader, making him easy to keep track of
* **esp_leaderequip** [0/1/2/3] - This determines the loadout of equipment for a team's leader
    * 0 means that the leaders do not get any special loadout, one item and one weapon of their choosing
    * 1 (default) means that the leaders can choose their weapon, and have all items available at once when they spawn
    * 2 means that the leaders can choose their item, and have all weapons available at once when they spawn
    * 3 means that the leaders have all items and all weapons when they spawn
* **esp_leaderenhance** [0/1] - This enhances the leader in that:
    * The leader's bandage time is halved
    * The leader receives periodic medkits, if medkit_max is > 0, to be used when they bandage
* **esp_enhancedslippers** [0/1/2] - This enhances the Stealth Slippers:
    * 0 means the Stealth Slippers are not enhanced in any way (classic)
    * 1 removes limping from all leg damage and 50% damage reduction when falling long distances
    * 2 removes limping from all leg damage and increase the height before fall damage occurs by 25%. 
    This used to be called `e_enhancedSlippers` from the original ETE, but was renamed to keep in with the naming convention of the new Espionage options.  *This option does not require Espionage to be enabled, it can be used in any game mode.*
* **esp_atl** [0/1] - default `0`, this forces ATL mode if `1`, even if the scenario loaded for the map is for ETV mode.  Has no effect on any map that is specifically setup for ATL mode or maps that load safe defaults, which are always ATL mode.

* **ETV-Specific Cvars**:
  * **esp_etv_halftime** [0/1] - Sets halftime rule of swapping teams in an ETV scenario after half of the roundlimit has passed.  A `roundlimit` of `0` automatically disables halftime.
  * **esp_showtarget** [0/1] - GHUD setting, enabling this will show a marker over your escort target, where the leader needs to reach to win the round
    * 0 disables the indicator pointing at your escort target location
    * 1 enables the indicator to assist your team in escorting your leader

* **esp_matchmode** [0/1] - toggles setting hard-coded defaults if matchmode is enabled, as posted below:

```
* Normal Slippers
* No team punishment
* Leader gets 1 weapon and all items
* Leader is enhanced and receives a maximum of 1 healthkit worth 25 health
```


Generally, these settings are great for all public servers:
```
esp 1
esp_punish 1
esp_showleader 1
esp_leaderequip 1
esp_leaderenhance 1
esp_enhancedslippers 1
```
Some may not like the punishment system, as it does not give any time for the team that lost their leader a few extra seconds to extract vengeance, so test this setting with your players.

### I would like to create my own scenes!
Great!  It's relatively easy.  There are configuration files that live inside of the `action/tng` directory with the extension of `.esp`.  These are Espionage config files.  I will go over the example of `urban4.esp`:
```
[esp]
type=atl
author=darksaint
name=Urban Gang Wars

[respawn]
red=10
blue=10
green=10

[spawns]
red=<274 1165 476 -148>,<553 -293 277 90>,<-783 -64 444 -60>,<342 632 319 180>
blue=<-1404 -13 59 90>,<-1548 1077 61 -90>,<-1387 1402 179 0>,<-1601 206 452 0>
green=<-635 592 65 45>,<-900 573 433 0>,<-660 -112 46 0>,<-904 826 473 -45>

[red_team]
name=The B-Team
leader_name=B.A. Barracuda
skin=male/ctf_r
leader_skin=male/babarracuda

[blue_team]
name=Peacekeepers
leader_name=Frank The Cop
skin=male/ctf_b
leader_skin=male/blues

[green_team]
name=Landscaping Crew
leader_name=The Incredible Chulk
skin=male/ctf_g
leader_skin=male/hulk2
```

* Under the `[esp]` header is information about the scene; the type (`atl`), the author and the name of the scene.
* Under the `[respawn]` header is how many seconds each team respawns.  Generally these should all be the same for fairness, but in cases where your `[spawns]` may heavily favor one team over the other on purpose (such as creating a scene for defending territory), you have the freedom to play with these values.
* Under the `[spawns]` header are the locations of the custom spawns that this scene uses.  You can defer to use the map's spawn points, in which case you simply remove this header and its contents entirely.
    * To generate spawn point locations, load the map locally and move your player to the location you want a spawn point to be.  In the console, type `viewpos`.  This will give you the vector locations (x y z) as well as the direction you are facing.
    * Reformat that into comma-delimited `<x y z w>` where `w` is the direction you are facing
    * Each team can have up to 4 custom spawn points
* Under each team header, you need to specify the following critiera, in no specific order:
    * `name` - Name of the team
    * `skin` - The team's skin
    * `leader_name` - The name of that team's leader
    * `leader_skin` - The skin that the leader will use
    * The keys and values here are case-sensitive, but the `name` and `leader_name` can have uppercase, spaces and _some_ special characters.
* Tips:
  * When creating spawn points, the `z` value should be at least 10 units higher than what viewpos returns, to avoid potentially getting stuck in the floor.  This means if your `viewpos` value is `24` then it should be `34` in the file.  For negative values (`-24`), you must add 10, so it's not `-34`, it's `-14`.
  * Double check your scenario file before committing it in case your spawn points don't work (stuck in a tree/wall, accidentially set a negative value instead of a positive, etc)
  * Reference skins that are in the AQtion distrib, so players don't see the default `male/resdog` skin if they don't have it.  If you want to suggest a new skin to use in your killer scenario, contact the AQ2World team in Discord.
  * Err on the side of shorter respawn times rather than longer ones -- it's more fun to play than to watch.  Some playtesting to get the perfect timings down may be needed, especially on maps where it's easy to defend a capture point, perhaps Team 2's respawn timer is a tad longer than Team 1's, for example.
  * When creating an ETV target, ensure that not only is it in a good spot, but keep in mind that the indicator arrow is 75 quake units floating above it -- this may clip into low ceilings and not look good.  The value of 75 units is so that the arrow doesn't interfere with visibility from the player's view.
  * Fill in details for the third team, even if you only envision this scenario being played out with 2 teams.  In the event that a server operator wants to run 3 team ATL mode 24/7, this will use your config rather than falling back on defaults.

Save your map-specific file with the name of the map to `action/tng/mapname.esp`.  If you formatted it correctly, it should load those values in the next time the map is loaded.

If something is malformatted or wrong, the game will do its best to handle it gracefully.  If it is having a problem finding values or other things are wrong with the file, it should ignore the file entirely and load `default.esp` values instead.  If this file is missing or also malformatted, there are some safe defaults that the gamelib has hard-coded that will work for any map, as it does not use custom spawn points.

---
### Some little-known features in Espionage mode
* In ETE mode, it would calculate pings and average across both teams.  This calculcation would be used to determine if the words "FAIR" or "NOT FAIR" would appear on the scoreboard.
* You could ignore a player, which is something we only recently added to TNG thanks to ReKTeK!
* Enhanced stealth slippers that stopped limping and increased the height that fall damage would occur.  These are now also in TNG but slightly modified in that you take 50% damage from falling rather than the increased height.  Imagine all of the cool jumps you can do now with little damage!
