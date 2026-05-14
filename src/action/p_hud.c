//-----------------------------------------------------------------------------
// p_hud.c
//
// $Id: p_hud.c,v 1.8 2002/01/24 02:24:56 deathwatch Exp $
//
//-----------------------------------------------------------------------------
// $Log: p_hud.c,v $
// Revision 1.8  2002/01/24 02:24:56  deathwatch
// Major update to Stats code (thanks to Freud)
// new cvars:
// stats_afterround - will display the stats after a round ends or map ends
// stats_endmap - if on (1) will display the stats scoreboard when the map ends
//
// Revision 1.7  2002/01/02 01:18:24  deathwatch
// Showing health icon when bandaging (thanks to Dome for submitting this code)
//
// Revision 1.6  2001/09/28 13:48:35  ra
// I ran indent over the sources. All .c and .h files reindented.
//
// Revision 1.5  2001/08/20 00:41:15  slicerdw
// Added a new scoreboard for Teamplay with stats ( when map ends )
//
// Revision 1.4  2001/07/16 19:02:06  ra
// Fixed compilerwarnings (-g -Wall).  Only one remains.
//
// Revision 1.3  2001/05/31 16:58:14  igor_rock
// conflicts resolved
//
// Revision 1.2.2.3  2001/05/25 18:59:52  igor_rock
// Added CTF Mode completly :)
// Support for .flg files is still missing, but with "real" CTF maps like
// tq2gtd1 the ctf works fine.
// (I hope that all other modes still work, just tested DM and teamplay)
//
// Revision 1.2.2.2  2001/05/20 18:54:19  igor_rock
// added original ctf code snippets from zoid. lib compilesand runs but
// doesn't function the right way.
// Jsut committing these to have a base to return to if something wents
// awfully wrong.
//
// Revision 1.2.2.1  2001/05/20 15:17:31  igor_rock
// removed the old ctf code completly
//
// Revision 1.2  2001/05/11 16:07:26  mort
// Various CTF bits and pieces...
//
// Revision 1.1.1.1  2001/05/06 17:25:15  igor_rock
// This is the PG Bund Edition V1.25 with all stuff laying around here...
//
//-----------------------------------------------------------------------------

#include "g_local.h"

// Red team colors
int red_team_red = 220;
int red_team_green = 60;
int red_team_blue = 60;
int alt_red_team_red = 110;
int alt_red_team_green = 45;
int alt_red_team_blue = 45;

// Blue team colors
int blue_team_red = 40;
int blue_team_green = 40;
int blue_team_blue = 220;
int alt_blue_team_red = 30;
int alt_blue_team_green = 60;
int alt_blue_team_blue = 110;

// Green team colors
int green_team_red = 40;
int green_team_green = 220;
int green_team_blue = 40;
int alt_green_team_red = 30;
int alt_green_team_green = 140;
int alt_green_team_blue = 30;
/*
  ======================================================================
  
  INTERMISSION
  
  ======================================================================
*/

void MoveClientToIntermission(edict_t *ent)
{
	PMenu_Close(ent);

	ent->client->layout = LAYOUT_SCORES;
	VectorCopy(level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0] * 8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1] * 8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2] * 8;
	VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	VectorClear(ent->client->ps.kick_angles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
	ent->client->ps.stats[STAT_FLASHES] = 0;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_framenum = 0;

	ent->watertype = 0;
	ent->waterlevel = 0;
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;
	ent->s.effects = 0;
	ent->s.renderfx = 0;
	ent->s.sound = 0;
	ent->s.event = 0;
	ent->s.solid = 0;
	ent->solid = SOLID_NOT;
	ent->svflags = SVF_NOCLIENT;

	ent->client->resp.sniper_mode = SNIPER_1X;
	ent->client->desired_fov = 90;
	ent->client->ps.fov = 90;
	ent->client->ps.stats[STAT_SNIPER_ICON] = 0;
	ent->client->pickup_msg_framenum = 0;

#ifndef NO_BOTS
	if( ent->is_bot )
		return;
#endif
	// add the layout
	DeathmatchScoreboardMessage(ent, NULL);
	gi.unicast(ent, true);
}

void BeginIntermission(edict_t *targ)
{
	int i;
	edict_t *ent;

	// Clear all timed messages
	free(timedMessages);

	if (level.intermission_framenum)
		return;			// already activated

	level.intermission_framenum = level.realFramenum;

	if (ctf->value) {
		CTFCalcScores();
	} else if (teamplay->value) {
		TallyEndOfLevelTeamScores();
	}

	// Generates stats for non-CTF, Teamplay or Matchmode
	if (!matchmode->value) {
		LOG_MATCH(); // Generates end of game stats
		LOG_END_MATCH_STATS(); // Generates end of match stats
	}
	CALL_DISCORD_WEBHOOK(DM_MATCH_END_MSG, MATCH_END_MSG, AWARD_NONE);

	// respawn any dead clients
	for (i = 0, ent = g_edicts + 1; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;
		if (ent->health <= 0)
			respawn(ent);
	}

	level.changemap = targ->map;
	level.intermission_exit = 0;

	// find an intermission spot
	ent = G_Find(NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{				// the map creator forgot to put in an intermission point...
		ent = G_Find(NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{				// chose one of four spots
		i = rand () & 3;
		while (i--)
		{
			ent = G_Find(ent, FOFS(classname), "info_player_intermission");
			if (!ent)		// wrap around the list
				ent = G_Find(ent, FOFS(classname), "info_player_intermission");
		}
	}

	if (ent) {
		VectorCopy( ent->s.origin, level.intermission_origin );
		VectorCopy( ent->s.angles, level.intermission_angle );
	}

	// move all clients to the intermission point
	for (i = 0, ent = g_edicts + 1; i < game.maxclients; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		MoveClientToIntermission(ent);
	}

	InitTransparentList();
}

/*
  ======================================================================
  Point of Interest
  
  This isn't perfect, but it gets you in the 'area' of a point of interest
  Could use some refactoring, but it works for now.
  Works much like intermission, but it more or less 'teleports' spectators
  and those who are not 'spawned in' to the point of interest.

  Points of interest are generally on the ground, so this adds 25 units
  to each axis to move the camera in the area without being directly 
  on top of the point of interest.
  ======================================================================
*/
void MoveClientToPOI(edict_t *ent, edict_t *poi)
{
    PMenu_Close(ent);

    VectorCopy(level.poi_origin, ent->s.origin);
    ent->s.origin[0] -= 25;
    ent->s.origin[1] -= 25;
    ent->s.origin[2] += 25;

    ent->client->ps.pmove.origin[0] = (level.poi_origin[0] - 25) * 8;
    ent->client->ps.pmove.origin[1] = (level.poi_origin[1] - 25) * 8;
    ent->client->ps.pmove.origin[2] = (level.poi_origin[2] + 25) * 8;

    vec3_t ownerv, o, ownerv_forward, ownerv_right;
    vec3_t angles;

    VectorCopy(level.poi_origin, ownerv);
    ownerv[2] += poi->viewheight;

    VectorCopy(ent->client->ps.viewangles, angles);
    AngleVectors(angles, ownerv_forward, ownerv_right, NULL);

    VectorNormalize(ownerv_forward);
    VectorMA(ownerv, -75, ownerv_forward, o);

    VectorCopy(o, ent->s.origin);
    VectorCopy(o, ent->client->ps.pmove.origin);

    VectorSubtract(level.poi_origin, ent->s.origin, ent->client->ps.viewangles);
    vectoangles(ent->client->ps.viewangles, ent->client->ps.viewangles);

    VectorClear(ent->client->ps.kick_angles);
    ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;
	ent->client->ps.stats[STAT_FLASHES] = 0;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_framenum = 0;

	ent->watertype = 0;
	ent->waterlevel = 0;
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;
	ent->s.effects = 0;
	ent->s.renderfx = 0;
	ent->s.sound = 0;
	ent->s.event = 0;
	ent->s.solid = 0;
	ent->solid = SOLID_NOT;
	ent->svflags = SVF_NOCLIENT;

	ent->client->resp.sniper_mode = SNIPER_1X;
	ent->client->desired_fov = 90;
	ent->client->ps.fov = 90;
	ent->client->ps.stats[STAT_SNIPER_ICON] = 0;
	ent->client->pickup_msg_framenum = 0;

#ifndef NO_BOTS
	if( ent->is_bot )
		return;
#endif
}

/*
  ==================
  DeathmatchScoreboardMessage
  
  ==================
*/
void DeathmatchScoreboardMessage (edict_t * ent, edict_t * killer)
{
	char entry[128];
	char string[1024];
	int stringlength;
	int i, j, totalClients;
	gclient_t *sortedClients[MAX_CLIENTS];
	int x, y;
	gclient_t *cl;
	edict_t *cl_ent;
	char *tag;

#ifndef NO_BOTS
	if (ent->is_bot)
		return;
#endif

	if (teamplay->value && !use_tourney->value)
	{
		// DW: If the map ends
		if (level.intermission_framenum) {
			if (stats_endmap->value && (gameSettings & GS_ROUNDBASED)) // And we are to show the stats screen
				A_ScoreboardEndLevel(ent, killer); // do it
			else																	// otherwise
				A_ScoreboardMessage(ent, killer);	// show the original
		}
		else
			A_ScoreboardMessage(ent, killer);
		return;
	}

	totalClients = G_SortedClients(sortedClients);

	// print level name and exit rules
	string[0] = 0;
	stringlength = 0;

	// add the clients in sorted order
	if (totalClients > 12)
		totalClients = 12;

	for (i = 0; i < totalClients; i++)
	{
		cl = sortedClients[i];
		cl_ent = g_edicts + 1 + (cl - game.clients);

		x = (i >= 6) ? 160 : 0;
		y = 32 + 32 * (i % 6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Q_snprintf (entry, sizeof (entry),
				"xv %i yv %i picn %s ", x + 32, y, tag);
			j = strlen (entry);
			if (stringlength + j > 1023)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// Convert ping to string so we can report pings as BOT if needed
		char pingstr[8];
		Q_snprintf(pingstr, sizeof(pingstr), "%d", min(cl->ping, 999));

		#ifndef NO_BOTS
		if (IS_BOT(cl_ent)) {
			if (!bot_reportasclient->value || !bot_reportpings->value) {
				Q_snprintf(pingstr, sizeof(pingstr), "BOT");
			}
		}
		#endif

		// send the layout
		Q_snprintf (entry, sizeof (entry),
			"client %i %i %i %i %s %i ",
			x, y, (int)(cl - game.clients), cl->resp.score,
			pingstr,
			(level.framenum - cl->resp.enterframe) / 600 / FRAMEDIV);
		j = strlen (entry);
		if (stringlength + j > 1023)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
  ==================
  DeathmatchScoreboard
  
  Draw instead of help message.
  Note that it isn't that hard to overflow the 1400 byte message limit!
  ==================
*/
void DeathmatchScoreboard(edict_t *ent)
{
#ifndef NO_BOTS
	if( ent->is_bot )
		return;
#endif
	DeathmatchScoreboardMessage(ent, ent->enemy);
	gi.unicast(ent, true);
}


/*
  ==================
  Cmd_Score_f
  
  Display the scoreboard
  ==================
*/
void Cmd_Score_f(edict_t *ent)
{
	ent->client->showinventory = false;
	
	if (ent->client->layout == LAYOUT_MENU)
		PMenu_Close(ent);
	
	if (ent->client->layout == LAYOUT_SCORES)
	{
		if (teamplay->value) {	// toggle scoreboards...
			ent->client->layout = LAYOUT_SCORES2;
			DeathmatchScoreboard(ent);
			return;
		}	
		ent->client->layout = LAYOUT_NONE;
		return;
	}

	if (ent->client->layout == LAYOUT_SCORES2) {
		ent->client->layout = LAYOUT_NONE;
		return;
	}
	
	ent->client->layout = LAYOUT_SCORES;
	DeathmatchScoreboard(ent);
}


/*
  ==================
  Cmd_Help_f
  
  Display the current help message
  ==================
*/
void Cmd_Help_f (edict_t * ent)
{
	// this is for backwards compatability
	Cmd_Score_f (ent);
}


//=======================================================================

/*
  ===============
  G_SetStats
  
  Rearranged for chase cam support -FB
  ===============
*/
void G_SetStats (edict_t * ent)
{
	gitem_t *item;

	if (jump->value)
	{
		Jmp_SetStats(ent);
		return;
	}

	if (!ent->client->chase_mode)
	{
		int icons[ 6 ], numbers[ 2 ], icon_count, i;
		int cycle = hud_items_cycle->value * FRAMEDIV;
		int weapon_ids[ 6 ] = { SNIPER_NUM, M4_NUM, MP5_NUM, M3_NUM, HC_NUM, DUAL_NUM };
		int s_item_ids[ 6 ] = { KEV_NUM, HELM_NUM, BAND_NUM, SIL_NUM, SLIP_NUM, LASER_NUM };

		//
		// health
		//
		ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
		ent->client->ps.stats[STAT_HEALTH] = ent->health;

		//
		// ammo (now clips really)
		//
		// zucc modified this to do clips instead
		if (!ent->client-> ammo_index
			/* || !ent->client->inventory[ent->client->ammo_index] */ )
		{
			ent->client->ps.stats[STAT_CLIP_ICON] = 0;
			ent->client->ps.stats[STAT_CLIP] = 0;
		}
		else
		{
			item = &itemlist[ent->client->ammo_index];
			if (item->typeNum < AMMO_MAX)
				ent->client->ps.stats[STAT_CLIP_ICON] = level.pic_items[item->typeNum];
			else
				ent->client->ps.stats[STAT_CLIP_ICON] = gi.imageindex(item->icon);
			ent->client->ps.stats[STAT_CLIP] = ent->client->inventory[ent->client->ammo_index];
		}

		// zucc display special item and special weapon
		// Raptor007: Modified to rotate through all carried special weapons and items.

		icon_count = 0;
		for( i = 0; i < 6; i ++ )
		{
			if( INV_AMMO( ent, weapon_ids[i] ) )
				icons[ icon_count ++ ] = level.pic_items[ weapon_ids[i] ];
		}
		if( icon_count && ! cycle )
			icon_count = 1;
		if( icon_count )
			ent->client->ps.stats[STAT_WEAPONS_ICON] = icons[ (level.framenum/cycle) % icon_count ];
		else
			ent->client->ps.stats[STAT_WEAPONS_ICON] = 0;

		icon_count = 0;
		for( i = 0; i < 6; i ++ )
		{
			if( INV_AMMO( ent, s_item_ids[i] ) )
				icons[ icon_count ++ ] = level.pic_items[ s_item_ids[i]];
		}
		if( icon_count && ! cycle )
			icon_count = 1;
		if( icon_count )
			ent->client->ps.stats[STAT_ITEMS_ICON] = icons[ ((level.framenum+cycle/2)/cycle) % icon_count ];
		else
			ent->client->ps.stats[STAT_ITEMS_ICON] = 0;

		// grenades remaining
		icon_count = 0;
		numbers[ icon_count ] = INV_AMMO( ent, GRENADE_NUM );
		if( numbers[ icon_count ] )
			icons[ icon_count ++ ] = level.pic_weapon_ammo[GRENADE_NUM];
		// MedKit
		numbers[ icon_count ] = ent->client->medkit;
		if( numbers[ icon_count ] )
			icons[ icon_count ++ ] = level.pic_health;
		// Cycle grenades and medkit if player has both.
		if( icon_count && ! cycle )
			icon_count = 1;
		if( icon_count )
		{
			int index = ((level.framenum+cycle/4)/cycle) % icon_count;
			ent->client->ps.stats[STAT_GRENADE_ICON] = icons[ index ];
			ent->client->ps.stats[STAT_GRENADES]     = numbers[ index ];
		}
		else
		{
			ent->client->ps.stats[STAT_GRENADE_ICON] = 0;
			ent->client->ps.stats[STAT_GRENADES]     = 0;
		}

		//
		// ammo by weapon
		// 
		//
		if (ent->client->weapon && ent->client->curr_weap)
		{
			switch (ent->client->curr_weap) {
			case MK23_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->mk23_rds;
				break;
			case MP5_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->mp5_rds;
				break;
			case M4_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->m4_rds;
				break;
			case M3_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->shot_rds;
				break;
			case HC_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->cannon_rds;
				break;
			case SNIPER_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->sniper_rds;
				break;
			case DUAL_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = ent->client->dual_rds;
				break;
			case KNIFE_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = INV_AMMO(ent, KNIFE_NUM);
				break;
			case GRENADE_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = level.pic_weapon_ammo[ent->client->curr_weap];
				ent->client->ps.stats[STAT_AMMO] = INV_AMMO(ent, GRENADE_NUM);
				break;
			case GRAPPLE_NUM:
				ent->client->ps.stats[STAT_AMMO_ICON] = 0;
				ent->client->ps.stats[STAT_AMMO] = 0;
				break;
			default:
				gi.dprintf ("Failed to find hud weapon/icon for num %d.\n", ent->client->curr_weap);
				break;
			}
		} else {
			ent->client->ps.stats[STAT_AMMO_ICON] = 0;
			ent->client->ps.stats[STAT_AMMO] = 0;
		}

		//
		// sniper mode icons
		//
		//if ( ent->client->sniper_mode )
		//      gi.cprintf (ent, PRINT_HIGH, "Sniper Zoom set at %d.\n", ent->client->sniper_mode);


		if (ent->client->resp.sniper_mode == SNIPER_1X
			|| ent->client->weaponstate == WEAPON_RELOADING
			|| ent->client->weaponstate == WEAPON_BUSY
			|| ent->client->no_sniper_display
			|| ! IS_ALIVE(ent) )
			ent->client->ps.stats[STAT_SNIPER_ICON] = 0;
		else
			ent->client->ps.stats[STAT_SNIPER_ICON] = level.pic_sniper_mode[ent->client->resp.sniper_mode];

		//
		// armor
		//
		//ent->client->ps.stats[STAT_ARMOR_ICON] = 0; // Replaced with STAT_TEAM_ICON.
		ent->client->ps.stats[STAT_ARMOR] = 0;

		//
		// timers
		//
		// if (ent->client->quad_framenum > level.framenum)
		// {
		// 	ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		// 	ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum) / HZ;
		// }
		// else if (ent->client->invincible_framenum > level.framenum)
		// {
		// 	ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		// 	ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum) / HZ;
		// }
		// else if (ent->client->enviro_framenum > level.framenum)
		// {
		// 	ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		// 	ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum) / HZ;
		// }
		// else if (ent->client->breather_framenum > level.framenum)
		// {
		// 	ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		// 	ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum) / HZ;
		// }
		// else
		// if (esp->value) {
		// 	if (!IS_LEADER(ent) && ent->client->respawn_framenum > 0 && ent->client->respawn_framenum > level.framenum > 0){
		// 		ent->client->ps.stats[STAT_TIMER_ICON] = level.pic_esp_respawn_icon;
		// 		ent->client->ps.stats[STAT_TIMER] = (ent->client->respawn_framenum - level.framenum) / HZ;
		// 	} else {
		// 		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		// 		ent->client->ps.stats[STAT_TIMER] = 0;
		// 	}
		// }
		// else
		// {
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
		//}

		//
		// selected item
		//
		if (ent->client->selected_item < 1) {
			ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
		} else {
			item = &itemlist[ent->client->selected_item];
			if (item->typeNum < AMMO_MAX)
				ent->client->ps.stats[STAT_SELECTED_ICON] = level.pic_items[item->typeNum];
			else
				ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(item->icon);
		}

		ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->selected_item;

		//
		// bandaging icon / current weapon if not shown
		//
		// TNG: Show health icon when bandaging (thanks to Dome for this code)
		if (ent->client->weaponstate == WEAPON_BANDAGING || ent->client->bandaging || ent->client->bandage_stopped)
			ent->client->ps.stats[STAT_HELPICON] = level.pic_health;
		else if ((ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91) && ent->client->weapon)
			ent->client->ps.stats[STAT_HELPICON] = level.pic_items[ent->client->weapon->typeNum];
		else
			ent->client->ps.stats[STAT_HELPICON] = 0;
		
		// Hide health, ammo, weapon, and bandaging state when free spectating.
		if( ! IS_ALIVE(ent) )
		{
			ent->client->ps.stats[STAT_HEALTH_ICON] = 0;
			ent->client->ps.stats[STAT_AMMO_ICON] = 0;
			ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
			ent->client->ps.stats[STAT_HELPICON] = 0;
		}
		
		// Team icon.
		if( teamplay->value && hud_team_icon->value && (ent->client->resp.team != NOTEAM) && IS_ALIVE(ent) )
			ent->client->ps.stats[STAT_TEAM_ICON] = level.pic_teamskin[ent->client->resp.team];
		else
			ent->client->ps.stats[STAT_TEAM_ICON] = 0;
	}

	//
	// pickup message
	//
	if (level.realFramenum > ent->client->pickup_msg_framenum)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (level.intermission_framenum || ent->client->layout)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	if (ent->client->showinventory && ent->health > 0)
		ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	if (level.intermission_framenum) {
		ent->client->ps.stats[STAT_SNIPER_ICON] = 0;
		ent->client->ps.stats[STAT_HELPICON] = 0;
		ent->client->ps.stats[STAT_ID_VIEW] = 0;
	} else {
		SetIDView(ent);
	}

	//FIREBLADE
	if (ctf->value)
		SetCTFStats (ent);
	else if (dom->value)
		SetDomStats (ent);
	else if (esp->value)
		SetEspStats (ent);
	else if (teamplay->value)
		A_Scoreboard (ent);
	//FIREBLADE
}


#ifdef AQTION_EXTENSION

void HUD_SetType(edict_t *clent, int type)
{
	if (clent->client->resp.hud_type == type)
		return;
	
	if (type == -1)
		HUD_ClientSetup(clent);
	else if (type == 1)
		HUD_SpectatorSetup(clent);
	else
	{
		Ghud_ClearForClient(clent);
		clent->client->resp.hud_type = 0;
	}
}

void HUD_ClientSetup(edict_t *clent)
{
	Ghud_ClearForClient(clent);
	clent->client->resp.hud_type = -1;

}

void HUD_ClientUpdate(edict_t *clent)
{

}
typedef enum {
    tm = 0,
    mm = 1,
    ts = 2,
    ss = 3
} hud_time_digits;

static int GetRemainingTimeDigits(hud_time_digits timeval)
{
    int remaining = 0, rmins = 0, rsecs = 0, gametime = 0;

    gametime = level.matchTime;

    remaining = (timelimit->value * 60) - gametime;

    rmins = remaining / 60;
    rsecs = remaining % 60;

	// Ensure rmins and rsecs are non-negative
    if (rmins < 0) {
        rmins = 0;
    }
    if (rsecs < 0) {
        rsecs = 0;
    }

    switch (timeval) {
        case tm: // Tens of minutes
            return (rmins / 10) % 10;
        case mm: // Minutes
            return rmins % 10;
        case ts: // Tens of seconds
            return (rsecs / 10) % 10;
        case ss: // Seconds
            return rsecs % 10;
        default:
            return 0; // Invalid timeval
    }
}

// Returns the number of decimal digits needed to display val (minimum 1).
// Mirrors the client renderer's auto-numsize: floor(log10(val))+1.
static int ScoreDigitCount(int val)
{
	if (val < 10)   return 1;
	if (val < 100)  return 2;
	if (val < 1000) return 3;
	return 4;
}

// Returns the pos_x for the team-2 score element (anchor 1,0) so that the
// ones digit always ends 4 px left of the icon at x=-28, regardless of how
// many digits are in the score.  Each additional high-order digit shifts the
// number 16 px further left (one DIGIT_WIDTH step).
static int T2ScorePosX(int digits)
{
	return -50 - 16 * (digits - 1);
}

static void HUD_UpdateTeamScores(edict_t *clent)
{
	int *hud = clent->client->resp.hud_items;
	int score2, n;

	// Team 1 (left edge anchor, anchor 0,0) — left-aligned, grows right.
	// size[0]=0 (auto-numsize) keeps the left edge of the number fixed at
	// pos_x+2=32; the ones digit shifts one slot right for each new digit place.
	Ghud_SetFlags(clent, hud[h_team_l], 0);
	Ghud_SetFlags(clent, hud[h_team_l_num], 0);
	Ghud_SetInt(clent, hud[h_team_l_num], ctf->value ? ctfgame.team1 : teams[TEAM1].score);

	// Team 2 (right edge anchor, anchor 1,0) — ones digit fixed, grows left.
	// Shift pos_x one DIGIT_WIDTH step left for each additional digit so the
	// ones digit always ends at the same screen position next to the icon.
	Ghud_SetFlags(clent, hud[h_team_r], 0);
	Ghud_SetFlags(clent, hud[h_team_r_num], 0);
	score2 = ctf->value ? ctfgame.team2 : teams[TEAM2].score;
	n = ScoreDigitCount(score2);
	Ghud_SetPosition(clent, hud[h_team_r_num], T2ScorePosX(n), 28);
	Ghud_SetInt(clent, hud[h_team_r_num], score2);
}

static void HUD_UpdateSpectatorTimer(edict_t *clent)
{
	int *hud = clent->client->resp.hud_items;

    // Update the HUD elements with the individual digits
	Ghud_SetInt(clent, hud[h_spectator_time_tm], GetRemainingTimeDigits(tm));
	Ghud_SetInt(clent, hud[h_spectator_time_mm], GetRemainingTimeDigits(mm));
	Ghud_SetInt(clent, hud[h_spectator_time_ts], GetRemainingTimeDigits(ts));
	Ghud_SetInt(clent, hud[h_spectator_time_ss], GetRemainingTimeDigits(ss));

	if (GetRemainingTimeDigits(tm) == 0 && GetRemainingTimeDigits(mm) < 1) {
		// Change bar color to red
		Ghud_SetColor(clent, hud[h_spectator_timer_border], 255, 0, 0, 120);
	} else if (GetRemainingTimeDigits(tm) == 0 && GetRemainingTimeDigits(mm) < 3){
		// Change bar color to orange
		Ghud_SetColor(clent, hud[h_spectator_timer_border], 255, 165, 0, 120);
	} else {
		// Change bar color to gray
		Ghud_SetColor(clent, hud[h_spectator_timer_border], 20, 20, 20, 120);
	}
}

void HUD_SpectatorTimerSetup(edict_t *clent)
{
	int *hud = clent->client->resp.hud_items;

    if (timelimit->value) {
        // GHUD top middle time display
        clent->client->resp.hud_type = 1;

		// Unhide the other HUD elements
		Ghud_SetFlags(clent, hud[h_spectator_timer_border], 0);
		Ghud_SetFlags(clent, hud[h_spectator_timer], 0);
		Ghud_SetFlags(clent, hud[h_spectator_time_tm], 0);
		Ghud_SetFlags(clent, hud[h_spectator_time_mm], 0);
		Ghud_SetFlags(clent, hud[h_spectator_time_sep], 0);
		Ghud_SetFlags(clent, hud[h_spectator_time_ts], 0);
		Ghud_SetFlags(clent, hud[h_spectator_time_ss], 0);
        
        // Timer box sized to fit digits only (scores moved to edge columns).
        // Digits span center±48px wide, 24px tall at y=30..54. 6px padding all sides.
        int x, y;
        x = 0;
        y = 30;

		hud[h_spectator_timer_border] = Ghud_NewElement(clent, GHT_FILL);
        Ghud_SetAnchor(clent, hud[h_spectator_timer_border], 0.5, 0);
		Ghud_SetPosition(clent, hud[h_spectator_timer_border], x - 55, y - 7);
        Ghud_SetSize(clent, hud[h_spectator_timer_border], 110, 38);
        Ghud_SetColor(clent, hud[h_spectator_timer_border], 50, 150, 50, 120);

		hud[h_spectator_timer] = Ghud_NewElement(clent, GHT_FILL);
        Ghud_SetAnchor(clent, hud[h_spectator_timer], 0.5, 0);
		Ghud_SetPosition(clent, hud[h_spectator_timer], x - 54, y - 6);
        Ghud_SetSize(clent, hud[h_spectator_timer], 108, 36);
        Ghud_SetColor(clent, hud[h_spectator_timer], 50, 50, 50, 120);

        // Add number elements for minutes
        hud[h_spectator_time_tm] = Ghud_AddNumber(clent, x - 50, y, 0);
        hud[h_spectator_time_mm] = Ghud_AddNumber(clent, x - 30, y, 0);

		// Draw timer seperator
		hud[h_spectator_time_sep] = Ghud_AddText(clent, x - 5, y + 12, ":");

		// Add number elements for seconds
        hud[h_spectator_time_ts] = Ghud_AddNumber(clent, x + 10, y, 0);
        hud[h_spectator_time_ss] = Ghud_AddNumber(clent, x + 30, y, 0);

		// Anchor everything to the top middle
		Ghud_SetAnchor(clent, hud[h_spectator_time_tm], 0.5, 0);
		Ghud_SetAnchor(clent, hud[h_spectator_time_mm], 0.5, 0);
		Ghud_SetAnchor(clent, hud[h_spectator_time_sep], 0.5, 0);
		Ghud_SetAnchor(clent, hud[h_spectator_time_ts], 0.5, 0);
		Ghud_SetAnchor(clent, hud[h_spectator_time_ss], 0.5, 0);

		} else {
			Ghud_SetFlags(clent, hud[h_spectator_time_tm], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_mm], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_sep], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_ts], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_ss], GHF_HIDE);
		}

	// GHUD team icons and scores — anchored above their respective health bar columns.
	// Left column: anchor(0,0), x=0..144. Right column: anchor(1,0), x=-144..0.
	// Score numbers sit at y=28 (24px tall digits), 4px above the first nameplate at y=56.
	// Icon (24x24) sits at x=4 (left) / x=-28 (right), score at x=30 / x=-66.

	// Team 1 — left edge anchor
	if (ctf->value) // CTF
		hud[h_team_l] = Ghud_AddIcon(clent, 4, 28, level.pic_ctf_flagbase[TEAM1], 24, 24);
	else if (esp->value) // Espionage
		hud[h_team_l] = Ghud_AddIcon(clent, 4, 28, level.pic_esp_teamicon[TEAM1], 24, 24);
	else if (matchmode->value) { // Matchmode: thin color bar across the full column
		hud[h_team_l] = Ghud_NewElement(clent, GHT_FILL);
		Ghud_SetPosition(clent, hud[h_team_l], 0, 24);
		Ghud_SetSize(clent, hud[h_team_l], 144, 4);
		Ghud_SetColor(clent, hud[h_team_l], red_team_red, red_team_green, red_team_blue, 255);
	} else // Teamplay/Domination
		hud[h_team_l] = Ghud_AddIcon(clent, 4, 28, level.pic_teamskin[TEAM1], 24, 24);
	Ghud_SetAnchor(clent, hud[h_team_l], 0, 0);

	// size[0] left at 0 (auto-numsize) so the renderer left-aligns from x=30.
	hud[h_team_l_num] = Ghud_AddNumber(clent, 30, 28, ctf->value ? ctfgame.team1 : teams[TEAM1].score);
	Ghud_SetAnchor(clent, hud[h_team_l_num], 0, 0);

	// Team 2 — right edge anchor
	if (ctf->value) // CTF
		hud[h_team_r] = Ghud_AddIcon(clent, -28, 28, level.pic_ctf_flagbase[TEAM2], 24, 24);
	else if (esp->value) // Espionage
		hud[h_team_r] = Ghud_AddIcon(clent, -28, 28, level.pic_esp_teamicon[TEAM2], 24, 24);
	else if (matchmode->value) { // Matchmode: thin color bar across the full column
		hud[h_team_r] = Ghud_NewElement(clent, GHT_FILL);
		Ghud_SetPosition(clent, hud[h_team_r], -144, 24);
		Ghud_SetSize(clent, hud[h_team_r], 144, 4);
		Ghud_SetColor(clent, hud[h_team_r], blue_team_red, blue_team_green, blue_team_blue, 255);
	} else // Teamplay/Domination
		hud[h_team_r] = Ghud_AddIcon(clent, -28, 28, level.pic_teamskin[TEAM2], 24, 24);
	Ghud_SetAnchor(clent, hud[h_team_r], 1, 0);

	{
		// Compute initial pos_x so ones digit is already correctly placed.
		int s2 = ctf->value ? ctfgame.team2 : teams[TEAM2].score;
		// size[0] left at 0 (auto-numsize) so the renderer right-aligns to pos_x.
		hud[h_team_r_num] = Ghud_AddNumber(clent, T2ScorePosX(ScoreDigitCount(s2)), 28, s2);
	}
	Ghud_SetAnchor(clent, hud[h_team_r_num], 1, 0);
}

void HUD_SpectatorStatsSetup(edict_t *clent)
{
	clent->client->resp.hud_type = 1;
	int *hud = clent->client->resp.hud_items;

	// GHUD bottom center stat display
	int x, y;
	int h_base = h_spectator_stats;
	int h_nbar = h_spectator_name_bar;
	int h_sbar = h_spectator_stats_bar;
	int h, j, k;

	// All elements are anchored from the gray bar at the bottom
	x = -160;
	y = -60;
	int bar_width = 305;
	int bar_height = 12;

	h = h_base; // gray bar
	hud[h] = Ghud_NewElement(clent, GHT_FILL);
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x, y);
	Ghud_SetSize(clent, hud[h], bar_width, bar_height);
	Ghud_SetFlags(clent, hud[h], GHF_HIDE);

	h = h_base + 1; // frags
	hud[h] = Ghud_AddText(clent, x + 500, y + 200, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 16, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	h = h_base + 2; // deaths
	hud[h] = Ghud_AddText(clent, x, y, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 72, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	h = h_base + 3; // damage
	hud[h] = Ghud_AddText(clent, x, y, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 120, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	h = h_base + 4; // accuracy
	hud[h] = Ghud_AddText(clent, x, y, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 184, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	h = h_base + 5; // accuracy awards
	hud[h] = Ghud_AddText(clent, x, y, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 248, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	h = h_base + 6; // impressive awards
	hud[h] = Ghud_AddText(clent, x, y, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 273, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	h = h_base + 7; // excellent awards
	hud[h] = Ghud_AddText(clent, x, y, "");
	Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	Ghud_SetPosition(clent, hud[h], x + 296, y + 2);
	Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

	// h = h_base + 10; // weapon selection
	// Ghud_SetAnchor(clent, hud[h], 0.5, 1);
	// hud[h] = Ghud_AddIcon(clent, x + 200, y + 200, level.pic_items[M4_NUM], 20, 20);
	

	j = h_nbar; // name bar
	hud[j] = Ghud_NewElement(clent, GHT_FILL);
	Ghud_SetAnchor(clent, hud[j], 0.5, 1);
	Ghud_SetPosition(clent, hud[j], x, y - 24);
	Ghud_SetSize(clent, hud[j], bar_width, bar_height);
	Ghud_SetFlags(clent, hud[j], GHF_HIDE);

	j = h_nbar + 1; // name print
	hud[j] = Ghud_AddText(clent, x, y + 22, "");
	Ghud_SetAnchor(clent, hud[j], 0.5, 1);
	Ghud_SetPosition(clent, hud[j], x + 140, y - 22);
	Ghud_SetSize(clent, hud[j], 20, 12);
	Ghud_SetTextFlags(clent, hud[j], UI_CENTER);

	k = h_sbar; // stat table bar
	hud[k] = Ghud_NewElement(clent, GHT_FILL);
	Ghud_SetAnchor(clent, hud[k], 0.5, 1);
	Ghud_SetPosition(clent, hud[k], x, y - 12);
	Ghud_SetSize(clent, hud[k], bar_width, bar_height);
	Ghud_SetFlags(clent, hud[k], GHF_HIDE);

	k = h_sbar + 1; // stat table text
	hud[k] = Ghud_AddText(clent, x, y - 10, "");
	Ghud_SetAnchor(clent, hud[k], 0.5, 1);
	Ghud_SetPosition(clent, hud[k], x, y - 10);
	Ghud_SetSize(clent, hud[k], 20, 12);
	Ghud_SetTextFlags(clent, hud[k], UI_LEFT);
}

void HUD_SpectatorSetup(edict_t *clent)
{
	Ghud_ClearForClient(clent);
	clent->client->resp.hud_type = 1;

	int nameplate_alpha = 230;
	
	int *hud = clent->client->resp.hud_items;
	int i;

	if (spectator_hud->value) {
		// Left-side nameplates
		if (teamplay->value) {
			for (i = 0; i < 6; i++)
			{
				int x, y, yy;
				int h_base = h_nameplate_l + (i * 5);
				int h;

				x = 0;
				y = 56;
				yy = y + (28 * i);

				h = h_base; // back bar
				hud[h] = Ghud_NewElement(clent, GHT_FILL);
				Ghud_SetPosition(clent, hud[h], x, yy);
				Ghud_SetAnchor(clent, hud[h], 0, 0);
				Ghud_SetSize(clent, hud[h], 144, 24);
				Ghud_SetColor(clent, hud[h], alt_red_team_red, alt_red_team_green, alt_red_team_blue, nameplate_alpha);

				h = h_base + 1; // health bar
				hud[h] = Ghud_NewElement(clent, GHT_FILL);
				Ghud_SetPosition(clent, hud[h], x, yy);
				Ghud_SetAnchor(clent, hud[h], 0, 0);
				Ghud_SetSize(clent, hud[h], 0, 24);
				Ghud_SetColor(clent, hud[h], red_team_red, red_team_green, red_team_blue, nameplate_alpha);

				h = h_base + 2; // name
				hud[h] = Ghud_AddText(clent, x + 142, yy + 3, "");
				Ghud_SetAnchor(clent, hud[h], 0, 0);
				Ghud_SetTextFlags(clent, hud[h], UI_RIGHT);

				h = h_base + 3; // k/d
				hud[h] = Ghud_AddText(clent, x + 142, yy + 14, "");
				Ghud_SetAnchor(clent, hud[h], 0, 0);
				Ghud_SetTextFlags(clent, hud[h], UI_RIGHT);

				h = h_base + 4; // weapon select
				hud[h] = Ghud_AddIcon(clent, x + 2, yy + 2, level.pic_items[M4_NUM], 20, 20);
				Ghud_SetAnchor(clent, hud[h], 0, 0);
			}

			// Right side nameplates
			for (i = 0; i < 6; i++)
			{
				int x, y, yy;
				int h_base = h_nameplate_r + (i * 5);
				int h;

				x = -144;
				y = 56;
				yy = y + (28 * i);

				h = h_base; // back bar
				hud[h] = Ghud_NewElement(clent, GHT_FILL);
				Ghud_SetPosition(clent, hud[h], x, yy);
				Ghud_SetAnchor(clent, hud[h], 1, 0);
				Ghud_SetSize(clent, hud[h], 144, 24);
				Ghud_SetColor(clent, hud[h], alt_blue_team_red, alt_blue_team_green, alt_blue_team_blue, nameplate_alpha);

				h = h_base + 1; // health bar
				hud[h] = Ghud_NewElement(clent, GHT_FILL);
				Ghud_SetPosition(clent, hud[h], x, yy);
				Ghud_SetAnchor(clent, hud[h], 1, 0);
				Ghud_SetSize(clent, hud[h], 0, 24);
				Ghud_SetColor(clent, hud[h], blue_team_red, blue_team_green, blue_team_blue, nameplate_alpha);

				h = h_base + 2; // name
				hud[h] = Ghud_AddText(clent, x + 2, yy + 3, "");
				Ghud_SetAnchor(clent, hud[h], 1, 0);
				Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

				h = h_base + 3; // k/d
				hud[h] = Ghud_AddText(clent, x + 2, yy + 14, "");
				Ghud_SetAnchor(clent, hud[h], 1, 0);
				Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

				h = h_base + 4; // weapon select
				hud[h] = Ghud_AddIcon(clent, x + 122, yy + 2, level.pic_items[M4_NUM], 20, 20);
				Ghud_SetAnchor(clent, hud[h], 1, 0);
			}
		}
		// GHUD chase player stats
		HUD_SpectatorStatsSetup(clent);

		// GHUD top middle time display
		HUD_SpectatorTimerSetup(clent);

		// Apply the limchasecam team visibility filter immediately after setup.
		// All nameplates are created visible by default; this hides enemy team
		// elements for dead team players (limchasecam=2) so they never appear
		// visible even for a single frame before HUD_SpectatorUpdate runs.
		HUD_SpectatorUpdate(clent);
	}
}

void HUD_SpectatorUpdate(edict_t *clent)
{
	int i;
	if (spectator_hud->value) {
		int *hud = clent->client->resp.hud_items;
		if (teamplay->value) {
			if (!(clent->client->pers.spec_flags & SPECFL_SPECHUD_NEW)) // hide all elements since client doesn't want them
			{
				for (i = 0; i <= h_spectator_time_ss; i++)
				{
					Ghud_SetFlags(clent, hud[i], GHF_HIDE);
				}
				return;
			}

			/* Determine which teams this viewer can see based on limchasecam.
			 * If limchasecam is enabled and the viewer is on a team (dead player),
			 * they should only see their own team's status, not the enemy team. */
			int viewer_team = clent->client->resp.team;
			qboolean limchase = (limchasecam->value && viewer_team != NOTEAM);
			qboolean can_see_team1 = (!limchase || viewer_team == TEAM1);
			qboolean can_see_team2 = (!limchase || viewer_team == TEAM2);

			gclient_t *team1_players[6];
			gclient_t *team2_players[6];
			gclient_t *sortedClients[MAX_CLIENTS];

			int totalClients, team1Clients, team2Clients;

			memset(team1_players, 0, sizeof(team1_players));
			memset(team2_players, 0, sizeof(team2_players));

			team1Clients = 0;
			team2Clients = 0;
			totalClients = G_SortedClients(sortedClients);

			for (i = 0; i < totalClients; i++)
			{
				gclient_t *cl = sortedClients[i];

				if (!cl->resp.team)
					continue;

				if (cl->resp.subteam)
					continue;

				if (cl->resp.team == TEAM1)
				{
					if (team1Clients >= 6)
						continue;

					team1_players[team1Clients] = cl;
					team1Clients++;
					continue;
				}
				if (cl->resp.team == TEAM2)
				{
					if (team2Clients >= 6)
						continue;

					team2_players[team2Clients] = cl;
					team2Clients++;
					continue;
				}
			}


			for (i = 0; i < 6; i++)
			{
				int x, y, yy;
				int h = h_nameplate_l + (i * 5);
				gclient_t *cl = team1_players[i];

				/* Hide Team 1 nameplates if viewer cannot see this team (limchasecam) */
				if (!cl || !can_see_team1)
				{
					Ghud_SetFlags(clent, hud[h + 0], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 1], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 2], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 3], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 4], GHF_HIDE);
					continue;
				}

				x = 0;
				y = 56;
				yy = y + (28 * i);

				// unhide our elements
				Ghud_SetFlags(clent, hud[h + 0], 0);
				Ghud_SetFlags(clent, hud[h + 1], 0);
				Ghud_SetFlags(clent, hud[h + 2], 0);
				Ghud_SetFlags(clent, hud[h + 3], 0);
				Ghud_SetFlags(clent, hud[h + 4], 0);

				// tint for deadness
				edict_t *cl_ent = g_edicts + 1 + (cl - game.clients);
				if (!IS_ALIVE(cl_ent))
				{
					Ghud_SetSize(clent, hud[h + 1], 0, 24);

					Ghud_SetPosition(clent, hud[h + 0], x, yy);
					Ghud_SetSize(clent, hud[h + 0], 144, 24);
				}
				else
				{
					float hp_invfrac;
					float hp_frac = bound(0, ((float)cl_ent->health / 100.0f), 1);
					hp_invfrac = floorf((1 - hp_frac) * 144) / 144;
					hp_frac = ceilf(hp_frac * 144) / 144; // round so we don't get gaps

					Ghud_SetSize(clent, hud[h + 1], 144 * hp_frac, 24);
					Ghud_SetSize(clent, hud[h + 0], 144 * hp_invfrac, 24);
					Ghud_SetPosition(clent, hud[h + 0], x + (144 * (hp_frac)), yy);
				}

				// generate strings
				char nm_s[17];
				char kdr_s[24];
				memcpy(nm_s, cl->pers.netname, 16);
				if (IS_ALIVE(cl_ent))
					snprintf(kdr_s, sizeof(kdr_s), "%i", cl->resp.kills);
				else
					snprintf(kdr_s, sizeof(kdr_s), "(%i)%c    %5i", cl->resp.deaths, 06, cl->resp.kills);

				nm_s[sizeof(nm_s) - 1] = 0; // make sure strings are terminated
				kdr_s[23] = 0;

				// update fields
				Ghud_SetText(clent, hud[h + 2], nm_s);
				Ghud_SetText(clent, hud[h + 3], kdr_s);

				int weapNum = cl->pers.chosenWeapon ? cl->pers.chosenWeapon->typeNum : 0;

				if (IS_ALIVE(cl_ent) && cl->curr_weap)
					Ghud_SetInt(clent, hud[h + 4], level.pic_items[cl->curr_weap]);
				else if (cl->resp.team == TEAM1 && weapNum)
					Ghud_SetInt(clent, hud[h + 4], level.pic_items[weapNum]);
				else // no weapon, set to mk23
					Ghud_SetInt(clent, hud[h + 4], level.pic_items[MK23_NUM]);
			}

			for (i = 0; i < 6; i++)
			{
				int x, y, yy;
				int h = h_nameplate_r + (i * 5);
				gclient_t *cl = team2_players[i];

				/* Hide Team 2 nameplates if viewer cannot see this team (limchasecam) */
				if (!cl || !can_see_team2)
				{
					Ghud_SetFlags(clent, hud[h + 0], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 1], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 2], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 3], GHF_HIDE);
					Ghud_SetFlags(clent, hud[h + 4], GHF_HIDE);
					continue;
				}

				x = -144;
				y = 56;
				yy = y + (28 * i);

				// unhide our elements
				Ghud_SetFlags(clent, hud[h + 0], 0);
				Ghud_SetFlags(clent, hud[h + 1], 0);
				Ghud_SetFlags(clent, hud[h + 2], 0);
				Ghud_SetFlags(clent, hud[h + 3], 0);
				Ghud_SetFlags(clent, hud[h + 4], 0);

				// tint for deadness
				edict_t *cl_ent = g_edicts + 1 + (cl - game.clients);
				if (!IS_ALIVE(cl_ent))
				{
					Ghud_SetSize(clent, hud[h + 1], 0, 24);
					Ghud_SetSize(clent, hud[h + 0], 144, 24);
					//Ghud_SetPosition(clent, hud[h + 0], x, y);
				}
				else
				{
					float hp_invfrac;
					float hp_frac = bound(0, ((float)cl_ent->health / 100.0f), 1);
					hp_invfrac = floorf((1 - hp_frac) * 144) / 144;
					hp_frac = ceilf(hp_frac * 144) / 144; // round so we don't get gaps

					Ghud_SetSize(clent, hud[h + 1], 144 * hp_frac, 24);
					Ghud_SetSize(clent, hud[h + 0], 144 * hp_invfrac, 24);
					Ghud_SetPosition(clent, hud[h + 1], x + (144 * hp_invfrac), yy);
					//Ghud_SetPosition(clent, hud[h + 0], x + (144 * (hp_frac)), y);
				}

				// generate strings
				char nm_s[17];
				char kdr_s[24];
				memcpy(nm_s, cl->pers.netname, 16);
				if (IS_ALIVE(cl_ent))
					snprintf(kdr_s, sizeof(kdr_s), "%i", cl->resp.kills);
				else
					snprintf(kdr_s, sizeof(kdr_s), "%-5i    %c(%i)", cl->resp.kills, 06, cl->resp.deaths);

				nm_s[sizeof(nm_s) - 1] = 0; // make sure strings are terminated
				kdr_s[23] = 0;

				// update fields
				Ghud_SetText(clent, hud[h + 2], nm_s);
				Ghud_SetText(clent, hud[h + 3], kdr_s);

				int weapNum = cl->pers.chosenWeapon ? cl->pers.chosenWeapon->typeNum : 0;

				if (IS_ALIVE(cl_ent) && cl->curr_weap)
					Ghud_SetInt(clent, hud[h + 4], level.pic_items[cl->curr_weap]);
				else if (cl->resp.team == TEAM2 && weapNum)
					Ghud_SetInt(clent, hud[h + 4], level.pic_items[weapNum]);
				else // no weapon, set to mk23
					Ghud_SetInt(clent, hud[h + 4], level.pic_items[MK23_NUM]);
			}
		}

		int h_base = h_spectator_stats;
		int h_nbar = h_spectator_name_bar;
		int h_sbar = h_spectator_stats_bar;
		// If we're chasing a target, display their stats

		//gi.dprintf("chase target: %s\n", clent->client->chase_target ? clent->client->chase_target->client->pers.netname : "none");
		//gi.dprintf("chase target mode: %i\n", clent->client->chase_mode);
		if (clent->client->chase_target) {
			// GHUD bottom center stat display
			Ghud_SetFlags(clent, hud[h_spectator_stats], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 1], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 2], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 3], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 4], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 5], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 6], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 7], 0);
			Ghud_SetFlags(clent, hud[h_spectator_name_bar], 0);
			Ghud_SetFlags(clent, hud[h_spectator_name_bar + 1], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats_bar], 0);
			Ghud_SetFlags(clent, hud[h_spectator_stats_bar + 1], 0);

			char nm_s[17];
			char frags_s[24];
			char deaths_s[24];
			char dmg_s[24];
			char acc_s[10];
			char a_s[10];
			char i_s[10];
			char e_s[10];
			edict_t *targ = clent->client->chase_target;
			memcpy(nm_s, targ->client->pers.netname, 16);
			snprintf(frags_s, sizeof(frags_s), "%i", targ->client->resp.kills);
			snprintf(deaths_s, sizeof(deaths_s), "%i", targ->client->resp.deaths);
			snprintf(dmg_s, sizeof(dmg_s), "%i", targ->client->resp.damage_dealt);
			snprintf(acc_s, sizeof(acc_s), "%.2f%%", CalculateAccuracy(targ));
			snprintf(a_s, sizeof(a_s), "%i", targ->client->resp.awardstats[ACCURACY]);
			snprintf(i_s, sizeof(i_s), "%i", targ->client->resp.awardstats[IMPRESSIVE]);
			snprintf(e_s, sizeof(e_s), "%i", targ->client->resp.awardstats[EXCELLENT]);
			// update fields

			// Change color based on team
			int nameplate_alpha = 180;

			// target name
			Ghud_SetText(clent, hud[h_nbar + 1], nm_s);
			// target stat bar
			Ghud_SetText(clent, hud[h_sbar + 1], "Frags  Deaths  Damage  Acc.    \x07  \x0E  \x0F");
			// target stat values
			Ghud_SetText(clent, hud[h_base + 1], frags_s);
			Ghud_SetText(clent, hud[h_base + 2], deaths_s);
			Ghud_SetText(clent, hud[h_base + 3], dmg_s);
			Ghud_SetText(clent, hud[h_base + 4], acc_s);
			Ghud_SetText(clent, hud[h_base + 5], a_s);
			Ghud_SetText(clent, hud[h_base + 6], i_s);
			Ghud_SetText(clent, hud[h_base + 7], e_s);

			// Color per team
			if (clent->client->chase_target) {
				if (targ->client->resp.team == TEAM1) {
					Ghud_SetColor(clent, hud[h_nbar], red_team_red, red_team_green, red_team_blue, nameplate_alpha);
					Ghud_SetColor(clent, hud[h_sbar], alt_red_team_red, alt_red_team_green, alt_red_team_blue, 255);
					Ghud_SetColor(clent, hud[h_base], 100, 100, 100, 255);
				} else if (targ->client->resp.team == TEAM2) {
					Ghud_SetColor(clent, hud[h_nbar], blue_team_red, blue_team_green, blue_team_blue, nameplate_alpha);
					Ghud_SetColor(clent, hud[h_sbar], alt_blue_team_red, alt_blue_team_green, alt_blue_team_blue, 255);
					Ghud_SetColor(clent, hud[h_base], 100, 100, 100, 255);
				} else if (targ->client->resp.team == TEAM3) {
					Ghud_SetColor(clent, hud[h_nbar], green_team_red, green_team_green, green_team_blue, nameplate_alpha);
					Ghud_SetColor(clent, hud[h_sbar], alt_green_team_red, alt_green_team_green, alt_green_team_blue, 255);
					Ghud_SetColor(clent, hud[h_base], 100, 100, 100, 255);
				} else if (targ->client->resp.team == 0) {  // Deathmatch!
					Ghud_SetColor(clent, hud[h_nbar], 220, 220, 220, nameplate_alpha);
					Ghud_SetColor(clent, hud[h_sbar], 110, 110, 110, 255);
					Ghud_SetColor(clent, hud[h_base], 150, 150, 150, 255);
				}
			}
		} else {  // Hide the stats if we're not chasing a target
			Ghud_SetFlags(clent, hud[h_spectator_stats], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 1], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 2], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 3], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 4], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 5], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 6], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats + 7], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_name_bar], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_name_bar + 1], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats_bar], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_stats_bar + 1], GHF_HIDE);
		}

		// Update team scores unconditionally every frame
		if (teamplay->value)
			HUD_UpdateTeamScores(clent);

		// Update the timer display if set and we're not in intermission
		if (timelimit->value && !level.intermission_framenum) {
        	HUD_UpdateSpectatorTimer(clent);
		} else {
			Ghud_SetFlags(clent, hud[h_spectator_timer_border], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_timer], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_tm], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_mm], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_sep], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_ts], GHF_HIDE);
			Ghud_SetFlags(clent, hud[h_spectator_time_ss], GHF_HIDE);
		}
	}
}

#endif

