//-----------------------------------------------------------------------------
// p_client.c
//
// $Id: p_client.c,v 1.90 2004/09/23 00:09:44 slicerdw Exp $
//
//-----------------------------------------------------------------------------
// $Log: p_client.c,v $
// Revision 1.90  2004/09/23 00:09:44  slicerdw
// Radio kill count was missing for falling death
//
// Revision 1.89  2004/04/08 23:19:51  slicerdw
// Optimized some code, added a couple of features and fixed minor bugs
//
// Revision 1.88  2003/10/01 19:24:14  igor_rock
// corrected a smaller bug (thanks to nopcode for the bug report)
//
// Revision 1.87  2003/06/15 21:43:53  igor
// added IRC client
//
// Revision 1.86  2003/06/15 15:34:32  igor
// - removed the zcam code from this branch (see other branch)
// - added fixes from 2.72 (source only) version
// - resetted version number to 2.72
// - This version should be exactly like the release 2.72 - just with a few
//   more fixes (which whoever did the source only variant didn't get because
//   he didn't use the CVS as he should. Shame on him.
//
// Revision 1.85  2003/02/10 02:12:25  ra
// Zcam fixes, kick crashbug in CTF fixed and some code cleanup.
//
// Revision 1.84  2002/12/31 17:07:22  igor_rock
// - corrected the Add_Ammo function to regard wp_flags
//
// Revision 1.83  2002/12/30 12:58:16  igor_rock
// - Corrected some comments (now it looks better)
// - allweapon mode now recognizes wp_flags
//
// Revision 1.82  2002/09/04 11:23:10  ra
// Added zcam to TNG and bumped version to 3.0
//
// Revision 1.81  2002/04/01 15:16:06  freud
// Stats code redone, tng_stats now much more smarter. Removed a few global
// variables regarding stats code and added kevlar hits to stats.
//
// Revision 1.80  2002/03/28 13:30:36  freud
// Included time played in ghost.
//
// Revision 1.79  2002/03/28 12:10:11  freud
// Removed unused variables (compiler warnings).
// Added cvar mm_allowlock.
//
// Revision 1.78  2002/03/25 23:35:19  freud
// Ghost code, use_ghosts and more stuff..
//
// Revision 1.77  2002/03/25 18:57:36  freud
// Added maximum number of stored player sessions (ghosts)
//
// Revision 1.76  2002/03/25 18:32:11  freud
// I'm being too productive.. New ghost command needs testing.
//
// Revision 1.75  2002/02/23 18:33:52  freud
// Fixed newline bug with announcer (EXCELLENT.. 1 FRAG LEFT) for logfiles
//
// Revision 1.74  2002/02/23 18:12:14  freud
// Added newlines back to the CenterPrintAll for IMPRESSIVE, EXCELLENT,
// ACCURACY and X FRAGS Left, it was screwing up the logfile.
//
// Revision 1.73  2002/02/19 09:32:47  freud
// Removed PING PONGs from CVS, not fit for release.
//
// Revision 1.72  2002/02/18 23:38:05  freud
// PING PONG..
//
// Revision 1.71  2002/02/18 23:25:42  freud
// More tweaks
//
// Revision 1.70  2002/02/18 20:21:36  freud
// Added PING PONG mechanism for timely disconnection of clients. This is
// based on a similar scheme as the scheme used by IRC. The client has
// cvar ping_timeout seconds to reply or will be disconnected.
//
// Revision 1.69  2002/02/18 18:25:51  ra
// Bumped version to 2.6, fixed ctf falling and kicking of players in ctf
// uvtime
//
// Revision 1.68  2002/02/18 17:17:21  freud
// Fixed the CTF leaving team bug. Also made the shield more efficient,
// No falling damage.
//
// Revision 1.67  2002/02/18 13:55:35  freud
// Added last damaged players %P
//
// Revision 1.66  2002/02/17 20:10:09  freud
// Better naming of auto_items is auto_equip, requested by Deathwatch.
//
// Revision 1.65  2002/02/17 20:01:32  freud
// Fixed stat_mode overflows, finally.
// Added 2 new cvars:
//      auto_join (0|1), enables auto joining teams from previous map.
//      auto_items (0|1), enables weapon and items caching between maps.
//
// Revision 1.64  2002/02/17 19:04:15  freud
// Possible bugfix for overflowing clients with stat_mode set.
//
// Revision 1.63  2002/02/01 16:09:49  freud
// Fixed the Taught how to fly bug
//
// Revision 1.62  2002/02/01 14:29:18  ra
// Attempting to fix tought how to fly no frag bug
//
// Revision 1.61  2002/01/31 11:15:06  freud
// Fix for crashes with stat_mode, not sure it works.
//
// Revision 1.60  2002/01/24 11:38:01  ra
// Cleanups
//
// Revision 1.59  2002/01/23 13:08:32  ra
// fixing tought how to fly bug
//
// Revision 1.58  2001/12/24 18:06:05  slicerdw
// changed dynamic check for darkmatch only
//
// Revision 1.56  2001/12/23 16:30:50  ra
// 2.5 ready. New stats from Freud. HC and shotgun gibbing seperated.
//
// Revision 1.55  2001/12/09 14:02:11  slicerdw
// Added gl_clear check -> video_check_glclear cvar
//
// Revision 1.54  2001/11/29 17:58:31  igor_rock
// TNG IRC Bot - First Version
//
// Revision 1.53  2001/11/08 20:56:24  igor_rock
// - changed some things related to wp_flags
// - corrected use_punch bug when player only has an empty weapon left
//
// Revision 1.52  2001/09/28 16:24:20  deathwatch
// use_rewards now silences the teamX wins sounds and added gibbing for the Shotgun
//
// Revision 1.51  2001/09/28 13:48:35  ra
// I ran indent over the sources. All .c and .h files reindented.
//
// Revision 1.50  2001/09/28 13:44:23  slicerdw
// Several Changes / improvements
//
// Revision 1.49  2001/09/05 14:33:57  slicerdw
// Added Fix's from the 2.1 release
//
// Revision 1.48  2001/09/03 14:25:00  deathwatch
// Added gibbing with HC (only happens rarely) when sv_gib is on and cleaned up
// the player_die function and made sure the flashlight gets turned off when someone
// is dead.
//
// Revision 1.47  2001/09/02 20:33:34  deathwatch
// Added use_classic and fixed an issue with ff_afterround, also updated version
// nr and cleaned up some commands.
//
// Updated the VC Project to output the release build correctly.
//
// Revision 1.46  2001/08/19 01:22:25  deathwatch
// cleaned the formatting of some files
//
// Revision 1.45  2001/08/17 21:31:37  deathwatch
// Added support for stats
//
// Revision 1.44  2001/08/15 14:50:48  slicerdw
// Added Flood protections to Radio & Voice, Fixed the sniper bug AGAIN
//
// Revision 1.43  2001/08/08 12:42:22  slicerdw
// Ctf Should finnaly be fixed now, lets hope so
//
// Revision 1.42  2001/08/06 23:35:31  ra
// Fixed an uvtime bug when clients join server while CTF rounds are already
// going.
//
// Revision 1.41  2001/08/06 14:38:45  ra
// Adding UVtime for ctf
//
// Revision 1.40  2001/08/06 13:41:41  slicerdw
// Added a fix for ctf..
//
// Revision 1.39  2001/08/06 03:00:49  ra
// Added FF after rounds. Please someone look at the EVIL if statments for me :)
//
// Revision 1.38  2001/08/03 15:08:32  ra
// Fix small bug in %K related to "tought how to fly" deaths.
//
// Revision 1.37  2001/07/20 11:56:04  slicerdw
// Added a check for the players spawning during countdown on ctf ( lets hope it works )
//
// Revision 1.36  2001/06/27 16:58:14  igor_rock
// corrected some limchasecam bugs
//
// Revision 1.35  2001/06/26 18:47:30  igor_rock
// added ctf_respawn cvar
//
// Revision 1.34  2001/06/25 11:44:47  slicerdw
// New Video Check System - video_check and video_check_lockpvs no longer latched
//
// Revision 1.33  2001/06/22 20:35:07  igor_rock
// fixed the flying corpse bug
//
// Revision 1.32  2001/06/22 18:37:01  igor_rock
// fixed than damn limchasecam bug - eentually :)
//
// Revision 1.31  2001/06/21 07:37:10  igor_rock
// fixed some limchasecam bugs
//
// Revision 1.30  2001/06/21 00:05:30  slicerdw
// New Video Check System done -  might need some revision but works..
//
// Revision 1.27  2001/06/20 07:21:21  igor_rock
// added use_warnings to enable/disable time/frags left msgs
// added use_rewards to enable/disable eimpressive, excellent and accuracy msgs
// change the configfile prefix for modes to "mode_" instead "../mode-" because
// they don't have to be in the q2 dir for doewnload protection (action dir is sufficient)
// and the "-" is bad in filenames because of linux command line parameters start with "-"
//
// Revision 1.26  2001/06/19 21:35:54  igor_rock
// If you select Sniper, sniper is your startweapon now.
//
// Revision 1.25  2001/06/19 21:10:05  igor_rock
// changed the "is now known" message to the normal namelimit
//
// Revision 1.24  2001/06/19 18:56:38  deathwatch
// New Last killed target system
//
// Revision 1.23  2001/06/18 18:14:09  igor_rock
// corrected bug with team none players shooting and flying around
//
// Revision 1.22  2001/06/01 19:18:42  slicerdw
// Added Matchmode Code
//
// Revision 1.21  2001/05/31 16:58:14  igor_rock
// conflicts resolved
//
// Revision 1.20  2001/05/20 15:00:19  slicerdw
// Some minor fixes and changings on Video Checking system
//
// Revision 1.19.2.3  2001/05/25 18:59:52  igor_rock
// Added CTF Mode completly :)
// Support for .flg files is still missing, but with "real" CTF maps like
// tq2gtd1 the ctf works fine.
// (I hope that all other modes still work, just tested DM and teamplay)
//
// Revision 1.19.2.2  2001/05/20 18:54:19  igor_rock
// added original ctf code snippets from zoid. lib compilesand runs but
// doesn't function the right way.
// Jsut committing these to have a base to return to if something wents
// awfully wrong.
//
// Revision 1.19.2.1  2001/05/20 15:17:31  igor_rock
// removed the old ctf code completly
//
// Revision 1.19  2001/05/20 12:54:18  igor_rock
// Removed newlines from Centered Messages like "Impressive"
//
// Revision 1.18  2001/05/16 13:26:38  slicerdw
// Too Many Userinfo Cvars( commented some) & Enabled death messages on CTF
//
// Revision 1.17  2001/05/13 15:01:45  ra
//
//
// In teamplay mode it is OK to teamkill as soon as the rounds are over. Changed
// things so that when the rounds are over it is also OK to plummet.
//
// Revision 1.16  2001/05/13 01:23:01  deathwatch
// Added Single Barreled Handcannon mode, made the menus and scoreboards
// look nicer and made the voice command a bit less loud.
//
// Revision 1.14  2001/05/12 13:51:20  mort
// Fixed ClientObituary Add_Frag bug in CTF
//
// Revision 1.13  2001/05/12 13:48:58  mort
// Fixed CTF ForceSpawn bug
//
// Revision 1.12  2001/05/12 13:37:38  mort
// Fixed CTF bug, god mode is now on when players spawn
//
// Revision 1.11  2001/05/12 10:50:16  slicerdw
// Fixed That Transparent List Thingy
//
// Revision 1.10  2001/05/11 16:07:26  mort
// Various CTF bits and pieces...
//
// Revision 1.9  2001/05/11 12:21:19  slicerdw
// Commented old Location support ( ADF ) With the ML/ETE Compatible one
//
// Revision 1.7  2001/05/07 21:18:35  slicerdw
// Added Video Checking System
//
// Revision 1.6  2001/05/07 20:06:45  igor_rock
// changed sound dir from sound/rock to sound/tng
//
// Revision 1.5  2001/05/07 02:05:36  ra
//
//
// Added tkok command to forgive teamkills.
//
// Revision 1.4  2001/05/07 01:44:07  ra
//
//
// Add a fix for the $$ skin crashing the server.
//
// Revision 1.3  2001/05/06 20:29:21  ra
//
//
// Adding comments to the limchasecam fix.
//
// Revision 1.2  2001/05/06 20:20:49  ra
//
//
// Fixing limchasecam.
//
// Revision 1.1.1.1  2001/05/06 17:29:49  igor_rock
// This is the PG Bund Edition V1.25 with all stuff laying around here...
//
//-----------------------------------------------------------------------------

#include "g_local.h"
#include "m_player.h"
#include "cgf_sfx_glass.h"


static void FreeClientEdicts(gclient_t *client)
{
	//remove lasersight
	if (client->lasersight) {
		G_FreeEdict(client->lasersight);
		client->lasersight = NULL;
	}

	//Turn Flashlight off
	if (client->flashlight) {
		G_FreeEdict(client->flashlight);
		client->flashlight = NULL;
	}

	//Remove grapple
	if (client->ctf_grapple) {
		G_FreeEdict(client->ctf_grapple);
		client->ctf_grapple = NULL;
	}

#if AQTION_EXTENSION
	//remove arrow
	if (client->arrow) {
		G_FreeEdict(client->arrow);
		client->arrow = NULL;
	}
#endif
}

void Announce_Reward(edict_t *ent, int rewardType) {
    char buf[256];
    char *soundFile;
    char *playername = ent->client->pers.netname;

    switch (rewardType) {
        case ACCURACY:
            if (ent->client->resp.streakHS/3 >= 2) {
                sprintf(buf,"ACCURACY %s (%dx)!", playername, ent->client->resp.streakHS/3);
            } else {
                sprintf(buf,"ACCURACY %s!", playername);
            }
            soundFile = "tng/accuracy.wav";
            break;
        case IMPRESSIVE:
            if (ent->client->resp.streakKills/5 >= 2) {
                sprintf(buf,"IMPRESSIVE %s (%dx)!", playername, ent->client->resp.streakKills/5);
            } else {
                sprintf(buf,"IMPRESSIVE %s!", playername);
            }
            soundFile = "tng/impressive.wav";
            break;
        case EXCELLENT:
            sprintf(buf,"EXCELLENT %s (%dx)!", playername, ent->client->resp.streakKills/12);
            soundFile = "tng/excellent.wav";
            break;
        case DOMINATING:
            sprintf(buf,"%s IS DOMINATING!", playername);
            soundFile = "radio/male/deliv3.wav";
            break;
        case UNSTOPPABLE:
            sprintf(buf,"%s IS UNSTOPPABLE!", playername);
            soundFile = "radio/male/deliv3.wav";
            break;
        default:
			gi.dprintf("%s: Unknown reward type %d for %s\n", __FUNCTION__, rewardType, playername);
            return;  // Something didn't jive here?
    }

    CenterPrintAll(buf);
    gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex(soundFile), 1.0, ATTN_NONE, 0.0);

    #if USE_AQTION
    if (stat_logs->value)
        LogAward(ent, rewardType);
    #endif
}

void Add_Frag(edict_t * ent, int mod)
{
	int frags = 0;

	if (in_warmup)
		return;

	ent->client->resp.kills++;
	// All normal weapon damage
	if (mod > 0 && mod < MAX_GUNSTAT) {
		ent->client->resp.gunstats[mod].kills++;
	}
	// Grenade splash, kicks and punch damage
	if (mod > 0 && ((mod == MOD_HG_SPLASH) || (mod == MOD_KICK) || (mod == MOD_PUNCH) || (mod = MOD_GRENADE_IMPACT))) {
		ent->client->resp.gunstats[mod].kills++;
	}

	if (IS_ALIVE(ent))
	{
		ent->client->resp.streakKills++;
		ent->client->resp.roundStreakKills++;
		if (ent->client->resp.streakKills > ent->client->resp.streakKillsHighest)
			ent->client->resp.streakKillsHighest = ent->client->resp.streakKills;

		if (ent->client->resp.streakKills % 5 == 0 && use_rewards->value)
		{
			Announce_Reward(ent, IMPRESSIVE);
		}
		else if (ent->client->resp.streakKills % 12 == 0 && use_rewards->value)
		{
			Announce_Reward(ent, EXCELLENT);
		}
	}

	// Regular frag for teamplay/matchmode
	if (teamplay->value && teamdm->value != 2)
		ent->client->resp.score++;	// just 1 normal kill

	// Increment team score if TeamDM is enabled
	if(teamdm->value)
		teams[ent->client->resp.team].score++;

	// Streak kill rewards in Deathmatch mode
	if (deathmatch->value && !teamplay->value) {
		if (ent->client->resp.streakKills < 4 || ! use_rewards->value)
			frags = 1;
		else if (ent->client->resp.streakKills < 8)
			frags = 2;
		else if (ent->client->resp.streakKills < 16)
			frags = 4;
		else if (ent->client->resp.streakKills < 32)
			frags = 8;
		else
			frags = 16;

		if(frags > 1)
		{
			gi.bprintf(PRINT_MEDIUM,
				"%s has %d kills in a row and receives %d frags for the kill!\n",
				ent->client->pers.netname, ent->client->resp.streakKills, frags );
			IRC_printf(IRC_T_GAME,
				"%n has %k kills in a row and receives %k frags for the kill!",
				ent->client->pers.netname, ent->client->resp.streakKills, frags );
		}
		ent->client->resp.score += frags;

		// Award team with appropriate streak reward count
		if(teamdm->value)
			teams[ent->client->resp.team].score += frags;

		// AQ:TNG Igor[Rock] changing sound dir
		if (fraglimit->value && use_warnings->value) {
			if (ent->client->resp.score == fraglimit->value - 1) {
				if (fragwarning < 3) {
					CenterPrintAll("1 FRAG LEFT...");
					gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
						gi.soundindex("tng/1_frag.wav"), 1.0, ATTN_NONE, 0.0);
					fragwarning = 3;
				}
			} else if (ent->client->resp.score == fraglimit->value - 2) {
				if (fragwarning < 2) {
					CenterPrintAll("2 FRAGS LEFT...");
					gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
						gi.soundindex("tng/2_frags.wav"), 1.0, ATTN_NONE, 0.0);
					fragwarning = 2;
				}
			} else if (ent->client->resp.score == fraglimit->value - 3) {
				if (fragwarning < 1) {
					CenterPrintAll("3 FRAGS LEFT...");
					gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
						gi.soundindex("tng/3_frags.wav"), 1.0, ATTN_NONE, 0.0);
					fragwarning = 1;
				}
			}
		}
		// end of changing sound dir
	}

//rekkie -- DEV_1 -- s
	// Random bot voice sounds

	// Debug this, sometimes bots will repeat this over and over again
	// TODO: Disable this if we get too close to the max sound limit
	if (use_voice->value && ent->is_bot && bot_randvoice->value > 0)
	{
		if (bot_randvoice->value > 100) bot_randvoice->value = 100;

		if (random() < (bot_randvoice->value / 100))
		{
			int voicechoice = rand() % 85;
			switch (voicechoice)
			{
			case 0:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/affirm.wav"), 1, ATTN_IDLE, 0);
				break;
			case 1:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/allbase.wav"), 1, ATTN_IDLE, 0);
				break;
			case 2:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/allbase2.wav"), 1, ATTN_IDLE, 0);
				break;
			case 3:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/arriba.wav"), 1, ATTN_IDLE, 0);
				break;
			case 4:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/audio1.wav"), 1, ATTN_IDLE, 0);
				break;
			case 5:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/badman.wav"), 1, ATTN_IDLE, 0);
				break;
			case 6:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/barf.wav"), 1, ATTN_IDLE, 0);
				break;
			case 7:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/becool.wav"), 1, ATTN_IDLE, 0);
				break;
			case 8:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/boing.wav"), 1, ATTN_IDLE, 0);
				break;
			case 9:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/bravo.wav"), 1, ATTN_IDLE, 0);
				break;
			case 10:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/cartmandontshoot.wav"), 1, ATTN_IDLE, 0);
				break;
			case 11:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/cdtrust.wav"), 1, ATTN_IDLE, 0);
				break;
			case 12:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/champion.wav"), 1, ATTN_IDLE, 0);
				break;
			case 13:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/chicken.wav"), 1, ATTN_IDLE, 0);
				break;
			case 14:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/childic.wav"), 1, ATTN_IDLE, 0);
				break;
			case 15:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/clint.wav"), 1, ATTN_IDLE, 0);
				break;
			case 16:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/comeback.wav"), 1, ATTN_IDLE, 0);
				break;
			case 17:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/doit.wav"), 1, ATTN_IDLE, 0);
				break;
			case 18:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/dontshoot.wav"), 1, ATTN_IDLE, 0);
				break;
			case 19:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/dum.wav"), 1, ATTN_IDLE, 0);
				break;
			case 20:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fap.wav"), 1, ATTN_IDLE, 0);
				break;
			case 21:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart1.wav"), 1, ATTN_IDLE, 0);
				break;
			case 22:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart2.wav"), 1, ATTN_IDLE, 0);
				break;
			case 23:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart3.wav"), 1, ATTN_IDLE, 0);
				break;
			case 24:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart4.wav"), 1, ATTN_IDLE, 0);
				break;
			case 25:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart5.wav"), 1, ATTN_IDLE, 0);
				break;
			case 26:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart6.wav"), 1, ATTN_IDLE, 0);
				break;
			case 27:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fart7.wav"), 1, ATTN_IDLE, 0);
				break;
			case 28:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fighting.wav"), 1, ATTN_IDLE, 0);
				break;
			case 29:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/filhands.wav"), 1, ATTN_IDLE, 0);
				break;
			case 30:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/fusk.wav"), 1, ATTN_IDLE, 0);
				break;
			case 31:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/getout.wav"), 1, ATTN_IDLE, 0);
				break;
			case 32:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/grimley.wav"), 1, ATTN_IDLE, 0);
				break;
			case 33:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/happen.wav"), 1, ATTN_IDLE, 0);
				break;
			case 34:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/headshot.wav"), 1, ATTN_IDLE, 0);
				break;
			case 35:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/iknow.wav"), 1, ATTN_IDLE, 0);
				break;
			case 36:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/isee.wav"), 1, ATTN_IDLE, 0);
				break;
			case 37:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/jabba.wav"), 1, ATTN_IDLE, 0);
				break;
			case 38:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/killme.wav"), 1, ATTN_IDLE, 0);
				break;
			case 39:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/kungfuft.wav"), 1, ATTN_IDLE, 0);
				break;
			case 40:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/l1.wav"), 1, ATTN_IDLE, 0);
				break;
			case 41:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/l2.wav"), 1, ATTN_IDLE, 0);
				break;
			case 42:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/laugh.wav"), 1, ATTN_IDLE, 0);
				break;
			case 43:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/lead.wav"), 1, ATTN_IDLE, 0);
				break;
			case 44:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/letsgo.wav"), 1, ATTN_IDLE, 0);
				break;
			case 45:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/letsrock.wav"), 1, ATTN_IDLE, 0);
				break;
			case 46:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/lol.wav"), 1, ATTN_IDLE, 0);
				break;
			case 47:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/lookwhat.wav"), 1, ATTN_IDLE, 0);
				break;
			case 48:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/mustbe.wav"), 1, ATTN_IDLE, 0);
				break;
			case 49:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/muu1.wav"), 1, ATTN_IDLE, 0);
				break;
			case 50:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/myvest.wav"), 1, ATTN_IDLE, 0);
				break;
			case 51:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/newbie.wav"), 1, ATTN_IDLE, 0);
				break;
			case 52:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/nice.wav"), 1, ATTN_IDLE, 0);
				break;
			case 53:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/no.wav"), 1, ATTN_IDLE, 0);
				break;
			case 54:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/nosub.wav"), 1, ATTN_IDLE, 0);
				break;
			case 55:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/pointy.wav"), 1, ATTN_IDLE, 0);
				break;
			case 56:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/pressluck.wav"), 1, ATTN_IDLE, 0);
				break;
			case 57:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/real.wav"), 1, ATTN_IDLE, 0);
				break;
			case 58:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/rofl.wav"), 1, ATTN_IDLE, 0);
				break;
			case 59:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/s1.wav"), 1, ATTN_IDLE, 0);
				break;
			case 60:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/s2.wav"), 1, ATTN_IDLE, 0);
				break;
			case 61:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/s3.wav"), 1, ATTN_IDLE, 0);
				break;
			case 62:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/shootorwhat.wav"), 1, ATTN_IDLE, 0);
				break;
			case 63:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/speedy.wav"), 1, ATTN_IDLE, 0);
				break;
			case 64:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/srikeme.wav"), 1, ATTN_IDLE, 0);
				break;
			case 65:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/suprisemf.wav"), 1, ATTN_IDLE, 0);
				break;
			case 66:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/terminat.wav"), 1, ATTN_IDLE, 0);
				break;
			case 67:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/thanku.wav"), 1, ATTN_IDLE, 0);
				break;
			case 68:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/tweetyhelp.wav"), 1, ATTN_IDLE, 0);
				break;
			case 69:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/unbel.wav"), 1, ATTN_IDLE, 0);
				break;
			case 70:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/vista.wav"), 1, ATTN_IDLE, 0);
				break;
			case 71:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/warchant.wav"), 1, ATTN_IDLE, 0);
				break;
			case 72:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/watching.wav"), 1, ATTN_IDLE, 0);
				break;
			case 73:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/whyuss.wav"), 1, ATTN_IDLE, 0);
				break;
			case 74:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/wrong.wav"), 1, ATTN_IDLE, 0);
				break;
			case 75:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/wtsamat.wav"), 1, ATTN_IDLE, 0);
				break;
			case 76:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/yeahbaby.wav"), 1, ATTN_IDLE, 0);
				break;
			case 77:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("user/yuck2.wav"), 1, ATTN_IDLE, 0);
				break;
			case 78:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("wizardext/mahadeva.wav"), 1, ATTN_IDLE, 0);
				break;
			case 79:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("barbq/badidea.wav"), 1, ATTN_IDLE, 0);
				break;
			case 80:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("barbq/bbq01.wav"), 1, ATTN_IDLE, 0);
				break;
			case 81:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("barbq/bbq02.wav"), 1, ATTN_IDLE, 0);
				break;
			case 82:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("barbq/scream.wav"), 1, ATTN_IDLE, 0);
				break;
			case 83:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("barbq/scream2.wav"), 1, ATTN_IDLE, 0);
				break;
			case 84:
				gi.sound(ent, CHAN_VOICE, gi.soundindex("barbq/scream3.wav"), 1, ATTN_IDLE, 0);
				break;
			default:
				break;
			}
		}

		// A little taunting...
		int wavechoice = rand() % 5;
		switch (wavechoice)
		{
		case 0:
			BOTLIB_Wave(ent, WAVE_FLIPOFF);
			break;
		case 1:
			BOTLIB_Wave(ent, WAVE_SALUTE);
			break;
		case 2:
			BOTLIB_Wave(ent, WAVE_TAUNT);
			break;
		case 3:
			BOTLIB_Wave(ent, WAVE_WAVE);
			break;
		case 4:
			BOTLIB_Wave(ent, WAVE_POINT);
			break;
		default:
			break;
		}
	}

	#ifndef NO_BOTS
	//darksaint -- Bot Chat -- s
	// Generates chat message if respawning (killed)
	if (ent->is_bot) {
		BOTLIB_Chat(ent, CHAT_INSULTS);
		if(bot_personality->value && bot_ragequit->value) {
			BotRageQuit(ent, false);
		}
	}
	#endif
	//darksaint -- Bot Chat -- e

	// Announce kill streak to player if use_killcounts is enabled on server
	if (use_killcounts->value) {
		// Report only killstreak during that round
		if(ent->client->resp.roundStreakKills)
			gi.cprintf(ent, PRINT_HIGH, "Kill count: %d\n", ent->client->resp.roundStreakKills);
	} else {
		// Report total killstreak across previous rounds
		if(ent->client->resp.streakKills)
			gi.cprintf(ent, PRINT_HIGH, "Kill count: %d\n", ent->client->resp.streakKills);
	}
}

void Subtract_Frag(edict_t * ent)
{
	if( in_warmup )
		return;

	ent->client->resp.kills--;
	ent->client->resp.score--;
	ent->client->resp.streakKills = 0;
	ent->client->resp.roundStreakKills = 0;
	if(teamdm->value)
		teams[ent->client->resp.team].score--;
}

void Add_Death( edict_t *ent, qboolean end_streak )
{
	if( in_warmup )
		return;

	ent->client->resp.deaths ++;
	if( end_streak ) {
		ent->client->resp.streakKills = 0;
		ent->client->resp.roundStreakKills = 0;
	}
}

// FRIENDLY FIRE functions

void Add_TeamWound(edict_t * attacker, edict_t * victim, int mod)
{
	if (!teamplay->value || !attacker->client || !victim->client) {
		return;
	}

	attacker->client->resp.team_wounds++;

	// Warn both parties that they are teammates. Since shotguns are pellet based,
	// make sure we don't overflow the client when using MOD_HC or MOD_SHOTGUN. The
	// ff_warning flag should have been reset before each attack.
	if (attacker->client->ff_warning == 0) {
		attacker->client->ff_warning++;
		gi.cprintf(victim, PRINT_HIGH, "You were hit by %s, your TEAMMATE!\n", attacker->client->pers.netname);
		gi.cprintf(attacker, PRINT_HIGH, "You hit your TEAMMATE %s!\n", victim->client->pers.netname);
	}
	// We want team_wounds to increment by one for each ATTACK, not after each 
	// bullet or pellet does damage. With the HAND CANNON this means 2 attacks
	// since it is double barreled and we don't want to go into p_weapon.c...
	attacker->client->resp.team_wounds = (attacker->client->team_wounds_before + 1);

	// If count is less than MAX_TEAMKILLS*3, return. If count is greater than
	// MAX_TEAMKILLS*3 but less than MAX_TEAMKILLS*4, print off a ban warning. If
	// count equal (or greater than) MAX_TEAMKILLS*4, ban and kick the client.
	if ((int) maxteamkills->value < 1)	//FB
		return;
	if (attacker->client->resp.team_wounds < ((int)maxteamkills->value * 3)) {
		return;
	} else if (attacker->client->resp.team_wounds < ((int) maxteamkills->value * 4)) {
		// Print a note to console, and issue a warning to the player.
		gi.cprintf(NULL, PRINT_MEDIUM,
			   "%s is in danger of being banned for wounding teammates\n", attacker->client->pers.netname);
		gi.cprintf(attacker, PRINT_HIGH,
			   "WARNING: You'll be temporarily banned if you continue wounding teammates!\n");
		return;
	} else {
		if (attacker->client->pers.ip[0]) {
			if (Ban_TeamKiller(attacker, (int) twbanrounds->value)) {
				gi.cprintf(NULL, PRINT_MEDIUM,
					   "Banning %s@%s for team wounding\n",
					   attacker->client->pers.netname, attacker->client->pers.ip);

				gi.cprintf(attacker, PRINT_HIGH,
					   "You've wounded teammates too many times, and are banned for %d %s.\n",
					   (int) twbanrounds->value,
					   (((int) twbanrounds->value > 1) ? "games" : "game"));
			} else {
				gi.cprintf(NULL, PRINT_MEDIUM,
					   "Error banning %s: unable to get ip address\n", attacker->client->pers.netname);
			}
			Kick_Client(attacker);
		}
	}

	return;
}

void Add_TeamKill(edict_t * attacker)
{
	if (!teamplay->value || !attacker->client || !team_round_going) {
		return;
	}

	attacker->client->resp.team_kills++;
	// Because the stricter team kill was incremented, lower team_wounds
	// by amount inflicted in last attack (i.e., no double penalty).
	if (attacker->client->resp.team_wounds > attacker->client->team_wounds_before) {
		attacker->client->resp.team_wounds = attacker->client->team_wounds_before;
	}
	// If count is less than 1/2 MAX_TEAMKILLS, print off simple warning. If
	// count is greater than 1/2 MAX_TEAMKILLS but less than MAX_TEAMKILLS,
	// print off a ban warning. If count equal or greater than MAX_TEAMKILLS,
	// ban and kick the client.
	if (((int) maxteamkills->value < 1) ||
		(attacker->client->resp.team_kills < (((int)maxteamkills->value % 2) + (int)maxteamkills->value / 2))) {
		gi.cprintf(attacker, PRINT_HIGH, "You killed your TEAMMATE!\n");
		return;
	} else if (attacker->client->resp.team_kills < (int) maxteamkills->value) {
		// Show this on the console
		gi.cprintf(NULL, PRINT_MEDIUM,
			   "%s is in danger of being banned for killing teammates\n", attacker->client->pers.netname);
		// Issue a warning to the player
		gi.cprintf(attacker, PRINT_HIGH, "WARNING: You'll be banned if you continue killing teammates!\n");
		return;
	} else {
		// They've killed too many teammates this game - kick 'em for a while
		if (attacker->client->pers.ip[0]) {
			if (Ban_TeamKiller(attacker, (int) tkbanrounds->value)) {
				gi.cprintf(NULL, PRINT_MEDIUM,
					   "Banning %s@%s for team killing\n",
					   attacker->client->pers.netname, attacker->client->pers.ip);
				gi.cprintf(attacker, PRINT_HIGH,
					   "You've killed too many teammates, and are banned for %d %s.\n",
					   (int) tkbanrounds->value,
					   (((int) tkbanrounds->value > 1) ? "games" : "game"));
			} else {
				gi.cprintf(NULL, PRINT_MEDIUM,
					   "Error banning %s: unable to get ip address\n", attacker->client->pers.netname);
			}
		}
		Kick_Client(attacker);
	}
}

// FRIENDLY FIRE

//
// Gross, ugly, disgustuing hack section
//

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start( edict_t * self )
{
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t * self)
{
	SP_misc_teleporter_dest(self);
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}

//=======================================================================

void player_pain(edict_t * self, edict_t * other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}

// ^^^

// PrintDeathMessage: moved the actual printing of the death messages to here, to handle
//  the fact that live players shouldn't receive them in teamplay.  -FB
void PrintDeathMessage(char *msg, edict_t * gibee)
{
	int j;
	edict_t *other;

	if (!teamplay->value || in_warmup) {
		gi.bprintf(PRINT_MEDIUM, "%s", msg);
		return;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_MEDIUM, "%s", msg);

	// First, let's print the message for gibee and its attacker. -TempFile
	gi.cprintf(gibee, PRINT_MEDIUM, "%s", msg);
	if (gibee->client->attacker && gibee->client->attacker != gibee)
		gi.cprintf(gibee->client->attacker, PRINT_MEDIUM, "%s", msg);

	if(!team_round_going)
		return;

	for (j = 1; j <= game.maxclients; j++) {
		other = &g_edicts[j];
		if (!other->inuse || !other->client)
			continue;
#ifndef NO_BOTS
		if( other->is_bot )
			continue;
#endif

		// only print if he's NOT gibee, NOT attacker, and NOT alive! -TempFile
		if (other != gibee && other != gibee->client->attacker && other->solid == SOLID_NOT)
			gi.cprintf(other, PRINT_MEDIUM, "%s", msg);
	}
}

void ClientObituary(edict_t * self, edict_t * inflictor, edict_t * attacker)
{
	int mod;
	int loc;
	char *message;
	char *message2;
	char death_msg[1024];	// enough in all situations? -FB
	qboolean friendlyFire;
	char *special_message = NULL;
	int n;

	self->client->resp.ctf_capstreak = 0;
	self->client->resp.dom_capstreak = 0;

	if (esp->value && IS_LEADER(self))
		self->client->resp.esp_capstreak = 0;

	friendlyFire = meansOfDeath & MOD_FRIENDLY_FIRE;
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	loc = locOfDeath;	// useful for location based hits
	message = NULL;
	message2 = "";

	// Reki: Print killfeed to spectators who ask for easily parsable stuff
	edict_t *other;
	int j;
	for (j = 1; j <= game.maxclients; j++) {
		other = &g_edicts[j];
		if (!other->inuse || !other->client || !teamplay->value)
			continue;

		if (other->client->resp.team) // we only want team 0 (spectators)
			continue;

		if (!(other->client->pers.spec_flags & SPECFL_KILLFEED)) // only print to spectators who want it
			continue;

		if (attacker == world || !attacker->client)
			sprintf(death_msg, "--KF %i %s, MOD %i\n",
				self->client->resp.team, self->client->pers.netname, mod);
		else
			sprintf(death_msg, "--KF %i %s, MOD %i, %i %s\n",
				attacker->client->resp.team, attacker->client->pers.netname, mod, self->client->resp.team, self->client->pers.netname);
		gi.cprintf(other, PRINT_MEDIUM, "%s", death_msg);
	}
	//

	if (attacker == self)
	{
		switch (mod) {
		case MOD_HELD_GRENADE:
			message = "tried to put the pin back in";
			break;
		case MOD_HG_SPLASH:
			if (self->client->pers.gender == GENDER_MALE)
				message = "didn't throw his grenade far enough";
			else if (self->client->pers.gender == GENDER_FEMALE)
				message = "didn't throw her grenade far enough";
			else
				message = "didn't throw its grenade far enough";
			break;
		case MOD_G_SPLASH:
			if (self->client->pers.gender == GENDER_MALE)
				message = "tripped on his own grenade";
			else if (self->client->pers.gender == GENDER_FEMALE)
				message = "tripped on her own grenade";
			else
				message = "tripped on its own grenade";
			break;
		default:
			if (self->client->pers.gender == GENDER_MALE)
				message = "killed himself";
			else if (self->client->pers.gender == GENDER_FEMALE)
				message = "killed herself";
			else
				message = "killed itself";
			break;
		}
	}

	if (!message) {
		switch (mod) {
		case MOD_BREAKINGGLASS:
			if( self->client->push_timeout > 40 )
				special_message = "was thrown through a window by";
			message = "ate too much glass";
			break;
		case MOD_SUICIDE:
			message = "is done with the world";
			break;
		case MOD_FALLING:
			if( self->client->push_timeout )
				special_message = "was taught how to fly by";
			//message = "hit the ground hard, real hard";
			if (self->client->pers.gender == GENDER_MALE)
				message = "plummets to his death";
			else if (self->client->pers.gender == GENDER_FEMALE)
				message = "plummets to her death";
			else
				message = "plummets to its death";
			break;
		case MOD_CRUSH:
			message = "was flattened";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			if( self->client->push_timeout )
				special_message = "melted thanks to";
			message = "melted";
			break;
		case MOD_LAVA:
			if( self->client->push_timeout )
				special_message = "was drop-kicked into the lava by";
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			if( self->client->push_timeout )
				special_message = "was shoved off the edge by";
			message = "was in the wrong place";
			break;
		}
	}

	if (message)
	{
		// handle falling with an attacker set
		if (special_message && self->client->attacker && self->client->attacker->client
		&& (self->client->attacker->client != self->client))
		{
			sprintf(death_msg, "%s %s %s\n",
				self->client->pers.netname, special_message, self->client->attacker->client->pers.netname);
			PrintDeathMessage(death_msg, self);
			IRC_printf(IRC_T_KILL, death_msg);
			AddKilledPlayer(self->client->attacker, self);

			#if USE_AQTION
			if (stat_logs->value) { // Only create stats logs if stat_logs is 1
				LogKill(self, inflictor, self->client->attacker);
			}
			#endif

			self->client->attacker->client->radio_num_kills++;

			//MODIFIED FOR FF -FB
			if (OnSameTeam(self, self->client->attacker))
			{
				if (!DMFLAGS(DF_NO_FRIENDLY_FIRE) && (!teamplay->value || team_round_going || !ff_afterround->value)) {
					self->enemy = self->client->attacker;
					Add_TeamKill(self->client->attacker);
					Subtract_Frag(self->client->attacker);	//attacker->client->resp.score--;
					Add_Death( self, false );
				}
			}
			else
			{
				Add_Frag(self->client->attacker, MOD_UNKNOWN);
				Add_Death( self, true );
			}

		}
		else
		{
			sprintf( death_msg, "%s %s\n", self->client->pers.netname, message );
			PrintDeathMessage(death_msg, self );
			IRC_printf( IRC_T_DEATH, death_msg );

			if (!teamplay->value || team_round_going || !ff_afterround->value)  {
				Subtract_Frag( self );
				Add_Death( self, true );
			}

			self->enemy = NULL;
      
			#if USE_AQTION
			if (stat_logs->value) { // Only create stats logs if stat_logs is 1
				LogWorldKill(self);
			}
			#endif
		}
		return;
	}
#if 0
		// handle bleeding, not used because bleeding doesn't get set
		if (mod == MOD_BLEEDING) {
			sprintf(death_msg, "%s bleeds to death\n", self->client->pers.netname);
			PrintDeathMessage(death_msg, self);
			return;
		}
#endif

	self->enemy = attacker;
	if (attacker && attacker->client)
	{
		switch (mod) {
		case MOD_MK23:	// zucc
			switch (loc) {
			case LOC_HDAM:
				if (self->client->pers.gender == GENDER_MALE)
					message = " has a hole in his head from";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " has a hole in her head from";
				else
					message = " has a hole in its head from";
				message2 = "'s Mark 23 pistol";
				break;
			case LOC_CDAM:
				message = " loses a vital chest organ thanks to";
				message2 = "'s Mark 23 pistol";
				break;
			case LOC_SDAM:
				if (self->client->pers.gender == GENDER_MALE)
					message = " loses his lunch to";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " loses her lunch to";
				else
					message = " loses its lunch to";
				message2 = "'s .45 caliber pistol round";
				break;
			case LOC_LDAM:
				message = " is legless because of";
				message2 = "'s .45 caliber pistol round";
				break;
			default:
				message = " was shot by";
				message2 = "'s Mark 23 Pistol";
			}
			break;
		case MOD_MP5:
			switch (loc) {
			case LOC_HDAM:
				message = "'s brains are on the wall thanks to";
				message2 = "'s 10mm MP5/10 round";
				break;
			case LOC_CDAM:
				message = " feels some chest pain via";
				message2 = "'s MP5/10 Submachinegun";
				break;
			case LOC_SDAM:
				message = " needs some Pepto Bismol after";
				message2 = "'s 10mm MP5 round";
				break;
			case LOC_LDAM:
				if (self->client->pers.gender == GENDER_MALE)
					message = " had his legs blown off thanks to";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " had her legs blown off thanks to";
				else
					message = " had its legs blown off thanks to";
				message2 = "'s MP5/10 Submachinegun";
				break;
			default:
				message = " was shot by";
				message2 = "'s MP5/10 Submachinegun";
			}
			break;
		case MOD_M4:
			switch (loc) {
			case LOC_HDAM:
				message = " had a makeover by";
				message2 = "'s M4 Assault Rifle";
				break;
			case LOC_CDAM:
				message = " feels some heart burn thanks to";
				message2 = "'s M4 Assault Rifle";
				break;
			case LOC_SDAM:
				message = " has an upset stomach thanks to";
				message2 = "'s M4 Assault Rifle";
				break;
			case LOC_LDAM:
				message = " is now shorter thanks to";
				message2 = "'s M4 Assault Rifle";
				break;
			default:
				message = " was shot by";
				message2 = "'s M4 Assault Rifle";
			}
			break;
		case MOD_M3:
			n = rand() % 2 + 1;
			if (n == 1) {
				message = " accepts";
				message2 = "'s M3 Super 90 Assault Shotgun in hole-y matrimony";
			} else {
				message = " is full of buckshot from";
				message2 = "'s M3 Super 90 Assault Shotgun";
			}
			break;
		case MOD_HC:
			n = rand() % 3 + 1;
			if (n == 1) {
				if (attacker->client->pers.hc_mode)	// AQ2:TNG Deathwatch - Single Barreled HC Death Messages
				{
					message = " underestimated";
					message2 = "'s single barreled handcannon shot";
				} else {
					message = " ate";
					message2 = "'s sawed-off 12 gauge";
				}
			} else if (n == 2 ){
				if (attacker->client->pers.hc_mode)	// AQ2:TNG Deathwatch - Single Barreled HC Death Messages
				{
					message = " won't be able to pass a metal detector anymore thanks to";
					message2 = "'s single barreled handcannon shot";
				} else {
					message = " is full of buckshot from";
					message2 = "'s sawed off shotgun";
				} 
			} else {
				// minch <3
				message = " was minched by";
			}
			break;
		case MOD_SNIPER:
			switch (loc) {
			case LOC_HDAM:
				if (self->client->ps.fov < 90) {
					if (self->client->pers.gender == GENDER_MALE)
						message = " saw the sniper bullet go through his scope thanks to";
					else if (self->client->pers.gender == GENDER_FEMALE)
						message = " saw the sniper bullet go through her scope thanks to";
					else
						message = " saw the sniper bullet go through its scope thanks to";
				} else
					message = " caught a sniper bullet between the eyes from";
				break;
			case LOC_CDAM:
				message = " was picked off by";
				break;
			case LOC_SDAM:
				message = " was sniped in the stomach by";
				break;
			case LOC_LDAM:
				message = " was shot in the legs by";
				break;
			default:
				message = " was sniped by";
				//message2 = "'s Sniper Rifle";
			}
			break;
		case MOD_DUAL:
			switch (loc) {
			case LOC_HDAM:
				message = " was trepanned by";
				message2 = "'s akimbo Mark 23 pistols";
				break;
			case LOC_CDAM:
				message = " was John Woo'd by";
				//message2 = "'s .45 caliber pistol round";
				break;
			case LOC_SDAM:
				message = " needs some new kidneys thanks to";
				message2 = "'s akimbo Mark 23 pistols";
				break;
			case LOC_LDAM:
				message = " was shot in the legs by";
				message2 = "'s akimbo Mark 23 pistols";
				break;
			default:
				message = " was shot by";
				message2 = "'s pair of Mark 23 Pistols";
			}
			break;
		case MOD_KNIFE:
			switch (loc) {
			case LOC_HDAM:
				if (self->client->pers.gender == GENDER_MALE)
					message = " had his throat slit by";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " had her throat slit by";
				else
					message = " had its throat slit by";
				break;
			case LOC_CDAM:
				message = " had open heart surgery, compliments of";
				break;
			case LOC_SDAM:
				message = " was gutted by";
				break;
			case LOC_LDAM:
				message = " was stabbed repeatedly in the legs by";
				break;
			default:
				message = " was slashed apart by";
				message2 = "'s Combat Knife";
			}
			break;
		case MOD_KNIFE_THROWN:
			switch (loc) {
				case LOC_HDAM:
				message = " caught";
				if (self->client->pers.gender == GENDER_MALE)
					message2 = "'s flying knife with his forehead";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message2 = "'s flying knife with her forehead";
				else
					message2 = "'s flying knife with its forehead";
				break;
			case LOC_CDAM:
				message = "'s ribs don't help against";
				message2 = "'s flying knife";
				break;
			case LOC_SDAM:
				if (self->client->pers.gender == GENDER_MALE)
					message = " sees the contents of his own stomach thanks to";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " sees the contents of her own stomach thanks to";
				else
					message = " sees the contents of its own stomach thanks to";
				message2 = "'s flying knife";
				break;
			case LOC_LDAM:
				if (self->client->pers.gender == GENDER_MALE)
					message = " had his legs cut off thanks to";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " had her legs cut off thanks to";
				else
					message = " had its legs cut off thanks to";
				message2 = "'s flying knife";
				break;
			default:
				message = " was hit by";
				message2 = "'s flying Combat Knife";
			}
			break;
		case MOD_KICK:
			n = rand() % 3 + 1;
			if (n == 1) {
				if (self->client->pers.gender == GENDER_MALE)
					message = " got his ass kicked by";
				else if (self->client->pers.gender == GENDER_FEMALE)
					message = " got her ass kicked by";
				else
					message = " got its ass kicked by";
			} else if (n == 2) {
				if (self->client->pers.gender == GENDER_MALE) {
					message = " couldn't remove";
					message2 = "'s boot from his ass";
				} else if (self->client->pers.gender == GENDER_FEMALE) {
					message = " couldn't remove";
					message2 = "'s boot from her ass";
				} else {
					message = " couldn't remove";
					message2 = "'s boot from its ass";
				}
			} else {
				if (self->client->pers.gender == GENDER_MALE) {
					message = " had a Bruce Lee put on him by";
					message2 = ", with a quickness";
				} else if (self->client->pers.gender == GENDER_FEMALE) {
					message = " had a Bruce Lee put on her by";
					message2 = ", with a quickness";
				} else {
					message = " had a Bruce Lee put on it by";
					message2 = ", with a quickness";
				}
			}
			break;
		case MOD_PUNCH:
			n = rand() % 3 + 1;
			if (n == 1) {
				message = " got a free facelift by";
			} else if (n == 2) {
				message = " was knocked out by";
			} else {
				message = " caught";
				message2 = "'s iron fist";
			}
			break;
		case MOD_BLASTER:
			message = "was blasted by";
			break;
		case MOD_GRENADE:
			message = "was popped by";
			message2 = "'s grenade";
			break;
		case MOD_G_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			break;
		case MOD_HYPERBLASTER:
			message = "was melted by";
			message2 = "'s hyperblaster";
			break;
		case MOD_HANDGRENADE:
			message = " caught";
			message2 = "'s handgrenade";
			break;
		case MOD_HG_SPLASH:
			message = " didn't see";
			message2 = "'s handgrenade";
			break;
		case MOD_HELD_GRENADE:
			message = " feels";
			message2 = "'s pain";
			break;
		case MOD_TELEFRAG:
			message = " tried to invade";
			message2 = "'s personal space";
			break;
		case MOD_GRAPPLE:
			message = " was caught by";
			message2 = "'s grapple";
			break;
		case MOD_GRENADE_IMPACT:
			message = " was deeply impacted by";
			message2 = "'s grenade";
			break;
		}	//end of case (mod)

		if (message)
		{
			sprintf(death_msg, "%s%s %s%s\n", self->client->pers.netname,
			message, attacker->client->pers.netname, message2);
			PrintDeathMessage(death_msg, self);
			IRC_printf(IRC_T_KILL, death_msg);
			AddKilledPlayer(attacker, self);

			#if USE_AQTION
			if (stat_logs->value) {
				LogKill(self, inflictor, attacker);
			}
			#endif

			if (friendlyFire) {
				if (!teamplay->value || team_round_going || !ff_afterround->value)
				{
					self->enemy = attacker; //tkok
					Add_TeamKill(attacker);
					Subtract_Frag(attacker);	//attacker->client->resp.score--;
					Add_Death( self, false );
				}
			} else {
				if (!teamplay->value || mod != MOD_TELEFRAG) {
					Add_Frag(attacker, mod);
					attacker->client->radio_num_kills++;
					Add_Death( self, true );
				}
			}

			#ifndef NO_BOTS
			//darksaint -- Bot Chat -- s
			// Generates chat message if respawning (killed)
			if (self->is_bot && mod < MOD_TOTAL) {  // Don't count 'killed him/herself' messages
				BOTLIB_Chat(self, CHAT_KILLED);
				if(bot_personality->value && bot_ragequit->value) {
					BotRageQuit(self, true);
				}
			}
			#endif
			//darksaint -- Bot Chat -- e


			return;
		}	// if(message)
	}

	sprintf(death_msg, "%s died\n", self->client->pers.netname);
	PrintDeathMessage(death_msg, self);
	IRC_printf(IRC_T_DEATH, death_msg);

	#if USE_AQTION
	if (stat_logs->value) { // Only create stats logs if stat_logs is 1
		LogWorldKill(self);
	}
	#endif

	Subtract_Frag(self);	//self->client->resp.score--;
	Add_Death( self, true );
}

// zucc used to toss an item on death
void EjectItem(edict_t * ent, gitem_t * item)
{
	edict_t *drop;
	float spread;

	if (item) {
		spread = 300.0 * crandom();
		ent->client->v_angle[YAW] -= spread;
		drop = Drop_Item(ent, item);
		ent->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

}

// unique weapons need to be specially treated so they respawn properly
void EjectWeapon(edict_t * ent, gitem_t * item)
{
	edict_t *drop;
	float spread;

	if (item) {
		spread = 300.0 * crandom();
		ent->client->v_angle[YAW] -= spread;
		drop = Drop_Item(ent, item);
		ent->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
		if (!in_warmup)
			drop->think = SpecialWeaponRespawnTimer;
	}

}

void EjectMedKit( edict_t *ent, int medkit )
{
	gitem_t *item = FindItem("Health");
	float spread = 300.0 * crandom();
	edict_t *drop = NULL;

	if( ! item )
		return;

	item->world_model = "models/items/healing/medium/tris.md2";
	ent->client->v_angle[YAW] -= spread;
	drop = Drop_Item( ent, item );
	ent->client->v_angle[YAW] += spread;
	drop->model = item->world_model;
	drop->classname = "medkit";
	drop->count = medkit;

	if( ! medkit_instant->value )
		drop->style = 4; // HEALTH_MEDKIT (g_items.c)
}

//zucc toss items on death
void TossItemsOnDeath(edict_t * ent)
{
	gitem_t *item;
	//qboolean quad = false;
	int i;

	// Don't drop items if leader, this is just a mess
	if (esp->value && IS_LEADER(ent))
		return;

	// don't bother dropping stuff when allweapons/items is active
	if (allitem->value) {
		// remove the lasersight because then the observer might have it
		item = GET_ITEM(LASER_NUM);
		ent->client->inventory[ITEM_INDEX(item)] = 0;
	} else {
		DeadDropSpec(ent);
	}

	if( medkit_drop->value > 0 )
		EjectMedKit( ent, medkit_drop->value );

	if (allweapon->value)// don't drop weapons if allweapons is on
		return;

	if (WPF_ALLOWED(MK23_NUM) && WPF_ALLOWED(DUAL_NUM)) {
		// give the player a dual pistol so they can be sure to drop one
		item = GET_ITEM(DUAL_NUM);
		ent->client->inventory[ITEM_INDEX(item)]++;
		EjectItem(ent, item);
	}

	// check for every item we want to drop when a player dies
	for (i = MP5_NUM; i < DUAL_NUM; i++) {
		item = GET_ITEM( i );
		while (ent->client->inventory[ITEM_INDEX( item )] > 0) {
			ent->client->inventory[ITEM_INDEX( item )]--;
			EjectWeapon( ent, item );
		}
	}

	item = GET_ITEM(KNIFE_NUM);
	if (ent->client->inventory[ITEM_INDEX(item)] > 0) {
		EjectItem(ent, item);
	}
// special items

	// Quad is unused in AQ2
	// if (!DMFLAGS(DF_QUAD_DROP))
	// 	quad = false;
	// else
	// 	quad = (ent->client->quad_framenum > (level.framenum + HZ));
}

void TossClientWeapon(edict_t * self)
{
	gitem_t *item;
	edict_t *drop;
	qboolean quad;
	float spread;

	item = self->client->weapon;
	if (!self->client->inventory[self->client->ammo_index])
		item = NULL;
	if (item && (strcmp(item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!DMFLAGS(DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + HZ));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item) {
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item(self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller(edict_t * self, edict_t * inflictor, edict_t * attacker)
{
	vec3_t dir;

	if (attacker && attacker != world && attacker != self) {
		VectorSubtract(attacker->s.origin, self->s.origin, dir);
	} else if (inflictor && inflictor != world && inflictor != self) {
		VectorSubtract(inflictor->s.origin, self->s.origin, dir);
	} else {
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180 / M_PI * atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
}

/*
==================
player_die
==================
*/
void player_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int n, mod;

	VectorClear(self->avelocity);

	self->takedamage = DAMAGE_NO;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove linked ctf flag

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;

	self->maxs[2] = -8;

	self->svflags |= SVF_DEADMONSTER;

	if (self->solid == SOLID_TRIGGER) {
		self->solid = SOLID_BBOX;
		RemoveFromTransparentList(self);
	}

	self->client->reload_attempts = 0;	// stop them from trying to reload
	self->client->weapon_attempts = 0;

	self->client->desired_zoom = 0;
	self->client->autoreloading = false;

	if (!self->deadflag) {
		if (ctf->value) {
			self->client->respawn_framenum = level.framenum + CTFGetRespawnTime(self) * HZ;
		}
		else if (esp->value) {
			self->client->respawn_framenum = level.framenum + EspGetRespawnTime(self) * HZ;
		}
		else if(teamdm->value) {
			self->client->respawn_framenum = level.framenum + (int)(teamdm_respawn->value * HZ);
		}
		else {
			self->client->respawn_framenum = level.framenum + 1 * HZ;
		}
		LookAtKiller(self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary(self, inflictor, attacker);
		if (ctf->value)
			CTFFragBonuses(self, inflictor, attacker);

		// TODO: Make this work
		if (esp->value)
			EspScoreBonuses(self, attacker);

		//TossClientWeapon (self);
		TossItemsOnDeath(self);

		if (ctf->value)
			CTFDeadDropFlag(self);

		// let's be safe, if the player was killed and grapple disabled before it
		CTFPlayerResetGrapple(self);

		if (!teamplay->value)
			Cmd_Help_f(self);	// show scores

		/*
		Updated chase cam calls
		*/
		if (limchasecam->value < 2 && attacker && attacker->client) {
			// if (esp->value)
			// 	EspionageChaseCam(self, attacker);
			// else 
			if (teamplay->value)
				TeamplayChaseCam(self, attacker);
		}
	}
	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->client->uvTime = 0;

	FreeClientEdicts(self->client);

	// clean up sniper rifle stuff
	self->client->no_sniper_display = 0;
	self->client->resp.sniper_mode = SNIPER_1X;
	self->client->desired_fov = 90;
	self->client->ps.fov = 90;
	Bandage(self);		// clear up the leg damage when dead sound?
	self->client->bandage_stopped = 0;
	self->client->medkit = 0;

	// clear inventory
	memset(self->client->inventory, 0, sizeof(self->client->inventory));

	// zucc - check if they have a primed grenade
	if (self->client->curr_weap == GRENADE_NUM
	&& ((self->client->ps.gunframe >= GRENADE_IDLE_FIRST  && self->client->ps.gunframe <= GRENADE_IDLE_LAST)
	||  (self->client->ps.gunframe >= GRENADE_THROW_FIRST && self->client->ps.gunframe <= GRENADE_THROW_LAST)))
	{
		// Reset Grenade Damage to 1.52 when requested:
		int damrad = use_classic->value ? GRENADE_DAMRAD_CLASSIC : GRENADE_DAMRAD;
		self->client->ps.gunframe = 0;
		vec3_t non_const_origin; // Convert to non-const
      	VectorCopy(vec3_origin, non_const_origin);
		fire_grenade2( self, self->s.origin, non_const_origin, damrad, 0, 2 * HZ, damrad * 2, false );
	}

	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	// Gibbing on really hard HC hit
	if ((((self->health < -35) && (mod == MOD_HC)) ||
		((self->health < -20) && (mod == MOD_M3))) && (sv_gib->value)) {
		gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 5; n++)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead(self, damage);
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
		self->takedamage = DAMAGE_NO;
	} else {		// normal death
		if (!self->deadflag) {
			static int i;

			i = (i + 1) % 3;
			// start a death animation
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
				SetAnimation( self, FRAME_crdeath1 - 1, FRAME_crdeath5, ANIM_DEATH );
			else
				switch (i) {
				case 0:
					SetAnimation( self, FRAME_death101 - 1, FRAME_death106, ANIM_DEATH );
					break;
				case 1:
					SetAnimation( self, FRAME_death201 - 1, FRAME_death206, ANIM_DEATH );
					break;
				case 2:
					SetAnimation( self, FRAME_death301 - 1, FRAME_death308, ANIM_DEATH );
					break;
				}
			if ((mod == MOD_SNIPER) || (mod == MOD_KNIFE)
				|| (mod == MOD_KNIFE_THROWN)) {
				gi.sound(self, CHAN_VOICE, gi.soundindex("misc/glurp.wav"), 1, ATTN_NORM, 0);
				// TempFile - BEGIN sniper gibbing
				if (mod == MOD_SNIPER) {
					int n;

					switch (locOfDeath) {
					case LOC_HDAM:
						if (sv_gib->value) {
							for (n = 0; n < 8; n++)
								ThrowGib(self,
									 "models/objects/gibs/sm_meat/tris.md2",
									 damage, GIB_ORGANIC);
							ThrowClientHead(self, damage);
						}
					}
				}
			} else
				gi.sound(self, CHAN_VOICE,
					 gi.soundindex(va("*death%i.wav", (rand() % 4) + 1)), 1, ATTN_NORM, 0);
		}
	}

	// zucc this will fix a jump kick death generating a weapon
	self->client->curr_weap = MK23_NUM;

	self->client->resp.idletime = 0;

	// zucc solves problem of people stopping doors while in their dead bodies
	// ...only need it in DM though...
	// ...for teamplay, non-solid will get set soon after in CopyToBodyQue
	if (!(gameSettings & GS_ROUNDBASED)) {
		self->solid = SOLID_NOT;
	}

	self->deadflag = DEAD_DEAD;
	gi.linkentity(self);

	// in ctf, when a player dies check if he should be moved to the other team
	if(ctf->value)
		CheckForUnevenTeams(self);

	if (esp->value && IS_LEADER(self)) {
		if (!in_warmup && team_round_going) {
			EspReportLeaderDeath(self);
		}
	}
}

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float PlayersRangeFromSpot(edict_t * spot)
{
	edict_t *player;
	float playerdistance, bestplayerdistance = 9999999;
	int n;


	for (n = 1; n <= game.maxclients; n++) {
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;
	
		playerdistance = Distance(spot->s.origin, player->s.origin);
		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectAnyDeathmatchSpawnPoint

I just need a spawnpoint, any spawnpoint...
================
*/
edict_t *SelectAnyDeathmatchSpawnPoint(void)
{
    edict_t *spot = NULL;
    edict_t **spots = NULL;
    int count = 0;

	gi.dprintf("Warning: too few spawnpoints in this map\n");

    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        spots = realloc(spots, sizeof(*spots) * (count + 1));
        if (!spots) {
            // Handle memory allocation error
            return NULL;
        }
        spots[count++] = spot;
    }

    if (count == 0) {
        // No DM spawns found, womp womp
        return NULL;
    }

    // Select a random spot
    spot = spots[rand() % count];

    // Free the spots array
    free(spots);

    return spot;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint(void)
{
	edict_t *spot, *spot1, *spot2;
	int count = 0;
	int selection;
	float range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1) {
			if (range1 < range2) {
				range2 = range1;
				spot2 = spot1;
			}
			range1 = range;
			spot1 = spot;
		} else if (range < range2) {
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2) {
		return (rand() % count) ? spot2 : spot1;
	}

	
	count -= 2;

	selection = rand() % count;

	spot = NULL;
	do {
		spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	}
	while (selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint(void)
{
	edict_t *bestspot;
	float bestdistance, bestplayerdistance;
	edict_t *spot;

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		bestplayerdistance = PlayersRangeFromSpot(spot);

		if (bestplayerdistance > bestdistance) {
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot) {
		return bestspot;
	}
	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint(void)
{
	edict_t *spot = NULL;

	if (DMFLAGS(DF_SPAWN_FARTHEST))
		spot = SelectFarthestDeathmatchSpawnPoint();
	else
		spot = SelectRandomDeathmatchSpawnPoint();

	return spot;
}

/*
UncommonSpawnPoint

This is used when a map does not have the appropriate spawn points for the
game mode being played.  This is used to prevent server crashes if a player
cannot spawn.
*/

edict_t *UncommonSpawnPoint(void)
{
	edict_t *spot = NULL;

	if (!spot) {
		gi.dprintf("Warning: failed to find deathmatch spawn point, unexpected spawns will be utilized\n");

		/*
		Try all possible classes of spawn points, and use DM weapon spawns as a last resort.
		*/
		char* spawnpoints[] = {
			"info_player_start",
			"info_player_coop",
			"info_player_team1",
			"info_player_team2",
			"info_player_team3",
			"info_player_deathmatch",
			"weapon_bfg",
			"weapon_chaingun",
			"weapon_machinegun",
			"weapon_rocketlauncher",
			"weapon_shotgun",
			"weapon_supershotgun",
			"weapon_railgun"
		};
		size_t num_spawnpoints = sizeof(spawnpoints) / sizeof(spawnpoints[0]);
		int i;
		for (i = 0; i < num_spawnpoints; ++i) {
			while ((spot = G_Find(spot, FOFS(classname), spawnpoints[i])) != NULL) {
				if (!game.spawnpoint[0] && !spot->targetname)
					break;

				if (!game.spawnpoint[0] || !spot->targetname)
					continue;

				if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
					break;
			}

			if (spot) {
				gi.dprintf("Warning: Uncommon spawn point of class %s\n", spawnpoints[i]);
				gi.dprintf("**If you are the map author, you need to be utilizing MULTIPLE info_player_deathmatch or info_player_team entities**\n");
				break;
			}
		}
	}

	return spot;
}

edict_t *SelectCoopSpawnPoint(edict_t *ent)
{
    int     index;
    edict_t *spot = NULL;
    char    *target;

    index = ent->client - game.clients;

    // player 0 starts in normal player spawn point
    if (!index)
        return NULL;

    spot = NULL;

    // assume there are four coop spots at each spawnpoint
    while (1) {
        spot = G_Find(spot, FOFS(classname), "info_player_coop");
        if (!spot)
            return NULL;    // we didn't have enough...

        target = spot->targetname;
        if (!target)
            target = "";
        if (Q_stricmp(game.spawnpoint, target) == 0) {
            // this is a coop spawn point for one of the clients here
            index--;
            if (!index)
                return spot;        // this is it
        }
    }

    return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
Espionage uses custom spawn points and custom respawns, but only
once a round has started, otherwise it uses the normal deathmatch
chosen ones
============
*/
void SelectSpawnPoint(edict_t * ent, vec3_t origin, vec3_t angles)
{
	edict_t *spot = NULL;

	//FIREBLADE
	if (coop->value){
		spot = SelectCoopSpawnPoint(ent);
	} else if (ctf->value) {
		spot = SelectCTFSpawnPoint(ent);
	} else if (esp->value) {
		// SelectEspSpawnPoint handles on-leader respawns as well as initial spawnpoints
		spot = SelectEspSpawnPoint(ent);
	} else if (dom->value) {
		spot = SelectDeathmatchSpawnPoint();
	} else if (!(gameSettings & GS_DEATHMATCH) && ent->client->resp.team && !in_warmup) {
		spot = SelectTeamplaySpawnPoint(ent);
	} else if (jump->value) {
		spot = SelectFarthestDeathmatchSpawnPoint();
	} else {
		spot = SelectDeathmatchSpawnPoint();
	}

	// desperation mode, find a spawnpoint or the server will crash
	if (!spot)
		spot = UncommonSpawnPoint();

	// Last resort, just choose any info_player_deathmatch even if it turns into
	// a messy telefrag nightmare
	if (!spot)
		spot = SelectAnyDeathmatchSpawnPoint();

	// If still no spot, then this map is just not playable, sorry
	if (!spot)
		Com_Error(ERR_DROP, "Couldn't find spawn point, map is not playable!");
			
	VectorCopy(spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy(spot->s.angles, angles);
}

//======================================================================

void InitBodyQue(void)
{
	int i;
	edict_t *ent;

	level.body_que = 0;
	for (i = 0; i < BODY_QUEUE_SIZE; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die(edict_t * self, edict_t * inflictor, edict_t * attacker, int damage, vec3_t point)
{
/*      int     n;*/

	if (self->health < -40) {
		// remove gibbing
/*                gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                for (n= 0; n < 4; n++)
                        ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
                self->s.origin[2] -= 48;
                ThrowClientHead (self, damage);*/
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue(edict_t * ent)
{
	edict_t *body;

	// grab a body que and cycle to the next one
	body = &g_edicts[game.maxclients + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity(ent);

	gi.unlinkentity(body);

	body->s = ent->s;
	body->s.number = body - g_edicts;
	body->s.event = EV_OTHER_TELEPORT;
	VectorCopy( body->s.origin, body->s.old_origin );
	VectorCopy( body->s.origin, body->old_origin );

	body->svflags = ent->svflags;
	VectorCopy(ent->mins, body->mins);
	VectorCopy(ent->maxs, body->maxs);
	VectorCopy(ent->absmin, body->absmin);
	VectorCopy(ent->absmax, body->absmax);
	VectorCopy(ent->size, body->size);
	// All our bodies will be non-solid -FB
	body->solid = SOLID_NOT;
	//body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;

//FB 5/31/99
	body->movetype = MOVETYPE_TOSS;	// just in case?
//        body->movetype = ent->movetype;
	VectorCopy(ent->velocity, body->velocity);
	body->mass = ent->mass;
	body->groundentity = NULL;
//FB 5/31/99
//FB 6/1/99
	body->s.renderfx = 0;
//FB

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

//PG BUND - BEGIN
	//Disable to be seen by irvision
	body->s.renderfx &= ~RF_IR_VISIBLE;
//PG BUND - END

	gi.linkentity(body);
}

void CleanBodies(void)
{
	int i;
	edict_t *ent;

	ent = g_edicts + game.maxclients + 1;
	for (i = 0; i < BODY_QUEUE_SIZE; i++, ent++) {
		gi.unlinkentity( ent );
		ent->solid = SOLID_NOT;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->svflags |= SVF_NOCLIENT;
	}
	level.body_que = 0;
}

void respawn(edict_t *self)
{
	//gi.dprintf("%s tried to respawn\n", self->client->pers.netname);

	if (self->solid != SOLID_NOT || self->deadflag == DEAD_DEAD)
		CopyToBodyQue(self);

	PutClientInServer(self);

	if (!(self->svflags & SVF_NOCLIENT))
	{
		if (team_round_going && !(gameSettings & GS_ROUNDBASED))
			AddToTransparentList(self);

		if (respawn_effect->value) {
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(self - g_edicts);
			gi.WriteByte(MZ_RESPAWN);
			gi.multicast(self->s.origin, MULTICAST_PVS);

			/*// add a teleportation effect
			self->s.event = EV_PLAYER_TELEPORT;

			// hold in place briefly
			self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
			self->client->ps.pmove.pm_time = 14;*/
		}
	}

	self->client->respawn_framenum = level.framenum + 2 * HZ;

	
	if (esp->value && team_round_going){
		// Optional respawn invulnerability in Espionage
		if (esp_respawn_uvtime->value){
			if (esp_debug->value)
				gi.dprintf("%s: Invuln activated\n", __FUNCTION__);
			self->client->uvTime = (int)esp_respawn_uvtime->value;
		}
	}
}

//==============================================================

void AllWeapons(edict_t * ent)
{
	int i;
	gitem_t *it;

	for (i = 0; i < game.num_items; i++) {
		it = itemlist + i;
		if (!it->pickup)
			continue;
		if (!(it->flags & IT_WEAPON))
			continue;

		if (!it->typeNum || !WPF_ALLOWED(it->typeNum))
			continue;

		switch(it->typeNum) {
		case MK23_NUM:
			ent->client->inventory[i] = 1;
			ent->client->mk23_rds = ent->client->mk23_max;
			break;
		case MP5_NUM:
			ent->client->inventory[i] = 1;
			ent->client->mp5_rds = ent->client->mp5_max;
			break;
		case M4_NUM:
			ent->client->inventory[i] = 1;
			ent->client->m4_rds = ent->client->m4_max;	    
			break;
		case M3_NUM:
			ent->client->inventory[i] = 1;
			ent->client->shot_rds = ent->client->shot_max;
			break;
		case HC_NUM:
			ent->client->inventory[i] = 1;
			ent->client->cannon_rds = ent->client->cannon_max;
			ent->client->shot_rds = ent->client->shot_max;
			break;
		case SNIPER_NUM:
			ent->client->inventory[i] = 1;
			ent->client->sniper_rds = ent->client->sniper_max;
			break;
		case DUAL_NUM:
			ent->client->inventory[i] = 1;
			ent->client->dual_rds = ent->client->dual_max;
			break;
		case KNIFE_NUM:
			ent->client->inventory[i] = 10;
			break;
		case GRENADE_NUM:
			ent->client->inventory[i] = tgren->value;
			break;
		}
	}
	
	for (i = 0; i < game.num_items; i++) {
		it = itemlist + i;
		if (!it->pickup)
			continue;
		if (!(it->flags & IT_AMMO))
			continue;
		Add_Ammo(ent, it, 1000);
	}
}

void AllItems(edict_t * ent)
{
	edict_t etemp;
	int i;
	gitem_t *it;

	for (i = 0; i < game.num_items; i++) {
		it = itemlist + i;
		if (!it->pickup)
			continue;
		if (!(it->flags & IT_ITEM))
			continue;

		etemp.item = it;

		if (ent->client->unique_item_total >= unique_items->value)
			ent->client->unique_item_total = unique_items->value - 1;
		Pickup_Special(&etemp, ent);
	}

}

// equips a client with item/weapon in teamplay

void EquipClient(edict_t * ent)
{
	gclient_t *client;
	gitem_t *item;
	edict_t etemp;
	int band = 0, itemNum = 0;

	client = ent->client;

	if(use_grapple->value)
		client->inventory[ITEM_INDEX(FindItem("Grapple"))] = 1;

	// Honor changes to wp_flags and itm_flags.
	if( client->pers.chosenWeapon && ! WPF_ALLOWED(client->pers.chosenWeapon->typeNum) )
		client->pers.chosenWeapon = NULL;
	if( client->pers.chosenItem && ! ITF_ALLOWED(client->pers.chosenItem->typeNum) )
		client->pers.chosenItem = NULL;

	if (client->pers.chosenItem) {
		if (client->pers.chosenItem->typeNum == BAND_NUM) {
			band = 1;
			if (tgren->value > 0)	// team grenades is turned on
			{
				item = GET_ITEM(GRENADE_NUM);
				client->inventory[ITEM_INDEX(item)] = tgren->value;
			}
		}
	}

	// set them up with initial pistol ammo
	if (WPF_ALLOWED(MK23_ANUM)) {
		item = GET_ITEM(MK23_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 2;
		else
			client->inventory[ITEM_INDEX(item)] = 1;
	}

	itemNum = client->pers.chosenWeapon ? client->pers.chosenWeapon->typeNum : 0;

	switch (itemNum) {
	case MP5_NUM:
		item = GET_ITEM(MP5_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->curr_weap = MP5_NUM;
		client->unique_weapon_total = 1;
		item = GET_ITEM(MP5_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 2;
		else
			client->inventory[ITEM_INDEX(item)] = 1;
		client->mp5_rds = client->mp5_max;
		break;
	case M4_NUM:
		item = GET_ITEM(M4_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->curr_weap = M4_NUM;
		client->unique_weapon_total = 1;
		item = GET_ITEM(M4_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 2;
		else
			client->inventory[ITEM_INDEX(item)] = 1;
		client->m4_rds = client->m4_max;
		break;
	case M3_NUM:
		item = GET_ITEM(M3_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->curr_weap = M3_NUM;
		client->unique_weapon_total = 1;
		item = GET_ITEM(SHELL_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 14;
		else
			client->inventory[ITEM_INDEX(item)] = 7;
		client->shot_rds = client->shot_max;
		break;
	case HC_NUM:
		item = GET_ITEM(HC_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->curr_weap = HC_NUM;
		client->unique_weapon_total = 1;
		item = GET_ITEM(SHELL_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 24;
		else
			client->inventory[ITEM_INDEX(item)] = 12;
		client->cannon_rds = client->cannon_max;
		break;
	case SNIPER_NUM:
		item = GET_ITEM(SNIPER_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[ITEM_INDEX(item)] = 1;
		client->weapon = item;
		client->curr_weap = SNIPER_NUM;
		client->unique_weapon_total = 1;
		item = GET_ITEM(SNIPER_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 20;
		else
			client->inventory[ITEM_INDEX(item)] = 10;
		client->sniper_rds = client->sniper_max;
		break;
	case DUAL_NUM:
		item = GET_ITEM(DUAL_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->curr_weap = DUAL_NUM;
		item = GET_ITEM(MK23_ANUM);
		if (band)
			client->inventory[ITEM_INDEX(item)] = 4;
		else
			client->inventory[ITEM_INDEX(item)] = 2;
		client->dual_rds = client->dual_max;
		break;
	case KNIFE_NUM:
		item = GET_ITEM(KNIFE_NUM);
		client->selected_item = ITEM_INDEX(item);
		if (band)
			client->inventory[client->selected_item] = 20;
		else
			client->inventory[client->selected_item] = 10;
		client->weapon = item;
		client->curr_weap = KNIFE_NUM;
		break;
	}

	memset(&etemp, 0, sizeof(etemp));
	if (client->pers.chosenItem) {
		etemp.item = client->pers.chosenItem;
		Pickup_Special(&etemp, ent);
	}
	if (item_kit_mode->value && client->pers.chosenItem2){
		client->inventory[ITEM_INDEX(client->pers.chosenItem2)] = 1;
		client->unique_item_total++;
	}

}

// Igor[Rock] start
void EquipClientDM(edict_t * ent)
{
	gclient_t *client;
	gitem_t *item;
	int itemNum = 0;

	client = ent->client;

	if(use_grapple->value)
		client->inventory[ITEM_INDEX(FindItem("Grapple"))] = 1;

	if (*strtwpn->string)
		itemNum = GetWeaponNumFromArg(strtwpn->string);

	// Give some ammo for the weapon
	switch (itemNum) {
	case MK23_NUM:
		return;
	case MP5_NUM:
		item = GET_ITEM(MP5_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->mp5_rds = client->mp5_max;
		client->curr_weap = MP5_NUM;
		if (!allweapon->value) {
			client->unique_weapon_total = 1;
		}
		item = GET_ITEM(MP5_ANUM);
		client->inventory[ITEM_INDEX(item)] = 1;
		break;
	case M4_NUM:
		item = GET_ITEM(M4_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->m4_rds = client->m4_max;
		client->curr_weap = M4_NUM;
		if (!allweapon->value) {
			client->unique_weapon_total = 1;
		}
		item = GET_ITEM(M4_ANUM);
		client->inventory[ITEM_INDEX(item)] = 1;
		break;
	case M3_NUM:
		item = GET_ITEM(M3_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->shot_rds = client->shot_max;
		client->curr_weap = M3_NUM;
		if (!allweapon->value) {
			client->unique_weapon_total = 1;
		}
		item = GET_ITEM(SHELL_ANUM);
		client->inventory[ITEM_INDEX(item)] = 7;
		break;
	case HC_NUM:
		item = GET_ITEM(HC_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->cannon_rds = client->cannon_max;
		client->shot_rds = client->shot_max;
		client->curr_weap = HC_NUM;
		if (!allweapon->value) {
			client->unique_weapon_total = 1;
		}
		item = GET_ITEM(SHELL_ANUM);
		client->inventory[ITEM_INDEX(item)] = 12;
		break;
	case SNIPER_NUM:
		item = GET_ITEM(SNIPER_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->sniper_rds = client->sniper_max;
		client->curr_weap = SNIPER_NUM;
		if (!allweapon->value) {
			client->unique_weapon_total = 1;
		}
		item = GET_ITEM(SNIPER_ANUM);;
		client->inventory[ITEM_INDEX(item)] = 10;
		break;
	case DUAL_NUM:
		item = GET_ITEM(DUAL_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 1;
		client->weapon = item;
		client->dual_rds = client->dual_max;
		client->mk23_rds = client->mk23_max;
		client->curr_weap = DUAL_NUM;
		item = GET_ITEM(MK23_ANUM);
		client->inventory[ITEM_INDEX(item)] = 2;
		break;
	case GRENADE_NUM:
		item = GET_ITEM(GRENADE_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = tgren->value;
		client->weapon = item;
		client->curr_weap = GRENADE_NUM;
		break;
	case KNIFE_NUM:
		item = GET_ITEM(KNIFE_NUM);
		client->selected_item = ITEM_INDEX(item);
		client->inventory[client->selected_item] = 10;
		client->weapon = item;
		client->curr_weap = KNIFE_NUM;
		break;
	}
}

// Igor[Rock] ende


/*
===========
ClientLegDamage

Called when a player takes leg damage
============
*/

void ClientLegDamage(edict_t *ent)
{
	ent->client->leg_damage = 1;
	ent->client->leghits++;

	if (esp_enhancedslippers->value && INV_AMMO(ent, SLIP_NUM)) { // we don't limp with enhanced slippers, so just ignore this leg damage.
		ent->client->leg_damage = 0;
		return;
	}

	// Reki: limp_nopred behavior
	switch (ent->client->pers.limp_nopred & 255)
	{
		case 0:
			break;
		case 2:
			if (sv_limp_highping->value <= 0)
				break;
			// if the 256 bit flag is set, we have to be cautious to only deactivate if ping swung significantly
			// so each leg break doesn't flipflop between behavior if client ping is fluctuating
			if (ent->client->pers.limp_nopred & 256)
			{
				if (ent->client->ping < (int)sv_limp_highping->value - 15)
				{
					ent->client->pers.limp_nopred &= ~256;
					break;
				}
			}
			else if (ent->client->ping < (int)sv_limp_highping->value)
				break;
			ent->client->pers.limp_nopred |= 256;
		case 1:
			if (esp_enhancedslippers->value && INV_AMMO(ent, SLIP_NUM)) // we don't limp with enhanced slippers, so just ignore this leg damage.
				break;

			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			break;
	}
	//

}

void ClientFixLegs(edict_t *ent)
{
	if (ent->client->leg_damage && ent->client->ctf_grapplestate <= CTF_GRAPPLE_STATE_FLY)
	{
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	}

	ent->client->leg_noise = 0;
	ent->client->leg_damage = 0;
	ent->client->leghits = 0;
	ent->client->leg_dam_count = 0;
}



/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/

void PutClientInServer(edict_t * ent)
{
	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };
	int index, going_observer, i;
	vec3_t spawn_origin, spawn_angles;
	gclient_t *client;
	client_persistant_t pers;
	client_respawn_t resp;
	gitem_t *item;
#if AQTION_EXTENSION
	cvarsyncvalue_t cl_cvar[CVARSYNC_MAX];
#endif

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint(ent, spawn_origin, spawn_angles);

	index = ent - g_edicts - 1;
	client = ent->client;

	FreeClientEdicts(client);

	// deathmatch wipes most client data every spawn
	resp = client->resp;
	pers = client->pers;
#if AQTION_EXTENSION
	memcpy(cl_cvar, client->cl_cvar, sizeof(client->cl_cvar));
#endif

	memset(client, 0, sizeof(*client));

#if AQTION_EXTENSION
	memcpy(client->cl_cvar, cl_cvar, sizeof(client->cl_cvar));
#endif
	client->pers = pers;
	client->resp = resp;


	client->clientNum = index;

	//zucc give some ammo
	// changed to mk23
	item = GET_ITEM( MK23_NUM );
	client->selected_item = ITEM_INDEX( item );
	client->inventory[client->selected_item] = 1;

	client->weapon = item;
	client->lastweapon = item;

	if (WPF_ALLOWED( KNIFE_NUM )) {
		item = GET_ITEM( KNIFE_NUM );
		client->inventory[ITEM_INDEX( item )] = 1;
		if (!WPF_ALLOWED( MK23_NUM )) {
			client->selected_item = ITEM_INDEX( item );
			client->weapon = item;
			client->lastweapon = item;
		}
	}
	client->curr_weap = client->weapon->typeNum;


	ent->health = 100;
	ent->max_health = 100;
	
	client->max_pistolmags = 2;
	client->max_shells = 14;
	client->max_mp5mags = 2;
	client->max_m4mags = 1;
	client->max_sniper_rnds = 20;

	client->knife_max = 10;
	client->grenade_max = 2;

	client->mk23_max = 12;
	client->mp5_max = 30;
	client->m4_max = 24;
	client->shot_max = 7;
	client->sniper_max = 6;
	client->cannon_max = 2;
	client->dual_max = 24;
	if (WPF_ALLOWED( MK23_NUM )) {
		client->mk23_rds = client->mk23_max;
		client->dual_rds = client->mk23_max;
	}

	client->knife_max = 10;
	client->grenade_max = 2;
	client->desired_fov = 90;


	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished_framenum = level.framenum + 12 * HZ;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~(FL_NO_KNOCKBACK | FL_GODMODE);
	ent->svflags &= ~(SVF_DEADMONSTER | SVF_NOCLIENT);

	VectorCopy(mins, ent->mins);
	VectorCopy(maxs, ent->maxs);
	VectorClear(ent->velocity);

	// clear playerstate values
	client->ps.fov = 90;
	client->ps.gunindex = gi.modelindex(client->weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;	// will use the skin specified model

#if AQTION_EXTENSION
	// teammate indicator arrows
	if (use_indicators->value && teamplay->value && !client->arrow && client->resp.team)
	{
		client->arrow = G_Spawn();
		client->arrow->solid = SOLID_NOT;
		client->arrow->movetype = MOVETYPE_NOCLIP;
		client->arrow->classname = "ind_arrow";
		client->arrow->owner = ent;
	}
#endif

	// zucc vwep
	//ent->s.modelindex2 = 255;             // custom gun model
	ShowGun(ent);

	ent->s.frame = 0;
	VectorCopy(spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy(ent->s.origin, ent->s.old_origin);
	VectorCopy(ent->s.origin, ent->old_origin);

	client->ps.pmove.origin[0] = ent->s.origin[0] * 8;
	client->ps.pmove.origin[1] = ent->s.origin[1] * 8;
	client->ps.pmove.origin[2] = ent->s.origin[2] * 8;

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy(ent->s.angles, client->ps.viewangles);
	VectorCopy(ent->s.angles, client->v_angle);

	// set the delta angle
	for (i = 0; i < 3; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->s.angles[i] - client->resp.cmd_angles[i]);


#ifndef NO_BOTS

	//rekkie -- s
	if (ent->is_bot)
	{
		if (ent->bot.bot_type == BOT_TYPE_BOTLIB) // BOTLIB
		{
			BOTLIB_Init(ent); // Initialize all the bot variables
			if (!bot_personality->value) {
				BOTLIB_SmartWeaponSelection(ent); // This is an excellent way to choose good weapon variety
			} else {
				// If using personalities, they have their own weapon/item preferences
				BOTLIB_BotPersonalityChooseWeapon(ent);
				if (item_kit_mode->value)
					BOTLIB_BotPersonalityChooseItemKit(ent);
				else
					BOTLIB_BotPersonalityChooseItem(ent);
			}
			// gi.dprintf("Bot %s has chosen %s and %s\n", 
			// 	ent->client->pers.netname, 
			// 	PrintWeaponName(ent->client->pers.chosenWeapon->typeNum),
			// 	PrintItemName(ent->client->pers.chosenItem->typeNum)
			// );
		}
		else // LTK bots
		{
			ent->bot.bot_type = BOT_TYPE_LTK; // Set LTK
			//rekkie -- e
			
			ent->classname = "bot";

			ent->last_node = -1;
			ent->is_jumping = false;
			ent->is_triggering = false;
			ent->grenadewait = 0;
			ent->react = 0.f;

			ent->next_move_time = level.framenum;
			ent->suicide_timeout = level.framenum + 15.0 * HZ;

			ent->killchat = false;
			VectorClear(ent->lastSeen);
			ent->cansee = false;

			ent->bot_speed = 0;
			VectorClear(ent->lastPosition);

			if (teamplay->value)
			{
				const char* s = Info_ValueForKey(ent->client->pers.userinfo, "skin");
				AssignSkin(ent, s, false /* nickChanged */);
				// Anti centipede timer
				ent->teamPauseTime = level.framenum + (3.0 + (rand() % 7)) * HZ;
				// Radio setup
				ent->teamReportedIn = true;
				ent->lastRadioTime = level.framenum;
				// Change facing angle for each bot
				if (numnodes)
				{
					int randomnode = (int)(num_players * random());
					VectorSubtract(nodes[randomnode].origin, ent->s.origin, ent->move_vector);
				}
				ent->move_vector[2] = 0;
			}
			else
				ent->teamPauseTime = level.framenum;

			//RiEvEr - new node pathing system
			memset(&(ent->pathList), 0, sizeof(ent->pathList));
			ent->pathList.head = ent->pathList.tail = NULL;
			//R

			ent->client->resp.radio.gender = (ent->client->pers.gender == GENDER_FEMALE) ? 1 : 0;
		}
	}
	
	if( (! ent->is_bot) || (ent->think != ACESP_HoldSpawn) )  // if( respawn )
	{
#endif

	if (teamplay->value) {
		going_observer = (!ent->client->resp.team || ent->client->resp.subteam);
	}
	else {
		going_observer = ent->client->pers.spectator;
		if (dm_choose->value && !ent->client->pers.dm_selected)
			going_observer = 1;
	}

	if (going_observer || level.intermission_framenum) {
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;

#if AQTION_EXTENSION
		if (!ent->client->resp.team)
			HUD_SetType(ent, 1);
#endif
		gi.linkentity(ent);
		return;
	}

#ifndef NO_BOTS
	}  // end if( respawn )
#endif

#if AQTION_EXTENSION
	HUD_SetType(ent, -1);
#endif

	if (!teamplay->value) {	// this handles telefrags...
		KillBox(ent);
	} else {
		ent->solid = SOLID_TRIGGER;
	}

	gi.linkentity( ent );

	if ((int)uvtime->value > 0) {
		if (teamplay->value && ! in_warmup) {
			if (!(gameSettings & GS_ROUNDBASED) && team_round_going && !lights_camera_action) {
				client->uvTime = uvtime->value;
			}
		} else if (dm_shield->value) {
			client->uvTime = uvtime->value;
		}
	}

	ent->client->medkit = 0;

	if( jump->value )
	{
		Jmp_EquipClient(ent);
		return;
	}

	// items up here so that the bandolier will change equipclient below
	if (allitem->value)
		AllItems(ent);

	if (gameSettings & GS_WEAPONCHOOSE)
		EquipClient(ent);
	else
		EquipClientDM(ent);

	if (allweapon->value)
		AllWeapons(ent);

	/*
	Team leaders in Espionage can receive different loadouts
	0 - No different loadout
	1 - Leaders get AllItems (default)
	2 - Leaders get AllWeapons
	3 - Leaders get AllItems and AllWeapons

	With leaderenhance enabled, the leaders also get a medkit
	*/
	if (esp->value) {
		if (esp_leaderequip->value){
			for (i = TEAM1; i <= teamCount; i++){
				if (ent == teams[i].leader) {
					if (esp_leaderequip->value == 1) {
						AllItems(ent);
					} else if (esp_leaderequip->value == 2) {
						AllWeapons(ent);
					} else if (esp_leaderequip->value == 3) {
						AllItems(ent);
						AllWeapons(ent);
					}
				}
			}
		}
		if (esp_leaderenhance->value){
			for (i = TEAM1; i <= teamCount; i++){
				if (ent == teams[i].leader) {
					if (esp_leaderequip->value == 1) {
						ent->client->medkit = 1;
					}
				}
			}
		}
		// For respawn-on-leader, if leader is ducking, client should also be ducking
		if (team_round_going && HAVE_LEADER(ent->client->resp.team)){
			ent->client->ps.pmove.pm_flags |= (teams[ent->client->resp.team].leader->client->ps.pmove.pm_flags & PMF_DUCKED);
		}
	}

	// force the current weapon up
	client->newweapon = client->weapon;
	ChangeWeapon(ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch(edict_t * ent)
{
	int checkFrame, saved_team = ent->client->resp.team;

	G_InitEdict(ent);

	memset(&ent->client->resp, 0, sizeof(ent->client->resp));

	ent->client->resp.enterframe = level.framenum;
	ent->client->resp.gldynamic = 1;
	
#if AQTION_EXTENSION
	if (teamplay->value)
	{
		HUD_SetType(ent, 1);
	}
	else
	{
		HUD_SetType(ent, -1);
	}
#endif

	if (!ent->client->pers.connected) {
		ent->client->pers.connected = true;
		ClientUserinfoChanged(ent, ent->client->pers.userinfo);
	}

	//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
	ent->client->pers.draw = gix->Draw();
	//if (ent->client->pers.draw)
	{
		// Default all to off state
		//ent->client->pers.draw->arrows_inuse = false;
		//ent->client->pers.draw->boxes_inuse = false;
		//ent->client->pers.draw->crosses_inuse = false;
		//ent->client->pers.draw->strings_inuse = false;
	}
#endif
	//rekkie -- debug drawing -- e

	// clear weapons and items if not auto_equipt
	if (!auto_equip->value || !(gameSettings & GS_WEAPONCHOOSE)) {
		ent->client->pers.chosenWeapon = NULL;
		ent->client->pers.chosenItem = NULL;
		ent->client->pers.dm_selected = 0;
		ent->client->pers.menu_shown = 0;
	} else {
		if (teamplay->value)
			ent->client->pers.menu_shown = 0;
	}

	if (!dm_choose->value && !warmup->value) {
		if (!ent->client->pers.chosenWeapon) {
			if (WPF_ALLOWED(MP5_NUM))
				ent->client->pers.chosenWeapon = GET_ITEM(MP5_NUM);
			else if (WPF_ALLOWED(MK23_NUM))
				ent->client->pers.chosenWeapon = GET_ITEM(MK23_NUM);
			else if (WPF_ALLOWED(KNIFE_NUM))
				ent->client->pers.chosenWeapon = GET_ITEM(KNIFE_NUM);
			else
				ent->client->pers.chosenWeapon = GET_ITEM(MK23_NUM);
		}
		if (!ent->client->pers.chosenItem)
			ent->client->pers.chosenItem = GET_ITEM(KEV_NUM);
	} else {
		if (wp_flags->value < 2 && !ent->client->pers.chosenWeapon)
			ent->client->pers.chosenWeapon = GET_ITEM(MK23_NUM);
	}


	TourneyNewPlayer(ent);
	vInitClient(ent);

#ifndef NO_BOTS
    	ACEIT_RebuildPlayerList();
		if (ent->is_bot)
			BOTLIB_SKILL_Init(ent); // Initialize the skill levels

#if USE_AQTION
		StatBotCheck();
#endif
#endif

	// locate ent at a spawn point
	PutClientInServer(ent);

	if (level.intermission_framenum) {
		MoveClientToIntermission(ent);
	} else {
		if (!teamplay->value && !dm_choose->value) {	//FB 5/31/99
			// send effect
			gi.WriteByte(svc_muzzleflash);
			gi.WriteShort(ent - g_edicts);
			gi.WriteByte(MZ_LOGIN);
			gi.multicast(ent->s.origin, MULTICAST_PVS);
		}
	}

	gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
	IRC_printf(IRC_T_SERVER, "%n entered the game", ent->client->pers.netname);

	// TNG:Freud Automaticly join saved teams.
	if (saved_team && auto_join->value == 1 && teamplay->value)
		JoinTeam(ent, saved_team, 1);
	else if (auto_join->value == 2 && teamplay->value)
		JoinTeamAutobalance(ent);


	if (!level.intermission_framenum) {
		if (!teamplay->value && ent->solid == SOLID_NOT) {
			gi.bprintf(PRINT_HIGH, "%s became a spectator\n", ent->client->pers.netname);
			IRC_printf(IRC_T_SERVER, "%n became a spectator", ent->client->pers.netname);
		}
		PrintMOTD(ent);
	}

	if(am->value && game.bot_count > 0){
		char msg[128];
		Q_snprintf(msg, sizeof(msg), "** This server contains BOTS for you to play with until real players join up!  Enjoy! **");
		gi.centerprintf(ent, "%s", msg);
	}

	ent->client->resp.motd_refreshes = 1;

	//AQ2:TNG - Slicer: Set time to check clients
	checkFrame = level.framenum + (int)(check_time->value * HZ);
	ent->client->resp.checkframe[0] = checkFrame;
	ent->client->resp.checkframe[1] = checkFrame + 2 * HZ;
	ent->client->resp.checkframe[2] = checkFrame + 3 * HZ;

	G_UpdatePlayerStatusbar(ent, 1);

	// make sure all view stuff is valid
	ClientEndServerFrame(ent);
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin(edict_t * ent)
{
	ent->client = game.clients + (ent - g_edicts - 1);

	ClientBeginDeathmatch(ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged(edict_t *ent, char *userinfo)
{
	char *s, tnick[16];
	qboolean nickChanged = false;
	gclient_t *client = ent->client;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo)) {
		strcpy(userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}
	// set name
	s = Info_ValueForKey(userinfo, "name");
	Q_strncpyz(tnick, s, sizeof(tnick));
	if(!tnick[0])
		strcpy(tnick, "unnamed");

	if (strcmp(client->pers.netname, tnick))
	{
		// on the initial update, we won't broadcast the message.
		if (!client->pers.mvdspec && client->pers.netname[0])
		{
			size_t i = 1;
			for( ; i <= game.maxclients; i ++ )
			{
				edict_t *other = &g_edicts[i];
				if( ! other->inuse || ! other->client )
					continue;
				if( team_round_going && (gameSettings & GS_ROUNDBASED) && ! deadtalk->value && ! IS_ALIVE(ent) && IS_ALIVE(other) )
					continue;
				if( IsInIgnoreList( other, ent ) )
					continue;
				gi.cprintf( other, PRINT_MEDIUM, "%s is now known as %s.\n", client->pers.netname, tnick ); //TempFile
			}
			if( dedicated->value )
				gi.dprintf( "%s is now known as %s.\n", client->pers.netname, tnick ); //TempFile
			IRC_printf(IRC_T_SERVER, "%n is now known as %n.", client->pers.netname, tnick);
			nickChanged = true;
		}
		strcpy(client->pers.netname, tnick);
	}
  
	if (client->pers.mvdspec) {
		client->pers.spectator = true;
	} else {
		if (!teamplay->value) {
			s = Info_ValueForKey(userinfo, "spectator");
			client->pers.spectator = (strcmp(s, "0") != 0);
		}

		// set skin
		s = Info_ValueForKey(userinfo, "skin");

		// AQ:TNG - JBravo fixing $$ Skin server crash bug
		if (strstr(s, "$$")) {
			Info_SetValueForKey(userinfo, "skin", "male/grunt");
			s = Info_ValueForKey(userinfo, "skin");
		}

		// combine name and skin into a configstring
		AssignSkin(ent, s, nickChanged);
	}

	client->ps.fov = 90;

	client->pers.firing_style = ACTION_FIRING_CENTER;
	// handedness
	s = Info_ValueForKey(userinfo, "hand");
	if (strlen(s)) {
		client->pers.hand = atoi(s);
		if (strstr(s, "classic high") != NULL)
			client->pers.firing_style = ACTION_FIRING_CLASSIC_HIGH;
		else if (strstr(s, "classic") != NULL)
			client->pers.firing_style = ACTION_FIRING_CLASSIC;
	}
	// save off the userinfo in case we want to check something later
	Q_strncpyz(client->pers.userinfo, userinfo, sizeof(client->pers.userinfo));

	s = Info_ValueForKey( client->pers.userinfo, "gender" );
	if (s[0] == 'f' || s[0] == 'F') {
		client->pers.gender = GENDER_FEMALE;
	} else if (s[0] == 'm' || s[0] == 'M') {
		client->pers.gender = GENDER_MALE;
	} else {
		client->pers.gender = GENDER_NEUTRAL;
	}


	// Reki - disable prediction on limping
#if AQTION_EXTENSION
	if (Client_GetProtocol(ent) == 38) // if we're using AQTION protocol, we have limp prediction
	{
		client->pers.limp_nopred = 0;
	}
	else
	{
#endif
		s = Info_ValueForKey(userinfo, "limp_nopred");
		int limp = atoi(s);
		if (limp == 1)
			client->pers.limp_nopred = 1; // client explicity wants new behavior 
		else if (s[0] == 0)
			client->pers.limp_nopred = 2 | (client->pers.limp_nopred & 256); // client doesn't specify, so use auto threshold
		else if (limp == 0)
			client->pers.limp_nopred = 0; // client explicity wants old behavior
#if AQTION_EXTENSION
	}
#endif


#if AQTION_EXTENSION
	if (!HAS_CVARSYNC(ent)) // only do these cl cvars if cvarsync isn't a thing, since it's much better than userinfo
	{
#endif
		// Reki - spectator options, force team overlay/send easily parsable kill feed prints
		s = Info_ValueForKey(userinfo, "cl_spectatorhud");
		if (atoi(s))
			client->pers.spec_flags |= SPECFL_SPECHUD | SPECFL_SPECHUD_NEW;
		else
			client->pers.spec_flags &= ~(SPECFL_SPECHUD | SPECFL_SPECHUD_NEW);

	#if AQTION_EXTENSION
		if (Client_GetProtocol(ent) == 38) // Reki: new clients get new spec hud
			client->pers.spec_flags &= ~SPECFL_SPECHUD;
	#endif

		s = Info_ValueForKey(userinfo, "cl_spectatorkillfeed");
		if (atoi(s))
			client->pers.spec_flags |= SPECFL_KILLFEED;
		else
			client->pers.spec_flags &= ~SPECFL_KILLFEED;

		// Reki - disable antilag for *my own shooting*, not others shooting at me
		s = Info_ValueForKey(userinfo, "cl_antilag");
		int antilag_value = client->pers.antilag_optout;
		if (s[0] == 0 || atoi(s) > 0)
			client->pers.antilag_optout = qfalse;
		else if (atoi(s) <= 0)
			client->pers.antilag_optout = qtrue;

		if (sv_antilag->value && antilag_value != client->pers.antilag_optout)
			gi.cprintf(ent, PRINT_MEDIUM, "YOUR CL_ANTILAG IS NOW SET TO %i\n", !client->pers.antilag_optout);
#if AQTION_EXTENSION
	}
#endif
}

/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect(edict_t * ent, char *userinfo)
{
	char *value, ipaddr_buf[64];
	int tempBan = 0;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey( userinfo, "ip" );

	if (strlen(value) > sizeof(ipaddr_buf) - 1)
		gi.dprintf("ipaddr_buf length exceeded\n");
	Q_strncpyz(ipaddr_buf, value, sizeof(ipaddr_buf));

	if (SV_FilterPacket(ipaddr_buf, &tempBan)) {
		userinfo[0] = '\0';
		if(tempBan)
			Info_SetValueForKey(userinfo, "rejmsg", va("Temporary banned for %i games.", tempBan));
		else
			Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}
	// check for a password
	value = Info_ValueForKey(userinfo, "password");
	if (*password->string && strcmp(password->string, "none") && strcmp(password->string, value)) {
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// We're not going to attempt to support reconnection...
	if (ent->client->pers.connected) {
		ClientDisconnect(ent);
	}

	memset(ent->client, 0, sizeof(gclient_t));

	Q_strncpyz(ent->client->pers.ip, ipaddr_buf, sizeof(ent->client->pers.ip));
	Q_strncpyz(ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo));

	#if USE_AQTION
	value = Info_ValueForKey(userinfo, "steamid");
	if (*value)
		Q_strncpyz(ent->client->pers.steamid, value, sizeof(ent->client->pers.steamid));
	value = Info_ValueForKey(userinfo, "discordid");
	if (*value)
		Q_strncpyz(ent->client->pers.discordid, value, sizeof(ent->client->pers.discordid));
	#endif

	if (game.serverfeatures & GMF_MVDSPEC) {
		value = Info_ValueForKey(userinfo, "mvdspec");
		if (*value) {
			ent->client->pers.mvdspec = true;
		}
	}

	if (game.maxclients > 1) {
		value = Info_ValueForKey(userinfo, "name");
		gi.dprintf("%s@%s connected\n", value, ipaddr_buf);
		IRC_printf(IRC_T_SERVER, "%n@%s connected", value, ipaddr_buf);
	}

	//rekkie -- silence ban -- s
	if (SV_FilterSBPacket(ipaddr_buf, NULL)) // Check if player has been silenced
	{
		ent->client->pers.silence_banned = true;
		value = Info_ValueForKey(userinfo, "name");
		gi.dprintf("%s has been [SILENCED] because they're on the naughty list\n", value); // Notify console the player is silenced
	}
	else
		ent->client->pers.silence_banned = false;
	//rekkie -- silence ban -- e

	//set connected on ClientBeginDeathmatch as clientconnect doesn't always
	//guarantee a client is actually making it all the way into the game.
	//ent->client->pers.connected = true;

	#ifndef NO_BOTS
	if(bot_chat->value)
		BOTLIB_Chat(ent, CHAT_WELCOME);
	#endif
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect(edict_t * ent)
{
	int i;
	edict_t *etemp;

	if (!ent->client)
		return;

	if (esp->value && matchmode->value) {
		char tempmsg[128];
		// We have to kill him first before he is removed as captain/leader
		killPlayer(ent, false);
		sprintf(tempmsg, "The captain of %s (%s) disconnected, the other team wins by default!", teams[ent->client->resp.team].name, ent->client->pers.netname);
		CenterPrintAll(tempmsg);
	}

	MM_LeftTeam( ent );
	if (esp->value)		
		EspLeaderLeftTeam(ent);
		
	ent->client->resp.team = 0;

	// drop items if they are alive/not observer
	if (ent->solid != SOLID_NOT && !ent->deadflag)
		TossItemsOnDeath(ent);

	FreeClientEdicts(ent->client);

	if (ent->solid == SOLID_TRIGGER)
		RemoveFromTransparentList(ent);

	gi.bprintf(PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
	IRC_printf(IRC_T_SERVER, "%n disconnected", ent->client->pers.netname);

	if( !teamplay->value && !ent->client->pers.spectator )
	{
		// send effect
		gi.WriteByte( svc_muzzleflash );
		gi.WriteShort( ent - g_edicts );
		gi.WriteByte( MZ_LOGOUT );
		gi.multicast( ent->s.origin, MULTICAST_PVS );
	}

	if( use_ghosts->value )
		CreateGhost( ent );

	// go clear any clients that have this guy as their attacker
	for (i = 1, etemp = g_edicts + 1; i <= game.maxclients; i++, etemp++) {
		if (etemp->inuse) {
			if (etemp->client && etemp->client->attacker == ent)
				etemp->client->attacker = NULL;
			if (etemp->enemy == ent)	// AQ:TNG - JBravo adding tkok
				etemp->enemy = NULL;
		}
	}

	TourneyRemovePlayer(ent);
	vClientDisconnect(ent);	// client voting disconnect

	if (ctf->value)
		CTFDeadDropFlag(ent);

	PMenu_Close(ent);

	gi.unlinkentity(ent);

	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.sound = 0;
	ent->s.event = 0;
	ent->s.effects = 0;
	ent->s.renderfx = 0;
	ent->s.solid = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->svflags = SVF_NOCLIENT;
	ent->client->pers.connected = false;

	teams_changed = true;

#ifndef NO_BOTS
	ent->is_bot = false;
	ent->think = NULL;
	ACEIT_RebuildPlayerList();
#if USE_AQTION
	StatBotCheck();

	#if USE_AQTION
		if(am->value){
			//attract_mode_bot_check();
		}
	#endif
#endif
#endif
}

void CreateGhost(edict_t * ent)
{
	int i;
	gghost_t *ghost;

	if (ent->client->resp.score == 0 && ent->client->resp.damage_dealt == 0) {
		return;
	}

	//check if its already there
	for (i = 0, ghost = ghost_players; i < num_ghost_players; i++, ghost++) {
		if (!strcmp(ghost->ip, ent->client->pers.ip) && !strcmp(ghost->netname, ent->client->pers.netname)) {
			break;
		}
	}

	if (i >= num_ghost_players) {
		if (num_ghost_players >= MAX_GHOSTS) {
			gi.dprintf( "Maximum number of ghosts reached.\n" );
			return;
		}
		ghost = &ghost_players[num_ghost_players++];
	}

	strcpy(ghost->ip, ent->client->pers.ip);
	strcpy(ghost->netname, ent->client->pers.netname);
	#if USE_AQTION
	strcpy(ghost->steamid, ent->client->pers.steamid);
	strcpy(ghost->discordid, ent->client->pers.discordid);
	#endif

	ghost->enterframe = ent->client->resp.enterframe;
	ghost->disconnect_frame = level.framenum;

	// Score
	ghost->score = ent->client->resp.score;
	ghost->damage_dealt = ent->client->resp.damage_dealt;
	ghost->kills = ent->client->resp.kills;
	ghost->deaths = ent->client->resp.deaths;
	ghost->ctf_caps = ent->client->resp.ctf_caps;
	ghost->ctf_capstreak = ent->client->resp.ctf_capstreak;
	ghost->team_kills = ent->client->resp.team_kills;
	ghost->streakKillsHighest = ent->client->resp.streakKillsHighest;
	ghost->streakHSHighest = ent->client->resp.streakHSHighest;

	// Teamplay variables
	if (teamplay->value) {
		ghost->weapon = ent->client->pers.chosenWeapon;
		ghost->item = ent->client->pers.chosenItem;
		ghost->team = ent->client->resp.team;
	}

	// Statistics
	ghost->shotsTotal = ent->client->resp.shotsTotal;
	ghost->hitsTotal = ent->client->resp.hitsTotal;

	memcpy(ghost->hitsLocations, ent->client->resp.hitsLocations, sizeof(ghost->hitsLocations));
	memcpy(ghost->gunstats, ent->client->resp.gunstats, sizeof(ghost->gunstats));
}

//==============================================================

edict_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t q_gameabi PM_trace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end)
{
	if (pm_passent && pm_passent->health > 0)
		return gi.trace(start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace(start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

// Raptor007: Allow weapon actions to start happening on any frame.
static void ClientThinkWeaponIfReady( edict_t *ent, qboolean update_idle )
{
	int old_weaponstate, old_gunframe;

	// If they just spawned, sync up the weapon animation with that.
	if( ! ent->client->weapon_last_activity )
		ent->client->weapon_last_activity = level.framenum;

	// If it's too soon since the last non-idle think, keep waiting.
	else if( level.framenum < ent->client->weapon_last_activity + game.framediv )
		return;

	// Clear weapon kicks.
	VectorClear( ent->client->kick_origin );
	VectorClear( ent->client->kick_angles );

	old_weaponstate = ent->client->weaponstate;
	old_gunframe = ent->client->ps.gunframe;

	Think_Weapon( ent );

	// If the weapon is or was in any state other than ready, wait before thinking again.
	if( (ent->client->weaponstate != WEAPON_READY) || (old_weaponstate != WEAPON_READY) )
	{
		ent->client->weapon_last_activity = level.framenum;
		ent->client->anim_started = ent->client->weapon_last_activity;
	}

	// Only allow the idle animation to update if it's been enough time.
	else if( ! update_idle || level.framenum % game.framediv != ent->client->weapon_last_activity % game.framediv )
		ent->client->ps.gunframe = old_gunframe;
}

void FrameStartZ( edict_t *ent )
{
#ifndef NO_FPS
	if( (FRAMEDIV == 1) || ! ent->inuse || ! IS_ALIVE(ent) || (ent->z_history_framenum != level.framenum - 1) )
		return;

	// Restore origin[2] from z_history[0] once at the very beginning of the frame.
	ent->s.origin[2] = ent->z_history[0];
	ent->z_history_framenum = 0;
/*
	if( game.serverfeatures & GMF_CLIENTNUM )
		ent->client->ps.pmove.origin[2] = ent->z_pmove;
*/
#endif
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink(edict_t * ent, usercmd_t * ucmd)
{
	gclient_t *client;
	edict_t *other;
	int i, j;
	pmove_t pm;
	char ltm[64] = "\0";

	FrameStartZ( ent );

	level.current_entity = ent;
	client = ent->client;
	
	client->antilag_state.curr_timestamp += (float)ucmd->msec / 1000; // antilag needs sub-server-frame timestamps

	if (level.intermission_framenum) {
		client->ps.pmove.pm_type = PM_FREEZE;
		// 
		if (level.realFramenum > level.intermission_framenum + 4 * HZ) {
			if (ent->inuse && client->resp.stat_mode > 0
			    && client->resp.stat_mode_intermission == 0) {
				client->resp.stat_mode_intermission = 1;
				Cmd_Stats_f(ent, ltm);
			}
		}
		// can exit intermission after five seconds
		if (level.realFramenum > level.intermission_framenum + 5 * HZ && (ucmd->buttons & BUTTON_ANY))
			level.intermission_exit = 1;
		return;
	}

	if (level.pauseFrames > 0)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		return;
	}
	pm_passent = ent;
	// FROM 3.20 -FB
	if (client->chase_mode) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
	} else {
		// ^^^
		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		if (ent->movetype == MOVETYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent->s.modelindex != 255)
			client->ps.pmove.pm_type = PM_GIB;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;

		client->ps.pmove.gravity = sv_gravity->value;

		pm.s = client->ps.pmove;
		for (i = 0; i < 3; i++) {
			pm.s.origin[i] = ent->s.origin[i] * 8;
			pm.s.velocity[i] = ent->velocity[i] * 8;
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s))) {
			pm.snapinitial = true;
			//      gi.dprintf ("pmove changed!\n");
		}

		pm.cmd = *ucmd;
		client->cmd_last = *ucmd;

		// Stumbling movement with leg damage.
		// darksaint ETE edit:  if esp_enhancedslippers are enabled/equipped, negate all stumbling
		qboolean has_enhanced_slippers = esp_enhancedslippers->value && INV_AMMO(ent, SLIP_NUM);
		if( client->leg_damage && ent->groundentity && ! has_enhanced_slippers )
		{
			#if AQTION_EXTENSION
			pm.s.pm_aq2_flags |= PMF_AQ2_LIMP;
			pm.s.pm_aq2_leghits = min(client->leghits, 255);
			#else
			int frame_mod_6 = (level.framenum / game.framediv) % 6;
			if( frame_mod_6 <= 2 )
			{
				pm.cmd.forwardmove = 0;
				pm.cmd.sidemove = 0;
			}
			else if( frame_mod_6 == 3 )
			{
				pm.cmd.forwardmove /= client->leghits + 1;
				pm.cmd.sidemove /= client->leghits + 1;
			}

			// Prevent jumping with leg damage.
			pm.s.pm_flags |= PMF_JUMP_HELD;
			#endif
		}
		#if AQTION_EXTENSION
		else
		{
			pm.s.pm_aq2_flags &= ~PMF_AQ2_LIMP;
			pm.s.pm_aq2_leghits = 0;
		}
		#endif

		#ifdef AQTION_EXTENSION
		if (ent->is_bot)
		{
			//pm.s.pm_aq2_flags |= PMF_AQ2_FRICTION; // Increase friction
		}
		#endif
		//rekkie -- Increase friction for bots  -- e

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;
		// perform a pmove
		gi.Pmove(&pm);

		//FB 6/3/99 - info from Mikael Lindh from AQ:G
		if (pm.maxs[2] == 4) {
			ent->maxs[2] = CROUCHING_MAXS2;
			pm.maxs[2] = CROUCHING_MAXS2;
			ent->viewheight = CROUCHING_VIEWHEIGHT;
			pm.viewheight = (float) ent->viewheight;
		}
		//FB 6/3/99

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		for (i = 0; i < 3; i++) {
			ent->s.origin[i] = pm.s.origin[i] * 0.125;
			ent->velocity[i] = pm.s.velocity[i] * 0.125;
		}

		if( ! client->leg_damage && ent->groundentity && ! pm.groundentity && pm.cmd.upmove >= 10 && pm.waterlevel == 0 )
			ent->client->jumping = 1;

		VectorCopy(pm.mins, ent->mins);
		VectorCopy(pm.maxs, ent->maxs);
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		ent->viewheight = pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;

		if( pm.groundentity || ! slopefix->value )
			ent->groundentity = pm.groundentity;
		else if( ent->groundentity && (ent->client->jumping || pm.waterlevel || (ent->velocity[2] > 0) || (ent->velocity[2] < -70) || ! ent->groundentity->inuse) )
			ent->groundentity = NULL;

		if( ent->groundentity )
			ent->groundentity_linkcount = ent->groundentity->linkcount;

		//rekkie -- DEV_1 -- s
		if (pm.groundentity)
			ent->last_touched_ground = 0; // Reset the counter
		else
			ent->last_touched_ground++; // Increment the counter
		//rekkie -- DEV_1 -- e

		if (ent->deadflag) {
			client->ps.viewangles[ROLL] = 40;
			client->ps.viewangles[PITCH] = -15;
			client->ps.viewangles[YAW] = client->killer_yaw;
		} else {
			VectorCopy(pm.viewangles, client->v_angle);
			VectorCopy(pm.viewangles, client->ps.viewangles);
		}

		if(client->ctf_grapple)
			CTFGrapplePull(client->ctf_grapple);

		gi.linkentity(ent);

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers(ent);

		// stop manipulating doors
		client->doortoggle = 0;

		if( client->jumping && (ent->solid != SOLID_NOT) && ! lights_camera_action && (!client->uvTime && !esp_punishment_phase) && ! jump->value )
		{
			kick_attack( ent );
			client->punch_desired = false;
		}

		// touch other objects
		for (i = 0; i < pm.numtouch; i++) {
			other = pm.touchents[i];
			for (j = 0; j < i; j++)
				if (pm.touchents[j] == other)
					break;
			if (j != i)
				continue;	// duplicated
			if (!other->touch)
				continue;
			other->touch(other, ent, NULL, NULL);
		}
	}

	//rekkie -- DEV_1 -- s
	// Always track what nodes human players are near, if any
	if (ent->is_bot == false)
	{
		// Track human players
		if (ent->solid != SOLID_NOT && ent->deadflag != DEAD_DEAD)
		{
			// Check if bot is touching a node that isn't on the path
			int nodes_touched; // Number of nodes touched
			int nodelist[MAX_NODELIST]; // Nodes touched
			nodes_touched = BOTLIB_NodeTouchNodes(ent->s.origin, vec3_origin, 32, ent->mins, ent->maxs, nodelist, MAX_NODELIST, INVALID);
			for (i = 0; i < nodes_touched; i++)
			{
				if (nodelist[i] != INVALID && nodes[nodelist[i]].inuse)
					ent->bot.current_node = nodelist[i]; // Update which node the player last touched
			}

			//ent->bot.current_node = ACEND_FindClosestReachableNode(ent, NODE_DENSITY, NODE_ALL);	// Update which node the player is closest to
		}
	}

	// Test if walking toward middle of map
	if (0 && ent->is_bot == false)
	{
		//if (BOTLIB_MovingToward(ent, tv(0, 0, 0), 0.3))
		{
			//Com_Printf("%s %s moving toward [%d]\n", __func__, ent->client->pers.netname, level.framenum);
		}

		if (BOTLIB_CanMove(ent, MOVE_BACK))
			Com_Printf("%s %s can move back [%d]\n", __func__, ent->client->pers.netname, level.framenum);
		if (BOTLIB_CanMove(ent, MOVE_FORWARD))
			Com_Printf("%s %s can move forward [%d]\n", __func__, ent->client->pers.netname, level.framenum);
		if (BOTLIB_CanMove(ent, MOVE_LEFT))
			Com_Printf("%s %s can move left [%d]\n", __func__, ent->client->pers.netname, level.framenum);
		if (BOTLIB_CanMove(ent, MOVE_RIGHT))
			Com_Printf("%s %s can move right [%d]\n", __func__, ent->client->pers.netname, level.framenum);
	}

	// Test throwing knife 'pitch' required to hit distant targets
	if (0 && ent->is_bot == false)
	{
		vec3_t org = { 0, 0, 0 };
		if (players[1] && players[1]->client)
		{
			VectorCopy(players[1]->s.origin, org);
			//Com_Printf("%s %s\n", __func__, players[1]->client->pers.netname);
		}

		float gravity = sv_gravity->value; // Gravity
		float distanceXYZ = VectorDistance(ent->s.origin, org); // Distance XY
		float distanceXY = VectorDistance(ent->s.origin, org); // Distance XYZ
		float height_diff = org[2] - ent->s.origin[2]; // Height difference
		float initialSpeed = 1200;

		float timeToMaxHeight = sqrtf((initialSpeed * initialSpeed) / (2 * gravity));
		float totalFlightTime = timeToMaxHeight + sqrtf((distanceXYZ / height_diff) + timeToMaxHeight * timeToMaxHeight);

		// Calculate total flight time
		totalFlightTime = sqrtf((2 * distanceXYZ) / sv_gravity->value);

		float horizontalVelocity = distanceXY / totalFlightTime;
		float verticalVelocity = ((horizontalVelocity * horizontalVelocity) + (initialSpeed * initialSpeed)) / (2 * height_diff);

		float launchAngleDegrees = atan2f(verticalVelocity, horizontalVelocity) * (180 / M_PI);
		launchAngleDegrees += 90;
		if (launchAngleDegrees)
		{
			launchAngleDegrees /= 2;

			if (height_diff > 0)
				launchAngleDegrees -= 90;
				
		}
		Com_Printf("%s pitch %f %f launchAngleDegrees %f\n", __func__, ent->s.angles[PITCH], ent->client->ps.viewangles[PITCH], launchAngleDegrees);

		ucmd->angles[PITCH] = launchAngleDegrees;
		ent->s.angles[PITCH] = launchAngleDegrees;
		ent->client->ps.viewangles[PITCH] = launchAngleDegrees;
	}

	if (0 && ent->is_bot == false)
	{
		int mk23_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, MK23_NUM);
		int dual_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, DUAL_NUM);
		int mp5_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, MP5_NUM);
		int m4_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, M4_NUM);
		int m3_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, M3_NUM);
		int hc_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, HC_NUM);
		int ssg_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, SNIPER_NUM);
		int knife_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, KNIFE_NUM);
		int gren_ammo = BOTLIB_CheckWeaponLoadedAmmo(ent, GRENADE_NUM);
		Com_Printf("%s %s mk23[%d] dual[%d] mp5[%d] m4[%d] m3[%d] hc[%d] ssg[%d] k[%d] g[%d]\n", __func__, ent->client->pers.netname, 
			mk23_ammo, dual_ammo, mp5_ammo, m4_ammo, m3_ammo, hc_ammo, ssg_ammo, knife_ammo, gren_ammo);
	}

	// Test CTF
	if (0 && ctf->value && ent->is_bot == false)
	{
		//BOTLIB_TeamFlagIsHome(ent, ent->client->resp.team);
		BOTLIB_IsFlagDropped(ent, TEAM1, 0);
		BOTLIB_IsFlagDropped(ent, TEAM1, 512);
		BOTLIB_IsFlagDropped(ent, TEAM2, 0);
		BOTLIB_IsFlagDropped(ent, TEAM2, 512);
		
		//BOTLIB_TeamFlagDropped(ent, ent->client->resp.team, 512);
		//BOTLIB_EnemyFlagDropped(ent, ent->client->resp.team, 512);
		/*
		if (ent->client->resp.team == TEAM1 && BOTLIB_TeamFlagDropped(ent, ent->client->resp.team, 512))
			Com_Printf("%s %s our dropped flag is nearby %f\n", __func__, ent->client->pers.netname, BOTLIB_DistanceToFlag(ent, FLAG_T1_NUM));
		else if (ent->client->resp.team == TEAM2 && BOTLIB_TeamFlagDropped(ent, ent->client->resp.team, 512))
			Com_Printf("%s %s our dropped flag is nearby %f\n", __func__, ent->client->pers.netname, BOTLIB_DistanceToFlag(ent, FLAG_T2_NUM));

		if (ent->client->resp.team == TEAM1 && BOTLIB_EnemyFlagDropped(ent, ent->client->resp.team, 512))
			Com_Printf("%s %s dropped enemy flag is nearby %f\n", __func__, ent->client->pers.netname, BOTLIB_DistanceToFlag(ent, FLAG_T2_NUM));
		else if (ent->client->resp.team == TEAM2 && BOTLIB_EnemyFlagDropped(ent, ent->client->resp.team, 512))
			Com_Printf("%s %s dropped enemy flag is nearby %f\n", __func__, ent->client->pers.netname, BOTLIB_DistanceToFlag(ent, FLAG_T1_NUM));
		*/

		/*
		if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false)
			Com_Printf("%s %s carrying red flag\n", __func__, bot_ctf_status.player_has_flag1->client->pers.netname);
		if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false)
			Com_Printf("%s %s carrying blue flag\n", __func__, bot_ctf_status.player_has_flag2->client->pers.netname);

		if (bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag1_is_home == false)
			Com_Printf("%s red flag was dropped\n", __func__);
		if (bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home == false)
			Com_Printf("%s blue flag was dropped\n", __func__);
		*/
	}

	// Show the speed of the player, their forward/back, and left/right movement
	if (0)
	{
		edict_t* self = ent;

		// Calculate the players current speed and direction and print the results to the console
		vec3_t velocity;
		VectorCopy(self->velocity, velocity);
		velocity[2] = 0;
		float speed = VectorLength(velocity);

		vec3_t angles;
		VectorCopy(self->client->v_angle, angles);
		angles[0] = 0;
		angles[2] = 0;

		vec3_t forward, right;
		AngleVectors(angles, forward, right, NULL);

		float forward_back = DotProduct(forward, velocity);
		float left_or_right = DotProduct(right, velocity);

		Com_Printf("%s: speed %f forward_back %f left_or_right %f\n", __func__, speed, forward_back, left_or_right);
	}

	// Trace forward (left and right of player) to look for blocked paths
	if (0)
	{
		if (ent->is_bot == false)
		{
			edict_t *self = ent;

			// Send out another trace on the left and right side of the bot parallel to the current direction the bot is facing

			vec3_t groundvec, end;
			vec3_t offset, forward, right, start, down_left_end; // , down_right_end, up_left_end, up_right_end;
			float forward_back, left_or_right, up_or_down, beam_width;

			groundvec[PITCH] = 0;
			groundvec[YAW] = self->client->v_angle[YAW];
			groundvec[ROLL] = 0;

			const float distance = 32;

			AngleVectors(groundvec, forward, NULL, NULL);	// Make a forwards pointing vector out of the bot's view angles
			VectorMA(self->s.origin, 60, forward, end);	// Make end equal to 60* the forward pointing vector

			beam_width = 2;	// The actual width is '4' (found in CL_ParseLaser() tent.c) - but slightly reduced it here because it's easier to see when it intersects with a wall
			forward_back = self->maxs[PITCH] + beam_width;	// + forward, - backward

			// Left (-32) of the feet from the ground + STEPSIZE -> pointing forward parallel to the forward vector
			left_or_right = self->mins[YAW] - 16;		// - left, + right
			up_or_down = self->mins[ROLL] + STEPSIZE;	// + up, - down
			VectorSet(offset, forward_back, left_or_right, up_or_down);
			AngleVectors(groundvec, forward, right, NULL);
			G_ProjectSource(self->s.origin, offset, forward, right, start);
			VectorMA(start, distance, forward, down_left_end);	// Make end equal to 200* the forward pointing vector
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(start);
			gi.WritePosition(down_left_end);
			gi.multicast(self->s.origin, MULTICAST_PHS);

			VectorMA(start, (distance - 16), forward, down_left_end);	// Move the end point back -16 units (mins[0])
			trace_t tr = gi.trace(start, tv(-16,-16,0), tv(16,16,56), down_left_end, self, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0 || tr.startsolid)
			{
				Com_Printf("%s: %s left blocked\n", __func__, self->client->pers.netname);
			}
			else
			{
				Com_Printf("%s: %s left clear\n", __func__, self->client->pers.netname);
			}

			// Right (+32) of the feet from the ground + STEPSIZE -> pointing forward parallel to the forward vector
			left_or_right = self->maxs[YAW] + 16;		// - left, + right
			up_or_down = self->mins[ROLL] + STEPSIZE;	// + up, - down
			VectorSet(offset, forward_back, left_or_right, up_or_down);
			AngleVectors(groundvec, forward, right, NULL);
			G_ProjectSource(self->s.origin, offset, forward, right, start);
			VectorMA(start, distance, forward, down_left_end);	// Make end equal to 200* the forward pointing vector
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(start);
			gi.WritePosition(down_left_end);
			gi.multicast(self->s.origin, MULTICAST_PHS);

			VectorMA(start, (distance - 16), forward, down_left_end);	// Move the end point back -16 units (mins[0])
			tr = gi.trace(start, tv(-16, -16, 0), tv(16, 16, 56), down_left_end, self, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0 || tr.startsolid)
			{
				Com_Printf("%s: %s right blocked\n", __func__, self->client->pers.netname);
			}
			else
			{
				Com_Printf("%s: %s right clear\n", __func__, self->client->pers.netname);
			}
		}
	}

	// Test code to see if we're to the right or left of the node
	if (0)
	{
		if (ent->is_bot == false)
		{
			// Trace test to see if we can see our next node
			int next_node = 1146; // urban2 --> 1151, 1146, 636
			int current_node = 1151; // urban2 --> 1113, 1151, 494
			//trace_t tr = gi.trace(ent->s.origin, NULL, NULL, nodes[next_node].origin, ent, MASK_DEADSOLID);
			//if (tr.fraction < 1.0) // blocked, try find another path to the same goal
			{
				//Com_Printf("%s %s is blocked [NODE_MOVE]... %f\n", __func__, ent->client->pers.netname, level.time);

				// Get distance from bot to next node
				vec3_t bot_to_next_vec;
				VectorSubtract(nodes[next_node].origin, ent->s.origin, bot_to_next_vec);
				float bot_to_next_dist = VectorLength(bot_to_next_vec);

				// Get distance from current node to next node
				vec3_t curr_to_next_vec;
				VectorSubtract(nodes[next_node].origin, nodes[current_node].origin, curr_to_next_vec);
				float curr_to_next_dist = VectorLength(curr_to_next_vec);

				// Calculate the normalized distance the bot has travelled between current and next node
				float bot_travelled_dist = 0;
				if (bot_to_next_dist < curr_to_next_dist)
					bot_travelled_dist = 1 - (bot_to_next_dist / curr_to_next_dist);
				else
					bot_travelled_dist = 1 - (curr_to_next_dist / bot_to_next_dist);

				// Normalize bot_travelled_dist
				//bot_travelled_dist = bot_travelled_dist - (int)bot_travelled_dist;

				Com_Printf("%s %s [b->n %f] [c->n %f] [%f]\n", __func__, ent->client->pers.netname, bot_to_next_dist, curr_to_next_dist, bot_travelled_dist);

				// Get the origin between the current and next node based on the normalized bot_travelled_dist
				vec3_t normalized_origin;
				//VectorScale(curr_to_next_vec, bot_travelled_dist, normalized_origin);
				LerpVector(nodes[current_node].origin, nodes[next_node].origin, bot_travelled_dist, normalized_origin);



				// Show a laser between next node and normalized_origin
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_BFG_LASER);
				gi.WritePosition(nodes[next_node].origin);
				gi.WritePosition(normalized_origin);
				gi.multicast(nodes[next_node].origin, MULTICAST_PVS);


				// Calculate the angle between the bot's origin and the normalized origin
				vec3_t normalized_origin_vec;
				VectorSubtract(normalized_origin, ent->s.origin, normalized_origin_vec);
				float bot_to_path_dist = VectorLength(normalized_origin_vec);
				//Com_Printf("%s %s [d %f]\n", __func__, ent->client->pers.netname, bot_to_path_dist);


				//VectorNormalize(normalized_origin);

				float current_yaw = anglemod(ent->s.angles[YAW]);

				vec3_t ideal_angle;
				vectoangles(normalized_origin_vec, ideal_angle);

				float ideal_yaw = anglemod(ideal_angle[YAW]);// +180;

				// print current_yaw and ideal_yaw
				//Com_Printf("%s %s [c %f] [i %f]\n", __func__, ent->client->pers.netname, current_yaw, ideal_yaw);

				// Figure out if current yaw is closer to idea yaw if we turn left
				float left_yaw;
				float right_yaw;
				// c = 301 i = 211
				// c = 213 i = 116 
				//const float less_than_90_degrees = 87; // 90 is the optimum angle, but we want to give a little leeway (otherwise the bot's yaw movement will oscillate)

				if (bot_to_path_dist > 16)
				{
					/*
					if (ideal_yaw > current_yaw)
					{
						right_yaw = ideal_yaw + (360 - current_yaw);
						left_yaw = ideal_yaw - current_yaw;
					}
					else
					{
						right_yaw = current_yaw - ideal_yaw;
						left_yaw = ideal_yaw + (360 - current_yaw);
					}
					*/

					if (ideal_yaw > current_yaw)
					{
						right_yaw = ideal_yaw - current_yaw;
						left_yaw = 360 - right_yaw;
					}
					else
					{
						left_yaw = current_yaw - ideal_yaw;
						right_yaw = 360 - left_yaw;
					}

					/*
					i > c
					c 41
					i 55
					right	i - c 				55 - 41 = 14
					left	(360 - c) - i		(360 - 41 = 319) - 55 = 264

					c > i
					c 55
					i 41
					right 	c - i 				55 - 41 = 14
					left 	(360 - c) - i 		(360 - 55 = 305) - 41 = 264
					*/

					//if (right_yaw < 0)
					//	right_yaw += 360;
					//if (left_yaw < 0)
					//	left_yaw += 360;

					//if (right_yaw > 360)
					//	right_yaw -= 360;
					//if (left_yaw > 360)
					//	left_yaw -= 360;

					Com_Printf("%s %s [c %f] [i %f] [ry %f] [ly %f] [dist %f]\n", __func__, ent->client->pers.netname, current_yaw, ideal_yaw, right_yaw, left_yaw, bot_to_path_dist);

					if (right_yaw < left_yaw)
						Com_Printf("%s %s move left\n", __func__, ent->client->pers.netname);
					else
						Com_Printf("%s %s move right\n", __func__, ent->client->pers.netname);

					//if (left_yaw < right_yaw)
					//	Com_Printf("%s %s move left %f\n", __func__, ent->client->pers.netname, left_yaw);
					//else
					//	Com_Printf("%s %s move right %f\n", __func__, ent->client->pers.netname, right_yaw);
				}
			}
		}
	}

	if (0 && ent->is_bot == false)
	{
		// Test for gaps/holes between the current and next node
		// Each tested is conducted 32 units apart

		vec3_t forward, end, origin, target;
		VectorCopy(nodes[1740].origin, origin);
		VectorCopy(nodes[1403].origin, target);
		float tested_distance = 0;

		// Show a laser between next node and normalized_origin
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(origin);
		gi.WritePosition(tv(target[0], target[1], target[2]));
		gi.multicast(origin, MULTICAST_PVS);

		// Get direction
		VectorSubtract(target, origin, forward);

		// Get XYZ distance
		float xyz_distance = VectorLength(forward);

		// Normalize direction vector
		VectorNormalize(forward);

		while (tested_distance + NODE_SIZE < xyz_distance)
		{
			tested_distance += NODE_SIZE; // Move next test forward

			VectorMA(origin, tested_distance, forward, end); // Find the end point origin of a vector of length tested_distance

			//VectorMA(origin, tested_distance, forward, end); // Find the end point origin of a vector of length tested_distance
			end[2] -= NODE_Z_HEIGHT; // Move end point to ground
			//end[2] -= (STEPSIZE + 1); // Move end point below ground level at the depth of step height + 1
			//end[2] = -4096;



			trace_t tr = gi.trace(end, NULL, NULL, tv(end[0], end[1], -4096), g_edicts, MASK_DEADSOLID);

			if (tr.startsolid)
			{
				//Com_Printf("%s dist %f z-diff startsolid\n", __func__, tested_distance, end[2] - tr.endpos[2]);
				continue;
			}

			//Com_Printf("%s [tested_distance %f] [o %f %f %f] [t %f %f %f] [e %f %f %f] [e %f %f %f]\n", __func__, tested_distance, origin[0], origin[1], origin[2], target[0], target[1], target[2], end[0], end[1], end[2], tr.endpos[0], tr.endpos[1], tr.endpos[2]);
			//Com_Printf("%s dist %f z-diff %f\n", __func__, tested_distance, ((end[2] - (NODE_Z_HEIGHT + STEPSIZE)) - tr.endpos[2]));

			if (end[2] - tr.endpos[2] > (NODE_Z_HEIGHT))
				Com_Printf("%s dist %f z-diff %f -- FOUND GAP\n", __func__, tested_distance, end[2] - tr.endpos[2]);
			else
				Com_Printf("%s dist %f z-diff %f\n", __func__, tested_distance, end[2] - tr.endpos[2]);

			// Show a laser between next node and normalized_origin
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(end);
			gi.WritePosition(tr.endpos);
			gi.multicast(origin, MULTICAST_PVS);

			//if (from == 1740 && to == 1403)
			{
				//Com_Printf("%s tested_distance %f z-diff %f\n", __func__, tested_distance, ((origin[2] - (NODE_Z_HEIGHT + STEPSIZE)) - tr.endpos[2]));
			}

			//if (tr.fraction == 1.0 && tr.startsolid == false) // Hit a gap (no floor)
			if (tr.endpos[2] < (origin[2] - (NODE_Z_HEIGHT + STEPSIZE))) // Hit a gap (no floor)
			{
				//Com_Printf("%s gap found at %f origin %f %f %f target %f %f %f end %f %f %f\n", __func__, tested_distance, origin[0], origin[1], origin[2], target[0], target[1], target[2], end[0], end[1], end[2]);
				// print gap found at tested_distance, and the difference between endpos[2] and (origin[2] - (NODE_Z_HEIGHT + STEPSIZE))
				//Com_Printf("%s gap found at tested_distance %f z-diff %f\n", __func__, tested_distance, ((origin[2] - (NODE_Z_HEIGHT + STEPSIZE)) - tr.endpos[2]));

				//is_gap = true;
				//break;
			}
		}
		Com_Printf("%s \n", __func__);
	}

	if (0 && ent->is_bot == false)
	{
		//trace_t tr = gi.trace(ent->s.origin, NULL, NULL, tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 128), ent, (MASK_PLAYERSOLID | MASK_OPAQUE));
		//Com_Printf("%s %s tr.plane.normal[%f] \n", __func__, ent->client->pers.netname, tr.plane.normal[2]);

		if (ent->bot.highlighted_node > 0)
		{
			// Get the player's direction based from their velocity
			vec3_t walkdir;
			VectorSubtract(ent->s.origin, ent->lastPosition, walkdir);
			//VectorCopy(ent->s.origin, ent->lastPosition);
			VectorNormalize(walkdir);
			vec3_t angle, forward, right, start, end, origin, offset;
			vectoangles(walkdir, angle);
			VectorCopy(ent->s.angles, angle); // Use the player's view angles (not their walk direction)
			angle[0] = 0;
			AngleVectors(angle, forward, right, NULL);
			VectorCopy(ent->s.origin, origin);
			origin[2] += 8; // [Origin 24 units] + [8 units] == 32 units heigh (same as node height)

			/*
			// Offset origin to the right
			//VectorSet(offset, 0, 32, 0);
			//G_ProjectSource(origin, offset, forward, right, start);

			VectorSet(offset, 0, 0, 0);
			G_ProjectSource(origin, offset, forward, right, start);
			VectorNormalize(forward);

			//VectorCopy(nodes[ent->bot.highlighted_node].origin, end);
			//VectorSubtract(end, right, forward);
			//VectorNormalize(forward);
			*/

			// Calculate the direction from the player's origin to the node origin
			vec3_t node_dir;
			VectorSubtract(nodes[ent->bot.highlighted_node].origin, ent->s.origin, node_dir);
			VectorNormalize(node_dir);
			{
				// Node direction
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_BFG_LASER);
				gi.WritePosition(ent->s.origin);
				gi.WritePosition(nodes[ent->bot.highlighted_node].origin);
				gi.multicast(ent->s.origin, MULTICAST_PHS);
			}

			VectorNormalize(forward);

			// Calculate the dot product of the forward dir and the node dir
			float dot = DotProduct(forward, node_dir);
			//Com_Printf("%s node %d dot %f\n", __func__, ent->bot.highlighted_node, dot);
			//if (dot < 0.995)

			if (1)
			{
				float dot_right = DotProduct(right, node_dir);
				VectorNegate(right, right);
				float dot_left = DotProduct(right, node_dir);
				if (dot_right > dot_left) // 
					Com_Printf("%s [MOVE RIGHT] node %d dot %f dot_right %f dot_left %f\n", __func__, ent->bot.highlighted_node, dot, dot_right, dot_left);
				else
					Com_Printf("%s [MOVE LEFT] node %d dot %f  dot_right %f dot_left %f\n", __func__, ent->bot.highlighted_node, dot, dot_right, dot_left);
			}

			if (0)
			{
				// Offset origin to the right
				vec3_t s_right;
				VectorSet(offset, 0, 0, 0);
				G_ProjectSource(origin, offset, forward, right, s_right);
				VectorSubtract(nodes[ent->bot.highlighted_node].origin, s_right, forward);
				VectorNormalize(forward);
				float dot_right = DotProduct(forward, node_dir);
				{
					// Player walk direction
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(TE_BFG_LASER);
					gi.WritePosition(s_right);
					gi.WritePosition(nodes[ent->bot.highlighted_node].origin);
					gi.multicast(s_right, MULTICAST_PHS);
				}

				// Offset origin to the left
				vec3_t s_left;
				VectorSet(offset, 0, -32, 0);
				G_ProjectSource(origin, offset, forward, right, s_left);
				VectorSubtract(nodes[ent->bot.highlighted_node].origin, s_left, forward);
				VectorNormalize(forward);
				float dot_left = DotProduct(forward, node_dir);
				{
					// Player walk direction
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(TE_BFG_LASER);
					gi.WritePosition(s_left);
					gi.WritePosition(nodes[ent->bot.highlighted_node].origin);
					gi.multicast(ent->s.origin, MULTICAST_PHS);
				}


				if (dot_right < dot_left) // 
					Com_Printf("%s [MOVE RIGHT] node %d dot %f dot_right %f dot_left %f\n", __func__, ent->bot.highlighted_node, dot, dot_right, dot_left);
				else
					Com_Printf("%s [MOVE LEFT] node %d dot %f  dot_right %f dot_left %f\n", __func__, ent->bot.highlighted_node, dot, dot_right, dot_left);
			}




			// Show visual
			if (0)
			{
				VectorSet(offset, 0, 32, 0);
				G_ProjectSource(origin, offset, forward, right, start);
				offset[0] += 1024; // Distance forward dir
				G_ProjectSource(origin, offset, forward, right, end);

				// Player walk direction
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_BFG_LASER);
				gi.WritePosition(start);
				gi.WritePosition(end);
				gi.multicast(ent->s.origin, MULTICAST_PHS);

				// Node direction
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_BFG_LASER);
				gi.WritePosition(ent->s.origin);
				gi.WritePosition(nodes[ent->bot.highlighted_node].origin);
				gi.multicast(ent->s.origin, MULTICAST_PHS);
			}
		}
	}

	// Expand additional nodes from the center of the edge, one heading 90 degrees left, and one heading 90 degrees right.
	if (0 && ent->is_bot == false)
	{
		//ACEND_BSP: [pt1:480.000000,640.000000,0.000000] [pt2:480.000000,752.000000,0.000000]
		vec3_t pt1 = { 480.000000, 640.000000, 0.000000 };
		vec3_t pt2 = { 480.000000, 752.000000, 0.000000 };
		vec3_t center;
		LerpVector(pt1, pt2, 0.50, center);

		vec3_t end_left, end_right;

		/*
		BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 90, 4, end_left);
		BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 270, 4, end_right);
		end_left[2] += 1;
		end_right[2] += 1;
		DAIC_Add_Node(end_left, tv(0, 0, 0), NODE_MOVE);
		DAIC_Add_Node(end_right, tv(0, 0, 0), NODE_MOVE);
		*/
		
		//ent->spin++;
		//if (ent->spin > 360)
		//	ent->spin = 0;

		//BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, ent->spin, 512, end_left); // Anti-clockwise
		BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 90, 3000, end_left);   // Anti-clockwise
		BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 270, 3000, end_right); // Clockwise

		// Show a laser between next node and normalized_origin
		/*
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(pt1);
		gi.WritePosition(pt2);
		gi.multicast(center, MULTICAST_PVS);
		*/

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(center);
		gi.WritePosition(end_left);
		gi.multicast(center, MULTICAST_PVS);


		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(center);
		gi.WritePosition(end_right);
		gi.multicast(center, MULTICAST_PVS);



		/*
		// Lift the end points up a bit so they don't get stuck in the ground
		end_left[2] += 1;
		end_right[2] += 1;

		// Test to see where the left and right meet a wall
		// Center is lifted up a bit so it doesn't get stuck in the ground, for the same reason as end_left and end_right
		trace_t tr_left_wall = gi.trace(tv(center[0], center[1], center[2] + 1), NULL, NULL, end_left, ent, MASK_PLAYERSOLID);
		trace_t tr_right_wall = gi.trace(tv(center[0], center[1], center[2] + 1), NULL, NULL, end_right, ent, MASK_PLAYERSOLID);

		//tr_left_wall.endpos[1] -= 32;
		//tr_right_wall.endpos[1] -= 32;
		// 
		// Find the middle between two points of an edge
		vec3_t left_mid, right_mid;
		LerpVector(center, tr_left_wall.endpos, 0.50, left_mid);
		LerpVector(center, tr_right_wall.endpos, 0.50, right_mid);

		//DAIC_Add_Node(left_mid, tr_left_wall.plane.normal, NODE_MOVE);
		//DAIC_Add_Node(left_mid, tr_right_wall.plane.normal, NODE_MOVE);

		// Test to see if left and right are touching the ground
		trace_t tr_left_ground = gi.trace(tr_left_wall.endpos, NULL, NULL, tv(tr_left_wall.endpos[0], tr_left_wall.endpos[1], tr_left_wall.endpos[2] - 2), ent, MASK_PLAYERSOLID);
		trace_t tr_right_ground = gi.trace(tr_right_wall.endpos, NULL, NULL, tv(tr_right_wall.endpos[0], tr_right_wall.endpos[1], tr_right_wall.endpos[2] - 2), ent, MASK_PLAYERSOLID);

		vec3_t back;

		// If the left is touching the ground, then we can add a node there
		if (tr_left_ground.fraction < 1.0)
		{
			// Back the node away from the wall a bit using the normal of the wall
			//vec3_t back;
			VectorMA(tr_left_wall.endpos, -NODE_Z_HALF_HEIGHT, tr_left_wall.plane.normal, back);
			//DAIC_Add_Node(back, tr_left_ground.plane.normal, NODE_MOVE);
		}
		// If the right is touching the ground, then we can add a node there
		if (tr_right_ground.fraction < 1.0)
		{
			// Back the node away from the wall a bit using the normal of the wall
			//vec3_t back;
			VectorMA(tr_right_wall.endpos, -NODE_Z_HALF_HEIGHT, tr_right_wall.plane.normal, back);
			//DAIC_Add_Node(back, tr_right_ground.plane.normal, NODE_MOVE);
		}
		*/
	}

	if (0 && ent->is_bot == false)
	{
		//Com_Printf("%s origin %f %f %f\n", __func__, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
		/*
		int f = 0, edges = 0;
		BOTLIB_UTIL_NEAREST_BSP_FACE(ent->s.origin, &f, &edges);
		Com_Printf("%s face %d  edges %d\n--------------\n", __func__, f, edges);
		for (int e = 0; e < nmesh.face[f].num_edges; e++)
		{
			Com_Printf("%s v1[%f %f %f]  v2[%f %f %f]\n", __func__, nmesh.face[f].edge[e].v[0][0], nmesh.face[f].edge[e].v[0][1], nmesh.face[f].edge[e].v[0][2], nmesh.face[f].edge[e].v[1][0], nmesh.face[f].edge[e].v[1][1], nmesh.face[f].edge[e].v[1][2]);
		}
		Com_Printf("--------------\n");
		*/
	}

	//rekkie -- surface data -- s
	/*
	if (ent->nav != NULL)
	{
		vec3_t start = { 0,0,0 }; vec3_t end = { 0,0,128 };
		#define U32_YELLOW  MakeColor(255, 255,   0, 255)
		//nav->DrawArrow(start, end, U32_YELLOW, 5.0);
		//game.framerate;

		// Local client (call the function directly)
		if (ent->client->clientNum == 0)
		{
			ent->client->pers.ip;
			if (ent->nav->DrawArrow != NULL)
			{
				// Assign void pointer nav->DrawArrow to a function pointer
				void(*DrawArrow)(int number, vec3_t start, vec3_t end, const uint32_t color, float line_width, int time) = ent->nav->DrawArrow;
				DrawArrow(start, end, U32_YELLOW, 50); // Call the function
			}
		}
		else // Networked clients (send a message)
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_DRAWARROW);
			gi.WriteLong(U32_YELLOW); // Color
			gi.WriteByte(5); // Line width
			gi.WritePosition(start);
			gi.WritePosition(end);
			//gi.WritePosition(ent->s.origin);
			//gi.WritePosition(tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + 128));
			gi.multicast(start, MULTICAST_PVS);
		}
	}
	*/

	if (0 && ent->is_bot == false)
	{
		// Check if touching ladder
		qboolean touching_ladder = false;
		{
			float yaw_rad = 0;
			vec3_t fwd = { 0 }, end = { 0 };
			trace_t tr;

			yaw_rad = DEG2RAD(ent->s.angles[YAW]);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);

			VectorMA(ent->s.origin, 64, fwd, end);

			vec3_t lmins = { -16, -16, -96 };
			vec3_t lmaxs = { 16, 16, 96 };

			tr = gi.trace(ent->s.origin, lmins, lmaxs, end, ent, MASK_PLAYERSOLID);

			touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
		}
		if (touching_ladder)
			Com_Printf("%s touching_ladder [%d]\n", __func__, level.framenum);
	}

	//if (1 && ent->is_bot == false)
	{
		//BOTLIB_GetEquipment(ent);
	}

	//rekkie -- debug drawing -- s
	//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
	//rekkie -- Walknodes -- s
	if (ent->is_bot == false && dedicated->value == 0 && client->chase_mode)
	{
		if (ent->bot.walknode.enabled) // Disable chase cam if nav_edit is enabled
			DisableChaseCam(ent);

		if (client->chase_target) // Disable nav_edit if chase cam is enabled
			ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_NONE; // Disable interaction with nodes

	}
	if (ent->is_bot == false && dedicated->value == 0 && ent->bot.walknode.enabled) // Always allow mouse control for nodes
	{
		BOTLIB_MouseControlNodes(ent, ucmd); // Use mouse to control node: link, move, add, delete, etc
	}
	// If we're allowed to interact with walknodes aka not in 'HIGHLIGHTED_TYPE_NONE' mode and ltk_showpath is off
	if (ent->is_bot == false && dedicated->value == 0 && ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_ADD && bot_showpath->value == 0 && ent->bot.walknode.enabled)
	{
		// Test if we're looking at a ladder
		qboolean on_ladder = false;
		{
			float yaw_rad = 0;
			vec3_t fwd = { 0 }, end = { 0 };
			trace_t tr;

			yaw_rad = DEG2RAD(ent->s.angles[YAW]);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);

			VectorMA(ent->s.origin, 16, fwd, end);

			tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_PLAYERSOLID);

			on_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
		}
		// Check we're touching a ladder - extended hitbox
		qboolean touching_ladder = false;
		{
			vec3_t lmins = { -20, -20, -32 };
			vec3_t lmaxs = { 20, 20, 32 };

			trace_t tr;
			tr = gi.trace(ent->s.origin, lmins, lmaxs, ent->s.origin, ent, MASK_PLAYERSOLID);
			touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
		}

		const int walknode_dist = 32;
		int nodes_touched = 0;
		int node = INVALID;
		int node_type = INVALID;
		float node_to_node_dist = 99999;
		int from, to;

		trace_t tr = gi.trace(ent->s.origin, NULL, NULL, tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 25), ent, MASK_PLAYERSOLID);
		float height_diff = ent->s.origin[2] - tr.endpos[2]; // Height difference between player and ground
		float distance = VectorDistance(ent->bot.walknode.last_ground_loc, tr.endpos); // Distance between ground touches
		float speed = VectorLength(ent->velocity); // Player speed

		//Com_Printf("%s height_diff %f\n", __func__, height_diff);

		//qboolean normal_jump = false;
		//qboolean strafe_jump = false;
		//if (ent->bot.walknode.last_ground_touch == 0 && (client->ps.pmove.pm_flags & PMF_JUMP_HELD) && ucmd->upmove > 0 && ucmd->sidemove == 0 && speed)
		//	normal_jump = true;
		//if (ent->bot.walknode.last_ground_touch == 0 && (client->ps.pmove.pm_flags & PMF_JUMP_HELD) && ucmd->upmove > 0 && ucmd->sidemove && speed)
		//	strafe_jump = true;
		//if (ent->bot.walknode.last_ground_touch == 0 && (client->ps.pmove.pm_flags & PMF_JUMP_HELD) && ucmd->upmove > 0)
		//	strafe_jump = true;
		//if (normal_jump || strafe_jump)
		//	Com_Printf("%s normal_jump[%d] strafe_jump[%d]\n", __func__, normal_jump, strafe_jump);

		// Update last ground location
		if (ent->groundentity || tr.fraction < 1)
		{
			ent->bot.walknode.last_ground_touch = 0;
			VectorCopy(tr.endpos, ent->bot.walknode.last_ground_loc);
			VectorCopy(tr.plane.normal, ent->bot.walknode.last_ground_normal);
		}
		else
			ent->bot.walknode.last_ground_touch++;

		// Nodes currently being touched
		int nodelist[MAX_NODELIST];
		nodes_touched = BOTLIB_NodeTouchNodes(ent->s.origin, tr.plane.normal, walknode_dist, ent->mins, ent->maxs, nodelist, MAX_NODELIST, INVALID);
		if (nodes_touched > 0)
		{
			if (1) // print node list
			{
				Com_Printf("[%d] Nodes touched [%d] nodelist [", level.framenum, nodes_touched);
				int count = 0;
				while (count < nodes_touched)
				{
					if (count + 1 < nodes_touched)
						Com_Printf("%d,", nodelist[count++]);
					else
						Com_Printf("%d", nodelist[count++]);
				}
				Com_Printf("]\n");
			}
			
			// Find the closest node
			int closest_node = INVALID;
			float closest_dist = 99999;
			for (i = 0; i < nodes_touched; i++)
			{
				float dist = VectorDistance(ent->s.origin, nodes[nodelist[i]].origin);
				if (dist <= walknode_dist && dist < closest_dist)
				{
					closest_dist = dist;
					closest_node = nodelist[i];
				}
			}
			node = closest_node; // Assign the closest touched node

			/*
			if (node != INVALID)
			{
				// If we're on a ladder, and the nearest node isn't a ladder node, 
				// pull it closer to the ladder and convert it to a ladder node.
				if ((on_ladder || touching_ladder) && node != INVALID && nodes[node].type != NODE_LADDER)
				{
					Com_Printf("%s %s converting [%d] to NODE_LADDER\n", __func__, ent->client->pers.netname, node);
					nodes[node].origin[0] = ent->s.origin[0]; // Pull node closer to ladder
					nodes[node].origin[1] = ent->s.origin[1]; // Pull node closer to ladder
					nodes[node].type = NODE_LADDER; // Convert node to ladder node
					// Update all links connected to this node to ladder links
					for (i = 0; i < numnodes; i++)
					{
						if (nodes[i].inuse == false) continue; // Ignore nodes not in use

						for (int l = 0; l < nodes[i].num_links; l++)
						{
							if (nodes[i].links[l].targetNode == node)
							{
								nodes[i].links[l].targetNodeType = NODE_LADDER;
								Com_Printf("%s %s updating link [%d] to NODE_LADDER\n", __func__, ent->client->pers.netname, i);
							}
						}
					}
				}

				if (ent->bot.walknode.touched_node != INVALID && node != ent->bot.walknode.touched_node)
					node_to_node_dist = VectorDistance(nodes[node].origin, nodes[ent->bot.walknode.touched_node].origin);
			}
			*/
		}


		// Assign node type
		if (ent->bot.walknode.last_ground_touch == 0)
		{
			node_type = NODE_MOVE;

			if (on_ladder || touching_ladder)
			{
				//Com_Printf("%s %s is touching a ladder\n", __func__, ent->client->pers.netname);
				node_type = NODE_LADDER;
			}

			if (ucmd->upmove < 0)
			{
				node_type = NODE_CROUCH;
				//node_type = INVALID;
			}
		}

		if (node_type != INVALID)
		{
			// If no nodes touched in the area, add a new node. Also check mins and maxs to make sure we're not adding a node while in noclip.
			if (nodes_touched == 0 && VectorEmpty(ent->mins) == false && VectorEmpty(ent->maxs) == false)
			{
				// Get the surface normal
				tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, MASK_PLAYERSOLID);
				// If the node is on a slope, raise it up depending on the slope normal and the node's mins/maxs hit box
				// This is done so the node hitbox is not inside the slope face
				{
					vec3_t exp_up;
					VectorCopy(tr.plane.normal, exp_up);
					exp_up[2] = 0;
					VectorNormalize(exp_up);
					exp_up[2] = 1;
					VectorScale(exp_up, 24, exp_up);
					VectorAdd(tr.endpos, exp_up, tr.endpos);
				}
				// Feed the raised up position back into the trace
				tr = gi.trace(tr.endpos, ent->mins, ent->maxs, ent->s.origin, ent, MASK_PLAYERSOLID);
				if (tr.startsolid == false) // Make sure we're not inside a wall
				{
					int node_added;
					if (node_added = BOTLIB_AddNode(ent->s.origin, tr.plane.normal, NODE_MOVE) != INVALID)
					{
						char typename[32] = { '\0' }; // Length of the longest node type name
						NodeTypeToString(ent, nodes[node_added].type, typename, sizeof(typename));
						Com_Printf("%s %s added node [%d] type [%s]\n", __func__, ent->client->pers.netname, node_added, typename);
					}
				}
			}

			nodes_touched = BOTLIB_NodeTouchNodes(ent->s.origin, tr.plane.normal, walknode_dist+8, ent->mins, ent->maxs, nodelist, MAX_NODELIST, INVALID);
			for (i = 0; i < nodes_touched; i++)
			{
				from = nodelist[i];
				for (int j = 0; j < nodes_touched; j++)
				{
					if (nodelist[i] == nodelist[j]) // Skip self
						continue;

					to = nodelist[j];
					if (from != INVALID && to != INVALID)
					{
						//BOTLIB_Reachability(from, to);

						float height = (nodes[to].origin[2] - nodes[from].origin[2]);
						if (height > 60) // Ignore if too high
							continue;
						if (height < -60) // Crouch when dropping down
							node_type = NODE_CROUCH;
						
						tr = gi.trace(nodes[from].origin, ent->mins, ent->maxs, nodes[to].origin, ent, MASK_PLAYERSOLID);
						if (tr.fraction == 1.0) // Don't link if we can't see the node
						{
							if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
							{
								if (BOTLIB_AddNodeLink(from, to, node_type, true))
								{
									Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, from, to, node_type);
									//ACEND_UpdateNodeReach(from, to); // Update path_table
								}
							}
						}
					}
				}
			}
			/*
			else if (node_to_node_dist <= walknode_dist+16 && ent->bot.walknode.touched_node != INVALID && ent->bot.walknode.touched_node != node)
			{
				int from = ent->bot.walknode.touched_node;
				int to = node;
				int from_type = nodes[from].type;
				int to_type = nodes[to].type;
				//float dist = VectorDistance(nodes[from].origin, nodes[to].origin);
				//if (from_type == to_type)// && dist <= 64)
				trace_t tr = gi.trace(nodes[from].origin, NULL, NULL, nodes[to].origin, ent, MASK_PLAYERSOLID | MASK_OPAQUE);
				if (tr.fraction == 1.0)
				{
					//if (BOTLIB_Reachability(from, to) == NODE_JUMPPAD)
					//	node_type = NODE_JUMPPAD;

					if (DC_AddNodeLink(from, to, node_type, true))
					{
						Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, ent->bot.walknode.touched_node, node, node_type);
						//ACEND_UpdateNodeReach(from, to); // Update path_table
					}

					//if (BOTLIB_Reachability(to, from) == NODE_JUMPPAD)
					//	node_type = NODE_JUMPPAD;

					if (DC_AddNodeLink(to, from, node_type, true))
					{
						Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, node, ent->bot.walknode.touched_node, node_type);
						//ACEND_UpdateNodeReach(to, from); // Update path_table
					}
				}
			}
			*/

			// Update last touched node
			ent->bot.walknode.touched_node = node;
		}

#if 0
		/*
		//else if (tr.fraction < 1 && (speed || ucmd->forwardmove || ucmd->sidemove || ucmd->upmove))
		else if ((tr.fraction < 1 || ent->groundentity))// && (ucmd->forwardmove || ucmd->sidemove || ucmd->upmove))
		{
			if (ent->bot.walknode.last_type == NODE_JUMP)
			{
				node_type = NODE_JUMP;
				ent->bot.walknode.last_type = INVALID;
			}
			else if (ent->bot.walknode.last_type == NODE_JUMPPAD)
			{
				//Com_Printf("%s NODE_JUMPPAD\n", __func__);
				node_type = NODE_JUMPPAD;
				ent->bot.walknode.last_type = INVALID;
			}
			else
			{
				//if (BOTLIB_VectorCompare(tr.endpos, ent->bot.last_ground_loc, STEPSIZE))
				//if (fabs(tr.endpos[2] - ent->bot.last_ground_loc[2]) <= STEPSIZE || VectorCompare(tr.plane.normal, ent->bot.last_ground_normal) || ent->bot.last_ground_touch <= 1)
				//if (ent->groundentity && (ucmd->forwardmove || ucmd->sidemove))
				if (tr.fraction < 1)
					node_type = NODE_MOVE;
				if (ucmd->upmove < 0)
					node_type = NODE_CROUCH;
				if (normal_jump)
				{
					node_type = NODE_JUMP;
					ent->bot.walknode.last_type = NODE_JUMP;
				}
				if (strafe_jump)
				{
					node_type = NODE_JUMPPAD;
					ent->bot.walknode.last_type = NODE_JUMPPAD;
				}
			}
		}
		*/
		


		//if (node_type > INVALID) Com_Printf("%s node_type[%d] framenum[%d]\n", __func__, node_type, level.framenum);

		
		if (node_type != INVALID)
		{
			if (nodes_touched == 0) // If no nodes touched in the area, add a new node
			{
				if (BOTLIB_AddNode(ent->s.origin, tr.plane.normal, node_type)) //BOTLIB_BoxIntersection
				{
					//Com_Printf("%s %s added node [%d] type [%d]\n", __func__, ent->client->pers.netname, numnodes - 1, node_type);
				}
			}

			// Get prevous touched node, if the current node and previous node are different
			// and they're the same type, and they can see each other, add a link
			// Try to link up to existing nodes
			//node = BOTLIB_TestForNodeDist(ent->s.origin, tr.plane.normal, 16, ent->absmin, ent->absmax);
			{
				int nodelist[MAX_NODELIST];
				nodes_touched = BOTLIB_NodeTouchNodes(ent->s.origin, tr.plane.normal, 8, ent->mins, ent->maxs, nodelist, MAX_NODELIST);
				if (nodes_touched > 0)
				{
					// Find the closest node
					float max_distance = 64; // Maximum distance to consider
					int closest_node = INVALID;
					float closest_dist = 99999;
					for (int i = 0; i < nodes_touched; i++)
					{
						float dist = VectorDistance(ent->s.origin, nodes[nodelist[i]].origin);
						if (dist <= max_distance && dist < closest_dist)
						{
							closest_dist = dist;
							closest_node = nodelist[i];
						}
					}
					node = closest_node; // Assign the closest touched node
					//node = nodelist[0]; // Assign the touched node

					if (0) // print node link list
					{
						for (int i = 0; i < nodes_touched; i++)
						{
							if (nodes[nodelist[i]].links[0].targetNode > INVALID)
							{
								Com_Printf("Node [%d] is linked to nodes [", nodelist[i]);
								for (int l = 0; l < nodes[nodelist[i]].num_links; l++)
								{
									Com_Printf(" %d ", nodes[nodelist[i]].links[l].targetNode);
								}
								Com_Printf("]\n");
							}
						}
					}

					if (0) // print node list
					{
						Com_Printf("Nodes touched [%d] nodelist [", nodes_touched);
						int count = 0;
						while (count < nodes_touched)
						{
							Com_Printf(" %d ", nodelist[count++]);
						}
						Com_Printf("]\n");
					}
					
				}
			}

			if (ent->bot.walknode.touched_node < 0) // Init node for the first time
				ent->bot.walknode.touched_node = node;

			//if (node > INVALID && node != ent->bot.touched_nodes[0])
			//	Com_Printf("%s %s touched node [%d] prev [%d]\n", __func__, ent->client->pers.netname, node, ent->bot.touched_nodes[0]);

			if (nodes_touched > 0 && ent->bot.walknode.touched_node > INVALID && node != ent->bot.walknode.touched_node)
			{
				//if (ent->bot.touched_nodes[0] != INVALID && ent->bot.touched_nodes[1] != INVALID)
				{
					int from = ent->bot.walknode.touched_node;
					int to = node;
					int from_type = nodes[from].type;
					int to_type = nodes[to].type;
					//float dist = VectorDistance(nodes[from].origin, nodes[to].origin);
					//if (from_type == to_type)// && dist <= 64)
					trace_t tr = gi.trace(nodes[from].origin, NULL, NULL, nodes[to].origin, ent, MASK_PLAYERSOLID | MASK_OPAQUE);
					if (tr.fraction == 1.0)
					{
						if (BOTLIB_Reachability(from, to) == NODE_JUMPPAD)
							node_type = NODE_JUMPPAD;

						if (DC_AddNodeLink(NULL, from, to, tv(0, 0, 0), false, node_type, true))
						{
							Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, ent->bot.walknode.touched_node, node, node_type);
							//ACEND_UpdateNodeReach(from, to); // Update path_table
						}

						//if (BOTLIB_Reachability(to, from) == NODE_JUMPPAD)
						//	node_type = NODE_JUMPPAD;

						if (DC_AddNodeLink(NULL, to, from, tv(0, 0, 0), false, node_type, true))
						{
							Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, node, ent->bot.walknode.touched_node, node_type);
							//ACEND_UpdateNodeReach(to, from); // Update path_table
						}

						
					}

				}
				ent->bot.walknode.touched_node = node;
			}

		}
		#endif
	}
	//rekkie -- debug drawing -- e
#endif
//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
	if (1 && ent->is_bot == false && dedicated->value == 0 && numnodes && bot_showpath->value == 0 && ent->bot.walknode.enabled)
	{
		uint32_t color, link_color, arrow_color;

		// Selection square
		//uint32_t node_color = 0;
		if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT || ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT_SMART)
		{
			if (VectorEmpty(ent->bot.walknode.selection_start) == false && VectorEmpty(ent->bot.walknode.selection_end) == false)
			{
				// Local client (call the function directly)
				if (ent->client && ent->client->pers.draw != NULL && ent->client->clientNum == 0)
				{
					if (ent->client->pers.draw->DrawSelection != NULL)
					{
						//Com_Printf("%s %s start [%f %f %f] -- end [%f %f %f]\n", __func__, ent->client->pers.netname, ent->bot.walknode.selection_start[0], ent->bot.walknode.selection_start[1], ent->bot.walknode.selection_start[2], ent->bot.walknode.selection_end[0], ent->bot.walknode.selection_end[1], ent->bot.walknode.selection_end[2]);
						void(*DrawSelection)(vec3_t start, vec3_t end, float min, float max, uint32_t color, float line_width, int time, qboolean occluded) = ent->client->pers.draw->DrawSelection;
						color = MakeColor(0, 255, 0, 255); // Green

						// Only draw when enough of the selection box has been sampled
						if (ent->bot.walknode.selection_min < 99990 && ent->bot.walknode.selection_max > -99990)
							DrawSelection(ent->bot.walknode.selection_start, ent->bot.walknode.selection_end, ent->bot.walknode.selection_min, ent->bot.walknode.selection_max, color, 3.0, 100, false); // Draw selection
					}

					if (ent->client->pers.draw->DrawBox != NULL)
					{
						void(*DrawString)(int number, vec3_t origin, const char* string, const uint32_t color, int time, qboolean occluded) = ent->client->pers.draw->DrawString;
						players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
						players[0]->client->pers.draw->strings_inuse = true; // Flag as being used
						
						players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
						void(*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = ent->client->pers.draw->DrawBox;
						
						int node = 0;
						
						

						// Show selected nodes
						for (int i = 0; i < ent->bot.walknode.selection_node_count; i++)
						{
							node = ent->bot.walknode.selection_nodes[i];

							//if (nodes[node].area > 0)
							//	node_color = nodes[node].area_color;

							// Only change node area if set by command AND it links to another area OR we're making a new area
							//if (ent->bot.walknode.selection_area_used && (area_is_linked || area_is_new))
							qboolean can_add_area_node = false; // If node can be added to area array
							if (ent->bot.walknode.selection_area_used)
							{
								// Find a free spot to add node to area array
								for (int n = 0; n < MAX_NAV_AREAS_NODES; n++)
								{
									if (DFS_area_nodes[ent->bot.walknode.selection_area][n] == INVALID)
									{
										DFS_area_nodes[ent->bot.walknode.selection_area][n] = node;
										can_add_area_node = true; // Success!
										break;
									}
								}
							}
							if (can_add_area_node)
							{
								nodes[node].area = ent->bot.walknode.selection_area; // Update node's area
								int first_area_node = DFS_area_nodes[nodes[node].area][0]; // Get first node in area

								/*
								// If one of the nodes we selected is already the area we desire
								int selected_node_of_same_area = INVALID;
								for (int n = 0; n < ent->bot.walknode.selection_node_count; n++)
								{
									if (nodes[ent->bot.walknode.selection_nodes[i]].area == ent->bot.walknode.selection_area)
									{
										selected_node_of_same_area = ent->bot.walknode.selection_nodes[i];
										break;
									}
								}
								*/

								if (ent->bot.walknode.selection_area_color) // No nodes in array so use custom color
								{
									nodes[node].area_color = ent->bot.walknode.selection_area_color;
									
									// Update all other area nodes in set
									// Find a free spot to add node to area array
									for (int n = 0; n < MAX_NAV_AREAS_NODES; n++)
									{
										if (DFS_area_nodes[ent->bot.walknode.selection_area][n] == INVALID)
											break;
										nodes[DFS_area_nodes[ent->bot.walknode.selection_area][n]].area_color = ent->bot.walknode.selection_area_color;
									}

								}
								else if (first_area_node > 0 && nodes[first_area_node].area_color) // Check for existing color
								{
									nodes[node].area_color = nodes[first_area_node].area_color; // Use existing color
								}
								else
								{
									nodes[node].area_color = MakeColor((rand() % 255), (rand() % 255), (rand() % 255), 255); // Assigned rand color
								}

								/*
								// Check for an existing area color
								//int first_area_node = DFS_area_nodes[nodes[node].area][0]; // Get first node in area
								if (first_area_node > 0 && nodes[first_area_node].area_color) // Check for existing color
								{
									nodes[node].area_color = nodes[first_area_node].area_color; // Use existing color
								}
								else if (ent->bot.walknode.selection_area_color) // No nodes in array so use custom color
									nodes[node].area_color = ent->bot.walknode.selection_area_color;
								else
									nodes[node].area_color = MakeColor(255, 255, 128, 255); // Gold  - Aassigned area nodes (area == 0)
								*/
							}

							//if (nodes[node].area_color)
							//	color = nodes[node].area_color;

							//if (node_color)
							//	color = node_color;

							if (ent->bot.walknode.selection_node_first != INVALID && nodes[node].area > 0 && nodes[node].area != nodes[ent->bot.walknode.selection_node_first].area)
								color = MakeColor(255, 0, 0, 255); // Red - Indicates the user is selecting nodes that already have an assigned area
							else
								color = MakeColor(0, 255, 0, 255); // Green - Indicates the user is selecting nodes that don't have an assigned area yet


							DrawBox(node, nodes[node].origin, color, nodes[node].mins, nodes[node].maxs, 100, true); // Draw node box
							DrawString(node, tv(nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2] + 20), va("%d", nodes[node].area), color, 100, true); // Draw node number
						}

						//*
						// Show all area nodes of <first_area_num>
						if (ent->bot.walknode.selection_node_count == 1)
						if (ent->bot.walknode.selection_node_first != INVALID && nodes[ent->bot.walknode.selection_node_first].area > 0)
						{
							if (nodes[node].area_color)
								color = nodes[node].area_color;

							//if (node_color)
							//	color = node_color;

							//color = MakeColor(0, 255, 0, 255); // Green

							for (int node = 0; node < numnodes; node++) // for each node
							{
								// Show all nodes of same area num
								if (nodes[node].area == nodes[ent->bot.walknode.selection_node_first].area)
								{
									DrawBox(node, nodes[node].origin, color, nodes[node].mins, nodes[node].maxs, 100, true); // Draw node box
								}
							}
						}
						//*/

						if (ent->bot.walknode.selection_area_used)
						{
							ent->bot.walknode.selection_area_color = 0;
							ent->bot.walknode.selection_area_used = false; // Turn off area
						}
					}
				}
			}
		}
		
		// Local client (call the function directly)
		if (ent->client && ent->client->pers.draw != NULL && ent->client->clientNum == 0)
		{
			if (ent->client->pers.draw->DrawBox != NULL)
			{
				void(*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = ent->client->pers.draw->DrawBox;
				void(*DrawArrow)(int number, vec3_t start, vec3_t end, const uint32_t color, float line_width, int time, qboolean occluded) = ent->client->pers.draw->DrawArrow;
				void(*DrawString)(int number, vec3_t origin, const char *string, const uint32_t color, int time, qboolean occluded) = ent->client->pers.draw->DrawString;
				players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
				players[0]->client->pers.draw->arrows_inuse = true; // Flag as being used
				players[0]->client->pers.draw->strings_inuse = true; // Flag as being used
				#define U32_YELLOW  MakeColor(255, 255,   0, 255)
				int linknum = 0;
				for (int node = 0; node < numnodes; node++) // for each node
				{
					if (nodes[node].inuse == false)
						continue; // Ignore nodes not in use

					if (ent->bot.walknode.highlighted_node != node && ent->bot.walknode.prev_highlighted_node != node)
					if (VectorDistance(ent->s.origin, nodes[node].origin) > 768)
						continue; // Ignore nodes too far away

					// Highlighted box
					if (ent->bot.walknode.highlighted_node == node)
						color = MakeColor(255, 0, 0, 255); // Red
					else if (nodes[node].type == NODE_JUMPPAD)
						color = MakeColor(128, 0, 128, 255); // Purple
					else if (nodes[node].type == NODE_LADDER)
						color = MakeColor(0, 255, 0, 255); // Green
					else if (nodes[node].type == NODE_WATER)
						color = MakeColor(255, 255, 255, 255); // White
					else if (nodes[node].type == NODE_CROUCH)
						color = MakeColor(255, 165, 0, 255); // Orange
					else if (nodes[node].type == NODE_BOXJUMP)
						color = MakeColor(128, 128, 128, 255); // Gray
					else if (nodes[node].type == NODE_POI)
						color = MakeColor(0, 255, 255, 255); // Cyan
					else if (nodes[node].type == NODE_POI_LOOKAT)
						color = MakeColor(0, 192, 192, 192); // Cyan-ish
					else // NODE_MOVE
						color = MakeColor(0, 0, 255, 255); // Blue

					if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT || ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT_SMART)
					{
						if (nodes[node].area > 0)
						{
							if (nodes[node].area_color > 0) // Custom color
								color = nodes[node].area_color;
							else
								color = MakeColor(255, 255, 128, 255); // Gold - Assigned area nodes (area > 0)
						}
						else
							color = MakeColor(128, 128, 128, 255); // Grey - Unassigned area nodes (area == 0)
					}

					// Draw the node links (arrows)
					if (ent->bot.walknode.highlighted_node_type != HIGHLIGHTED_NODE_SELECT && ent->bot.walknode.highlighted_node_type != HIGHLIGHTED_NODE_SELECT_SMART)
					{
						for (int link = 0; link < nodes[node].num_links; link++) // for each node - MAXLINKS
						{
							int to = nodes[node].links[link].targetNode;
							if (to != INVALID)
							{
								// Link nodes: only show links to and from highlighted node
								if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINK && ent->bot.walknode.highlighted_node != INVALID)
								{
									if (to != ent->bot.walknode.highlighted_node && node != ent->bot.walknode.highlighted_node)
										continue;
								}
								// Change link type: only show links to and from highlighted node
								if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINKTYPE)
								{
									if (ent->bot.walknode.highlighted_node != INVALID && to != ent->bot.walknode.highlighted_node && node != ent->bot.walknode.highlighted_node)
										continue;
									if (ent->bot.walknode.prev_highlighted_node != INVALID && to != ent->bot.walknode.prev_highlighted_node && node != ent->bot.walknode.prev_highlighted_node)
										continue;
								}
								switch (nodes[node].links[link].targetNodeType) {
									case NODE_MOVE:
										arrow_color = MakeColor(255, 255, 0, 255); // Yellow
										break;
									case NODE_JUMPPAD:
										arrow_color = MakeColor(128, 0, 128, 255); // Purple
										break;
									case NODE_LADDER:
										arrow_color = MakeColor(0, 255, 0, 255); // Green
										break;
									case NODE_WATER:
										arrow_color = MakeColor(255, 255, 255, 255); // White
										break;
									case NODE_CROUCH:
										arrow_color = MakeColor(255, 165, 0, 255); // Orange
										break;
									case NODE_BOXJUMP:
										arrow_color = MakeColor(128, 128, 128, 255); // Gray
										break;
									case NODE_POI:
										arrow_color = MakeColor(0, 255, 255, 255); // Cyan
										break;
									case NODE_POI_LOOKAT:
										arrow_color = MakeColor(0, 192, 192, 192); // Cyan-ish
										break;
									default:
										arrow_color = MakeColor(255, 255, 0, 255); // Yellow
										break;
								}

								DrawArrow(linknum, nodes[node].origin, nodes[to].origin, arrow_color, 1.0, 100, true); // Draw node link
								linknum++;
							}
						}
					}

					DrawBox(node, nodes[node].origin, color, nodes[node].mins, nodes[node].maxs, 100, true); // Draw node box

					if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT || ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT_SMART)
					{
						if (nodes[node].area > 0) // Only draw area num if > 0
							DrawString(node, tv(nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2] + 20), va("%d", nodes[node].area), U32_YELLOW, 100, true); // Draw area number
					}
					else {
						switch (nodes[node].links->targetNodeType) {
							case NODE_MOVE:
								link_color = MakeColor(255, 255, 0, 255); // Yellow
								break;
							case NODE_JUMPPAD:
								link_color = MakeColor(128, 0, 128, 255); // Purple
								break;
							case NODE_LADDER:
								link_color = MakeColor(0, 255, 0, 255); // Green
								break;
							case NODE_WATER:
								link_color = MakeColor(255, 255, 255, 255); // White
								break;
							case NODE_CROUCH:
								link_color = MakeColor(255, 165, 0, 255); // Orange
								break;
							case NODE_BOXJUMP:
								link_color = MakeColor(128, 128, 128, 255); // Gray
								break;
							case NODE_POI:
								link_color = MakeColor(0, 255, 255, 255); // Cyan
								break;
							case NODE_POI_LOOKAT:
								link_color = MakeColor(0, 192, 192, 192); // Cyan-ish
								break;
							default:
								link_color = MakeColor(255, 255, 0, 255); // Yellow
								break;
						}
						DrawString(node, tv(nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2] + 20), va("%d", node), link_color, 100, true); // Draw node number
					}
				}
			}
		}
		/*
		else // Networked clients (send a message)
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_DRAWARROW);
			gi.WriteLong(U32_YELLOW); // Color
			gi.WriteByte(5); // Line width
			gi.WritePosition(start);
			gi.WritePosition(end);
			//gi.WritePosition(ent->s.origin);
			//gi.WritePosition(tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + 128));
			gi.multicast(start, MULTICAST_PVS);
		}
		*/
	}
#endif
	//rekkie -- debug drawing -- e
	//rekkie -- Walknodes -- e

	//rekkie -- surface data -- e

	if (0 && ent->is_bot == false)
	{
		//ent->client->idle_weapon, ent->client->bandage_stopped, ent->client->bandaging
		//Com_Printf("%s %s idle_weapon[%d] bandage_stopped[%d] bandaging[%d]\n", __func__, ent->client->pers.netname, ent->client->idle_weapon, ent->client->bandage_stopped, ent->client->bandaging);
		//Com_Printf("%s %s viewheight[%d]\n", __func__, ent->client->pers.netname, ent->viewheight);
		
		if (0)
		{
			edict_t* self = ent; // DELETE ME !!!!!!!!!!!!!!!!!!!!!!

			int	ammo_index, rounds;
			gitem_t* ammo_item;
			qboolean loaded = true;
			qboolean clips = true;

			if (self->client->weapon == FindItem(MK23_NAME) || self->client->weapon == FindItem(DUAL_NAME) 
				|| self->client->weapon == FindItem(MP5_NAME) || self->client->weapon == FindItem(M4_NAME)
				|| self->client->weapon == FindItem(M3_NAME) || self->client->weapon == FindItem(HC_NAME)
				|| self->client->weapon == FindItem(SNIPER_NAME))
			{
				if (self->client->weapon->ammo) // Do we have ammo
				{
					ammo_item = FindItem(self->client->weapon->ammo);
					ammo_index = ITEM_INDEX(ammo_item);
					if (self->client->inventory[ammo_index] < 1)
						clips = false;
					else
						clips = true;
				}
				// Check ammo
				if (self->client->weapon == FindItem(MK23_NAME))
				{
					rounds = self->client->mk23_rds;
					if (self->client->weaponstate == WEAPON_END_MAG)
						loaded = false;
				}
				else if (self->client->weapon == FindItem(DUAL_NAME))
				{
					rounds = self->client->dual_rds;
					if (self->client->weaponstate == WEAPON_END_MAG)
						loaded = false;
				}
				else if (self->client->weapon == FindItem(MP5_NAME))
				{
					rounds = self->client->mp5_rds;
					if (rounds < 1)
						loaded = false;
				}
				else if (self->client->weapon == FindItem(M4_NAME))
				{
					rounds = self->client->m4_rds;
					if (rounds < 1)
						loaded = false;
				}
				else if (self->client->weapon == FindItem(M3_NAME))
				{
					rounds = self->client->shot_rds;
					if (rounds < self->client->shot_max)
						loaded = false;
				}
				else if (self->client->weapon == FindItem(HC_NAME))
				{
					rounds = self->client->cannon_rds;
					if (hc_single->value && self->client->pers.hc_mode) // Single barrel fire mode
					{
						if (rounds < 1)
							loaded = false;
					}
					else if (rounds < 2) // Double barrel fire mode
						loaded = false;
				}
				else if (self->client->weapon == FindItem(SNIPER_NAME))
				{
					rounds = self->client->sniper_rds;
					if (rounds < self->client->sniper_max)
						loaded = false;
				}

				//Com_Printf("%s %s inv_ammo[%d] rnds[%d] loaded[%d] clips[%d]\n", __func__, self->client->pers.netname, self->client->inventory[ammo_index], rounds, loaded, clips);

				// No ammo - drop weapon
				if (!loaded && !clips)
				{
					DropSpecialWeapon(self);
				}
				// Reload
				else if (!loaded)
				{
					Cmd_New_Reload_f(self);
				}
			}
		}
	}

	if (0 && ent->is_bot == false)
	{
		if (players[1] && players[1]->is_bot)
		{
			// Project trace from the player's weapon POV
			vec3_t      start, end;
			vec3_t      forward, right;
			vec3_t      offset;
			AngleVectors(ent->client->v_angle, forward, right, NULL);
			VectorSet(offset, 0, 0, ent->viewheight - 8);
			G_ProjectSource(ent->s.origin, offset, forward, right, start);
			float dist = VectorDistance(ent->s.origin, players[1]->s.origin);
			VectorMA(start, dist, forward, end);

			// Adjust the mins/maxs box size based on size
			vec3_t bmins = { -16, -16, +9};
			vec3_t bmaxs = { 16, 16, +10};

			// Update absolute box min/max in the world
			vec3_t absmin, absmax;
			VectorAdd(end, bmins, absmin);
			VectorAdd(end, bmaxs, absmax);

			if (BOTLIB_BoxIntersection(absmin, absmax, players[1]->absmin, players[1]->absmax)) // Do boxes intersect?
				Com_Printf("%s %s can hit %s\n", __func__, ent->client->pers.netname, players[1]->client->pers.netname);

			//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
			if (1) // Debug draw predicted enemy origin - blue is predicted, yellow is actual
			{
				uint32_t blue = MakeColor(0, 0, 255, 255); // Blue
				uint32_t yellow = MakeColor(255, 255, 0, 255); // Yellow
				void (*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = NULL;
				DrawBox = players[0]->client->pers.draw->DrawBox;
				players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
				DrawBox(players[1]->s.number, players[1]->s.origin, blue, players[1]->mins, players[1]->maxs, 100, false); // Player hitbox
				DrawBox(ent->s.number, end, yellow, bmins, bmaxs, 100, false); // Projectile hitbox
			}
#endif
			//rekkie -- debug drawing -- e
		}

	}

	if (0 && ent->is_bot == false)
	{
		// Clear peak velocity if the player stops, then starts moving again
		if (VectorEmpty(ent->velocity))
			ent->velocity_clear = true;
		if (ent->velocity_clear && VectorEmpty(ent->velocity) == false)
		{
			ent->velocity_clear = false;
			VectorClear(ent->velocity_peak);
			ent->speed_peak = 0;
		}

		if (ent->velocity[0] > ent->velocity_peak[0]) ent->velocity_peak[0] = ent->velocity[0]; // 800
		if (ent->velocity[1] > ent->velocity_peak[1]) ent->velocity_peak[1] = ent->velocity[1]; // 870
		if (ent->velocity[2] > ent->velocity_peak[2]) ent->velocity_peak[2] = ent->velocity[2]; // 260
		
		// Get the surface normal of the ground the player is standing on
		trace_t tr = gi.trace(ent->s.origin, NULL, NULL, tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 128), ent, MASK_PLAYERSOLID);

		// calculate velocity speed
		float speed = VectorLength(ent->velocity);
		if (speed > ent->speed_peak) ent->speed_peak = speed; // 937

		Com_Printf("%s speed[%f] peak[%f] normal[%f] velocity_peak[%f %f %f]\n", __func__, speed, ent->speed_peak, tr.plane.normal[2], ent->velocity_peak[0], ent->velocity_peak[1], ent->velocity_peak[2]);
	}

	

	int show_nodes_max_dist = 29999; // max dist nodes will show from selected point in space
	//show_nodes_max_dist = 99999;
	// true
	// false
	static qboolean show_distance = false;
	static qboolean show_nodes = false;
	static qboolean show_MOVE_nodes_only = false; // only shows nodes that are for MOVE - requires 'show_nodes = true'
	static qboolean show_step_nodes_only = false; // only shows nodes that are for STEPS - requires 'show_nodes = true'
	static qboolean show_item_nodes_only = false; // only shows nodes that are for ITEMS - requires 'show_nodes = true'
	static qboolean show_ladder_nodes_only = false; // only shows nodes that are for LADDER_UP / LADDER_DOWN - requires 'show_nodes = true'
	//
	static qboolean show_links = false;
	static qboolean show_MOVE_links_only = false; // only shows links that are for MOVE - requires 'show_links = true'
	//
	static qboolean print_link_details = false;
	static qboolean show_ladder_links = false;
	static qboolean bot_visit_selected_node = true; // tell bots to go to selected node (requires show_nodes = true)
	if (ent->is_bot == false && dedicated->value == 0)
	{
		// Force turn on nodes if links active
		//if (show_links && !show_nodes) show_nodes = true;
		//if (show_nodes && !show_links) show_links = true;

		// Project trace from the player's weapon POV -- s
		vec3_t      start, end;
		vec3_t      forward, right;
		vec3_t      offset;
		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 7, ent->viewheight - 8);
		offset[1] = 0;
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		VectorMA(start, 8192, forward, end);
		trace_t tr;
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
		// Project trace from the player's weapon POV -- e

		// Display nodes cloest to the player
		int latched_buttons = client->latched_buttons;
		int oldbuttons = client->buttons;
		int buttons = ucmd->buttons;
		latched_buttons |= buttons & ~oldbuttons;
		if (latched_buttons & BUTTON_ATTACK)
		{
			//LaunchPlayer(ent, tr.endpos);
			//BOTLIB_Jump_Takeoff(ent, NULL, tr.endpos, ent->viewheight, ent->velocity);
			//BOTLIB_DoParabolaJump(ent, tr.endpos);

			if (show_distance) // Calculate the distance to the target
			{
				vec3_t dist;
				//vec3_t velocity;
				VectorSubtract(tr.endpos, ent->s.origin, dist);
				float distance = VectorLength(dist);
				Com_Printf("%s dist %f\n", __func__, distance);
			}

			if (show_nodes)
			{
				int closest_dist = 99999;
				int closest_node = INVALID;
				int nodes_shown = 0;
				vec3_t v;
				float dist;
				for (int node = 0; node < numnodes; node++) // for each node
				{
					if (nodes[node].inuse == false) continue; // Ignore nodes not in use

					//if (nodes_shown > 256)
					//	break;

					if (show_MOVE_nodes_only) // only show MOVE nodes
					{
						if (nodes[node].type != NODE_MOVE) // if not a move node, continue searching
							continue;
					}
					if (show_step_nodes_only) // only show step nodes
					{
						//show_nodes_max_dist *= 3; // increase distance shown
						if (nodes[node].type != NODE_STEP) // if not a step node, continue searching
							continue;
					}
					if (show_item_nodes_only)
					{
						//show_nodes_max_dist *= 3; // increase distance shown
						if (nodes[node].type != NODE_ITEM) // if not a step node, continue searching
							continue;
					}
					if (show_ladder_nodes_only)
					{
						//show_nodes_max_dist *= 3; // increase distance shown
						if (nodes[node].type != NODE_LADDER_DOWN && nodes[node].type != NODE_LADDER_UP) // if not a step node, continue searching
							continue;
					}

					VectorSubtract(nodes[node].origin, tr.endpos, v); // subtract first
					dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

					if (dist < show_nodes_max_dist)
					{
						// Find closest node
						if (dist < closest_dist)
						{
							closest_dist = dist;
							closest_node = node;
						}
						ACEND_ShowNode(node);
						nodes_shown++;

						//ent->bot.highlighted_node = node;
						

					}
				}

				ent->show_node_links = closest_node;
				ent->show_node_links_time = level.framenum + 10.0 * HZ;
				if (closest_node >= 0)
					Com_Printf("%s %s selected node %d type %d  origin[%f %f %f]\n", __func__, ent->client->pers.netname, closest_node, nodes[closest_node].type, nodes[closest_node].origin[0], nodes[closest_node].origin[1], nodes[closest_node].origin[2]);

				// tell bots to go to selected node
				if (bot_visit_selected_node)
					if (closest_node > 0 && closest_node < numnodes)
					{
						for (int i = 0; i <= num_players; i++)
						{
							if (players[i] && players[i]->is_bot && players[i]->health > 0)
							{
								//BOTLIB_SetGoal(players[i], closest_node);
								if (BOTLIB_CanGotoNode(players[i], players[i]->bot.goal_node, false))
								{
									Com_Printf("%s %s is heading to node %d\n", __func__, players[i]->client->pers.netname, closest_node);
								}
								else
								{
									Com_Printf("%s %s cannot reach node %d\n", __func__, players[i]->client->pers.netname, closest_node);
								}
							}
						}
					}

				if (print_link_details)
					if (ent->show_node_links != INVALID && ent->show_node_links < numnodes)
					{
						//Com_Printf("%s Node %d type %d\n", __func__, nodes[ent->show_node_links].nodenum, nodes[ent->show_node_links].type);

						int targetNode;
						for (int i = 0; i < MAXLINKS; i++)
						{
							// Is target node valid
							if ((targetNode = nodes[ent->show_node_links].links[i].targetNode) > INVALID)
							{
								if (show_MOVE_links_only) // only show MOVE links
								{
									if (nodes[targetNode].type != NODE_MOVE) // if not a move link, continue searching
										continue;
								}

								float higher = 0; float lower = 99999;
								if (nodes[ent->show_node_links].origin[2] > nodes[targetNode].origin[2])
									higher = (nodes[ent->show_node_links].origin[2] - nodes[targetNode].origin[2]);
								else if (nodes[ent->show_node_links].origin[2] < nodes[targetNode].origin[2])
									lower = (nodes[targetNode].origin[2] - nodes[ent->show_node_links].origin[2]);

								VectorSubtract(nodes[targetNode].origin, nodes[ent->show_node_links].origin, v);
								dist = VectorLength(v);

								if (higher > 0)  // Node is higher than origin
									Com_Printf("%s n[%i to %i] - t[%i to %i] - dist %f z_up: %f\n", __func__, ent->show_node_links, targetNode, nodes[ent->show_node_links].type, nodes[targetNode].type, dist, higher);
								else if (lower != 99999) // Node is lower than origin
									Com_Printf("%s n[%i to %i] - t[%i to %i] - dist %f z_dn: %f\n", __func__, ent->show_node_links, targetNode, nodes[ent->show_node_links].type, nodes[targetNode].type, dist, lower);
								else // Node at equal dist to origin
									Com_Printf("%s n[%i to %i] - t[%i to %i] - dist %f z_eq: %d\n", __func__, ent->show_node_links, targetNode, nodes[ent->show_node_links].type, nodes[targetNode].type, dist, 0);
							}
						}
					}
			}
		}

		// Show links from node
		if (show_links)
			if (ent->show_node_links != INVALID && ent->show_node_links < numnodes && ent->show_node_links_time > level.framenum)
			{
				int targetNode;
				for (int i = 0; i < MAXLINKS; i++)
				{
					// Is target node valid
					if ((targetNode = nodes[ent->show_node_links].links[i].targetNode) > INVALID) // Node
					{
						if (show_MOVE_links_only) // only show MOVE links
						{
							if (nodes[targetNode].type != NODE_MOVE) // if not a move link, continue searching
								continue;
						}

						// Show only this type
						//if (nodes[ent->show_node_links].links[i].targetNodeType == NODE_MOVE)
						{
							// Show a visual laser link
							gi.WriteByte(svc_temp_entity);
							gi.WriteByte(TE_BFG_LASER);
							gi.WritePosition(nodes[ent->show_node_links].origin); // start
							gi.WritePosition(nodes[targetNode].origin); // end
							gi.multicast(nodes[ent->show_node_links].origin, MULTICAST_PHS);
						}
					}
				}
			}

		// Show all ladder links
		if (show_ladder_links)
			//if (ent->show_node_links_time > level.framenum)
		{
			for (int node = 0; node < numnodes; node++) // for each node
			{	
				if (nodes[node].inuse == false) continue; // Ignore nodes not in use
				/*
				if ((node + 1) < numnodes && nodes[node].type == NODE_LADDER_DOWN && nodes[node + 1].type == NODE_LADDER_UP) // Bottom and top of ladder
				{
					vec3_t v;
					float dist;
					VectorSubtract(nodes[node].origin, tr.endpos, v); // Find distance from player shot trace to bottom of ladder
					dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
					if (dist < 149999)
					{
						//Com_Printf("%s targetNode %i type %i\n", __func__, targetNode, nodes[targetNode].type);
						// Show a visual laser link
						gi.WriteByte(svc_temp_entity);
						gi.WriteByte(TE_BFG_LASER);
						gi.WritePosition(nodes[node].origin); // start
						gi.WritePosition(nodes[node+1].origin); // end
						gi.multicast(nodes[node].origin, MULTICAST_PHS);
					}
				}
				*/
				if (nodes[node].type == NODE_LADDER_UP || nodes[node].type == NODE_LADDER_DOWN) // Node is ladder
				//if (nodes[node].type == NODE_LADDER) // Node is ladder
				{
					int targetNode;
					for (int i = 0; i < MAXLINKS; i++)
					{
						// Is target node valid
						targetNode = nodes[node].links[i].targetNode;
						if (targetNode > INVALID) // && (nodes[targetNode].type == NODE_LADDER_UP && nodes[targetNode].type == NODE_LADDER_DOWN)) // Target is also ladder
						{
							vec3_t v;
							float dist;
							VectorSubtract(nodes[node].origin, tr.endpos, v); // subtract first
							dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
							if (dist < 9999)
							{
								//Com_Printf("%s targetNode %i type %i\n", __func__, targetNode, nodes[targetNode].type);

								// Show a visual laser link
								gi.WriteByte(svc_temp_entity);
								gi.WriteByte(TE_BFG_LASER);
								gi.WritePosition(nodes[node].origin); // start
								gi.WritePosition(nodes[targetNode].origin); // end
								gi.multicast(nodes[node].origin, MULTICAST_PHS);
							}
						}
					}
				}

			}
		}
	}
	//rekkie -- DEV_1 -- e

#ifndef NO_BOTS
	//rekkie -- remember last position if it's different
	//if (VectorDistance(ent->s.origin, ent->bot.last_position) > 8)
	//if (VectorLength(ent->velocity) > 37)
	//	VectorCopy(ent->s.origin, ent->bot.last_position);
	if (VectorCompare(ent->bot.tmp_position, ent->bot.last_position) == false && VectorCompare(ent->s.origin, ent->bot.tmp_position) == false)
		VectorCopy(ent->bot.tmp_position, ent->bot.last_position);
	if (VectorCompare(ent->s.origin, ent->bot.tmp_position) == false)
		VectorCopy(ent->s.origin, ent->bot.tmp_position);
#endif

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;
	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK) {
		//TempFile
		//We're gonna fire in this frame? Then abort any punching.
		client->punch_framenum = level.framenum;
		client->punch_desired = false;

		if (ent->solid == SOLID_NOT && ent->deadflag != DEAD_DEAD && !in_warmup) {
			client->latched_buttons = 0;

//rekkie -- DEV_1 -- prevent chase cam while show nodes is enabled -- s
#ifndef NO_BOTS
			if (ent->bot.walknode.enabled == false) // Allow chase cam if nav_edit is disabled
#endif
//rekkie -- DEV_1 -- prevent chase cam while show nodes is enabled -- e
			 NextChaseMode( ent );
		} else {
			ClientThinkWeaponIfReady( ent, false );
		}
	}

	if (client->chase_mode) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target) {
					ChaseNext(ent);
				} else {
					GetChaseTarget(ent);
				}
			}
		} else {
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
		}
	}

	if( ucmd->forwardmove || ucmd->sidemove || (client->oldbuttons != client->buttons)
		|| ((ent->solid == SOLID_NOT) && (ent->deadflag != DEAD_DEAD)) ) // No idle noises at round start.
			client->resp.idletime = 0;
	else if( ! client->resp.idletime )
		client->resp.idletime = level.framenum;
}

/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame(edict_t * ent)
{
	gclient_t *client;
	int buttonMask, going_observer = 0;

	FrameStartZ( ent );

	client = ent->client;

	if (sv_antilag->value) // if sv_antilag is enabled, we want to track our player position for later reference
		antilag_update(ent);

	//PaTMaN's jmod
	if(jump->value) {
		if ((client->resp.toggle_lca) && (client->pers.spectator))
			client->resp.toggle_lca = 0;
		else if (client->resp.toggle_lca)
			Cmd_PMLCA_f(ent);
	}

#if AQTION_EXTENSION
	// resync pm_timestamp so all limps are roughly synchronous, to try to maintain original behavior
	unsigned short world_timestamp = (int)(level.time * 1000) % 60000;
	client->ps.pmove.pm_timestamp = world_timestamp;

	// update dimension mask for team-only entities
	client->dimension_observe = 1 | (1 << client->resp.team);

	if (client->resp.hud_type == 1)
	{
		client->dimension_observe |= 0xE; // true spectators can see all teams
		HUD_SpectatorUpdate(ent);
	}
	else
	{
		HUD_ClientUpdate(ent);
	}
#endif

	if (client->resp.penalty > 0 && level.realFramenum % HZ == 0)
		client->resp.penalty--;

	if (level.intermission_framenum)
		return;

	if( team_round_going && IS_ALIVE(ent) )
		client->resp.motd_refreshes = motd_time->value;  // Stop showing motd if we're playing.
	else if( lights_camera_action )
		client->resp.last_motd_refresh = level.realFramenum;  // Don't interrupt LCA with motd.
	else if ((int)motd_time->value > client->resp.motd_refreshes * 2 && ent->client->layout != LAYOUT_MENU) {
		if (client->resp.last_motd_refresh + 2 * HZ < level.realFramenum) {
			client->resp.last_motd_refresh = level.realFramenum;
			client->resp.motd_refreshes++;
			PrintMOTD( ent );
		}
	// This next message appears perpetually until the conditions to make the message go away are met
	} else if (ent->client->layout != LAYOUT_MENU) {
		if (printrules->value) { // Do not print rules unless printrules is 1
			if (PrintGameMessage(ent)) {
				if (client->resp.last_gamemsg_refresh + 2 * HZ < level.realFramenum) {
					client->resp.last_gamemsg_refresh = level.realFramenum;
					client->resp.gamemsg_refreshes++;
				}
			}
		}
	}

	// show team or weapon menu immediately when connected
	if (auto_menu->value && ent->client->layout != LAYOUT_MENU && !client->pers.menu_shown && (teamplay->value || dm_choose->value)) {
		Cmd_Inven_f( ent );
	}

	if (!teamplay->value)
	{
		// force spawn when weapon and item selected in dm
		if (!ent->client->pers.spectator && dm_choose->value && !client->pers.dm_selected) {
			if (client->pers.chosenWeapon && (client->pers.chosenItem || itm_flags->value == 0)) {
				client->pers.dm_selected = 1;

				gi.bprintf(PRINT_HIGH, "%s joined the game\n", client->pers.netname);
				IRC_printf(IRC_T_SERVER, "%n joined the game", client->pers.netname);

				respawn(ent);

				if (!(ent->svflags & SVF_NOCLIENT)) { // send effect
					gi.WriteByte(svc_muzzleflash);
					gi.WriteShort(ent - g_edicts);
					gi.WriteByte(MZ_LOGIN);
					gi.multicast(ent->s.origin, MULTICAST_PVS);
				}
			}
			return;
		}

		if (level.framenum > client->respawn_framenum && (ent->solid == SOLID_NOT && ent->deadflag != DEAD_DEAD) != ent->client->pers.spectator)
		{
			if (ent->client->pers.spectator){
				killPlayer(ent, false);
			} else {
				gi.bprintf(PRINT_HIGH, "%s rejoined the game\n", ent->client->pers.netname);
				IRC_printf(IRC_T_SERVER, "%n rejoined the game", ent->client->pers.netname);
				respawn(ent);
			}
		}
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	ClientThinkWeaponIfReady( ent, true );
	PlayWeaponSound( ent );

	if (ent->deadflag) {
		// wait for any button just going down
		if (level.framenum > client->respawn_framenum)
		{
			// Special consideration here for Espionage, as we DO want to respawn in a GS_ROUNDBASED game
			if (teamplay->value) {
				going_observer = ((gameSettings & GS_ROUNDBASED) || !client->resp.team || client->resp.subteam);
			}
			else
			{
				going_observer = ent->client->pers.spectator;
				if (going_observer) {
					gi.bprintf(PRINT_HIGH, "%s became a spectator\n", ent->client->pers.netname);
					IRC_printf(IRC_T_SERVER, "%n became a spectator", ent->client->pers.netname);
				}
			}

			if (going_observer) {
				CopyToBodyQue(ent);
				ent->solid = SOLID_NOT;
				ent->svflags |= SVF_NOCLIENT;
				ent->movetype = MOVETYPE_NOCLIP;
				ent->health = 100;
				ent->deadflag = DEAD_NO;
				ent->client->ps.gunindex = 0;
				client->ps.pmove.delta_angles[PITCH] = ANGLE2SHORT(0 - client->resp.cmd_angles[PITCH]);
				client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT(client->killer_yaw - client->resp.cmd_angles[YAW]);
				client->ps.pmove.delta_angles[ROLL] = ANGLE2SHORT(0 - client->resp.cmd_angles[ROLL]);
				ent->s.angles[PITCH] = 0;
				ent->s.angles[YAW] = client->killer_yaw;
				ent->s.angles[ROLL] = 0;
				VectorCopy(ent->s.angles, client->ps.viewangles);
				VectorCopy(ent->s.angles, client->v_angle);
				gi.linkentity(ent);

				if (teamplay->value && !in_warmup && limchasecam->value) {
					ent->client->chase_mode = 0;
					NextChaseMode( ent );
				}

				if (esp->value) {
					// LCA countdown occurs below in EspRespawnLCA()
					// then Action!
					EspRespawnPlayer(ent);
				}
			}
			else
			{
				// in deathmatch, only wait for attack button
				buttonMask = BUTTON_ATTACK;
				if ((client->latched_buttons & buttonMask) || DMFLAGS(DF_FORCE_RESPAWN)) {
					respawn(ent);
					client->latched_buttons = 0;
				}
			}
		} else { // !(level.framenum > client->respawn_framenum)
			if (esp->value)
				EspRespawnLCA(ent);
		}
		return;
	}

	if (ent->solid != SOLID_NOT)
	{
		int idleframes = client->resp.idletime ? (level.framenum - client->resp.idletime) : 0;

		if( client->punch_desired && ! client->jumping && ! lights_camera_action && ! client->uvTime )
			punch_attack( ent );
		client->punch_desired = false;

		if( (ppl_idletime->value > 0) && idleframes && (idleframes % (int)(ppl_idletime->value * HZ) == 0) )
			//plays a random sound/insane sound, insane1-11.wav
			if (!jump->value) // Don't play insane sounds in jmod
				gi.sound( ent, CHAN_VOICE, gi.soundindex(va( "insane/insane%i.wav", rand() % 11 + 1 )), 1, ATTN_NORM, 0 );

		if( (sv_idleremove->value > 0) && (idleframes > (sv_idleremove->value * HZ)) && client->resp.team )
		{
			// Removes member from team once sv_idleremove value in seconds has been reached
			int idler_team = client->resp.team;
			if( teamplay->value )
				LeaveTeam( ent );
			if( matchmode->value )
			{
				MM_LeftTeam( ent );
				teams[ idler_team ].ready = 0;
			}
			client->resp.idletime = 0;
			gi.dprintf( "%s has been removed from play due to reaching the sv_idleremove timer of %i seconds\n",
				client->pers.netname, (int) sv_idleremove->value );
		}

		if (client->autoreloading && (client->weaponstate == WEAPON_END_MAG)
			&& (client->curr_weap == MK23_NUM)) {
			client->autoreloading = false;
			Cmd_New_Reload_f( ent );
		}

		if (client->uvTime && FRAMESYNC) {
			client->uvTime--;
			if (!client->uvTime)
			{
				if (team_round_going)
				{
					if (ctf->value && ctfgame.type == 2) {
						gi.centerprintf(ent,
							"ACTION!\n"
							"\n"
							"You are %s the %s base!",
							(client->resp.team == ctfgame.offence ?
							"ATTACKING" : "DEFENDING"),
							CTFOtherTeamName(ctfgame.offence));
					} else {
						gi.centerprintf(ent, "ACTION!");
					}
				}
			}
			else if (client->uvTime % 10 == 0)
			{
				if (ctf->value && ctfgame.type == 2) {
					gi.centerprintf(ent,
						"Shield %d\n"
						"\n"
						"You are %s the %s base!",
						client->uvTime / 10,
						(client->resp.team == ctfgame.offence ?
						"ATTACKING" : "DEFENDING"),
						CTFOtherTeamName(ctfgame.offence));
				}
				else {
					gi.centerprintf(ent, "Shield %d", client->uvTime / 10);
				}
			}
		}
	}

	if (!in_warmup || ent->movetype != MOVETYPE_NOCLIP)
		client->latched_buttons = 0;
}
