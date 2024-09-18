#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

void BOTLIB_Init(edict_t* self)
{
	self->classname = "bot";
	self->suicide_timeout = level.framenum + 15.0 * HZ;

	self->nav = NULL; //rekkie -- surface data

	self->enemy = NULL;
	self->movetarget = NULL;
	self->last_jumppad_node = INVALID;
	self->bot_strafe = 0;
	self->show_node_links = INVALID;
	self->show_node_links_time = 0;

	//RiEvEr - new node pathing system
	memset(&(self->pathList), 0, sizeof(self->pathList));
	self->pathList.head = self->pathList.tail = NULL;
	//R

	self->client->resp.radio.gender = (self->client->pers.gender == GENDER_FEMALE) ? 1 : 0;

	// Save previous data (TODO: copy entire bot.skill struct)
	float prev_skill = self->bot.skill.overall;
	int pId = self->bot.personality.pId;

	memset(&self->bot, 0, sizeof(bot_t));

	// Restore previous data
	self->bot.skill.overall = prev_skill;
	self->bot.personality.pId = pId;

	// Ping
	// Set the average ping this bot will see
	int rng_ping_range = rand() % 5;
	if (rng_ping_range == 0)
		self->bot.bot_baseline_ping = (int)(3 + (random() * 33)); // Low ping bastard
	else if (rng_ping_range <= 3)
		self->bot.bot_baseline_ping = (int)(3 + (random() * 66)); // Average pinger
	else
		self->bot.bot_baseline_ping = (int)(7 + (random() * 227)); // High ping bastard

	// Enemies
	self->bot.old_enemy = NULL;
	self->bot.enemy_in_xhair = false;
	self->bot.enemy_dist = 0;
	self->bot.enemy_height_diff = 0;
	self->bot.enemy_seen_time = 0;
	self->bot.enemy_chase_time = 0;

	//memset(&self->bot.noises, 0, sizeof(int) * MAX_CLIENTS);
	//memset(&self->bot.noise_time, 0, sizeof(int) * MAX_CLIENTS);

	// Nav
	self->bot.state = BOT_MOVE_STATE_NONE; // Init bot navigation
	self->bot.prev_node = INVALID;
	self->bot.current_node = INVALID;
	self->bot.next_node = INVALID;
	self->bot.goal_node = INVALID;
	self->bot.stuck_node = INVALID;
	self->bot.walknode.touched_node = INVALID;
	self->bot.walknode.highlighted_node = INVALID;
	self->bot.walknode.prev_highlighted_node = INVALID;


	// Weapons
	self->bot.last_sniper_zoom_time = 0;
	self->bot.last_weapon_change_time = 0;

	if (teamplay->value) // Reset bot radio at the start of each round
	{
		// Radio
		BOTLIB_PrecacheRadioSounds();
		self->bot.radioTeamReportedIn = false;
		self->bot.radioLastSawAllyTime = 0;
		self->bot.radioTeamReportedInDelay = 75 + (rand() % 101); // Add a random delay between 75 and 175 ricks
		self->bot.radioReportIn = false;
		self->bot.radioReportInTime = 0;
		self->bot.radioReportInDelay = 30 + (rand() % 60); // Add a random delay between 30 and 90 ticks
		self->bot.radioBandaging = false;
		self->bot.radioLastHumanMsg[0] = '\0';
		self->bot.radioReportKills = (rand() % 2); // RNG if this bot will report its kills or not

		// Enemies
		if (ff_afterround->value)
			self->bot.ff_allies_after_rnd = ((rand() % 2) == 0); // 50% chance the bot will attack allies after a TP round ends

		// Update skin to match TP skins
		const char* s = Info_ValueForKey(self->client->pers.userinfo, "skin");
		AssignSkin(self, s, false /* nickChanged */);
	}

	// Bot personalities
	if (bot_personality->value) {
		BOTLIB_LoadBotPersonality(self);

		// Bot skill adjustment based on map and other prefs
		// Simple adjustments for now
		int skillPlus = 0;
		skillPlus += self->bot.personality.map_prefs;
		self->bot.skill.overall += skillPlus; // This will decrease skill if map_prefs is negative, max -1
		if (self->bot.skill.overall > MAX_BOTSKILL)
			self->bot.skill.overall = MAX_BOTSKILL;
	}
}

//rekkie -- Quake3 -- s
//==============
// BOTLIB_BotInputToUserCommand
// Translates bot input to actual Q2 movement calls
//==============
void BOTLIB_BotInputToUserCommand(edict_t* ent, bot_input_t* bi, usercmd_t* ucmd, vec3_t delta_angles, int time)
{
	vec3_t angles, forward, right;
	float f, r, u, m;

	//clear the whole structure
	//memset(ucmd, 0, sizeof(usercmd_t));

	//the duration for the user command in milli seconds
	//ucmd->serverTime = time;
	//ucmd->weapon = bi->weapon;



	//set the buttons
	if (bi->actionflags & ACTION_RESPAWN) ucmd->buttons = BUTTON_ATTACK;
	if (bi->actionflags & ACTION_ATTACK) ucmd->buttons |= BUTTON_ATTACK;
	bi->actionflags &= ~ACTION_ATTACK;

	/*
	if (bi->actionflags & ACTION_TALK) ucmd->buttons |= BUTTON_TALK;
	if (bi->actionflags & ACTION_GESTURE) ucmd->buttons |= BUTTON_GESTURE;
	if (bi->actionflags & ACTION_USE) ucmd->buttons |= BUTTON_USE_HOLDABLE;
	if (bi->actionflags & ACTION_WALK) ucmd->buttons |= BUTTON_WALKING;
	if (bi->actionflags & ACTION_AFFIRMATIVE) ucmd->buttons |= BUTTON_AFFIRMATIVE;
	if (bi->actionflags & ACTION_NEGATIVE) ucmd->buttons |= BUTTON_NEGATIVE;
	if (bi->actionflags & ACTION_GETFLAG) ucmd->buttons |= BUTTON_GETFLAG;
	if (bi->actionflags & ACTION_GUARDBASE) ucmd->buttons |= BUTTON_GUARDBASE;
	if (bi->actionflags & ACTION_PATROL) ucmd->buttons |= BUTTON_PATROL;
	if (bi->actionflags & ACTION_FOLLOWME) ucmd->buttons |= BUTTON_FOLLOWME;
	*/



	// Radio wave animations
	// Enemy interaction
	if (bi->radioflags & RADIO_ENEMY_DOWN) BOTLIB_Wave(ent, WAVE_POINT);
	else if (bi->radioflags & RADIO_ENEMY_SIGHTED) BOTLIB_Wave(ent, WAVE_FLIPOFF);
	// Assistance required
	else if (bi->radioflags & RADIO_TAKING_FIRE) BOTLIB_Wave(ent, WAVE_FLIPOFF);
	else if (bi->radioflags & RADIO_IM_HIT) BOTLIB_Wave(ent, WAVE_TAUNT);
	else if (bi->radioflags & RADIO_COVER) BOTLIB_Wave(ent, WAVE_TAUNT);
	// Team reporting
	else if (bi->radioflags & RADIO_TEAM_REPORT_IN) BOTLIB_Wave(ent, WAVE_WAVE);
	else if (bi->radioflags & RADIO_REPORTIN) BOTLIB_Wave(ent, WAVE_SALUTE);
	else if (bi->radioflags & RADIO_TEAMMATE_DOWN) BOTLIB_Wave(ent, WAVE_SALUTE);
	// Directions
	else if (bi->radioflags & RADIO_BACK) BOTLIB_Wave(ent, WAVE_POINT);
	else if (bi->radioflags & RADIO_FORWARD) BOTLIB_Wave(ent, WAVE_POINT);
	else if (bi->radioflags & RADIO_UP) BOTLIB_Wave(ent, WAVE_POINT);
	else if (bi->radioflags & RADIO_DOWN) BOTLIB_Wave(ent, WAVE_POINT);
	else if (bi->radioflags & RADIO_LEFT) BOTLIB_Wave(ent, WAVE_POINT);
	else if (bi->radioflags & RADIO_RIGHT) BOTLIB_Wave(ent, WAVE_POINT);







	ent->bot.bi.radioflags = 0; // Clear radio flags


	// Adjust throwing pitch if using throwing knives
	//if (ent->client->weapon == FindItemByNum(KNIFE_NUM) && ent->client->pers.knife_mode == 1) // Throwing knives
	//	bi->viewangles[PITCH] = BOTLIB_ThrowingKnifePitch(ent);


	//set the view angles
	//NOTE: the ucmd->angles are the angles WITHOUT the delta angles
	ucmd->angles[PITCH] = ANGLE2SHORT(bi->viewangles[PITCH]);
	ucmd->angles[YAW] = ANGLE2SHORT(bi->viewangles[YAW]);
	ucmd->angles[ROLL] = ANGLE2SHORT(bi->viewangles[ROLL]);
	/*
	//subtract the delta angles
	for (j = 0; j < 3; j++) {
		temp = ucmd->angles[j] - delta_angles[j];
		ucmd->angles[j] = temp;
	}
	*/


	if (bi->dir[2])
		angles[PITCH] = bi->viewangles[PITCH];
	else
		angles[PITCH] = 0;
	angles[YAW] = bi->viewangles[YAW];
	angles[ROLL] = 0;

	// No movement
	//if (VectorEmpty(bi->dir)) return;
	if (bi->actionflags & ACTION_HOLDPOS)
	{
		// Kill velocity
		bi->actionflags = 0;
		bi->speed = 0;
		ent->velocity[0] *= 0.9;
		ent->velocity[1] *= 0.9;
		ent->velocity[2] *= 0.9;

		return;
	}
	else
	{
		//NOTE: movement is relative to the REAL view angles
		//get the horizontal forward and right vector
		AngleVectors(angles, forward, right, NULL);
		//bot input speed is in the range [0, 400] - SPEED_RUN
		//bi->speed = bi->speed * 127 / SPEED_RUN;

		// Cap max speed
		if (bi->speed > SPEED_RUN) bi->speed = SPEED_RUN;
		else if (bi->speed < -SPEED_RUN) bi->speed = -SPEED_RUN;

		//set the view independent movement
		f = DotProduct(forward, bi->dir);
		r = DotProduct(right, bi->dir);
		u = fabs(forward[2]) * bi->dir[2];
		m = fabs(f);

		if (fabs(r) > m) {
			m = fabs(r);
		}

		if (fabs(u) > m) {
			m = fabs(u);
		}

		if (m > 0) {
			f *= bi->speed / m;
			r *= bi->speed / m;
			u *= bi->speed / m;
		}

		if (ent->client->leg_damage == 0) // No jumping if legs are broken
			u = 0;

		ucmd->forwardmove = f;
		ucmd->sidemove = r;
		ucmd->upmove = u;
	}

	if (bi->actionflags & ACTION_MOVEFORWARD) ucmd->forwardmove = SPEED_RUN; // 127
	if (bi->actionflags & ACTION_MOVEBACK) ucmd->forwardmove = -SPEED_RUN; // -127
	if (bi->actionflags & ACTION_MOVELEFT) ucmd->sidemove = -SPEED_RUN; // -127
	if (bi->actionflags & ACTION_MOVERIGHT) ucmd->sidemove = SPEED_RUN; // 127


	//jump/moveup
//if (bi->actionflags & ACTION_JUMP && ent->client->leg_damage == 0) ucmd->upmove = SPEED_RUN; // 127

	if ((bi->actionflags & ACTION_HOLDJUMP) && (ent->groundentity)) // Stop holding jump if on ground
	{
		bi->actionflags &= ~ACTION_HOLDJUMP;
	}
	if ((bi->actionflags & ACTION_HOLDJUMP)) // Hold jump if in air
	{
		if (ent->groundentity == NULL)
			ucmd->upmove = SPEED_RUN;
		else
			bi->actionflags &= ~ACTION_HOLDJUMP;
	}
	if (bi->actionflags & ACTION_JUMP && ent->client->leg_damage == 0) // Initiate jump if no leg damage
	{
		// Slow down
		//ucmd->forwardmove = 0;
		//ucmd->sidemove = 0;
		//ucmd->upmove = 0;

		//bi->actionflags &= ~ACTION_JUMP;
		//bi->actionflags |= ACTION_HOLDJUMP;
		ucmd->upmove = SPEED_RUN;
		if (ent->groundentity) // Give a slight boost if on ground
			ent->velocity[2] += 50;

		ucmd->forwardmove = SPEED_RUN;
	}
	if (bi->actionflags & ACTION_BOXJUMP && ent->client->leg_damage == 0) // Initiate jump if no leg damage
	{
		// Slow down
		//ucmd->forwardmove = 0;
		//ucmd->sidemove = 0;
		//ucmd->upmove = 0;

		//bi->actionflags &= ~ACTION_JUMP;
		//bi->actionflags |= ACTION_HOLDJUMP;
		ucmd->upmove = SPEED_RUN;
		if (ent->groundentity) // Give a slight boost if on ground
			ent->velocity[2] += 325; // 400

		ucmd->forwardmove = SPEED_RUN;
	}
	if (bi->actionflags & ACTION_JUMPPAD && ent->client->leg_damage == 0) // Initiate jump if no leg damage
	{
		// Check the bot is touching a jump node before attempting to jump
		int nodelist[MAX_NODELIST];
		int nodes_touched = BOTLIB_NodeTouchNodes(ent->s.origin, vec3_origin, 8, ent->mins, ent->maxs, nodelist, MAX_NODELIST, INVALID);
		if (nodes_touched && ent->bot.current_node != INVALID && ent->bot.next_node != INVALID)
		{
			for (int i = 0; i < nodes_touched; i++)
			{
				//Com_Printf("%s %s touched node %d\n", __func__, ent->client->pers.netname, nodelist[i]);
				//if (nodes[nodelist[i]].type & NODE_JUMPPAD)
				if (nodes[nodelist[i]].nodenum == ent->bot.current_node)
				{
					//Com_Printf("%s %s touched jump node %d\n", __func__, ent->client->pers.netname, nodelist[i]);
					// Attempt jump
					BOTLIB_DoParabolaJump(ent, nodes[ent->bot.next_node].origin);
					bi->actionflags &= ~ACTION_JUMPPAD;
					break;
				}
			}
		}
	}
	//crouch/movedown
	if (bi->actionflags & ACTION_CROUCH) ucmd->upmove = -SPEED_RUN; // -127

	//ladder - movedown, moveup
	if (bi->actionflags & ACTION_MOVEDOWN)
	{
		ucmd->forwardmove = 50; // jump/move up
		ucmd->upmove = -SPEED_WALK; // crouch

		// Slow turning speed
		ent->velocity[0] *= 0.25;
		ent->velocity[1] *= 0.25;

		ent->velocity[2] = -100; // Slow decent

		//Com_Printf("%s %s ladder down\n", __func__, ent->client->pers.netname);

		//vec3_t dist;
		//VectorNormalize(dist);
		//VectorScale(dist, 20, ent->velocity); // Apply it
	}
	if (bi->actionflags & ACTION_MOVEUP)
	{
		/*
		// Check we're looking in the direction of a ladder
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

		if (on_ladder)
		{
			ent->velocity[2] += 10;
			ucmd->upmove = SPEED_WALK;
			Com_Printf("%s %s ladder up\n", __func__, ent->client->pers.netname);
		}
		*/

		//Com_Printf("%s %s ladder up\n", __func__, ent->client->pers.netname);

		ucmd->forwardmove = SPEED_RUN; // jump/move up
		ucmd->upmove = SPEED_RUN; // jump/move up

		// Slow turning speed
		ent->velocity[0] *= 0.25;
		ent->velocity[1] *= 0.25;

		// Go up
		ent->velocity[2] = 200; // Slow upward movement

		/*
		// Check if touching ladder with an overly large hitbox
		if (0)
		{
			vec3_t min = { -32, -32, -60 };
			vec3_t max = { 32, 32, 60 };
			trace_t tr = gi.trace(ent->s.origin, min, max, ent->s.origin, ent, MASK_PLAYERSOLID);
			if ((tr.contents & CONTENTS_LADDER) || tr.contents & CONTENTS_SOLID)
			{
				ent->velocity[2] = 200; // Slow upward movement

				//Com_Printf("%s %s ladder up\n", __func__, ent->client->pers.netname);
				return;
			}
		}
		*/

		//ent->movetype == MOVETYPE_FLY;

		/*
		vec3_t dist;
		VectorNormalize(dist);
		VectorScale(dist, 20, ent->velocity); // Apply it

		for (int i = 0; i < 3; i++)
			ent->velocity[i] = 200 * bi->dir[i];
		*/
	}


	// Air control
	//if ((bi->actionflags & ACTION_JUMP) || (bi->actionflags & ACTION_HOLDJUMP))
	//if (ent->groundentity == NULL)
	if (bi->actionflags & ACTION_JUMP)
		//if (0)
	{
		float wishspeed, addspeed, accelspeed, currentspeed;

		wishspeed = VectorNormalize(bi->dir);
		if (wishspeed > SPEED_RUN) wishspeed = SPEED_RUN;

		currentspeed = DotProduct(ent->velocity, bi->dir);

		addspeed = wishspeed - currentspeed;
		addspeed *= 10.1;
		if (addspeed <= 0) {
			return;
		}

		//accelspeed = ent->accel * FRAMETIME * wishspeed;
		accelspeed = SPEED_RUN * FRAMETIME * wishspeed;
		if (accelspeed > addspeed) {
			accelspeed = addspeed;
		}


		/*
		float z = accelspeed * bi->dir[2];
		if (z < 0)
		{
			// Slow down velocity
			//Com_Printf("%s %s velocity %f\n", __func__, ent->client->pers.netname, z);
			ent->velocity[2] += z;
		}
		*/

		ent->velocity[0] += accelspeed * bi->dir[0];
		ent->velocity[1] += accelspeed * bi->dir[1];
		ent->velocity[2] += accelspeed * bi->dir[2];

		//for (i = 0; i < 3; i++) {
		//	ent->velocity[i] += accelspeed * bi->dir[i];
		//	//ent->velocity[i] = accelspeed * bi->dir[i];
		//}
		//Com_Printf("%s %s velocity[%f %f %f]\n", __func__, ent->client->pers.netname, ent->velocity[0], ent->velocity[1], ent->velocity[2]);
	}

	//bi->actionflags = 0; // Clear action flags
	ent->bot.bi.actionflags = 0;
}

int BOTLIB_AutoAdjustSkill(edict_t * self)
{
	int highest_score = 0; // Highest player score
	int score_diff = 0; // Difference between player and best player's score
	float score_percent_diff = 1.0; // Difference in percent between player and best player's score 

	//Init the variable bot skill
	if (self->bot.skill.overall < 1)
		self->bot.skill.overall = bot_skill->value;

	if (self->client == NULL)
		return 0;

	// Get highest score
	for (int i = 0; i <= num_players; i++)
	{
		if (players[i] == NULL || players[i]->client == NULL) continue;

		if (players[i]->client->resp.score > highest_score)
			highest_score = players[i]->client->resp.score;
	}

	// Find the difference between highest score and our score (the gap between two scores)
	if (self->client->resp.score < highest_score)
	{
		score_diff = highest_score - self->client->resp.score; // Get score gap

		if (score_diff > (int)bot_skill_threshold->value) // If the score gap is greater than threshold
		{
			// Get the diff in percent: 1.0 is equal, 0.5 is half the score of the best player. Higher is better.
			// > 0.75 is normal
			// > 0.5 is okay
			// < 0.5 is poor
			score_percent_diff = (float)self->client->resp.score / (float)highest_score;

			if (score_percent_diff >= 0.75)
			{
				self->bot.skill.overall = bot_skill->value;
				//Com_Printf("%s %s adjusting bot skill [-] [%f]\n", __func__, self->client->pers.netname, bot_skill->value);
				return 0; // Normal
			}
			else if (score_percent_diff >= 0.5)
			{
				self->bot.skill.overall = self->bot.skill.overall + 0.25; // Increase bot skill slower

				if (self->bot.skill.overall > (MAX_BOTSKILL - 1)) // Max skill
					self->bot.skill.overall = bot_skill->value; // Scale skill back down to base level

				//Com_Printf("%s %s adjusting bot skill [+] [%f]\n", __func__, self->client->pers.netname, self->bot.skill.overall);

				return 1; // Okay
			}
			else if (score_percent_diff < 0.5)
			{
				self->bot.skill.overall = self->bot.skill.overall + 0.5; // Increase bot skill faster

				if (self->bot.skill.overall > (MAX_BOTSKILL - 1)) // Max skill
					self->bot.skill.overall = bot_skill->value; // Scale skill back down to base level

				//Com_Printf("%s %s adjusting bot skill [+] [%f]\n", __func__, self->client->pers.netname, self->bot.skill.overall);

				return 2; // Poor
			}
		}
	}

	self->bot.skill.overall = bot_skill->value;
	//Com_Printf("%s %s adjusting bot skill [-] [%f]\n", __func__, self->client->pers.netname, bot_skill->value);
	return 0; // Normal

	/*
	PlayerScores player_scores[MAX_CLIENTS]; // All player scores
	int player_id = 0; // Our player id

	// Check if players have completed at least the first round. We only want to adjust skill after players have an established score.
	qboolean completed_a_round = false;
	for (int i = TEAM1; i <= teamCount; i++)
	{
		if (teams[i].score > 0)
		{
			completed_a_round = true;
			break;
		}
	}
	if (completed_a_round == false) // Ignore the first round
		return 0;

	// Init player scores
	for (int i = 0; i <= num_players; i++)
	{
		if (players[i] == self) // If this is us
			player_id = i; // Store our own id

		player_scores[i].playernum = i; // Store the player id

		// Store the score
		if (players[i] == NULL || players[i]->client == NULL)
			player_scores[i].score = 0; // Zero invalid clients
		else
			player_scores[i].score = players[i]->client->resp.score; // Valid client score

		if (player_scores[i].score > highest_score)
			highest_score = player_scores[i].score;
	}

	// Sort scores from highest to lowest
	qsort(player_scores, num_players, sizeof(PlayerScores), BOTLIB_ScoreCompare);

	// Find where we rank
	int rank = 0;
	for (int i = 0; i <= num_players; i++)
	{
		// Only rank against player's with a score
		//if (player_scores[i].score > 0)
			rank++;

		// Break when we find ourself
		if (player_id == player_scores[i].playernum)
			break;
	}

	float percent = ((float)rank / (float)num_players);

	//Com_Printf("%s %s rank[%d of %d] = %f score[%d]\n", __func__, self->client->pers.netname, rank, num_players, percent, self->client->resp.score);

	// If bot score is in the bottom half (50% or greater) adjust its skill so it can aim to be in the top 10%
	if (percent >= 0.75) // Higher is worse
	{
		self->bot.skill.overall = self->bot.skill.overall + 0.5; // Increase bot skill faster

		if (self->bot.skill.overall > MAX_BOTSKILL) // Max skill
			self->bot.skill.overall = MAX_BOTSKILL;

		//Com_Printf("%s %s adjusting bot skill [+] [%d]\n", __func__, self->client->pers.netname, (int)self->bot.skill.overall);
	}
	else if (percent >= 0.5) // Higher is worse
	{
		self->bot.skill.overall = self->bot.skill.overall + 0.25; // Increase bot skill slower

		if (self->bot.skill.overall > MAX_BOTSKILL) // Max skill
			self->bot.skill.overall = MAX_BOTSKILL;

		//Com_Printf("%s %s adjusting bot skill [+] [%d]\n", __func__, self->client->pers.netname, (int)self->bot.skill.overall);
	}
	else // Return skill back to base level
	{
		self->bot.skill.overall = bot_skill->value;
		//Com_Printf("%s %s adjusting bot skill [-] [%d]\n", __func__, self->client->pers.netname, (int)bot_skill->value);
	}

	// Return skill bracket: low, med, high. Used to recommend a weapon and item
	if (percent < 0.25)
		return 0; // Low
	else if (percent < 0.5)
		return 1; // Med

	return 2; // High
	*/
}

///////////////////////////////////////////////////////////////////////
// Main Think function for bot
///////////////////////////////////////////////////////////////////////
void BOTLIB_Think(edict_t* self)
{
	usercmd_t ucmd;

	//rekkie -- Fake Bot Client -- s
	if (level.framenum % 10 == 0) // Update bot info every 10th frame
	{
		// Set self->client->ping to random bot->bot_baseline_ping, then vary by +-3 (ping jitter)
		int ping_jitter = (rand() % 7); // Positive jitter
		if (ping_jitter && (rand() % 1) == 0)
			ping_jitter -= (ping_jitter * 2); // Negative jitter

		//Com_Printf("%s %s [%d + %d]\n", __func__, self->client->pers.netname, self->bot.bot_baseline_ping, ping_jitter);

		self->bot.bot_ping = self->bot.bot_baseline_ping + ping_jitter;
		if (self->bot.bot_ping < 5) self->bot.bot_ping = 1; // Min ping
		self->client->ping = self->bot.bot_ping;
		
		
		SV_BotUpdateInfo(self->client->pers.netname, self->bot.bot_ping, self->client->resp.score); // So the server can fake the bot as a 'client'
		//gi.SV_BotUpdateInfo(self->client->pers.netname, self->bot.bot_ping, self->client->resp.score);
	}
	//rekkie -- Fake Bot Client -- e

	// Set up client movement
	VectorCopy(self->client->ps.viewangles, self->s.angles);
	VectorSet(self->client->ps.pmove.delta_angles, 0, 0, 0);
	memset(&ucmd, 0, sizeof(ucmd));

	// Make bots end intermission and enter the next map (especially useful to change maps when no 'human' players are around)
	if (level.intermission_framenum)
	{
		ucmd.buttons = BUTTON_ANY; // This button is pressed to end the intermission
		goto end_think;
	}

	// Stop trying to think if the bot can't respawn.
	if (!IS_ALIVE(self) && ((gameSettings & GS_ROUNDBASED) || (self->client->respawn_framenum > level.framenum)))
	{
		goto end_think;
	}

	// Force respawn 
	if (self->deadflag == DEAD_DEAD)
	{
		self->client->buttons = 0;
		ucmd.buttons = BUTTON_ATTACK;
	}

	// Don't execute thinking code if not alive
	if (self->deadflag != DEAD_NO || self->health <= 0)
	{
		goto end_think;
	}

	// Make the bot brain dead for a period of time
	if (self->bot.pause_time)
	{
		self->bot.pause_time--; // Reduce timer to zero
		self->bot.bi.speed = 0;
		goto end_think; // Skip bot logic
	}

	// darksaint: reduced this to 15 because bots aren't moving quickly enough on spawn and telefrags are happening
	// Original value was 75
	if (level.framenum < 15) // Wait for a little before processing AI on a new map
		goto end_think; // Skip bot logic

	//gi.dprintf("%s: My bot state is %d\n", __func__, self->bot.state);
	if (ctf->value) // CTF Goals
	{
		BOTLIB_CTF_Goals(self);
	}
	else if (esp->value) // ESP Goals
	{
		// Do nothing until this is fixed
		BOTLIB_ESP_Goals(self);
	}

	// Check if the bot is in a NAV state (I need a nav) or if NONE
	else if (self->bot.state == BOT_MOVE_STATE_NAV || self->bot.state == BOT_MOVE_STATE_NONE)
	{
		/*
		if (rand() % 2) // Area 1
		{
			BOTLIB_CanGotoNode(self, nodes[64].nodenum, 0);
		}
		else // Area 9
		{
			BOTLIB_CanGotoNode(self, nodes[3344].nodenum, 0);
		}
		*/

		/*
		//int curr_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
		//if (curr_node != 2505)
		if (1 && rand() % 2) // Area 0
		{
			if (rand() % 2)
				BOTLIB_CanGotoNode(self, nodes[2505].nodenum, 0);
			else
				BOTLIB_CanGotoNode(self, nodes[3346].nodenum, 0);
		}
		else // Area 1
		{
			if (rand() % 2)
				BOTLIB_CanGotoNode(self, nodes[999].nodenum, 0);
			else
				BOTLIB_CanGotoNode(self, nodes[38].nodenum, 0);
		}
		*/

		//else // DM and TP goals
		{
			//self->bot.pause_time = 100;
			//Com_Printf("\n%s %s [%d] BOT_MOVE_STATE_NAV BOTLIB_PickLongRangeGoal() -------------------- \n", __func__, self->client->pers.netname, level.framenum);

			//Com_Printf("%s %s [%d] BOT_MOVE_STATE_NAV BOTLIB_PickLongRangeGoal()\n", __func__, self->client->pers.netname, level.framenum);
			
			//nav_area.total_areas = 0; // Turn off area based nav

			// THIS IS WHERE THE bot.current_node is set //
			BOTLIB_PickLongRangeGoal(self);

			//Com_Printf("%s %s [%d] BOT_MOVE_STATE_NAV BOTLIB_PickLongRangeGoal() curr[%d] goal[%d] -------------------- \n", __func__, self->client->pers.netname, level.framenum, self->bot.current_node, self->bot.goal_node);
		}
	}
	/*
	if (self->bot.state == BOT_MOVE_STATE_NAV_NEXT) // If map has nodes grouped into areas
	{
		//self->bot.pause_time = 100;
		//Com_Printf("%s %s [%d] BOT_MOVE_STATE_NAV_NEXT BOTLIB_GetNextAreaNode() -------------------- \n", __func__, self->client->pers.netname, level.framenum);
		BOTLIB_GetNextAreaNode(self); // Get next area
	}
	*/



	// Kill the bot if completely stuck somewhere
	//if(VectorLength(self->velocity) > 37) //
	//	self->suicide_timeout = level.framenum + 10.0 * HZ;
	//if( self->suicide_timeout < level.framenum && !teamplay->value )
	//	killPlayer( self, true );

	// Kill the bot if they've not moved between nodes in a timely manner, stuck!
	//gi.dprintf("%s is currently at node %i\n", self->client->pers.netname, self->bot.current_node);

	// Non-teamplay stuck suicide
	if (!teamplay->value) {
		if (self->bot.node_travel_time > 120) {
			killPlayer(self, true);
		}
		// Too often teamplay bots will suicide because there's a bit of waiting around
	} else if (self->bot.node_travel_time > 160 && 
		current_round_length > 60 && 
		!lights_camera_action &&
		!holding_on_tie_check) {
			BOTLIB_PickLongRangeGoal(self);
			//killPlayer(self, true);
	}

	// Find any short range goal
	//ACEAI_PickShortRangeGoal(self);

	//BOTLIB_GetWeaponsAndAmmo(self); //rekkie -- Locate and pickup a primary weapon if we need one


	if (0) // Always follow player -- && (level.framenum % HZ == 0))
	{
		for (int p = 0; p < num_players; p++) // Cycle based on how many players we find
		{
			if (players[p]->is_bot == false && players[p]->bot.current_node != 0 && players[p]->solid == SOLID_BBOX && players[p]->deadflag == DEAD_NO)
			{
				// Get friendly node
				int n = players[p]->bot.current_node;	// This is for bots and humans. For humans their current node is now updated in p_client.c ClientThink()
				if (n == INVALID)
					continue;

				if (self->bot.goal_node != n)
				{
					Com_Printf("%s %s following %s node %i at %f %f %f\n", __func__, self->client->pers.netname, players[p]->client->pers.netname, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);
					//BOTLIB_SetGoal(self, n);
				}
			}
		}
	}




	//if (1)
	{
		BOTLIB_FindVisibleAllies(self); // Find visible allies
		BOTLIB_Radio(self, &ucmd);
		self->bot.see_enemies = BOTLIB_FindEnemy(self); // Find visible enemies
		BOTLIB_Reload(self); // Reload the weapon if needed

		// This doesn't mean that the bot sees itself as an enemy
		// self->enemy is which bot the current self->bot is targeting
		if (self->enemy)
		{
			// Chase after the new enemy
			if (self->bot.enemy_chase_time < level.framenum && self->enemy->bot.current_node != self->bot.goal_node)
			{
				qboolean chase_enemy = false;
				if ((FindItem(HC_NAME) == self->client->weapon ||
					FindItem(M3_NAME) == self->client->weapon ||
					FindItem(DUAL_NAME) == self->client->weapon ||
					FindItem(KNIFE_NAME) == self->client->weapon)
					&& self->bot.enemy_dist > 200)
				{
					chase_enemy = true;
					self->bot.enemy_chase_time = level.framenum + 1 * HZ; // Delay next call
				}
				else if (FindItem(SNIPER_NAME) == self->client->weapon)// && self->bot.enemy_dist > 1500)
				{
					chase_enemy = false;
					self->bot.enemy_chase_time = level.framenum + (((rand() % 20) + 10) * HZ); // Delay next call
				}
				else if ((FindItem(M4_NAME) == self->client->weapon || FindItem(MP5_NAME) == self->client->weapon) && self->bot.enemy_dist > 1024)
				{
					chase_enemy = true;
					self->bot.enemy_chase_time = level.framenum + (((rand() % 10) + 10) * HZ); // Delay next call
				}

				if (chase_enemy)
				{
					// Get enemy node
					if (BOTLIB_CanGotoNode(self, self->enemy->bot.current_node, false)) // Make sure we can visit the node they're at
					{
						//self->bot.state = BOT_MOVE_STATE_MOVE;
						//BOTLIB_SetGoal(self, self->enemy->bot.current_node);
						//Com_Printf("%s %s visiting enemy %s node %i [delay: %i vs %i] [wep: %s]\n", __func__, self->client->pers.netname, self->enemy->client->pers.netname, self->enemy->bot.current_node, self->bot.enemy_chase_time, level.framenum, self->client->weapon->pickup_name);
					}
				}
			}
		}

		// If the bot is on a slope, raise it up depending on the slope normal and the bot mins/maxs hit box
		{
			self->bot.touch_ground = gi.trace(self->s.origin, self->mins, self->maxs, tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 48), self, (MASK_PLAYERSOLID | MASK_OPAQUE));

			/*
			self->bot.touch_ground = gi.trace(self->s.origin, self->mins, self->maxs, tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 128), self, (MASK_PLAYERSOLID | MASK_OPAQUE));
			vec3_t exp_up;
			VectorCopy(self->bot.touch_ground.plane.normal, exp_up);
			exp_up[2] = 0;
			VectorNormalize(exp_up);
			exp_up[2] = 1;
			VectorScale(exp_up, 24, exp_up);
			// Feed the raised up position back into the trace
			VectorAdd(self->bot.touch_ground.endpos, exp_up, self->bot.touch_ground.endpos);
			self->bot.touch_ground = gi.trace(self->bot.touch_ground.endpos, NULL, NULL, self->bot.touch_ground.endpos, self, (MASK_PLAYERSOLID | MASK_OPAQUE));
			*/

		}

		if (self->bot.state == BOT_MOVE_STATE_MOVE || self->bot.state == BOT_MOVE_STATE_WANDER || self->bot.state == BOT_MOVE_STATE_STAND)
		{
			BOTLIB_FollowPath(self); // Get current and next node back from nav code.
			BOTLIB_Wander(self, &ucmd);
		}

		BOTLIB_TouchingLadder(self);
		BOTLIB_Look(self, &ucmd);

		BOTLIB_ChooseWeapon(self);

		// When out of sight of enemies
		if (self->bot.see_enemies == false)
		{
			BOTLIB_Healing(self, &ucmd); // Check if bot needs to heal
			BOTLIB_ReadyWeapon(self); // Change to a better weapon

			// Sniper bots should zoom in before an encounter
			if ((rand() % 10) == 0)
				BOTLIB_SniperZoom(self);

		}

		//if (self->bot.see_enemies == true && self->bot.enemy_in_xhair && //(self->enemy && self->enemy->deadflag == DEAD_NO) &&
		//	(self->client->weaponstate != WEAPON_RELOADING) && (self->client->bandaging == 0) &&
		//	(teamplay->value && lights_camera_action <= 1) || teamplay->value == 0)
		if (self->bot.see_enemies)
		{
			if (self->bot.enemy_in_xhair)
				BOTLIB_Attack(self, &ucmd);

			else if (self->client->weapon == FindItemByNum(HC_NUM) && BOTLIB_Infront(self, self->enemy, 0.3))
				BOTLIB_Attack(self, &ucmd);

			else if (self->client->weapon == FindItemByNum(GRENADE_NUM) && BOTLIB_Infront(self, self->enemy, 0.3))
				BOTLIB_Attack(self, &ucmd);
		}
	}


	// Remember where we were, to check if we got stuck.
	//VectorCopy( self->s.origin, self->lastPosition );

	// set bot's view angle
	ucmd.angles[PITCH] = ANGLE2SHORT(self->s.angles[PITCH]);
	ucmd.angles[YAW] = ANGLE2SHORT(self->s.angles[YAW]);
	ucmd.angles[ROLL] = ANGLE2SHORT(self->s.angles[ROLL]);

end_think:

	ucmd.msec = 1000 / BOT_FPS;
	self->client->ps.pmove.pm_time = ucmd.msec / 8;

	BOTLIB_BotInputToUserCommand(self, &self->bot.bi, &ucmd, self->bot.bi.viewangles, self->client->ps.pmove.pm_time);

	ClientThink(self, &ucmd);

	self->nextthink = level.framenum + 1; //(game.framerate / BOT_FPS);
}



// Return's the optimal pitch for throwing a knife at an enemy
float BOTLIB_ThrowingKnifePitch(edict_t* self)
{
	if (self->enemy == NULL)
		return 0;

	float gravity = sv_gravity->value; // Gravity
	float distanceXYZ = VectorDistance(self->s.origin, self->enemy->s.origin); // Distance XY
	float distanceXY = VectorDistance(self->s.origin, self->enemy->s.origin); // Distance XYZ
	float height_diff = self->enemy->s.origin[2] - self->s.origin[2]; // Height difference
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
	if (launchAngleDegrees > 89 || launchAngleDegrees < -89)
		launchAngleDegrees = 0;

	Com_Printf("%s %s pitch[%f]\n", __func__, self->enemy->client->pers.netname, launchAngleDegrees);

	return launchAngleDegrees;

	//ucmd->angles[PITCH] = launchAngleDegrees;
}

void BOTLIB_TouchingLadder(edict_t* self)
{
	//trace_t tr;

	{
		// Check if touching ladder
		{
			float yaw_rad = 0;
			vec3_t fwd = { 0 }, end = { 0 };
			trace_t tr;

			yaw_rad = DEG2RAD(self->s.angles[YAW]);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);

			//VectorMA(self->s.origin, 1, fwd, end);
			//tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_PLAYERSOLID);

			VectorMA(self->s.origin, 1, fwd, end);
			vec3_t lmins = { -16, -16, -96 };
			vec3_t lmaxs = { 16, 16, 96 };
			tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);

			self->bot.touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));

			if (self->bot.touching_ladder == false)
			{
				VectorMA(self->s.origin, 8, fwd, end);
				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);
				self->bot.touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			}
			if (self->bot.touching_ladder == false)
			{
				VectorMA(self->s.origin, 16, fwd, end);
				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);
				self->bot.touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			}
			if (self->bot.touching_ladder == false)
			{
				VectorMA(self->s.origin, 32, fwd, end);
				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);
				self->bot.touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			}
		}
	}
}

// Record footsteps
void BOTLIB_Footsteps(edict_t *self)
{
	if (self->s.event == EV_FOOTSTEP)
		PlayerNoise(self, self->s.origin, PNOISE_SELF);
}

// Keep a record of player noises and their locations
void BOTLIB_PlayerNoise(edict_t* who, vec3_t where, int type)
{
	// Each player
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (players[i] == who) // Find the right player
		{
			// Assign based on type
			if (type == PNOISE_SELF) // Getting in and out of water, drowning, walking
			{
				botlib_noises.self_time[i] = 120;
				botlib_noises.owner[i] = who;
				VectorCopy(where, botlib_noises.self_origin[i]);

				// Randomize the noise origin a bit
				if (rand() % 2 == 0)
					botlib_noises.self_origin[i][0] += 512;
				else
					botlib_noises.self_origin[i][0] -= 512;

				if (rand() % 2 == 0)
					botlib_noises.self_origin[i][1] += 512;
				else
					botlib_noises.self_origin[i][1] -= 512;

				if (rand() % 2 == 0)
					botlib_noises.self_origin[i][2] += 64;
				else
					botlib_noises.self_origin[i][2] -= 32;
			}
			else if (type == PNOISE_WEAPON) // Weapons with muzzle flash
			{
				botlib_noises.weapon_time[i] = 120;
				botlib_noises.owner[i] = who;
				VectorCopy(where, botlib_noises.weapon_origin[i]);

				// Randomize the noise origin a bit
				if (rand() % 2 == 0)
					botlib_noises.weapon_origin[i][0] += 512;
				else
					botlib_noises.weapon_origin[i][0] -= 512;

				if (rand() % 2 == 0)
					botlib_noises.weapon_origin[i][1] += 512;
				else
					botlib_noises.weapon_origin[i][1] -= 512;

				if (rand() % 2 == 0)
					botlib_noises.self_origin[i][2] += 64;
				else
					botlib_noises.self_origin[i][2] -= 32;
			}
			else if (type == PNOISE_IMPACT) // Knife impact, or grenade explosion
			{
				botlib_noises.impact_time[i] = 120;
				botlib_noises.owner[i] = who;
				VectorCopy(where, botlib_noises.impact_origin[i]);

				// Randomize the noise origin a bit
				if (rand() % 2 == 0)
					botlib_noises.impact_origin[i][0] += 512;
				else
					botlib_noises.impact_origin[i][0] -= 512;

				if (rand() % 2 == 0)
					botlib_noises.impact_origin[i][1] += 512;
				else
					botlib_noises.impact_origin[i][1] -= 512;

				if (rand() % 2 == 0)
					botlib_noises.self_origin[i][2] += 64;
				else
					botlib_noises.self_origin[i][2] -= 32;
			}
		}
	}

	// Code from BASEQ2 -- s
	/*
	edict_t     *noise;

	if (type == PNOISE_WEAPON) {
		if (who->client->silencer_shots) {
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise) {
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	} else { // type == PNOISE_IMPACT
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy(where, noise->s.origin);
	VectorSubtract(where, noise->maxs, noise->absmin);
	VectorAdd(where, noise->maxs, noise->absmax);
	noise->last_sound_framenum = level.framenum;
	gi.linkentity(noise);
	*/
	// Code from BASEQ2 -- e
}

///////////////////////////////////////////////////////////////////////
// Checks if bot can move (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime. 
///////////////////////////////////////////////////////////////////////
qboolean BOTLIB_CanMove(edict_t* self, int direction)
{
	vec3_t start, end;
	vec3_t forward, right;
	vec3_t offset;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles, angles);

	if (direction == MOVE_LEFT)
		angles[1] += 90;
	else if (direction == MOVE_RIGHT)
		angles[1] -= 90;
	else if (direction == MOVE_BACK)
		angles[1] -= 180;

	// Trace ahead to see if safe or hitting wall
	{
		AngleVectors(angles, forward, right, NULL);
		VectorSet(offset, 0, 7, self->viewheight - 8);
		offset[1] = 0;
		G_ProjectSource(self->s.origin, offset, forward, right, start);
		VectorMA(start, 64, forward, end);
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID | MASK_OPAQUE);
		if (tr.fraction < 1.0 || tr.contents & MASK_DEADLY || (tr.ent && (tr.ent->touch == hurt_touch)))
			return false;
	}

	// Trace down to see if safe
	{
		AngleVectors(angles, forward, right, NULL); // Set up the vectors

		//VectorSet(offset, 36, 0, 24);
		//G_ProjectSource(self->s.origin, offset, forward, right, start);

		VectorSet(offset, 36, 0, -110); // RiEvEr reduced drop distance
		G_ProjectSource(self->s.origin, offset, forward, right, end);

		tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID | MASK_OPAQUE);
		if (((tr.fraction == 1.0) && !((lights_camera_action || self->client->uvTime) && BOTCOL_CanMoveSafely(self, angles))) // avoid falling after LCA
			|| (tr.contents & MASK_DEADLY)							  // avoid SLIME or LAVA
			|| (tr.ent && (tr.ent->touch == hurt_touch)))			  // avoid MOD_TRIGGER_HURT
		{
			return false;
		}
	}


	return true; // yup, can move
}