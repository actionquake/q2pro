//-----------------------------------------------------------------------------
// g_local.h -- local definitions for game module
//
// $Id: g_local.h,v 1.68 2004/04/08 23:19:51 slicerdw Exp $
//
//-----------------------------------------------------------------------------
// $Log: g_local.h,v $
// Revision 1.68  2004/04/08 23:19:51  slicerdw
// Optimized some code, added a couple of features and fixed minor bugs
//
// Revision 1.67  2003/06/15 21:43:53  igor
// added IRC client
//
// Revision 1.66  2003/06/15 15:34:32  igor
// - removed the zcam code from this branch (see other branch)
// - added fixes from 2.72 (source only) version
// - resetted version number to 2.72
// - This version should be exactly like the release 2.72 - just with a few
//   more fixes (which whoever did the source only variant didn't get because
//   he didn't use the CVS as he should. Shame on him.
//
// Revision 1.65  2002/09/04 11:23:09  ra
// Added zcam to TNG and bumped version to 3.0
//
// Revision 1.64  2002/04/01 15:16:06  freud
// Stats code redone, tng_stats now much more smarter. Removed a few global
// variables regarding stats code and added kevlar hits to stats.
//
// Revision 1.63  2002/03/30 17:20:59  ra
// New cvar use_buggy_bandolier to control behavior of dropping bando and grenades
//
// Revision 1.62  2002/03/28 13:30:36  freud
// Included time played in ghost.
//
// Revision 1.61  2002/03/28 12:10:11  freud
// Removed unused variables (compiler warnings).
// Added cvar mm_allowlock.
//
// Revision 1.60  2002/03/28 11:46:03  freud
// stat_mode 2 and timelimit 0 did not show stats at end of round.
// Added lock/unlock.
// A fix for use_oldspawns 1, crash bug.
//
// Revision 1.59  2002/03/25 23:35:19  freud
// Ghost code, use_ghosts and more stuff..
//
// Revision 1.58  2002/03/25 18:32:11  freud
// I'm being too productive.. New ghost command needs testing.
//
// Revision 1.57  2002/03/24 22:45:53  freud
// New spawn code again, bad commit last time..
//
// Revision 1.56  2002/02/19 10:28:43  freud
// Added to %D hit in the kevlar vest and kevlar helmet, also body for handcannon
// and shotgun.
//
// Revision 1.55  2002/02/19 09:32:47  freud
// Removed PING PONGs from CVS, not fit for release.
//
// Revision 1.54  2002/02/18 20:21:36  freud
// Added PING PONG mechanism for timely disconnection of clients. This is
// based on a similar scheme as the scheme used by IRC. The client has
// cvar ping_timeout seconds to reply or will be disconnected.
//
// Revision 1.53  2002/02/18 13:55:35  freud
// Added last damaged players %P
//
// Revision 1.52  2002/02/17 20:10:09  freud
// Better naming of auto_items is auto_equip, requested by Deathwatch.
//
// Revision 1.51  2002/02/17 20:01:32  freud
// Fixed stat_mode overflows, finally.
// Added 2 new cvars:
// 	auto_join (0|1), enables auto joining teams from previous map.
// 	auto_items (0|1), enables weapon and items caching between maps.
//
// Revision 1.50  2002/02/01 17:49:56  freud
// Heavy changes in stats code. Removed lots of variables and replaced them
// with int arrays of MODs. This cleaned tng_stats.c up a whole lots and
// everything looks well, might need more testing.
//
// Revision 1.49  2002/01/24 02:24:56  deathwatch
// Major update to Stats code (thanks to Freud)
// new cvars:
// stats_afterround - will display the stats after a round ends or map ends
// stats_endmap - if on (1) will display the stats scoreboard when the map ends
//
// Revision 1.48  2001/12/24 18:06:05  slicerdw
// changed dynamic check for darkmatch only
//
// Revision 1.46  2001/12/23 21:19:41  deathwatch
// Updated stats with location and average
// cleaned it up a bit as well
//
// Revision 1.45  2001/12/23 16:30:50  ra
// 2.5 ready. New stats from Freud. HC and shotgun gibbing seperated.
//
// Revision 1.44  2001/12/09 14:02:11  slicerdw
// Added gl_clear check -> video_check_glclear cvar
//
// Revision 1.43  2001/11/07 11:41:37  igor_rock
// removed unused variables stats_shots_hd and stats_shots_m
//
// Revision 1.42  2001/10/18 12:55:35  deathwatch
// Added roundtimeleft
//
// Revision 1.41  2001/09/30 03:09:34  ra
// Removed new stats at end of rounds and created a new command to
// do the same functionality.   Command is called "time"
//
// Revision 1.40  2001/09/29 19:54:04  ra
// Made a CVAR to turn off extratimingstats
//
// Revision 1.39  2001/09/28 13:48:34  ra
// I ran indent over the sources. All .c and .h files reindented.
//
// Revision 1.38  2001/09/28 13:44:23  slicerdw
// Several Changes / improvements
//
// Revision 1.37  2001/09/02 20:33:34  deathwatch
// Added use_classic and fixed an issue with ff_afterround, also updated version
// nr and cleaned up some commands.
//
// Updated the VC Project to output the release build correctly.
//
// Revision 1.36  2001/08/20 00:41:15  slicerdw
// Added a new scoreboard for Teamplay with stats ( when map ends )
//
// Revision 1.35  2001/08/18 17:14:04  deathwatch
// Flashlight Added (not done yet, needs to prevent DEAD ppl from using it,
// the glow should be white and a bit smaller if possible and the daiper needs
// to be gone. Also, it should only work in 'darkmatch' I guess and it should
// make a sound when you turn it on/off.
//
// Revision 1.34  2001/08/18 01:28:06  deathwatch
// Fixed some stats stuff, added darkmatch + day_cycle, cleaned up several files, restructured ClientCommand
//
// Revision 1.33  2001/08/17 21:31:37  deathwatch
// Added support for stats
//
// Revision 1.32  2001/08/15 14:50:48  slicerdw
// Added Flood protections to Radio & Voice, Fixed the sniper bug AGAIN
//
// Revision 1.31  2001/08/06 14:38:45  ra
// Adding UVtime for ctf
//
// Revision 1.30  2001/08/06 03:00:49  ra
// Added FF after rounds. Please someone look at the EVIL if statments for me :)
//
// Revision 1.29  2001/07/16 18:28:46  ra
// Changed a 40 second hard limit on mapvoting into a cvar.
//
// Revision 1.28  2001/06/25 11:59:20  slicerdw
// Small Fix
//
// Revision 1.27  2001/06/25 11:44:47  slicerdw
// New Video Check System - video_check and video_check_lockpvs no longer latched
//
// Revision 1.26  2001/06/22 16:34:05  slicerdw
// Finished Matchmode Basics, now with admins, Say command tweaked...
//
// Revision 1.25  2001/06/21 00:05:30  slicerdw
// New Video Check System done -  might need some revision but works..
//
// Revision 1.22  2001/06/20 07:21:21  igor_rock
// added use_warnings to enable/disable time/frags left msgs
// added use_rewards to enable/disable eimpressive, excellent and accuracy msgs
// change the configfile prefix for modes to "mode_" instead "../mode-" because
// they don't have to be in the q2 dir for doewnload protection (action dir is sufficient)
// and the "-" is bad in filenames because of linux command line parameters start with "-"
//
// Revision 1.21  2001/06/19 18:56:38  deathwatch
// New Last killed target system
//
// Revision 1.20  2001/06/18 12:36:40  igor_rock
// added new irvision mode (with reddish screen and alpha blend) and corresponding
// new cvar "new_irvision" to enable the new mode
//
// Revision 1.19  2001/06/13 08:39:13  igor_rock
// changed "cvote" to "use_cvote" (like the other votecvars)
//
// Revision 1.18  2001/06/13 08:00:25  igor_rock
// corrected stupid error (wrong changes committed)
//
// Revision 1.17  2001/06/13 07:55:17  igor_rock
// Re-Added a_match.h and a_match.c
// Added CTF Header for a_ctf.h and a_ctf.c
//
// Revision 1.16  2001/06/01 19:18:42  slicerdw
// Added Matchmode Code
//
// Revision 1.15  2001/05/31 16:58:14  igor_rock
// conflicts resolved
//
// Revision 1.14.2.4  2001/05/25 18:59:52  igor_rock
// Added CTF Mode completly :)
// Support for .flg files is still missing, but with "real" CTF maps like
// tq2gtd1 the ctf works fine.
// (I hope that all other modes still work, just tested DM and teamplay)
//
// Revision 1.14.2.3  2001/05/20 18:54:19  igor_rock
// added original ctf code snippets from zoid. lib compilesand runs but
// doesn't function the right way.
// Jsut committing these to have a base to return to if something wents
// awfully wrong.
//
// Revision 1.14.2.2  2001/05/20 15:54:43  igor_rock
// added ctf variables and include, "ctf_team" (zoid) changes to "team" (aq2:tng)
//
// Revision 1.14.2.1  2001/05/20 15:17:31  igor_rock
// removed the old ctf code completly
//
// Revision 1.14  2001/05/14 21:10:16  igor_rock
// added wp_flags support (and itm_flags skeleton - doesn't disturb in the moment)
//
// Revision 1.13  2001/05/13 01:23:01  deathwatch
// Added Single Barreled Handcannon mode, made the menus and scoreboards
// look nicer and made the voice command a bit less loud.
//
// Revision 1.12  2001/05/12 21:19:51  ra
//
//
// Added punishkills.
//
// Revision 1.11  2001/05/12 20:58:22  ra
//
//
// Adding public mapvoting and kickvoting. Its controlable via cvar's mv_public
// and vk_public (both default off)
//
// Revision 1.10  2001/05/12 13:48:58  mort
// Fixed CTF ForceSpawn bug
//
// Revision 1.9  2001/05/11 16:07:25  mort
// Various CTF bits and pieces...
//
// Revision 1.8  2001/05/11 12:21:19  slicerdw
// Commented old Location support ( ADF ) With the ML/ETE Compatible one
//
// Revision 1.6  2001/05/08 12:55:43  igor_rock
// added inlcude file <ctype.h>
//
// Revision 1.5  2001/05/07 22:03:15  slicerdw
// Added sv stuffcmd
//
// Revision 1.4  2001/05/07 21:18:34  slicerdw
// Added Video Checking System
//
// Revision 1.3  2001/05/07 08:32:17  mort
// Basic CTF code
// No spawns etc
// Just the cvars and flag entity
//
// Revision 1.2  2001/05/07 02:05:36  ra
//
//
// Added tkok command to forgive teamkills.
//
// Revision 1.1.1.1  2001/05/06 17:29:25  igor_rock
// This is the PG Bund Edition V1.25 with all stuff laying around here...
//
//-----------------------------------------------------------------------------

#include <ctype.h>

#include "shared/shared.h"
#include "q_ghud.h"
#include <stddef.h>

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define		GAME_INCLUDE
#include	"shared/game.h"
#include	"shared/gameext.h"

#include	"a_team.h"
#include	"a_game.h"
#include	"a_menu.h"
#include	"a_radio.h"
#include	"a_xcmds.h"
#include	"a_xgame.h"
#include	"tng_ini.h"
#include	"a_tourney.h"
#include	"a_xvote.h"
#include	"a_xmenu.h"
#include	"a_vote.h"
#include	"a_match.h"
#include	"tng_stats.h"		// Adding TNG Stats File
#include	"tng_irc.h"
#include	"tng_balancer.h"
#include	"tng_jump.h"
#include	"g_grapple.h"
#include	"p_antilag.h"
#if AQTION_CURL
#include 	"tng_net.h"
#endif

#ifndef NO_BOTS
//#include	"acesrc/botnav.h"
#include	"botlib/botlib.h"
#endif

// 9b15 commit from upstream -- using player_state_old_t instead of player_state_new_t
typedef struct gclient_s gclient_t;

#define		getEnt(entnum)	(edict_t *)((char *)globals.edicts + (globals.edict_size * entnum))	//AQ:TNG Slicer - This was missing
#define		GAMEVERSION			"action"	// the "gameversion" client command will print this plus compile date

#define G_FEATURES (GMF_CLIENTNUM | GMF_PROPERINUSE | GMF_MVDSPEC | GMF_WANT_ALL_DISCONNECTS | GMF_VARIABLE_FPS)

// protocol bytes that can be directly added to messages
#define svc_muzzleflash         1
#define svc_muzzleflash2        2
#define svc_temp_entity         3
#define svc_layout              4
#define svc_inventory           5
#define svc_sound				9
#define svc_stufftext           11
#define	svc_configstring		13

// These are mapped to the enum svc_ops_t in protocol.h q2pro
// If these need adjusted in q2pro, adjust them here too
// Warning: this breaks backwards compatbility with earlier versions of AQtion if you change it
#define svc_extend				30
#define svc_userstatistic		31

//==================================================================

#define MASK_VOLUME			1
#define MASK_ATTENUATION	2
#define MASK_POSITION		4
#define MASK_ENTITY_CHANNEL 8
#define MASK_TIMEOFS		16

// view pitching times
#define DAMAGE_TIME             0.5f
#define FALL_TIME               0.3f

// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define SPAWNFLAG_NOT_EASY              BIT(8)
#define SPAWNFLAG_NOT_MEDIUM            BIT(9)
#define SPAWNFLAG_NOT_HARD              BIT(10)
#define SPAWNFLAG_NOT_DEATHMATCH        BIT(11)
#define SPAWNFLAG_NOT_COOP              BIT(12)

// edict->flags
#define FL_FLY                  BIT(0)
#define FL_SWIM                 BIT(1)      // implied immunity to drowining
#define FL_IMMUNE_LASER         BIT(2)
#define FL_INWATER              BIT(3)
#define FL_GODMODE              BIT(4)
#define FL_NOTARGET             BIT(5)
#define FL_IMMUNE_SLIME         BIT(6)
#define FL_IMMUNE_LAVA          BIT(7)
#define FL_PARTIALGROUND        BIT(8)      // not all corners are valid
#define FL_WATERJUMP            BIT(9)      // player jumping out of water
#define FL_TEAMSLAVE            BIT(10)     // not the first on the team
#define FL_NO_KNOCKBACK         BIT(11)
#define FL_POWER_ARMOR          BIT(12)     // power armor (if any) is active

#define FL_NO_DAMAGE_EFFECTS	BIT(20)	// no damage effects
#define FL_ACCELERATE			BIT(29)  // accelerative movement
#define FL_RESPAWN              BIT(31)     // used for item respawning

// edict->client->pers.spec_flags
#define SPECFL_KILLFEED					BIT(0)
#define SPECFL_SPECHUD					BIT(1)
#define SPECFL_SPECHUD_NEW				BIT(2)

// variable server FPS
#ifndef NO_FPS
#define HZ              game.framerate // (int)sv_fps->value
#define FRAMETIME       game.frametime
#define FRAMEDIV        game.framediv  // (sv_fps / 10)
#define FRAMESYNC       !(level.framenum % game.framediv)
#else
#define HZ              BASE_FRAMERATE
#define FRAMETIME       BASE_FRAMETIME_1000
#define FRAMEDIV        1
#define FRAMESYNC       1
#endif

#define KEYFRAME(x)   (level.framenum + (x) - (level.framenum % FRAMEDIV))

#define NEXT_FRAME(ent, func) \
    ((ent)->think = (func), (ent)->nextthink = level.framenum + 1)

#define NEXT_KEYFRAME(ent, func) \
    ((ent)->think = (func), (ent)->nextthink = KEYFRAME(FRAMEDIV))

// memory tags to allow dynamic memory to be cleaned up
#define TAG_GAME        765	// clear when unloading the dll
#define TAG_LEVEL       766	// clear when loading a new level


#define MELEE_DISTANCE  80

#define BODY_QUEUE_SIZE  8

typedef enum
{
  DAMAGE_NO,
  DAMAGE_YES,			// will take damage if hit
  DAMAGE_AIM			// auto targeting recognizes this
}
damage_t;

typedef enum
{
  WEAPON_READY,
  WEAPON_ACTIVATING,
  WEAPON_DROPPING,
  WEAPON_FIRING,

  WEAPON_END_MAG,
  WEAPON_RELOADING,
  WEAPON_BURSTING,
  WEAPON_BUSY,			// used by sniper rifle when engaging zoom, if I want to make laser sight toggle on/off  this could be used for that too...
  WEAPON_BANDAGING
}
weaponstate_t;

typedef enum
{
  AMMO_BULLETS,
  AMMO_SHELLS,
  AMMO_ROCKETS,
  AMMO_GRENADES,
  AMMO_CELLS,
  AMMO_SLUGS
}
ammo_t;

//tng_net.c
typedef enum {
	SERVER_WARMING_UP = BIT(0),   // 1
	DEATH_MSG = BIT(1),           // 2
	CHAT_MSG = BIT(2),            // 4
	AWARD_MSG = BIT(3),           // 8
	SERVER_MSG = BIT(4),          // 16
	MATCH_START_MSG = BIT(5),     // 32
	MATCH_END_MSG = BIT(6),       // 64
	PICKUP_REQ_MSG = BIT(7),      // 128
	NOTIFY_MAX = BIT(8)           // 256 (enable all)
} Discord_Notifications;

// Default messages
#define MM_MATCH_END_MSG "Matchmode Results"
#define DM_MATCH_END_MSG "Deathmatch Results"
#define MM_3_MIN_WARN "3 minutes remaining in the map"
#define PICKUP_GAME_REQUEST "A pickup game has been started"
#define TP_MATCH_START_MSG "Match is about to begin!"
#define TP_MATCH_END_MSG "Match has ended!"

//deadflag
#define DEAD_NO                         0
#define DEAD_DYING                      1
#define DEAD_DEAD                       2
#define DEAD_RESPAWNABLE                3

//range
#define RANGE_MELEE                     0
#define RANGE_NEAR                      1
#define RANGE_MID                       2
#define RANGE_FAR                       3

//gib types
#define GIB_ORGANIC                     0
#define GIB_METALLIC                    1

//monster ai flags
#define AI_STAND_GROUND         BIT(0)
#define AI_TEMP_STAND_GROUND    BIT(1)
#define AI_SOUND_TARGET         BIT(2)
#define AI_LOST_SIGHT           BIT(3)
#define AI_PURSUIT_LAST_SEEN    BIT(4)
#define AI_PURSUE_NEXT          BIT(5)
#define AI_PURSUE_TEMP          BIT(6)
#define AI_HOLD_FRAME           BIT(7)
#define AI_GOOD_GUY             BIT(8)
#define AI_BRUTAL               BIT(9)
#define AI_NOSTEP               BIT(10)
#define AI_DUCKED               BIT(11)
#define AI_COMBAT_POINT         BIT(12)
#define AI_MEDIC                BIT(13)
#define AI_RESURRECTING         BIT(14)

//monster attack state
#define AS_STRAIGHT                     1
#define AS_SLIDING                      2
#define AS_MELEE                        3
#define AS_MISSILE                      4

// armor types
#define ARMOR_NONE                      0
#define ARMOR_JACKET                    1
#define ARMOR_COMBAT                    2
#define ARMOR_BODY                      3
#define ARMOR_SHARD                     4

// power armor types
#define POWER_ARMOR_NONE                0
#define POWER_ARMOR_SCREEN              1
#define POWER_ARMOR_SHIELD              2

// handedness values
#define RIGHT_HANDED                    0
#define LEFT_HANDED                     1
#define CENTER_HANDED                   2


// game.serverflags values
#define SFL_CROSS_TRIGGER_1     BIT(0)
#define SFL_CROSS_TRIGGER_2     BIT(1)
#define SFL_CROSS_TRIGGER_3     BIT(2)
#define SFL_CROSS_TRIGGER_4     BIT(3)
#define SFL_CROSS_TRIGGER_5     BIT(4)
#define SFL_CROSS_TRIGGER_6     BIT(5)
#define SFL_CROSS_TRIGGER_7     BIT(6)
#define SFL_CROSS_TRIGGER_8     BIT(7)
#define SFL_CROSS_TRIGGER_MASK  (BIT(8) - 1)


// noise types for PlayerNoise
#define PNOISE_SELF                     0
#define PNOISE_WEAPON                   1
#define PNOISE_IMPACT                   2


// edict->movetype values
typedef enum
{
  MOVETYPE_NONE,		// never moves
  MOVETYPE_NOCLIP,		// origin and angles change with no interaction
  MOVETYPE_PUSH,		// no clip to world, push on box contact
  MOVETYPE_STOP,		// no clip to world, stops on box contact

  MOVETYPE_WALK,		// gravity
  MOVETYPE_STEP,		// gravity, special edge handling
  MOVETYPE_FLY,
  MOVETYPE_TOSS,		// gravity
  MOVETYPE_TOSS_NOPUSH,	// gravity, don't get pushed by doors/plats
  MOVETYPE_FLYMISSILE,		// extra size to monsters
  MOVETYPE_BOUNCE,
  MOVETYPE_BLOOD
}
movetype_t;



typedef struct
{
  int base_count;
  int max_count;
  float normal_protection;
  float energy_protection;
  int armor;
}
gitem_armor_t;


// gitem_t->flags
#define IT_WEAPON               1	// use makes active weapon
#define IT_AMMO                 2
#define IT_ARMOR                4
#define IT_STAY_COOP            8
#define IT_KEY                  16
#define IT_POWERUP              32
#define IT_ITEM                 64
#define IT_FLAG                 128


// weapon names
/*
bind 2 "use M3 Super 90 Assault Shotgun;"
bind 3 "use MP5/10 Submachinegun"
bind 4 "use Handcannon"
bind 5 "use M4 Assault Rifle"
bind 6 "use Sniper Rifle"
*/
#define MK23_NAME    "MK23 Pistol"
#define MP5_NAME     "MP5/10 Submachinegun"
#define M4_NAME      "M4 Assault Rifle"
#define M3_NAME      "M3 Super 90 Assault Shotgun"
#define HC_NAME      "Handcannon"
#define SNIPER_NAME  "Sniper Rifle"
#define DUAL_NAME    "Dual MK23 Pistols"
#define KNIFE_NAME   "Combat Knife"
#define GRENADE_NAME "M26 Fragmentation Grenade"

#define MK23_AMMO_NAME    "Pistol Magazine"
#define MP5_AMMO_NAME     "MP5 Magazine"
#define M4_AMMO_NAME      "M4 Magazine"
#define SHOTGUN_AMMO_NAME "12 Gauge Shells"
#define SNIPER_AMMO_NAME  "AP Sniper Ammo"

#define SIL_NAME     "Silencer"
#define SLIP_NAME    "Stealth Slippers"
#define BAND_NAME    "Bandolier"
#define KEV_NAME     "Kevlar Vest"
#define HELM_NAME    "Kevlar Helmet"
#define LASER_NAME   "Lasersight"

#define C_KIT_NAME	 "Commando Kit"
#define S_KIT_NAME	 "Stealth Kit"
#define A_KIT_NAME	 "Assassin Kit"

#define C_KIT_NAME_FULL	 "Commando Kit (Bandolier + Helm)"
#define S_KIT_NAME_FULL	 "Stealth Kit (Slippers + Silencer)"
#define A_KIT_NAME_FULL	 "Assassin Kit (Laser + Silencer)"

#define NO_NUM					0

#define MK23_NUM				1
#define MP5_NUM					2
#define M4_NUM					3
#define M3_NUM					4
#define HC_NUM					5
#define SNIPER_NUM				6
#define DUAL_NUM				7
#define KNIFE_NUM				8
#define GRENADE_NUM				9

#define SIL_NUM					10
#define SLIP_NUM				11
#define BAND_NUM				12
#define KEV_NUM					13
#define LASER_NUM				14
#define HELM_NUM				15

#define MK23_ANUM				16
#define MP5_ANUM				17
#define M4_ANUM					18
#define SHELL_ANUM				19
#define SNIPER_ANUM				20

#define FLAG_T1_NUM				21
#define FLAG_T2_NUM				22

#define GRAPPLE_NUM				23

#define ITEM_MAX_NUM			24

#define C_KIT_NUM				25
#define S_KIT_NUM				26
#define A_KIT_NUM				27

#define KIT_MAX_NUM				28

#define WEAPON_COUNT			9
#define ITEM_COUNT				6
#define AMMO_COUNT				5
#define KIT_COUNT               3
#define WEAPON_FIRST			1
#define WEAPON_MAX				WEAPON_FIRST+WEAPON_COUNT
#define ITEM_FIRST				WEAPON_MAX
#define ITEM_MAX				ITEM_FIRST+ITEM_COUNT
#define AMMO_FIRST				ITEM_MAX
#define AMMO_MAX				AMMO_FIRST+AMMO_COUNT
#define KIT_FIRST				C_KIT_NUM
#define KIT_MAX					KIT_MAX_NUM

//AQ2:TNG - Igor adding wp_flags/itm_flags
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define WPF_MASK				((1 << WEAPON_COUNT) - 1)
#define WPF_DEFAULT				511 //WPF_MASK
#define WPF_DEFAULT_STR			TOSTRING(WPF_DEFAULT)
#define WPF_ALLOWED(typeNum)	((int)wp_flags->value & items[typeNum].flag)

#define ITF_MASK				((1 << ITEM_COUNT) - 1)
#define ITF_DEFAULT				63 //ITF_MASK
#define ITF_DEFAULT_STR			TOSTRING(ITF_DEFAULT)
#define ITF_ALLOWED(typeNum)	((int)itm_flags->value & items[typeNum].flag)

#define MAX_SPAWNS 512		// max DM spawn points supported

// High Scores support from OpenFFA
#define MAX_HIGH_SCORES	10

typedef struct highscore_s
{
	char name[MAX_CLIENT_NAME];
    int score;
	float fragsper;
	float accuracy;
    time_t time;
} highscore_t;


//AQ2:TNG End adding flags

typedef struct itemList_s
{
	int		index;
	int		flag;
} itemList_t;

extern itemList_t items[ITEM_MAX_NUM];

// sniper modes
#define SNIPER_1X		0
#define SNIPER_2X		1
#define SNIPER_4X		2
#define SNIPER_6X		3
#define SNIPER_MODE_MAX	4

//TempFile sniper zoom moved to constants
#define SNIPER_FOV1		90
#define SNIPER_FOV2		45
#define SNIPER_FOV4		20
#define SNIPER_FOV6		10

#define GRENADE_IDLE_FIRST  40
#define GRENADE_IDLE_LAST   69
#define GRENADE_THROW_FIRST 4
#define GRENADE_THROW_LAST  9	// throw it on frame 8?


// these should be server variables, when I get around to it
//#define UNIQUE_WEAPONS_ALLOWED 2
//#define UNIQUE_ITEMS_ALLOWED   1
#define SPEC_WEAPON_RESPAWN 		1
#define BANDAGE_TIME    			27	// 10 = 1 second
#define ENHANCED_BANDAGE_TIME		10
#define BLEED_TIME      			10	// 10 = 1 second is time for losing 1 health at slowest bleed rate
// Igor's back in Time to hard grenades :-)
#define GRENADE_DAMRAD_CLASSIC  	170
#define GRENADE_DAMRAD          	250


typedef struct gitem_s
{
  char *classname;		// spawning name

  qboolean (*pickup) (struct edict_s * ent, struct edict_s * other);
  void (*use) (struct edict_s * ent, struct gitem_s * item);
  void (*drop) (struct edict_s * ent, struct gitem_s * item);
  void (*weaponthink) (struct edict_s * ent);
  char *pickup_sound;
  char *world_model;
  int world_model_flags;
  char *view_model;

  // client side info
  char *icon;
  char *pickup_name;		// for printing on pickup
  int count_width;		// number of digits to display by icon
  int quantity;			// for ammo how much, for weapons how much is used per shot
  char *ammo;			// for weapons
  int flags;			// IT_* flags
  void *info;
  int tag;
  char *precaches;		// string of all models, sounds, and images this item will use
  int typeNum;
}
gitem_t;

typedef struct precache_s {
    struct precache_s   *next;
    void                (*func)(void);
} precache_t;

//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
  gclient_t *clients;		// [maxclients]

  // can't store spawnpoint in level, because
  // it would get overwritten by the savegame restore
  char spawnpoint[512];		// needed for coop respawns

  // store latched cvars here that we want to get at often
  int maxclients;
  int maxentities;

  // cross level triggers
  int serverflags;
  int serverfeatures;

  int framerate;
  float frametime;
  int framediv;

  // items
  int num_items;
	
  // stats
  char matchid[MAX_QPATH];
  int gamemode;
  int gamemodeflags;
  int roundNum;
  qboolean ai_ent_found;
  int bot_count;

  // API-related
  int srv_announce_timeout;

  //q2pro protocol extensions
  cs_remap_t  csr;
  precache_t  *precaches;

  // High Scores support from OpenFFA
  char        dir[MAX_OSPATH]; // where variable data is stored
  
  // Bot personalities loaded
  #ifndef NO_BOTS
  char* bot_file_path[MAX_QPATH];
  int used_bot_personalities;
  #endif

  #if AQTION_CURL
  // Discord Webhook limits
  qboolean time_warning_sent; 	// This is set to true when the time warning has been sent, resets every map
  
  #endif
}
game_locals_t;

// Map features, utilized by / requires bot_personality to be enabled
typedef struct map_features_s
{
	float volume;
	float openness;
	float water_amt;
	float lava_amt;
	float slime_amt;

} map_features_t;

//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
  int framenum;
  float time;

  char level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)

  char mapname[MAX_QPATH];	// the server name (base1, etc)

  char nextmap[MAX_QPATH];	// go here when fraglimit is hit
  //+

  char tempmap[MAX_QPATH];	//PG BUND
 
  // intermission state
  int intermission_framenum;	// time the intermission was started
  int intermission_exit;
  vec3_t intermission_origin;
  vec3_t intermission_angle;

  // high scores from OpenFFA
  highscore_t scores[MAX_HIGH_SCORES];
  int numscores;
  time_t record;        // not zero if scores updated

  char *changemap;

  char statusbar[1024];  //MAX is 1536 = (MAX_QPATH * (CS_AIRACCEL - CS_STATUSBAR))
  char spec_statusbar[1024];
  int spec_statusbar_lastupdate;

  int pic_health;
  int pic_items[ITEM_MAX_NUM];
  int pic_weapon_ammo[WEAPON_MAX];
  int pic_sniper_mode[SNIPER_MODE_MAX];
  int pic_teamskin[TEAM_TOP];
  int pic_teamtag;
  
  int pic_ctf_teamtag[TEAM_TOP];
  int pic_ctf_flagbase[TEAM_TOP];
  int pic_ctf_flagtaken[TEAM_TOP];
  int pic_ctf_flagdropped[TEAM_TOP];

  int snd_fry;
  int snd_lights;
  int snd_camera;
  int snd_action;
  int snd_teamwins[TEAM_TOP];
  int snd_silencer;
  int snd_headshot;
  int snd_vesthit;
  int snd_knifethrow;
  int snd_kick;
  int snd_noammo;
  int snd_grenhead;
  int snd_grenhelm;
  int snd_grenbody;

  int model_null;
  int model_lsight;
#if AQTION_EXTENSION
  int model_arrow;
#endif

  edict_t *current_entity;	// entity running from G_RunFrame

  int body_que;			// dead bodies

  int specspawn;		// determines if initial spawning has occured

  int realFramenum; //when game paused, framenum stays the same
  int pauseFrames;
  float matchTime;
  float emptyTime;
  int weapon_sound_framenum;
  int pic_teamplay_timer_icon;

  int pic_leaderskin[TEAM_TOP];  
  int pic_esp_teamtag[TEAM_TOP];
  int pic_esp_teamicon[TEAM_TOP];
  int pic_esp_leadericon[TEAM_TOP];
  int pic_esp_respawn_icon;

  // Point of interest
  vec3_t poi_origin;
  vec3_t poi_angle;

  // tng_net.c
  int lc_recently_sent[NOTIFY_MAX];	// Used to prevent spamming of the endpoint
  // Map features
  map_features_t map_features;
}
level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct
{
  // world vars
  char *sky;
  float skyrotate;
  vec3_t skyaxis;
  char *nextmap;

  int lip;
  int distance;
  int height;
  char *noise;
  float pausetime;
  char *item;
  char *gravity;

  float minyaw;
  float maxyaw;
  float minpitch;
  float maxpitch;
}
spawn_temp_t;


typedef struct
{
  // fixed data
  vec3_t start_origin;
  vec3_t start_angles;
  vec3_t end_origin;
  vec3_t end_angles;

  int sound_start;
  int sound_middle;
  int sound_end;

  float accel;
  float speed;
  float decel;
  float distance;

  float wait;

  // state data
  int state;
  vec3_t dir;
  float current_speed;
  float move_speed;
  float next_speed;
  float remaining_distance;
  float decel_distance;
  void (*endfunc) (edict_t *);
}
moveinfo_t;


extern game_locals_t game;
extern level_locals_t level;
extern game_import_t gi;
extern game_export_t globals;
extern const game_import_ex_t *gix;
extern const game_export_ex_t gex;
extern spawn_temp_t st;

extern int sm_meat_index;

// means of death
typedef enum {
    MOD_UNKNOWN = 0,
    MOD_MK23,
    MOD_MP5,
    MOD_M4,
    MOD_M3,
    MOD_HC,
    MOD_SNIPER,
    MOD_DUAL,
    MOD_KNIFE,
    MOD_KNIFE_THROWN,
    MOD_GRENADE,
    MOD_G_SPLASH,
    MOD_HANDGRENADE,
    MOD_HG_SPLASH,
    MOD_PUNCH,
    MOD_BLASTER,
    MOD_HYPERBLASTER,
    MOD_WATER,
    MOD_SLIME,
    MOD_LAVA,
    MOD_CRUSH,
    MOD_TELEFRAG,
    MOD_FALLING,
    MOD_SUICIDE,
    MOD_HELD_GRENADE,
    MOD_EXPLOSIVE,
    MOD_BARREL,
    MOD_BOMB,
    MOD_EXIT,
    MOD_SPLASH,
    MOD_TARGET_LASER,
    MOD_TRIGGER_HURT,
    MOD_HIT,
    MOD_TARGET_BLASTER,
    MOD_BLEEDING,
    MOD_KICK,
    MOD_GRAPPLE,
	MOD_GRENADE_IMPACT,
    MOD_TOTAL,
    MOD_FRIENDLY_FIRE = 0x8000000
} ModTable;

// types of locations that can be hit
typedef enum {
    LOC_HDAM = 1,    // head
    LOC_CDAM,        // chest
    LOC_SDAM,        // stomach
    LOC_LDAM,        // legs
    LOC_KVLR_HELMET, // kevlar helmet	Freud, for %D
    LOC_KVLR_VEST,   // kevlar vest 		Freud, for %D
    LOC_NO,          // Shot by shotgun or handcannon
    LOC_MAX
} ModLocation;

// Awards
typedef enum {
	AWARD_NONE,
    ACCURACY,
    IMPRESSIVE,
    EXCELLENT,
	DOMINATING,
	UNSTOPPABLE,
	AWARD_MAX
} Awards;

// Game Modes
typedef enum {
    GM_TEAMPLAY,
    GM_TEAMDM,
    GM_CTF,
    GM_TOURNEY,
    GM_DEATHMATCH,
    GM_DOMINATION,
    GM_ASSASSINATE_THE_LEADER,
    GM_ESCORT_THE_VIP,
	GM_MAX
} GameMode;

// Game Mode Flags
#define GMF_NONE 0
#define GMF_3TEAMS 1
//#define GMF_NEW_MODE 2       // If new game mode flags are created, use 2 for its value first
#define GMF_DARKMATCH 4
#define GMF_MATCHMODE 8

// Game Mode Names
#define GMN_TEAMPLAY "Teamplay"
#define GMN_TEAMDM "TeamDM"
#define GMN_CTF "CTF"
#define GMN_TOURNEY "Tourney"
#define GMN_DEATHMATCH "Deathmatch"
#define GMN_DOMINATION "Domination"
#define GMN_ESPIONAGE "Espionage"
#define GMN_JUMP "Jump"
#define GMN_3TEAMS "3 Teams"
//#define GMN_NEW_MODE 2       // If new game mode flags are created, use 2 for its value first
#define GMN_DARKMATCH "Darkmatch"
#define GMN_MATCHMODE "Matchmode"

extern int meansOfDeath;
// zucc for hitlocation of death
extern int locOfDeath;
// stop an armor piercing round that hits a vest
extern int stopAP;

extern edict_t *g_edicts;

#define FOFS(x)  q_offsetof(edict_t, x)
#define STOFS(x) q_offsetof(spawn_temp_t, x)
#define LLOFS(x) (ptrdiff_t)&(((level_locals_t *)0)->x)
#define CLOFS(x) (ptrdiff_t)&(((gclient_t *)0)->x)

#define random()        ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()       (2.0 * (random() - 0.5))

#define DMFLAGS(x)     (((int)dmflags->value & x) != 0)
#define MSGFLAGS(x)	   (((int)msgflags->value & x) != 0)

#ifndef NO_BOTS
#define AQ2WTEAMSIZE	46
#define NUMNAMES		10
#endif

extern cvar_t *maxentities;
extern cvar_t *deathmatch;
extern cvar_t *coop;
extern cvar_t *dmflags;
extern cvar_t *needpass;
extern cvar_t *hostname;
extern cvar_t *teamplay;
extern cvar_t *radiolog;
extern cvar_t *motd_time;
extern cvar_t *actionmaps;
extern cvar_t *roundtimelimit;
extern cvar_t *maxteamkills;
extern cvar_t *tkbanrounds;
extern cvar_t *twbanrounds;
extern cvar_t *limchasecam;
extern cvar_t *roundlimit;
extern cvar_t *skipmotd;
extern cvar_t *nohud;
extern cvar_t *hud_team_icon;
extern cvar_t *hud_items_cycle;
extern cvar_t *noscore;
extern cvar_t *hud_noscore;
extern cvar_t *use_newscore;
extern cvar_t *scoreboard;
extern cvar_t *actionversion;
extern cvar_t *net_port;
#ifndef NO_BOTS
extern cvar_t *ltk_jumpy;
#endif
extern cvar_t *use_voice;
extern cvar_t *ppl_idletime;
extern cvar_t *use_tourney;
extern cvar_t *use_3teams;
extern cvar_t *use_randoms; // Random weapons and items mode
extern cvar_t *use_kickvote;
extern cvar_t *mv_public;
extern cvar_t *vk_public;
extern cvar_t *punishkills;
extern cvar_t *mapvote_waittime;
extern cvar_t *ff_afterround;
extern cvar_t *use_buggy_bandolier;
extern cvar_t *uvtime;
extern cvar_t *use_mapvote;	// enable map voting
extern cvar_t *use_scramblevote;
extern cvar_t *sv_gib;  // Enables or disables gibs
extern cvar_t *sv_crlf;
extern cvar_t *vrot;
extern cvar_t *rrot;
extern cvar_t *empty_rotate;
extern cvar_t *empty_exec;
extern cvar_t *strtwpn;
extern cvar_t *llsound; // Enable or disable the lowlag sounds (needed for selectable gun sounds)
extern cvar_t *loud_guns;
extern cvar_t *sync_guns;
extern cvar_t *silentwalk;
extern cvar_t *slopefix;
extern cvar_t *use_cvote;
extern cvar_t *new_irvision; // Enables or disables the new infra-red vision
extern cvar_t *use_rewards; // Enables or disables the rewards system (Accuracy, Impressive, Excellent, etc)
extern cvar_t *use_warnings; // Enables or disables the warnings system (One minute left...etc)
extern cvar_t *video_check;	//AQ2:TNG - Slicer: For Video Checking
extern cvar_t *video_checktime;	//interval between cheat checks
extern cvar_t *video_max_3dfx;
extern cvar_t *video_max_3dfxam;
extern cvar_t *video_max_opengl;
extern cvar_t *video_check_lockpvs;
extern cvar_t *video_check_glclear;
extern cvar_t *video_force_restart;
extern cvar_t *check_time;
extern cvar_t *matchmode; // Enables or disables matchmode
extern cvar_t *darkmatch; // Enables or disables darkmatch
extern cvar_t *day_cycle;	// If darkmatch is on, this value is the nr of seconds between each interval (day, dusk, night, dawn)
extern cvar_t *use_flashlight;  // Allow flashlight when not darkmatch?

extern cvar_t *hearall;		// used in match mode
extern cvar_t *deadtalk;	// Allow dead players to be heard by live players (chat)
extern cvar_t *force_skin;  // Forces model/skin on all players
extern cvar_t *mm_forceteamtalk;
extern cvar_t *mm_adminpwd;
extern cvar_t *mm_allowlock;
extern cvar_t *mm_pausecount;
extern cvar_t *mm_pausetime;

extern cvar_t *teamdm;
extern cvar_t *teamdm_respawn;
extern cvar_t *respawn_effect;

extern cvar_t *item_respawnmode;  // 0 sets items to respawn in the classic way (info_player_deathmatch), 1 spawns them at specific locations

extern cvar_t *use_mvd2;	// JBravo: activate mvd2 recording on servers running q2pro

extern cvar_t *item_respawn;	// Timer: Interval between item respawns
extern cvar_t *weapon_respawn;  // Timer: Interval between weapon respawns
extern cvar_t *ammo_respawn;    // Timer: Interval between ammo respawns

extern cvar_t *wave_time;  // How often a player can use the 'wave' commands in seconds

extern cvar_t *use_punch;  // Enable or disable the punch command

extern cvar_t *radio_max;
extern cvar_t *radio_time;
extern cvar_t *radio_ban;
extern cvar_t *radio_repeat;
//SLIC2
extern cvar_t *radio_repeat_time;

extern cvar_t *hc_single;  // Enable or disable the single shot handcannon
extern cvar_t *hc_boost; //rekkie -- allow HC to 'boost' the player
extern cvar_t *hc_boost_percent; //rekkie -- allow HC to 'boost' the player
extern cvar_t *hc_silencer; //rekkie -- allow HC to 'boost' the player
extern cvar_t *wp_flags;   // Weapon flags (bans)
extern cvar_t *itm_flags;  // Item flags (bans)
extern cvar_t *use_classic;	// Use_classic resets weapon balance to 1.52
extern cvar_t *use_gren_bonk;  // Toggle on/off direct grenade impact damage

extern cvar_t *warmup;    // Enables warmup (value in seconds)
extern cvar_t *warmup_bots;  // Enables bots to spawn during warmup (value in number of bots)
extern cvar_t *round_begin;  // Enables round_begin (value in seconds)
extern cvar_t *spectator_hud; // Enables or disables the fancy spectator hud

extern cvar_t *fraglimit;
extern cvar_t *timelimit;
extern cvar_t *maptime;
extern cvar_t *capturelimit;
extern cvar_t *password;
extern cvar_t *g_select_empty;
extern cvar_t *g_protocol_extensions;  // Enable or disable protocol extensions.  For legacy compatibility, set this to "0"
extern cvar_t *dedicated;
extern cvar_t *steamid;

extern cvar_t *filterban;
extern cvar_t* silenceban; //rekkie -- silence ban
extern cvar_t *flood_threshold;

extern cvar_t *sv_gravity;
extern cvar_t *sv_maxvelocity;

extern cvar_t *gun_x, *gun_y, *gun_z;
extern cvar_t *sv_rollspeed;
extern cvar_t *sv_rollangle;

extern cvar_t *run_pitch;
extern cvar_t *run_roll;
extern cvar_t *bob_up;
extern cvar_t *bob_pitch;
extern cvar_t *bob_roll;

extern cvar_t *sv_cheats;
extern cvar_t *maxclients;

extern cvar_t *unique_weapons; // Max number of unique weapons a player can carry
extern cvar_t *unique_items;  // Max number of unique items a player can carry
extern cvar_t *ir;		//toggles if bandolier works as infra-red sensor

extern cvar_t *knifelimit; // Maximum number of knives a player can carry
extern cvar_t *tgren; // Maximum number of grenades players start with (with bandolier)
extern cvar_t *allweapon; // Enable or disable all weapons on spawn
extern cvar_t *allitem; // Enable or disable all items on spawn
extern cvar_t *allow_hoarding; // Allow carrying multiple of the same special item or unique weapon.

extern cvar_t *stats_endmap; // If on (1), show the accuracy/etc stats at the end of a map
extern cvar_t *stats_afterround; // TNG Stats, collect stats between rounds

extern cvar_t *auto_join;	// Automaticly join clients to teams they were on in last map.
extern cvar_t *auto_equip;	// Remember weapons and items for players between maps.
extern cvar_t *auto_menu;	// Automatically show the join menu

extern cvar_t *dm_choose;  // Enables or disables DM mode weapon/item choosing
extern cvar_t *dm_shield;  // Enables or disables DM shield

extern cvar_t *tourney_lca; // Enables or disabled lights camera action for tourney mode

// TNG:Freud - new spawning system
extern cvar_t *use_oldspawns; // Legacy old spawn system
// TNG:Freud - ghosts
extern cvar_t *use_ghosts;   // Enable or disable ghosts (stats)

// zucc from action
extern cvar_t *sv_shelloff; // Enable or disable shell ejection
extern cvar_t *shelllimit;  // Maximum number of shells on the ground
extern cvar_t *shelllife;   // Time before shells disappear
extern cvar_t *splatlimit;  // Maximum number of blood splat decals
extern cvar_t *bholelimit;  // Maximum number of bullet hole decals
extern cvar_t *splatlife;   // Time before blood splats disappear
extern cvar_t *bholelife;   // Time before bullet holes disappear

extern cvar_t *medkit_drop;
extern cvar_t *medkit_time;
extern cvar_t *medkit_instant;
extern cvar_t *medkit_max;
extern cvar_t *medkit_value;

// BEGIN AQ2 ETE
extern cvar_t *esp;  // Enable or disable Espionage mode
extern cvar_t *atl;  // Enable or disable Assassinate the Leader mode (do not set this manually)
extern cvar_t *etv;	 // Enable or disable Escort the VIP mode (do not set this manually)
extern cvar_t *esp_atl;  // Prefer ATL mode even if ETV mode is available
extern cvar_t *esp_punish;  // Enable or disable punishment for losing the around
extern cvar_t *esp_etv_halftime;  // Enable or disable halftime in ETV mode
extern cvar_t *esp_showleader;  // Enable or disable showing the leader (not implemented)
extern cvar_t *esp_showtarget;  // Enable or disable showing the target (not implemented)
extern cvar_t *esp_leaderequip; // Enable or disable leader equipment enhancements
extern cvar_t *esp_leaderenhance; // Enable or disable leader enhancements (fast bandage, medkits)
extern cvar_t *esp_enhancedslippers; // Enable or disable enhanced slippers (does not require Espionage)
extern cvar_t *esp_matchmode; // Enable or disable matchmode in Espionage mode
extern cvar_t *esp_respawn_uvtime; // Enable or disable respawn time in UV mode in seconds
extern cvar_t *esp_debug; // Enable or disable debug mode (very spammy)

// END AQ2 ETE

// 2023
extern cvar_t *use_killcounts;  // Adjust how kill streaks are counted
extern cvar_t *am; // Enable or disable Attract Mode (ltk bots)
extern cvar_t *zoom_comp;  // Enable or disable zoom compensation
extern cvar_t *item_kit_mode;  // Enable or disable item kit mode
extern cvar_t *gun_dualmk23_enhance; // Enable or disable enhanced dual mk23s (laser + silencer)
extern cvar_t *printrules; // Enable or disable printing of rules
extern cvar_t *timedmsgs;  // Enable or disable timed messages
extern cvar_t *mm_captain_teamname; // Set the team name to the captain's name in matchmode
extern cvar_t *sv_killgib; // Enable or disable gibbing on kill command

// 2024
extern cvar_t *warmup_unready;
// cURL integration
extern cvar_t *sv_curl_enable;
extern cvar_t *sv_discord_announce_enable;
extern cvar_t *sv_curl_stat_enable;
extern cvar_t *sv_aws_access_key;
extern cvar_t *sv_aws_secret_key;
extern cvar_t *sv_curl_discord_info_url;
extern cvar_t *sv_curl_discord_pickup_url;
extern cvar_t *server_ip;
extern cvar_t *server_port;
extern cvar_t *sv_last_announce_interval;
extern cvar_t *sv_last_announce_time;
extern cvar_t *msgflags;
extern cvar_t *use_pickup;
//end cUrl integration

extern cvar_t *training_mode; // Sets training mode vars
extern cvar_t *g_highscores_dir; // Sets the highscores directory
extern cvar_t *g_highscores_countbots; // Toggles if we save highscores achieved by bots
extern cvar_t *lca_grenade; // Allows grenade pin pulling during LCA
extern cvar_t *breakableglass; // Moved from cgf_sfx_glass, enables breakable glass (0,1,2)
extern cvar_t *glassfragmentlimit; // Moved from cgf_sfx_glass, sets glass fragment limit

#if AQTION_EXTENSION
extern int (*engine_Client_GetVersion)(edict_t *ent);
extern int (*engine_Client_GetProtocol)(edict_t *ent);

int Client_GetVersion(edict_t *ent);
int Client_GetProtocol(edict_t *ent);

extern void(*engine_Ghud_ClearForClient)(edict_t *ent);
extern int(*engine_Ghud_NewElement)(edict_t *ent, int type);
extern void(*engine_Ghud_RemoveElement)(edict_t *ent, int i);
extern void(*engine_Ghud_SetFlags)(edict_t *ent, int i, int val);
extern void(*engine_Ghud_SetInt)(edict_t *ent, int i, int val);
extern void(*engine_Ghud_SetText)(edict_t *ent, int i, char *text);
extern void(*engine_Ghud_SetPosition)(edict_t *ent, int i, int x, int y, int z);
extern void(*engine_Ghud_SetAnchor)(edict_t *ent, int i, float x, float y);
extern void(*engine_Ghud_SetColor)(edict_t *ent, int i, int r, int g, int b, int a);
extern void(*engine_Ghud_SetSize)(edict_t *ent, int i, int x, int y);

void  Ghud_ClearForClient(edict_t *ent);
int   Ghud_NewElement(edict_t *ent, int type);
void  Ghud_RemoveElement(edict_t *ent, int i);
void  Ghud_SetFlags(edict_t *ent, int i, int val);
void  Ghud_SetInt(edict_t *ent, int i, int val);
void  Ghud_SetText(edict_t *ent, int i, char *text);
void  Ghud_SetPosition(edict_t *ent, int i, int x, int y);
void  Ghud_SetPosition3D(edict_t *ent, int i, int x, int y, int z);
void  Ghud_SetAnchor(edict_t *ent, int i, float x, float y);
void  Ghud_SetColor(edict_t *ent, int i, int r, int g, int b, int a);
void  Ghud_SetSize(edict_t *ent, int i, int x, int y);

int   Ghud_AddIcon(edict_t *ent, int x, int y, int image, int sizex, int sizey);
int   Ghud_AddText(edict_t *ent, int x, int y, char *text);
void  Ghud_SetTextFlags(edict_t *ent, int i, int uiflags);
int   Ghud_AddNumber(edict_t *ent, int x, int y, int value);


extern void(*engine_CvarSync_Set)(int index, const char *name, const char *val);
void  CvarSync_Set(int index, const char *name, const char *val);
#endif

//botlib
extern bsp_t* (*SV_BSP)(void);
extern nav_t* (*CS_NAV)(void);
extern debug_draw_t* (*CS_DebugDraw)(void);
extern void (*SV_BotUpdateInfo)(char* name, int ping, int score);
extern void (*SV_BotConnect)(char* name);
extern void (*SV_BotDisconnect)(char* name);
extern void (*SV_BotClearClients)(void);


// 2022
extern cvar_t *sv_limp_highping;
extern cvar_t *server_id; // Unique server_id
extern cvar_t *stat_logs; // Enables/disables logging of stats
extern cvar_t *mapvote_next_limit; // Time left that disables map voting
extern cvar_t *stat_apikey; // Stats URL key
extern cvar_t *stat_url; // Stats URL endpoint
extern cvar_t *server_announce_url; // Server announce URL endpoint
extern cvar_t *g_spawn_items; // Enables item spawning in GS_WEAPONCHOOSE games
extern cvar_t *gm; // Gamemode
extern cvar_t *gmf; // Gamemodeflags
extern cvar_t *sv_idleremove; // Remove idlers

#if AQTION_EXTENSION
extern cvar_t *use_newirvision;		// enable new irvision (only highlight baddies)
extern cvar_t *use_indicators;		// enable/allow indicators
extern cvar_t *use_xerp;			// allow clients to use cl_xerp
#endif

// Discord SDK integration with Q2Pro
extern cvar_t *cl_discord;
extern cvar_t *cl_discord_id;
extern cvar_t *cl_discord_discriminator;
extern cvar_t *cl_discord_username;
extern cvar_t *cl_discord_avatar;

#define world   (&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN              0x00000001
#define ITEM_NO_TOUCH                   0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM                    0x00010000
#define DROPPED_PLAYER_ITEM             0x00020000
#define ITEM_TARGETS_USED               0x00040000

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP           1
#define FFL_NOSPAWN             2

typedef enum
{
  F_INT,
  F_FLOAT,
  F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
  F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
  F_VECTOR,
  F_ANGLEHACK,
  F_EDICT,			// index on disk, pointer in memory
  F_ITEM,			// index on disk, pointer in memory
  F_CLIENT,			// index on disk, pointer in memory
  F_FUNCTION,
  F_MMOVE,
  F_IGNORE
}
fieldtype_t;

typedef struct
{
  char *name;
  int ofs;
  fieldtype_t type;
  int flags;
}
field_t;


extern field_t fields[];
extern gitem_t itemlist[];

//
// g_cmds.c
//
qboolean FloodCheck(edict_t *ent);
void	Cmd_Help_f(edict_t *ent);
void	Cmd_Score_f(edict_t *ent);
void	Cmd_Inven_f(edict_t *ent);
edict_t *LookupPlayer(edict_t *ent, const char *text, qboolean checkNUM, qboolean checkNick);

//
// g_items.c
//
void PrecacheItem (gitem_t * it);
void InitItems (void);
void SetItemNames (void);
gitem_t *FindItem (char *pickup_name);
gitem_t *FindItemByClassname (char *classname);
gitem_t *FindItemByNum (int num);
#define ITEM_INDEX(x) ((x)-itemlist)
#define INV_AMMO(ent, num) ((ent)->client->inventory[items[(num)].index])
#define GET_ITEM(num) (&itemlist[items[(num)].index])
edict_t *Drop_Item (edict_t * ent, gitem_t * item);
void SetRespawn (edict_t * ent, float delay);
void ChangeWeapon (edict_t * ent);
void PrecacheItems( void );
void SpawnItem (edict_t * ent, gitem_t * item);
void Think_Weapon (edict_t * ent);
void AddItem(edict_t *ent, gitem_t *item);
qboolean Add_Ammo (edict_t * ent, gitem_t * item, int count);
void Touch_Item (edict_t * ent, edict_t * other, cplane_t * plane,
		 csurface_t * surf);

//
// g_utils.c
//
qboolean KillBox (edict_t *ent);
void	G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find(edict_t *from, ptrdiff_t fieldofs, char *match);
edict_t *findradius(edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget(char *targetname);
void	G_UseTargets(edict_t *ent, edict_t * activator);
void	G_SetMovedir(vec3_t angles, vec3_t movedir);

void	G_InitEdict(edict_t *e);
edict_t *G_Spawn(void);
edict_t *G_Spawn_Decal( void );
void	G_FreeEdict(edict_t *e);

void	G_TouchTriggers(edict_t *ent);
void	G_TouchSolids(edict_t *ent);

size_t  G_HighlightStr(char *dst, const char *src, size_t size);

char	*G_CopyString(char *in);
qboolean visible(edict_t *self, edict_t *other, int mask);

#ifndef NO_BOTS
qboolean ai_visible( edict_t *self, edict_t *other );
qboolean infront( edict_t *self, edict_t *other );
#endif
void disablecvar(cvar_t *cvar, char *msg);
int eztimer(int seconds);
float sigmoid(float x);
edict_t* FindEdictByClient(gclient_t* client);

// Re-enabled for bots
float *tv (float x, float y, float z);
char *vtos (const vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);

// g_trigger.c
void hurt_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t * ent1, edict_t * ent2);
qboolean CanDamage (edict_t * targ, edict_t * inflictor);
void T_Damage (edict_t * targ, edict_t * inflictor, edict_t * attacker,
	       const vec3_t dir, const vec3_t point, const vec3_t normal, int damage,
	       int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t * inflictor, edict_t * attacker, float damage,
		     edict_t * ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS                   BIT(0)	// damage was indirect
#define DAMAGE_NO_ARMOR                 BIT(1)	// armour does not protect from this damage
#define DAMAGE_ENERGY                   BIT(2)	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK             BIT(3)	// do not affect velocity, just view angles
#define DAMAGE_BULLET                   BIT(4)	// damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION            BIT(5)	// armor, shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD                  300
#define DEFAULT_BULLET_VSPREAD                  500
#define DEFAULT_SHOTGUN_HSPREAD                 1000
#define DEFAULT_SHOTGUN_VSPREAD                 500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT        12
#define DEFAULT_SHOTGUN_COUNT                   12
#define DEFAULT_SSHOTGUN_COUNT                  20

//rekkie -- DEV_1 -- s
//
// g_func.c
//
void door_use(edict_t* self, edict_t* other, edict_t* activator);

// from a_cmds.c
void _SetSniper(edict_t * ent, int zoom);
//rekkie -- DEV_1 -- e

//
// g_misc.c
//
void ThrowHead (edict_t * self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t * self, int damage);
void ThrowGib (edict_t * self, char *gibname, int damage, int type);
void BecomeExplosion1 (edict_t * self);
void SP_misc_teleporter_dest(edict_t* ent);

//
// g_weapon.c
//
void ThrowDebris (edict_t * self, char *modelname, float speed,
		  vec3_t origin);
void fire_bullet (edict_t * self, vec3_t start, vec3_t aimdir, int damage,
		  int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t * self, vec3_t start, vec3_t aimdir, int damage,
		   int kick, int hspread, int vspread, int count, int mod);
//SLIC2 changed argument name hyper to hyperb
void fire_blaster (edict_t * self, vec3_t start, vec3_t aimdir, int damage,
		   int speed, int effect, qboolean hyperb);
void fire_grenade2 (edict_t * self, vec3_t start, vec3_t aimdir, int damage,
	int speed, int timer, float damage_radius,
		    qboolean held);

void kick_attack(edict_t *ent);
void punch_attack(edict_t *ent);
int knife_attack(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void knife_throw(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void knife_touch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf);
void fire_bullet_sparks(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_bullet_sniper(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void setFFState(edict_t* ent);

//
// g_client.c
//
void respawn (edict_t * ent);
void BeginIntermission (edict_t * targ);
void PutClientInServer (edict_t * ent);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t * ent);

//
// g_ext.c
//
#if AQTION_EXTENSION
void G_InitExtEntrypoints(void);
void* G_FetchGameExtension(char *name);
#endif

//
// g_player.c
//
void player_pain (edict_t * self, edict_t * other, float kick, int damage);
void player_die (edict_t * self, edict_t * inflictor, edict_t * attacker,
		 int damage, vec3_t point);

//
// g_svcmds.c
//
void ServerCommand (void);
qboolean SV_FilterPacket (char *from, int *temp);
qboolean SV_FilterSBPacket(char* from, int* temp); //rekkie -- silence ban
void Kick_Client (edict_t * ent);
qboolean Ban_TeamKiller (edict_t * ent, int rounds);

//
// p_view.c
//
void ClientEndServerFrame (edict_t * ent);
void SetAnimation( edict_t *ent, int frame, int anim_end, int anim_priority );
qboolean OnLadder( edict_t *ent );

//
// p_hud.c
//
void MoveClientToIntermission (edict_t * client);
void MoveClientToPOI(edict_t *ent, edict_t *poi);
void G_SetStats (edict_t * ent);
void G_CheckChaseStats (edict_t * ent);
void ValidateSelectedItem (edict_t * ent);
void DeathmatchScoreboard(edict_t* ent);
void DeathmatchScoreboardMessage (edict_t * client, edict_t * killer);

//
// g_pweapon.c
//
void PlayerNoise (edict_t * who, vec3_t where, int type);

//
// g_phys.c
//
void G_RunEntity (edict_t * ent);
edict_t *SV_TestEntityPosition (edict_t * ent);

//
// g_main.c
//
void EndDMLevel (void);
void ExitLevel (void);
edict_t *ChooseRandomPlayer(int teamNum, qboolean allowBot);

//
// g_chase.c
//
void DisableChaseCam( edict_t *ent );
void UpdateChaseCam (edict_t * ent);
int ChaseTargetGone (edict_t * ent);
void NextChaseMode( edict_t *ent );
void SetChase( edict_t *ent, edict_t *target );
void ChaseNext (edict_t * ent);
void ChasePrev (edict_t * ent);
void GetChaseTarget (edict_t * ent);
void TeamplayChaseCam(edict_t *self, edict_t *attacker);
void EspionageChaseCam(edict_t *self, edict_t *attacker);


//
// g_spawn.c
//
void ChangePlayerSpawns(void);
void ED_CallSpawn( edict_t *ent );
char* ED_NewString(const char* string);
void G_UpdateSpectatorStatusbar( void );
void G_UpdatePlayerStatusbar( edict_t *ent, int force );
int Gamemodeflag(void);
int Gamemode(void);
#if USE_AQTION
#define GENERATE_UUID() generate_uuid()
void generate_uuid(void);
#else
#define GENERATE_UUID()
#endif
//
// p_client.c
//
edict_t* SelectRandomDeathmatchSpawnPoint(void);
edict_t* SelectFarthestDeathmatchSpawnPoint(void);
edict_t* SelectDeathmatchSpawnPoint(void);
float PlayersRangeFromSpot(edict_t* spot);
void ClientLegDamage(edict_t* ent);
void ClientFixLegs(edict_t *ent);
void ClientUserinfoChanged(edict_t* ent, char* userinfo);
void ClientDisconnect(edict_t* ent);
void CopyToBodyQue(edict_t* ent);
void Announce_Reward(edict_t *ent, int rewardType);

//p_weapon.c
void Weapon_Generic( edict_t * ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST,
	int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST,
	int FRAME_RELOAD_LAST, int FRAME_LASTRD_LAST,
	int *pause_frames, int *fire_frames,
	void( *fire ) (edict_t * ent) );
void PlayWeaponSound( edict_t *ent );

void P_ProjectSource(gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void weapon_grenade_fire(edict_t* ent, qboolean held);
void InitTookDamage(void);
void ProduceShotgunDamageReport(edict_t*);

//tng_stats.c
void G_RegisterScore(void);
int G_CalcRanks(gclient_t **ranks);
void G_LoadScores(void);

// Compiler macros for stat logging
#if USE_AQTION
#define STAT_BOT_CHECK() StatBotCheck()
void StatBotCheck(void);
#define LOG_KILL(ent, inflictor, attacker) LogKill(ent, inflictor, attacker)
void LogKill(edict_t *ent, edict_t *inflictor, edict_t *attacker);
#define LOG_WORLD_KILL(ent) LogWorldKill(ent)
void LogWorldKill(edict_t *ent);
#define LOG_CAPTURE(capturer) LogCapture(capturer)
void LogCapture(edict_t *capturer);
#define LOG_MATCH() LogMatch()
void LogMatch(void);
#define LOG_AWARD(ent, award) LogAward(ent, award)
void LogAward(edict_t *ent, int award);
#define LOG_END_MATCH_STATS() LogEndMatchStats()
void LogEndMatchStats(void);
#else
#define STAT_BOT_CHECK()
#define LOG_KILL(ent, inflictor, attacker)
#define LOG_WORLD_KILL(ent)
#define LOG_CAPTURE(capturer)
#define LOG_MATCH()
#define LOG_AWARD(ent, award)
#define LOG_END_MATCH_STATS()
#endif

//============================================================================

// client_t->anim_priority
#define ANIM_BASIC              0	// stand / run
#define ANIM_WAVE               1
#define ANIM_JUMP               2
#define ANIM_PAIN               3
#define ANIM_ATTACK             4
#define ANIM_DEATH              5
// in 3.20 there is #define ANIM_REVERSE 6    -FB
// zucc vwep - based on info from Hentai
#define ANIM_REVERSE            -1

#define MAX_SKINLEN				32
#define MAX_TEAMNAMELEN			32

#define MAX_GUNSTAT MOD_GRENADE //Max MOD to track

typedef enum {
	GENDER_MALE,
	GENDER_FEMALE,
	GENDER_NEUTRAL
} gender_t;

typedef enum {
	LAYOUT_NONE,
	LAYOUT_SCORES,
	LAYOUT_SCORES2,
	LAYOUT_MENU
} layout_t;

#define GENDER_STR( ent, he, she, it ) (((ent)->client->pers.gender == GENDER_MALE) ? he : (((ent)->client->pers.gender == GENDER_FEMALE) ? she : it))

typedef struct gunStats_s
{
	int shots;		//Number of shots
	int hits;		//Number of hits
	int headshots;	//Number of headshots
	int kills;		//Number of kills
	int damage;		//Damage dealt
} gunStats_t;

typedef struct lt_stats_s
{
    int frags;
    int deaths;
    int64_t damage;
} lt_stats_t;

// client data that stays across multiple level loads
typedef struct
{
	char userinfo[MAX_INFO_STRING];
	char netname[16];
	char ip[64];
	int hand;

	qboolean connected;		// a loadgame will leave valid entities that
	// just don't have a connection yet

	//rekkie -- surface data -- s
//#if DEBUG_DRAWING
	debug_draw_t* draw; // debug drawing functions
//#endif
	//rekkie -- surface data -- e

	qboolean silence_banned; //rekkie -- silence ban

	int admin;

	gender_t	gender;
	qboolean spectator;
	int firing_style;

	qboolean mvdspec;

	xmenu_t x_menu;

	gitem_t *chosenItem;		// item for teamplay
	gitem_t *chosenWeapon;	// weapon for teamplay

	int menu_shown;		// has the main menu been shown
	qboolean dm_selected;		// if dm weapon selection has been done once

	// Reki - added these options, controllable via userinfo cvar (Reki update 2/24/23, controllable via cvarsync as well)
	int limp_nopred;
	int spec_flags;
	qboolean antilag_optout;
#if AQTION_EXTENSION
	int cl_xerp;
	int cl_indicators;
#endif

	int mk23_mode;		// firing mode, semi or auto
	int mp5_mode;
	int m4_mode;
	int knife_mode;
	int grenade_mode;
	int hc_mode;
	int id;			// id command on or off
	int irvision;			// ir on or off (only matters if player has ir device, currently bandolier)

	ignorelist_t ignorelist;
	gitem_t *chosenItem2;		// Support for item kit mode

	#if USE_AQTION
	char steamid[24];
	char discordid[24];
	#endif

}
client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
  int enterframe;		// level.framenum the client entered the game

  int score;			// frags, etc

  vec3_t cmd_angles;		// angles sent over in the last command

  int sniper_mode;		//level of zoom

  int kills;			// real kills

  int deaths;			// deaths

  int damage_dealt;		// keep track of damage dealt by player to other players

  int team;			// team the player is on
  int subteam;

  int ctf_state;
  int ctf_caps;
  int ctf_capstreak;
  int ctf_lasthurtcarrier;
  int ctf_lastreturnedflag;
  int ctf_flagsince;
  int ctf_lastfraggedcarrier;

  int joined_team;		// last frame # at which the player joined a team
  int lastWave;			//last time used wave

  radio_t radio;
 
  int motd_refreshes;
  int last_motd_refresh;
  int gamemsg_refreshes;
  int last_gamemsg_refresh;
  edict_t *last_chase_target;	// last person they chased, to resume at the same place later...

  // Number of team kills this game
  int team_kills;
  int team_wounds;
  
  int idletime;
  int totalidletime;
  int tourneynumber;
  edict_t *kickvote;

  char *mapvote;		// pointer to map voted on (if any)
  char *cvote;			// pointer to config voted on (if any)
  qboolean scramblevote;	// want scramble

  int ignore_time;		// framenum when the player called ignore - to prevent spamming
	
  int stat_mode;    		// Automatical Send of statistics to client
  int stat_mode_intermission;

  int shotsTotal;					//Total number of shots
  int hitsTotal;					//Total number of hits
  int streakKills;					//Kills in a row
  int roundStreakKills;				//Kills in a row in that round
  int streakHS;						//Headshots in a Row
  int streakKillsHighest;			//Highest kills in a row
  int streakHSHighest;				//Highest headshots in a Row

  int hitsLocations[LOC_MAX];		//Number of hits for different locations
  gunStats_t gunstats[MOD_TOTAL]; //Number of shots/hits for different guns, adjusted to MOD_TOTAL to allow grenade, kick and punch stats
  int awardstats[AWARD_MAX];			//Number of impressive, excellent and accuracy awards
  //AQ2:TNG - Slicer: Video Checking and further Cheat cheking vars
  char vidref[16];
  char gldriver[16];
  float gllockpvs;
  float glmodulate;
  float glclear;
  float gldynamic;
  float glbrightness;
  qboolean checked;
  int checkframe[3];

  int				penalty;

  float jmp_highspeed;
  float jmp_falldmglast;
  vec3_t jmp_teleport_origin;
  vec3_t jmp_teleport_v_angle;
  qboolean jmp_teleport_ducked;

#if AQTION_EXTENSION
  int	hud_items[128];
  int	hud_type;
#endif

  int esp_state;
  int esp_caps;						// How many times a player has captured the case
  int esp_capstreak;				// As leader, how many caps in a row
  int esp_capstreakbest;			// Best cap streak
  int esp_leaderkillstreak;			// How many enemy leader kills a player has had in a row
  int esp_leaderkillstreakbest;		// Best leader kill streak
  int esp_leaderprotectcount;		// How many times a player has protected the leader
  int esp_leaderfragcount;			// How many times this player has fragged a leader
  int esp_capdefendercount;			// How many times a player has defended the capture point
  
  // timers
  int esp_lastprotectcap;			// Last time this player protected the capture point (framenum)
  int esp_lastprotectleader;		// Last time this player protected the leader (framenum)
  int esp_lasthurtleader;			// Last time this player hurt the leader (framenum)
  int esp_leadertime;				// Last time this player was leader (framenum) to prevent spamming
  int esp_respawn_sounds;			// 3: Played Lights, 2: Played Camera, 1: Played Action, 0: Play nothing
  qboolean is_volunteer;

  int medkit_award_time;

  //PaTMaN's jmod
  int toggle_lca;
  int toggles;

  // Domination stats
  int dom_caps;						// How many times a player captured a dom point
  int dom_capstreak;				// How many times a player captured a dom point in a row
  int dom_capstreakbest;			// Best cap streak for domination

  // Long term stats retreived from database
  lt_stats_t* lt_stats; // Long-term stats
}
client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
// 9b15 commit from upstream -- using player_state_old_t instead of player_state_new_t
struct gclient_s
{
	// known to server
	player_state_old_t	ps;		// communicated by server to clients
	int					ping;

	// known to compatible server
	int					clientNum;

	// Reki: cvar sync
#if AQTION_EXTENSION
	cvarsyncvalue_t cl_cvar[CVARSYNC_MAX];
#endif

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	layout_t	layout;		// set layout stat
	qboolean	showinventory;	// set layout stat

	pmenuhnd_t	menu;		// current menu

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	int			weapon_last_activity;

	gitem_t		*newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	int			damage_dealt;		// total damage dealt to other players (used for hit markers)

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	vec3_t		kick_angles;		// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;	// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;
	qboolean	ladder, old_ladder;

	int			next_drown_framenum;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;
	int			anim_started;

	// powerup timers
	int			quad_framenum;
	int			invincible_framenum;
	int			breather_framenum;
	int			enviro_framenum;

	qboolean	grenade_blew_up;
	int			grenade_framenum;
	int			silencer_shots;
	int			weapon_sound;

	int			pickup_msg_framenum;
	int			respawn_framenum;		// can respawn when time > this
  
	edict_t		*chase_target;
	int			chase_mode;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_pistolmags;
	int			max_shells;
	int			max_mp5mags;
	int			max_m4mags;
	int			max_sniper_rnds;

	int			mk23_max;
	int			mk23_rds;

	int			dual_max;
	int			dual_rds;
	int			shot_max;
	int			shot_rds;
	int			sniper_max;
	int			sniper_rds;
	int			mp5_max;
	int			mp5_rds;
	int			m4_max;
	int			m4_rds;
	int			cannon_max;
	int			cannon_rds;
	int			knife_max;
	int			grenade_max;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

	int			curr_weap;		// uses NAME_NUM values

	int			fired;			// keep track of semi auto
	int			burst;			// remember if player is bursting or not
	int			fast_reload;	// for shotgun/sniper rifle
	int			idle_weapon;	// how many frames to keep our weapon idle
	int			desired_fov;	// what fov does the player want? (via zooming)
	int			desired_zoom;	// either 0, 1, 2, 4 or 6. This is set to 0 if no zooming shall be done, and is set to 0 after zooming is done.

	int			unique_weapon_total;
	int			unique_item_total;
	int			drop_knife;
	int			knife_sound;		// we attack several times when slashing but only want 1 sound

	int			punch_framenum;
	qboolean	punch_desired;	//controlled in ClientThink

	int			reload_attempts;
	int			weapon_attempts;

	qboolean	autoreloading;	//used for dual -> mk23 change with reloading

	int			took_damage;		//Took damage from multihit weapons

	int			no_sniper_display;
  
	int			bandaging;
	int			bandage_stopped;
	qboolean	weapon_after_bandage_warned;	// to fix message bug when calling weapon while bandaging

	int			leg_damage;
	int			leg_dam_count;
	int			leg_noise;
	int			leghits;

	int			bleeding;			//remaining points to bleed away
	int			bleed_remain;
	vec3_t		bleedloc_offset;	// location of bleeding (from origin)
	int			bleeddelay;			// how long until we bleed again

	int			doortoggle;			// set by player with opendoor command

	edict_t		*attacker;		// keep track of the last person to hit us
	int			attacker_mod;	// and how they hit us
	int			attacker_loc;	// location of the hit

	int			push_timeout;	// timeout for how long an attacker will get fall death credit

	int			jumping;

	antilag_t	antilag_state;

	// Number of teammate woundings this game and a "before attack" tracker
	int			team_wounds_before;
	int			ff_warning;

	int			radio_num_kills;

	int			last_damaged_part;
	char		last_damaged_players[256];
	edict_t		*last_killed_target[MAX_LAST_KILLED];

	int			uvTime;
  
	qboolean	team_force;		// are we forcing a team change

	edict_t		*lasersight; // laser
	edict_t		*flashlight; // Flashlight

	int			medkit;

	edict_t		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	int			ctf_grapplereleaseframe;	// frame of grapple release

#if AQTION_EXTENSION
	//AQTION - Reki: Teammate indicators
	edict_t		*arrow;
#endif

	// used for extrapolation
	usercmd_t	cmd_last;

	// visiblity mask
	unsigned int dimension_observe;
};


//rekkie -- s
#ifndef NO_BOTS

typedef enum {
	HIGHLIGHTED_NODE_NONE,		// No interaction
	HIGHLIGHTED_NODE_SELECT,	// Node selection
	HIGHLIGHTED_NODE_SELECT_SMART,	// Smart node selection
	HIGHLIGHTED_NODE_ADD,		// Add nodes (either by walking around or mouse1 '+attack' on the ground)
	HIGHLIGHTED_NODE_FLOODFILL,	// Flood fills the area with nodes (mouse1 '+attack')
	HIGHLIGHTED_NODE_LINK,		// Add/del link between prev and curr highlighted nodes
	HIGHLIGHTED_NODE_MOVE,		// Move nodes
	HIGHLIGHTED_NODE_TYPE,		// Edit node types: NODE_MOVE, ladder, jump, etc (mouse1 '+attack' to change type)
	HIGHLIGHTED_NODE_LINKTYPE,	// Edit node-to-node link type: crouch, jumppad, boxjump, etc
	HIGHLIGHTED_NODE_DEL		// Delete nodes
} highlighted_type_t;
typedef struct walknodes_s
{
	int last_type;		// Previous node type
	int touched_node; // Last nodes touched
	vec3_t last_ground_loc; // Location the player last touched the ground
	vec3_t last_ground_normal; // Normal the player last touched
	int last_ground_touch;

	qboolean enabled; // If we're editing nodes or not
	highlighted_type_t highlighted_node_type; // Link, move, ...  highlighted_type_t
	int highlighted_node;	// The node currently being highlighted
	int prev_highlighted_node;	// The node previously highlighted
	int highlighted_time; // Highlight timer
	int highlighted_counter;

	
	vec3_t selection_start; // Starting point of the selection square
	vec3_t selection_end; // Ending point of the selection square
	float selection_max; // Sets the highest selection point
	float selection_min; // Sets the lowest selection point
	int selection_node_first; // First selected node (used to get first area selected)
	int selection_node_count; // Selected nodes count
	int selected_node_count_prev; // Prevous selected nodes count
	int selection_nodes[MAX_NAV_AREAS_NODES]; // Selected nodes are stored here
	int selection_area; // Sets the area num
	unsigned int selection_area_color; // Sets the area color
	qboolean selection_area_used; // Checks if area num was used

} walknodes_t;
// Hold all the bot information in a single struct
#define MAX_NODELIST 512

typedef enum {
	BOT_TYPE_NONE,		// No bot type
	BOT_TYPE_LTK,		// LTK - OG bots
	BOT_TYPE_BOTLIB,	// BOTLIB -- New bots
} bot_type_t;

// Most float values here are between -1 and 1
typedef struct bot_personality_s
{
	// These are +1 because we're ignoring the first index [0]
	// So that MK23_NUM (1) stays at 1 here as well
	float weapon_prefs[WEAPON_COUNT + 1];  	//-1 = Will never choose, 1 = Will always choose
	float item_prefs[ITEM_COUNT +1];      	//-1 = Will never choose, 1 = Will always choose

	float map_prefs;						//-1 = Hate, 0 = Neutral, 1 = Love
	float combat_demeanor;					//-1 = Timid | 1 = Aggressive
	float chat_demeanor;					//-1 = Quiet | 1 = Chatty
	int leave_percent; 						// Percentage calculated that the bot will leave the map.  Recalculated/increases every time the bot dies.

	char skin_pref;			// Skin preference, if DM mode
	int pId;                                // Personality id (used as an index)
	qboolean isActive;                      // Determines if bot is active in game or not (avoid dupes)

} bot_personality_t;

typedef enum bot_skill_types
{
	BOT_SKILL_OVERALL,
	BOT_SKILL_AIM,
	BOT_SKILL_REACTION,
	BOT_SKILL_MOVEMENT,
	BOT_SKILL_TEAMWORK,
	BOT_SKILL_COMMUNICATION,
	BOT_SKILL_WEAPON,
	BOT_SKILL_MAP,
} bot_skill_types_t;

typedef struct bot_skill_s
{
	float overall;	// Overall skill
	float aim;		// Aim skill
	float reaction;	// Reaction skill
	float movement;	// Movement skill
	float teamwork;	// Teamwork skill
	float communication;	// Communication skill
	float weapon_skill[WEAPON_COUNT];	// Weapon skill
	float map_skill;	// Map skill
} bot_skill_t;

typedef struct bot_s
{
	int bot_type;

	int state; // Bot states: STATE_MOVE, STATE_WANDER, etc

	int pause_time; // Pause bot logic, this is a countdown timer (useful for debugging)

	// Nodes
	int goal_node; // The node the bot should try to reach
	int current_node; // The current node the bot (or player) is at
	int next_node; // The next node (if any) the bot needs to reach
	int prev_node; // The previous node (if any) the bot came from

	// Area NAV
	int start_area; // Starting area
	int current_area; // Current area we're in
	int goal_area; // Goal area to reach
	int next_area; // Next area to reach
	int next_area_nodes[MAX_NAV_AREAS_EDGES]; // Edge-to-edge nodes
	int next_area_nodes_counter; // Keep track of next node array num
	int next_area_node; // Next node to reach
	int next_area_counter; // Keep track of next area array num
	int path_taken; // If there are multiple paths, select one and save it here
	int area_heatmap[MAX_NAV_AREAS]; // Track how often an area gets used
	
	vec3_t tmp_position; // 
	vec3_t last_position; // Remember last position

	int bot_baseline_ping;		// Set the baseline ping for this bot  //rekkie -- Fake Bot Client
	int bot_ping;				// Set the fake ping using its baseline  //rekkie -- Fake Bot Client

	// Bot inputs
	bot_input_t bi;
	qboolean jumppad; // Jump was conducted
	int node_jump_from; // Node bot jumped from
	int node_jump_to; // Node bot jumped to
	float jumppad_last_time; // Time until jump can be used again
	float jumppad_land_time; // Time until landing measures can take place
	trace_t touch_ground; // If touching the ground. (touch_ground.fraction < 1.0) is true, (touch_ground.fraction == 1.0) is false
	qboolean touching_ladder; // If bot is touching a ladder

	// Stuck prevention
	qboolean stuck; // If the bot is stuck true/false
	int stuck_tries; // How many times the bot tried to unstick
	int stuck_wander_time; // Wander until counter is zero
	vec3_t stuck_random_dir;
	int stuck_last_negate; // Last time we negated the bot's velocity to stop it from sliding off a steep slope
	qboolean stuck_rnd_dir; // If the bot is trying a random direction to get unstuck
	int stuck_node; // The node the bot trying to head to if stuck
	vec3_t stuck_pos; // Current pos we're stuck at
	vec3_t stuck_old_pos; // Previous pos we were stuck at

	// Nodes
	int node_list[MAX_NODELIST]; // A copy of the pathList each time it's created - useful to see any nodes the list contained
	int node_list_count; // Total nodes added to the current node_list
	int node_list_current; // The current node
	qboolean node_random_path; // If the bot is taking a direct or random path to goal
	//
	int node_prev; // The previous node
	int node_travel_time; // The time taken to reach the next node
	int highlighted_node;
	vec3_t bot_walk_dir; // The current direction from the bot to the next node
	//
	qboolean node_poi_holding; // If the bot is currently holding
	float node_poi_time; // How long the bot should stay at a point of interest node
	int node_poi_lookat_node; // Which node the bot is looking at
	float node_poi_look_time; // How long to look at a lookat node

	// Weapons
	int last_sniper_zoom_time; // Delay time between sniper zoom attempts
	int last_weapon_change_time; // Delay time between changing weapons
	int last_weapon_reload_time; // Delay time between reloading weapons

	// Skill
	bot_skill_t skill; // Variable bot skill levels

	// Items
	edict_t *get_item; // The current item the bot wants and is located next or or inside of a node

	// CTF
	bot_ctf_state_t bot_ctf_state; // Get flag, retrieve flag, intercept flag carrier, etc.
	float ctf_support_time; // Time between ally support checks

	// Espionage
	bot_esp_state_t bot_esp_state;

	// Adding walk nodes
	walknodes_t walknode; // Holds all the walk node data

	// Enemy & Damage
	edict_t *old_enemy;				// Previous enemy, if any
	qboolean see_enemies;			// If the bot has line of sight to an enemy
	float reaction_time;			// How quickly the bot reacts to an enemy in sight
	qboolean enemy_in_xhair;		// If bot's crosshair is lined up with enemy
	float enemy_dist;				// Distance to enemy
	float enemy_height_diff;		// Difference in height between bot and enemy
	int enemies_num;				// Number of enemies in sight
	int enemies[MAX_CLIENTS];		// Store the enemies in sight
	int enemies_weight[MAX_CLIENTS];// Store the enemies by weight
	float enemy_seen_time;			// Last time bot sighted the enemy
	vec3_t enemy_seen_loc;			// Last seen location
	float bot_bandage_delay_time;	// Delay before bandaging
	int enemy_chase_time;			// Delay before chasing enemy attempts

	// Allies
	int allies_num;					// Number of allies in sight
	int allies[MAX_CLIENTS];		// Store the allies in sight
	qboolean ff_allies_after_rnd;	// If the bot is allowed to kill friendlies after a TP round ends


	// Radio
	qboolean radioTeamReportedIn;	// Request team report in
	int radioLastSawAllyTime;		// Last time since bot saw an ally
	int radioTeamReportedInDelay;	// Add a delay before triggering the call to request a "team report in" call
	//
	qboolean radioReportIn;			// Flag if bot needs to respond to a "team report in" request
	int radioReportInTime;			// Time a "team report in" request
	int radioReportInDelay;			// Delay when responding to a "team report in" request
	//
	qboolean radioBandaging;		// Flag true to get the bot to radio report
	//
	char radioLastHumanMsg[48];		// Get the lastest radio call from a real player on our team
	//
	qboolean radioReportKills;		// Flag if the bot reports its kills in radio and chat
	//
	int lastChatTime;				// Last time the bot chatted

	bot_personality_t personality;	// Personality struct

} bot_t;
#endif
//rekkie -- e

struct edict_s
{
	entity_state_t s;
	struct gclient_s *client;	// NULL if not a player
								// the server expects the first part
								// of gclient_s to be a player_state_t
								// but the rest of it is opaque

	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;			// linked to a division node or leaf

	int			num_clusters;	// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;		// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;

	entity_state_extension_t    x;
	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;		// sv.time when the object was freed

	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down

	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	int			air_finished_framenum;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	int			nextthink;
	void		(*prethink)(edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	int			touch_debounce_framenum;	// are all these legit?  do we need more/less of them?
	int			pain_debounce_framenum;
	int			damage_debounce_framenum;
	int			fly_sound_debounce_framenum;	//move to clientinfo
	float		last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;

	int			watertype;
	int			waterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;		// for bonus items

	// common data blocks
	moveinfo_t	moveinfo;

	// hack for proper s.old_origin updates
	vec3_t		old_origin;

#ifndef NO_FPS
	float		z_history[ MAX_FRAMEDIV ];
	//short		z_pmove;
	int			z_history_framenum;
	int			z_history_count;
#endif

	// visibility mask
	unsigned int dimension_visible;

	// action
	qboolean	splatted;
	int			classnum;
	int			typeNum;

	float		friction; // If the ent has friction, default 1.0  //rekkie -- DEV_1 -- s

	//rekkie - Quake3 -- s
#ifndef NO_BOTS
	bot_t bot;
	//rekkie -- surface data -- s
	nav_t* nav;
	//rekkie -- surface data -- e

	//rekkie -- DEV_1 -- s
	botlib_sll_t pathList;	// Single linked list of node numbers
	//rekkie -- DEV_1 -- e

	int current_link; // current link -- //rekkie

	vec3_t nearest_path_point; // between curr -> next node, this is the origin the bot is closest to along that path

	float just_spawned_timeout;	// Delay before first moving after spawning
	qboolean just_spawned;		// If the bot has just spawned
	qboolean just_spawned_go;	// When the bot should move after spawning
	//
	int last_touched_ground;	// Last time player/bot touched ground
	int last_jumppad_node;		// Last node we conducted a jumppad from
	float next_node_distance;	// Keep trace of distance
	int prev_next_node;
	byte strafe_jumps;			// Keep track of how many strafe jumps were made
	//
	//int num_nextnodes;			// How many nodes in the nodepath
	//int *nextnode;				// Node node list
	//
	//int tmp_curr_node;
	//int tmp_prev_node;
	//int prev_node;
	vec3_t prev_velocity;
	char node_name[32];			// Node name consisting of node num + node type
	int node_num;				// Node number is here so it can be linked back to nodes[nodenum].nodenum
	//
	float highest_velocity;
	//
	int show_node_links;		// Display node links from the node we're looking at
	int show_node_links_time;	// Time to display links

#endif
	//rekkie - Quake3 -- e

#ifndef NO_BOTS
	qboolean velocity_clear;
	vec3_t velocity_peak; // record the player peak velocity
	float speed_peak;

	int old_health;

	int recheck_timeout;
	int jumphack_timeout;

	qboolean is_bot; 
	qboolean is_jumping; 
	qboolean is_triggering; 
	 
	// For movement 
	vec3_t move_vector;  
	float next_move_time; 
	float wander_timeout; 
	float suicide_timeout; 
 
//AQ2 ADD 
	// Door and pause time stuff. 
	float	last_door_time;	// Used to open doors without immediately closing them again! 
	float	teamPauseTime;	// To stop the centipede effect and seperate the team out a little 
	qboolean	teamReportedIn;	// Have we reported in yet? 
	float	lastRadioTime;	// Don't use the radio too often 
	// Path to follow

	float	antLastCallTime;	// Check for calling complex pathsearcher 
	// Who killed me? 
	edict_t	*lastkilledby;	// Set in ClientObituary... 
	int grenadewait; // Raptor007: Moved here from player_state_t.
//AQ2 END 
 
	// For node code

	int current_node; // current node
	int next_node; // the node that will take us one step closer to our goal 
	int goal_node; // current goal node
	int node_timeout; 
	int last_node; 
	int tries;


	 
	// AI related stuff 
	int weaponchoice; 
	int equipchoice; 
	float	fLastZoomTime;	// Time we last changed sniper zoom mode 
 
	// Enemy related 
	qboolean	killchat;	// Have we reported an enemy death and taunted him 
	vec3_t		lastSeen; 
	qboolean	cansee; 
	float react;            // How long enemy has been in view.
 
	// States 
	int state;	//ACE only 
	int botState; 
	int nextState; 
	int secondaryState; 
 
	// Movement 
	int	bot_strafe; 
	int bot_speed; 
	qboolean	bCrawl; 
	qboolean	bLastJump; 
	vec3_t	lastPosition; 
	qboolean	nameused[NUMNAMES][NUMNAMES];
	qboolean	newnameused[AQ2WTEAMSIZE];
	#if AQTION_EXTENSION
	//AQTION - Reki: Entity indicators
	edict_t		*obj_arrow;
	#endif

#endif
};

typedef struct
{
	char netname[16];
	char ip[64];
	int enterframe;
	int disconnect_frame;
	int score;
	int kills;
	int deaths;
	int damage_dealt;
	int ctf_caps;
	int shotsTotal;
	int hitsTotal;
	int hitsLocations[LOC_MAX];
	gunStats_t gunstats[MOD_TOTAL];
	int awardstats[AWARD_MAX];
	int team;
	gitem_t *weapon;
	gitem_t *item;
	// Extended stats
	#if USE_AQTION
	char steamid[24];
	char discordid[24];
	#endif
	int ctf_capstreak;
	int team_kills;
	int streakKillsHighest;
	int streakHSHighest;

}
gghost_t;

#define MAX_GHOSTS 64 // If we need more ghosts than this, let's cross that bridge
extern gghost_t ghost_players[MAX_GHOSTS];
extern int num_ghost_players;

void CreateGhost (edict_t * ent);

//zucc
void LaserSightThink (edict_t * self);
void SP_LaserSight (edict_t * self, gitem_t * item);
void Cmd_Reload_f (edict_t * ent);
void Cmd_New_Reload_f (edict_t * ent);
void Cmd_New_Weapon_f (edict_t * ent);
void Cmd_Weapon_f (edict_t * ent);
void Cmd_OpenDoor_f (edict_t * ent);
void Cmd_Bandage_f (edict_t * ent);
void Cmd_ID_f (edict_t * ent);
void Cmd_IR_f (edict_t * ent);
int GetWeaponNumFromArg(const char *s);
int GetItemNumFromArg(const char *s);
void Cmd_Choose_f (edict_t * ent);
void Cmd_TKOk (edict_t * ent);	// AQ:TNG - JBravo adding tkok
void Cmd_FF_f( edict_t *ent );
void Cmd_Time (edict_t * ent);	// AQ:TNG - JBravo adding time
void Cmd_Roundtimeleft_f(edict_t *ent); // AQ:TNG - DW added roundtimeleft
void Cmd_Noclip_f(edict_t *ent);
void DropSpecialWeapon (edict_t * ent);
void ReadySpecialWeapon (edict_t * ent);
void DropSpecialItem (edict_t * ent);
void Bandage (edict_t * ent);
void ShowGun (edict_t * ent);	// hentai's vwep function added by zucc
void FL_think (edict_t * self);	// TNG Flashlight
void FL_make (edict_t * self);	// TNG Flashlight
const char* PrintWeaponName( int weapon );
const char* PrintItemName( int item );

// spec functions
void SetupSpecSpawn (void);
void RespawnSpec (edict_t * ent);
void Drop_Spec (edict_t * ent, gitem_t * item);
void SpecThink (edict_t * spec);
void DeadDropSpec (edict_t * ent);

void SpecialWeaponRespawnTimer (edict_t * ent);	// p_weapons.c
void ThinkSpecWeap (edict_t * ent);
void DropExtraSpecial (edict_t * ent);
void TransparentListSet (solid_t solid_type);

void killPlayer(edict_t *ent, qboolean suicidePunish);
int TP_GetTeamFromArg(const char *name);
int G_SortedClients( gclient_t **sortedList );
int G_NotSortedClients( gclient_t **sortedList );
void A_ScoreboardMessage( edict_t * ent, edict_t * killer );

//local to g_combat but needed in p_view
void SpawnDamage (int type, const vec3_t origin, const vec3_t normal, int damage);
void Killed (edict_t * targ, edict_t * inflictor, edict_t * attacker,
	     int damage, vec3_t point);

void Add_Frag(edict_t * ent, int mod);
void Subtract_Frag (edict_t * ent);
void Add_Death( edict_t *ent, qboolean end_streak );

void PrintDeathMessage(char *msg, edict_t * gibee);

void SetIDView (edict_t * ent);

qboolean Pickup_Special (edict_t * ent, edict_t * other);

// action function
edict_t *FindEdictByClassnum (char *classname, int classnum);

void EjectBlooder (edict_t * self, vec3_t start, vec3_t veloc);
void EjectShell (edict_t * self, vec3_t start, int toggle);
void AddDecal (edict_t * self, trace_t * tr);
void AddSplat (edict_t * self, vec3_t point, trace_t * tr);

//AQ2:TNG - Slicer New location support
#define MAX_LOCATIONS_IN_BASE		256	// Max amount of locations
// location structure
typedef struct
{
  int x;
  int y;
  int z;
  int rx;
  int ry;
  int rz;
  char desc[128];
}
placedata_t;

// Externals for accessing location structures
extern int ml_count;
extern placedata_t locationbase[];
extern char ml_creator[101];
//AQ2:TNG END

void Cmd_Ghost_f (edict_t * ent);
void Cmd_AutoRecord_f(edict_t * ent);

typedef struct team_s
{
	char name[24];
	char skin[MAX_SKINLEN];
	char skin_index[MAX_QPATH];
	int score, total;
	int ready, locked;
	int pauses_used, wantReset;
	cvar_t	*teamscore;
	edict_t	*captain;
	// Espionage
	edict_t *leader;
	int respawn_timer;
	qboolean leader_dead;
	char leader_name[MAX_SKINLEN];
	char leader_skin[MAX_QPATH];
	char leader_skin_index[MAX_QPATH];
#if AQTION_EXTENSION
#if AQTION_HUD
	int	 ghud_resettime;
	byte ghud_icon;
	byte ghud_num;
#endif
#endif
}team_t;

extern team_t teams[TEAM_TOP];
#define WEAP_ITM_NAME_LEN 32
#define PARSE_BUFSIZE 256
#define IS_ALIVE(ent) ((ent)->solid != SOLID_NOT && (ent)->deadflag != DEAD_DEAD)

#define GS_DEATHMATCH	1
#define GS_TEAMPLAY		2
#define GS_MATCHMODE	4
#define GS_ROUNDBASED	8
#define GS_WEAPONCHOOSE 16

extern int gameSettings;

#include "a_ctf.h"
#include "a_dom.h"
#include "a_esp.h"

#if AQTION_EXTENSION
#define HAS_CVARSYNC(ent) (Client_GetProtocol(ent) == 38 && Client_GetVersion(ent) >= 3013)

// hud (through ghud extension)
typedef enum {
	h_nameplate_l = 0,
	h_nameplate_r = 30,
	h_nameplate_end = 61,
	h_team_l,
	h_team_l_num,
	h_team_r,
	h_team_r_num,
} huditem_t;

void HUD_SetType(edict_t *clent, int type);
void HUD_ClientSetup(edict_t *clent);
void HUD_ClientUpdate(edict_t *clent);
void HUD_SpectatorSetup(edict_t *clent);
void HUD_SpectatorUpdate(edict_t *clent);

// cvar sync
typedef enum {
	clcvar_cl_antilag,
	clcvar_cl_indicators,
	clcvar_cl_xerp,
	clcvar_cl_spectatorhud,
	clcvar_cl_spectatorkillfeed,
} clcvar_t;

// UI flags from q2pro
#define UI_LEFT             BIT(0)
#define UI_RIGHT            BIT(1)
#define UI_CENTER           (UI_LEFT | UI_RIGHT)
#define UI_BOTTOM           BIT(2)
#define UI_TOP              BIT(3)
#define UI_MIDDLE           (UI_BOTTOM | UI_TOP)
#define UI_DROPSHADOW       BIT(4)
#define UI_ALTCOLOR         BIT(5)
#define UI_IGNORECOLOR      BIT(6)
#define UI_XORCOLOR         BIT(7)
#define UI_AUTOWRAP         BIT(8)
#define UI_MULTILINE        BIT(9)
#define UI_DRAWCURSOR       BIT(10)
#endif

#ifndef NO_BOTS
#include "acesrc/acebot.h"

#endif

typedef struct {
    int teamNum;
    edict_t *ent;
    int seconds;
    char *msg;
    qboolean fired;
} Message;
extern Message *timedMessages;

void addTimedMessage(int teamNum, edict_t *ent, int seconds, char *msg);
void FireTimedMessages(void);

//tng_net.c
#if AQTION_CURL
void lc_shutdown_function(void);
qboolean lc_init_function(void);
void lc_once_per_gameframe(void);
#define CALL_DISCORD_WEBHOOK(msg, type, award) lc_discord_webhook(msg, type, award)
void lc_discord_webhook(char* message, Discord_Notifications msgtype, Awards awardtype);
#define CALL_STATS_API(stats) lc_aqtion_stat_send(stats)
qboolean lc_aqtion_stat_send(const char *stats);
void lc_start_request_function(request_t* request);
#else
#define CALL_DISCORD_WEBHOOK(msg, type, award) // Do nothing if AQTION_CURL is disabled
#define CALL_STATS_API(stats) // Do nothing if AQTION_CURL is disabled
#endif
