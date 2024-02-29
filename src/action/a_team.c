//-----------------------------------------------------------------------------
// Teamplay-related code for Action (formerly Axshun).  
// Some of this is borrowed from Zoid's CTF (thanks Zoid)
// -Fireblade
//
// $Id: a_team.c,v 1.88 2003/06/15 21:43:53 igor Exp $
//
//-----------------------------------------------------------------------------
// $Log: a_team.c,v $
// Revision 1.88  2003/06/15 21:43:53  igor
// added IRC client
//
// Revision 1.87  2002/04/01 14:00:08  freud
// After extensive checking I think I have found the spawn bug in the new
// system.
//
// Revision 1.86  2002/03/28 20:53:45  deathwatch
// updated credits (forgot QNI in the clan list)
//
// Revision 1.85  2002/03/28 13:34:01  deathwatch
// updated credits
//
// Revision 1.84  2002/03/28 12:10:11  freud
// Removed unused variables (compiler warnings).
// Added cvar mm_allowlock.
//
// Revision 1.83  2002/03/28 11:46:03  freud
// stat_mode 2 and timelimit 0 did not show stats at end of round.
// Added lock/unlock.
// A fix for use_oldspawns 1, crash bug.
//
// Revision 1.82  2002/03/27 15:16:56  freud
// Original 1.52 spawn code implemented for use_newspawns 0.
// Teamplay, when dropping bandolier, your drop the grenades.
// Teamplay, cannot pick up grenades unless wearing bandolier.
//
// Revision 1.81  2002/03/26 21:49:01  ra
// Bufferoverflow fixes
//
// Revision 1.80  2002/03/25 17:44:17  freud
// Small fix
//
// Revision 1.79  2002/03/24 22:45:53  freud
// New spawn code again, bad commit last time..
//
// Revision 1.77  2002/02/27 16:07:13  deathwatch
// Updated Credits menu
//
// Revision 1.76  2002/02/23 18:52:36  freud
// Added myself to the credits menu :)
//
// Revision 1.75  2002/02/23 18:33:52  freud
// Fixed newline bug with announcer (EXCELLENT.. 1 FRAG LEFT) for logfiles
//
// Revision 1.74  2002/02/23 18:12:14  freud
// Added newlines back to the CenterPrintAll for IMPRESSIVE, EXCELLENT,
// ACCURACY and X FRAGS Left, it was screwing up the logfile.
//
// Revision 1.73  2002/02/18 13:55:35  freud
// Added last damaged players %P
//
// Revision 1.72  2002/02/17 23:25:29  freud
// Fixed a small bug where stats were sent twice on votes and roundlimits
//
// Revision 1.71  2002/02/17 19:04:14  freud
// Possible bugfix for overflowing clients with stat_mode set.
//
// Revision 1.70  2002/02/03 01:07:28  freud
// more fixes with stats
//
// Revision 1.69  2002/02/01 12:54:08  ra
// messin with stat_mode
//
// Revision 1.68  2002/01/24 10:39:32  ra
// Removed an old debugging statement
//
// Revision 1.67  2002/01/24 02:24:56  deathwatch
// Major update to Stats code (thanks to Freud)
// new cvars:
// stats_afterround - will display the stats after a round ends or map ends
// stats_endmap - if on (1) will display the stats scoreboard when the map ends
//
// Revision 1.66  2001/12/30 04:00:09  ra
// Players that switch between teams before dying should also be punished.
//
// Revision 1.65  2001/12/30 03:39:52  ra
// Added to punishkills system if people do team none just before dieing
//
// Revision 1.64  2001/12/27 23:29:46  slicerdw
// Reset sub and captain vars when doing "team none"
//
// Revision 1.63  2001/12/05 15:27:35  igor_rock
// improved my english (actual -> current :)
//
// Revision 1.62  2001/12/02 16:41:52  igor_rock
// corrected the teamscores (they where switched)
//
// Revision 1.61  2001/12/02 16:16:16  igor_rock
// added "Actual Score" message after Round
//
// Revision 1.60  2001/11/25 19:09:25  slicerdw
// Fixed Matchtime
//
// Revision 1.59  2001/11/16 13:01:39  deathwatch
// Fixed 'no team wins' sound - it wont play now with use_warnings 0
// Precaching misc/flashlight.wav
//
// Revision 1.58  2001/11/03 17:33:06  ra
// Yes another warning gone
//
// Revision 1.57  2001/11/03 17:21:57  deathwatch
// Fixed something in the time command, removed the .. message from the voice command, fixed the vote spamming with mapvote, removed addpoint command (old pb command that wasnt being used). Some cleaning up of the source at a few points.
//
// Revision 1.56  2001/11/02 16:07:47  ra
// Changed teamplay spawn code so that teams dont spawn in the same place
// often in a row
//
// Revision 1.55  2001/09/30 03:09:34  ra
// Removed new stats at end of rounds and created a new command to
// do the same functionality.   Command is called "time"
//
// Revision 1.54  2001/09/29 20:18:26  ra
// Its boo boo day today
//
// Revision 1.53  2001/09/29 19:54:04  ra
// Made a CVAR to turn off extratimingstats
//
// Revision 1.52  2001/09/29 19:16:47  ra
// Made a boo boo in the timing stuff.
//
// Revision 1.51  2001/09/29 19:15:38  ra
// Added some more timing stuff
//
// Revision 1.50  2001/09/29 17:21:04  ra
// Fix a small 3teams bug
//
// Revision 1.49  2001/09/28 16:24:19  deathwatch
// use_rewards now silences the teamX wins sounds and added gibbing for the Shotgun
//
// Revision 1.48  2001/09/28 15:03:26  ra
// replacing itoa with a sprintf() call 'cause itoa is MIA on Linux
//
// Revision 1.47  2001/09/28 13:48:34  ra
// I ran indent over the sources. All .c and .h files reindented.
//
// Revision 1.46  2001/09/28 13:44:23  slicerdw
// Several Changes / improvements
//
// Revision 1.45  2001/09/26 18:13:48  slicerdw
// Fixed the roundtimelimit thingy which was ending the game ( confused with roundlimit )
//
// Revision 1.44  2001/08/08 12:42:22  slicerdw
// Ctf Should finnaly be fixed now, lets hope so
//
// Revision 1.43  2001/08/06 14:38:44  ra
// Adding UVtime for ctf
//
// Revision 1.42  2001/08/01 13:54:26  ra
// Hack to keep scoreboard from revealing whos alive during matches
//
// Revision 1.41  2001/07/30 10:17:59  igor_rock
// added some parenthesis in the 3 minutes warning clause
//
// Revision 1.40  2001/07/28 19:30:05  deathwatch
// Fixed the choose command (replaced weapon for item when it was working with items)
// and fixed some tabs on other documents to make it more readable
//
// Revision 1.39  2001/07/16 19:02:06  ra
// Fixed compilerwarnings (-g -Wall).  Only one remains.
//
// Revision 1.38  2001/07/15 02:08:40  slicerdw
// Added the "Team" section on scoreboard2 using matchmode
//
// Revision 1.37  2001/07/10 13:16:57  ra
// Fixed bug where the "3 MINUTES LEFT" warning gets printed at the begining
// of rounds that are only 2 minutes long.
//
// Revision 1.36  2001/07/09 17:55:50  slicerdw
// Small change on the Board
//
// Revision 1.35  2001/06/28 20:29:58  igor_rock
// changed the scoreboard to redruce length (and changed the debug output to report at 1023 lenght)
//
// Revision 1.34  2001/06/28 14:36:40  deathwatch
// Updated the Credits Menu a slight bit (added Kobra)
//
// Revision 1.33  2001/06/27 20:24:03  igor_rock
// changed the matchmode scoreboard completly (did a new one)
//
// Revision 1.32  2001/06/27 17:50:09  igor_rock
// fixed the vote reached bug in teamplay and matchmode
//
// Revision 1.31  2001/06/27 16:58:14  igor_rock
// corrected some limchasecam bugs
//
// Revision 1.30  2001/06/25 20:59:17  ra
//
// Adding my clantag.
//
// Revision 1.29  2001/06/22 18:37:01  igor_rock
// fixed than damn limchasecam bug - eentually :)
//
// Revision 1.28  2001/06/20 19:23:19  igor_rock
// added vcehckvotes for ctf mode "in game" ;)
//
// Revision 1.27  2001/06/20 07:21:21  igor_rock
// added use_warnings to enable/disable time/frags left msgs
// added use_rewards to enable/disable eimpressive, excellent and accuracy msgs
// change the configfile prefix for modes to "mode_" instead "../mode-" because
// they don't have to be in the q2 dir for doewnload protection (action dir is sufficient)
// and the "-" is bad in filenames because of linux command line parameters start with "-"
//
// Revision 1.26  2001/06/19 20:56:45  igor_rock
// fixed the matchmode scoreboard - finally :-)
//
// Revision 1.25  2001/06/19 18:56:38  deathwatch
// New Last killed target system
//
// Revision 1.24  2001/06/18 20:29:42  igor_rock
// subs don't respawn (says slicer :)
// splitted the scoreboard for matchmode (new if/else branch)
//
// Revision 1.23  2001/06/06 18:57:14  slicerdw
// Some tweaks on Ctf and related things
//
// Revision 1.21  2001/06/05 20:00:14  deathwatch
// Added ICE-M to credits, fixed some stuff
//
// Revision 1.20  2001/06/05 18:47:11  slicerdw
// Small tweaks to matchmode
//
// Revision 1.18  2001/06/01 19:18:42  slicerdw
// Added Matchmode Code
//
// Revision 1.17  2001/05/31 16:58:14  igor_rock
// conflicts resolved
//
// Revision 1.16.2.4  2001/05/27 11:47:53  igor_rock
// added .flg file support and timelimit bug fix
//
// Revision 1.16.2.3  2001/05/25 18:59:52  igor_rock
// Added CTF Mode completly :)
// Support for .flg files is still missing, but with "real" CTF maps like
// tq2gtd1 the ctf works fine.
// (I hope that all other modes still work, just tested DM and teamplay)
//
// Revision 1.16.2.2  2001/05/20 18:54:19  igor_rock
// added original ctf code snippets from zoid. lib compilesand runs but
// doesn't function the right way.
// Jsut committing these to have a base to return to if something wents
// awfully wrong.
//
// Revision 1.16.2.1  2001/05/20 15:17:31  igor_rock
// removed the old ctf code completly
//
// Revision 1.16  2001/05/20 12:54:18  igor_rock
// Removed newlines from Centered Messages like "Impressive"
//
// Revision 1.15  2001/05/17 16:18:13  igor_rock
// added wp_flags and did some itm_flags and other corrections
//
// Revision 1.14  2001/05/17 14:54:47  igor_rock
// added itm_flags for teamplay and ctf
//
// Revision 1.13  2001/05/13 01:23:01  deathwatch
// Added Single Barreled Handcannon mode, made the menus and scoreboards
// look nicer and made the voice command a bit less loud.
//
// Revision 1.12  2001/05/12 19:17:03  slicerdw
// Changed The Join and Weapon choosing Menus
//
// Revision 1.11  2001/05/12 18:38:27  deathwatch
// Tweaked MOTD and Menus some more
//
// Revision 1.10  2001/05/12 17:36:33  deathwatch
// Edited the version variables and updated the menus. Added variables:
// ACTION_VERSION, TNG_VERSION and TNG_VERSION2
//
// Revision 1.9  2001/05/12 14:52:47  mort
// Fixed bug of people being able to respawn when choosing a new item
//
// Revision 1.8  2001/05/12 13:48:58  mort
// Fixed CTF ForceSpawn bug
//
// Revision 1.7  2001/05/12 08:20:01  mort
// CTF bug fix, makes sure flags have actually spawned before certain functions attempt to use them
//
// Revision 1.6  2001/05/11 16:12:03  mort
// Updated path locations for CTF flag loading and CTF hacked spawns
//
// Revision 1.5  2001/05/11 16:07:25  mort
// Various CTF bits and pieces...
//
// Revision 1.4  2001/05/11 12:21:18  slicerdw
// Commented old Location support ( ADF ) With the ML/ETE Compatible one
//
// Revision 1.2  2001/05/07 20:06:45  igor_rock
// changed sound dir from sound/rock to sound/tng
//
// Revision 1.1.1.1  2001/05/06 17:24:38  igor_rock
// This is the PG Bund Edition V1.25 with all stuff laying around here...
//
//-----------------------------------------------------------------------------

#include "g_local.h"
#include "cgf_sfx_glass.h"


qboolean team_game_going = false;	// is a team game going right now?
qboolean team_round_going = false;	// is an actual round of a team game going right now?

int team_round_countdown = 0;	// countdown variable for start of a round
int rulecheckfrequency = 0;	// accumulator variable for checking rules every 1.5 secs
int lights_camera_action = 0;	// countdown variable for "lights...camera...action!" 
int timewarning = 0;		// countdown variable for "x Minutes left"
int fragwarning = 0;		// countdown variable for "x Frags left"
int holding_on_tie_check = 0;	// when a team "wins", countdown for a bit and wait...
int current_round_length = 0;	// frames that the current team round has lasted
int round_delay_time = 0;	// time gap between round end and new round
int in_warmup = 0;		// if warmup is currently on
qboolean teams_changed = false;  // Need to update the join menu.

team_t teams[TEAM_TOP];
int	teamCount = 2;
int gameSettings;

//#define MAX_SPAWNS 512		// moved to g_local.h

edict_t *potential_spawns[MAX_SPAWNS];
int num_potential_spawns;
edict_t *teamplay_spawns[MAX_TEAMS];
trace_t trace_t_temp;		// used by our trace replace macro in ax_team.h

// <TNG:Freud New spawning variables>
int NS_num_used_farteamplay_spawns[MAX_TEAMS];
int NS_num_potential_spawns[MAX_TEAMS];
edict_t *NS_potential_spawns[MAX_TEAMS][MAX_SPAWNS];
edict_t *NS_used_farteamplay_spawns[MAX_TEAMS][MAX_SPAWNS];
int NS_randteam;
// </TNG:Freud>
hl_weapon_status_t weapon_status[WEAPON_MAX][TEAM_TOP];

void CreditsMenu (edict_t * ent, pmenu_t * p);
static transparent_list_t transparentList[MAX_CLIENTS];
static size_t transparentEntryCount = 0;
transparent_list_t *transparent_list = NULL;
static transparent_list_t *transparentlistFree = NULL;

void InitTransparentList( void )
{
	transparent_list = NULL;
	transparentlistFree = NULL;
	transparentEntryCount = 0;
}

void AddToTransparentList( edict_t *ent )
{
	transparent_list_t *entry;

	if (transparentlistFree) {
		entry = transparentlistFree;
		transparentlistFree = entry->next;
	}
	else if (transparentEntryCount < MAX_CLIENTS) {
		entry = &transparentList[transparentEntryCount++];
	}
	else {
		return;
	}

	entry->ent = ent;
	entry->next = transparent_list;
	transparent_list = entry;
}

void RemoveFromTransparentList( edict_t *ent )
{
	transparent_list_t *entry, **back;

	back = &transparent_list;
	
	for (entry = *back; entry; entry = *back) {
		if (entry->ent == ent) {
			*back = entry->next;
			entry->next = transparentlistFree;
			transparentlistFree = entry;
			return;
		}
		
		back = &entry->next;
	}
}

void TransparentListSet( solid_t solid_type )
{
	transparent_list_t *entry;

	for (entry = transparent_list; entry; entry = entry->next) {
		if (entry->ent->solid == solid_type)
			continue;

		entry->ent->solid = solid_type;
		gi.linkentity( entry->ent );
	}
}

qboolean OnTransparentList( const edict_t *ent )
{
	const transparent_list_t *entry;

	for( entry = transparent_list; entry; entry = entry->next )
	{
		if( entry->ent == ent )
			return true;
	}

	return false;
}

void ReprintMOTD (edict_t * ent, pmenu_t * p)
{
	PMenu_Close (ent);
	PrintMOTD (ent);
}

char* PrintMatchRules(void)
{
	static char rulesmsg[1024];

	// Espionage rules
	if (esp->value) {
		int rndlimit = (int)roundlimit->value;
		if (atl->value) {
			if (!teams[TEAM1].leader && !teams[TEAM2].leader && !teams[TEAM3].leader) {
				Q_snprintf( rulesmsg, sizeof( rulesmsg ), "Frag the other team's leader to win, but don't forget to protect your own!\n");
			}
			else if (teamCount == TEAM2) {
				Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s leader: %s (%s)\n\n%s leader: %s (%s)\n\nFrag the other team's leader to win, but don't forget to protect your own!\n",
					teams[TEAM1].name, teams[TEAM1].leader->client->pers.netname, teams[TEAM1].leader_name,
					teams[TEAM2].name, teams[TEAM2].leader->client->pers.netname, teams[TEAM2].leader_name );
				} else {
					Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s leader: %s (%s)\n\n%s leader: %s (%s)\n\n%s leader: %s (%s)\n\nFrag the other team's leaders to win, but don't forget to protect your own!\n",
					teams[TEAM1].name, teams[TEAM1].leader->client->pers.netname, teams[TEAM1].leader_name,
					teams[TEAM2].name, teams[TEAM2].leader->client->pers.netname, teams[TEAM2].leader_name,
					teams[TEAM3].name, teams[TEAM3].leader->client->pers.netname, teams[TEAM3].leader_name );
				}
				// Roundlimit info
				static char addmsg[64];
				Q_snprintf(addmsg, sizeof(addmsg), "\n** The first team to %i points wins! **", (int)rndlimit);
				Q_strncatz( rulesmsg, addmsg, sizeof( rulesmsg ) );

		} else if (etv->value) {
				if (!teams[TEAM1].leader)
					Q_snprintf( rulesmsg, sizeof( rulesmsg ), "\nTeam 1 is trying to escort their leader to the briefcase!\n\nTeam 2 is tasked with preventing that from happening!\n");
				else if (teams[TEAM1].leader) {
					Q_snprintf( rulesmsg, sizeof( rulesmsg ), "\n%s: Escort your leader %s to the %s! Don't get them killed!\n\n%s: DO NOT let %s get to the %s! Use lethal force!\n\nTeam 1 Respawn Timer: %i seconds\nTeam 2 Respawn Timer: %i seconds\n\nThe first team to %i points wins!\n",
						teams[TEAM1].name, teams[TEAM1].leader->client->pers.netname, espsettings.target_name, 
						teams[TEAM2].name, teams[TEAM1].leader->client->pers.netname, espsettings.target_name, 
						(int)teams[TEAM1].respawn_timer, (int)teams[TEAM2].respawn_timer,
						rndlimit );
			// Append a little extra if halftime is enabled
				if(esp_etv_halftime->value && roundlimit->value > 3){
					static char addmsg[64];
					Q_snprintf(addmsg, sizeof(addmsg), "\n** Halftime is enabled: Teams switch at round %i **", (int)rndlimit/2);
					Q_strncatz( rulesmsg, addmsg, sizeof( rulesmsg ) );
				}
			}
		}
	}
	// CTF rules
	else if (ctf->value)
	{
		if (!capturelimit->value) {
			Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s\nvs\n%s\n\nCapture the other team's flag!\nNo capturelimit set!\n",
			teams[TEAM1].name, teams[TEAM2].name );
		} else {
			Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s\nvs\n%s\n\nCapture the other team's flag!\nThe first team to %s captures wins!\n",
			teams[TEAM1].name, teams[TEAM2].name, capturelimit->string );
		}
	}
	// Domination rules
	else if (dom->value)
	{
		if (teamCount == TEAM2) {
			Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s\nvs\n%s\n\nCapture all of the checkpoints!\n",
				teams[TEAM1].name, teams[TEAM2].name );
		} else {
			Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s\nvs\n%s\nvs\n%s\n\nCapture all of the checkpoints!\n",
				teams[TEAM1].name, teams[TEAM2].name, teams[TEAM3].name );
		}
	}
	// This covers both Teamplay and TeamDM
	else if (teamplay->value)
	{
		if (teamCount == TEAM2) {
			Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s\nvs\n%s\n\nFrag the other team!\n",
				teams[TEAM1].name, teams[TEAM2].name );
		} else if (teamCount == TEAM3) {
			Q_snprintf( rulesmsg, sizeof( rulesmsg ), "%s\nvs\n%s\nvs\n%s\n\nFrag the other teams!\n",
				teams[TEAM1].name, teams[TEAM2].name, teams[TEAM3].name );
		}
	}
	else // If nothing else matches, just say glhf
	{
		Q_snprintf( rulesmsg, sizeof( rulesmsg ), "Frag 'em all!  Good luck and have fun!\n");
	}

    if (rulesmsg[0] == '\0') {
        // Handle empty
        return "";
    }

	return rulesmsg;
}

void JoinTeamAuto (edict_t * ent, pmenu_t * p)
{
	int i, team = TEAM1, num1 = 0, num2 = 0, num3 = 0, score1, score2, score3;

	for (i = 0; i < game.maxclients; i++)
	{
		if (!g_edicts[i + 1].inuse)
			continue;
		if (game.clients[i].resp.team == TEAM1)
			num1++;
		else if (game.clients[i].resp.team == TEAM2)
			num2++;
		else if (game.clients[i].resp.team == TEAM3)
			num3++;
	}

	score1 = teams[TEAM1].score;
	score2 = teams[TEAM2].score;
	score3 = teams[TEAM3].score;

	if(ctf->value) {
		CTFCalcScores();
		GetCTFScores(&score1, &score2);
	}

	/* there are many different things to consider when selecting a team */
	if (num1 > num2 || (num1 == num2 && score1 > score2))
		team = TEAM2;

	if (teamCount == 3)
	{
		if (team == TEAM1)
		{
			if (num1 > num3 || (num1 == num3 && score1 > score3))
				team = TEAM3;
		}
		else
		{
			if (num2 > num3 || (num2 == num3 && score2 > score3))
				team = TEAM3;
		}
	}

	JoinTeam(ent, team, 0);
}

void JoinTeamAutobalance (edict_t * ent)
{
	JoinTeamAuto(ent, NULL);
}

void JoinTeam1 (edict_t * ent, pmenu_t * p)
{
	JoinTeam(ent, TEAM1, 0);
}

void JoinTeam2 (edict_t * ent, pmenu_t * p)
{
	JoinTeam(ent, TEAM2, 0);
}

void JoinTeam3 (edict_t * ent, pmenu_t * p)
{
	if (teamCount == 3)
		JoinTeam(ent, TEAM3, 0);
}

void LeaveTeams (edict_t * ent, pmenu_t * p)
{
	LeaveTeam(ent);
	PMenu_Close(ent);
	OpenJoinMenu(ent);
}

void SelectWeapon2(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, MP5_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(MP5_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/mp5slide.wav"), 1.0);
}

void SelectWeapon3(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, M3_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(M3_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/m3in.wav"), 1.0);
}

void SelectWeapon4(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, HC_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(HC_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/cclose.wav"), 1.0);
}

void SelectWeapon5(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, SNIPER_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(SNIPER_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/ssgbolt.wav"), 1.0);
}

void SelectWeapon6(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, M4_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(M4_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/m4a1slide.wav"), 1.0);
}

void SelectWeapon0(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, KNIFE_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(KNIFE_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/swish.wav"), 1.0);
}

void SelectWeapon9(edict_t *ent, pmenu_t *p)
{
	if (highlander->value && !Highlander_Check(ent, DUAL_NUM))
		return;

	ent->client->pers.chosenWeapon = GET_ITEM(DUAL_NUM);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
	unicastSound(ent, gi.soundindex("weapons/mk23slide.wav"), 1.0);
}

void UnequipWeapon(edict_t *ent)
{
	ent->client->pers.chosenWeapon = NULL;
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("weapons/swish.wav"), 1.0);
}

void SelectItem1(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(KEV_NUM);
	if(item_kit_mode->value){
		// This is so it clears the chosenItem2 if a previous kit was chosen
		ent->client->pers.chosenItem2 = NULL;
	}
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/veston.wav"), 1.0);
}

void SelectItem2(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(LASER_NUM);
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/lasersight.wav"), 1.0);
}

void SelectItem3(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(SLIP_NUM);
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/veston.wav"), 1.0);
}

void SelectItem4(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(SIL_NUM);
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/screw.wav"), 1.0);
}

void SelectItem5(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(BAND_NUM);
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/veston.wav"), 1.0);
}

void SelectItem6(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(HELM_NUM);
	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/veston.wav"), 1.0);
}

// Commando kit
void SelectKit1(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(BAND_NUM);
	ent->client->pers.chosenItem2 = GET_ITEM(HELM_NUM);

	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/veston.wav"), 1.0);
}

// Stealth kit
void SelectKit2(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(SLIP_NUM);
	ent->client->pers.chosenItem2= GET_ITEM(SIL_NUM);

	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/screw.wav"), 1.0);
}

// Assassin kit
void SelectKit3(edict_t *ent, pmenu_t *p)
{
	ent->client->pers.chosenItem = GET_ITEM(LASER_NUM);
	ent->client->pers.chosenItem2= GET_ITEM(SIL_NUM);

	PMenu_Close(ent);
	unicastSound(ent, gi.soundindex("misc/lasersight.wav"), 1.0);
}

// newrand returns n, where 0 >= n < top
int newrand (int top)
{
	return (int) (random () * top);
}

void SelectRandomWeapon(edict_t *ent, pmenu_t *p)
{
	menu_list_weapon weapon_list[7] = {
		{ .num = MP5_NUM, .sound = "weapons/mp5slide.wav", .name = MP5_NAME },
		{ .num = M3_NUM, .sound = "weapons/m3in.wav", .name = M3_NAME },
		{ .num = HC_NUM, .sound = "weapons/cclose.wav", .name = HC_NAME },
		{ .num = SNIPER_NUM, .sound = "weapons/ssgbolt.wav", .name = SNIPER_NAME },
		{ .num = M4_NUM, .sound = "weapons/m4a1slide.wav", .name = M4_NAME },
		{ .num = KNIFE_NUM, .sound = "weapons/swish.wav", .name = KNIFE_NAME },
		{ .num = DUAL_NUM, .sound = "weapons/mk23slide.wav", .name = DUAL_NAME }
	};

	int rand = newrand(7);
	menu_list_weapon selected_weapon = weapon_list[rand];
	// prevent picking current weapon
	if (ent->client->pers.chosenWeapon) {
		while (selected_weapon.num == ent->client->pers.chosenWeapon->typeNum)
		{
			rand = newrand(7);
			selected_weapon = weapon_list[rand];
		}
	}
	
	ent->client->pers.chosenWeapon = GET_ITEM(selected_weapon.num);
	unicastSound(ent, gi.soundindex(selected_weapon.sound), 1.0);
	gi.centerprintf(ent, "You selected %s", selected_weapon.name);
	PMenu_Close(ent);
	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
}

void SelectRandomItem(edict_t *ent, pmenu_t *p)
{
	int selected_weapon = ent->client->pers.chosenWeapon->typeNum;

	// Create array with limited items on certain weapons to not have silly kombos
	menu_list_item item_list[6] = {
		{ .num = KEV_NUM, .sound = "misc/veston.wav", .name = KEV_NAME },
		{ .num = SLIP_NUM, .sound = "misc/veston.wav", .name = SLIP_NAME },
		{ .num = BAND_NUM, .sound = "misc/veston.wav", .name = BAND_NAME },
		{ .num = HELM_NUM, .sound = "misc/veston.wav", .name = HELM_NAME },
    };
	int listCount = 4;

	menu_list_item item_sil = { .num = SIL_NUM, .sound = "misc/screw.wav", .name = SIL_NAME };
	menu_list_item item_las = { .num = LASER_NUM, .sound = "misc/lasersight.wav", .name = LASER_NAME };

	if (selected_weapon == SNIPER_NUM)
	{
		item_list[4] = item_sil;
		listCount = 5;
	}
	if (selected_weapon == M4_NUM)
	{
		item_list[4] = item_las;
		listCount = 5;
	}
	if (selected_weapon == MP5_NUM)
	{
		item_list[4] = item_sil;
		item_list[5] = item_las;
		listCount = 6;
	}

	int rand = newrand(listCount);
	menu_list_item selected_item = item_list[rand];

	if (ent->client->pers.chosenItem) {
		while (selected_item.num == ent->client->pers.chosenItem->typeNum && selected_item.num < SIL_NUM)
		{
			rand = newrand(listCount);
			selected_item = item_list[rand];
		}
	} else {
		while (selected_item.num < SIL_NUM)
		{
			rand = newrand(listCount);
			selected_item = item_list[rand];
		}
	}

	ent->client->pers.chosenItem = GET_ITEM(selected_item.num);
	unicastSound(ent, gi.soundindex(selected_item.sound), 1.0);
	gi.centerprintf(ent, "You selected %s", selected_item.name);
	PMenu_Close(ent);
}

void SelectRandomWeaponAndItem(edict_t *ent, pmenu_t *p)
{
	int i;
	int rand = newrand(7);
	// WEAPON
	menu_list_weapon weapon_list[7] = {
		{ .num = MP5_NUM, .sound = "weapons/mp5slide.wav", .name = MP5_NAME },
		{ .num = M3_NUM, .sound = "weapons/m3in.wav", .name = M3_NAME },
		{ .num = HC_NUM, .sound = "weapons/cclose.wav", .name = HC_NAME },
		{ .num = SNIPER_NUM, .sound = "weapons/ssgbolt.wav", .name = SNIPER_NAME },
		{ .num = M4_NUM, .sound = "weapons/m4a1slide.wav", .name = M4_NAME },
		{ .num = KNIFE_NUM, .sound = "weapons/swish.wav", .name = KNIFE_NAME },
		{ .num = DUAL_NUM, .sound = "weapons/mk23slide.wav", .name = DUAL_NAME }
	};

	menu_list_weapon selected_weapon = weapon_list[rand];
	// prevent picking current weapon
	if (ent->client->pers.chosenWeapon) {
		while (selected_weapon.num == ent->client->pers.chosenWeapon->typeNum)
		{
			rand = newrand(7);
			selected_weapon = weapon_list[rand];
		}
	}
	
	ent->client->pers.chosenWeapon = GET_ITEM(selected_weapon.num);
	unicastSound(ent, gi.soundindex(selected_weapon.sound), 1.0);

	// ITEM
	// Create array with limited items on certain weapons to not have silly kombos
	menu_list_item item_list[6] = {
		{ .num = KEV_NUM, .sound = "misc/veston.wav", .name = KEV_NAME },
		{ .num = SLIP_NUM, .sound = "misc/veston.wav", .name = SLIP_NAME },
		{ .num = BAND_NUM, .sound = "misc/veston.wav", .name = BAND_NAME },
		{ .num = HELM_NUM, .sound = "misc/veston.wav", .name = HELM_NAME },
	};
	int listCount = 4;

	menu_list_item item_sil = { .num = SIL_NUM, .sound = "misc/screw.wav", .name = SIL_NAME };
	menu_list_item item_las = { .num = LASER_NUM, .sound = "misc/lasersight.wav", .name = LASER_NAME };

	if (selected_weapon.num == SNIPER_NUM)
	{
		item_list[4] = item_sil;
		listCount = 5;
	}
	if (selected_weapon.num == M4_NUM)
	{
		item_list[4] = item_las;
		listCount = 5;
	}
	if (selected_weapon.num == MP5_NUM)
	{
		item_list[4] = item_sil;
		item_list[5] = item_las;
		listCount = 6;
	}

	rand = newrand(listCount);
	menu_list_item selected_item = item_list[rand];

	if (ent->client->pers.chosenItem) {
		while (selected_item.num == ent->client->pers.chosenItem->typeNum)
		{
			rand = newrand(listCount);
			selected_item = item_list[rand];
		}
	}

	for (i = 0; i < listCount; i++) {
		gi.cprintf(ent, PRINT_HIGH, "%i %s\n", item_list[i].num, item_list[i].name);
	}

	ent->client->pers.chosenItem = GET_ITEM(selected_item.num);
	unicastSound(ent, gi.soundindex(selected_item.sound), 1.0);
	gi.centerprintf(ent, "You selected %s and %s", selected_weapon.name, selected_item.name);
	PMenu_Close(ent);
}

void CreditsReturnToMain (edict_t * ent, pmenu_t * p)
{
	PMenu_Close (ent);
	if (teamplay->value) {
		OpenJoinMenu (ent);
	}
}

//PG BUND BEGIN
void DoAGoodie (edict_t * ent, pmenu_t * p)
{
  //PG BUND
	unicastSound(ent, gi.soundindex("boss3/bs3idle1.wav"), 1.0);
	//stuffcmd (ent, "play boss3/bs3idle1.wav");
}
//PG BUND END

// AQ2:TNG - Igor adding the Rock-Sound ;-)
void RockClan (edict_t * ent, pmenu_t * p)
{
  gi.cprintf (ent, PRINT_HIGH, "Let's Rock! http://www.rock-clan.de/\n");
  //PG BUND
  unicastSound(ent, gi.soundindex("user/letsrock.wav"), 1.0);
  //stuffcmd (ent, "play user/letsrock.wav");
}
// AQ2:TNG - End Rock-Sound

// AQ2:TNG Deathwatch - Just for slicer :)
void SlicersCat (edict_t * ent, pmenu_t * p)
{
  gi.cprintf (ent, PRINT_HIGH, "sLiCeR [dW] couldn't have done it without his cat!\n");
  //PG BUND
  unicastSound(ent, gi.soundindex("makron/laf4.wav"), 1.0);
  //stuffcmd (ent, "play makron/laf4.wav");
}
// AQ2:TNG End

// AQ2:TNG Deathwatch - Just for QNI ppl
void QuakeNigguhz (edict_t * ent, pmenu_t * p)
{
  gi.cprintf (ent, PRINT_HIGH, "For all the homies!\n");
  //PG BUND
  unicastSound(ent, gi.soundindex("world/xian1.wav"), 1.0);
  //stuffcmd (ent, "play world/xian1.wav");
}

// AQ2:TNG Deathwatch - Editing all menus to show the correct credits, version, names, locations, urls, etc
pmenu_t creditsmenu[] = {
  {"*" TNG_TITLE, PMENU_ALIGN_CENTER, NULL, NULL},
  {"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F", PMENU_ALIGN_CENTER, NULL, NULL},
  {"*Design Team", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Deathwatch", PMENU_ALIGN_LEFT, NULL, DoAGoodie},
	{"Elviz", PMENU_ALIGN_LEFT, NULL, DoAGoodie},
	{"Freud [QNI]", PMENU_ALIGN_LEFT, NULL, QuakeNigguhz},
  {"Igor[Rock]", PMENU_ALIGN_LEFT, NULL, RockClan},
  {"JBravo[QNI]", PMENU_ALIGN_LEFT, NULL, QuakeNigguhz},
  {"sLiCeR [dW]", PMENU_ALIGN_LEFT, NULL, SlicersCat},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"*Credits", PMENU_ALIGN_LEFT, NULL, NULL},
  {"(in no particular order)", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Clan Rock, dW, QNI & DP,", PMENU_ALIGN_LEFT, NULL, NULL},
  {"Kobra, Zarjazz,", PMENU_ALIGN_LEFT, NULL, NULL},
  {"Killerbee, Rookie[Rock],", PMENU_ALIGN_LEFT, NULL, NULL},
  {"PG Bund[Rock], Mort,", PMENU_ALIGN_LEFT, NULL, NULL},
  {"ICE-M, Palmtree,", PMENU_ALIGN_LEFT, NULL, NULL},
	{"Tempfile, Blackmonk,", PMENU_ALIGN_LEFT, NULL, NULL},
	{"Dome, Papst, Apr/ Maniac", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Return to main menu", PMENU_ALIGN_LEFT, NULL, CreditsReturnToMain},
  {"TAB to exit menu", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"v" VERSION, PMENU_ALIGN_RIGHT, NULL, NULL},
//PG BUND END
};

pmenu_t weapmenu[] = {
  {"*" TNG_TITLE, PMENU_ALIGN_CENTER, NULL, NULL},
  {"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F", PMENU_ALIGN_CENTER, NULL, NULL},
  {"Select your Weapon", PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  //AQ2:TNG - Igor adding wp_flags
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "MP5/10 Submachinegun", SelectWeapon2
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "M3 Super90 Assault Shotgun", SelectWeapon3
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Handcannon", SelectWeapon4
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "SSG 3000 Sniper Rifle", SelectWeapon5
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "M4 Assault Rifle", SelectWeapon6
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Combat Knives", SelectWeapon0
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Akimbo Pistols", SelectWeapon9
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Random Weapon", PMENU_ALIGN_LEFT, NULL, SelectRandomWeapon},
  {"Random Weapon and Item", PMENU_ALIGN_LEFT, NULL, SelectRandomWeaponAndItem},
  //AQ2:TNG End adding wp_flags
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  //AQ2:TNG - Slicer: changing this
  //{"Leave Team", PMENU_ALIGN_LEFT, NULL, LeaveTeams},
  {"Return to Main Menu", PMENU_ALIGN_LEFT, NULL, CreditsReturnToMain},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  //AQ2:TNG END
  {"Use arrows to move cursor", PMENU_ALIGN_LEFT, NULL, NULL},
  {"ENTER to select", PMENU_ALIGN_LEFT, NULL, NULL},
  {"TAB to exit menu", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"v" VERSION, PMENU_ALIGN_RIGHT, NULL, NULL},
};

pmenu_t itemmenu[] = {
  {"*" TNG_TITLE, PMENU_ALIGN_CENTER, NULL, NULL},
  {"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F", PMENU_ALIGN_CENTER, NULL, NULL},
  {"Select your Item", PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  //AQ2:TNG Igor adding itm_flags
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Kevlar Vest", SelectItem1
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Laser Sight", SelectItem2
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Stealth Slippers", SelectItem3
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Silencer", SelectItem4
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Bandolier", SelectItem5
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},	// "Kevlar Helmet", SelectItem6
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Random Item", PMENU_ALIGN_LEFT, NULL, SelectRandomItem},
  //AQ2:TNG end adding itm_flags
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Use arrows to move cursor", PMENU_ALIGN_LEFT, NULL, NULL},
  {"ENTER to select", PMENU_ALIGN_LEFT, NULL, NULL},
  {"TAB to exit menu", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"v" VERSION, PMENU_ALIGN_RIGHT, NULL, NULL},
};

pmenu_t itemkitmenu[] = {
  {"*" TNG_TITLE, PMENU_ALIGN_CENTER, NULL, NULL},
  {"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F", PMENU_ALIGN_CENTER, NULL, NULL},
  {"Select your Item", PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  //AQ2:TNG Igor adding itm_flags
  {KEV_NAME, PMENU_ALIGN_LEFT, NULL, NULL},	// "Kevlar Vest", SelectItem1
  {C_KIT_NAME, PMENU_ALIGN_LEFT, NULL, NULL},	// "Commando Kit", SelectKit1
  {S_KIT_NAME, PMENU_ALIGN_LEFT, NULL, NULL},	// "Stealth Kit", SelectKit2
  {A_KIT_NAME, PMENU_ALIGN_LEFT, NULL, NULL},	// "Assassin Kit", SelectKit3
  //AQ2:TNG end adding itm_flags
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Use arrows to move cursor", PMENU_ALIGN_LEFT, NULL, NULL},
  {"ENTER to select", PMENU_ALIGN_LEFT, NULL, NULL},
  {"TAB to exit menu", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"v" VERSION, PMENU_ALIGN_RIGHT, NULL, NULL},
};

pmenu_t randmenu[] = {
  {"*" TNG_TITLE, PMENU_ALIGN_CENTER, NULL, NULL},
  {"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F", PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Weapon menu disabled!", PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"You get random weapon and item", PMENU_ALIGN_CENTER, NULL, NULL},
  {"when round begins.", PMENU_ALIGN_CENTER, NULL, NULL},
  //AQ2:TNG end adding itm_flags
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Return to Main Menu", PMENU_ALIGN_LEFT, NULL, CreditsReturnToMain},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Use arrows to move cursor", PMENU_ALIGN_LEFT, NULL, NULL},
  {"ENTER to select", PMENU_ALIGN_LEFT, NULL, NULL},
  {"TAB to exit menu", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"v" VERSION, PMENU_ALIGN_RIGHT, NULL, NULL},
};

/*
PaTMaN's JMOD
*/
void ToggleLaser(edict_t *ent, pmenu_t *p)
{
	Cmd_Toggle_f(ent, "laser");
}

void ToggleSlippers(edict_t *ent, pmenu_t *p)
{
	//strcpy(gi.args(),"togglecode slippers");
	Cmd_Toggle_f(ent, "slippers");
}


//PaTMaN - Item Menu
pmenu_t pmitemmenu[] = {
  {"*Item Menu                  (command binds)",		PMENU_ALIGN_LEFT,	NULL, NULL					},
  { "-----------------------------------------------",	PMENU_ALIGN_LEFT,	NULL, NULL					},
  { "Laser Sight                (jmod laser)",			PMENU_ALIGN_LEFT,	NULL, ToggleLaser			},
  { "Slippers                   (jmod slippers)",		PMENU_ALIGN_LEFT,	NULL, ToggleSlippers		},
  { NULL,												PMENU_ALIGN_LEFT,	NULL, NULL					},
  { "Respawn to Closest Spawn   (jmod spawnc)",			PMENU_ALIGN_LEFT,	NULL, Cmd_GotoPC_f   		},
  { "Respawn to Random Spawn    (jmod spawnp)",			PMENU_ALIGN_LEFT,	NULL, Cmd_GotoP_f   		},

};


void OpenPMItemMenu(edict_t *ent)
{
	PMenu_Open(ent, pmitemmenu, 2, sizeof(pmitemmenu) / sizeof(pmenu_t));
	ent->client->curr_menu = MENU_PMITEM;
}

//End PaTMaN's jmod add

//AQ2:TNG - slicer
void VotingMenu (edict_t * ent, pmenu_t * p)
{
	PMenu_Close (ent);
	vShowMenu (ent, "");
	ent->client->curr_menu = MENU_VOTING;
}
//AQ2:TNG END

pmenu_t joinmenu[] = {
  {"*" TNG_TITLE, PMENU_ALIGN_CENTER, NULL, NULL},
  {"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F", PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL /* lvl name */ , PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL, PMENU_ALIGN_CENTER, NULL, NULL},
  {NULL /* team 1 */ , PMENU_ALIGN_LEFT, NULL, JoinTeam1},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL /* team 2 */ , PMENU_ALIGN_LEFT, NULL, JoinTeam2},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL /* team 3 */ , PMENU_ALIGN_LEFT, NULL, JoinTeam3},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL /* auto-join */ , PMENU_ALIGN_LEFT, NULL, JoinTeamAuto},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  //AQ2:TNG - Slicer
  {"Voting & Ignoring Menus", PMENU_ALIGN_LEFT, NULL, VotingMenu},
  //AQ2:TNG END
  {"MOTD", PMENU_ALIGN_LEFT, NULL, ReprintMOTD},
  {"Credits", PMENU_ALIGN_LEFT, NULL, CreditsMenu},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"Use arrows to move cursor", PMENU_ALIGN_LEFT, NULL, NULL},
  {"ENTER to select", PMENU_ALIGN_LEFT, NULL, NULL},
  {"TAB to exit menu", PMENU_ALIGN_LEFT, NULL, NULL},
  {NULL, PMENU_ALIGN_LEFT, NULL, NULL},
  {"v" VERSION, PMENU_ALIGN_RIGHT, NULL, NULL},
};
// AQ2:TNG End

void CreditsMenu (edict_t * ent, pmenu_t * p)
{
	PMenu_Close (ent);
	PMenu_Open (ent, creditsmenu, 4, sizeof (creditsmenu) / sizeof (pmenu_t));
	ent->client->curr_menu = MENU_CREDITS;
	unicastSound(ent, gi.soundindex("world/elv.wav"), 1.0);
}

void killPlayer( edict_t *ent, qboolean suicidePunish )
{
	if (!IS_ALIVE(ent))
		return;
	
	int damage = 100000;

	if (suicidePunish && punishkills->value)
	{
		edict_t *attacker = ent->client->attacker;
		if (attacker && attacker != ent && attacker->client)
		{
			char deathmsg[128];
			Q_snprintf( deathmsg, sizeof( deathmsg ), "%s ph34rs %s so much %s committed suicide! :)\n",
				ent->client->pers.netname, attacker->client->pers.netname,
				ent->client->pers.gender ? "she" : "he");
			
			PrintDeathMessage( deathmsg, ent );
			
			if (team_round_going || !OnSameTeam( ent, ent->client->attacker )) {
				Add_Frag( ent->client->attacker, MOD_SUICIDE );
				Subtract_Frag( ent );
				Add_Death( ent, true );
			}
		}
	}

	// Throws bloody gibs everywhere
	if (sv_killgib->value) {
		int n;
		for (n = 0; n < 10; n++)
			ThrowGib(ent, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead(ent, damage);
		ent->client->anim_priority = ANIM_DEATH;
		ent->client->anim_end = 0;
		//ent->takedamage = DAMAGE_NO;
	}
	
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;

	vec3_t non_const_origin; // Convert to non-const
	VectorCopy(vec3_origin, non_const_origin);
	player_die(ent, ent, ent, damage, non_const_origin);
	ent->deadflag = DEAD_DEAD;
}

char *TeamName (int team)
{
	if (team >= TEAM1 && team <= TEAM3)
		return teams[team].name;
	else
		return "None";
}

/*
AssignSkin

Anytime this is called, all player models are re-skinned
*/
void AssignSkin (edict_t * ent, const char *s, qboolean nickChanged)
{
	int playernum = ent - g_edicts - 1;
	char *p;
	char t[MAX_SKINLEN], skin[64] = "\0";
	const char *default_skin = "male/grunt";

	if( force_skin->string[0] )
	{
		s = force_skin->string;
		default_skin = force_skin->string;
	}

	// Accounts for each Espionage mode, in matchmode or not
	if (esp->value){
		/*
		In ATL mode (espsettings == 0), all teams must have a leader, and will have their own skin
		In ETV mode (espsettings == 1), only TEAM1 gets a leader.
		*/
	
		switch (ent->client->resp.team) {
		case TEAM1:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[TEAM1].skin);
			if (IS_LEADER(ent)){
				Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[TEAM1].leader_skin);
			}
			break;
		case TEAM2:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[TEAM2].skin);
			if ((atl->value) && IS_LEADER(ent)){
				Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[TEAM2].leader_skin);
			}
			break;
		case TEAM3:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[TEAM3].skin);
			if ((atl->value) && IS_LEADER(ent)){
				Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[TEAM3].leader_skin);
			}
			break;
		default:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, default_skin);
			break;
		}
	} else if ((ctf->value || dom->value) && !matchmode->value) {
		// forcing CTF model
		if(ctf_model->string[0]) {
			/* copy at most bytes that the skin name itself fits in with the delimieter and NULL */
			Q_strncpyz( t, ctf_model->string, MAX_SKINLEN-strlen(CTF_TEAM1_SKIN)-1 );
			Q_strncatz(t, "/", sizeof(t));
		} else {
			Q_strncpyz(t, s, sizeof(t));
		}

		if ((p = strrchr (t, '/')) != NULL)
			p[1] = 0;
		else
			strcpy (t, "male/");

		switch (ent->client->resp.team)
		{
		case TEAM1:
			Q_snprintf(skin, sizeof(skin), "%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM1_SKIN);
			break;
		case TEAM2:
			Q_snprintf(skin, sizeof(skin), "%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN);
			break;
		default:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, default_skin);
			break;
		}
	}
	else // Normal teamplay mode, not CTF/DOM/ESP/Matchmode
	{
		switch (ent->client->resp.team)
		{
		case TEAM1:
		case TEAM2:
		case TEAM3:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, teams[ent->client->resp.team].skin);
			break;
		default:
			Q_snprintf(skin, sizeof(skin), "%s\\%s", ent->client->pers.netname, (teamplay->value ? default_skin : s));
			break;
		}
	}

	gi.configstring(game.csr.playerskins + playernum, skin);
}

/*
==============
TP_GetTeamFromArgs
==============
*/
int TP_GetTeamFromArg(const char *name)
{
	int i;

	if (!name || !*name)
		return -1;

	if (!name[1])
	{
		i = Q_tolower(name[0]);
		if (i == '1' || i == 'a')
			return TEAM1;

		if (i == '2' || i == 'b')
			return TEAM2;

		if (teamCount > 2 && (i == '3' || i == 'c'))
			return TEAM3;

		if (i == '0' || i == 's')
			return NOTEAM;
	}

	for (i = TEAM1; i <= teamCount; i++) {
		if (!Q_stricmp(name, teams[i].name))
			return i;
	}

	if (!Q_stricmp(name, "none") || !Q_stricmp(name, "spec"))
		return NOTEAM;

	if (ctf->value)
	{
		if (!Q_stricmp(name, "red"))
			return TEAM1;
		if (!Q_stricmp(name, "blue"))
			return TEAM2;
	}

	return -1;
}

qboolean TeamSizeLimiter(edict_t *ent, int teamNum, int teamSize)
{
	// Check if the desired team size exceeds the maximum allowed size
	if (highlander->value) {
		if ((teamNum == TEAM1 && level.teamcount[TEAM1] >= teamSize) ||
			(teamNum == TEAM2 && level.teamcount[TEAM2] >= teamSize) ||
			(teamNum == TEAM3 && level.teamcount[TEAM3] >= teamSize)) {
			gi.cprintf(ent, PRINT_HIGH, "Highlander Mode: Cannot join team %s: Max players per team is %i\n", TeamName(teamNum), teamSize);
			return false;
		}
	}
	return true;
}

void Team_f (edict_t * ent)
{
	char *t;
	int desired_team = NOTEAM;
	char team[24];

	if (!teamplay->value)
		return;

	//PG BUND - BEGIN (Tourney extension)
	if (use_tourney->value) {
		gi.cprintf(ent, PRINT_MEDIUM, "Currently running tourney mod, team selection is disabled.");
		return;
	}
	//PG BUND - END (Tourney extension)

	if (esp->value && IS_LEADER(ent)) {
		gi.cprintf(ent, PRINT_MEDIUM, "You are a team leader, you cannot change teams.\n");
		return;
	}
	
	Q_strncpyz(team, gi.args(), sizeof(team));
	t = team;
	//  t = gi.args ();
	if (!*t)
	{
		if (ctf->value)
			gi.cprintf(ent, PRINT_HIGH, "You are on %s.\n", CTFTeamName(ent->client->resp.team));
		else
			gi.cprintf(ent, PRINT_HIGH, "You are on %s.\n", TeamName(ent->client->resp.team));

		return;
	}

	if( (ent->client->resp.joined_team > 0) && (level.realFramenum - ent->client->resp.joined_team < 5 * HZ) )
	{
		gi.cprintf(ent, PRINT_HIGH, "You must wait 5 seconds before changing teams again.\n");
		return;
	}

	desired_team = TP_GetTeamFromArg(t);
	if (desired_team == -1) {
		gi.cprintf(ent, PRINT_HIGH, "Unknown team '%s'.\n", t);
		return;
	}

	if (desired_team == NOTEAM)
	{
		if (ent->client->resp.team == NOTEAM)
			gi.cprintf(ent, PRINT_HIGH, "You're not on a team.\n");
		else
			LeaveTeam(ent);

		return;
	}

	if (ent->client->resp.team == desired_team) {
		gi.cprintf(ent, PRINT_HIGH, "You are already on %s.\n", TeamName(ent->client->resp.team));
		return;
	}

	JoinTeam(ent, desired_team, 1);
}

void JoinTeam (edict_t * ent, int desired_team, int skip_menuclose)
{
	char *s, *a;
	int oldTeam;

	if (!skip_menuclose)
		PMenu_Close (ent);

	oldTeam = ent->client->resp.team;
	if (oldTeam == desired_team || ent->client->pers.mvdspec)
		return;

#ifndef NO_BOTS
	// Bots are always allowed to join their selected team.
	if( ent->is_bot )
		;
	else
#endif
	if (matchmode->value)
	{
		if (mm_allowlock->value && teams[desired_team].locked) {
			if (skip_menuclose)
				gi.cprintf(ent, PRINT_HIGH, "Cannot join %s (locked)\n", TeamName(desired_team));
			else
				gi.centerprintf(ent, "Cannot join %s (locked)", TeamName(desired_team));

			return;
		}
	}
	else
	{
		if(eventeams->value && desired_team != NOTEAM) {
			if(!IsAllowedToJoin(ent, desired_team)) {
				gi.centerprintf(ent, "Cannot join %s (has too many players)", TeamName(desired_team));
				return;
			}
		}
	}

	if (highlander->value){
		int i;
		for (i = TEAM1; i < TEAM_TOP; i++) {
			if (!TeamSizeLimiter(ent, i, HIGHLANDER_MAX_PLAYERS)) {
				return;
			}
		}
	}

	MM_LeftTeam( ent );

	a = (oldTeam == NOTEAM) ? "joined" : "changed to";

	// Team count update
	level.teamcount[ent->client->resp.team]--;
	level.teamcount[desired_team]++;

	ent->client->resp.team = desired_team;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	AssignSkin(ent, s, false);
	
	ent->flags &= ~FL_GODMODE;
	killPlayer(ent, true);

	if (ctf->value)
	{
		ent->client->resp.ctf_state = CTF_STATE_START;
		gi.bprintf (PRINT_HIGH, "%s %s %s.\n", ent->client->pers.netname, a, CTFTeamName(desired_team));
		IRC_printf (IRC_T_GAME, "%n %s %n.", ent->client->pers.netname, a, CTFTeamName(desired_team));
	}
	else
	{
		gi.bprintf (PRINT_HIGH, "%s %s %s.\n", ent->client->pers.netname, a, TeamName(desired_team));
		IRC_printf (IRC_T_GAME, "%n %s %n.", ent->client->pers.netname, a, TeamName(desired_team));
	}

	ent->client->resp.joined_team = level.realFramenum;

	if (oldTeam == NOTEAM || desired_team == NOTEAM) {
		G_UpdatePlayerStatusbar(ent, 1);
	}

#if AQTION_EXTENSION
	if (desired_team == NOTEAM)
		HUD_SetType(ent, 1);
	else
		HUD_SetType(ent, -1);
#endif

	if (level.intermission_framenum)
		return;

	// Espionage join a game in progress
	if (esp->value && team_round_going && ent->inuse && ent->client->resp.team)
	{
		PutClientInServer (ent);
		//AddToTransparentList (ent);
	}

	if (!(gameSettings & GS_ROUNDBASED) && team_round_going && ent->inuse && ent->client->resp.team)
	{
		PutClientInServer (ent);
		AddToTransparentList (ent);
	}

	#if USE_AQTION
	if (in_warmup && warmup_bots->value) {
		PutClientInServer (ent);
	}
	#endif

	//AQ2:TNG END
	if (!skip_menuclose && (gameSettings & GS_WEAPONCHOOSE) && !use_randoms->value)
		OpenWeaponMenu(ent);

	if (use_randoms->value)
	{
		SelectRandomWeaponAndItem(ent, weapmenu);
	}

	gi.cprintf(ent, PRINT_CHAT, "You joined Team %i (%s).\n", desired_team, TeamName(desired_team));

	teams_changed = true;
}

void LeaveTeam (edict_t * ent)
{
	char *genderstr;
	
	if (ent->client->resp.team == NOTEAM)
		return;

	killPlayer(ent, true);
	
	genderstr = GENDER_STR(ent, "his", "her", "its");

	gi.bprintf (PRINT_HIGH, "%s left %s team.\n", ent->client->pers.netname, genderstr);
	IRC_printf (IRC_T_GAME, "%n left %n team.", ent->client->pers.netname, genderstr);

	MM_LeftTeam( ent );

	level.teamcount[ent->client->resp.team]--;

	if (esp->value)
		EspLeaderLeftTeam ( ent );

	ent->client->resp.joined_team = 0;
	ent->client->resp.team = NOTEAM;
	G_UpdatePlayerStatusbar(ent, 1);

#if AQTION_EXTENSION
	HUD_SetType(ent, 1);
#endif

	teams_changed = true;
}

void ReturnToMain (edict_t * ent, pmenu_t * p)
{
	PMenu_Close (ent);
	OpenJoinMenu (ent);
}

char *menu_itemnames[KIT_MAX_NUM] = {
	"",
	MK23_NAME,
	MP5_NAME,
	M4_NAME,
	M3_NAME,
	HC_NAME,
	"SSG 3000 Sniper Rifle",
	"Akimbo Pistols",
	"Combat Knives",
	GRENADE_NAME,
	SIL_NAME,
	SLIP_NAME,
	BAND_NAME,
	KEV_NAME,
	"Laser Sight",
	HELM_NAME,
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	C_KIT_NAME_FULL,
	S_KIT_NAME_FULL,
	A_KIT_NAME_FULL,
};


typedef struct menuentry_s
{
	int		itemNum;
	void (*SelectFunc) (edict_t * ent, struct pmenu_s * entry);
} menuentry_t;

void OpenItemMenu (edict_t * ent)
{
	menuentry_t *menuEntry, menu_items[] = {
		{ KEV_NUM, SelectItem1 },
		{ LASER_NUM, SelectItem2 },
		{ SLIP_NUM, SelectItem3 },
		{ SIL_NUM, SelectItem4 },
		{ BAND_NUM, SelectItem5 },
		{ HELM_NUM, SelectItem6 }
		};
	int i, count, pos = 4;

	count = sizeof( menu_items ) / sizeof( menu_items[0] );

	if ((int)itm_flags->value & ITF_MASK)
	{
		for (menuEntry = menu_items, i = 0; i < count; i++, menuEntry++) {
			if (!ITF_ALLOWED(menuEntry->itemNum))
				continue;

			itemmenu[pos].text = menu_itemnames[menuEntry->itemNum];
			itemmenu[pos].SelectFunc = menuEntry->SelectFunc;
			pos++;
		}

		if ( pos > 4 )
		{
			for (; pos < 10; pos++)
			{
				itemmenu[pos].text = NULL;
				itemmenu[pos].SelectFunc = NULL;
			}

			PMenu_Open(ent, itemmenu, 4, sizeof(itemmenu) / sizeof(pmenu_t));
			ent->client->curr_menu = MENU_ITEMS;
			return;
		}
	}

	PMenu_Close(ent);
}

void OpenItemKitMenu (edict_t * ent)
{
	menuentry_t *kitmenuEntry, kit_menu_items[] = {
		{ KEV_NUM, SelectItem1 },
		{ C_KIT_NUM, SelectKit1 },
		{ S_KIT_NUM, SelectKit2 },
		{ A_KIT_NUM, SelectKit3 }
		};
	int i, count, pos = 4;

	count = sizeof( kit_menu_items ) / sizeof( kit_menu_items[0] );

	for (kitmenuEntry = kit_menu_items, i = 0; i < count; i++, kitmenuEntry++) {
		itemkitmenu[pos].text = menu_itemnames[kitmenuEntry->itemNum];
		itemkitmenu[pos].SelectFunc = kitmenuEntry->SelectFunc;
		pos++;
	}

	if ( pos > 4 )
	{
		for (; pos < 10; pos++)
		{
			itemkitmenu[pos].text = NULL;
			itemkitmenu[pos].SelectFunc = NULL;
		}

		PMenu_Open(ent, itemkitmenu, 4, sizeof(itemkitmenu) / sizeof(pmenu_t));
		ent->client->curr_menu = MENU_ITEMKITS;
		return;
	}

	PMenu_Close(ent);
}

void OpenWeaponMenu (edict_t * ent)
{
	if (use_randoms->value)
	{
		PMenu_Open(ent, randmenu, 4, sizeof(randmenu) / sizeof(pmenu_t));
		ent->client->curr_menu = MENU_RANDOM;
		return;
	}

	menuentry_t *menuEntry, menu_items[] = {
		{ MP5_NUM, SelectWeapon2 },
		{ M3_NUM, SelectWeapon3 },
		{ HC_NUM, SelectWeapon4 },
		{ SNIPER_NUM, SelectWeapon5 },
		{ M4_NUM, SelectWeapon6 },
		{ KNIFE_NUM, SelectWeapon0 },
		{ DUAL_NUM, SelectWeapon9 }
	};
	int i, count, pos = 4;

	count = sizeof( menu_items ) / sizeof( menu_items[0] );

	if ((int)wp_flags->value & WPF_MASK)
	{
		for (menuEntry = menu_items, i = 0; i < count; i++, menuEntry++) {
			if (!WPF_ALLOWED(menuEntry->itemNum))
				continue;

			if (weapon_status[menu_items[i].itemNum][ent->client->resp.team].owner == NULL ||
			weapon_status[menu_items[i].itemNum][weapon_status[menu_items[i].itemNum][ent->client->resp.team].owner->client->resp.team].owner->client->resp.team != ent->client->resp.team) {
				weapmenu[pos].text = menu_itemnames[menu_items[i].itemNum];
				weapmenu[pos].SelectFunc = menu_items[i].SelectFunc;
				pos++;
			}
		}

		if (pos > 4)
		{
			for (; pos < 11; pos++)
			{
				weapmenu[pos].text = NULL;
				weapmenu[pos].SelectFunc = NULL;
			}

			PMenu_Open(ent, weapmenu, 4, sizeof(weapmenu) / sizeof(pmenu_t));
			ent->client->curr_menu = MENU_WEAPONS;
			return;
		}
	}

	if(!item_kit_mode->value){
		OpenItemMenu(ent);
	} else {
		OpenItemKitMenu(ent);
	}
}

// AQ2:TNG Deathwatch - Updated this for the new menu
void UpdateJoinMenu( void )
{
	static char levelname[28];
	static char team1players[28];
	static char team2players[28];
	static char team3players[28];
	int num1 = 0, num2 = 0, num3 = 0, i;

	if (ctf->value)
	{
		joinmenu[4].text = "Join Red Team";
		joinmenu[4].SelectFunc = JoinTeam1;
		joinmenu[6].text = "Join Blue Team";
		joinmenu[6].SelectFunc = JoinTeam2;
		joinmenu[8].text = NULL;
		joinmenu[8].SelectFunc = NULL;
		if (ctf_forcejoin->string && *ctf_forcejoin->string)
		{
			if (Q_stricmp (ctf_forcejoin->string, "red") == 0)
			{
				joinmenu[6].text = NULL;
				joinmenu[6].SelectFunc = NULL;
			}
			else if (Q_stricmp (ctf_forcejoin->string, "blue") == 0)
			{
				joinmenu[4].text = NULL;
				joinmenu[4].SelectFunc = NULL;
			}
		}
	}
	else
	{
		joinmenu[4].text = teams[TEAM1].name;
		joinmenu[4].SelectFunc = JoinTeam1;
		joinmenu[6].text = teams[TEAM2].name;
		joinmenu[6].SelectFunc = JoinTeam2;
		if (teamCount == 3)
		{
			joinmenu[8].text = teams[TEAM3].name;
			joinmenu[8].SelectFunc = JoinTeam3;
		}
		else
		{
			joinmenu[8].text = NULL;
			joinmenu[8].SelectFunc = NULL;
		}
	}
	joinmenu[11].text = "Auto-join team";
	joinmenu[11].SelectFunc = JoinTeamAuto;

	levelname[0] = '*';
	if (g_edicts[0].message)
		Q_strncpyz(levelname + 1, g_edicts[0].message, sizeof(levelname) - 1);
	else
		Q_strncpyz(levelname + 1, level.mapname, sizeof(levelname) - 1);

	for (i = 0; i < game.maxclients; i++)
	{
		if (!g_edicts[i + 1].inuse)
			continue;
		if (game.clients[i].resp.team == TEAM1)
			num1++;
		else if (game.clients[i].resp.team == TEAM2)
			num2++;
		else if (game.clients[i].resp.team == TEAM3)
			num3++;
	}

	sprintf (team1players, "  (%d players)", num1);
	sprintf (team2players, "  (%d players)", num2);
	sprintf (team3players, "  (%d players)", num3);

	joinmenu[2].text = levelname;
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	if (joinmenu[8].text && (teamCount == 3))
		joinmenu[9].text = team3players;
	else
		joinmenu[9].text = NULL;
}

// AQ2:TNG END

void OpenJoinMenu (edict_t * ent)
{
	//PG BUND - BEGIN (Tourney extension)
	if (use_tourney->value)
	{
		OpenWeaponMenu (ent);
		return;
	}
	//PG BUND - END (Tourney extension)

	UpdateJoinMenu();

	PMenu_Open (ent, joinmenu, 11 /* magic for Auto-join menu item */, sizeof (joinmenu) / sizeof (pmenu_t));
	ent->client->curr_menu = MENU_TEAM;
}

void gib_die( edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point );  // g_misc

void CleanLevel (void)
{
	int i, base;
	edict_t *ent;
	base = 1 + game.maxclients + BODY_QUEUE_SIZE;
	ent = g_edicts + base;
	
	for (i = base; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->classname)
			continue;
		switch (ent->typeNum) {
			case MK23_NUM:
			case MP5_NUM:
			case M4_NUM:
			case M3_NUM:
			case HC_NUM:
			case SNIPER_NUM:
			case DUAL_NUM:
			case KNIFE_NUM:
			case GRENADE_NUM:
			case SIL_NUM:
			case SLIP_NUM:
			case BAND_NUM:
			case KEV_NUM:
			case LASER_NUM:
			case HELM_NUM:
			case MK23_ANUM:
			case MP5_ANUM:
			case M4_ANUM:
			case SHELL_ANUM:
			case SNIPER_ANUM:
				G_FreeEdict( ent );
				break;
			default:
				if((ent->die == gib_die)
				|| (strcmp( ent->classname, "medkit" ) == 0)
				|| (strcmp( ent->classname, "decal" ) == 0)
				|| (strcmp( ent->classname, "splat" ) == 0)
				|| (strcmp( ent->classname, "shell" ) == 0))
					G_FreeEdict( ent );
		}
	}
	
	CleanBodies();
	// fix glass
	CGF_SFX_RebuildAllBrokenGlass ();
}

void MakeAllLivePlayersObservers(void);

void ResetScores (qboolean playerScores)
{
	int i;
	edict_t *ent;

	team_round_going = team_round_countdown = team_game_going = 0;
	current_round_length = 0;
	lights_camera_action = holding_on_tie_check = 0;

	timewarning = fragwarning = 0;
	level.pauseFrames = 0;
	level.matchTime = 0;
	num_ghost_players = 0;

	MakeAllLivePlayersObservers();

	for(i = TEAM1; i < TEAM_TOP; i++)
	{
		teams[i].score = teams[i].total = 0;
		teams[i].ready = teams[i].locked = 0;
		teams[i].pauses_used = teams[i].wantReset = 0;
		gi.cvar_forceset(teams[i].teamscore->name, "0");
	}

	ctfgame.team1 = 0;
	ctfgame.team2 = 0;
	ctfgame.total1 = 0;
	ctfgame.total2 = 0;
	ctfgame.last_flag_capture = 0;
	ctfgame.last_capture_team = 0;
	ctfgame.halftime = 0;

	if(!playerScores)
		return;

	for (i = 0; i < game.maxclients; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		ent->client->resp.score = 0;
		ent->client->resp.kills = 0;
		ent->client->resp.damage_dealt = 0;
		ent->client->resp.streakHS = 0;
		ent->client->resp.streakKills = 0;
		ent->client->resp.roundStreakKills = 0;
		ent->client->resp.ctf_caps = 0;
		ent->client->resp.ctf_capstreak = 0;
		ent->client->resp.esp_capstreak = 0;
		ent->client->resp.deaths = 0;
		ent->client->resp.team_kills = 0;
		ent->client->resp.team_wounds = 0;
		ent->enemy = NULL;
		ResetStats(ent);
	}
}

void CenterPrintAll (const char *msg)
{
	int i;
	edict_t *ent;

	gi.cprintf (NULL, PRINT_HIGH, "%s\n", msg);	// so it goes to the server console...

	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (ent->is_bot)
			continue;
		if (ent->inuse)
			gi.centerprintf (ent, "%s", msg);
	}
}

void CenterPrintTeam (int teamNum, const char *msg)
{
	int i;
	edict_t *ent;
	
	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (ent->is_bot)
			continue;
		if (ent->inuse && ent->client->resp.team == teamNum)
			gi.centerprintf (ent, "%s", msg);
	}
}

void CenterPrintLevelTeam (int teamNum, int printlvl, const char *msg)
{
	int i;
	edict_t *ent;
	
	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (ent->is_bot)
			continue;
		if (ent->inuse && ent->client->resp.team == teamNum)
			gi.cprintf(ent, printlvl, "%s", msg);
	}
}

int TeamHasPlayers (int team)
{
	int i, players;
	edict_t *ent;

	players = 0;

	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (!ent->inuse)
			continue;

		if (game.clients[i].resp.team == team)
			players++;
	}

	return players;
}

int _numclients( void );  // a_vote.c

qboolean BothTeamsHavePlayers(void)
{
	int players[TEAM_TOP] = { 0 }, i, teamsWithPlayers;
	edict_t *ent;

	//AQ2:TNG Slicer Matchmode
	if (matchmode->value && !TeamsReady())
		return false;
	//AQ2:TNG END

	if (use_tourney->value)
		return (LastOpponent > 1);

	if( ! _numclients() )
		return false;

	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (!ent->inuse || game.clients[i].resp.team == NOTEAM)
			continue;
		if (!game.clients[i].resp.subteam)
			players[game.clients[i].resp.team]++;
	}

	teamsWithPlayers = 0;
	for (i = TEAM1; i <= teamCount; i++)
	{
		if (players[i]) {
			teamsWithPlayers++;
		}
	}

	return (teamsWithPlayers >= 2);
}

// CheckForWinner: Checks for a winner (or not).
int CheckForWinner(void)
{
	int players[TEAM_TOP] = { 0 }, i = 0, teamNum = 0, teamsWithPlayers = 0;
	edict_t *ent;

	for (i = 0; i < game.maxclients; i++){
		ent = &g_edicts[1 + i];
		if (!ent->inuse || ent->solid == SOLID_NOT)
			continue;

		teamNum = game.clients[i].resp.team;
		if (teamNum == NOTEAM)
			continue;

		players[teamNum]++;
	}
	teamsWithPlayers = 0;
	for (i = TEAM1; i <= teamCount; i++){
		if (players[i]) {
			teamsWithPlayers++;
			teamNum = i;
		}
	}

	if (!(gameSettings & GS_ROUNDBASED))
		return WINNER_NONE;

	if (esp->value){
		if (atl->value) {
			if (teamCount == TEAM2) {
				if (teams[TEAM1].leader_dead && teams[TEAM2].leader_dead) {
					return WINNER_TIE;
				} else if (teams[TEAM1].leader_dead) {
					return TEAM2;
				} else if (teams[TEAM2].leader_dead) {
					return TEAM1;
				}
			} else if (teamCount == TEAM3) {
				if (teams[TEAM1].leader_dead && teams[TEAM2].leader_dead && teams[TEAM3].leader_dead) {
					return WINNER_TIE;
				} else if (teams[TEAM1].leader_dead && teams[TEAM2].leader_dead) {
					return TEAM3;
				} else if (teams[TEAM1].leader_dead && teams[TEAM3].leader_dead) {
					return TEAM2;
				} else if (teams[TEAM2].leader_dead && teams[TEAM3].leader_dead) {
					return TEAM1;
				} 
			}
		} else if (etv->value) {
			// Check if this value is true, which means the escorting team wins
			// By default it is false
			if (espsettings.escortcap == true) {
				return TEAM1;
			} else if (teams[TEAM1].leader_dead){
				return TEAM2;
			} else if (teamsWithPlayers) // This is in case the last person on team 2 leaves without dying, TEAM1 wins
				return (teamsWithPlayers > 1) ? WINNER_NONE : teamNum;
		}
	
	} else if (!esp->value) {
		if (teamsWithPlayers)
			return (teamsWithPlayers > 1) ? WINNER_NONE : teamNum;

		//gi.dprintf("I was called somehow\n\n\n");
		return WINNER_TIE;
	}
	return WINNER_NONE;
}

// CheckForForcedWinner: A winner is being forced, find who it is.
int CheckForForcedWinner(void)
{
	int players[TEAM_TOP] = { 0 };
	int health[TEAM_TOP] = { 0 };
	int i, teamNum, bestTeam, secondBest;
	edict_t *ent;

	/*
	In ATL mode, if all leaders are alive, the round ends in a tie
	health notwithstanding
	In ETV mode, if the escorting team has
	not captured the point, the defending team wins
	*/
	if (esp->value){
		if (atl->value){
			if (teamCount == TEAM2){
				if (IS_ALIVE(teams[TEAM1].leader) && 
				IS_ALIVE(teams[TEAM2].leader)){
					return WINNER_TIE;
				}
			} else if (teamCount == TEAM3){
				if (IS_ALIVE(teams[TEAM1].leader) && 
				IS_ALIVE(teams[TEAM2].leader) && 
				IS_ALIVE(teams[TEAM3].leader)){
					return WINNER_TIE;
				}
			}
		} else if (etv->value){
			if (espsettings.escortcap == false){
				return TEAM2;
			}
		}
	}

	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (!ent->inuse || ent->solid == SOLID_NOT)
			continue;
		teamNum = game.clients[i].resp.team;
		if (teamNum == NOTEAM)
			continue;

		players[teamNum]++;
		health[teamNum] += ent->health;
	}

	bestTeam = secondBest = NOTEAM;
	for (i = TEAM1; i <= teamCount; i++)
	{
		if (players[i] < players[bestTeam]) {
			continue;
		}
		if (players[i] > players[bestTeam]) {
			bestTeam = i;
			secondBest = NOTEAM;
			continue;
		}
		//Same amound of players, check health
		if (health[i] < health[bestTeam]) {
			continue;
		}
		if (health[i] > health[bestTeam]) {
			bestTeam = i;
			secondBest = NOTEAM;
			continue;
		}
		//Same as bestTeam
		secondBest = i;
	}

	if (bestTeam == NOTEAM || secondBest != NOTEAM)
		return WINNER_TIE;

	return bestTeam;
}

static void SpawnPlayers(void)
{
	int i;
	edict_t *ent;

	if (esp->value)
		NS_SetupTeamSpawnPoints ();

	if (gameSettings & GS_ROUNDBASED)
	{
		if (!use_oldspawns->value)
			NS_SetupTeamSpawnPoints ();
		else
			SetupTeamSpawnPoints ();
	}

	InitTransparentList();
	for (i = 0, ent = &g_edicts[1]; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		if (!ent->client->resp.team || ent->client->resp.subteam)
			continue;

		// make sure teamplay spawners always have some weapon, warmup starts only after weapon selected
		if (!ent->client->pers.chosenWeapon) {
			if (!highlander->value){
				ent->client->pers.chosenWeapon = GET_ITEM(MP5_NUM);  // Highlander mode removes default MP5
			} else if (WPF_ALLOWED(MP5_NUM) && !highlander->value) {
				ent->client->pers.chosenWeapon = GET_ITEM(MP5_NUM);
			}

			if (WPF_ALLOWED(MK23_NUM)) {
				ent->client->pers.chosenWeapon = GET_ITEM(MK23_NUM);
			} else if (WPF_ALLOWED(KNIFE_NUM)) {
				ent->client->pers.chosenWeapon = GET_ITEM(KNIFE_NUM);
			} else {
				ent->client->pers.chosenWeapon = GET_ITEM(MK23_NUM);
			}
		}

		if (!ent->client->pers.chosenItem) {
			ent->client->pers.chosenItem = GET_ITEM(KEV_NUM);
		}

		// Random weapons and items mode.
		// Force random weapon and item on spawn.
		if (use_randoms->value)
		{
			SelectRandomWeaponAndItem(ent, weapmenu);
		}

#ifndef NO_BOTS
		if( !Q_stricmp( ent->classname, "bot" ) )
			ACESP_PutClientInServer( ent, true, ent->client->resp.team );
		else
#endif
		PutClientInServer(ent);
		AddToTransparentList(ent);
	}

	if(matchmode->value	&& limchasecam->value)
	{
		for (i = 0, ent = &g_edicts[1]; i < game.maxclients; i++, ent++)
		{
			if (!ent->inuse)
				continue;

			if (!ent->client->resp.team || !ent->client->resp.subteam)
				continue;

			ent->client->chase_mode = 0;
			NextChaseMode( ent );
		}
	}
}

void RunWarmup (void)
{
	int i, dead;
	edict_t *ent;

	if (!warmup->value || team_round_going || lights_camera_action || (team_round_countdown > 0 && team_round_countdown <= 101))
		return;

	// Allows re-warmup if teams unready during matchmode
	if (!warmup_unready->value && (matchmode->value && level.matchTime > 0))
		return;

	if (!in_warmup)
	{
		in_warmup = 1;
		InitTransparentList();
	}

	for (i = 0, ent = &g_edicts[1]; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;
		
		if(!ent->client->resp.team || ent->client->resp.subteam)
			continue;
		
		if (!ent->client->pers.chosenWeapon || !ent->client->pers.chosenItem)
			continue;

		dead = (ent->solid == SOLID_NOT && ent->deadflag == DEAD_NO && ent->movetype == MOVETYPE_NOCLIP);
		if (dead && ent->client->latched_buttons & BUTTON_ATTACK)
		{
			ent->client->latched_buttons = 0;
			PutClientInServer(ent);
			AddToTransparentList(ent);
			if (matchmode->value && level.matchTime > 0)
				gi.centerprintf(ent, "TEAMS NOT READY: RETURNING TO WARMUP");
			else
				gi.centerprintf(ent, "WARMUP");
		}
	}
	#if USE_AQTION
	if (warmup_bots->value){
		gi.cvar_forceset("am", "1");
		gi.cvar_forceset("am_botcount", warmup_bots->string);
		attract_mode_bot_check();
	}
	#endif
}

void StartRound (void)
{
	team_round_going = 1;
	current_round_length = 0;
}

static void StartLCA(void)
{
	if ((gameSettings & (GS_WEAPONCHOOSE|GS_ROUNDBASED)))
		CleanLevel();

	if (esp->value)
		// Re-skin everyone to ensure only one leader skin
		EspSkinCheck();

	if (use_tourney->value && !tourney_lca->value)
	{
		lights_camera_action = TourneySetTime (T_SPAWN);
		TourneyTimeEvent(T_SPAWN, lights_camera_action);
	}
	else
	{
		CenterPrintAll ("LIGHTS...");
		gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, level.snd_lights, 1.0, ATTN_NONE, 0.0);
		lights_camera_action = 43;	// TempFile changed from 41
	}
	SpawnPlayers();

	if (esp->value) {
		esp_punishment_phase = false;
		EspResetCapturePoint();
		EspAnnounceDetails(false);
	}
}

// FindOverlap: Find the first (or next) overlapping player for ent.
edict_t *FindOverlap (edict_t * ent, edict_t * last_overlap)
{
	int i;
	edict_t *other;
	vec3_t diff;

	for (i = last_overlap ? last_overlap - g_edicts : 0; i < game.maxclients; i++)
	{
		other = &g_edicts[i + 1];

		if (!other->inuse || other->client->resp.team == NOTEAM
			|| other == ent || !IS_ALIVE(other))
			continue;

		VectorSubtract(ent->s.origin, other->s.origin, diff);

		if (diff[0] >= -33 && diff[0] <= 33 &&
			diff[1] >= -33 && diff[1] <= 33 && diff[2] >= -65 && diff[2] <= 65)
			return other;
	}

	return NULL;
}

void ContinueLCA (void)
{
	if (use_tourney->value && !tourney_lca->value)
	{
		TourneyTimeEvent (T_SPAWN, lights_camera_action);
		if (lights_camera_action == 1)
		{
			StartRound();
		}
	}
	else
	{
		if (lights_camera_action == 23)
		{
			CenterPrintAll("CAMERA...");
			gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, level.snd_camera , 1.0, ATTN_NONE, 0.0);
		}
		else if (lights_camera_action == 3)
		{
			CenterPrintAll("ACTION!");
			gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, level.snd_action, 1.0, ATTN_NONE, 0.0);
		}
		else if (lights_camera_action == 1)
		{
			StartRound();
		}
	}
	lights_camera_action--;
}

void MakeAllLivePlayersObservers (void)
{
	edict_t *ent;
	int saveteam, i;

	/* if someone is carrying a flag it will disappear */
	if(ctf->value)
		CTFResetFlags();

	// Reset Espionage flag
	if (esp->value && etv->value)
		EspResetCapturePoint();

	for (i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (!ent->inuse)
			continue;
		if(ent->solid == SOLID_NOT && !ent->deadflag)
			continue;

		saveteam = ent->client->resp.team;
		ent->client->resp.team = NOTEAM;
		PutClientInServer(ent);
		ent->client->resp.team = saveteam;
	}
}

// PrintScores: Prints the current score on the console
void PrintScores (void)
{
	if (teamCount == 3) {
		gi.bprintf (PRINT_HIGH, "Current score is %s: %d to %s: %d to %s: %d\n", TeamName (TEAM1), teams[TEAM1].score, TeamName (TEAM2), teams[TEAM2].score, TeamName (TEAM3), teams[TEAM3].score);
		IRC_printf (IRC_T_TOPIC, "Current score on map %n is %n: %k to %n: %k to %n: %k", level.mapname, TeamName (TEAM1), teams[TEAM1].score, TeamName (TEAM2), teams[TEAM2].score, TeamName (TEAM3), teams[TEAM3].score);
	} else {
		gi.bprintf (PRINT_HIGH, "Current score is %s: %d to %s: %d\n", TeamName (TEAM1), teams[TEAM1].score, TeamName (TEAM2), teams[TEAM2].score);
		IRC_printf (IRC_T_TOPIC, "Current score on map %n is %n: %k to %n: %k", level.mapname, TeamName (TEAM1), teams[TEAM1].score, TeamName (TEAM2), teams[TEAM2].score);
	}
}

qboolean CheckTimelimit( void )
{
	if (timelimit->value > 0)
	{
		if (level.matchTime >= timelimit->value * 60)
		{
			int i;

			for (i = TEAM1; i < TEAM_TOP; i++) {
				teams[i].ready = 0;
			}
			
			timewarning = fragwarning = 0;
			
			if (matchmode->value) {
				SendScores();
				team_round_going = team_round_countdown = team_game_going = 0;
				MakeAllLivePlayersObservers();
				ctfgame.halftime = 0;
			} else {
				gi.bprintf( PRINT_HIGH, "Timelimit hit.\n" );
				IRC_printf( IRC_T_GAME, "Timelimit hit." );
				if (!(gameSettings & GS_ROUNDBASED))
					ResetPlayers();
				EndDMLevel();
			}
			
			team_round_going = team_round_countdown = team_game_going = 0;
			level.matchTime = 0;
			
			return true;
		}
		
		// CTF or Espionage with use_warnings should have the same warnings when the map is ending as it does for halftime (see CTFCheckRules).
		// Otherwise, use_warnings should warn about 3 minutes and 1 minute left, but only if there aren't round ending warnings.
		if( use_warnings->value && (ctf->value || ! roundtimelimit->value) )
		{
			if( timewarning < 3 && (ctf->value && level.matchTime >= timelimit->value * 60 - 10 ))
			{
				gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex("world/10_0.wav"), 1.0, ATTN_NONE, 0.0 );
				timewarning = 3;
			}
			else if( timewarning < 2 && level.matchTime >= (timelimit->value - 1) * 60 )
			{
				CenterPrintAll( "1 MINUTE LEFT..." );
				gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex("tng/1_minute.wav"), 1.0, ATTN_NONE, 0.0 );
				timewarning = 2;
				if (esp->value){
					if (esp_debug->value)
						gi.dprintf("%s: level.matchTime = %f\n", __FUNCTION__, level.matchTime);
					EspAnnounceDetails(true);
				}
			}
			else if( timewarning < 1 && (! ctf->value) && timelimit->value > 3 && level.matchTime >= (timelimit->value - 3) * 60 )
			{
				CenterPrintAll( "3 MINUTES LEFT..." );
				gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex("tng/3_minutes.wav"), 1.0, ATTN_NONE, 0.0 );
				timewarning = 1;
			}
		}
	}
	
	return false;
}

int WonGame(int winner);

static qboolean CheckRoundTimeLimit( void )
{
	if (roundtimelimit->value > 0)
	{
		int roundLimitFrames = (int)(roundtimelimit->value * 600);
		
		if (current_round_length >= roundLimitFrames)
		{
			int winTeam = NOTEAM;

			gi.bprintf( PRINT_HIGH, "Round timelimit hit.\n" );
			IRC_printf( IRC_T_GAME, "Round timelimit hit." );

			winTeam = CheckForForcedWinner();
			if (WonGame( winTeam ))
				return true;

			team_round_going = 0;
			timewarning = fragwarning = 0;
			lights_camera_action = 0;
			holding_on_tie_check = 0;
			team_round_countdown = 71;
			
			return true;
		}

		if (use_warnings->value && timewarning < 2)
		{
			roundLimitFrames -= current_round_length;
			
			if (roundLimitFrames <= 600)
			{
				CenterPrintAll( "1 MINUTE LEFT..." );
				gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex( "tng/1_minute.wav" ), 1.0, ATTN_NONE, 0.0 );
				timewarning = 2;
				if (esp->value) {
					if (esp_debug->value)
						gi.dprintf("%s: roundLimitFrames = %d\n", __FUNCTION__, roundLimitFrames);
					EspAnnounceDetails(true);
				}
			}
			else if (roundLimitFrames <= 1800 && timewarning < 1 && roundtimelimit->value > 3)
			{
				CenterPrintAll( "3 MINUTES LEFT..." );
				gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex( "tng/3_minutes.wav" ), 1.0, ATTN_NONE, 0.0 );
				timewarning = 1;
			}
		}
	}
	return false;
}

static qboolean CheckRoundLimit( void )
{
	if (roundlimit->value > 0)
	{
		int i, winTeam = NOTEAM;

		for (i = TEAM1; i <= teamCount; i++) {
			if (teams[i].score >= (int)roundlimit->value) {
				winTeam = i;
				break;
			}
		}

		if (winTeam != NOTEAM)
		{
			for (i = TEAM1; i < TEAM_TOP; i++) {
				teams[i].ready = 0;
			}

			timewarning = fragwarning = 0;
			if (matchmode->value) {
				SendScores();
				team_round_going = team_round_countdown = team_game_going = 0;
				MakeAllLivePlayersObservers();
			} else {
				gi.bprintf( PRINT_HIGH, "Roundlimit hit.\n" );
				IRC_printf( IRC_T_GAME, "Roundlimit hit." );
				EndDMLevel();
			}
			team_round_going = team_round_countdown = team_game_going = 0;
			level.matchTime = 0;
			return true;
		}
	}
	return false;
}

// WonGame: returns true if we're exiting the level.
int WonGame (int winner)
{
	edict_t *player, *cl_ent; // was: edict_t *player;
	int i;
	char arg[64];

	gi.bprintf (PRINT_HIGH, "The round is over:\n");
	IRC_printf (IRC_T_GAME, "The round is over:");
	if (winner == WINNER_TIE)
	{
		gi.bprintf (PRINT_HIGH, "It was a tie, no points awarded!\n");
		IRC_printf (IRC_T_GAME, "It was a tie, no points awarded!");

		if(use_warnings->value)
			gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, level.snd_teamwins[0], 1.0, ATTN_NONE, 0.0);
		if (esp->value)
			EspEndOfRoundCleanup();
		PrintScores ();
	}
	else
	{
		if (use_tourney->value)
		{
			if(winner == TEAM1)
				player = TourneyFindPlayer(1);
			else
				player = TourneyFindPlayer(NextOpponent);

			if (player)
			{
				gi.bprintf (PRINT_HIGH, "%s was victorious!\n",
				player->client->pers.netname);
				IRC_printf (IRC_T_GAME, "%n was victorious!",
				player->client->pers.netname);
				TourneyWinner (player);
			}
		}
		else
		{
			gi.bprintf (PRINT_HIGH, "%s won!\n", TeamName(winner));
			IRC_printf (IRC_T_GAME, "%n won!", TeamName(winner));
			// AQ:TNG Igor[Rock] changing sound dir
			if(use_warnings->value)
				gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, level.snd_teamwins[winner], 1.0, ATTN_NONE, 0.0);
			// end of changing sound dir
			teams[winner].score++;
			if (esp->value)
				EspEndOfRoundCleanup();
			gi.cvar_forceset(teams[winner].teamscore->name, va("%i", teams[winner].score));

			PrintScores ();
		}
	}

	if (CheckTimelimit())
		return 1;
	
	if (CheckRoundLimit())
		return 1;
	
	if (vCheckVote()) {
		EndDMLevel ();
		team_round_going = team_round_countdown = team_game_going = 0;
		return 1;
	}
	vNewRound ();

	if (teamplay->value && (!timelimit->value || level.matchTime <= ((timelimit->value * 60) - 5)))
	{
		arg[0] = '\0';
		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = &g_edicts[1 + i];

			if (cl_ent->inuse && cl_ent->client->resp.stat_mode == 2)
					Cmd_Stats_f(cl_ent, arg);
		}
	}
	// Increment roundNum for tracking
	game.roundNum++;

	// Reset kill streaks in team modes
	if (use_killcounts->value){
		for (i = 0; i < game.maxclients; i++) {
			cl_ent = g_edicts + 1 + i;
			cl_ent->client->resp.roundStreakKills = 0;
		}
	}

	return 0;
}


int CheckTeamRules (void)
{
	int winner = WINNER_NONE, i;
	int checked_tie = 0;
	char buf[1024];
	struct tm *now = NULL;
	time_t tnow = 0;
	char ltm[64] = "";
	char mvdstring[512] = "";

	if (round_delay_time && use_tourney->value)
	{
		TourneyTimeEvent (T_END, round_delay_time);
		round_delay_time--;
		if (!round_delay_time)
		{
			TourneyNewRound ();
			team_round_countdown = TourneySetTime( T_RSTART );
			TourneyTimeEvent (T_START, team_round_countdown);
		}
		return 0;
	}

	if (lights_camera_action)
	{
		ContinueLCA ();
		return 0;
	}

	if (team_round_going)
		current_round_length++;

	if (holding_on_tie_check)
	{
		if (esp->value) {
			holding_on_tie_check = 0;
			checked_tie = 1;
		}
		holding_on_tie_check--;
		if (holding_on_tie_check > 0)
			return 0;
		holding_on_tie_check = 0;
		checked_tie = 1;
	}

	if (team_round_countdown)
	{
		team_round_countdown--;
		if(!team_round_countdown)
		{
			if (!esp->value && BothTeamsHavePlayers())
			{
				in_warmup = 0;
				team_game_going = 1;
				StartLCA();
			}
			else if (esp->value && AllTeamsHaveLeaders() && BothTeamsHavePlayers())
			{
				if (esp_debug->value)
					gi.dprintf("%s: Esp mode on, All teams have leaders, Both teams have players\n", __FUNCTION__);
				in_warmup = 0;
				team_game_going = 1;
				StartLCA();
			}
			else
			{
				if (esp->value && !AllTeamsHaveLeaders()) {
					if (atl->value){
						CenterPrintAll ("All Teams Must Have a Leader!\nType 'leader' in console to volunteer!");
					} else if (etv->value) {
						CenterPrintTeam(TEAM1, "Team 1 Must Have a Leader!\nType 'leader' in console to volunteer!");
						CenterPrintTeam(TEAM2, "Waiting on Team 1 to select a leader...");
					}
					EspEndOfRoundCleanup();
				} else if (!matchmode->value || TeamsReady()) {
					CenterPrintAll ("Not enough players to play!");
						// Remove all leaders if they are bots
						if (esp->value){
							
						}
				} else {
					CenterPrintAll ("Both Teams Must Be Ready!");
				}
				team_round_going = team_round_countdown = team_game_going = 0;
				MakeAllLivePlayersObservers ();
				if (matchmode->value && warmup_unready->value)
					RunWarmup();
			}
		}
		else if(use_tourney->value)
		{
			TourneyTimeEvent (T_START, team_round_countdown);
		}
		else
		{
			if (team_round_countdown == 101)
			{
				gi.sound (&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
				gi.soundindex ("world/10_0.wav"), 1.0, ATTN_NONE, 0.0);

				#if USE_AQTION
				// Cleanup and remove all bots, it's go time!
				if (warmup_bots->value){
					gi.cvar_forceset("am", "0");
					gi.cvar_forceset("am_botcount", "0");
					attract_mode_bot_check();
					ACESP_RemoveBot("all");
					CenterPrintAll("All bots removed, good luck and have fun!");

					//Re-enable stats now that the bots are gone
					game.ai_ent_found = false;
					gi.cvar_forceset(stat_logs->name, "1");
				}

				// Move spectators to the capturepoint
				if (esp->value && etv->value) {
					// find the capturepoint
					edict_t *capturepoint;
					edict_t *ent;
					capturepoint = G_Find(NULL, FOFS(classname), "item_flag");
					if (!capturepoint) // Somehow we're in ETV with no capture point?	
						gi.dprintf("ERROR: No capture point (item_flag) found for ETV!?\n");

					if (capturepoint) {
						gi.dprintf("INFO: capture point (item_flag) found\n");
						VectorCopy( capturepoint->s.origin, level.poi_origin );
						VectorCopy( capturepoint->s.angles, level.poi_angle );
					}

					// move all spectators to the capture point
					for (i = 0, ent = g_edicts + 1; i < game.maxclients; i++, ent++)
					{
						if (!ent->inuse || !ent->client || ent->is_bot)
							continue;
						if (ent->solid != SOLID_NOT)
							continue;
						// if (!ent->client->pers.spectator)
						// 	continue;
						gi.dprintf("INFO: moving spectators\n");

						MoveClientToPOI(ent, capturepoint);
					}
				}
				#endif
			}
		}
		if(team_round_countdown == 41 && !matchmode->value)
		{
			while(CheckForUnevenTeams(NULL));
		}
	}

	// check these rules every 1.5 seconds...
	if (++rulecheckfrequency % 15 && !checked_tie)
		return 0;

	if (matchmode->value)
	{
		if (mm_allowlock->value)
		{
			for (i = TEAM1; i <= teamCount; i++)
			{
				if (teams[i].locked && !TeamHasPlayers( i ))
				{
					teams[i].locked = 0;
					sprintf( buf, "%s unlocked (no players)", TeamName( i ) );
					CenterPrintAll( buf );
				}
			}
		}
	}

	if (!team_round_going)
	{
		RunWarmup();

		if (CheckTimelimit())
			return 1;

		if (vCheckVote()) {
			EndDMLevel();
			team_round_going = team_round_countdown = team_game_going = 0;
			return 1;
		}

		if (esp->value) {
			if (!AllTeamsHaveLeaders())
				return 1;

			if (EspCheckRules()){
				EndDMLevel();
				team_round_going = team_round_countdown = team_game_going = 0;
				return 1;
			}
		}

		if (!team_round_countdown)
		{
			if (BothTeamsHavePlayers() || (esp->value && AllTeamsHaveLeaders() && BothTeamsHavePlayers()))
			{
				if (use_tourney->value)
				{
					TourneyNewRound ();
					team_round_countdown = TourneySetTime( T_START );
					TourneyTimeEvent (T_START, team_round_countdown);
				}
				else
				{
					int warmup_length = max( warmup->value, round_begin->value );
					char buf[64] = "";
					if (esp->value && BothTeamsHavePlayers()) {
						sprintf( buf, "All teams are ready!\nThe round will begin in %d seconds!", warmup_length );
					} else {
						sprintf( buf, "The round will begin in %d seconds!", warmup_length );
					}
					CenterPrintAll( buf );
					team_round_countdown = warmup_length * 10 + 2;

					// JBravo: Autostart q2pro MVD2 recording on the server
					if( use_mvd2->value )
					{
						tnow = time(NULL);
						now = localtime(&tnow);
						strftime( ltm, 64, "%Y%m%d-%H%M%S", now );
						Q_snprintf( mvdstring, sizeof(mvdstring), "mvdrecord %s-%s\n", ltm, level.mapname );
						gi.AddCommandString( mvdstring );
						gi.bprintf( PRINT_HIGH, "Starting MVD recording to file %s-%s.mvd2\n", ltm, level.mapname );
					}
					// JBravo: End MVD2
				}
			}
		}
    }
	else
	/* team_round_going */
	{
		if (!(gameSettings & GS_ROUNDBASED))
		{
			if (CheckTimelimit())
				return 1;

			if (ctf->value && CTFCheckRules())
			{
				ResetPlayers();
				EndDMLevel();
				team_round_going = team_round_countdown = team_game_going = 0;
				return 1;
			}

			if (dom->value && DomCheckRules())
			{
				EndDMLevel();
				team_round_going = team_round_countdown = team_game_going = 0;
				return 1;
			}

			if (vCheckVote()) {
				EndDMLevel ();
				team_round_going = team_round_countdown = team_game_going = 0;
				return 1;
			}

			if (!BothTeamsHavePlayers() || (esp->value && !AllTeamsHaveLeaders()))
			{
				if (!matchmode->value || TeamsReady())
					CenterPrintAll( "Not enough players to play!" );
				else if (esp->value && !AllTeamsHaveLeaders())
					CenterPrintAll ("Both Teams Must Have a Leader!\nType 'leader' in console to volunteer!");
				else
					CenterPrintAll( "Both Teams Must Be Ready!" );

				team_round_going = team_round_countdown = team_game_going = 0;
				MakeAllLivePlayersObservers();
				if (matchmode->value && warmup_unready->value)
					RunWarmup();

				/* try to restart the game */
				while (CheckForUnevenTeams( NULL ));
			}
			return 0; //CTF and teamDM dont need to check winner, its not round based
		}
		else if ((gameSettings & GS_ROUNDBASED))
		// Team round is going, and it's GS_ROUNDBASED
		{
			if (esp->value) {
				//Debugging
				if (esp_debug->value)
					EspDebug();

				if (EspCheckRules()){
					EndDMLevel();
					team_round_going = team_round_countdown = team_game_going = 0;
					return 1;
				}
				GenerateMedKit(false);
				EspCleanUp();
				EspLeaderCheck();
			}
		}

		winner = CheckForWinner();
		if (winner != WINNER_NONE)
		{
			if (!checked_tie)
			{
				holding_on_tie_check = 50;
				return 0;
			}
			if (WonGame(winner))
				return 1;

			team_round_going = 0;
			lights_camera_action = 0;
			holding_on_tie_check = 0;
			timewarning = fragwarning = 0;

			if (use_tourney->value)
				round_delay_time = TourneySetTime (T_END);
			else
				team_round_countdown = 71;

			return 0;
		}

		if (CheckRoundTimeLimit())
			return 1;
	}
	return 0;
}


void A_Scoreboard (edict_t * ent)
{
	int wteam = 0;

	// int timeRemaining = (level.matchTime >= (timelimit->value - 1) * 60);

	// if (timelimit->value && timeRemaining < 60){
	// 		ent->client->ps.stats[STAT_TIMER_ICON] = level.pic_esp_respawn_icon;
	// 		ent->client->ps.stats[STAT_TIMER] = timeRemaining / HZ;
	// }

	if (ent->client->layout == LAYOUT_SCORES)
	{
		// blink header of the winning team during intermission
		if (level.intermission_framenum && ((level.realFramenum / FRAMEDIV) & 8))
		{			// blink 1/8th second
			if (teams[TEAM1].score > teams[TEAM2].score)
				wteam = TEAM1;
			else if (teams[TEAM2].score > teams[TEAM1].score)
				wteam = TEAM2;
			else if (teams[TEAM1].total > teams[TEAM2].total)	// frag tie breaker
				wteam = TEAM1;
			else if (teams[TEAM2].total > teams[TEAM1].total)
				wteam = TEAM2;

			if(teamCount == 3)
			{
				if(wteam) {
					if (teams[TEAM3].score > teams[wteam].score)
						wteam = TEAM3;
					else if (teams[TEAM3].score == teams[wteam].score) {
						if(teams[TEAM3].total > teams[wteam].total)
							wteam = TEAM3;
						else if (teams[TEAM3].total == teams[wteam].total)
							wteam = 0;
					}
				} else {
					if(teams[TEAM3].score > teams[TEAM1].score)
						wteam = TEAM3;
					else if (teams[TEAM3].total > teams[TEAM1].total)
						wteam = TEAM3;
				}
			}

			if (wteam == 1)
				ent->client->ps.stats[STAT_TEAM1_PIC] = 0;
			else if (wteam == 2)
				ent->client->ps.stats[STAT_TEAM2_PIC] = 0;
			else if (wteam == 3 && (teamCount == 3))
				ent->client->ps.stats[STAT_TEAM3_PIC] = 0;
			else // tie game!
			{
				ent->client->ps.stats[STAT_TEAM1_PIC] = 0;
				ent->client->ps.stats[STAT_TEAM2_PIC] = 0;
				if(teamCount == 3)
					ent->client->ps.stats[STAT_TEAM3_PIC] = 0;
			}
		}
		else
		{
			ent->client->ps.stats[STAT_TEAM1_PIC] = level.pic_teamskin[TEAM1];
			ent->client->ps.stats[STAT_TEAM2_PIC] = level.pic_teamskin[TEAM2];
			if (teamCount == 3)
				ent->client->ps.stats[STAT_TEAM3_PIC] = level.pic_teamskin[TEAM3];
		}

		ent->client->ps.stats[STAT_TEAM1_SCORE] = teams[TEAM1].score;
		ent->client->ps.stats[STAT_TEAM2_SCORE] = teams[TEAM2].score;
		if (teamCount == 3)
			ent->client->ps.stats[STAT_TEAM3_SCORE] = teams[TEAM3].score;
	}
}


static int G_PlayerCmp( const void *p1, const void *p2 )
{
	gclient_t *a = *(gclient_t * const *)p1;
	gclient_t *b = *(gclient_t * const *)p2;
	
	if (a->resp.score != b->resp.score)
		return b->resp.score - a->resp.score;
	
	if (a->resp.deaths < b->resp.deaths)
		return -1;
	if (a->resp.deaths > b->resp.deaths) 
		return 1;
	
	if (a->resp.damage_dealt > b->resp.damage_dealt)
		return -1;
	if (a->resp.damage_dealt < b->resp.damage_dealt)
		return 1;
	
	return 0;
}

int G_SortedClients( gclient_t **sortedList )
{
	int i, total = 0;
	gclient_t *client;

	for (i = 0, client = game.clients; i < game.maxclients; i++, client++) {
		if (!client->pers.connected || client->pers.mvdspec)
			continue;

		sortedList[total++] = client;
	}

	qsort( sortedList, total, sizeof( gclient_t * ), G_PlayerCmp );

	return total;
}

int G_NotSortedClients( gclient_t **sortedList )
{
	int i, total = 0;
	gclient_t *client;

	for (i = 0, client = game.clients; i < game.maxclients; i++, client++) {
		if (!client->pers.connected || client->pers.mvdspec)
			continue;

		sortedList[total++] = client;
	}

	return total;
}

#define MAX_SCOREBOARD_SIZE 1024
#define TEAM_HEADER_WIDTH	160 //skin icon and team tag
#define TEAM_ROW_CHARS		32  //"yv 42 string2 \"name\" "
#define TEAM_ROW_WIDTH		160 //20 chars, name and possible captain tag
#define TEAM_ROW_CHARS2		44  //yv %d string%c \"%-15s %3d %3d %3d\" "
#define TEAM_ROW_WIDTH2		216 //27 chars, name Frg Tim Png 
#define TEAM_ROW_GAP		30

// Maximum number of lines of scores to put under each team's header.
#define MAX_SCORES_PER_TEAM 9

#define MAX_PLAYERS_PER_TEAM 8

void A_NewScoreboardMessage(edict_t * ent)
{
	char buf[1024];
	char string[1024] = { '\0' };
	gclient_t *sortedClients[MAX_CLIENTS];
	int total[TEAM_TOP] = { 0, 0, 0, 0 };
	int i, j, line = 0, lineh = 8;
	int dead, alive, totalClients, maxPlayers, printCount;
	gclient_t *cl;
	edict_t *cl_ent;

	// show alive players when dead
	dead = (!IS_ALIVE(ent) || !team_round_going);
	if (limchasecam->value != 0)
		dead = 0;

	totalClients = G_SortedClients(sortedClients);

	for(i = 0; i < totalClients; i++) {
		total[sortedClients[i]->resp.team]++;
	}

	// print teams
	for (i = TEAM1; i <= teamCount; i++)
	{
		Q_snprintf( buf, sizeof( buf ), "xv 44 yv %d string2 \"%3d %-11.11s Frg Tim Png\"", line++ * lineh, teams[i].score, teams[i].name );
		Q_strncatz( string, buf, sizeof( string ) );

		Q_snprintf( buf, sizeof( buf ), "xv 44 yv %d string2 \"%s\" ",
			line++ * lineh,
			"\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F \x9D\x9E\x9F \x9D\x9E\x9F \x9D\x9E\x9F"
			);
		Q_strncatz( string, buf, sizeof( string ) );

		if (!total[i])
			continue;

		printCount = 0;
		if (total[i] > MAX_PLAYERS_PER_TEAM)
			maxPlayers = MAX_PLAYERS_PER_TEAM - 1;
		else
			maxPlayers = total[i];

		for (j = 0; j < totalClients; j++)
		{
			cl = sortedClients[j];
			if (cl->resp.team != i)
				continue;

			cl_ent = g_edicts + 1 + (cl - game.clients);
			alive = IS_ALIVE(cl_ent);

			Q_snprintf( buf, sizeof( buf ), "xv 44 yv %d string%c \"%-15s %3d %3d %3d\"",
				line++ * lineh,
				(alive && dead ? '2' : ' '),
				cl->pers.netname,
				cl->resp.score,
				(level.framenum - cl->resp.enterframe) / 600 / FRAMEDIV,
				min(cl->ping, 999) );
			Q_strncatz( string, buf, sizeof( string ) );
			printCount++;
			if (printCount >= maxPlayers)
				break;
		}

		// show the amount of excess players
		if (total[i] > MAX_PLAYERS_PER_TEAM) {
			Q_snprintf( buf, sizeof( buf ), "xv 44 yv %d string \"   ..and %d more\"", line++ * lineh, total[i] - MAX_PLAYERS_PER_TEAM + 1 );
			Q_strncatz( string, buf, sizeof( string ) );
		}

		line++;
	}

	string[sizeof( string ) - 1] = '\0';
	if (strlen( string ) > MAX_SCOREBOARD_SIZE - 1) {
		string[MAX_SCOREBOARD_SIZE - 1] = '\0';
	}

	gi.WriteByte( svc_layout );
	gi.WriteString( string );
}

//AQ2:TNG - Slicer Modified Scores for Match Mode
void A_ScoreboardMessage (edict_t * ent, edict_t * killer)
{
	char string[2048];
	gclient_t *sortedClients[MAX_CLIENTS], *cl;
	edict_t *cl_ent;
	int totalClients;
	int maxsize = 1000, i, j, line_x, line_y;

	string[0] = 0;

	if (ent->client->layout == LAYOUT_SCORES)
	{
		char footer[256], playername[16];
		int team, len, footerLen = 0, remaining, deadview;
		int total[TEAM_TOP] = {0,0,0,0};
		int totalsubs[TEAM_TOP] = {0,0,0,0};
		int totalscore[TEAM_TOP] = {0,0,0,0};
		int totalalive[TEAM_TOP] = {0,0,0,0};
		int totalaliveprinted;
		int name_pos, flagindex = 0;
		int tpic[TEAM_TOP][2] = {{0,0},{24,26},{25,27},{30,31}};
		char temp[16];
		int maxPlayersPerTeam, scoreWidth = 3, rowWidth, rowChars, rowGap, headerOffset = 0;
		int maxPlayers, printCount, base_x, showExtra = 0, subLines = 0;

		// new scoreboard for regular teamplay up to 16 players
		if (use_newscore->value == 1 && teamplay->value && !matchmode->value && !ctf->value) {
			A_NewScoreboardMessage(ent);
			return;
		}

		if (use_newscore->value > 1 && teamCount < 3) {
			showExtra = 1;
			rowWidth = max(TEAM_HEADER_WIDTH, TEAM_ROW_WIDTH2);
			rowChars = TEAM_ROW_CHARS2;
			rowGap = TEAM_ROW_GAP;
		} else {
			rowWidth = max(TEAM_HEADER_WIDTH, TEAM_ROW_WIDTH);
			rowChars = TEAM_ROW_CHARS;
			rowGap = 0;
		}
		
		base_x = 160 - ((rowWidth + rowGap) * teamCount) / 2 + rowGap / 2;

		if(ctf->value)
		{
			base_x += 8;
			tpic[TEAM1][0] = 30;
			tpic[TEAM2][0] = 31;
		}
		else if(esp->value)
		{
			base_x += 8;
			tpic[TEAM1][0] = STAT_TEAM1_PIC;
			tpic[TEAM2][0] = STAT_TEAM2_PIC;
			tpic[TEAM3][0] = STAT_TEAM3_PIC;
		}
		else if(teamdm->value)
		{
			scoreWidth = 3;
		}

		deadview = (!IS_ALIVE(ent) || !team_round_going);
		// AQ:TNG - Hack to keep scoreboard from revealing whos alive during matches - JBravo
		if (limchasecam->value != 0)
			deadview = 0;

		if (noscore->value)
			totalClients = G_NotSortedClients(sortedClients);
		else 
			totalClients = G_SortedClients(sortedClients);

		ent->client->ps.stats[STAT_TEAM_HEADER] = level.pic_teamtag;

		for (i = 0; i < totalClients; i++) {
			cl = sortedClients[i];
			team = cl->resp.team;

			if (cl->resp.subteam) {
				totalsubs[team]++;
				continue;
			}
			
			cl_ent = g_edicts + 1 + (cl - game.clients);
			if (IS_ALIVE(cl_ent))
				totalalive[team]++;

			totalscore[team] += cl->resp.score;
			total[team]++;
		}

		len = 0;
		//Build team headers
		if (use_newscore->value > 2 && rowWidth > TEAM_HEADER_WIDTH)
			headerOffset = (rowWidth - TEAM_HEADER_WIDTH) / 2;

		rowWidth += rowGap;

		sprintf(string + len, "yv 8 ");
		len = strlen(string);
		//Add skin img
		for (i = TEAM1, line_x = base_x + headerOffset; i <= teamCount; i++, line_x += rowWidth)
		{
			sprintf(string + len,
				"if %i xv %i pic %i endif ",
				tpic[i][0], line_x, tpic[i][0]);
			len = strlen(string);
		}

		//Add team tag img
		if (!ctf->value) {
			Q_strncatz(string, "if 22 ", sizeof(string));
			len = strlen(string);
			for (i = TEAM1, line_x = base_x + headerOffset; i <= teamCount; i++, line_x += rowWidth)
			{
				sprintf(string + len, "xv %i pic 22 ", line_x + 32);
				len = strlen(string);
			}
			Q_strncatz(string, "endif ", sizeof(string));
			len = strlen(string);
		}

		//Add player info
		sprintf( string + len, "yv 28 " );
		len = strlen( string );
		for (i = TEAM1, line_x = base_x + headerOffset; i <= teamCount; i++, line_x += rowWidth)
		{
			if (matchmode->value)
				Q_snprintf(temp, sizeof(temp), "%4i/%2i/%-2d", totalscore[i], total[i], totalsubs[i]);
			else
				Q_snprintf(temp, sizeof(temp), "%4i/%2i", totalscore[i], total[i]);

			sprintf( string + len,
				"xv %i string \"%s\" ",
				line_x + 32, temp );
			len = strlen( string );
		}

		//Add score
		sprintf( string + len, "yv 12 " );
		len = strlen( string );
		for (i = TEAM1, line_x = base_x + headerOffset; i <= teamCount; i++, line_x += rowWidth)
		{
			sprintf( string + len,
				"xv %i num %i %i ",
				line_x + (ctf->value ? 90 : 96), scoreWidth, tpic[i][1] );
			len = strlen( string );
		}

		//Add team name
		sprintf( string + len, "yv 0 " );
		len = strlen( string );
		for (i = TEAM1, line_x = base_x + headerOffset; i <= teamCount; i++, line_x += rowWidth)
		{
			name_pos = ((20 - strlen( teams[i].name )) / 2) * 8;
			if (name_pos < 0)
				name_pos = 0;

			sprintf( string + len,
				"xv %d string \"%s\" ",
				line_x + name_pos, teams[i].name );
			len = strlen( string );
		}

		//Build footer
		footer[0] = 0;
		if (matchmode->value)
		{
			int secs, mins;

			i = level.matchTime;
			mins = i / 60;
			secs = i % 60;

			sprintf( footer, "yv 128 " );
			footerLen = strlen( footer );
			for (i = TEAM1, line_x = base_x + 39; i <= teamCount; i++, line_x += rowWidth)
			{
				sprintf( footer + footerLen, "xv %i string2 \"%s\" ",
					line_x, teams[i].ready ? "Ready" : "Not Ready" );
				footerLen = strlen( footer );
			}

			sprintf( footer + footerLen, "xv 112 yv 144 string \"Time: %d:%02d\" ", mins, secs );
			footerLen = strlen( footer );
		}

		remaining = MAX_SCOREBOARD_SIZE - 1 - len - footerLen;

		maxPlayersPerTeam = MAX_SCORES_PER_TEAM;
		if (maxPlayersPerTeam > (remaining / rowChars) / teamCount)
			maxPlayersPerTeam = (remaining / rowChars) / teamCount;

		for (i = TEAM1, line_x = base_x; i <= teamCount; i++, line_x += rowWidth)
		{
			line_y = 42;
			sprintf( string + len, "xv %i ", line_x );
			len = strlen(string);
			
			if (showExtra) {
				sprintf( string + len, "yv %d string2 \"Name            Frg Tim Png\" ", line_y );
				len = strlen( string );
				line_y += 8;
			}

			if (ctf->value)
				flagindex = (i == TEAM1) ? items[FLAG_T1_NUM].index : items[FLAG_T2_NUM].index;

			printCount = 0;
			totalaliveprinted = 0;

			maxPlayers = maxPlayersPerTeam;

			if (matchmode->value) {
				subLines = 1 + min(totalsubs[i], 2); //for subs
				if (maxPlayers - subLines < 4)
					subLines = 1;

				maxPlayers -= subLines;
			}

			if (total[i] > maxPlayers)
				maxPlayers = maxPlayers - 1;
			else
				maxPlayers = total[i];

			if (maxPlayers)
			{
				for (j = 0; j < totalClients; j++)
				{
					cl = sortedClients[j];

					if (cl->resp.team != i)
						continue;
					if (cl->resp.subteam)
						continue;

					cl_ent = g_edicts + 1 + (cl - game.clients);
					if (IS_ALIVE(cl_ent))
						totalaliveprinted++;

					playername[0] = 0;
					if (IS_CAPTAIN(cl_ent) || IS_LEADER(cl_ent)) {
						playername[0] = '@';
						playername[1] = 0;
					}
					Q_strncatz(playername, cl->pers.netname, sizeof(playername));
					if (showExtra) {
						sprintf( string + len,
							"yv %d string%s \"%-15s %3d %3d %3d\" ",
							line_y,
							(deadview && cl_ent->solid != SOLID_NOT) ? "2" : "",
							playername,
							cl->resp.score,
							(level.framenum - cl->resp.enterframe) / (60 * HZ),
							min(cl->ping, 999) );
					} else {
						sprintf( string + len,
							"yv %i string%s \"%s\" ",
							line_y,
							(deadview && cl_ent->solid != SOLID_NOT) ? "2" : "",
							playername );
					}

					len = strlen( string );
					if (ctf->value && cl->inventory[flagindex]){
						sprintf( string + len, "xv %i picn sbfctf%s xv %i ", line_x - 8, i == TEAM1 ? "2" : "1", line_x );
						len = strlen( string );
					}

					line_y += 8;
					printCount++;
					if (printCount >= maxPlayers)
						break;
				}

				// Print remaining players if we ran out of room...
				if (printCount < total[i])
				{
					if (!deadview)	// live player viewing scoreboard...
					{
						sprintf( string + len,
							"yv %i string \"..and %i more\" ",
							line_y, total[i] - printCount );
						len = strlen( string );
					}
					else			// dead player viewing scoreboard...
					{
						sprintf( string + len,
							"yv %i string%s \"..and %i/%i more\" ",
							line_y,
							(totalalive[i] - totalaliveprinted) ? "2" : "",
							totalalive[i] - totalaliveprinted,
							total[i] - printCount );
						len = strlen( string );
					}
					line_y += 8;
				}
			}

			if (subLines == 1 && totalsubs[i]) //Subs
			{
				line_y = 96;
				sprintf( string + len, "yv %i string2 \"%d Subs\" ", line_y, totalsubs[i] );
				len = strlen( string );
				line_y += 8;

			}
			else if (subLines > 0)
			{
				line_y = 96;
				sprintf( string + len, "yv %i string2 \"Subs\" ", line_y );
				len = strlen( string );

				line_y += 8;
				printCount = 0;

				if (totalsubs[i] > subLines - 1)
					maxPlayers = subLines - 2;
				else
					maxPlayers = totalsubs[i];

				if (maxPlayers)
				{
					for (j = 0; j < totalClients; j++)
					{
						cl = sortedClients[j];

						if (cl->resp.team != i)
							continue;
						if (!cl->resp.subteam)
							continue;

						cl_ent = g_edicts + 1 + (cl - game.clients);
						sprintf( string + len,
							"yv %d string \"%s%s\" ",
							line_y, IS_CAPTAIN(cl_ent) ? "@" : "", cl->pers.netname );
						len = strlen( string );

						line_y += 8;
						printCount++;
						if (printCount >= maxPlayers)
							break;
					}

					// Print remaining players if we ran out of room...
					if (printCount < totalsubs[i])
					{
						sprintf( string + len,
							"yv %i string \" + %i more\" ",
							line_y, totalsubs[i] - printCount );
						len = strlen( string );

						line_y += 8;
					}
				}
			}
		}

		if (footerLen) {
			string[MAX_SCOREBOARD_SIZE - 1 - footerLen] = 0;
			Q_strncatz(string, footer, sizeof(string));
		}
	}
	else if (ent->client->layout == LAYOUT_SCORES2)
	{
		const char *sb = scoreboard->string;
		int sb_len = 0;
		int chars = 0;

		if( ! sb[0] )
		{
			if( noscore->value )
				sb = "NMP";
			else if( ctf->value )
				sb = "SNMPCT";
			else if( teamdm->value )
				sb = "FNMPDT";
			else
				sb = "FNMPIT";
		}

		sb_len = strlen(sb);

		for( i = 0; i < sb_len; i ++ )
		{
			char field = toupper( sb[ i ] );
			if( i )
				chars ++;
			if(      field == 'F' ) chars +=  5;
			else if( field == 'T' ) chars +=  4;
			else if( field == 'N' ) chars += 15;
			else if( field == 'M' ) chars +=  4;
			else if( field == 'P' ) chars +=  4;
			else if( field == 'C' ) chars +=  4;
			else if( field == 'S' ) chars +=  5;
			else if( field == 'K' ) chars +=  5;
			else if( field == 'D' ) chars +=  6;
			else if( field == 'I' ) chars +=  6;
			else if( field == 'A' ) chars +=  3;
			else chars ++;
		}

		if (noscore->value)
			totalClients = G_NotSortedClients(sortedClients);
		else
			totalClients = G_SortedClients(sortedClients);

		line_x = 160 - (chars * 4);
		sprintf( string, "xv %i yv 32 string2 \"", line_x );

		for( i = 0; i < sb_len; i ++ )
		{
			char field = toupper( sb[ i ] );
			if( i )
				strcat( string, " " );

			if(      field == 'F' ) strcat( string, "Frags" );
			else if( field == 'T' ) strcat( string, "Team" );
			else if( field == 'N' ) strcat( string, "Player         " );
			else if( field == 'M' ) strcat( string, "Time" );
			else if( field == 'P' ) strcat( string, "Ping" );
			else if( field == 'C' ) strcat( string, "Caps" );
			else if( field == 'S' ) strcat( string, "Score" );
			else if( field == 'K' ) strcat( string, "Kills" );
			else if( field == 'D' ) strcat( string, "Deaths" );
			else if( field == 'I' ) strcat( string, "Damage" );
			else if( field == 'A' ) strcat( string, "Acc" );
			else sprintf( string + strlen(string), "%c", sb[ i ] );
		}

		strcat( string, "\" yv 40 string2 \"" );

		for( i = 0; i < sb_len; i ++ )
		{
			char field = toupper( sb[ i ] );
			if( i )
				strcat( string, " " );

			if(      field == 'F' ) strcat( string, "\x9D\x9E\x9E\x9E\x9F" );
			else if( field == 'T' ) strcat( string, "\x9D\x9E\x9E\x9F" );
			else if( field == 'N' ) strcat( string, "\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F" );
			else if( field == 'M' ) strcat( string, "\x9D\x9E\x9E\x9F" );
			else if( field == 'P' ) strcat( string, "\x9D\x9E\x9E\x9F" );
			else if( field == 'C' ) strcat( string, "\x9D\x9E\x9E\x9F" );
			else if( field == 'S' ) strcat( string, "\x9D\x9E\x9E\x9E\x9F" );
			else if( field == 'K' ) strcat( string, "\x9D\x9E\x9E\x9E\x9F" );
			else if( field == 'D' ) strcat( string, "\x9D\x9E\x9E\x9E\x9E\x9F" );
			else if( field == 'I' ) strcat( string, "\x9D\x9E\x9E\x9E\x9E\x9F" );
			else if( field == 'A' ) strcat( string, "\x9D\x9E\x9F" );
			else sprintf( string + strlen(string), "%c", sb[ i ] );
		}

		strcat( string, "\" " );

		line_y = 48;

		for (i = 0; i < totalClients; i++)
		{
			cl = sortedClients[i];
			cl_ent = g_edicts + 1 + (cl - game.clients);

			sprintf( string + strlen(string), "yv %i string \"", line_y );

			for( j = 0; j < sb_len; j ++ )
			{
				char buf[ 16 ] = "";
				char field = toupper( sb[ j ] );
				if( j )
					strcat( string, " " );

				if(      field == 'F' ) Q_snprintf( buf, sizeof(buf), "%5i", min( 99999, cl->resp.score ) );
				else if( field == 'T' )
				{
					if( matchmode->value )
					{
						char suffix = ' ';
						if( IS_CAPTAIN(cl_ent) )
							suffix = 'C';
						else if( cl->resp.subteam )
							suffix = 'S';
						Q_snprintf( buf, sizeof(buf), "  %c%c", (cl->resp.team ? (cl->resp.team + '0') : ' '), suffix );
					}
					else
						Q_snprintf( buf, sizeof(buf), "   %c", (cl->resp.team ? (cl->resp.team + '0') : ' ') );
				}
				else if( field == 'N' ) Q_snprintf( buf, sizeof(buf), "%-15s", cl->pers.netname );
				else if( field == 'M' )
				{
					int minutes = (level.framenum - cl->resp.enterframe) / (60 * HZ);
					if( minutes < 60 )
						Q_snprintf( buf, sizeof(buf), "%4i", minutes );
					else if( minutes < 600 )
						Q_snprintf( buf, sizeof(buf), "%1i:%02i", minutes / 60, minutes % 60 );
					else
						Q_snprintf( buf, sizeof(buf), "%3ih", min( 999, minutes / 60 ) );
				}
				else if( field == 'P' )
				{
#ifndef NO_BOTS
					if( cl_ent->is_bot )
						strcpy( buf, " BOT" );
					else
#endif
						Q_snprintf( buf, sizeof(buf), "%4i", min( 9999, cl->ping ) );
				}
				else if( field == 'C' ) Q_snprintf( buf, sizeof(buf), "%4i", min( 9999, cl->resp.ctf_caps ) );
				else if( field == 'S' ) Q_snprintf( buf, sizeof(buf), "%5i", min( 99999, cl->resp.score ) );
				else if( field == 'K' ) Q_snprintf( buf, sizeof(buf), "%5i", min( 99999, cl->resp.kills) );
				else if( field == 'D' ) Q_snprintf( buf, sizeof(buf), "%6i", min( 999999, cl->resp.deaths) );
				else if( field == 'I' ) Q_snprintf( buf, sizeof(buf), "%6i", min( 999999, cl->resp.damage_dealt) );
				else if( field == 'A' ) Q_snprintf( buf, sizeof(buf), "%3.f", cl->resp.shotsTotal ? (double) cl->resp.hitsTotal * 100.0 / (double) cl->resp.shotsTotal : 0. );
				else sprintf( buf, "%c", sb[ j ] );

				strcat( string, buf );
			}

			strcat( string, "\" " );

			line_y += 8;
			if (strlen(string) > (maxsize - 100) && i < (totalClients - 2))
			{
				sprintf (string + strlen (string), "yv %d string \"..and %d more\" ",
					line_y, (totalClients - i - 1) );
				line_y += 8;
				break;
			}
		}
	}

	if (strlen(string) > MAX_SCOREBOARD_SIZE - 1) { // for debugging...
		gi.dprintf("Warning: scoreboard string neared or exceeded max length\nDump:\n%s\n---\n", string);
		string[MAX_SCOREBOARD_SIZE - 1] = '\0';
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

// called when we enter the intermission
void TallyEndOfLevelTeamScores (void)
{
	int i;
	gi.sound (&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
		gi.soundindex ("world/xian1.wav"), 1.0, ATTN_NONE, 0.0);

	teams[TEAM1].total = teams[TEAM2].total = teams[TEAM3].total = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		if (!g_edicts[i + 1].inuse)
			continue;
		if (game.clients[i].resp.team == NOTEAM)
			continue;

		teams[game.clients[i].resp.team].total += game.clients[i].resp.score;
	}

	// Stats begin
	#if USE_AQTION
		if (stat_logs->value && !matchmode->value) {
			LogMatch(); // Generates end of game stats
			LogEndMatchStats(); // Generates end of match logs
		}
	#endif
	// Stats: Reset roundNum
	game.roundNum = 0;
	// Stats end
}

/*
 * Teamplay spawning functions...
 */

edict_t *SelectTeamplaySpawnPoint (edict_t * ent)
{
	// Print all of the teamplay_spawns
	// int i;
	// for (i = 0; i < num_potential_spawns; i++)
	// 	gi.dprintf ("teamplay_spawn %d: %s\n", i, potential_spawns[i]->classname);
	// 	gi.dprintf("%s was called!\n\n\n", __FUNCTION__);
  return teamplay_spawns[ent->client->resp.team - 1];
}

// SpawnPointDistance: 
// Returns the distance between two spawn points (or any entities, actually...)
float SpawnPointDistance (edict_t * spot1, edict_t * spot2)
{
	return Distance(spot1->s.origin, spot2->s.origin);
}

spawn_distances_t *spawn_distances;
// GetSpawnPoints:
// Put the spawn points into our potential_spawns array so we can work with them easily.
void GetSpawnPoints (void)
{
	edict_t *spot = NULL;

	num_potential_spawns = 0;

	if ((spot = G_Find (spot, FOFS (classname), "info_player_team1")) != NULL)
	{
		potential_spawns[num_potential_spawns] = spot;
		num_potential_spawns++;
	}

	if ((spot = G_Find (spot, FOFS (classname), "info_player_team2")) != NULL)
	{
		potential_spawns[num_potential_spawns] = spot;
		num_potential_spawns++;
	}

	spot = NULL;
	while ((spot = G_Find (spot, FOFS (classname), "info_player_deathmatch")) != NULL)
	{
		potential_spawns[num_potential_spawns] = spot;
		num_potential_spawns++;
		if (num_potential_spawns >= MAX_SPAWNS)
		{
			gi.dprintf ("Warning: MAX_SPAWNS exceeded\n");
			break;
		}
	}

	if(spawn_distances)
		gi.TagFree (spawn_distances);

	spawn_distances = (spawn_distances_t *)gi.TagMalloc (num_potential_spawns *
					sizeof (spawn_distances_t), TAG_GAME);
}

// compare_spawn_distances is used by the qsort() call
int compare_spawn_distances (const void *sd1, const void *sd2)
{
	if (((spawn_distances_t *)sd1)->distance < ((spawn_distances_t *)sd2)->distance)
		return -1;
	else if (((spawn_distances_t *)sd1)->distance > ((spawn_distances_t *)sd2)->distance)
		return 1;
	else
		return 0;
}

void SelectRandomTeamplaySpawnPoint (int team, qboolean teams_assigned[])
{
	teamplay_spawns[team] = potential_spawns[newrand(num_potential_spawns)];
	teams_assigned[team] = true;
}

void SelectFarTeamplaySpawnPoint (int team, qboolean teams_assigned[])
{
	int x, y, spawn_to_use, preferred_spawn_points, num_already_used,
	total_good_spawn_points;
	float closest_spawn_distance, distance;

	if (team < 0 || team >= MAX_TEAMS) {
		gi.dprintf( "Out-of-range teams value in SelectFarTeamplaySpawnPoint, skipping...\n" );
		return;
	}

	num_already_used = 0;
	for (x = 0; x < num_potential_spawns; x++)
	{
		closest_spawn_distance = 2000000000;

		for (y = 0; y < teamCount; y++)
		{
			if (teams_assigned[y])
			{
				distance = SpawnPointDistance (potential_spawns[x], teamplay_spawns[y]);
				if (distance < closest_spawn_distance)
					closest_spawn_distance = distance;
			}
		}

		if (closest_spawn_distance == 0)
			num_already_used++;

		spawn_distances[x].s = potential_spawns[x];
		spawn_distances[x].distance = closest_spawn_distance;
	}

	qsort (spawn_distances, num_potential_spawns,
	sizeof (spawn_distances_t), compare_spawn_distances);

	total_good_spawn_points = num_potential_spawns - num_already_used;

	if (total_good_spawn_points <= 4)
		preferred_spawn_points = 1;
	else if (total_good_spawn_points <= 10)
		preferred_spawn_points = 2;
	else
		preferred_spawn_points = 3;

	//FB 6/1/99 - make DF_SPAWN_FARTHEST force far spawn points in TP
	if (DMFLAGS(DF_SPAWN_FARTHEST))
		preferred_spawn_points = 1;
	//FB 6/1/99

	spawn_to_use = newrand (preferred_spawn_points);

	teams_assigned[team] = true;
	teamplay_spawns[team] = spawn_distances[num_potential_spawns - spawn_to_use - 1].s;
}

// SetupTeamSpawnPoints:
//
// Setup the points at which the teams will spawn.
//
void SetupTeamSpawnPoints (void)
{
	qboolean teams_assigned[MAX_TEAMS];
	int i, l;

	for (l = 0; l < MAX_TEAMS; l++)
	{
		teamplay_spawns[l] = NULL;
		teams_assigned[l] = false;
	}

	l = newrand(teamCount);

	SelectRandomTeamplaySpawnPoint(l, teams_assigned);

	for(i = l+1; i < teamCount+l; i++) {
		SelectFarTeamplaySpawnPoint(i % teamCount, teams_assigned);
	}
}

// TNG:Freud New Spawning system
// NS_GetSpawnPoints:
// Put the potential spawns into arrays for each team.
void NS_GetSpawnPoints (void)
{
	int x, i;

	NS_randteam = newrand(teamCount);

	for (x = 0; x < teamCount; x++)
	{
		NS_num_used_farteamplay_spawns[x] = 0;
		NS_num_potential_spawns[x] = num_potential_spawns;
		for(i = 0; i < num_potential_spawns; i++)
			NS_potential_spawns[x][i] = potential_spawns[i];
	}
}

// TNG:Freud
// NS_SelectRandomTeamplaySpawnPoint
// Select a random spawn point for the team from remaining spawns.
// returns false if no spawns left.
qboolean NS_SelectRandomTeamplaySpawnPoint (int team, qboolean teams_assigned[])
{
	int spawn_point, z;

	if (NS_num_potential_spawns[team] < 1) {
		gi.dprintf("New Spawncode: gone through all spawns, re-reading spawns\n");
		NS_GetSpawnPoints ();
		NS_SetupTeamSpawnPoints ();
		return false;
	}

	spawn_point = newrand (NS_num_potential_spawns[team]);
	NS_num_potential_spawns[team]--;

	teamplay_spawns[team] = NS_potential_spawns[team][spawn_point];
	teams_assigned[team] = true;

	for (z = spawn_point;z < NS_num_potential_spawns[team];z++) {
		NS_potential_spawns[team][z] = NS_potential_spawns[team][z + 1];
	}

	return true;
}

// TNG:Freud
#define MAX_USABLESPAWNS 3
// NS_SelectFarTeamplaySpawnPoint
// Selects farthest teamplay spawn point from available spawns.
qboolean NS_SelectFarTeamplaySpawnPoint (int team, qboolean teams_assigned[])
{
	int u, x, y, z, spawn_to_use, preferred_spawn_points, num_already_used,
	total_good_spawn_points;
	float closest_spawn_distance, distance;
	edict_t *usable_spawns[MAX_USABLESPAWNS];
	qboolean used;
	int num_usable;

	if (team < 0 || team >= MAX_TEAMS) {
		gi.dprintf( "Out-of-range teams value in SelectFarTeamplaySpawnPoint, skipping...\n" );
		return false;
	}

	num_already_used = 0;
	for (x = 0; x < NS_num_potential_spawns[team]; x++)
	{
		closest_spawn_distance = 2000000000;

		for (y = 0; y < teamCount; y++)
		{
			if (teams_assigned[y])
			{
				distance =
				SpawnPointDistance (NS_potential_spawns[team][x], teamplay_spawns[y]);

				if (distance < closest_spawn_distance)
					closest_spawn_distance = distance;
			}
		}

		if (closest_spawn_distance == 0)
			num_already_used++;

		spawn_distances[x].s = NS_potential_spawns[team][x];
		spawn_distances[x].distance = closest_spawn_distance;
	}

	qsort (spawn_distances, NS_num_potential_spawns[team],
	sizeof (spawn_distances_t), compare_spawn_distances);

	total_good_spawn_points = NS_num_potential_spawns[team] - num_already_used;

	if (total_good_spawn_points <= 4)
		preferred_spawn_points = 1;
	else if (total_good_spawn_points <= 10)
		preferred_spawn_points = 2;
	else
		preferred_spawn_points = 3;

	//FB 6/1/99 - make DF_SPAWN_FARTHEST force far spawn points in TP
	if (DMFLAGS(DF_SPAWN_FARTHEST))
		preferred_spawn_points = 1;
	//FB 6/1/99

	num_usable = 0;
	for (z = 0;z < preferred_spawn_points;z++) {
		used = false;
		for (u = 0;u < NS_num_used_farteamplay_spawns[team];u++) {
			if (NS_used_farteamplay_spawns[team][u] ==
			spawn_distances[NS_num_potential_spawns[team] - z - 1].s) {
				used = true;
				break;
			}
		}
		if (used == false) {
			usable_spawns[num_usable] = spawn_distances[NS_num_potential_spawns[team] - z - 1].s;
			num_usable++;
			if (num_usable >= MAX_USABLESPAWNS) {
				break;
			}
		}
	}
	if (num_usable < 1) {
		NS_SetupTeamSpawnPoints();
		return false;
	}

	spawn_to_use = newrand(num_usable);

	NS_used_farteamplay_spawns[team][NS_num_used_farteamplay_spawns[team]] = usable_spawns[spawn_to_use];
	NS_num_used_farteamplay_spawns[team]++;

	teams_assigned[team] = true;
	teamplay_spawns[team] = usable_spawns[spawn_to_use];

	return true;
}

// TNG:Freud
// NS_SetupTeamSpawnPoints
// Finds and assigns spawn points to each team.
void NS_SetupTeamSpawnPoints (void)
{
	qboolean teams_assigned[MAX_TEAMS];
	int l;

	for (l = 0; l < MAX_TEAMS; l++) {
		teamplay_spawns[l] = NULL;
		teams_assigned[l] = false;
	}

	if (NS_SelectRandomTeamplaySpawnPoint (NS_randteam, teams_assigned) == false)
		return;

	for (l = 0; l < teamCount; l++) {
		if (l != NS_randteam && NS_SelectFarTeamplaySpawnPoint(l, teams_assigned) == false)
			return;
	}
}

qboolean Highlander_Check(edict_t *ent, int weaponNum)
{
	char *playername = ent->client->pers.netname;
	gitem_t *chosenWeapon = ent->client->pers.chosenWeapon;
	qboolean available = false;
	qboolean weaponChange = false;
	int i, j = 0;

	// gi.dprintf("Is weapon available: %d\n", weapon_status[weaponNum].available);
	// gi.dprintf("Which team owns it: %d\n", weapon_status[weaponNum].team);
	// if (owner != NULL && owner->client->pers.netname != NULL)
    // 	gi.dprintf("Which player owns it: %s\n", owner->client->pers.netname);
	// gi.dprintf("Which team wants it: %d\n", ent->client->resp.team);
	// gi.dprintf("Which player wants it: %s\n", playername);

	// Run a check for availability
	for (i = 0; i < WEAPON_MAX; i++) {
		for (j = 0; j < TEAM_TOP; j++) {
			// If the weapon has an owner in the current team, update its availability
			if (weapon_status[i][j].owner != NULL) {
				available = false;
			} else {
				available = true;
			}
		}
	}

	// If the player had a different weapon and is selecting this one, and it's available, return true,
    // and release the previous weapon so that it made be selectable by another member of this team
    if (ent->client->pers.chosenWeapon != NULL && available) {
        weapon_status[ent->client->pers.chosenWeapon->typeNum][ent->client->resp.team].owner = NULL;
        //gi.cprintf(ent, PRINT_HIGH, "Highlander Mode: %s selected %d, releasing %d\n", ent->client->pers.netname, weaponNum, ent->client->pers.chosenWeapon->typeNum);
        weapon_status[weaponNum][ent->client->resp.team].owner = ent;
		weaponChange = true;
    }

	if (available){
		gi.cprintf(ent, PRINT_HIGH, "Highlander Mode: That weapon is available, selecting it\n");
		gi.dprintf("Highlander Mode: That weapon is available, selecting it\n");
		weapon_status[weaponNum][ent->client->resp.team].owner = ent;
		weaponChange = true;
	}

	// The expectation is that the weapon is available always, unless it's owned by another player on your team
	if (weapon_status[weaponNum][ent->client->resp.team].owner != ent &&
	weapon_status[weaponNum][ent->client->resp.team].owner->client->resp.team == ent->client->resp.team &&
	!available) {
		gi.cprintf(ent, PRINT_HIGH, "Highlander Mode: That weapon is not available, %s owns it, select another\n", weapon_status[weaponNum][ent->client->resp.team].owner->client->pers.netname);
		gi.cprintf(weapon_status[weaponNum][ent->client->resp.team].owner, PRINT_HIGH, "%s wants your weapon!\n", playername);
		return false;
	}


	// If the player already selected this weapon, unequip it
    if (ent->client->pers.chosenWeapon != NULL && ent->client->pers.chosenWeapon->typeNum == weaponNum) {
		gi.cprintf(ent, PRINT_HIGH, "Highlander Mode: Unequipping weapon to make it available to others\n");
		weapon_status[ent->client->pers.chosenWeapon->typeNum][ent->client->resp.team].owner = NULL;
		UnequipWeapon(ent);
    }

	if (weaponChange){

		// Attempt to auto-refresh the menu:

		for (int i = 0; i < maxclients->value; i++) {
		edict_t *ent = g_edicts + 1 + i;

		if (!ent->inuse || !ent->client || ent->is_bot)
			continue;

		if (ent->client->curr_menu == MENU_WEAPONS) {
			gi.dprintf("Refreshing menu for %s, they were in menu %d\n", ent->client->pers.netname, ent->client->curr_menu);
			PMenu_Close(ent);
			PMenu_Open(ent, weapmenu, 4, sizeof(weapmenu) / sizeof(pmenu_t));
			}
		}
		return true;
	}

    return false;
}

/* 
Simple function that just returns the opposite team number
Obviously, do not use this for 3team functions
*/
int OtherTeam(int teamNum)
{
	if (teamNum < 0 || teamNum > TEAM2) {
		gi.dprintf("OtherTeam() was called but parameter supplied is not 1 or 2");
		return 0;
	}

	if (teamNum == 1)
		return TEAM2;
	else
		return TEAM1;

	// Returns zero if teamNum is not 1 or 2
	return 0;
}