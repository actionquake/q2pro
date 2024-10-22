# Action Quake 2: The Next Generation
**Version 2.83 (Nov 14th, 2023)**  
[http://www.aq2tng.barrysworld.net](http://www.aq2tng.barrysworld.net)  
by Igor, JBravo, Slicer, Deathwatch, Freud, Elviz, Rektek  
Additions and enhancements by darksaint, Reki, Rektek and the AQ2World team


# Table of Contents
- [Action Quake 2: The Next Generation](#action-quake-2-the-next-generation)
- [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Installation](#installation)
  - [Features](#features)
    - [Mapvoting](#mapvoting)
      - [Commands](#commands)
    - [Kickvoting](#kickvoting)
      - [Commands](#commands-1)
    - [Configvoting](#configvoting)
      - [Commands](#commands-2)
    - [Teamplay](#teamplay)
      - [Commands](#commands-3)
    - [Tourney](#tourney)
      - [Commands](#commands-4)
    - [3 Teams Teamplay](#3-teams-teamplay)
      - [Commands](#commands-5)
    - [Capture the Flag](#capture-the-flag)
      - [Commands](#commands-6)
    - [Domination](#domination)
      - [Commands](#commands-7)
    - [Matchmode](#matchmode)
      - [Commands](#commands-8)
    - [Voice Command](#voice-command)
      - [Commands](#commands-9)
    - [Low Lag Sounds](#low-lag-sounds)
      - [Commands](#commands-10)
    - [Announcer](#announcer)
      - [Commands](#commands-11)
    - [Kevlar Helmet](#kevlar-helmet)
    - [Single Barreled Handcannon](#single-barreled-handcannon)
      - [Commands](#commands-12)
    - [Enemy Down Radio Reporting](#enemy-down-radio-reporting)
    - [Player Ignoring](#player-ignoring)
      - [Commands](#commands-13)
    - [Video Setting Checking](#video-setting-checking)
      - [Commands](#commands-14)
    - [Location Files](#location-files)
    - [Punching](#punching)
      - [Commands](#commands-15)
    - [Sniper Zooming](#sniper-zooming)
      - [Cvars](#cvars)
      - [Commands](#commands-16)
    - [New Say Variables](#new-say-variables)
    - [Time and Roundtimeleft](#time-and-roundtimeleft)
      - [Commands](#commands-17)
    - [sv stuffcmd](#sv-stuffcmd)
      - [Commands](#commands-18)
    - [Grenade Strength](#grenade-strength)
      - [Commands](#commands-19)
    - [Total Kills](#total-kills)
    - [Random Rotation](#random-rotation)
      - [Commands](#commands-20)
    - [Vote Rotation](#vote-rotation)
      - [Commands](#commands-21)
    - [MapVote Next](#mapvote-next)
      - [Commands](#commands-22)
    - [Empty Rotate](#empty-rotate)
      - [Commands](#commands-23)
    - [Bandage Text](#bandage-text)
    - [Deathmatch Weapon](#deathmatch-weapon)
      - [Commands](#commands-24)
    - [Control Characters](#control-characters)
      - [Commands](#commands-25)
    - [Anti Camping](#anti-camping)
      - [Commands](#commands-26)
    - [Anti Idle](#anti-idle)
      - [Commands](#commands-27)
    - [Gibs](#gibs)
      - [Commands](#commands-28)
    - [Automatic Reloading of Pistol](#automatic-reloading-of-pistol)
    - [Weapon Banning](#weapon-banning)
    - [Item Banning](#item-banning)
    - [Weapon/Item Counts](#weaponitem-counts)
    - [Teamkilling after a Round](#teamkilling-after-a-round)
    - [New IR Vision](#new-ir-vision)
    - [Radio and Voice Flood Protection](#radio-and-voice-flood-protection)
    - [Darkmatch](#darkmatch)
    - [Map Restarting](#map-restarting)
    - [Statistics](#statistics)
    - [Automatic Joining/Equipping/Menu](#automatic-joiningequippingmenu)
    - [Automatic Demo Recording](#automatic-demo-recording)
    - [Spawn Code](#spawn-code)
    - [Ghost](#ghost)
    - [Bandolier behavior](#bandolier-behavior)
    - [Bots](#bots)
      - [Legacy LTK Bots](#legacy-ltk-bots)
      - [BOTLIB (Rektek) Bots](#botlib-rektek-bots)
    - [Slap](#slap)
    - [Variable Framerate Support](#variable-framerate-support)
    - [Q2pro MVD server demo support](#q2pro-mvd-server-demo-support)
    - [Latency Compensation](#latency-compensation)
    - [General quality of life improvements](#general-quality-of-life-improvements)
      - [Client Prediction](#client-prediction)
      - [Force Spawn items](#force-spawn-items)
      - [Warmup](#warmup)
      - [Item Kit Mode](#item-kit-mode)
      - [Print rules](#print-rules)
    - [Espionage](#espionage)
    - [Gun mechanics/enhancements](#gun-mechanicsenhancements)
    - [Highscores](#highscores)
    - [Outbound messaging](#outbound-messaging)
  - [Contact Information](#contact-information)
  - [Credits](#credits)
---

## Introduction

The Next Generation started as three separate projects: Action Quake 2: Edition, maintained by PG Bund and Igor, Action Quake 2: JBravo Edition, maintained by JBravo and Action Quake 2: Millennium, maintained by Slicer, Mort and Deathwatch.  It has continued as a project under the AQ2World team, with contributions from many others, in the form of **AQtion** (packaged q2pro + aq2-tng) available on Steam now for free.

---

## Installation

Unpack the quake2/action directory and edit the ini files in the action/config directory to suit your needs. Edit the server.cfg to whatever settings you want to use. Make sure you set your inifile variable to point to the correct action.ini in action/config.

---

## Features

### Mapvoting

TNG offers mapvoting so that clients can change the map by voting for one. This is useful for those servers that wish to offer the maps the people want to play on. There is a 40 second voting block to prevent players with fast connections to join and vote for a map before others got a chance to join/vote.

#### Commands
- Server settings:
  - <a name="use_mapvote"></a>`use_mapvote [0/1]` - toggle to turn mapvoting on (1) or off (0).
  - <a name="mapvote_pass"></a>`mapvote_pass [0..100]` - Percentage of votes a single map needs before the vote passes.
  - <a name="mapvote_need"></a>`mapvote_need [0..100]` - Percentage of players that has to vote before any vote may pass.
  - <a name="mapvote_min"></a>`mapvote_min [#]` - Minimum number of players that need to vote for a single map. 0 to ignore that.
  - <a name="mapvote_waittime"></a>`mapvote_waittime [#]` - Number of seconds people can't vote for a new map when they enter a map.
  - <a name="maplistname"></a>`maplistname "maplist.ini"` - filename/location of the file containing all the maps people can vote for.
  - <a name="mv_public"></a>`mv_public [0/1]` - When on (1), it will show what maps someone votes for.

### Kickvoting
This offers clients to kick those unwanted players from the server when no admin is about.  

#### Commands
- Server settings:
  - `use_kickvote [0/1]` - toggle to turn kickvoting on (1) or off (0).
  - `kickvote_pass [0..100]` - Percentage of votes a single player needs before the vote passes.
  - `kickvote_need [0..100]` - Percentage of players that has to vote before any vote may pass.
  - `kickvote_min [#]` - Minimum number of players that need to vote for a single player. 0 to ignore that.
  - `kickvote_tempban [0/1]` - When set to 1, it will ban the kicked person until the map changes
  - `vk_public [0/1]` - When on (1), it will show who votes for who.
- Client settings:
  - `kicklist` - this will display a list of all players with their id number
  - `votekick "player"` - this will cast your votekick vote for player "player"
  - `votekicknum [#]` - this will cast your votekick vote for the player with id number #


### Configvoting
Configvoting allows clients to vote for a certain configuration, predefined by the server admin, to be put on the server, in the same way as mapvoting. To vote for a config, open up the menu ('menu' in the console) and select Configvoting from the list.

#### Commands
- Server settings:
  - `use_cvote [0/1]` - toggle to turn configvoting on (1) or off (0).
  - `cvote_pass [0..100]` - Percentage of votes a single config needs before the vote passes.
  - `cvote_need [0..100]` - Percentage of players that has to vote before any vote may pass.
  - `cvote_min [#]` - Minimum number of players that need to vote for a single config. 0 to ignore that.
  - `configlistname "configlist.ini"` - filename/location of the file containing all the configs people can vote for.

### Teamplay
Enables standard 2-team round-based teamplay.  Kill everyone on the other team to win a round.

#### Commands
- Server settings:
  - `teamplay [0/1]` (default: "0") - setting to 1 enables teamplay, requires new map or server restart
  - `sv t[i]name <string>` - Where i is 1, 2 or 3, sets a team name in the absence of action.ini.  Will take effect immediately.  Example: sv t1name "Robbers"
  - `sv t[i]skin <string>` - Where i is 1, 2 or 3, sets a team skin in the absence of action.ini.  Will take effect on a new round of teamplay or a new map.  Example: sv t2skin "male/ctf_b"
  - `sv t[i]skin_index <string>` - Where i is 1, 2 or 3, sets a team skin index in the absence of action.ini.  Will take effect on a new map or server restart.  Example: sv t3skin_index "siris_i"

### Tourney
For those who want to offer a Rocket Arena style one on one server, this mode is ideal. It will let two players spawn and face each other. The winner will stay for the next round and the loser will go back to the queue. The winner then takes up the next player in the queue.

Example: There are 4 players on a server, A, B, C and Z. Both A and B play each other, C and Z are in the queue. B wins and A gets put back in the queue (which then is C, Z and A). Then, B plays the next in the queue, namely C. If B would win, he'd face Z and C would get put back in the queue. If C would win, he would play Z next and B would be put back in the queue.

#### Commands
- Server settings:
  - `use_tourney [0/1]` - This will turn tourney mode on (1) or off (0). To use it, teamplay needs to be on. (1)
  - `tourney_lca [0/1]` - Set to 1 to enable "Lights Camera Action" for tourney mode, set to 0 for classic silence

### 3 Teams Teamplay
For more exciting teamplay games, TNG offers a 3 Teams mode of play. This variant of Teamplay will put three teams against each other.

#### Commands
- Server settings:
  - `use_3teams [0/1]` - This will turn 3 Teams on (1) or off (0). To use it, teamplay needs to be on. (1)

### Capture the Flag
The popular gameplay mode from many games and mods is available in TNG as well. In this mode, there are two teams, Red and Blue. Both have a flag on the map, and they need to take the enemy flag and bring it to their own to score a point. (Capture the Flag, or cap for short) The other team will attempt the same of course.

To prevent spawn camping, there is an invulnerability/shield mode to give some people some protection.

Flag locations and CTF player spawns should be specified in tng/mapname.ctf files.

#### Commands
- Server settings:
  - `ctf [0/1]` - This will turn CTF on (1) or off (0). It will automatically turn Teamplay on (1).
  - `capturelimit [#]` - The maximum number of captures before a map will change. Set to 0 to ignore that.
  - `ctf_respawn [#]` - The time in seconds before a player will respawn after having died. Overrides respawn timers in .ctf files
  - `ctf_dropflag [0/1]` - Allow clients to drop the flag or not.
  - `uvtime [#]` - The number of seconds *10 of the duration of the 'shield' effect. (for example 40 is 4 secs)
  - `ctf_dyn_respawn [0/1]` - Default 0, this will reduce the respawn timer for a losing team periodically.  Will self-correct as that team mounts a comeback.
- Client settings:
  - `drop flag` - Drop the flag if you're holding it

### Domination
This is most closely related to Unreal Tournament's domination mode, but is also similar to king of the hill and conquest modes in general. There are fixed flags (typically 3) scattered around the map, and touching a flag will claim it for your team. Keep control of flags to score points; your team gains 1 point per flag controlled every second. The score limit is 200 per flag in 2-team mode, or 150 per flag in 3-team mode, which means a typical 2-team game with 3 flags has a score limit of 600.

Flag locations can be specified in tng/mapname.dom files. If not found, the server will attempt to generate them automatically from player spawn locations.

#### Commands
- Server settings:
  - `dom [0/1]` - This will turn Domination mode on (1) or off (0), and also turns Teamplay on (1).
  - `use_3teams [0/1]` - This will turn 3 Teams on (1) or off (0).
  - `uvtime [#]` - The number of seconds * 10 of the duration of the 'shield' effect (ex: 40 = 4 secs).

### Matchmode
Matchmode is a special form of Teamplay, made for clanmatches. It has several features which can be useful during matches and league games. It makes sure the whole timelimit is played.

Clients will have a few more things to do during matchmode: they have to have a captain and substitutes. Everybody in a team will only be able to talk to his/her own team, with the exception of the captain (see below). Subs are players who are part of the team but aren't playing at the moment. With limchasecam enabled, they will be limited to viewing their own team.

#### Commands
- Server settings:
  - `matchmode [0/1]` - Turns Matchmode on (1) or off (0). Turning Matchmode on will turn Teamplay on too.
  - `mm_forceteamtalk [0/1/2]` - this will change the way people can talk on the server. When set to 0, it will be the same as with normal teamplay. When set to 1, the teams will not be able to talk with each other, except for the admin and the captains. When set to 2, teams can talk with each other until the match starts and when the match is paused or over.
  - `mm_adminpwd <pass>` - this will set the password for match admins.
  - `mm_allowlock [0/1]` - when on (1), this will allow captains to lock/unlock their teams.
  - `mm_captain_teamname - [0/1]` - default 0, if enabled (1) then the team name will change to the name of the captain, prepended with the string "Team", for example, "Team Suislide" if Suislide is the captain of that team.
- Client settings:
  - `captain` - Become the team's captain if not captain exists yet. A captain can ready/unready his team and speak to the other team
  - `sub` - this will make you a sub for the team or remove you from the subs and back in the team
  - `ready` - this will ready/unready the team. A new round won't start if a team isn't read
  - `teamname "name"` - allows the captain to set the name of his/her team
  - `teamskin "male/resdog"` - allows the captain to set the name of his/her team 
  - `matchadmin <pass>` - this will allow a player to get admin status
  - `lock` - allows a captain to lock his team. When a team is locked, no one can join it. Locks are removed on a new map
  - `unlock` - allows a captain to unlock his team

### Voice Command
The voice command allows clients to play taunts for other players to hear. (as long as they have the sound file)

#### Commands
- Server settings:
  - `use_voice [0/1]` - When on (1), it will allow the use of voice commands.
- Client settings:
  - `voice "sound.wav"` - this will play sound.wav for all players to hear. (as long as the others have sound.wav) This command requires the .wav extension. (client side)

### Low Lag Sounds
To reduce the number of packets being sent by AQ2 during big firefights, TNG has included the option to turn Low Lag Sounds on. This will use a different set of sounds for each weapon, ones that reduce the number of packets being sent.

In AQtion, there are already gun sounds installed that you can select from.  If using non-AQtion clients, when you use this, the sounds of the weapons will change to those of Quake 2 weapons. You can update your sounds with the following:

- MK23 Pistol: `sound/weapons/mk23fire.wav`
- Handcannon: `sound/weapons/cannon_fire.wav`
- MP5 Submachinegun: `sound/weapons/mp5fire.wav`
- SSG-3000 Sniper Rifle: `sound/weapons/ssgfire.wav`
- M4 Assault Rifle: `sound/weapons/m4a1fire.wav`
- M3 Super Shotgun: `sound/weapons/shotgf1b.wav`

#### Commands
- Server setting:
  - `llsound [0/1]`
- Client settings:
  - `cl_mk23_sound`
  - `cl_mp5_sound`
  - `cl_m4_sound`
  - `cl_m3_sound`
  - `cl_hc_sound`
  - `cl_ssg_sound`

### Announcer
The announcer that says "Lights, Camera, Action!" has his vocabulary increased by several words, announcing when people perform several feats, such as high accuracy ("Accuracy") or make an impressive shot ("Impressive") but also when a team wins ("Team X Wins") or when the fraglimit is about to get hit.

#### Commands
- Server settings:
  - `use_warnings [0/1]` - When on (1), it will play all warnings such as "3 minutes left" and "Team X Wins".
  - `use_rewards [0/1]` - When on (1), it will play the sounds on rewards such as "Impressive".
  - `use_killcounts [0/1]` - When on (1), server will announce kill counts.

### Kevlar Helmet
A new item, the Kevlar Helmet acts like the Kevlar Vest does, only it protects the head of the player wearing it. To disable it, use item banning.

### Single Barreled Handcannon
We have added the feature to let clients fire one barrel of the Handcannon at the same time. This has as benefit that the Handcannon will be able to fire a second shot fast after the next one, but as drawback is is that it is not as strong as a double barreled shot, or shotgun shot for that matter.

#### Commands
- `hc_single [0/1]` - Set to 1 to enable single barreled handcannon. Default is 1.

### Enemy Down Radio Reporting
Upgraded the 'enemy down' radio sound to report the number of enemies that have been killed as well.

### Player Ignoring
Player Ignoring can be done through the menu. When ignoring a player, you won't hear any messages from him.

#### Commands
- `menu` - this will open the TNG menu. (client side)
- `ignore "player"` - this will ignore the player with the name "player".
- `ignorelist` - this will display a list of all players on the server with their id.
- `ignorenum [#]` - ignorenum will ignore the player with the id given.
- `ignoreclear` - this will clear your ignore list.

### Video Setting Checking
TNG offers a way to check for gl_lockpvs and to cap the gl_modulate settings.

#### Commands
- `video_check [0/1]` - turn checking for gl_modulate capping on (1) or off (0)
- `video_check_lockpvs [0/1]` -  turn checking for gl_lockpvs on (1) or off (0)
- `video_check_glclear [0/1]` -  turn checking for gl_clear on (1) or off (0)
- `video_checktime [#]` - sets interval between checks in seconds
- `video_max_3dfx [#]` - Maximum value for gl_modulate for 3dfx users.
- `video_max_3dfxam [#]` - Maximum value for gl_modulate for 3dfx users with the AMD patch.
- `video_max_opengl [#]` - Maximum value for gl_modulate for OpenGL users.
- `video_force_restart [0/1]` - If on (1), the server will force a client to do a video_restart when he connects.

### Location Files
To support the %L and %S commands, the server needs to install the location files. Location files are files which contain coordinates for locations on the map and names for them. The location files itself have the extension of .aqg and should be put in the action/tng directory on the server.

### Punching
We have added the extra 'punch' attack, which basically is a weaker version of a kick. It is very close range and does up to 20 damage and can punch the weapon of a player out of his hands. When bandaging, shooting, reloading or zoomed with a sniper rifle, you cannot punch.

#### Commands
- Server setting:
    - `use_punch [0/1]` - when on (1), it will allow clients to use the punch command
- Client setting:
    - `punch` - this will preform a punch

### Sniper Zooming
The `lens` command is in TNG and offers increased control over the zooming of the sniper rifle. Zoom sensitivity can be adjusted based on the zoom level with these client cvars.  Any `cl_zoom_xx` value set to `0` will not change the sensitivity for that zoom level.  For example, if you have 2x set to `4` and 4x set to `0`, 4x will also be `4` since it is not changing from 2x's setting.  Zooming all the way back out will restore your original sensitivity setting.

#### Cvars
- Server cvars:
  - `zoom_comp [0/1]` - server cvar, set to "1" to compensate zoom delay based on ping. Every 80ms ping reduces 1 frame, minimum of 1 frame. Default is "0", do not compensate
- Client cvars:
  - `cl_zoom_autosens` - default 0, if 1, it enables the use of the cvars below
  - `cl_zoom_2x` - default 0, set this to the sensitivity level you want to use at a 2x zoom
  - `cl_zoom_4x` - default 0, set this to the sensitivity level you want to use at a 4x zoom
  - `cl_zoom_6x` - default 0, set this to the sensitivity level you want to use at a 6x zoom

#### Commands
- Client commands:
  - `lens` - changes zoom on sniper rifle
  - `lens [#]` - 1,2,4,6 - if you use any other number it'll go to the nearest zoom mode
  - `lens in` - zooms the rifle in to the next zooming level
  - `lens out` - zooms the rifle out to the previous zooming level

### New Say Variables
TNG offers several new variables clients can use in their text (Client settings):
- `%K` - shows the nickname of the person you killed last. 
- `%P` - shows the nickname of the person you damaged last.
- `%D` - shows the location where you hit your enemy last. This can either be head, chest, stomach, legs, kevlar vest or kevlar helmet.
- `%L` - shows the location where you are at that moment. (if the map has a location file)
- `%S` - shows the location where you are looking at. (if the map has a location file)
- `%E` - shows the enemy you are looking at.
- `%F` - shows the weapon the enemy is holding.
- `%me` - shows your nickname like IRC's /me.

### Time and Roundtimeleft
By typing 'time' in the console, a client can see the remaining time on that map. This doesn't work in matchmode.
By typing 'roundtimeleft' in the console during a round, it will give a general indication of the amount of time left that round. This only works when there is a roundtimelimit.

#### Commands
- Client settings:
    - `time` - display the remaining time
    - `roundtimeleft` - display the remaining roundtime

### sv stuffcmd
This feature will allow people who got rcon to send commands to the clients.

#### Commands
- `sv stuffcmd <player #/team1/team2/all> <client command>` - If you use $<variable name>, it will display your own setting for that variable. If you use !<variable name> it will display the client's variable name. 

Example:
- `sv stuffcmd all say !gl_modulate` - This will make all clients say their gl_modulate setting.

### Grenade Strength
Grenades are a little bit more powerful, so they're not as useless as they were since AQ 1.52 (250 instead 170 - in AQ2 1.51 they were even stronger) - should stop people using grenade jumps in AQ2 (after all, it should be an ACTION mod - have you EVER seen someone doing grenade jumps in a real life action film? (exclude Matrix - it's a science fiction...))

#### Commands
- `use_classic [0/1]` - Turn on (1) to use the original 1.52 values.

### Total Kills
The scoreboard of TNG will now show the total kills for each player. Kills is the total number of kills without the negatives (suicides, cratering, teamkills) subtracted.

### Random Rotation
Random Map Rotation will make the server pick a random map from the maplist when the current map ends. This will make the rotations less static.

#### Commands
- `rrot [0/1]` - Set to 1 to turn random rotation on.

### Vote Rotation
Voterotation computes the all time votes for maps and changes the maplist as wanted. If a map is played the amount of players is subtracted from the all time votes value.

#### Commands
- `vrot [0/1]` - Turn on (1), to use voterotation.

### MapVote Next
Basically, this is a simplified and immediate form of Vote Rotation. When the current map ends, if there are any active map votes, the server moves to the most voted map instead of the next in rotation.

#### Commands
- `mapvote_next [0/1]` - Default on (1), set to 0 to disable this feature.
- `mapvote_next_limit [#]` - Default (0, disabled), how many seconds remaining in a map (via timelimit) to restrict the use of votemap, so last second voters can't change the next map. Enforced on mapvote_next=1.

### Empty Rotate
Prevent unpopular maps from keeping your server empty. This feature will rotate the map after the server has been empty for some number of minutes. You can also specify a config file to execute, which can be used to reset the server to default settings after configvote or rotate between multiple mode configs.

#### Commands
- Server settings:
    - `empty_rotate [#]` - Number of minutes before rotating empty server. Default 0 disables this feature.
    - `empty_exec "file.cfg"` - Optional config to exec with empty rotate. Default blank disables this.

### Bandage Text
When a client starts bandaging, a message ("You've started bandaging") will appear. Also the Health icon will appear on the bottom of the screen.

### Deathmatch Weapon
A new variable allows the server to give players a starting weapon during deathmatch. When set, the players will get the weapon of that type with full ammo when they spawn.

#### Commands
- Server settings:
    - `dmweapon "weaponname"` - Starting weapon for DM mode (dmweapon), default is "MK23 Pistol".
    - `dm_choose [0/1]` - allows players to choose their starting weapon in DM mode, 0 disables/1 enables
    - `dm_shield [#]` - Sets a time value for how long the invulnerability shield in DM mode. Value of 30 = 3 seconds, for example.

### Control Characters
Some proxies use control characters to hide the name of the user. However, it is used by several players as well to cause abuse and do spam messages.

#### Commands
- `sv_crlf [0/1]` - if off (0), clients are not allowed to issue CR/LF control characters in chat messages.

### Anti Camping
To discourage camping, TNG has a feature which will make a client who is standing still for a certain number of seconds to make noises.

#### Commands
- `ppl_idletime [#]` - Allowed idle time in seconds before a client will start to make sounds. 0 to turn it off.

### Anti Idle
To help with problem idlers, use this feature to remove them from their teams so that someone else can join in without needing to votekick. This affects teamplay modes only, as regular DM does not use teams. If you want to remove in DM mode, use sv_idlekick.

#### Commands
- `sv_idleremove [#]` - Allow idle time in seconds before a player gets removed from their team. Suggested this be a value over 60. Set to 0 to disable.

### Gibs
Gibs are a rewarding sight for successful headshots. TNG offers a variable which can turn gibs on headshots on. Also handcannon shots that do enough damage will gib someone.

#### Commands
- `sv_gib [0/1]` - when on (1), it will display gibs
- `sv_killgib [0/1]` - when on (1), upon using the `kill` command, the player will explode into giblets instead of just dying

### Automatic Reloading of Pistol
When someone has akimbo pistols and they are both empty, and got one clip left, to press reload and it will switch back to single pistol and reload that with the clip.

### Weapon Banning
To allow/disallow the usage of certain weapons, TNG supports weapon banning.

**Commands:**
`wp_flags [#]` - The number for this variable is the combination of the following:
- MK 23   	- 1
- MP5 	- 2
- M4	  	- 4
- Shotgun 	- 8
- Handcannon	- 16
- Sniper	- 32
- Akimbo	- 64
- Knife	- 128
- Grenade	- 256

For all weapons: 511 (default)

**Example:** You want to make a sniper rifle only server: you set wp_flags to 32, the only weapon you allow.
Or, if you only want pistols: 65 (1+64).

### Item Banning
To allow/disallow the usage of certain items, TNG supports item banning.

**Commands:**
`itm_flags [#]` - The number for this variable is the combination of the following:
- Silencer 	  - 1
- Slippers	  - 2
- Bandolier	  - 4
- Kevlar Vest	  - 8
- Lasersight	  - 16
- Kevlar Helmet - 32

For all weapons: 63 (default)

**Example:** You want to have slippers and silencers only on your server: you set itm_flags to 3 (1+2).

### Weapon/Item Counts
Normally, AQ2 allows one item and one weapon.  These settings can adjust that so that you spawn with more, or can hold more in general.
- `allweapon [0/1]` - default 0, if enabled, all players spawn with all weapons, with exception of any weapon bans
- `allitem [0/1]` - default 0, if enabled, all players spawn with all items, with exception of any item bans
- `allow_hoarding [#]` - default 0, if enabled, allows players to pick up more than 1 special item or weapon, but does not spawn with them, like allweapon or allitem will do


### Teamkilling after a Round
To add a little more fun in the game, we've added an option which will allow a team to TK each other after a round has ended during Teamplay. This is only available to servers where FF is off.

**Commands:**
`ff_afterround [0/1]` - Set to 1 to allow TKing (without punishments) after a round has ended. FF needs to be off.

### New IR Vision
An updated IR Vision is available. This does, besides adding the red glow to players, another visual effect.

**Commands:**
`new_irvision [0/1]` - Set to 1 to turn the new IR Vision on.

### Radio and Voice Flood Protection
To stop people spamming with the Radio and Voice command, there is flood protection for both in TNG.

**Commands:**
- `radio_repeat [#]` - Number of the same radio/voice calls before being stopped. 0 is off.
- `radio_ban [#]` - Number of seconds a player will be silenced for after spamming.
- `radio_time [#]` - Time, in seconds, where a client may have a radio_max number of radio/voice calls in.
- `radio_max [#]` - Number of radio/voice calls allowed with the radio_time period.

### Darkmatch
Ever wanted to play AQ2 in total darkness? TNG allows you to experience this by using darkmatch. Darkmatch has 3 modes. In it's first mode, the map will be pitch black, except for certain lights. This means that players have to be careful where they walk as they do not know when they will walk off a building. Also, firing your weapon will make you show up clearly.

In the second mode, the map is in its near black mode. The map will be very dark, and it will allow for more stealthy play.

The third mode will simulate a real day. The map will go from normal lighting to pitch black and then back again to normal lighting.

Of course, to assist players, there is a flashlight available in darkmatch only. When used, it will create a circle of light at wherever you point, but it will also make you show up.

**Commands:**
- `darkmatch [0/1/2/3]` - when set to 1 it's total darkness, when set to 2, its near darkness, when set to 3 it will simulate day/night.
- `day_cycle [#]` - Time in seconds where Darkmatch mode 3 will change its light settings
- `use_flashlight [0/1]` - allow flashlight use in non-darkmatch
- `flashlight` - this toggle will turn the flashlight on and off. (client side)

### Map Restarting
Sometimes you do want to restart a map, to reset the scores there. But a normal map command will disconnect everybody and will reset everything on the server. Or you want to load a map without disconnecting everybody.

For this, TNG has two new commands.

**Commands:**
- `sv softmap <mapname>` - this command will load 'mapname' as if it was next in the rotation.
- `sv map_restart` - this command will restart the map as if it was next in the rotation.

### Statistics
To see how well players are doing, we have implemented statistics into TNG. This feature will track various statistics of each player, for example accuracy or frags per minute.

**Commands:**
- `stats_endmap [0/1]` - when set to 1, this will display the stats scoreboard at the end of the map instead of the normal screen.
- `stats_afterround [0/1]` - when on (1), this will also record stats for events that happen when a round ends, for example when ff_afterround or FF is on.
- `stats` - this will display the individual stats of the player. (client side)
- `stats list` - this will display a list of all players and their ID. (client side)
- `stats [#]` - this will display the stats for the player with the id given. (client side)
- `stats_mode [0/1/2]` - when set to 1, it will automatically display the stats of the player at the end of each round. When set to 2, it will automatically display the stats of the player at the end of the map. By default this is set to 0 (off). (client side)

### Automatic Joining/Equipping/Menu
For the lazy players under us, we have created three new commands to make things easier.

**Commands:**
- `auto_join [0/1/2]` - when on (1), the players will automatically join the team he was in during the last map. If set to 2, it will automatically join the team with the least players, or randomly if equal.
- `auto_equip [0/1]` - when on (1), the players will automatically be given the equipment he had during the last map.
- `auto_menu [0/1]` - when on (1), the menu will automatically open when you join the server.

### Automatic Demo Recording
To ease the recording of demos by a client over several maps, we have implemented a feature which will let the client automatically record a demo on each new map. When enabled, the server will keep track of the recording of the client, and will create a demo of the map with the following name: date_time-map.dm2 In matchmode, the demo's name will be: date_time-team1_vs_team2-map.dm2

**Commands:**
`autorecord` - Starts automatic recording of demos.

### Spawn Code
TNG offers a new spawn code for Teamplay. This mode prevents teams to spawn twice on the same spot and will go through most spawns.

**Commands:**
`use_oldspawns [0/1]` - Defaults to 0 (off). When set to 0, it will use the new spawning method for teamplay.

### Ghost
With the use of the ghost command, the client can get back when he has been disconnected and get your frags and stats back. Ghost does require the client to have the same nick and ip when using the ghost command.

**Commands:**
- `use_ghosts [0/1]` - when set to on (1), the server allows the clients to use the ghost command.
- `ghost` - this will restore the scores, stats, teams and items of the client. (client side)

### Bandolier behavior
TNG updates the way the bandolier behaves when dropping it. It will prevent people from keeping the grenade(s) (when enabled) after having dropped the bandolier.

**Commands:**
`use_buggy_bandolier [0/1]` - if you wish to revert back to the old bandolier behavior, set this to 1

### Bots

#### Legacy LTK Bots
Now you can fill out your teams with bots, or create an entire team of bots to fight. You can define persistent bots in bots/botdata.cfg, or create and remove them on demand. Bots wander pretty stupidly unless you create a linked network of nodes for them to follow.  This documentation exists for reference only, LTK bots have been replaced by the BOTLIB Bots as described below, use at your own risk.

**Commands:**
- `sv addbot [team] [name]` - add a bot for the duration of the current map
- `sv removebot [name]` - remove a bot
- `ltk_skill [0-10]` - set bot skill (default 5)
- `ltk_chat [0/1]` - allow bots to chat
- `ltk_loadbots [0/1]` - 0 to stop bots from automatically joining the game, 1 for default behavior
- `ltk_botfile [filename]` - Set filename of the botfile you want to load.  Default is 'botdata' (this loads bots/botdata.cfg)
- `sv botdebug [on/off]` - toggle bot node editing and output for debugging
- `addnode [type]` - place a new node at your current position (it will tell you its number)
- `addlink [num1] [num2]` - add a link from node num1 to num2, and a reverse link if possible
- `removelink [num1] [num2]` - remove the link from node num1 to num2; does not remove reverse
- `movenode [num]` - move an existing node to your current position
- `nodetype [num] [type]` - show or change the type of a node
- `shownode [num]` - display a temporary marker of a node's location
- `sv initnodes` - clear all nodes
- `sv loadnodes` - load saved nodes from terrain/<mapname>.ltk
- `sv savenodes` - save all nodes and links for this map to terrain/<mapname>.ltk

#### BOTLIB (Rektek) Bots
Taking aspects of the existing bots and greatly enhancing their navigation and behavioral capabilities, the new botlib bots have superceded the legacy LTK bots.  This is a very new system and as such will be in a regular flux of change, enhancements and adjustments.

**Server Commands:**
- `bot_enable [0/1]` - Server cvar, if enabled, will load all necessary components to enable bots.  Default is disabled [0]
- `sv bots <#> [#]` - Server command, if command is issued without arguments, it will print out the existing bot counts, bots on teams and other information.  If provided with a single value (`sv bots 3`) this will add 3 bots to the game.  If this is a team game, it should auto balance the bots across teams.  If provided with two values (`sv bots 3 1`) this will assign 3 bots to team 1.  Please be aware of your maxclients limits as the server console will alert you if you have added too many bots.
- `bot_remember <#>` - Server cvar, how long (in seconds) the bot remembers an enemy after visibility has been lost.  This is experimental and being reevaluated for usability, and may be replaced in the future
- `bot_reaction <#>` - Server cvar, how long (in seconds) until the bot reacts to an enemy in sight.  Lower values mean a faster reaction.  This is experimental and being reevaluated for usability, and may be replaced in the future
- `bot_randvoice <#>` - Server cvar, percentage chance that bots use random user voice wavs [min: 0 max: 100].  Suggest disabling or setting to a very low value because user voice wavs take up limited sound slots on non-extended protocol servers (256 max sounds versus 2048) -- if a server runs out of sound slots and tries to cache another sound, it will crash.  There is a need to re-evaluate how to handle this on non-extended protocol servers.
- `bot_randname [0/1]` - Server cvar, allow bots to pick a random name.  Suggest keeping enabled
- `bot_chat [0/1]` - Server cvar, enables bot chat.
- `bot_countashuman [0/1]` - Server cvar, enabling this will allow teamplay-based games to progress without humans being in the server.  Set to 0 to force games to not count bots as clients in terms of teamplay
- `bot_navautogen [0/1]` - Server cvar, enabling this will auto generate a navmesh for any maps that do not already have one, on map load time.  This automatic navmesh is far from perfect, but it does allow bots to traverse maps rather than stand still.  A far superior option is to have a handcrafted navmesh for the map.
- `bot_debug [0/1]` - Server cvar, will enable debug messaging for BOTLIB functionality where enabled
- `bot_reportasclient [0/1]` - Server cvar, if enabled, will report bots as real clients to server masters.  Default is disabled [0]

**Client Commands:**
These commands only work on local map loads, not connections to dedicated servers.  They are meant to be used to create navmeshes.
- `nav_edit` - Enters nav edit mode.  Enter again to toggle out of nav edit mode.
- `nav_toggle` - Toggles nav visibility: None, nodes, and bot paths
- `nav_autogen` - If in nav_edit mode, will automatically generate a navmesh that will interconnect nodes that are reachable via info_player_deathmatch entities.  This performs 90% of the work for you, so that most flat surfaces will have a full mesh ready to use.
- `nav_save` - This saves the current navmesh to `bots/nav/mapname.nav`
- `nav_load` - This will load a navmesh from `bots/nav/mapname.nav`.  Loading a map that already has a navmesh will perform this automatically, there is no need to run this command except to possibly revert to an earlier navmesh if you make a mistake and want to undo
**Nav Interface**
When generating a navmesh, you will use the `reload` key to cycle through different options (add node, remove node, link node, etc.).  Use the `+attack` key to interact with the navmesh.  More documentation around performing this is coming soon.

### Slap
Server admins can slap players into the air, and optionally deal damage.

**Commands:**
`sv slap [name] [damage] [power]` - slap a player, with optional damage (default 0) and power (default 100)

### Variable Framerate Support
Although Quake 2 typically runs the server at a fixed 10 frames per second, some advanced Quake 2 servers can be built with variable framerate support to allow faster updating.  AQ2-TNG now supports this.  At higher fps, lag is reduced: the server responds more quickly to shooting and other client commands.

**Commands:**
- `sv_fps [10/20/30/40/50/60]` - set server framerate (default is 10) -- suggested values are [10/20/30]
- `sync_guns [0/1/2]` - 0 plays gun sounds on any frame, 1 syncs with recent shots, 2 delays all to 10fps (default 1)

### Q2pro MVD server demo support
When AQ2TNG is running under a q2pro Quake2 executable that is properly configured (built with CONFIG_MVD_SERVER set to 1 at compile time it can record every single match into a dedicated file.  For this to work the cvar sv_mvd_enable must also be set to 1.  The demos go into the action/demos folder on the server and contain all players points of view for the entire match.   To activate this simply set use_mvd2 1 in your server config.

### Latency Compensation
Antilag allows server operator to enable lag-compensation for aiming with hitscan weapons, useful for making high ping games more fair. Optionally, the server operator can enable interpolation along with antilag for aiming directly at player models to hit them. This has the side effect of being 'shot around corners', so the best use for this setting tends to be for matches where all players pings are very low.

**Commands:**
- `sv_antilag [0/1]` (default: "1") - Setting to "1" enables lag compensation functionality when firing hitscan weapons.
- `sv_antilag_interp [0/1]` (default: "0") - Setting to "1" enables interpolation for hitscan weapons. Requires sv_antilag "1".

### General quality of life improvements
`sv_limp_highping [#]` - server cvar, players above this ping threshold will have movement prediction disabled with leg damage to make things less jittery. Value is set in ping ms, players with a ping value equal or higher to this value will have less jittery movement. Default value is '70'

#### Client Prediction
`limp_nopred [0/1]` - client cvar, clients can set this to force on or off movement prediction when taking leg damage. Set to "0" for classic behavior, set to "1" to fix the jitter

#### Force Spawn items
`g_spawn_items [0/1]` - server cvar, set to "1" to allow for item/ammo/weapon spawns in games that you choose your starting weapon, for example, dm_choose, teamplay, etc. Forced set to "0" for matchmode. Set "0" for classic play.

#### Warmup
`warmup [#]` - server cvar, set in seconds, minimum is 15, 20 is common. This is the amount of time in seconds after both captains ready up that warmups will continue until the first round begins. Set to 0 to disable warmup entirely.
`warmup_bots [#]` - server cvar, set in number of bots to add. This adds this many bots to the warmup phase of matchmode. Set to '0' to disable warmup_bots, suggested value for usage is '4' to '6'.

#### Item Kit Mode
`item_kit_mode [0/1]` - server cvar, default 0. Works in any mode where you choose a weapon (GS_WEAPONCHOOSE), it combines items into kits:
- Commando Kit: `Bandolier + Kevlar Helm`
- Stealth Kit: `Slippers + Silencer`
- Assassin Kit: `Laser Sight + Silencer`
Players may drop the items anytime (using `drop item`) but you can only pick one up at a time, assuming the server doesn't allow for more. On player entity death, both items will drop. LTK bots do not use kits, they will retain their normal behavior. esp_enhancedslippers settings are honored, boosting the effectiveness of the Stealth Kit even more.

#### Print rules
`printrules [0/1]` - server cvar, default 0.  If enabled, a printout of the rules will display once the countdown begins in Teamplay modes.

### Espionage
Inspired by AQ2:ETE, these additions are optional server vars to create a different variety of gameplay. Review the document called Espionage.md to understand how to run a server and how to create and edit scene files.
- `esp [0/1]` - Activates Espionage mode.  Default 0
- `atl [0/1]` - Sets Assassinate the Leader mode, if esp is enabled.
- `etv [0/1]` - Sets Escort the VIP mode, will default to Assassinate the Leader if the map configuration is missing or malformed
- `esp_punish [0/1/2]` - Sets punishment mode for losing team.  0 is default post-round teamplay with FF on.  1 immediately kills all remaining members of the losing team.  2 generates an invulnerability shield on the remaining winning team members for the duration of post-round celebrations, so they can deal with the remaining losing team members in style
- `esp_etv_halftime [0/1]` - Sets halftime rule of swapping teams in an ETV scenario after half of the timelimit has passed
- `esp_showleader [0/1]` - GHUD setting, enabling this will show a marker over your team's leader at all times so that you can find him
- `esp_showtarget [0/1]` - GHUD setting, enabling this will show a marker over your escort target, where the leader needs to reach to win the round
- `esp_leaderequip [0/1/2/3]` - Adjusts the loadout of all leaders, see [espionage.md](./espionage.md) for details
- `esp_enhancedslippers [0|1|2]`: default `0`
  -  If enabled, this removes all limping, and:
    -  `1`: 50% damage reduction on falls
    -  `2`: 25% increased fall height before falling damage is taken
  - You do not need to have Espionage mode enabled to use this feature.

### Gun mechanics/enhancements
- `gun_dualmk23_enhance [0/1]` - server cvar, default 0.  If enabled, this allows both the silencer and the laser sight to be used on the Dual MK23 Pistols.
- `use_gren_bonk [0/1]` - server cvar, default 0.  If enabled, this enables impact damage of the grenade to cause damage on direct contact with a player.  The speed of which the grenade is thrown will determine the damage dealt. Thanks to JukS for the idea and the code.
- `lca_grenade` - server cvar, default 0.  If enabled, players can pull the grenade pin during Lights Camera Action, but they still cannot throw it until Action!

### Highscores
Borrowing code from OpenTDM (thank you Skuller!), high scores are stored in a local file on the server.  Each time a new high score is achieved, it is registered in this file.  The high scores are separated by map and by game mode.  For example, `highscores/dm/wizs.txt` is the highscores file for the map `wizs` in `dm` mode.
- `g_highscores_dir` - serverr cvar, default is `highscores`; not much reason to change this default, but you can if you want
- `g_highscores_countbots` - server cvar, default is `0`, if enabled, bots will be included in high score recording if they achieve a high score
- `highscores` - client command, this will display the high scores in the console

### Outbound messaging
The latest versions enable the use of libcurl to send outbound communications over HTTP/HTTPS, such as to a Discord webhook or a JSON API.  This requires some credentialed information to be stored as cvars, so as a server admin, you are responsible to maintaining these values as secrets, like rcon.
- `use_pickup [0/1]` - server cvar, default 0.  This enables the "Request a Pickup Game" option in the menus, as well as the `pickup` command.  Requires `sv_curl_enable` to be enabled, msgflags of `128` and `sv_curl_discord_info_url` or `sv_curl_discord_pickup_url` to have correct values
- `pickup` - client command, you can initiate this from the console or bind it to a key.  This performs the same functionality as the menu option.  Assuming the server is setup correctly, it will send a pickup match request with server information to a Discord channel.  To limit spam, there is a 5 minute cooldown before anyone in the server can send another request.

## Contact Information
Contacting the AQ2World Team is easy, join our Discord or visit our forums, or leave a Github Issue.  We are always looking for feedback and suggestions.
- Discord Server: [https://discord.aq2world.com](https://discord.aq2world.com)
- Forums: [https://forums.aq2world.com](https://forums.aq2world.com)

Bug reports are now handled on GitHub: [https://github.com/actionquake/q2pro/issues](https://github.com/actionquake/q2pro/issues)

---

*Legacy contact information for the original TNG team:*
If you wish to contact the TNG team, to report a bug, suggest a new feature, or offer JBravo your sister, you can find us on IRC on QuakeNet (irc.quakenet.org) in #action.dev

Send your bug reports to tng-bugs@ra.is and for normal inquiries, email tng@ra.is.

> (NOTE: I doubt any of the above is still accurate, but I'm leaving it here to honor the original devs.)

## Credits
Thanks go to the following people who have helped us make AQ2:The Next Generation in one way or another.

- Mort - Original Millennium CTF and lots of help with coding
- QDevels - Information and code samples
- Clan dW, Clan ROCK, Clan DP and Clan QNI - Testing and suggestions
- All BWAQSL Admins for suggestions and playtesting
- BarrysWorld, Clix, IOL, The Last Resort and Simnet for providing test servers
- iD Software - Original Flag Models/Sounds
- Davross and Chaka - Flag/Banner Skins
- Kobra - Announcer sounds
- BlackMonk - Supplying us with all kinds of info
- Dome - For helping us with some issues and supplying ideas
- Papst - For helping with the overflow problem
- Everybody who reported bugs and features to us
- And of course every AQ2 player, let's keep AQ2 alive!