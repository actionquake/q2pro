// Espionage Mode by darksaint
// File format inspired by a_dom.c by Raptor007 and a_ctf.c from TNG team
// Re-worked from scratch from the original AQDT team, Black Monk, hal9000 and crew

#include "g_local.h"

espsettings_t espsettings;

// edict_t *potential_spawns[MAX_SPAWNS];
// int num_potential_spawns;

//cvar_t *esp_respawn = NULL;
//int esp_potential_spawns;
int esp_last_chosen_spawn = 0;
int esp_spawnpoint_index[TEAM_TOP] = {-1};

edict_t* chosenSpawnpoint[TEAM_TOP] = {NULL};
edict_t* etvTarget = NULL;

unsigned int esp_team_effect[] = {
	EF_BLASTER | EF_ROTATE | EF_TELEPORTER,
	EF_FLAG1,
	EF_GREEN_LIGHT | EF_COLOR_SHELL
};

unsigned int esp_team_fx[] = {
	RF_GLOW,
	RF_FULLBRIGHT,
	RF_FULLBRIGHT,
	RF_SHELL_GREEN
};

int esp_flag_count = 0;
int esp_team_flags[ TEAM_TOP ] = {0};
int esp_winner = NOTEAM;
int esp_flag = 0;
int esp_leader_pics[ TEAM_TOP ] = {0};
int esp_last_score = 0;
qboolean esp_punishment_phase = false;

/*
Toggles between the two game modes
*/
void EspForceEspionage(espmode_t espmode)
{
	gi.cvar_forceset("esp", "1");
	if (espmode == 0 || esp_atl->value) {
		espsettings.esp_mode = ESPMODE_ATL;
	} else if (espmode == 1) {
		espsettings.esp_mode = ESPMODE_ETV;
	}

	//gi.dprintf("Espionage mode set to %s\n", (espmode == 0) ? "ATL" : "ETV");
}

int EspCapturePointOwner( edict_t *flag )
{
	if( flag->s.effects == esp_team_effect[ TEAM1 ] )
		return TEAM1;
	return NOTEAM;
}


char *timedMessageStrings[NUM_MESSAGES] = {
    "LAST ROUND BEFORE HALFTIME..."
    // Add more messages as needed
};
/*
Timed message handler for Espionage, FMI look for the
TimedMessageIndex enum and timedMessageStrings array in a_esp.h

Example usage: addTimedMessage(TEAM1, NULL, 10, ESP_HALFTIME_WARNING)
would send message ESP_HALFTIME_WARNING to team 1, 10 seconds into a new round
TEAM0 would send to all players
Team is ignored when an entity is supplied as the second argument
*/
void EspTimedMessageHandler(int teamNum, edict_t *ent, int seconds, int timedMsgIndex)
{
	// Take care to specify an entity if referencing a player, else this can crash the server
	// Validate entity is a player
	if (teamNum > teamCount || teamNum < 0)
		return;
	if (ent && !ent->client){
		gi.dprintf("%s: Invalid entity passed\n", __func__);
		return;
	}
	// Convert frames to seconds
	//seconds = seconds * 10;

	addTimedMessage(teamNum, ent, seconds, timedMessageStrings[timedMsgIndex]);
}

/*
Supply two edicts, return distance between the two
*/
int _EspDistanceFromEdict(edict_t *ent, edict_t *target)
{
	vec3_t dist;
	VectorSubtract(ent->s.origin, target->s.origin, dist);
	return VectorLength(dist);
}

/*
Espionage Defender Leader Bonus
*/
void _EspBonusDefendLeader(edict_t *targ, edict_t *attacker)
{
	edict_t *leader = NULL;

	// You can't defend a leader on a team that doesn't have one
	if (attacker->client->resp.team == TEAM2 && espsettings.esp_mode == ESPMODE_ETV)
		return;

	if (IS_LEADER(targ))
		leader = targ;
	/*
	Leader protection bonus
	*/
	if (leader && leader != attacker && attacker->client->resp.team == leader->client->resp.team) {
		int attacker_leader_dist = _EspDistanceFromEdict(attacker, leader);
		int defender_leader_dist = _EspDistanceFromEdict(targ, leader);

		/* If attacker or defender are within ESP_ATTACKER_PROTECT_RADIUS units of the leader
			or the leader is visible to either player
		*/
		if (attacker_leader_dist < ESP_ATTACKER_PROTECT_RADIUS ||
		    defender_leader_dist < ESP_ATTACKER_PROTECT_RADIUS ||
			visible(leader, targ, MASK_SOLID) || visible(leader, attacker, MASK_SOLID)) {
			attacker->client->resp.score += ESP_LEADER_DANGER_PROTECT_BONUS;
			attacker->client->resp.esp_leaderprotectcount++;
			gi.bprintf(PRINT_MEDIUM, "%s gets %d bonus points for defending %s in the field!\n",
				   attacker->client->pers.netname, ESP_LEADER_DANGER_PROTECT_BONUS, teams[attacker->client->resp.team].leader_name);
		}
	}
	// Set framenum to prevent multiple bonuses too quickly
	attacker->client->resp.esp_lastprotectcap = level.realFramenum;
	return;
}

/*
Espionage Harrass Enemy Leader Bonus
*/
void _EspBonusHarassLeader(edict_t *targ, edict_t *attacker)
{
	edict_t *leader = NULL;

	// You can't attack a leader on a team that doesn't have one
	if (attacker->client->resp.team == TEAM1 && espsettings.esp_mode == ESPMODE_ETV)
		return;

	if (IS_LEADER(targ))
		leader = targ;
	/*
	Leader attack bonus
	*/
	if (leader && leader != attacker && attacker->client->resp.team != leader->client->resp.team) {
		int attacker_leader_dist = _EspDistanceFromEdict(attacker, leader);

		/* If attacker or defender are within ESP_ATTACKER_PROTECT_RADIUS units of the leader
			or the leader is visible to either player
		*/
		if (attacker_leader_dist < ESP_ATTACKER_HARASS_RADIUS ||
			visible(leader, targ, MASK_SOLID) || visible(leader, attacker, MASK_SOLID)) {
			attacker->client->resp.score += ESP_LEADER_HARASS_BONUS;
			gi.bprintf(PRINT_MEDIUM, "%s gets %d bonus points for harassing %s in the field!\n",
				   attacker->client->pers.netname, ESP_LEADER_HARASS_BONUS, teams[targ->client->resp.team].leader_name);
		}
	}
	// Set framenum to prevent multiple bonuses too quickly
	attacker->client->resp.esp_lasthurtleader = level.framenum;
	return;
}

/*
Espionage Leader Frag Bonus
*/
void _EspBonusFragLeader(edict_t *targ, edict_t *attacker)
{
	int i, enemyteam;
	edict_t *ent;

	if (IS_LEADER(targ)){
		attacker->client->resp.esp_lasthurtleader = level.framenum;
		attacker->client->resp.score += ESP_LEADER_FRAG_BONUS;
		gi.bprintf(PRINT_MEDIUM,
			   "%s gets %d bonus points for eliminating the enemy leader!\n", 
			   attacker->client->pers.netname, ESP_LEADER_FRAG_BONUS);

		// Stats
		attacker->client->resp.esp_leaderfragcount++;
		
		// NULL check
		if (espsettings.lastkilledleader) {
			/* If the killer of the leader was not the same as the last killer of the leader
				then reset the lastkilledleader and set the new edict
			*/
			if (espsettings.lastkilledleader != attacker) {
				// Set high killstreak if it's higher than the current best
				if (espsettings.lastkilledleader->client->resp.esp_leaderkillstreak > espsettings.lastkilledleader->client->resp.esp_leaderkillstreakbest) {
					espsettings.lastkilledleader->client->resp.esp_leaderkillstreakbest = espsettings.lastkilledleader->client->resp.esp_leaderkillstreak;
				}
				// Reset leader killstreak for that player
				espsettings.lastkilledleader->client->resp.esp_leaderkillstreak = 0;
				// Set the new player as the killer of the leader and award a streak start
				espsettings.lastkilledleader = attacker;
				attacker->client->resp.esp_leaderkillstreak++;
			} else {  // Same killer of the leader, increase killstreak
				attacker->client->resp.esp_leaderkillstreak++;
			}
		} else {
			espsettings.lastkilledleader = attacker;
		}

		enemyteam = (targ->client->resp.team != attacker->client->resp.team);
		for (i = 1; i <= game.maxclients; i++) {
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.team == enemyteam)
				ent->client->resp.esp_lasthurtleader = 0;
		}

	// Set framenum to prevent multiple bonuses too quickly
	attacker->client->resp.esp_lasthurtleader = level.realFramenum;
	return;
	}
}

/*
Espionage Capture Bonus
*/
void _EspBonusCapture(edict_t *attacker, edict_t *flag)
{
	edict_t *ent;

	gi.bprintf( PRINT_HIGH, "%s has reached the %s for %s! +%d points!\n",
		flag->owner->client->pers.netname,
		espsettings.target_name,
		teams[ flag->owner->client->resp.team ].name,
		ESP_LEADER_CAPTURE_BONUS );

	// Stats
	flag->owner->client->resp.esp_caps++;
	flag->owner->client->resp.esp_capstreak++;
	if (flag->owner->client->resp.esp_capstreak > flag->owner->client->resp.esp_capstreakbest)
		flag->owner->client->resp.esp_capstreakbest = flag->owner->client->resp.esp_capstreak;
	LogCapture(flag->owner);

	// Bonus points awarded
	flag->owner->client->resp.score += ESP_LEADER_CAPTURE_BONUS;

	// Check if teammates are nearby, they deserve a bonus too
	int i;
	for (i = 0; i < game.maxclients; i++){
		ent = &g_edicts[1 + i];
		if( !(ent->inuse && ent->client && ent->client->resp.team) )
			continue;
		else if( ent == flag->owner )
			continue;
		else if( ent->client->resp.team != flag->owner->client->resp.team )
			continue;
		else if( _EspDistanceFromEdict(ent, flag) > ESP_TARGET_PROTECT_RADIUS )
			continue;
		else {
			// Bonus point awarded
			ent->client->resp.score += ESP_LEADER_ESCORT_BONUS;
		}
	}
}

/*
Espionage Defender Capture Point Bonus
*/
void _EspBonusDefendCapture(edict_t *targ, edict_t *attacker)
{
	// Only ETV mode, and only for those on TEAM2
	if (!(espsettings.esp_mode == ESPMODE_ETV))
		return;
	if (attacker->client->resp.team != TEAM2)
		return;

	edict_t *cap = espsettings.capturepoint;
	int attacker_cap_dist = _EspDistanceFromEdict(attacker, cap);
	int defender_cap_dust = _EspDistanceFromEdict(targ, cap);

	// If the attacker or defender are within 400 units of the capture point, 
	// or the capture point is visible to either player
	if (attacker_cap_dist < ESP_ATTACKER_PROTECT_RADIUS || 
	defender_cap_dust < ESP_ATTACKER_PROTECT_RADIUS ||
	visible(cap, targ, MASK_SOLID) || 
	visible(cap, attacker, MASK_SOLID)) {
		// we defended the base flag
		attacker->client->resp.score += ESP_FLAG_DEFENSE_BONUS;
		if (cap->solid == SOLID_NOT) {
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s.\n",
				attacker->client->pers.netname, espsettings.target_name);
			IRC_printf(IRC_T_GAME, "%n defends the %n.\n",
				attacker->client->pers.netname,
				espsettings.target_name);
		}
	}

	// Stats
	attacker->client->resp.esp_capdefendercount++;

	// Set framenum to prevent multiple bonuses too quickly
	attacker->client->resp.esp_lastprotectcap = level.realFramenum;
	return;
}

void EspScoreBonuses(edict_t * targ, edict_t * attacker)
{
	/*
	Multiple bonus points, for:
	- Fragging the enemy leader
	- Defending your leader
	- Defending the capture point as TEAM2
	*/

	// You don't get bonus points after the round is over
	if (!team_round_going)
		return;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	int enemyteam;
	enemyteam = (targ->client->resp.team != attacker->client->resp.team);
	if (!enemyteam)
		return;		// whoever died isn't on a team

	// Is the attacker eligible for these bonuses?
	int lpc = attacker->client->resp.esp_lastprotectcap;
	int lpl = attacker->client->resp.esp_lastprotectleader;
	int lhl = attacker->client->resp.esp_lasthurtleader;

	if (esp_debug->value){
		int bonusEligible = (level.realFramenum - ESP_BONUS_COOLDOWN);
		gi.dprintf("%s: lpc is %d, lpl is %d, lhl is %d\n", __func__, lpc, lpl, lhl);
		gi.dprintf("%s: attacker is %s, targ is %s\n", __func__, attacker->client->pers.netname, targ->client->pers.netname);
		gi.dprintf("%s: level.realFramenum is %i, subtracted ESP_BONUS_COOLDOWN is %i\n", __func__, level.realFramenum, bonusEligible);
	}

	// If the attacker has already received a bonus in the last ESP_BONUS_COOLDOWN frames
	if (lpc > (level.realFramenum - ESP_BONUS_COOLDOWN))
		_EspBonusDefendCapture(targ, attacker);
	if (lpl > (level.realFramenum - ESP_BONUS_COOLDOWN))
		_EspBonusDefendLeader(targ, attacker);
	if (lhl > (level.realFramenum - ESP_BONUS_COOLDOWN))
		_EspBonusHarassLeader(targ, attacker);

	// This will always award a bonus if the attacker is eligible, no time limit
	_EspBonusFragLeader(targ, attacker);

}

void EspCapturePointThink( edict_t *flag )
{
	// If the flag was touched this frame, make it owned by that team.
	if( flag->owner && flag->owner->client && flag->owner->client->resp.team ) {
		unsigned int effect = esp_team_effect[ flag->owner->client->resp.team ];
		if( flag->s.effects != effect )
		{
			edict_t *ent = NULL;
			int prev_owner = EspCapturePointOwner( flag );

			//gi.dprintf("prev flag owner team is %d\n", prev_owner);

			if( prev_owner != NOTEAM )
				esp_team_flags[ prev_owner ] --;

			flag->s.effects = effect;
			flag->s.renderfx = esp_team_fx[ flag->owner->client->resp.team ];
			esp_team_flags[ flag->owner->client->resp.team ] ++;
			flag->s.modelindex = esp_flag;

			// Get flag location if possible.

			// Commented out but may still be useful at some point
			// This grabs the map location name
			    // qboolean has_loc = false;
				// char location[ 128 ] = "(";
				// has_loc = GetPlayerLocation( flag, location + 1 );
				// if( has_loc )
				// 	strcat( location, ") " );
				// else
				// 	location[0] = '\0';

			_EspBonusCapture(flag->owner, flag);

			// This is the game state, where the leader made it to the capture point
			espsettings.escortcap = true;

			// Escort point captured, end round and start again
			gi.sound( &g_edicts[0], CHAN_BODY | CHAN_NO_PHS_ADD, gi.soundindex("aqdt/aqg_bosswin.wav"), 1.0, ATTN_NONE, 0.0 );
			// espsettings.escortcap = flag->owner->client->resp.team;
			if (esp_punish->value) {
				esp_punishment_phase = true;
				EspPunishment(OtherTeam(flag->owner->client->resp.team));
			}

			if (use_rewards->value) {
				if(teams[TEAM1].leader->client->resp.esp_capstreak == 5)
					Announce_Reward(teams[TEAM1].leader, DOMINATING);
				if(teams[TEAM1].leader->client->resp.esp_capstreak == 10)
					Announce_Reward(teams[TEAM1].leader, UNSTOPPABLE);
			}

			for( ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent ++ )
			{
				if( ! (ent->inuse && ent->client && ent->client->resp.team) )
					continue;
				else if( ent == flag->owner )
					unicastSound( ent, gi.soundindex("tng/flagcap.wav"), 0.5 );
			}
		}
	}
	flag->owner = NULL;

	flag->nextthink = level.framenum + FRAMEDIV;
}

void EspTouchCapturePoint( edict_t *flag, edict_t *player, cplane_t *plane, csurface_t *surf )
{
	if( ! player->client )
		return;
	if( ! player->client->resp.team )
		return;
	if( (player->health < 1) || ! IS_ALIVE(player) )
		return;
	if( lights_camera_action || in_warmup )
		return;
	if( player->client->uvTime )
		return;
	// Player must be team leader on team 1 to activate the flag
	if (!IS_LEADER(player) || player->client->resp.team != TEAM1)
		return;

	// If the flag hasn't been touched this frame, the player will take it.
	if( ! flag->owner )
		flag->owner = player;
}

void EspMakeCapturePoint(edict_t *flag)
{
	vec3_t dest = {0};
	trace_t tr = {0};

	if (esp_debug->value)
		gi.dprintf("%s: Creating a new capturepoint\n", __func__);
	VectorSet( flag->mins, -15, -15, -15 );
	VectorSet( flag->maxs,  15,  15,  15 );

	// Put the flag on the ground.
	VectorCopy( flag->s.origin, dest );
	dest[2] -= 128;
	tr = gi.trace( flag->s.origin, flag->mins, flag->maxs, dest, flag, MASK_SOLID );
	if( ! tr.startsolid )
		VectorCopy( tr.endpos, flag->s.origin );

	VectorCopy( flag->s.origin, flag->old_origin );

	flag->solid = SOLID_TRIGGER;
	flag->movetype = MOVETYPE_NONE;
	flag->s.modelindex = esp_flag;
	flag->s.skinnum = 0;
	flag->s.effects = esp_team_effect[ NOTEAM ];
	flag->s.renderfx = esp_team_fx[ NOTEAM ];
	flag->owner = NULL;
	flag->touch = EspTouchCapturePoint;
	NEXT_KEYFRAME( flag, EspCapturePointThink );
	flag->classname = "item_flag";
	flag->svflags &= ~SVF_NOCLIENT;
	gi.linkentity( flag );

	/* Indicator arrow
    	This appears regardless of indicator settings
	*/
	#if AQTION_EXTENSION
    if (!flag->obj_arrow){
		flag->obj_arrow = G_Spawn();
		flag->obj_arrow->solid = SOLID_NOT;
		flag->obj_arrow->movetype = MOVETYPE_NOCLIP;
		flag->obj_arrow->classname = "ind_arrow_objective";
		flag->obj_arrow->owner = flag;
		flag->obj_arrow->s.effects |= RF_INDICATOR | RF_GLOW;
		flag->obj_arrow->s.modelindex = level.model_arrow;

		VectorCopy(flag->s.origin, flag->obj_arrow->s.origin);
		// float arrow above briefcase
		flag->obj_arrow->s.origin[2] += 75;

		gi.linkentity(flag->obj_arrow);

		if (esp_debug->value){
			gi.dprintf("%s: ** Indicator arrow spawned at <%f %f %f>\n", __func__, flag->obj_arrow->s.origin[0], flag->obj_arrow->s.origin[1], flag->obj_arrow->s.origin[2]);
			gi.dprintf("%s: ** Flag coordinates are: <%f %f %f>\n", __func__, flag->s.origin[0], flag->s.origin[1], flag->s.origin[2]);
		}
	}
	#endif

	// Globally accessible capturepoint
	etvTarget = flag;
	
	esp_flag_count ++;
}

// Function to find the sole capturepoint, remove it and recreate it exactly as it was
// This is used to reset the capturepoint after a round resets
void EspResetCapturePoint(void)
{
	edict_t *ent = NULL;
	edict_t *flag = NULL;

	// Reset escortcap value
	espsettings.escortcap = false;

	// Find the flag
	while ((ent = G_Find(ent, FOFS(classname), "item_flag")) != NULL) {
		flag = ent;
	}
	if (flag){
		EspMakeCapturePoint(flag);
	}
}

void EspSetTeamSpawns(int team, char *str)
{
	edict_t *spawn = NULL;
	char *next;
	vec3_t pos;
	float angle;
	espsettings_t *es = &espsettings;
	int esp_potential_spawns = 0;

	char *team_spawn_name = "info_player_team1";
	if(team == TEAM2)
		team_spawn_name = "info_player_team2";
	if (teamCount == 3 && team == TEAM3)
		team_spawn_name = "info_player_team3";

	/* find and remove all team spawns for this team */
	while ((spawn = G_Find(spawn, FOFS(classname), team_spawn_name)) != NULL) {
		G_FreeEdict (spawn);
	}

	next = strtok(str, ",");
	do {
		if (sscanf(next, "<%f %f %f %f>", &pos[0], &pos[1], &pos[2], &angle) != 4) {
			if (esp_debug->value)
				gi.dprintf("%s: invalid spawn point: %s, expected <x y z a>\n", __func__, next);
			continue;
		}

		spawn = G_Spawn();
		VectorCopy(pos, spawn->s.origin);
		spawn->s.angles[YAW] = angle;
		spawn->classname = ED_NewString (team_spawn_name);

		if (esp_debug->value)
			gi.dprintf("%s: Created spawnpoint for %s at <%f %f %f %f>\n", __func__, team_spawn_name, pos[0], pos[1], pos[2], angle);
		es->custom_spawns[team][esp_potential_spawns] = spawn;
		esp_potential_spawns++;
		if (esp_potential_spawns >= MAX_SPAWNS)
		{
			if (esp_debug->value)
				gi.dprintf("%s: Warning: MAX_SPAWNS exceeded\n", __func__);
			break;
		}

		next = strtok(NULL, ",");
	} while(next != NULL);
	
}

void EspEnforceDefaultSettings(char *defaulttype)
{
	qboolean default_team = (Q_stricmp(defaulttype,"team")==0) ? true : false;
	qboolean default_respawn = (Q_stricmp(defaulttype,"respawn")==0) ? true : false;
	qboolean default_author = (Q_stricmp(defaulttype,"author")==0) ? true : false;
	int i = 0;

	if (esp_debug->value)
		gi.dprintf("%s: defaulttype is %s\n", __func__, defaulttype);

	if(default_author) {
		EspForceEspionage(ESPMODE_ATL);
		Q_strncpyz(espsettings.author, "AQ2World Team", sizeof(espsettings.author));
		Q_strncpyz(espsettings.name, "Time for Action!", sizeof(espsettings.name));

		gi.dprintf(" Author           : %s\n", espsettings.author);
		gi.dprintf(" Name		  : %s\n", espsettings.name);
		gi.dprintf(" Game type        : Assassinate the Leader\n");
	}

	if(default_respawn) {
		for (i = TEAM1; i <= teamCount; i++) {
			teams[i].respawn_timer = ESP_DEFAULT_RESPAWN_TIME;
		}
		gi.dprintf("  Respawn Rate: %d seconds\n", ESP_DEFAULT_RESPAWN_TIME);
	}

	if(default_team) {
		/// Default skin/team/names - red team
		Q_strncpyz(teams[TEAM1].name, ESP_RED_TEAM, sizeof(teams[TEAM1].name));
		Q_strncpyz(teams[TEAM1].skin, ESP_RED_SKIN, sizeof(teams[TEAM1].skin));
		Q_strncpyz(teams[TEAM1].leader_name, ESP_RED_LEADER_NAME, sizeof(teams[TEAM1].leader_name));
		Q_strncpyz(teams[TEAM1].leader_skin, ESP_RED_LEADER_SKIN, sizeof(teams[TEAM1].leader_skin));
		Q_strncpyz(teams[TEAM1].skin_index, "../pics/ctf_r_i", sizeof(teams[TEAM1].skin_index));
		Q_snprintf(teams[TEAM1].leader_skin_index, sizeof(teams[TEAM1].leader_skin_index), "../players/%s_i", teams[TEAM1].leader_skin);

		/// Default skin/team/names - blue team
		Q_strncpyz(teams[TEAM2].name, ESP_BLUE_TEAM, sizeof(teams[TEAM2].name));
		Q_strncpyz(teams[TEAM2].skin, ESP_BLUE_SKIN, sizeof(teams[TEAM2].skin));
		Q_strncpyz(teams[TEAM2].leader_name, ESP_BLUE_LEADER_NAME, sizeof(teams[TEAM2].leader_name));
		Q_strncpyz(teams[TEAM2].leader_skin, ESP_BLUE_LEADER_SKIN, sizeof(teams[TEAM2].leader_skin));
		Q_strncpyz(teams[TEAM2].skin_index, "../pics/ctf_b_i", sizeof(teams[TEAM2].skin_index));
		Q_snprintf(teams[TEAM2].leader_skin_index, sizeof(teams[TEAM2].leader_skin_index), "../players/%s_i", teams[TEAM2].leader_skin);
		if(teamCount == 3) {
			/// Default skin/team/names - green team
			Q_strncpyz(teams[TEAM3].name, ESP_GREEN_TEAM, sizeof(teams[TEAM3].name));
			Q_strncpyz(teams[TEAM3].skin, ESP_GREEN_SKIN, sizeof(teams[TEAM3].skin));
			Q_strncpyz(teams[TEAM3].leader_name, ESP_GREEN_LEADER_NAME, sizeof(teams[TEAM3].leader_name));
			Q_strncpyz(teams[TEAM3].leader_skin, ESP_GREEN_LEADER_SKIN, sizeof(teams[TEAM3].leader_skin));
			Q_strncpyz(teams[TEAM3].skin_index, "../pics/ctf_g_i", sizeof(teams[TEAM3].skin_index));
			Q_snprintf(teams[TEAM3].leader_skin_index, sizeof(teams[TEAM3].leader_skin_index), "../players/%s_i", teams[TEAM3].leader_skin);
		}
		gi.dprintf("  Red Team: %s -- Skin: %s\n", ESP_RED_TEAM, ESP_RED_SKIN);
		gi.dprintf("  Red Leader: %s -- Skin: %s\n", ESP_RED_LEADER_NAME, ESP_RED_LEADER_SKIN);
		gi.dprintf("  Blue Team: %s -- Skin: %s\n", ESP_BLUE_TEAM, ESP_BLUE_SKIN);
		if(espsettings.esp_mode == ESPMODE_ATL)
			gi.dprintf("  Blue Leader: %s -- Skin: %s\n", ESP_BLUE_LEADER_NAME, ESP_BLUE_LEADER_SKIN);
		if(teamCount == 3){
			gi.dprintf("  Green Team: %s -- Skin: %s\n", ESP_GREEN_TEAM, ESP_GREEN_SKIN);
			gi.dprintf("  Green Leader: %s -- Skin: %s\n", ESP_GREEN_LEADER_NAME, ESP_GREEN_LEADER_SKIN);
		}
	}
}

qboolean EspLoadConfig(const char *mapname)
{
	char buf[1024];
	char *ptr;
	qboolean no_file = false;
	qboolean loaded_default_file = false;
	espsettings_t *es = &espsettings;
	FILE *fh;
	int i = 0;

	memset(&espsettings, 0, sizeof(espsettings));

	//esp_flag = gi.modelindex("models/items/bcase/g_bc1.md2");
	esp_flag = gi.modelindex("models/cases/b_case.md3");

	gi.dprintf("** Trying to load Espionage configuration file for %s **\n", mapname);

	sprintf (buf, "%s/tng/%s.esp", GAMEVERSION, mapname);

	fh = fopen (buf, "r");
	if (!fh) {
		//Default to ATL mode in this case
		gi.dprintf ("Warning: Espionage configuration file \" %s \" was not found.\n", buf);
		EspForceEspionage(ESPMODE_ATL);
		sprintf (buf, "%s/tng/default.esp", GAMEVERSION);
		fh = fopen (buf, "r");
		if (!fh){
			gi.dprintf ("Warning: Default Espionage configuration file was not found.\n");
			gi.dprintf ("Using hard-coded Assassinate the Leader scenario settings.\n");
			no_file = true;
		} else {
			gi.dprintf("Found %s, attempting to load it...\n", buf);
			loaded_default_file = true;
		}
	} else {
    // Check if the file is empty
		fseek(fh, 0, SEEK_END);
		long size;
		size = ftell(fh);
		if (size == 0) {
			no_file = true;
			gi.dprintf("ERROR: Espionage file %s is empty, loading safe defaults\n", buf);
		}
	}

	// Hard-coded scenario settings so things don't break
	if(no_file){
		EspForceEspionage(ESPMODE_ATL);
		// TODO: A better GHUD method to display this?
		gi.dprintf("-------------------------------------\n");
		gi.dprintf("Hard-coded Espionage configuration loaded\n");

		// Set game type to ATL
		/// Default game settings
		EspEnforceDefaultSettings("author");
		EspEnforceDefaultSettings("respawn");
		EspEnforceDefaultSettings("team");

	} else { // A valid file was found, load it

		gi.dprintf("-------------------------------------\n");
		gi.dprintf("Espionage configuration found at %s\n", buf);
		ptr = INI_Find(fh, "esp", "author");
		if(ptr) {
			gi.dprintf("- Author    : %s\n", ptr);
			Q_strncpyz(espsettings.author, ptr, sizeof(espsettings.author));
		}
		ptr = INI_Find(fh, "esp", "name");
		if(ptr) {
			gi.dprintf("- Name      : %s\n", ptr);
			Q_strncpyz(espsettings.name, ptr, sizeof(espsettings.name));
		}

		// This is where we're setting which Espionage mode we want
		ptr = INI_Find(fh, "esp", "type");
		char *gametypename = ESPMODE_ATL_NAME;
		if(!strcmp(ptr, ESPMODE_ATL_SNAME) && !strcmp(ptr, ESPMODE_ETV_SNAME)){
			gi.dprintf("Warning: Value for '[esp] type is not 'etv' or 'atl', forcing ATL mode\n");
		    gi.dprintf("- Game type : %s\n", ESPMODE_ATL_NAME);
			EspForceEspionage(ESPMODE_ATL);
			gametypename = ESPMODE_ATL_NAME;
		} else {
			if(ptr) {
				if((strcmp(ptr, ESPMODE_ETV_SNAME) == 0 && esp_atl->value == 0)){
					EspForceEspionage(ESPMODE_ETV);
					gametypename = ESPMODE_ETV_NAME;
				} else {
					EspForceEspionage(ESPMODE_ATL);
					gametypename = ESPMODE_ATL_NAME;
				}
			}
			if (use_3teams->value) {
				// Only ATL available in 3 team mode
				EspForceEspionage(ESPMODE_ATL);
				gametypename = ESPMODE_ATL_NAME;
			}
			gi.dprintf("- Game type : %s\n", gametypename);
		}
		// Force ATL mode trying to get ETV mode going with 3teams
		if (espsettings.esp_mode == ESPMODE_ETV && use_3teams->value){
			gi.dprintf("%s and use_3team are incompatible, defaulting to %s", ESPMODE_ETV_NAME, ESPMODE_ATL_NAME);
			EspForceEspionage(ESPMODE_ATL);
		}

		// End of Espionage game mode forced settings

		gi.dprintf("- Respawn times\n");
		char *r_respawn_time, *b_respawn_time, *g_respawn_time;

		r_respawn_time = INI_Find(fh, "respawn", "red");
		b_respawn_time = INI_Find(fh, "respawn", "blue");
		if(teamCount == 3)
			g_respawn_time = INI_Find(fh, "respawn", "green");
		else
			g_respawn_time = NULL;

		if ((!r_respawn_time || !b_respawn_time) || (teamCount == 3 && !g_respawn_time)){
			gi.dprintf("Warning: Malformed or missing settings for respawn times\n");
			gi.dprintf("Enforcing defaults\n");
			EspEnforceDefaultSettings("respawn");
		} else {
			if(r_respawn_time) {
				gi.dprintf("    Red     : %s seconds\n", r_respawn_time);
				teams[TEAM1].respawn_timer = atoi(r_respawn_time);
			}
			if(b_respawn_time) {
				gi.dprintf("    Blue    : %s seconds\n", b_respawn_time);
				teams[TEAM2].respawn_timer = atoi(b_respawn_time);
			}
			if (teamCount == 3){
				if(g_respawn_time) {
					gi.dprintf("    Green   : %s seconds\n", g_respawn_time);
					teams[TEAM3].respawn_timer = atoi(g_respawn_time);
				}
			}
		}

		// Only set the flag if the scenario is ETV
		if(espsettings.esp_mode == ESPMODE_ETV) {
			gi.dprintf("- Target\n");
			ptr = INI_Find(fh, "target", "escort");
			if(ptr) {
				ptr = strchr( ptr, '<' );
				while( ptr )
				{
					edict_t *flag = G_Spawn();

					char *space = NULL, *end = strchr( ptr + 1, '>' );
					if( end )
						*end = '\0';

					flag->s.origin[0] = atof( ptr + 1 );
					space = strchr( ptr + 1, ' ' );
					if( space )
					{
						flag->s.origin[1] = atof( space );
						space = strchr( space + 1, ' ' );
						if( space )
						{
							flag->s.origin[2] = atof( space );
							space = strchr( space + 1, ' ' );
							if( space )
								flag->s.angles[YAW] = atof( space );
						}
					}

					EspMakeCapturePoint( flag );

					// Set the capture point in the settings
					es->capturepoint = flag;

					ptr = strchr( (end ? end : ptr) + 1, '<' );
				}

				if( !esp_flag_count ) {
					gi.dprintf("Warning: Espionage needs 'escort' target coordinates in: tng/%s.esp\n", mapname);
					gi.dprintf("* Forcing ATL mode to be safe\n");
					EspForceEspionage(ESPMODE_ATL);
				}

				ptr = INI_Find(fh, "target", "name");
				size_t ptr_len = strlen(ptr);
				if(ptr) {
					if (ptr_len <= MAX_ESP_STRLEN) {
						gi.dprintf("    Area    : %s\n", ptr);
					} else {
						gi.dprintf("Warning: [target] name > 32 characters, setting to \"The Spot\"");
						ptr = "The Spot";
					}
					Q_strncpyz(espsettings.target_name, ptr, sizeof(espsettings.target_name));
				}
			} else {
				gi.dprintf( "Warning: Escort target coordinates not found in tng/%s.esp\n", mapname );
				gi.dprintf("* Forcing ATL mode to be safe\n");
				EspForceEspionage(ESPMODE_ATL);
			}
		}

		// No custom spawns in default file
		if (loaded_default_file){
			// Clear custom spawns
			for (i = TEAM1; i <= teamCount; i++) {
				memset(es->custom_spawns[i], 0, sizeof(es->custom_spawns[i]));
			}
		}

		if (!loaded_default_file) {
			ptr = INI_Find(fh, "spawns", "red");
			if(ptr) {
				gi.dprintf("Team 1 spawns: %s\n", ptr);
				EspSetTeamSpawns(TEAM1, ptr);
			}
			ptr = INI_Find(fh, "spawns", "blue");
			if(ptr) {
				gi.dprintf("Team 2 spawns: %s\n", ptr);
				EspSetTeamSpawns(TEAM2, ptr);
			}
			if (teamCount == 3){
				ptr = INI_Find(fh, "spawns", "green");
				if(ptr) {
					gi.dprintf("Team 3 spawns: %s\n", ptr);
					EspSetTeamSpawns(TEAM3, ptr);
				}
			}
		}

		gi.dprintf("- Teams\n");
		for (i = TEAM1; i <= teamCount; i++) {
			const char *team_color;
			switch (i) {
				case TEAM1:
					team_color = "red_team";
					break;
				case TEAM2:
					team_color = "blue_team";
					break;
				case TEAM3:
					team_color = "green_team";
					break;
				default:
					continue;
			}

			ptr = INI_Find(fh, team_color, "name");
			if (ptr) {
				Q_strncpyz(teams[i].name, ptr, sizeof(teams[i].name));
			}

			ptr = INI_Find(fh, team_color, "skin");
			if (ptr) {
				Q_strncpyz(teams[i].skin, ptr, sizeof(teams[i].skin));
			}

			ptr = INI_Find(fh, team_color, "leader_name");
			if (ptr) {
				Q_strncpyz(teams[i].leader_name, ptr, sizeof(teams[i].leader_name));
			}

			ptr = INI_Find(fh, team_color, "leader_skin");
			if (ptr) {
				Q_strncpyz(teams[i].leader_skin, ptr, sizeof(teams[i].leader_skin));
			}
		}

		qboolean missing_property = false;
			for (i = TEAM1; i <= teamCount; i++) {
				if (strlen(teams[i].skin) == 0 || 
				strlen(teams[i].name) == 0 || 
				strlen(teams[i].leader_name) == 0 || 
				strlen(teams[i].leader_skin) == 0) {
					missing_property = true;
					break;
				}
			}
			if (missing_property) {
				gi.dprintf("Warning: Could not read value for team skin, name, leader or leader_skin; review your file\n");
				gi.dprintf("* Enforcing safe defaults\n");
				EspEnforceDefaultSettings("team");
				espsettings.custom_skins = false;
			} else {
				espsettings.custom_skins = true;
			}

			for (i = TEAM1; i <= teamCount; i++) {
				const char *team_color;
				switch (i) {
					case TEAM1:
						team_color = "Red";
						break;
					case TEAM2:
						team_color = "Blue";
						break;
					case TEAM3:
						team_color = "Green";
						break;
					default:
						continue;
				}

				gi.dprintf("    %s Team: %s, Skin: %s\n", team_color, teams[i].name, teams[i].skin);
				gi.dprintf("    %s Leader: %s, Skin: %s\n\n", team_color, teams[i].leader_name, teams[i].leader_skin);
			}
	}

	gi.dprintf("-------------------------------------\n");

	if (fh)
		fclose(fh);

	// Load skin indexes
	if (!no_file || !loaded_default_file) {
		if (esp_debug->value)
			gi.dprintf("%s: *** Loading skin indexes\n", __func__);
		Q_snprintf(teams[TEAM1].skin_index, sizeof(teams[TEAM1].skin_index), "../players/%s_i", teams[TEAM1].skin);
		Q_snprintf(teams[TEAM2].skin_index, sizeof(teams[TEAM2].skin_index), "../players/%s_i", teams[TEAM2].skin);
		Q_snprintf(teams[TEAM3].skin_index, sizeof(teams[TEAM3].skin_index), "../players/%s_i", teams[TEAM3].skin);
		Q_snprintf(teams[TEAM1].leader_skin_index, sizeof(teams[TEAM1].leader_skin_index), "../players/%s_i", teams[TEAM1].leader_skin);
		if (espsettings.esp_mode == ESPMODE_ATL) {
			Q_snprintf(teams[TEAM2].leader_skin_index, sizeof(teams[TEAM2].leader_skin_index), "../players/%s_i", teams[TEAM2].leader_skin);
			Q_snprintf(teams[TEAM3].leader_skin_index, sizeof(teams[TEAM3].leader_skin_index), "../players/%s_i", teams[TEAM3].leader_skin);
		}
	}

	if((espsettings.esp_mode == ESPMODE_ETV) && teamCount == 3){
		gi.dprintf("Warning: ETV mode requested with use_3teams enabled, forcing ATL mode");
		EspForceEspionage(ESPMODE_ATL);
	}

	return true;
}

int EspGetRespawnTime(edict_t *ent)
{
	int spawntime = teams[ent->client->resp.team].respawn_timer;
	if(ent->client->resp.team == TEAM1 && teams[TEAM1].respawn_timer > -1)
		spawntime = teams[TEAM1].respawn_timer;
	else if(ent->client->resp.team == TEAM2 && teams[TEAM2].respawn_timer > -1)
		spawntime = teams[TEAM2].respawn_timer;
	else if((teamCount == 3) && ent->client->resp.team == TEAM3 && teams[TEAM3].respawn_timer > -1)
		spawntime = teams[TEAM3].respawn_timer;

	if (!IS_LEADER(ent) && team_round_going) {
		gi.centerprintf(ent, "You will respawn in %d seconds\n", spawntime);
	}
	return spawntime;
}

/*
TODO: Fire this when there's 4 seconds left before a respawn
*/
void EspRespawnLCA(edict_t *ent)
{

	// Print out all conditions below as debug prints
	// This is massively noisy so I'm setting it so that esp_debug must be 2 to see it
	// if (esp_debug->value == 2)
	// 	gi.dprintf("%s: ent->inuse is %d\n", __func__, ent->inuse);
	// 	gi.dprintf("%s: ent->client->resp.team is %d\n", __func__, ent->client->resp.team);
	// 	gi.dprintf("%s: ent->client->respawn_framenum is %d\n", __func__, ent->client->respawn_framenum);
	// 	gi.dprintf("%s: IS_LEADER(ent) is %d\n", __func__, IS_LEADER(ent));
	// 	gi.dprintf("%s: ent->is_bot is %d\n", __func__, ent->is_bot);
	// 	gi.dprintf("%s: team_round_going is %d\n", __func__, team_round_going);

	// Basically we just want real, dead players who are in the respawn waiting period
	if (!ent->inuse ||
	ent->client->resp.team == NOTEAM ||
	ent->client->respawn_framenum <= 0 ||
	IS_LEADER(ent) ||
	ent->is_bot ||
	!team_round_going)
		return;

	if (ent->client->resp.team && !IS_ALIVE(ent)){
		int timercalc = (ent->client->respawn_framenum - level.framenum);

		if (esp_debug->value)
			gi.dprintf("%s: Level framenum is %d, respawn timer was %d for %s, timercalc is %i, esp_respawn_sounds was %i\n", 
			__func__, level.framenum, ent->client->respawn_framenum, ent->client->pers.netname, timercalc, ent->client->resp.esp_respawn_sounds);

		// Subtract current framenum from respawn_timer to get a countdown
		if (timercalc <= 0){
			// Play no sound, they've respawned by now
			return;
		} else if (timercalc <= (20 * FRAMEDIV) && ent->client->resp.esp_respawn_sounds == 2) {
			gi.centerprintf(ent, "CAMERA...");
			unicastSound(ent, gi.soundindex("atl/camera.wav"), 1.0);
			//gi.sound(ent, CHAN_VOICE, level.snd_camera, 1.0, ATTN_STATIC, 0.0);
			ent->client->resp.esp_respawn_sounds = 1;
			return;
		} else if (timercalc <= (40 * FRAMEDIV) && ent->client->resp.esp_respawn_sounds == 0) {
			gi.centerprintf(ent, "LIGHTS...");
			unicastSound(ent, gi.soundindex("atl/lights.wav"), 1.0);
			//gi.sound(ent, CHAN_VOICE, level.snd_lights, 1.0, ATTN_STATIC, 0.0);
			ent->client->resp.esp_respawn_sounds = 2;
			return;
		}
	} else {
		return;
	}
}

/* 
Espionage respawn logic, depends on modified respawn() to tell
the game to spawn the players at their leader spot
*/
void EspRespawnPlayer(edict_t *ent)
{
	// Leaders do not respawn
	if (IS_LEADER(ent))
		return;

	// Only respawn if the round is going
	if (team_round_going) {
		// Don't respawn until the current framenum is more than the respawn timer's framenum
		if (esp_debug->value)
			gi.dprintf("%s: Level framenum is %d, respawn timer was %d for %s\n", __func__, level.framenum, ent->client->respawn_framenum, ent->client->pers.netname);

		if (level.framenum > ent->client->respawn_framenum) {
			if (esp_debug->value) {
				gi.dprintf("Which mode are we in? ATL:0, ETV:1 :: Mode: %d\n", espsettings.esp_mode);
				gi.dprintf("Is team 1 leader alive? %d\n", IS_ALIVE(teams[TEAM1].leader));
				gi.dprintf("Is team 1's leader NULL? %d\n", teams[TEAM1].leader == NULL);
			}

			// Only respawn if leader(s) are still alive and the round is still going
			if (espsettings.esp_mode == ESPMODE_ATL) {
				if (teams[ent->client->resp.team].leader != NULL && IS_ALIVE(teams[ent->client->resp.team].leader)) {
					gi.centerprintf(ent, "ACTION!");
					unicastSound(ent, gi.soundindex("atl/action.wav"), 1.0);
					//gi.sound(ent, CHAN_VOICE, level.snd_action, 1.0, ATTN_STATIC, 0.0);
					ent->client->resp.esp_respawn_sounds = 0;
					respawn(ent);
				}

			} else if (espsettings.esp_mode == ESPMODE_ETV) {
				if (teams[TEAM1].leader != NULL && IS_ALIVE(teams[TEAM1].leader)) {
					gi.centerprintf(ent, "ACTION!");
					unicastSound(ent, gi.soundindex("atl/action.wav"), 1.0);
					//gi.sound(ent, CHAN_VOICE, level.snd_action, 1.0, ATTN_STATIC, 0.0);
					ent->client->resp.esp_respawn_sounds = 0;
					respawn(ent);
				}
			}
		}
	}
}

/*
Internally used only, spot check if leader is alive
depending on the game mode
Please run a NULL check on the leader before calling this
*/
qboolean _EspLeaderAliveCheck(edict_t *ent, edict_t *leader)
{
	if (espsettings.esp_mode < 0) {
		gi.dprintf("%s: Warning: Invalid espsettings.esp_mode returned\n", __func__);
		return false;
	}

	if (!leader)
		return false;

	if (espsettings.esp_mode == ESPMODE_ATL) {
		if (teams[leader->client->resp.team].leader &&
		IS_ALIVE(teams[leader->client->resp.team].leader))
			return true;
		else
			return false;
	}

	if (espsettings.esp_mode == ESPMODE_ETV) {
		if (ent->client->resp.team == TEAM1 &&
		IS_ALIVE(teams[TEAM1].leader))
			return true;
		else
			return false;
	}

	return false;
}

/*
This should return one spawnpoint coordinate for each team when round begins,
so that everyone per team spawns in the same place.  After that, it should
randomly choose a spawnpoint for each team, based on the es->custom_spawns[index#]

This is reset back to default in EspEndOfRoundCleanup()
*/
edict_t *SelectEspCustomSpawnPoint(edict_t* ent)
{
	espsettings_t *es = &espsettings;
    int teamNum = ent->client->resp.team;

	// An index has been set, so spawn there, else find a random one
	if (esp_spawnpoint_index[teamNum] >= 0)
		return es->custom_spawns[teamNum][esp_spawnpoint_index[teamNum]];
	
	// fresh round, so pick a random spawn
	else {
		int count = EspSpawnpointCount(teamNum);
		int random_index = rand() % count;
		if (count > 0) {
			do {
				random_index = (random_index + 1) % count; // Cycle through the spawn points
			} while (count > 1 && random_index == esp_last_chosen_spawn); // Keep generating a new index until it is different from the last one, unless there is only one spawn point
		} else {
			// If we count zero custom spawns, then we need to safely return a better function
			// so we can spawn teams apart
			if (esp_debug->value)
				gi.dprintf("%s: With zero spawnpoints, I am calling SelectRandomDeathmatchSpawnPoint()\n", __func__);
			return SelectRandomDeathmatchSpawnPoint();
		}
		// Keep track of which spawn was last chosen
		esp_last_chosen_spawn = random_index;
		esp_spawnpoint_index[teamNum] = esp_last_chosen_spawn;
		if (esp_debug->value)
			gi.dprintf("%s: For team %d, random index is %d\n", __func__, teamNum, esp_spawnpoint_index[teamNum]);
	}
	// Everyone on each team spawns on the same spawnpoint index (spawn together)
	edict_t *spawn_point = es->custom_spawns[teamNum][esp_spawnpoint_index[teamNum]];
	// if (spawn_point != NULL) {
	// 	gi.dprintf("For team %d, random index is %d, spawn coordinates were %f %f %f\n", teamNum, esp_spawnpoint_index[teamNum], spawn_point->s.origin[0], spawn_point->s.origin[1], spawn_point->s.origin[2]);
	// } else {
	// 	gi.dprintf("For team %d, random index is %d, but the spawn point is NULL\n", teamNum, esp_spawnpoint_index[teamNum]);
	// }

	if (spawn_point) {
		espsettings.round_spawnpoint[teamNum] = spawn_point;
		return spawn_point;
	} else {
		gi.dprintf("%s: No spawnpoint found, safely return NULL so we can try another one\n", __func__);
		return NULL;
	}
}

edict_t *EspRespawnOnLeader(edict_t *ent, char *cname)
{
	edict_t		*teamLeader, *spawn;
	vec3_t 		respawn_coords;

	teamLeader = teams[ent->client->resp.team].leader;
	VectorCopy(teamLeader->s.origin, respawn_coords);

	// Perform a trace to check if the spawn point is within a solid brush
    if (SV_TestEntityPosition(teamLeader)) {
		gi.dprintf("%s: %s: Team %d leader's spawn point was inside a solid brush, defaulting to original spawnpoint\n", __func__, ent->client->pers.netname, ent->client->resp.team);
		gi.cprintf(ent, PRINT_HIGH, "Your Leader is in a tight spot, you're unable to spawn near them!\n");
		return chosenSpawnpoint[ent->client->resp.team];
	} else {
		spawn = G_Spawn();
		respawn_coords[2] += 9; // So they don't spawn in the floor
		VectorCopy(respawn_coords, spawn->s.origin);
		spawn->s.angles[YAW] = teamLeader->s.angles[YAW]; // Facing the same direction as the leader on respawn
		spawn->classname = ED_NewString(cname);
		spawn->think = G_FreeEdict;
		spawn->nextthink = level.framenum + 1;
		//ED_CallSpawn(spawn);

		if (esp_debug->value)
			gi.dprintf("%s: %s on team %d respawn coordinates are %f %f %f %f\n", __func__, ent->client->pers.netname, ent->client->resp.team, teamLeader->s.origin[0], teamLeader->s.origin[1], teamLeader->s.origin[2], teamLeader->s.angles[YAW]);

		return spawn;
	}
}

edict_t *SelectEspSpawnPoint(edict_t *ent)
{
	//edict_t 	*spot, *spot1, *spot2;
	//int 		count = 0;
	//int 		selection;
	//float 		range, range1, range2;

	char 		*cname;
	int			teamNum = ent->client->resp.team;
	edict_t* 	chosen_spawn_point = NULL;

	ent->client->resp.esp_state = ESP_STATE_PLAYING;

	switch (teamNum) {
		case TEAM1:
			cname = "info_player_team1";
			break;
		case TEAM2:
			cname = "info_player_team2";
			break;
		case TEAM3:
			cname = "info_player_deathmatch";
			break;
		default:
			cname = "info_player_deathmatch";
			break;
	}

	/* Regular initial round spawn logic:
	Check if this is a respawn or a round start spawn
	Respawn Logic: 
	1. It's ETV mode
	2. You are on TEAM1
	3. TEAM1's leader is alive
	4. Then spawn at the leader's location
	(if you are on TEAM2, you respawn at your original spawnpoint)
		or
	1. It's ATL mode
	2. Your leader is alive
	3. Then spawn at your leader's location

	If none of this is true, it's a round start spawn
	*/

	if (in_warmup)
		return SelectDeathmatchSpawnPoint();

	if (esp_debug->value){
		gi.dprintf("%s: Is team round going? %d\n", __func__, team_round_going);
		gi.dprintf("%s: Is the team leader alive? %d\n", __func__, _EspLeaderAliveCheck(ent, teams[ent->client->resp.team].leader));
	}

	// Time to respawn on the leader!
	if (team_round_going && _EspLeaderAliveCheck(ent, teams[ent->client->resp.team].leader)) {
		return EspRespawnOnLeader(ent, cname);
	} else {
		// Custom spawns take precedence over standard spawns
		if ((EspSpawnpointCount(teamNum) > 0)) {
			chosen_spawn_point = SelectEspCustomSpawnPoint(ent);
			// Keeping track of the chosen spawnpoint
			chosenSpawnpoint[teamNum] = chosen_spawn_point;
			return chosen_spawn_point;

		// but if there are none, then we go back to old faithful
		} else {
			if (esp_debug->value)
				gi.dprintf("%s: No custom spawns, defaulting to teamplay spawn\n", __func__);

			// NULL check, if there are no teamplay (info_player_team...) spawns, then default to deathmatch spawns
			if (SelectTeamplaySpawnPoint(ent)) {
				chosen_spawn_point = SelectTeamplaySpawnPoint(ent);
				// Keeping track of the chosen spawnpoint
				chosenSpawnpoint[teamNum] = chosen_spawn_point;
				return chosen_spawn_point;
			} else {
				chosen_spawn_point = SelectDeathmatchSpawnPoint();
				// Keeping track of the chosen spawnpoint
				chosenSpawnpoint[teamNum] = chosen_spawn_point;
				return chosen_spawn_point;
			}
		}
	}
	// All else fails, use deathmatch spawn points
	if (esp_debug->value)
		gi.dprintf("%s: Defaulted all the way to using SelectFarthestDeathmatchSpawnPoint\n", __func__);
	return SelectFarthestDeathmatchSpawnPoint();
}

void SetEspStats( edict_t *ent )
{
	int i;

	// GHUD team icons, ATL gets leader skin indexes, ETV gets team skin indexes
	for(i = TEAM1; i <= teamCount; i++)
		if (espsettings.esp_mode == ESPMODE_ETV)
			level.pic_teamskin[i] = gi.imageindex(teams[i].skin_index);
		else if (espsettings.esp_mode == ESPMODE_ATL)
			level.pic_teamskin[i] = gi.imageindex(teams[i].leader_skin_index);

	// Load scoreboard images
	level.pic_esp_teamtag[TEAM1] = gi.imageindex(teams[TEAM1].skin_index);
	level.pic_esp_teamicon[TEAM1] = gi.imageindex(teams[TEAM1].skin_index);
	level.pic_esp_leadericon[TEAM1] = gi.imageindex(teams[TEAM1].leader_skin_index);
	gi.imageindex("sbfctf1");

	level.pic_esp_teamtag[TEAM2] = gi.imageindex(teams[TEAM2].skin_index);
	level.pic_esp_teamicon[TEAM2] = gi.imageindex(teams[TEAM2].skin_index);
	level.pic_esp_leadericon[TEAM2] = gi.imageindex(teams[TEAM2].leader_skin_index);
	gi.imageindex("sbfctf2");

	if (espsettings.esp_mode == ESPMODE_ATL && teamCount == 3) {
		level.pic_esp_teamtag[TEAM3] = gi.imageindex(teams[TEAM3].skin_index);
		level.pic_esp_teamicon[TEAM3] = gi.imageindex(teams[TEAM3].skin_index);
		level.pic_esp_leadericon[TEAM3] = gi.imageindex(teams[TEAM3].leader_skin_index);
		gi.imageindex("sbfctf3");
	}

	// Now set the HUD
	// Set the new respawn icon indicator
	ent->client->ps.stats[STAT_TIMER_ICON] = level.pic_esp_respawn_icon;

	ent->client->ps.stats[ STAT_TEAM1_HEADER ] = level.pic_esp_teamtag[ TEAM1 ];
	ent->client->ps.stats[ STAT_TEAM2_HEADER ] = level.pic_esp_teamtag[ TEAM2 ];

	// Team scores for the score display and HUD.
	ent->client->ps.stats[ STAT_TEAM1_SCORE ] = teams[ TEAM1 ].score;
	ent->client->ps.stats[ STAT_TEAM2_SCORE ] = teams[ TEAM2 ].score;

	// Team icons for the score display and HUD.
	ent->client->ps.stats[ STAT_TEAM1_PIC ] = level.pic_esp_teamicon[ TEAM1 ];
	ent->client->ps.stats[ STAT_TEAM2_PIC ] = level.pic_esp_teamicon[ TEAM2 ];
	// ent->client->ps.stats[ STAT_TEAM1_LEADERPIC ] = level.pic_esp_leadericon[ TEAM1 ];
	// ent->client->ps.stats[ STAT_TEAM2_LEADERPIC ] = level.pic_esp_leadericon[ TEAM2 ];

	if (teamCount == 3) {
		ent->client->ps.stats[ STAT_TEAM3_SCORE ] = teams[ TEAM3 ].score;
		ent->client->ps.stats[ STAT_TEAM3_PIC ] = level.pic_esp_teamicon[ TEAM3 ];
		//ent->client->ps.stats[ STAT_TEAM3_LEADERPIC ] = level.pic_esp_leadericon[ TEAM3 ];
	}

	// Shows the timer and icon, but does not count down if ent is Leader, it stays at 0

	if (ent->client->respawn_framenum > 0 &&
	ent->client->respawn_framenum - level.framenum > 0){
		ent->client->ps.stats[STAT_TIMER_ICON] = level.pic_esp_respawn_icon;
		if (!IS_LEADER(ent))
			ent->client->ps.stats[STAT_TIMER] = (ent->client->respawn_framenum - level.framenum) / HZ;
		else
			ent->client->ps.stats[STAT_TIMER] = 0;
	}

	// During gameplay, flash your team's icon
	if( (team_round_going || lights_camera_action > 0) && ((level.realFramenum / FRAMEDIV) & 4) )
	{
		if (ent->client->resp.team == TEAM1)
			ent->client->ps.stats[ STAT_TEAM1_PIC ] = 0;
		else if (ent->client->resp.team == TEAM2)
			ent->client->ps.stats[ STAT_TEAM2_PIC ] = 0;
		else if (ent->client->resp.team == TEAM3)
			ent->client->ps.stats[ STAT_TEAM3_PIC ] = 0;
	}

	// During intermission, blink the team icon of the winning team.
	if( level.intermission_framenum && ((level.realFramenum / FRAMEDIV) & 8) )
	{
		if (esp_winner == TEAM1)
			ent->client->ps.stats[ STAT_TEAM1_PIC ] = 0;
		else if (esp_winner == TEAM2)
			ent->client->ps.stats[ STAT_TEAM2_PIC ] = 0;
		else if (teamCount == 3 && esp_winner == TEAM3)
			ent->client->ps.stats[ STAT_TEAM3_PIC ] = 0;
	}

	ent->client->ps.stats[STAT_ID_VIEW] = 0;
	if (!ent->client->pers.id)
		SetIDView(ent);
}

void EspSwapTeams(void)
{
	edict_t *ent;
	int i;

	// Swap members of both teams and strip leadership from TEAM1
	for (i = 0; i < game.maxclients; i++) {
		ent = &g_edicts[1 + i];
		if (ent->inuse && ent->client->resp.team) {
			EspLeaderLeftTeam(ent);
			ent->client->resp.is_volunteer = false;
			ent->client->resp.team = OtherTeam(ent->client->resp.team);
			AssignSkin(ent, teams[ent->client->resp.team].skin, false);
		}
	}

	/* swap scores too! */
	i = teams[TEAM1].score;
	teams[TEAM1].score = teams[TEAM2].score;
	teams[TEAM2].score = i;

	teams_changed = true;
}

void KillEveryone(int teamNum)
{
	edict_t *ent;
	int i;

	for (i = 0; i < game.maxclients; i++){
		ent = &g_edicts[1 + i];
		if (!ent->inuse)
			continue;
		if(ent->solid == SOLID_NOT && !ent->deadflag)
			continue;
		if(IS_LEADER(ent)) // Don't kill a dead leader
			continue;
		if (ent->client->resp.team == teamNum){
			killPlayer(ent, false);
		}
	}
}

qboolean EspCheckETVRules(void)
{
	int t1 = teams[TEAM1].score;
	int t2 = teams[TEAM2].score;
	int roundlimitwarn = (((int)roundlimit->value / 2) - 1);

	// Espionage ETV uses the same roundlimit cvars as teamplay
	if(roundlimit->value && 
	(teams[TEAM1].score >= roundlimit->value || 
	teams[TEAM2].score >= roundlimit->value) ){
		gi.bprintf(PRINT_HIGH, "Roundlimit hit.\n");
		IRC_printf(IRC_T_GAME, "Roundlimit hit.\n");
		return true;
	}
	// Must be etv mode, halftime must have not occured yet, and be enabled, and the roundlimit must be set

	// Print all condition states
	if (esp_debug->value) {
		gi.dprintf("-- Debug start %s --\n", __func__);
		gi.dprintf("Roundlimit is %f\n", roundlimit->value);
		gi.dprintf("Team 1 score is %d\n", teams[TEAM1].score);
		gi.dprintf("Team 2 score is %d\n", teams[TEAM2].score);
		gi.dprintf("Team 1 + Team 2 score is %d\n", t1 + t2);
		gi.dprintf("Roundlimit warning is %d\n", roundlimitwarn);
		gi.dprintf("Halftime is %d\n", espsettings.halftime);
		gi.dprintf("ETV is %d\n", espsettings.esp_mode);
		gi.dprintf("Use warnings is %f\n", use_warnings->value);
		gi.dprintf("ETV halftime is %f\n", esp_etv_halftime->value);
		gi.dprintf("-- Debug end %s --\n", __func__);
	}

	if (!roundlimit->value && esp_etv_halftime->value){
		// No roundlimit means no halftime either
		disablecvar(esp_etv_halftime, "No Roundlimit");
		return false;
	}

	// This is checking for conditions involving halftime
	if (esp_etv_halftime->value) {
		static qboolean halftimeMessageAdded = false;

		if(!espsettings.halftime && (t1 + t2 == roundlimitwarn)){
			if( use_warnings->value && !halftimeMessageAdded ){
				gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex("world/incoming.wav"), 1.0, ATTN_NONE, 0.0 );
				EspTimedMessageHandler(TEAM1, NULL, 3, ESP_HALFTIME_WARNING);
				halftimeMessageAdded = true;
			}
		} else if(!espsettings.halftime && (t1 + t2 == (roundlimit->value / 2))){
			team_round_going = team_round_countdown = team_game_going = 0;
			MakeAllLivePlayersObservers();
			EspSwapTeams();
			CenterPrintLevelTeam(TEAM1, PRINT_LOW, "The teams have been switched!\nYour team needs a leader to volunteer!\n");
			CenterPrintLevelTeam(TEAM2, PRINT_LOW, "The teams have been switched!\nYou are now defending!\n");
			gi.sound(&g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD,
					gi.soundindex("misc/secret.wav"), 1.0, ATTN_NONE, 0.0);
			espsettings.halftime = 1;
		}
	}

	// Default return false, which includes !esp_etv_halftime->value
	return false;
}

qboolean EspCheckRules(void)
{
	// Expand if we introduce other Espionage modes
	if (espsettings.esp_mode == ESPMODE_ETV)
		return EspCheckETVRules();
	
	return false;
}

/*
Skin Check
Validates players and leaders have the correct skin for their team
*/
void EspSkinCheck(void)
{
	edict_t *ent;
	int i;

	// If we don't have proper leaders, don't run this
	if (!EspLeaderCheck()) {
		gi.dprintf("Leaders not found, not re-skinning");
		return;
	}

	for (i = 0; i < game.maxclients; i++) {
		ent = &g_edicts[1 + i];
		if (!ent->inuse || ent->client->resp.team == NOTEAM)
			continue;
		if (ent->client->resp.team == TEAM1) {
			if (!IS_LEADER(ent)) {
				AssignSkin(ent, teams[TEAM1].skin, false);
			} else {
				AssignSkin(ent, teams[TEAM1].leader_skin, false);
			}
		} else if (ent->client->resp.team == TEAM2) {
			if (!IS_LEADER(ent)) {
				AssignSkin(ent, teams[TEAM2].skin, false);
			} else {
				AssignSkin(ent, teams[TEAM2].leader_skin, false);
			}
		} else if (ent->client->resp.team == TEAM3) {
			if (!IS_LEADER(ent)) {
				AssignSkin(ent, teams[TEAM3].skin, false);
			} else {
				AssignSkin(ent, teams[TEAM3].leader_skin, false);
			}
		}
	}
}

/*
This check is similiar to checking that all teams have
Captains in matchmode
*/
qboolean AllTeamsHaveLeaders(void)
{
	int teamsWithLeaders = 0;
	int i = 0;

	//AQ2:TNG Slicer Matchmode
	if (matchmode->value && !TeamsReady())
		return false;
	//AQ2:TNG END

	for (i = TEAM1; i <= teamCount; i++){
		if (HAVE_LEADER(i)) {
			teamsWithLeaders++;
		}
	}

	// Only Team 1 needs a leader in ETV mode
	if((espsettings.esp_mode == ESPMODE_ETV) && HAVE_LEADER(TEAM1)) {
		if (esp_debug->value)
		  gi.dprintf("ETV team has a leader\n");
		teams[TEAM1].leader_dead = false;  // Reset this in case the leader left and returned
		return true;
	} else if(espsettings.esp_mode == ESPMODE_ATL && (teamsWithLeaders == teamCount)){
		if (esp_debug->value)
			gi.dprintf("Teams with leaders is the same as the team count\n");
		teams[TEAM1].leader_dead = false;  // Reset this in case the leader left and returned
		teams[TEAM2].leader_dead = false;
		if (teamCount == 3)
			teams[TEAM3].leader_dead = false;
	} else {
		return false;
	}


	if (esp_debug->value)
		gi.dprintf("%s: Leadercount: %d\n", __func__, teamsWithLeaders);
	return false;
}

edict_t* EspGetLeader(int teamNum)
{
	if (teamNum == NOTEAM)
		return NULL;

	if (teams[teamNum].leader == NULL){
		if (esp_debug->value)
			gi.dprintf("%s: No leader found for team %d\n", __func__, teamNum);
		return NULL;
	}

	return teams[teamNum].leader;
}

qboolean EspSetLeader(int teamNum, edict_t *ent)
{
	edict_t *oldLeader = teams[teamNum].leader;
	char temp[128];

	if (teamNum == NOTEAM){
		ent = NULL;
		return false;
	}

	if (espsettings.esp_mode == ESPMODE_ETV && teamNum != TEAM1) {
		gi.cprintf(ent, PRINT_MEDIUM, "** Only the Red team (team 1) has a leader in ETV mode **\n");
		return false;
	}

	// NULL check and checks if leadertime is more than 0 and less than 10 seconds ago
	if ((ent && ent->client->resp.esp_leadertime > 0) && 
	(level.realFramenum - ent->client->resp.esp_leadertime < 10 * HZ)){
		if (IS_CAPTAIN(ent)){ // This is to avoid printing this message when becoming captain
			gi.cprintf(ent, PRINT_HIGH, "** You must wait 10 seconds between toggling your leader role! **\n");
		}
		return false;
	}


	teams[teamNum].leader = ent;
	// if(ent) // Only assign a skin to an ent
	// 	AssignSkin(ent, teams[teamNum].leader_skin, false);

	if (!ent) {
		if (!team_round_going || (gameSettings & GS_ROUNDBASED)) {
			if (teams[teamNum].ready) {
				Q_snprintf( temp, sizeof( temp ), "%s has lost their leader and is no longer ready to play!", teams[teamNum].name );
				CenterPrintAll( temp );
			}
			teams[teamNum].ready = 0;
		}
		if (oldLeader) {
			Q_snprintf(temp, sizeof(temp), "%s is no longer %s's leader\n", oldLeader->client->pers.netname, teams[teamNum].name );
			CenterPrintAll(temp);
			//gi.bprintf( PRINT_HIGH, "%s needs a new leader!  Enter 'volunteer' to apply for duty\n", teams[teamNum].name );
		}
		teams[teamNum].locked = 0;
	}

	if (ent != oldLeader && ent != NULL) {
		Q_snprintf(temp, sizeof(temp), "%s is now %s's leader\n", ent->client->pers.netname, teams[teamNum].name );
		CenterPrintAll(temp);
		gi.cprintf( ent, PRINT_CHAT, "You are the leader of '%s'\n", teams[teamNum].name );
		unicastSound(ent, gi.soundindex("aqdt/leader.wav"), 1.0);
		//gi.sound( &g_edicts[0], CHAN_VOICE | CHAN_NO_PHS_ADD, gi.soundindex( "misc/comp_up.wav" ), 1.0, ATTN_NONE, 0.0 );
		AssignSkin(ent, teams[teamNum].leader_skin, false);
		// Set the time the player became leader so they can't unleader immediately after
		ent->client->resp.esp_leadertime = level.realFramenum;
		return true;
	}

	return false;
}

/*
Call this if the leader of a team disconnects/leaves
and there are no active volunteers for that team
*/
qboolean EspChooseRandomLeader(int teamNum)
{
	int players[TEAM_TOP] = { 0 }, i, numPlayers = 0;
    edict_t *ent, *playerList[MAX_CLIENTS];

	if (team_round_going) {
		if (esp_debug->value)
			gi.dprintf("%s: I was called because someone disconnected\n", __func__);

		if (matchmode->value && !TeamsReady())
			return false;

		// Count the number of players on the team and add them to the playerList
		for (i = 0; i < game.maxclients; i++) {
			ent = &g_edicts[1 + i];
			// Must be on a team and alive, as leaders can't respawn, choosing a dead ent will stop the game
			if (!ent->inuse || game.clients[i].resp.team == NOTEAM || !IS_ALIVE(ent))
				continue;
			if (!game.clients[i].resp.subteam && game.clients[i].resp.team == teamNum) {
				players[teamNum]++;
				playerList[numPlayers++] = ent;
			}
		}

		// If no players are left on the team, return
		if (players[teamNum] == 0)
			return false;

		if (esp_debug->value)
			gi.dprintf("%s: players on team %d: %d\n", __func__, teamNum, players[teamNum]);

		// Choose a random player from the playerList
		ent = playerList[rand() % numPlayers];

		if (esp_debug->value)
			gi.dprintf("%s: Randomly selected player on team %d: %s\n", __func__, teamNum, ent->client->pers.netname);

		// Set the selected player as the leader
		EspSetLeader(teamNum, ent);
		return true;
	} // If we get this far, that the only players here are bots and non-volunteers.  Halt the game
	return false;
}

/*
Check if this team has any volunteers, choose them first before
randomly choosing non-volunteers.  Returns NULL if none are found.
*/
edict_t *EspVolunteerCheck(int teamNum)
{
	int i = 0;
	edict_t *ent;

	for (i = 0; i < game.maxclients; i++) {
        ent = g_edicts + 1 + i;
        if (ent->inuse && ent->client->resp.team == teamNum && ent->client->resp.is_volunteer) {
            return ent;
        }
    }
	// No volunteer found
    return NULL;
}

/*
Check if each team has a leader, if not, choose a volunteer, else choose one at random
This should only fail if there is no one to choose
*/
qboolean EspLeaderCheck(void)
{
	int i = 0;
	edict_t *newLeader;
	qboolean athl = AllTeamsHaveLeaders();
	int t1count = TotalPlayersOnTeam(TEAM1);
	int t2count = TotalPlayersOnTeam(TEAM2);

	// Setting leader deadness to true if there's no one left on the team
	// This is meant to end the round immediately if a team has no members on it
	if (teamCount == 2){
		if (t1count == 0 && t2count > 0){
			teams[TEAM1].leader_dead = true;
		} else if (t2count == 0 && t1count > 0){
			teams[TEAM2].leader_dead = true;
		}
	} else if (teamCount == 3){
		int t3count = TotalPlayersOnTeam(TEAM3);
		if (t1count == 0 && t2count > 0 && t3count > 0){
			teams[TEAM1].leader_dead = true;
		} else if (t2count == 0 && t1count > 0 && t3count > 0){
			teams[TEAM2].leader_dead = true;
		} else if (t3count == 0 && t1count > 0 && t2count > 0){
			teams[TEAM3].leader_dead = true;
		}
	}

	// Quick sanity check in case this ever occurs as I've seen it on rare occasions
	for (i = TEAM1; i <= teamCount; i++) {
		if (teams[i].leader && !IS_ALIVE(teams[i].leader) && team_round_going && !holding_on_tie_check) {
			if (esp_debug->value)
				gi.dprintf("%s: Team %i leader is dead, but the round is still going...?\n", __func__, teams[i].leader->client->resp.team);
			athl = false;
		}
	}

	// Bot scan, rotate leaders if a leader is a bot and it died last round
	for (i = TEAM1; i <= teamCount; i++) {
		if (teams[i].leader && teams[i].leader->is_bot && teams[i].leader_dead) {
			EspChooseRandomLeader(i);
		}
	}

	// If we all have leaders, then we're good
	if (athl) {
		if (esp_debug->value)
			gi.dprintf("%s: I don't need a leader!\n", __func__);
		return true;
	} else {
		if (esp_debug->value)
			gi.dprintf("%s: I need a leader!\n", __func__);
		// We do not all have leaders, so we must cycle through each team
		for (i = TEAM1; i <= teamCount; i++) {
			if (!HAVE_LEADER(i)) { // If this team does not have a leader, get one
				newLeader = EspVolunteerCheck(i);
				if (newLeader) {
					if (EspSetLeader(i, newLeader)) {
						if (esp_debug->value)
							gi.dprintf("%s: I found a leader!\n", __func__);
						return true;
					}
				} else {  // Oops, no volunteers, then we force someone to be a leader
					if (EspChooseRandomLeader(i)) {
						if (esp_debug->value)
							gi.dprintf("%s: I need a random leader!\n", __func__);
						return true;
					} else {
						return false;
					}
				}

				// If we still don't have a leader, then the next round can't begin
				if (!newLeader) {
					if (team_round_going){
						gi.bprintf( PRINT_HIGH, "%s no longer has a leader, they can not win this round\n", teams[i].name );
						teams[i].leader_dead = true; // This should trigger round end
						KillEveryone(i);
					}
					teams[i].leader = NULL; // Clear this in case some strange things happen
					gi.bprintf( PRINT_HIGH, "%s needs a new leader!  Enter 'leader' to apply for duty\n", teams[i].name );
				}
			}
		}
	}
	return false;
}

void EspLeaderLeftTeam( edict_t *ent )
{
	int teamNum = ent->client->resp.team;

	ent->client->resp.is_volunteer = false;
	if (!IS_LEADER(ent)){
		return;
	} else {
		EspSetLeader( teamNum, NULL );

		ent->client->resp.subteam = 0;

		if (!teams[teamNum].leader) {
			EspLeaderCheck();
		}
	}
}

/*
Called from EspReportLeaderDeath in ATL mode
*/
static int EspReportATLWinner(int dead_leader_team)
{
	int winner = 0;

	switch (dead_leader_team) {
		case TEAM1:
			if (IS_ALIVE(teams[TEAM2].leader) && !IS_ALIVE(teams[TEAM3].leader))
				winner = TEAM2;
			else if (!IS_ALIVE(teams[TEAM2].leader) && IS_ALIVE(teams[TEAM3].leader))
				winner = TEAM3;
			break;
		case TEAM2:
			if (IS_ALIVE(teams[TEAM1].leader) && !IS_ALIVE(teams[TEAM3].leader))
				winner = TEAM1;
			else if (!IS_ALIVE(teams[TEAM1].leader) && IS_ALIVE(teams[TEAM3].leader))
				winner = TEAM3;
			break;
		case TEAM3:
			if (IS_ALIVE(teams[TEAM1].leader) && !IS_ALIVE(teams[TEAM2].leader))
				winner = TEAM1;
			else if (!IS_ALIVE(teams[TEAM1].leader) && IS_ALIVE(teams[TEAM2].leader))
				winner = TEAM2;
			break;
	}

	return winner;
}

/*
This is called from player_die, and only called
if the player was a leader
*/
int EspReportLeaderDeath(edict_t *ent)
{
	// Get the team the leader was on
	int dead_leader_team = ent->client->resp.team;
	int winner = 0;

	// Set the dead leader val
	teams[dead_leader_team].leader_dead = true;

	// This checks if leader was on TEAM 1 in ETV mode
	if (espsettings.esp_mode == ESPMODE_ETV) {
		if (dead_leader_team == TEAM1) {
			winner = TEAM2;
		}
	}

	// ATL mode - 2 team winner checks
	if (espsettings.esp_mode == ESPMODE_ATL) {
		if (teamCount == 2) {
			if (dead_leader_team == TEAM1)
				winner = TEAM2;
			else if (dead_leader_team == TEAM2)
				winner = TEAM1;
		// 3 team winner checks
		} else {
			winner = EspReportATLWinner(dead_leader_team);
		}
	}

	// Find all players in the game and play this sound
	gi.sound(&g_edicts[0], CHAN_BODY | CHAN_NO_PHS_ADD, gi.soundindex("tng/leader_death.wav"), 1.0, ATTN_NONE, 0.0);
	if (esp_punish->value) {
		esp_punishment_phase = true;
		EspPunishment(dead_leader_team);
	}

	// Stats Reset
	if (ent->client->resp.esp_capstreak > ent->client->resp.esp_capstreakbest)
		ent->client->resp.esp_capstreakbest = ent->client->resp.esp_capstreak;
	ent->client->resp.esp_capstreak = 0;
	
	// gi.dprintf("\n\nAdded message handler\n\n");
	// EspTimedMessageHandler(0, NULL, 15, ESP_LEADER_DIED);

	return winner;
}

void MakeTeamInvulnerable(int winner, int uvtime)
{
	edict_t *ent;
	int i = 0;

	for (i = 0; i < game.maxclients; i++){
		ent = &g_edicts[1 + i];
		// Make alive clients invulnerable
		if ((game.clients[i].resp.team == winner) && (IS_ALIVE(ent))){
			ent->client->uvTime = uvtime;
		}
	}
}

void EspPunishment(int teamNum)
{
	// Only perform team punishments if there's only 2 teams
	if (esp->value && teamCount == 2){
		if(esp_punish->value == 1){
			// Immediately kill all losing members of the remaining team
			KillEveryone(teamNum);
		} else if (esp_punish->value == 2){
			// Grant uv shield to winning team
			int uvtime = 60;
			MakeTeamInvulnerable(OtherTeam(teamNum), uvtime);
		}
	}
}

/*
This adds a medkit to the player's inventory, up to the medkit_max value
Passing a true parameter instantly provides this medkit
Passing a false parameter assumes you want to generate one at a time interval set by medkit_time
*/
void GenerateMedKit(qboolean instant)
{
	int i = 0;
	edict_t *ent;
	int roundseconds = current_round_length / 10;
	int interval = (int)medkit_time->value;
	int max_kits = (int)medkit_max->value;

	// Do nothing if the ent already has max medkits
	for (i = 0; i < game.maxclients; i++) {
		ent = &g_edicts[1 + i];
		if (IS_LEADER(ent)) {
			if (ent->client->medkit >= max_kits)
				return;
			else if (roundseconds - ent->client->resp.medkit_award_time >= interval) {
				ent->client->resp.medkit_award_time = roundseconds;
				ent->client->medkit++;

				if (esp_debug->value){
					gi.dprintf("%s: I generated a medkit for %s, %d seconds since the round started, %d frames in\n", __func__, ent->client->pers.netname, roundseconds, roundseconds);
					gi.dprintf("%s: %s has %d medkits now\n", __func__, ent->client->pers.netname, ent->client->medkit);
				}
			}
		}
	}
}

void EspSetupStatusbar( void )
{
	Q_strncatz(level.statusbar,
		// Respawn indicator
		"yb -220 " "if 9 xr -28 pic 9 endif " "xr -100 num 4 10 ",
		sizeof(level.statusbar) );

	Q_strncatz(level.statusbar, 
		// Red Team
		"yb -172 " "if 24 xr -32 pic 24 endif " "xr -100 num 4 26 "
		// Blue Team
		"yb -140 " "if 25 xr -32 pic 25 endif " "xr -100 num 4 27 ",
		sizeof(level.statusbar) );
	
	if( teamCount >= 3 )
	{
		Q_strncatz(level.statusbar, 
			// Green Team
			"yb -108 " "if 30 xr -32 pic 30 endif " "xr -100 num 4 31 ",
			sizeof(level.statusbar) );
	}
}

void EspAnnounceDetails( qboolean timewarning )
{
	int i;
	edict_t *ent;

	// This is warning the players that the round is about to end
	// and they need to accomplish their goals
	if (timewarning){
		for (i = 0; i < game.maxclients; i++){
			ent = g_edicts + 1 + i;
			if (!ent->inuse || ent->is_bot || ent->client->resp.team == NOTEAM)
				continue;
			if (espsettings.esp_mode == ESPMODE_ATL){
				CenterPrintAll("You're running low on time! Kill the enemy leader!\n");
			} else if (espsettings.esp_mode == ESPMODE_ETV){
				CenterPrintTeam(TEAM1, "Capture that briefcase or the other team wins!\n");
				CenterPrintTeam(TEAM2, "Keep it up! If they can't cap, they can't win!\n");
			}
		}
	}

	// This is used at the beginning of the round to tell players what to do
	if (!timewarning) {
		for (i = 0; i < game.maxclients; i++){
			ent = g_edicts + 1 + i;
			if (!ent->inuse)
				continue;
			if (IS_LEADER(ent)){
				//gi.sound(ent, CHAN_VOICE, gi.soundindex("aqdt/leader.wav"), 1, ATTN_STATIC, 0);
				unicastSound(ent, gi.soundindex("aqdt/leader.wav"), 1.0);
				gi.cprintf(ent, PRINT_MEDIUM, "Take cover, you're the leader!\n");
			}
			if (!IS_LEADER(ent)) {
				if (espsettings.esp_mode == ESPMODE_ATL){
					gi.cprintf(ent, PRINT_MEDIUM, "Defend your leader and attack the other one to win!\n");
				} else if (espsettings.esp_mode == ESPMODE_ETV){
					if (ent->client->resp.team == TEAM1)
						gi.cprintf(ent, PRINT_HIGH, "Escort your leader to the briefcase!\n");
					else
						gi.cprintf(ent, PRINT_HIGH, "Kill the enemy leader to win!\n");
				}
			}
		}
	}
}

/*
Because some rounds continue for a while, let's clean up the bodies periodically
*/
int cleanupInterval = 20; // Initial value
void EspCleanUp(void)
{
	int roundseconds = current_round_length / 10;
	int intervalAdd = 20;

	if (esp_debug->value)
		gi.dprintf("%s: Cleanupinterval is %d, the time is now %d\n", __func__, cleanupInterval, roundseconds);
	if (roundseconds >= cleanupInterval) {
		CleanBodies();
		cleanupInterval = cleanupInterval + intervalAdd;

		if (esp_debug->value)
			gi.dprintf("%s: Bodies cleaned, cleanupinterval is now %d\n", __func__, cleanupInterval);
	}
}

/*
Call this at the end of the round to get us back to a good state
*/
void EspEndOfRoundCleanup(void)
{
	int i = 0;
	// Reset leader_dead for all teams before next round starts
	// as well as any bots that were leaders, somehow
	for (i = TEAM1; i <= teamCount; i++) {
		if (teams[i].leader && teams[i].leader->is_bot)
			teams[i].leader = NULL;
		if (TotalPlayersOnTeam(i) == 0)  //Clears leader edict if no one is left on the team
			teams[i].leader = NULL;
		teams[i].leader_dead = false;
	}

	// Check that we have leaders for the next round
	EspLeaderCheck();

	// Reset cleanup interval
	cleanupInterval = 20;

	// Reset the last spawnpoint index, this is reset to 
	// -1 because 0 is a valid index
	// Also reset last round's spawnpoint edict
	for (i = TEAM1; i <= teamCount; i++) {
		esp_spawnpoint_index[i] = -1;
		espsettings.round_spawnpoint[i] = NULL;
	}

	// Reset these three timed stats for the next round for each player
	edict_t *ent;
	for (i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		ent->client->resp.esp_lastprotectcap = 0;
		ent->client->resp.esp_lastprotectleader = 0;
		ent->client->resp.esp_lasthurtleader = 0;
	}

	/* 
	Note:  Resetting the ETV escort point is not done here,
	it is performed in EspResetCapturePoint() and MUST be called at 
	the _beginning_ of the round, not the end.  Calling it at the 
	end of the round has the possibility of recapturing the point
	after the round has ended but before the next round begins
	*/
}

/*

*/
int EspSpawnpointCount(int teamNum)
{
    espsettings_t *es = &espsettings;
    int i;
    int spawn_count = 0;

    // Get count of how many team spawns there are in espsettings.customspawns
    for (i = 0; i < MAX_SPAWNS; i++) {
        if (es->custom_spawns[teamNum][i]) {
            if (es->custom_spawns[teamNum][i] == NULL)
                break;
			if (esp_debug->value)
				gi.dprintf("%s: Team %d spawncount %d coordinates are: %f, %f, %f\n", __func__, teamNum, i, es->custom_spawns[teamNum][i]->s.origin[0], es->custom_spawns[teamNum][i]->s.origin[1], es->custom_spawns[teamNum][i]->s.origin[2]);
            spawn_count++;
        }
    }

	if (esp_debug->value)
    	gi.dprintf("%s: Team %d has %d custom spawnpoints\n", __func__, teamNum, spawn_count);
    return spawn_count;
}

void EspDebug(void)
{
	// Debugging:
	int i;
	for (i = TEAM1; i <= teamCount; i++) {

		if (HAVE_LEADER(i)) {
			gi.dprintf("%s: Team %i leader: %s\n", __func__, i, teams[i].leader->client->pers.netname);
		} else {
			gi.dprintf("%s: Team %d does not have a leader\n", __func__, i);
		}
	}
	edict_t *ent;
	int entcount = 0;
	for (i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		if (ent->client->resp.is_volunteer) {
			gi.dprintf("%s is a volunteer for team %i\n", ent->client->pers.netname, ent->client->resp.team);
		}
		if (ent->client->resp.team == TEAM1){
			gi.dprintf("%s is on team 1\n", ent->client->pers.netname);
			entcount++;
		}
		if (ent->client->resp.team == TEAM2){
			gi.dprintf("%s is on team 2\n", ent->client->pers.netname);
			entcount++;
		}
		if (ent->client->resp.team == NOTEAM){
			gi.dprintf("%s is not on a team\n", ent->client->pers.netname);
			entcount++;
		}
	}
	gi.dprintf("There are %d players\n", entcount);

	// Respawn timers
	for (i = TEAM1; i <= teamCount; i++) {
		gi.dprintf("Spawn timer for team %d: %d\n", i, teams[i].respawn_timer);
	}

	for (i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		if (ent->client->resp.team == TEAM1){
			gi.dprintf("%s has %d frames left to respawn\n", ent->client->pers.netname, ent->client->respawn_framenum - level.realFramenum);
		}
		if (ent->client->resp.team == TEAM2){
			gi.dprintf("%s has %d frames left to respawn\n", ent->client->pers.netname, ent->client->respawn_framenum - level.realFramenum);
		}
	}
}