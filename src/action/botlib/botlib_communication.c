#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"
#include "../m_player.h" // For waving frame types, i.e: FRAME_flip01

// Bot wave gestures
void BOTLIB_Wave(edict_t* ent, int type)
{
	if (ent->groundentity == NULL) return; // Don't wave when not on ground
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) return; // Don't wave when ducked
	if (ent->client->anim_priority > ANIM_WAVE) return; // Don't wave when higher animation priority are active

	// Time limit between waves
	if (ent->client->resp.lastWave + (1 * HZ) > level.framenum)
		return;
	ent->client->resp.lastWave = level.framenum; // Update last wave


	switch (type)
	{
	case WAVE_FLIPOFF:
		SetAnimation(ent, FRAME_flip01 - 1, FRAME_flip12, ANIM_WAVE); //flipoff
		break;
	case WAVE_SALUTE:
		SetAnimation(ent, FRAME_salute01 - 1, FRAME_salute11, ANIM_WAVE); //salute
		break;
	case WAVE_TAUNT:
		SetAnimation(ent, FRAME_taunt01 - 1, FRAME_taunt17, ANIM_WAVE); //taunt
		break;
	case WAVE_WAVE:
		SetAnimation(ent, FRAME_wave01 - 1, FRAME_wave11, ANIM_WAVE); //wave
		break;
	case WAVE_POINT:
	default:
		SetAnimation(ent, FRAME_point01 - 1, FRAME_point12, ANIM_WAVE); //point
		break;
	}
}

void BOTLIB_PrecacheRadioSounds()
{
	int i;
	char path[MAX_QPATH];

	//male
	for (i = 0; i < bot_numMaleSnds; i++)
	{
		Com_sprintf(path, sizeof(path), "%s%s.wav", "radio/male/", bot_male_radio_msgs[i].msg);
		bot_male_radio_msgs[i].sndIndex = gi.soundindex(path);
	}

	//female
	for (i = 0; i < bot_numFemaleSnds; i++)
	{
		Com_sprintf(path, sizeof(path), "%s%s.wav", "radio/female/", bot_female_radio_msgs[i].msg);
		bot_female_radio_msgs[i].sndIndex = gi.soundindex(path);
	}
}
void BOTLIB_RadioBroadcast(edict_t* ent, edict_t* partner, char* msg)
{
	int j, i, msg_len, numSnds;
	edict_t* other;
	bot_radio_msg_t* radio_msgs;
	int  msg_soundIndex = 0;
	char msgname_num[8], filteredmsg[48];
	qboolean found = false;
	radio_t* radio;

	if (!IS_ALIVE(ent))
		return;

	if (!teamplay->value)
	{
		if (!DMFLAGS((DF_MODELTEAMS | DF_SKINTEAMS)))
			return;			// don't allow in a non-team setup...
	}

	/*
	//TempFile END
		//AQ2:TNG Slicer
	if (radio_repeat->value)
	{  //SLIC2 Optimization
		if (CheckForRepeat(ent, i) == false)
			return;
	}

	if (radio_max->value)
	{
		if (CheckForFlood(ent) == false)
			return;
	}
	*/

	radio = &ent->client->resp.radio;
	if (radio->gender)
	{
		radio_msgs = bot_female_radio_msgs;
		numSnds = bot_numFemaleSnds;
	}
	else
	{
		radio_msgs = bot_male_radio_msgs;
		numSnds = bot_numMaleSnds;
	}

	i = found = 0;
	msg_len = 0;

	Q_strncpyz(filteredmsg, msg, sizeof(filteredmsg));

	for (i = 0; i < numSnds; i++)
	{
		if (!Q_stricmp(radio_msgs[i].msg, filteredmsg))
		{
			found = true;
			msg_soundIndex = radio_msgs[i].sndIndex;
			msg_len = radio_msgs[i].length;
			break;
		}
	}
	if (!found)
	{
		Com_Printf("%s '%s' is not a valid radio message", __func__, filteredmsg);
		return;
	}

	//TempFile BEGIN
	if (Q_stricmp(filteredmsg, "enemyd") == 0)
	{
		if (ent->client->radio_num_kills > 1 && ent->client->radio_num_kills <= 10)
		{
			// If we are reporting enemy down, add the number of kills.
			sprintf(msgname_num, "%i", ent->client->radio_num_kills);
			ent->client->radio_num_kills = 0;	// prevent from getting into an endless loop

			BOTLIB_RadioBroadcast(ent, partner, msgname_num);	// Now THAT'S recursion! =)
		}
		ent->client->radio_num_kills = 0;
	}

	if (partner)
	{
		BOTLIB_AddRadioMsg(&partner->client->resp.radio, msg_soundIndex, msg_len, ent);
		return;
	}

	//AQ2:TNG END
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		//Com_Printf("%s [%d]%s [%d]%s\n", __func__, ent->client->resp.team, ent->client->pers.netname, other->client->resp.team, other->client->pers.netname); //ent1->client->resp.team == ent2->client->resp.team;
		if (!OnSameTeam(ent, other))
			continue;
		BOTLIB_AddRadioMsg(&other->client->resp.radio, msg_soundIndex, msg_len, ent);
	}
}

// Allow bots to send a chat message to players
void BOTLIB_Say(edict_t* ent, char* pMsg, qboolean team_message)
{
	int     j, /*i,*/ offset_of_text;
	edict_t* other;
	char    text[2048];
	//gclient_t *cl;


	if (!teamplay->value)
		return;

	if (ent->client->resp.team == NOTEAM)
		return;

	if (ent->solid == SOLID_NOT || ent->deadflag == DEAD_DEAD)
	{
		if (team_message)
			Com_sprintf(text, sizeof(text), "[DEAD] (%s):", ent->client->pers.netname); // Dead, say team
		else
			Com_sprintf(text, sizeof(text), "[DEAD] %s:", ent->client->pers.netname); // Dead, say all
	}
	else if (team_message)
		Com_sprintf(text, sizeof(text), "(%s):", ent->client->pers.netname); // Alive, say team
	else
		Com_sprintf(text, sizeof(text), "%s:", ent->client->pers.netname); // Alive, say all

	offset_of_text = strlen(text);  //FB 5/31/99

	strcat(text, pMsg);

	// don't let text be too long for malicious reasons
	// ...doubled this limit for Axshun -FB
	if (strlen(text) > 300)
		text[300] = 0;

	if (ent->solid != SOLID_NOT && ent->deadflag != DEAD_DEAD)
		ParseSayText(ent, text + offset_of_text, strlen(text));  //FB 5/31/99 - offset change
	// this will parse the % variables, 
	// and again check 300 limit afterwards -FB
	// (although it checks it without the name in front, oh well)

	strcat(text, "\n");

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (other->is_bot || Q_stricmp(other->classname, "bot") == 0) // Don't send msg to bots
			continue;
		if (team_message && !OnSameTeam(ent, other)) // If team message
			continue;
		if (teamplay->value && team_round_going)
		{
			if ((ent->solid == SOLID_NOT || ent->deadflag == DEAD_DEAD)
				&& (other->solid != SOLID_NOT && other->deadflag != DEAD_DEAD))
				continue;
		}

		safe_cprintf(other, PRINT_CHAT, "%s", text);
	}
}

// Returns kill counter. Each time this is checked, the kill counter is reset.
static int BOTLIB_ReadKilledPlayers(edict_t* ent)
{
	int kills = 0;
	edict_t* targ;

	for (int i = 0; i < MAX_LAST_KILLED; i++)
	{
		targ = ent->client->last_killed_target[i];
		if (!targ)
			break;

		if (!targ->inuse || !targ->client || OnSameTeam(ent, targ)) // Remove disconnected or team players from list
		{
			for (int j = i + 1; j < MAX_LAST_KILLED; j++)
				ent->client->last_killed_target[j - 1] = ent->client->last_killed_target[j];

			ent->client->last_killed_target[MAX_LAST_KILLED - 1] = NULL;
			i--;
			continue;
		}

		kills++;
	}

	return kills;
}

// Reports a kill counter and adds killed names to the buffer
static void BOTLIB_GetLastKilledTargets(edict_t* self, int kills, char* buf)
{
	if (kills > 1)
		sprintf(buf, " %c [ %d ] %dx Enemies Down! [ %c ", '\x06', self->client->resp.score, kills, '\x07'); // Build bandage string
	else
		sprintf(buf, " %c [ %d ] Enemy Down! [ %c ", '\x06', self->client->resp.score, '\x07'); // Build bandage string

	Q_strncatz(buf, self->client->last_killed_target[0]->client->pers.netname, PARSE_BUFSIZE); // First kill

	// If multiple kills
	for (int i = 1; i < kills; i++) // Max kills: MAX_LAST_KILLED
	{
		if (i == kills - 1) // Last two kills (a, b, c, d and e)
			Q_strncatz(buf, " and ", PARSE_BUFSIZE);
		else // Multiple kills (a, b,...)
			Q_strncatz(buf, ", ", PARSE_BUFSIZE);

		// Print the additional player names
		Q_strncatz(buf, self->client->last_killed_target[i]->client->pers.netname, PARSE_BUFSIZE);
	}

	Q_strncatz(buf, " ]", PARSE_BUFSIZE);

	self->client->last_killed_target[0] = NULL;
}

// Builds a list of enemy names in sight
static void BOTLIB_GetEnemyList(edict_t* self, char* buf)
{
	sprintf(buf, "[ %s", players[self->bot.enemies[0]]->client->pers.netname); // Print first player

	// If multiple enemies
	if (self->bot.enemies_num > 0)
	{
		for (int i = 1; i < self->bot.enemies_num; i++)
		{
			if (i == 3) // If names exceed the limit
			{
				char extra[20];
				sprintf(extra, ", and %d more...", self->bot.enemies_num - 3);
				Q_strncatz(buf, extra, PARSE_BUFSIZE);
				break;
			}

			if (i == self->bot.enemies_num - 1)
				Q_strncatz(buf, ", and ", PARSE_BUFSIZE); // Last two players (a, b, c, d and e)
			else
				Q_strncatz(buf, ", ", PARSE_BUFSIZE); // Multiple players (a, b,...)

			Q_strncatz(buf, players[self->bot.enemies[i]]->client->pers.netname, PARSE_BUFSIZE); // Print the additional player names
		}
	}

	Q_strncatz(buf, " ]", PARSE_BUFSIZE);
}

// Returns a list of nearby teammates
static void BOTLIB_GetNearbyTeamList(edict_t* self, char* buf)
{
	sprintf(buf, "[ %s", players[self->bot.allies[0]]->client->pers.netname); // Print first player

	for (int i = 1; i < self->bot.allies_num; i++)
	{
		if (i == 3) // If names exceed the limit
		{
			char extra[20];
			sprintf(extra, ", and %d more", self->bot.allies_num - 3);
			Q_strncatz(buf, extra, PARSE_BUFSIZE);
			break;
		}

		if (i == self->bot.allies_num - 1)
			Q_strncatz(buf, ", and ", PARSE_BUFSIZE); // Last two players (a, b, c, d and e)
		else
			Q_strncatz(buf, ", ", PARSE_BUFSIZE); // Multiple players (a, b,...)

		Q_strncatz(buf, players[self->bot.allies[i]]->client->pers.netname, PARSE_BUFSIZE); // Print the additional player names
	}

	Q_strncatz(buf, " ]", PARSE_BUFSIZE);
}

// Radio commands
void BOTLIB_Radio(edict_t* self, usercmd_t* ucmd)
{
	char buffer[256]; // Chat buffer

	if (teamplay->value == 0) return; // Only radio in TP games
	if (team_round_going == false || lights_camera_action) return; // Only allow radio during a real match (after LCA and before win/loss announcement)
	//if (team_round_going && lights_camera_action == 0)

	//if (self->groundentity == NULL) return; // Don't wave when not on ground
	//if (self->client->ps.pmove.pm_flags & PMF_DUCKED) return; // Don't wave when ducked
	//if (self->client->anim_priority > ANIM_WAVE) return; // Don't wave when higher animation priority are active

	// Find how many bots on our team are alive
	short bots_alive = 0;
	for (int i = 0; i <= num_players; i++)
	{
		if (players[i] == NULL || players[i]->client == NULL || players[i]->solid == SOLID_NOT || players[i]->deadflag != DEAD_NO)
			continue;

		if (players[i]->is_bot == false)
			continue;

		if (OnSameTeam(self, players[i]) == false)
			continue;

		bots_alive++;
	}

	// CALLER: Team report in
	if (self->bot.radioTeamReportedIn == false &&
		BOTLIB_EnemiesAlive(self) && //BOTLIB_AlliesAlive(self) && 
		self->bot.see_enemies == false)
	{
		if (self->bot.allies_num > 0)
			self->bot.radioLastSawAllyTime = 0; // Reset counter because we spotted an ally
		else
			self->bot.radioLastSawAllyTime++; // No ally seen

		if (self->bot.radioLastSawAllyTime > self->bot.radioTeamReportedInDelay) // Haven't seen any friendly players in a while
		{
			self->bot.radioTeamReportedIn = true; // Only allow once per round

			sprintf(buffer, " %c Team report in %c", '\x0E', '\x0E'); // Build string
			BOTLIB_Say(self, buffer, true);						// Say it

			self->bot.bi.radioflags |= RADIO_TEAM_REPORT_IN;	// Wave
			BOTLIB_RadioBroadcast(self, NULL, "treport");		// Sound

			// Find a bot on our team who is alive, request they answer the report status
			for (int i = 0; i <= num_players; i++)
			{
				if (players[i] == NULL || players[i] == self || players[i]->client == NULL || players[i]->solid == SOLID_NOT || (players[i]->flags & FL_NOTARGET) || players[i]->deadflag != DEAD_NO)
					continue;

				if (players[i]->is_bot == false)
					continue;

				if (OnSameTeam(self, players[i]) == false)
					continue;

				// We found a bot to report its status
				players[i]->bot.radioReportIn = true; // Request this bot report in
				break;
			}
		}
	}

	// CALLEE: Random chance to flag this bot to respond to a 'team report in' request from a human
	if (Q_stricmp(self->bot.radioLastHumanMsg, "treport") == 0)
	{
		if (bots_alive)
		{
			if ((rand() % bots_alive) == 1) // Scale chance based on bots on our team are alive (2 bots = 50%, 4 bots = 25%, ...)
				self->bot.radioReportIn = true; // Request this bot report in
		}

		self->bot.radioLastHumanMsg[0] = '\0'; // Remove last radio call from bot memory
	}

	// CALLEE: Bot responding in to a 'team report in' request (either bot or human)
	if (self->bot.radioReportIn)
	{
		self->bot.radioReportInTime++;

		if (self->bot.radioReportInTime > self->bot.radioReportInDelay) // RNG delay
		{
			self->bot.radioReportInTime = 0;
			self->bot.radioReportIn = false;

			// Reaport near enemies
			if (self->bot.enemies_num)
			{
				char nearby_enemies[PARSE_BUFSIZE];
				BOTLIB_GetEnemyList(self, nearby_enemies);
				sprintf(buffer, " %c Status [ %d%c ] near enemies ", '\x0E', self->health, '\x05'); // Build string
				Q_strncatz(buffer, nearby_enemies, PARSE_BUFSIZE);
				//Com_Printf("%s %s: %s\n", __func__, self->client->pers.netname, buffer);
			}
			// Report near allies
			else if (self->bot.allies_num)
			{
				char nearby_team[PARSE_BUFSIZE];
				BOTLIB_GetNearbyTeamList(self, nearby_team);
				sprintf(buffer, " %c Status [ %d%c ] near team ", '\x0F', self->health, '\x05'); // Build string
				Q_strncatz(buffer, nearby_team, PARSE_BUFSIZE);
				//Com_Printf("%s %s: %s\n", __func__, self->client->pers.netname, buffer);
			}
			// Report alone
			else
			{
				int report_type = (rand() % 2);

				// Basic report
				if (report_type == 0)
				{
					sprintf(buffer, " %c Reporting in! [ %d%c ]", '\x0F', self->health, '\x05'); // Build string
				}
				else
				{
					char weap_name[PARSE_BUFSIZE];
					char item_name[PARSE_BUFSIZE];
					GetWeaponName(self, weap_name);
					GetItemName(self, item_name);
					sprintf(buffer, " %c Reporting in! [ %d%c ] Equipped with %c %s and %s", '\x0F', self->health, '\x05', '\x07', weap_name, item_name); // Build string
				}
			}

			BOTLIB_Say(self, buffer, true);					// Say it

			self->bot.bi.radioflags |= RADIO_REPORTIN;		// Wave
			BOTLIB_RadioBroadcast(self, NULL, "reportin");	// Sound
		}
	}


	//BOTUT_Cmd_Say_f(self, "Equipped with %W and %I. Current health %H."); // Text


	// Bandaging
	if (self->bot.radioBandaging) // If bandaging, flag the bot to radio this in
	{
		if (IS_ALIVE(self))
		{
			sprintf(buffer, " %c Bandaging [ %d%c ] %c", '\x04', self->health, '\x05', '\x04'); // Build string - 0x04 bandage symbol, 0x05 heart symbol
			BOTLIB_Say(self, buffer, true);					// Say it

			if (self->health > 75) // Taking fire
			{
				self->bot.bi.radioflags |= RADIO_TAKING_FIRE;	// Wave
				BOTLIB_RadioBroadcast(self, NULL, "taking_f");	// Sound
			}
			else if (self->health > 50) // Cover me
			{
				self->bot.bi.radioflags |= RADIO_COVER;			// Wave
				BOTLIB_RadioBroadcast(self, NULL, "cover");		// Sound
			}
			else // I'm hit
			{
				self->bot.bi.radioflags |= RADIO_IM_HIT;		// Wave
				BOTLIB_RadioBroadcast(self, NULL, "im_hit");	// Sound
			}
		}

		self->bot.radioBandaging = false;
	}

	//BOTUT_Cmd_Say_f(self, "Equipped with %W and %I. Current health %H."); // Text

	// If one or more enemies have been killed, report them as enemy down
	// Each report clears the list of enemies killed.
	if (self->bot.radioReportKills && self->bot.see_enemies == false) // Delay report until free of enemies
	{
		int kills = BOTLIB_ReadKilledPlayers(self);
		//if (kills && (rand() % 100 <= kills) ) // Scale chance based kills, more kills == more chance to report

		/*if (kills && kills < 2 && (rand() % 100 > 30))
		{
			self->client->last_killed_target[0] = NULL;
			kills = 0;
		}*/

		if (kills)
		{
			BOTLIB_GetLastKilledTargets(self, kills, buffer);	// Build string
			BOTLIB_Say(self, buffer, true);						// Say it

			self->bot.bi.radioflags |= RADIO_ENEMY_DOWN;	// Wave

			// Radio how many enemies killed
			/*switch (kills)
			{
				case 1: BOTLIB_RadioBroadcast(self, NULL, "1"); break;
				case 2: BOTLIB_RadioBroadcast(self, NULL, "2"); break;
				case 3: BOTLIB_RadioBroadcast(self, NULL, "3"); break;
				case 4: BOTLIB_RadioBroadcast(self, NULL, "4"); break;
				case 5: BOTLIB_RadioBroadcast(self, NULL, "5"); break;
				case 6: BOTLIB_RadioBroadcast(self, NULL, "6"); break;
				case 7: BOTLIB_RadioBroadcast(self, NULL, "7"); break;
				case 8: BOTLIB_RadioBroadcast(self, NULL, "8"); break;
				case 9: BOTLIB_RadioBroadcast(self, NULL, "9"); break;
				default: break;
			}*/

			// Radio enemy down
			BOTLIB_RadioBroadcast(self, NULL, "enemyd");	// Sound
		}
	}
}
