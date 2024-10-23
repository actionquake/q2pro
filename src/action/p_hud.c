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

		// send the layout
		Q_snprintf (entry, sizeof (entry),
			"client %i %i %i %i %i %i ",
			x, y, (int)(cl - game.clients), cl->resp.score, cl->ping,
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


#if AQTION_EXTENSION

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


void HUD_SpectatorSetup(edict_t *clent)
{
	Ghud_ClearForClient(clent);
	clent->client->resp.hud_type = 1;

	int *hud = clent->client->resp.hud_items;
	int i;

	if (teamplay->value && spectator_hud->value)
	{
		for (i = 0; i < 6; i++)
		{
			int x, y;
			int h_base = h_nameplate_l + (i * 5);
			int h;

			x = 0;
			y = 28 + (28 * i);

			h = h_base; // back bar
			hud[h] = Ghud_NewElement(clent, GHT_FILL);
			Ghud_SetPosition(clent, hud[h], x, y);
			Ghud_SetAnchor(clent, hud[h], 0, 0);
			Ghud_SetSize(clent, hud[h], 144, 24);
			Ghud_SetColor(clent, hud[h], 110, 45, 45, 230);

			h = h_base + 1; // health bar
			hud[h] = Ghud_NewElement(clent, GHT_FILL);
			Ghud_SetPosition(clent, hud[h], x, y);
			Ghud_SetAnchor(clent, hud[h], 0, 0);
			Ghud_SetSize(clent, hud[h], 0, 24);
			Ghud_SetColor(clent, hud[h], 220, 60, 60, 230);

			h = h_base + 2; // name
			hud[h] = Ghud_AddText(clent, x + 142, y + 3, "");
			Ghud_SetAnchor(clent, hud[h], 0, 0);
			Ghud_SetTextFlags(clent, hud[h], UI_RIGHT);

			h = h_base + 3; // k/d
			hud[h] = Ghud_AddText(clent, x + 142, y + 14, "");
			Ghud_SetAnchor(clent, hud[h], 0, 0);
			Ghud_SetTextFlags(clent, hud[h], UI_RIGHT);

			h = h_base + 4; // weapon select
			hud[h] = Ghud_AddIcon(clent, x + 2, y + 2, level.pic_items[M4_NUM], 20, 20);
			Ghud_SetAnchor(clent, hud[h], 0, 0);
		}

		for (i = 0; i < 6; i++)
		{
			int x, y;
			int h_base = h_nameplate_r + (i * 5);
			int h;

			x = -144;
			y = 28 + (28 * i);

			h = h_base; // back bar
			hud[h] = Ghud_NewElement(clent, GHT_FILL);
			Ghud_SetPosition(clent, hud[h], x, y);
			Ghud_SetAnchor(clent, hud[h], 1, 0);
			Ghud_SetSize(clent, hud[h], 144, 24);
			Ghud_SetColor(clent, hud[h], 30, 60, 110, 230);

			h = h_base + 1; // health bar
			hud[h] = Ghud_NewElement(clent, GHT_FILL);
			Ghud_SetPosition(clent, hud[h], x, y);
			Ghud_SetAnchor(clent, hud[h], 1, 0);
			Ghud_SetSize(clent, hud[h], 0, 24);
			Ghud_SetColor(clent, hud[h], 40, 80, 220, 230);

			h = h_base + 2; // name
			hud[h] = Ghud_AddText(clent, x + 2, y + 3, "");
			Ghud_SetAnchor(clent, hud[h], 1, 0);
			Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

			h = h_base + 3; // k/d
			hud[h] = Ghud_AddText(clent, x + 2, y + 14, "");
			Ghud_SetAnchor(clent, hud[h], 1, 0);
			Ghud_SetTextFlags(clent, hud[h], UI_LEFT);

			h = h_base + 4; // weapon select
			hud[h] = Ghud_AddIcon(clent, x + 122, y + 2, level.pic_items[M4_NUM], 20, 20);
			Ghud_SetAnchor(clent, hud[h], 1, 0);
		}

		hud[h_team_l] = Ghud_AddIcon(clent, 2, 2, level.pic_teamskin[1], 24, 24);
		Ghud_SetAnchor(clent, hud[h_team_l], 0, 0);
		hud[h_team_l_num] = Ghud_AddNumber(clent, 96, 2, 0);
		Ghud_SetSize(clent, hud[h_team_l_num], 2, 0);
		Ghud_SetAnchor(clent, hud[h_team_l_num], 0, 0);

		hud[h_team_r] = Ghud_AddIcon(clent, -26, 2, level.pic_teamskin[2], 24, 24);
		Ghud_SetAnchor(clent, hud[h_team_r], 1, 0);
		hud[h_team_r_num] = Ghud_AddNumber(clent, -128, 2, 0);
		Ghud_SetSize(clent, hud[h_team_r_num], 1, 0);
		Ghud_SetAnchor(clent, hud[h_team_r_num], 1, 0);
	}
}

void HUD_SpectatorUpdate(edict_t *clent)
{
	int i;

	if (teamplay->value && spectator_hud->value)
	{
		int *hud = clent->client->resp.hud_items;

		if (!(clent->client->pers.spec_flags & SPECFL_SPECHUD_NEW)) // hide all elements since client doesn't want them
		{
			for (i = 0; i <= h_team_r_num; i++)
			{
				Ghud_SetFlags(clent, hud[i], GHF_HIDE);
			}
			return;
		}

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

		// team 1 (red team)
		Ghud_SetFlags(clent, hud[h_team_l], 0);
		Ghud_SetFlags(clent, hud[h_team_l_num], 0);
		Ghud_SetInt(clent, hud[h_team_l_num], teams[TEAM1].score);

		for (i = 0; i < 6; i++)
		{
			int x, y;
			int h = h_nameplate_l + (i * 5);
			gclient_t *cl = team1_players[i];

			if (!cl) // if there is no player, hide all the elements for their plate
			{
				Ghud_SetFlags(clent, hud[h + 0], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 1], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 2], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 3], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 4], GHF_HIDE);
				continue;
			}

			x = 0;
			y = 28 + (28 * i);

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

				Ghud_SetPosition(clent, hud[h + 0], x, y);
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
				Ghud_SetPosition(clent, hud[h + 0], x + (144 * (hp_frac)), y);
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


		// team 2 (blue team)
		Ghud_SetFlags(clent, hud[h_team_r], 0);
		Ghud_SetFlags(clent, hud[h_team_r_num], 0);
		Ghud_SetInt(clent, hud[h_team_r_num], teams[TEAM2].score);
		if (teams[TEAM2].score >= 10) // gotta readjust size for justifying purposes
			Ghud_SetSize(clent, hud[h_team_r_num], 2, 0);
		else
			Ghud_SetSize(clent, hud[h_team_r_num], 1, 0);

		for (i = 0; i < 6; i++)
		{
			int x, y;
			int h = h_nameplate_r + (i * 5);
			gclient_t *cl = team2_players[i];

			if (!cl) // if there is no player, hide all the elements for their plate
			{
				Ghud_SetFlags(clent, hud[h + 0], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 1], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 2], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 3], GHF_HIDE);
				Ghud_SetFlags(clent, hud[h + 4], GHF_HIDE);
				continue;
			}

			x = -144;
			y = 28 + (28 * i);

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
				Ghud_SetPosition(clent, hud[h + 1], x + (144 * hp_invfrac), y);
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
}

#endif

