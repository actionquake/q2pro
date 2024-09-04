#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

int	dc_total_male_names;		// Total male names
int	dc_total_female_names;		// Total female names
bot_names_t bot_male[MAX_BOT_NAMES];	// Cached copy
bot_names_t bot_female[MAX_BOT_NAMES];	// Cached copy
bot_connections_t bot_connections;

// Find a free entity for the bot to use
edict_t* BOTLIB_FindFreeEntity(void)
{
	edict_t* bot = NULL;
	for (int i = game.maxclients; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			return bot; // Success
	}

	return NULL; // Failure
}

qboolean BOTLIB_GetRandomBotFileLine(const char* file, char* buffer)
{
	FILE* f;
	//int		comments_num = 0;		// Keep track of how many comments the file has
	int		line_num = 0;			// Keep track of lines
	int		curr_len;				// Current length of the line
	char	curr_line[1024];		// Accounts for reading lines that could be fairly long (comments)
	curr_line[0] = '\0';			// Current line

	cvar_t* game_dir = gi.cvar("game", "action", 0); // Directory of the gamelib
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);	 // Directory of the bot files in the gamelib
	char	filename[MAX_QPATH];	// Filename to load from
#ifdef	_WIN32
	int	i;							// Keep track where we are in the filename array
	i = sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, botdir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, file);
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/");
	strcat(filename, file);
#endif

	if (strlen(filename) <= 0) return false;

	// Open and check for success
	if ((f = fopen(filename, "r")) == NULL) // Read text file
	{
		gi.dprintf("%s failed to open bot file: %s\n", __func__, filename);
		return false;
	}

	// Find how many lines the file has
	int random_line;
	while (fgets(curr_line, sizeof(curr_line), f) != NULL)
	{
		line_num++;
	}
	fclose(f);

	// Seed the random number generator with a high-resolution timestamp
	// so that we don't get accidental repeats of random bot names
	seed_random_number_generator();
	random_line = rand() % line_num + 1;

	if(bot_debug->value)
		gi.dprintf("%s %s random_line[%d]\n", __func__, filename, random_line);

	//Com_Printf("%s %s random_line[%d]\n", __func__, filename, random_line);

	// Open and check for success
	if ((f = fopen(filename, "r")) != NULL) // Read text file
	{
		line_num = -1;

		// Read each line
		while (fgets(curr_line, sizeof(curr_line), f) != NULL)
		{
			line_num++; // Advance forward

			//if (feof(f)) // End of file
			//	break; // Could not get a prefix name

			if (curr_line[0] == '/' && curr_line[1] == '/') // Skip comment lines
				continue;

			// If we're at or above the correct line, not a comment, not empty
			//if (line_num >= random_line && curr_line[0] != '/' && curr_line[0])
			if (line_num >= random_line)
			{
				curr_len = strlen(curr_line);
				if (curr_len < MAX_QPATH) // Make sure name is within acceptable length
				{
					//Com_Printf("%s %s curr_line[%d] = %s\n", __func__, filename, line_num, curr_line);
					Q_strlcpy(buffer, curr_line, curr_len);
					break;
				}
			}
		}
		fclose(f);
	}

	if (strlen(buffer) > 0)
	{
		return true;
	}
	else
	{
		buffer[0] = '\0'; // Terminate empty string
		return false;
	}
}

// Save bots from the previous map to file
#define BOTS_FILE_PREV_MAP_VERSION 1
qboolean BOTLIB_SaveBotsFromPreviousMap(void)
{
	FILE* f;
	char filename[128];
	const char* file = "prev_map_bots.txt";
	int i;
	cvar_t* game_dir = gi.cvar("game", "action", 0);
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

	//if (bot_connections.total_bots <= 0) // Don't write unless we have bots
	//	return false;

#ifdef _WIN32
	i = sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, botdir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, file);
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/");
	strcat(filename, file);
#endif

	//Com_Printf("%s Writing previous bots to file %s\n", __func__, filename);

	// Write to file
	f = fopen(filename, "w"); // Open the file in write mode
	if (f == NULL) 
	{
		Com_Printf("Could not write to file %s\n", filename);
		return false;
	}

	fprintf(f, "// Bots from a previous map\n");
	fprintf(f, "%d\n", BOTS_FILE_PREV_MAP_VERSION);
	fprintf(f, "%d\n", bot_connections.total_bots);
	fprintf(f, "%d\n", (int)bot_maxteam->value);
	if (use_3teams->value)
		fprintf(f, "3teams\n");
	else if (teamplay->value)
		fprintf(f, "teamplay\n");
	else
		fprintf(f, "deathmatch\n");

	for (i = 0; i < num_players; i++)
	{
		if (players[i]->is_bot == false) continue; // Skip humans

		// Gender
		fprintf(f, "%d\n", players[i]->client->pers.gender);

		// Team
		fprintf(f, "%d\n", players[i]->client->resp.team);

		// Name
		fprintf(f, "%s\n", players[i]->client->pers.netname);

		// Skin
		fprintf(f, "%s\n", Info_ValueForKey(players[i]->client->pers.userinfo, "skin")); 
	}

	fclose(f); // Close the file after use

	return true;
}

// This will return when we should rotate bots, default false
qboolean BotCountManagerTimer(void)
{
	// Consider rotating when the map has freshly loaded
	if (level.framenum < 100) {
		BOTLIB_Debug("%s: BotCountManagerTimer: framenum\n");
		return true;
	}
	// Consider rotating when the map has ended
	if (level.intermission_framenum) {
		BOTLIB_Debug("%s: BotCountManagerTimer: intermission\n");
		return true;
	}
	// Consider rotating at 5% chance
	if (rand() % 100 < 5) {
		BOTLIB_Debug("%s: BotCountManagerTimer: percent\n");
		return true;
	}

	return false;
}

void BOTLIB_BotCountAdd(int bots_to_spawn)
{
	bots_to_spawn = abs(bots_to_spawn);
	for (int i = 0; i < bots_to_spawn; i++) {
		if (teamplay->value) // Add a bot to a team with the lowest player count
		{
			if (use_3teams->value) // 3TEAMS
			{
				if (bot_connections.total_team1 <= bot_connections.total_team2 && bot_connections.total_team1 <= bot_connections.total_team3)
					BOTLIB_SpawnBot(TEAM1, INVALID, NULL, NULL);
				else if (bot_connections.total_team2 <= bot_connections.total_team1 && bot_connections.total_team2 <= bot_connections.total_team3)
					BOTLIB_SpawnBot(TEAM2, INVALID, NULL, NULL);
				else if (bot_connections.total_team3 <= bot_connections.total_team1 && bot_connections.total_team3 <= bot_connections.total_team2)
					BOTLIB_SpawnBot(TEAM3, INVALID, NULL, NULL);
				else // If all teams are equal size, randomly add bot to one of the teams
				{
					int choice = rand() % 3;
					if (choice == 0)
						BOTLIB_SpawnBot(TEAM1, INVALID, NULL, NULL);
					else if (choice == 1)
						BOTLIB_SpawnBot(TEAM2, INVALID, NULL, NULL);
					else
						BOTLIB_SpawnBot(TEAM3, INVALID, NULL, NULL);
				}
			}
			else // 2-team Teamplay
			{
				if (bot_connections.total_team1 < bot_connections.total_team2)
					BOTLIB_SpawnBot(TEAM1, INVALID, NULL, NULL);
				else if (bot_connections.total_team1 > bot_connections.total_team2)
					BOTLIB_SpawnBot(TEAM2, INVALID, NULL, NULL);
				else // If both teams are equal size, randomly add bot to one of the teams
				{
					if (rand() % 2 == 0)
						BOTLIB_SpawnBot(TEAM1, INVALID, NULL, NULL);
					else
						BOTLIB_SpawnBot(TEAM2, INVALID, NULL, NULL);
				}
			}
		}
		else // DM
			BOTLIB_SpawnBot(0, INVALID, NULL, NULL);
	}
}

void BOTLIB_BotCountRemove(int bots_to_spawn)
{
	bots_to_spawn = abs(bots_to_spawn);
	for (int i = 0; i < bots_to_spawn; i++) {
		if (teamplay->value) // Remove a bot from team with the highest player count
		{
			if (use_3teams->value)
			{
				if (bot_connections.total_team1 >= bot_connections.total_team2 && bot_connections.total_team1 >= bot_connections.total_team3)
					BOTLIB_RemoveTeamplayBot(TEAM1);
				else if (bot_connections.total_team2 >= bot_connections.total_team1 && bot_connections.total_team2 >= bot_connections.total_team3)
					BOTLIB_RemoveTeamplayBot(TEAM2);
				else if (bot_connections.total_team3 >= bot_connections.total_team1 && bot_connections.total_team3 >= bot_connections.total_team2)

					BOTLIB_RemoveTeamplayBot(TEAM3);
				else // If all teams are equal size, randomly remove bot to one of the teams
				{
					int choice = rand() % 3;
					if (choice == 0)
						BOTLIB_RemoveTeamplayBot(TEAM1);
					else if (choice == 1)
						BOTLIB_RemoveTeamplayBot(TEAM2);
					else
						BOTLIB_RemoveTeamplayBot(TEAM3);
				}
			}
			else
			{
				if (bot_connections.total_team1 > bot_connections.total_team2)
					BOTLIB_RemoveTeamplayBot(TEAM1);
				else if (bot_connections.total_team1 < bot_connections.total_team2)
					BOTLIB_RemoveTeamplayBot(TEAM2);
				else // If both teams are equal size, randomly remove bot to one of the teams
				{
					if (rand() % 2 == 0)
						BOTLIB_RemoveTeamplayBot(TEAM1);
					else
						BOTLIB_RemoveTeamplayBot(TEAM2);
				}
			}
		}
		else // DM
			BOTLIB_RemoveBot("");
	}
}

void BOTLIB_BotCountManager(int bots_to_spawn)
{
	if (bots_to_spawn < 0) {
		BOTLIB_BotCountRemove(bots_to_spawn);
	} else if (bots_to_spawn > 0) {
		BOTLIB_BotCountAdd(bots_to_spawn);
	} else {  // bots_to_spawn == 0 - no bots to add or remove
		return;
	}
}

// Add bots from the previous map from file
// Parameters: 
// percent of bots to keep from previous map
qboolean BOTLIB_AddBotsFromPreviousMap(float percent)
{
	FILE* f;
	qboolean print_dbg = false;		// Print debug info
	int		line_num = 0;			// Keep track of lines

	int		prev_bots_skipped = 0;	// How many bots we skipped adding from previous map
	int		prev_bot_count = 0;		// How many bots from the previous map
	int		bots_added = 0;			// How many bots we added from previous map
	char	gamemode[MAX_QPATH];	// Gamemode string buffer
	int		prev_gamemode = 0;		// Previous gamemode
	int		prev_team = 0;			// Previous Team
	int		prev_gender = 0;		// Previous Gender
	char	prev_name[MAX_QPATH];	// Previous Name
	char	prev_skin[MAX_QPATH];	// Previous Skin
	
	int		curr_len = 0;			// Current length of the line
	char	curr_line[1024];		// Accounts for reading lines that could be fairly long (comments)
	curr_line[0] = '\0';			// Current line

	const char* file = "prev_map_bots.txt";
	cvar_t* game_dir = gi.cvar("game", "action", 0); // Directory of the gamelib
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);	 // Directory of the bot files in the gamelib
	char	filename[MAX_QPATH];	// Filename to load from

#ifdef	_WIN32
	int	i;							// Keep track where we are in the filename array
	i = sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, botdir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, file);
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/");
	strcat(filename, file);
#endif

	if (strlen(filename) <= 0) return false;

	// Open and check for success
	if ((f = fopen(filename, "r")) == NULL) // Read text file
	{
		return false; // Ignore if file doesn't exist
	}

	// Open and check for success
	if ((f = fopen(filename, "r")) != NULL) // Read text file
	{
		// Ignore first line
		fgets(curr_line, sizeof(curr_line), f); // First line is commented
		if (print_dbg) Com_Printf("%s line[%d] %s", __func__, ++line_num, curr_line);

		// Get file version
		fgets(curr_line, sizeof(curr_line), f);
		int version = atoi(curr_line);
		if (print_dbg) Com_Printf("%s line[%d] version[%d]\n", __func__, ++line_num, version);
		if (version != BOTS_FILE_PREV_MAP_VERSION)
		{
			return false;
		}

		// Get previous bot count
		fgets(curr_line, sizeof(curr_line), f);
		prev_bot_count = atoi(curr_line);
		if (print_dbg) Com_Printf("%s line[%d] bot_count[%d]\n", __func__, ++line_num, prev_bot_count);
		if (prev_bot_count <= 0)
		{
			//gi.dprintf("%s failed to process bot count from file: %s\n", __func__, filename);
			return false;
		}

		// Get previous bot_maxteam num
		fgets(curr_line, sizeof(curr_line), f);
		//bot_maxteam->value = atoi(curr_line);
		gi.cvar_set("bot_maxteam", va("%d", atoi(curr_line))); // Override if manually added
		if (print_dbg) Com_Printf("%s line[%d] bot_maxteam[%f]\n", __func__, ++line_num, bot_maxteam->value);
		if (bot_maxteam->value < 0 || bot_maxteam->value > MAX_CLIENTS)
		{
			gi.cvar_set("bot_maxteam", va("%d", 0)); // Override if manually added
			gi.dprintf("%s failed to process bot_maxteam from file: %s\n", __func__, filename);
			return false;
		}

		// Get gamemode
		fgets(curr_line, sizeof(curr_line), f); // Second line is gamemode
		if (print_dbg) Com_Printf("%s line[%d] %s", __func__, ++line_num, curr_line);
		curr_len = strlen(curr_line);
		Q_strlcpy(gamemode, curr_line, curr_len);
		if (Q_strcasestr(gamemode, "deathmatch") != NULL)
		{
			if (print_dbg) Com_Printf("%s Gamemode is DEATHMATCH\n", __func__);
			prev_gamemode = 1;  // DM
		}
		else if (Q_strcasestr(gamemode, "teamplay") != NULL)
		{
			if (print_dbg) Com_Printf("%s Gamemode is TEAMPLAY\n", __func__);
			prev_gamemode = 2;  // TP
		}
		else if (Q_strcasestr(gamemode, "3teams") != NULL)
		{
			if (print_dbg) Com_Printf("%s Gamemode is 3TEAMS\n", __func__);
			prev_gamemode = 3;  // 3TEAMS
		}

		// Read each line
		while (fgets(curr_line, sizeof(curr_line), f))
		{
			// Get gender
			prev_gender = atoi(curr_line);
			if (prev_gender < GENDER_MALE || prev_gender > GENDER_NEUTRAL)
			{
				Com_Printf("%s failed to process bot gender from file: %s\n", __func__, filename);
				return false;
			}

			// Get team
			fgets(curr_line, sizeof(curr_line), f); // Get next line
			prev_team = atoi(curr_line);
			if (prev_team < NOTEAM || prev_team > TEAM3)
			{
				Com_Printf("%s failed to process bot team from file: %s\n", __func__, filename);
				return false;
			}

			// Get name
			fgets(curr_line, sizeof(curr_line), f); // Get next line
			curr_len = strlen(curr_line);
			if (curr_len >= MAX_QPATH) // Make sure name is within acceptable length
				continue;
			Q_strlcpy(prev_name, curr_line, curr_len);

			// Get skin
			fgets(curr_line, sizeof(curr_line), f); // Get next line
			curr_len = strlen(curr_line);
			if (curr_len >= MAX_QPATH) // Make sure name is within acceptable length
				continue;
			Q_strlcpy(prev_skin, curr_line, curr_len);

			if (print_dbg) Com_Printf("%s %s curr_line[%d]. GM[%s] Gender[%d] Team[%d] Name[%s] Skin[%s]\n", __func__, filename, ++line_num, gamemode, prev_gender, prev_team, prev_name, prev_skin);

			if (random() > (percent / 100)) // Percent chance to skip adding bot from previous map
			{
				prev_bots_skipped++; // Bots that were not added from previous map
				continue;
			}

			//if (bot_maxteam->value && bots_added >= (int)bot_maxteam->value) // Limit bots to bot_maxteam, if its set
			//	break;
			if (prev_bot_count && bots_added >= prev_bot_count)
				break;

			if (use_3teams->value)
			{
				// Bot was previously in a different game mode joining a 3teams game, randomize the team it joins
				if (prev_gamemode == 1 || prev_gamemode == 2) // DM or TP
					prev_team = (rand() % TEAM3) + 1;

				if (prev_team == TEAM1) bot_connections.desire_team1++;
				else if (prev_team == TEAM2) bot_connections.desire_team2++;
				else if (prev_team == TEAM3) bot_connections.desire_team3++;

				if (print_dbg) Com_Printf("%s Gender[%d] Team[%d] Name[%s]\n", __func__, prev_gender, prev_team, prev_name);
				BOTLIB_SpawnBot(prev_team, prev_gender, prev_name, NULL);
				bots_added++;
			}
			else if (teamplay->value) // TP and CTF
			{
				// Bot was previously in a different game mode joining a teamplay game, randomize the team it joins
				if (prev_gamemode == 1 || prev_gamemode == 3) // DM or 3TEAMS
					prev_team = (rand() % TEAM2) + 1;

				if (prev_team == TEAM1) bot_connections.desire_team1++;
				else if (prev_team == TEAM2) bot_connections.desire_team2++;

				if (print_dbg) Com_Printf("%s Gender[%d] Team[%d] Name[%s]\n", __func__, prev_gender, prev_team, prev_name);
				BOTLIB_SpawnBot(prev_team, prev_gender, prev_name, NULL);
				bots_added++;
			}
			else // DM
			{
				if (strlen(prev_skin))
					BOTLIB_SpawnBot(0, prev_gender, prev_name, prev_skin);
				else
					BOTLIB_SpawnBot(0, prev_gender, prev_name, NULL);
				bots_added++;
			}
		}
		fclose(f);
	}


	if (bots_added)
	{
		//if (bot_maxteam->value < 1) // If max team wasn't set, use it so bots can be shuffled
		//	bot_maxteam->value = prev_bot_count;

		bot_connections.desire_bots = bots_added; // How many bots
		//bot_connections.auto_balance_bots = true;

		return true;
	}
	
	return false;
}

void BOTLIB_RandomizeTeamNames(edict_t* bot)
{
	char name[MAX_QPATH];
	int len = 0;
	const int max_name_size = sizeof(teams[TEAM1].name); // Team names are limited to 20 characters (19 chars + terminator)

	//qboolean same_name = false;
	for (int i = 0; i < MAX_TEAMS; i++) // For each team
	{
		// Try for random team names that fit within 18 characters
		for (int t = 0; t < 32; t++) // Max loops
		{
			BOTLIB_GetRandomBotFileLine("team_names.txt", name);  // Get a random team name

			// Get name lengths
			len = strlen(name);

			// Find if the name length will fit within 20 chars (name + space + terminator == 20)
			if (len <= (max_name_size - 2)) // adjust max size to account for space + terminator
			{
				// Make sure the name is different
				if (Q_strcasestr(name, teams[TEAM1].name) != NULL) // If same name as Team1
					continue;
				if (Q_strcasestr(name, teams[TEAM2].name) != NULL) // If same name as Team2
					continue;
				if (Q_strcasestr(name, teams[TEAM3].name) != NULL) // If same name as Team3
					continue;

				// Merge the names together
				Q_strncpyz(teams[i].name, va("%s", name), len + 2); // name + space + terminator
				Com_Printf("%s Setting Team Name: %s\n", __func__, teams[i].name);
				break;
			}
		}
	}
}


void BOTLIB_RandomizeTeamSkins(edict_t* bot)
{
	//Com_Printf("%s before T1[%s] T2[%s] T3[%s]\n", __func__, teams[TEAM1].skin, teams[TEAM2].skin, teams[TEAM3].skin);

	// Randomize skins
	BOTLIB_RandomSkin(bot, teams[TEAM1].skin, INVALID);
	BOTLIB_RandomSkin(bot, teams[TEAM2].skin, INVALID);
	BOTLIB_RandomSkin(bot, teams[TEAM3].skin, INVALID);

	// Check if any of the randomized skins are the same, if so re-roll them until they're different
	qboolean same_skin = false;
	for (int i = 0; i < 64; i++)
	{
		same_skin = false;
		if (Q_strcasestr(teams[TEAM2].skin, teams[TEAM1].skin) != NULL) // If Team2 is the same skin as Team1
		{
			same_skin = true;
			//Com_Printf("%s same skin detected T1[%s] T2[%s] T3[%s] +++++++++\n", __func__, teams[TEAM1].skin, teams[TEAM2].skin, teams[TEAM3].skin);
			BOTLIB_RandomSkin(bot, teams[TEAM2].skin, INVALID); // Re-randomize skin
		}

		if (Q_strcasestr(teams[TEAM3].skin, teams[TEAM1].skin) != NULL) // If Team3 is the same skin as Team1
		{
			same_skin = true;
			//Com_Printf("%s same skin detected T1[%s] T2[%s] T3[%s] +++++++++\n", __func__, teams[TEAM1].skin, teams[TEAM2].skin, teams[TEAM3].skin);
			BOTLIB_RandomSkin(bot, teams[TEAM3].skin, INVALID); // Re-randomize skin
		}

		if (Q_strcasestr(teams[TEAM3].skin, teams[TEAM2].skin) != NULL) // If Team3 is the same skin as Team2
		{
			same_skin = true;
			//Com_Printf("%s same skin detected T1[%s] T2[%s] T3[%s] +++++++++\n", __func__, teams[TEAM1].skin, teams[TEAM2].skin, teams[TEAM3].skin);
			BOTLIB_RandomSkin(bot, teams[TEAM3].skin, INVALID); // Re-randomize skin
		}

		if (same_skin == false) // No same skins detected, so exit loop early
			break;
	}

	//Com_Printf("%s after T1[%s] T2[%s] T3[%s]\n", __func__, teams[TEAM1].skin, teams[TEAM2].skin, teams[TEAM3].skin);

	// Update skin associated indexes
	Q_snprintf(teams[TEAM1].skin_index, sizeof(teams[TEAM1].skin_index), "../players/%s_i", teams[TEAM1].skin);
	Q_snprintf(teams[TEAM2].skin_index, sizeof(teams[TEAM2].skin_index), "../players/%s_i", teams[TEAM2].skin);
	Q_snprintf(teams[TEAM3].skin_index, sizeof(teams[TEAM3].skin_index), "../players/%s_i", teams[TEAM3].skin);

	// Change team picture using the updated skin indexes
	level.pic_teamskin[TEAM1] = gi.imageindex(teams[TEAM1].skin_index);
	level.pic_teamskin[TEAM2] = gi.imageindex(teams[TEAM2].skin_index);
	level.pic_teamskin[TEAM3] = gi.imageindex(teams[TEAM3].skin_index);
}

/*
void DC_CacheRandomBotNames(void)
{
	int i, n;

	// Cache male names
	n = 0;
	for (i = 0; i < MAX_BOT_NAMES; i++)
	{
		DC_LoadRandomBotName(GENDER_MALE, &bot_male[n]);
		n++;
	}
	// Cache female names
	n = 0;
	for (i = 0; i < MAX_BOT_NAMES; i++)
	{
		DC_LoadRandomBotName(GENDER_MALE, &bot_female[n]);
		n++;
	}
}
void DC_GetRandomBotName(byte gender, char* bot_name)
{
	if (gender == GENDER_MALE)
	{
		//Q_strlcpy(bot_name, bot_male[n], name_length);
	}
	else if (gender == GENDER_FEMALE)
	{

	}
	else
	{
		if (random() < 0.5)
		{

		}
		else
		{

		}
	}
}
*/

//======================================
// BOTLIB_GetRandomClanSymbol()
// Gets a random clan tag char symbol: ie --> [ ], ( ), { }, < >, etc
// Returns char
//======================================
int BOTLIB_GetRandomClanSymbol(void)
{
	int sym = rand() % 25;
	switch (sym)
	{
	case 0: return '!';
	case 1: return '#';
	case 2: return '$';
	case 3: return '%';
	case 4: return '&';
	case 5: return '(';
	case 6: return ')';
	case 7: return '*';
	case 8: return '+';
	case 9: return ',';
	case 10: return '-';
	case 11: return '.';
	//case 12: return '/';
	case 12: return ':';
	case 13: return '<';
	case 14: return '=';
	case 15: return '>';
	case 16: return '?';
	case 17: return '@';
	case 18: return '[';
	case 19: return ']';
	//case 21: return '\\';
	case 20: return '^';
	case 21: return '_';
	case 22: return '{';
	case 23: return '|';
	case 24: return '}';
	default: return ' ';
	}
}

//======================================
// BOTLIB_GetOpposingClanSymbol()
// If a symbol has an opposite: ie --> [ ], ( ), { }, < >, etc
// Returns opposite side, else returns the input symbol
//======================================
int BOTLIB_GetOpposingClanSymbol(char symbol)
{
	if (symbol == '[')
		return ']';
	if (symbol == ']')
		return '[';

	if (symbol == '(')
		return ')';
	if (symbol == ')')
		return '(';

	if (symbol == '{')
		return '}';
	if (symbol == '}')
		return '{';

	if (symbol == '<')
		return '>';
	if (symbol == '>')
		return '<';

	if (symbol == '\\')
		return '/';
	if (symbol == '/')
		return '\\';

	return symbol;
}


//======================================
// BOTLIB_GetRandomClanLetter()
// Gets a random clan letter
// Returns char
//======================================
int BOTLIB_GetRandomClanLetter(void)
{
	// Gets a random ASCII letter between 65 and 90, or 97 and 122
	int letter = rand() % 52;
	switch (letter)
	{
	case 0: return 'A';
	case 1: return 'B';
	case 2: return 'C';
	case 3: return 'D';
	case 4: return 'E';
	case 5: return 'F';
	case 6: return 'G';
	case 7: return 'H';
	case 8: return 'I';
	case 9: return 'J';
	case 10: return 'K';
	case 11: return 'L';
	case 12: return 'M';
	case 13: return 'N';
	case 14: return 'O';
	case 15: return 'P';
	case 16: return 'Q';
	case 17: return 'R';
	case 18: return 'S';
	case 19: return 'T';
	case 20: return 'U';
	case 21: return 'V';
	case 22: return 'W';
	case 23: return 'X';
	case 24: return 'Y';
	case 25: return 'Z';
	case 26: return 'a';
	case 27: return 'b';
	case 28: return 'c';
	case 29: return 'd';
	case 30: return 'e';
	case 31: return 'f';
	case 32: return 'g';
	case 33: return 'h';
	case 34: return 'i';
	case 35: return 'j';
	case 36: return 'k';
	case 37: return 'l';
	case 38: return 'm';
	case 39: return 'n';
	case 40: return 'o';
	case 41: return 'p';
	case 42: return 'q';
	case 43: return 'r';
	case 44: return 's';
	case 45: return 't';
	case 46: return 'u';
	case 47: return 'v';
	case 48: return 'w';
	case 49: return 'x';
	case 50: return 'y';
	case 51: return 'z';
	default: return ' ';
	}
}



void BOTLIB_GetRandomName(char* name, const int gender)
{
	if (gender == GENDER_MALE)
	{
		if (random() < 0.8)
			BOTLIB_GetRandomBotFileLine("males.txt", name);
		else
			BOTLIB_GetRandomBotFileLine("other.txt", name);
	}
	else // GENDER_FEMALE
		BOTLIB_GetRandomBotFileLine("females.txt", name);

	//if (strlen(name) <= 0) // Something went wrong. No name was set.
	//	Q_strlcpy(name, "AqtionMan", 9); // Set a default
}

void BOTLIB_GetNamePrefix(char* name, const int type)
{
	if (type)
		BOTLIB_GetRandomBotFileLine("clans.txt", name);  // Clan tag
	else
		BOTLIB_GetRandomBotFileLine("prefix.txt", name); // Prefix
}

void BOTLIB_GetNamePostfix(char* name, const int type)
{
	BOTLIB_GetRandomBotFileLine("postfix.txt", name); // Postfix
}

void BOTLIB_GetRandomPrefix(char* name)
{
	byte tag = 0;
	char outer_symbol = 0;
	char inner_symbol = 0;

	outer_symbol = BOTLIB_GetRandomClanSymbol();
	if (random() < 0.5) // 50% chance to add an inner symbol
		inner_symbol = BOTLIB_GetRandomClanSymbol();

	// Add the prefix tag
	name[tag] = outer_symbol;
	if (inner_symbol)
		name[++tag] = inner_symbol;

	// Add the clan acronym
	name[++tag] = BOTLIB_GetRandomClanLetter();
	name[++tag] = BOTLIB_GetRandomClanLetter();
	if (random() < 0.25) name[++tag] = BOTLIB_GetRandomClanLetter(); // percent chance to add a third clan letter

	// Add the suffix tag
	if (inner_symbol)
		name[++tag] = BOTLIB_GetOpposingClanSymbol(inner_symbol); // Add the opposing symbol, if any
	name[++tag] = BOTLIB_GetOpposingClanSymbol(outer_symbol);

	// print the bot name
	name[++tag] = '\0';
	//Com_Printf("CLAN TAG  %s\n", name);
}



//======================================
// DC_LoadRandomBotName()
// Gets a random bot name from file; bot_name is limited to a maximum of 16 chars, and the size of the list can be up to random()'s maximum of 32,767
// Genders can be: GENDER_MALE, GENDER_FEMALE, GENDER_NEUTRAL
// Returns bot_name
//======================================
void BOTLIB_LoadRandomBotName(edict_t* bot, char* bot_name, int gender)
{
	//for (int n = 0; n < 100; n++)
	{
		int rnglen = 0;					// Length of rng name
		int namelen = 0;				// Length of name
		int clanlen = 0;				// Length of clan name
		int prefixlen = 0;				// Length of Prefix name
		int postfixlen = 0;				// Length of Postfix name
		char name[MAX_QPATH];			// Name
		char clan_name[MAX_QPATH];		// Clan name
		char prefix_name[MAX_QPATH];	// Prefix name
		char postfix_name[MAX_QPATH];	// Postfix name
		char rng_clan_name[MAX_QPATH];	// Random Clan name
		qboolean name_with_rng = false;		// If name + rng clan will fit within 16 characters
		qboolean name_with_clan = false;	// If name + clan will fit within 16 characters
		qboolean name_with_prefix = false;	// If name + prefix will fit within 16 characters
		qboolean name_with_postfix = false;	// If name + postfix will fit within 16 characters

		// Get random names from file
		//Q_strlcpy(name, "AqtionMan", 10);	// Set a default name
		BOTLIB_GetRandomName(name, gender); // Get name based on gender
		BOTLIB_GetNamePrefix(clan_name, 0);
		BOTLIB_GetNamePrefix(prefix_name, 1);
		BOTLIB_GetNamePostfix(postfix_name, 1);
		BOTLIB_GetRandomPrefix(rng_clan_name);

		// Decide if we're going to only take only part of a name (i.e. Arnold Schwarzenegger - we can take either Arnold or Schwarzenegger)
		if (rand() % 2 == 0)
		{
			qboolean two_part_name = false;
			char name_cpy[MAX_QPATH];
			char name_pt1[MAX_QPATH];
			char name_pt2[MAX_QPATH];
			Q_strlcpy(name_cpy, name, sizeof(name)); // Make a copy of the original string because strtok() will edit it
			char* split_name = strtok(name_cpy, " ");
			if (split_name != NULL)
			{
				Q_strlcpy(name_pt1, split_name, sizeof(split_name));
				split_name = strtok(NULL, " ");
			}
			if (split_name != NULL)
			{
				two_part_name = true;
				Q_strlcpy(name_pt2, split_name, sizeof(split_name));
			}
			if (two_part_name)
			{
				//Com_Printf("%s x1 name[%s]\n", __func__, name_pt1, strlen(name_pt1));
				//Com_Printf("%s x2 name[%s]\n", __func__, name_pt2, strlen(name_pt2));

				if (rand() % 2 == 0)
					Q_strlcpy(name, name_pt1, sizeof(name_pt1));
				else
					Q_strlcpy(name, name_pt2, sizeof(name_pt2));
			}

			//Com_Printf("%s full name[%s]\n", __func__, name, strlen(name));
		}
		
		//Com_Printf("%s clan[%s][%d] prefix[%s][%d] postfix[%s][%d] rng_clan_name[%s][%d] name[%s][%d]\n", __func__, clan_name, strlen(clan_name), prefix_name, strlen(prefix_name), postfix_name, strlen(postfix_name), rng_clan_name, strlen(rng_clan_name), name, strlen(name));

		// Get name lengths
		namelen = strlen(name);
		rnglen = strlen(rng_clan_name);
		clanlen = strlen(clan_name);
		prefixlen = strlen(prefix_name);
		postfixlen = strlen(postfix_name);

		// Find what fits with the bot's name (14 chars for both names + space in between == 15)
		if ((namelen + rnglen) < 15)
			name_with_rng = true;
		if ((namelen + clanlen) < 15)
			name_with_clan = true;
		if ((namelen + prefixlen) < 15)
			name_with_prefix = true;
		if ((namelen + postfixlen) < 15)
			name_with_postfix = true;

		Q_strncpyz(bot_name, va("%s", name), namelen + 2); // Set a random name

		// Merge the names together
		int name_type = rand() % 100;
		if (name_type < 15 && name_with_rng)
			Q_strncpyz(bot_name, va("%s %s", rng_clan_name, name), namelen + rnglen + 2); // RNG clan + name
		else if (name_type < 20 && name_with_clan)
			Q_strncpyz(bot_name, va("%s %s", clan_name, name), namelen + clanlen + 2); // clan + name
		else if (name_type < 25 && name_with_prefix)
			Q_strncpyz(bot_name, va("%s %s", prefix_name, name), namelen + prefixlen + 2); // prefix + name
		else if (name_type < 30 && name_with_postfix)
			Q_strncpyz(bot_name, va("%s %s", name, postfix_name), namelen + postfixlen + 2); // name + postfix

		//Com_Printf("%s name[%s][%d]\n", __func__, bot_name, strlen(bot_name));
	}
}

// Sets a random skin and returns the skin's gender
// Parameters: gender - If a valid gender is passed, and not INVALID (-1) then pick a skin based on gender
int BOTLIB_RandomSkin(edict_t* bot, char* skin, int force_gender)
{
	int type = 0, rnd;

	strcpy(skin, "male/grunt"); // Default skin

	// Size of each collection of skins
	const int actionmale = 30;
	const int actionrally = 6;
	const int aqmarine = 8;
	const int female = 22;
	const int male = 88;
	const int messiah = 26;
	const int sas = 25;
	const int sydney = 2;
	const int terror = 25;

	// When NOT forcing a gender using
	if (force_gender == INVALID)
	{
		int rng_type = rand() % 2; // Randomize which randomizer we use. Either by [1] Collection Size, or [2] Random Type.

		// -- [1] Collection Size --
		// Since each collection varies in size, our random choice takes into account the size of the collection of skins.
		// Collections with the most skins have a greater chance to be chosen.
		if (rng_type == 0)
		{
			int sum = actionmale + actionrally + aqmarine + female + male + messiah + sas + sydney + terror;
			int rnd = (rand() % sum) + 1; // Random number from total

			if (rnd <= actionmale)
				type = 0; // actionmale
			else if (rnd <= (actionmale + actionrally))
				type = 1; // actionrally
			else if (rnd <= (actionmale + actionrally + aqmarine))
				type = 2; // aqmarine
			else if (rnd <= (actionmale + actionrally + aqmarine + female))
				type = 3; // female
			else if (rnd <= (actionmale + actionrally + aqmarine + female + male))
				type = 4; // male
			else if (rnd <= (actionmale + actionrally + aqmarine + female + male + messiah))
				type = 5; // messiah
			else if (rnd <= (actionmale + actionrally + aqmarine + female + male + messiah + sas))
				type = 6; // sas
			else if (rnd <= (actionmale + actionrally + aqmarine + female + male + messiah + sas + sydney))
				type = 7; // sydney
			else
				type = 8; // terror
		}
		// -- [2] Random Type --
		else // Alternatively we could just randomly pick from one of the collections regardless of how many skins it has
		{
			type = rand() % 9; // Actionmale, actionrally, aqmarine, female, male, messiah, sas, sydney, terror
		}
	}
	else // Forcing a gender
	{
		// (type == 0) // Actionmale -- male
		// (type == 1) // Actionrally -- female
		// (type == 2) // Aqmarine -- male
		// (type == 3) // Female -- female
		// (type == 4) // Male -- male
		// (type == 5) // Messiah -- male
		// (type == 6) // Sas -- male
		// (type == 7) // Sydney -- female
		// (type == 8) // Terror -- male

		if (force_gender == GENDER_MALE)
		{
			int rnd_gen_skin = rand() % 6; // Pick one of the male skins

			if (rnd_gen_skin == 0)		type = 0; // Actionmale
			else if (rnd_gen_skin == 1) type = 2; // Aqmarine
			else if (rnd_gen_skin == 2) type = 4; // Male
			else if (rnd_gen_skin == 3) type = 5; // Messiah
			else if (rnd_gen_skin == 4) type = 6; // Sas
			else						type = 8; // Terror
		}
		else // GENDER_FEMALE
		{
			int rnd_gen_skin = rand() % 2; // Pick one of the female skins

			if (rnd_gen_skin == 0)		type = 1; // Actionrally
			else						type = 3; // Female
			// Skipping Sydney because skin has no VWEP
		}
	}

	if (type == 0) // Actionmale
	{
		rnd = rand() % actionmale;
		switch (rnd)
		{
			case 0: sprintf(skin, "actionmale/agent"); break;
			case 1: sprintf(skin, "actionmale/aqgthugboss"); break;
			case 2: sprintf(skin, "actionmale/axef"); break;
			case 3: sprintf(skin, "actionmale/badmutha"); break;
			case 4: sprintf(skin, "actionmale/BlackStars"); break;
			case 5: sprintf(skin, "actionmale/blood"); break;
			case 6: sprintf(skin, "actionmale/blucedojo"); break;
			case 7: sprintf(skin, "actionmale/bluceree"); break;
			case 8: sprintf(skin, "actionmale/Bruce_Wayne"); break;
			case 9: sprintf(skin, "actionmale/castor"); break;
			case 10: sprintf(skin, "actionmale/chellobeta"); break;
			case 11: sprintf(skin, "actionmale/chucky"); break;
			case 12: sprintf(skin, "actionmale/crip"); break;
			case 13: sprintf(skin, "actionmale/ctf_b"); break;
			case 14: sprintf(skin, "actionmale/ctf_r"); break;
			case 15: sprintf(skin, "actionmale/dafist"); break;
			case 16: sprintf(skin, "actionmale/dubeta"); break;
			case 17: sprintf(skin, "actionmale/invince"); break;
			case 18: sprintf(skin, "actionmale/jewels"); break;
			case 19: sprintf(skin, "actionmale/jungseal"); break;
			case 20: sprintf(skin, "actionmale/killer"); break;
			case 21: sprintf(skin, "actionmale/leonreno"); break;
			case 22: sprintf(skin, "actionmale/lfc"); break;
			case 23: sprintf(skin, "actionmale/morpheus"); break;
			case 24: sprintf(skin, "actionmale/RCMP"); break;
			case 25: sprintf(skin, "actionmale/scarf"); break;
			case 26: sprintf(skin, "actionmale/scarfchain"); break;
			case 27: sprintf(skin, "actionmale/seagull"); break;
			case 28: sprintf(skin, "actionmale/shinobi"); break;
			case 29: sprintf(skin, "actionmale/swat"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_MALE;
	}
	else if (type == 1) // Actionrally
	{
		rnd = rand() % actionrally;
		switch (rnd)
		{
			case 0: sprintf(skin, "actionrally/ctf_b"); break;
			case 1: sprintf(skin, "actionrally/ctf_r"); break;
			case 2: sprintf(skin, "actionrally/modsquad"); break;
			case 3: sprintf(skin, "actionrally/natasha"); break;
			case 4: sprintf(skin, "actionrally/pulp"); break;
			case 5: sprintf(skin, "actionrally/trinity"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_FEMALE;
	}
	else if (type == 2) // Aqmarine
	{
		rnd = rand() % aqmarine;
		switch (rnd)
		{
			case 0: sprintf(skin, "aqmarine/aquamarine"); break;
			case 1: sprintf(skin, "aqmarine/centurion"); break;
			case 2: sprintf(skin, "aqmarine/desert"); break;
			case 3: sprintf(skin, "aqmarine/marine1"); break;
			case 4: sprintf(skin, "aqmarine/marine2"); break;
			case 5: sprintf(skin, "aqmarine/urban"); break;
			case 6: sprintf(skin, "aqmarine/usmc"); break;
			case 7: sprintf(skin, "aqmarine/woods"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_MALE;
	}
	else if (type == 3) // Female
	{
		rnd = rand() % female;
		switch (rnd)
		{
			case 0: sprintf(skin, "female/ctf_b"); break;
			case 1: sprintf(skin, "female/ctf_r"); break;
			case 2: sprintf(skin, "female/kw_aqua"); break;
			case 3: sprintf(skin, "female/kw_black"); break;
			case 4: sprintf(skin, "female/kw_blue"); break;
			case 5: sprintf(skin, "female/kw_green"); break;
			case 6: sprintf(skin, "female/kw_pink"); break;
			case 7: sprintf(skin, "female/kw_red"); break;
			case 8: sprintf(skin, "female/kw_white"); break;
			case 9: sprintf(skin, "female/kw_yellow"); break;
			case 10: sprintf(skin, "female/leeloop"); break;
			case 11: sprintf(skin, "female/sarah_ohconnor"); break;
			case 12: sprintf(skin, "female/tankgirl"); break;

			// baseq2 skins (No longer supplied with Aqtion)
			case 13: sprintf(skin, "female/athena"); break;
			case 14: sprintf(skin, "female/brianna"); break;
			case 15: sprintf(skin, "female/cobalt"); break;
			case 16: sprintf(skin, "female/doomgal"); break;
			case 17: sprintf(skin, "female/ensign"); break;
			case 18: sprintf(skin, "female/jezebel"); break;
			case 19: sprintf(skin, "female/jungle"); break;
			case 20: sprintf(skin, "female/venus"); break;
			case 21: sprintf(skin, "female/voodoo"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_FEMALE;
	}
	else if (type == 4) // Male
	{
		rnd = rand() % male;
		switch (rnd)
		{
			case 0: sprintf(skin, "male/{oz}"); break;
			case 1: sprintf(skin, "male/adidas"); break;
			case 2: sprintf(skin, "male/aqgthug"); break;
			case 3: sprintf(skin, "male/aqgvillain"); break;
			case 4: sprintf(skin, "male/austin2"); break;
			case 5: sprintf(skin, "male/babarracuda"); break;
			case 6: sprintf(skin, "male/beastdl"); break;
			case 7: sprintf(skin, "male/bluebeard"); break;
			case 8: sprintf(skin, "male/blues"); break;
			case 9: sprintf(skin, "male/boba"); break;
			case 10: sprintf(skin, "male/borya"); break;
			case 11: sprintf(skin, "male/bravo"); break;
			case 12: sprintf(skin, "male/brucelee"); break;
			case 13: sprintf(skin, "male/bruces"); break;
			case 14: sprintf(skin, "male/chow"); break;
			case 15: sprintf(skin, "male/chowda"); break;
			case 16: sprintf(skin, "male/Clan_UNA"); break;
			case 17: sprintf(skin, "male/conan"); break;
			case 18: sprintf(skin, "male/coolio"); break;
			case 19: sprintf(skin, "male/cop"); break;
			case 20: sprintf(skin, "male/copper"); break;
			case 21: sprintf(skin, "male/ctf_b"); break;
			case 22: sprintf(skin, "male/ctf_g"); break;
			case 23: sprintf(skin, "male/ctf_r"); break;
			case 24: sprintf(skin, "male/ctf_y"); break;
			case 25: sprintf(skin, "male/cyrus"); break;
			case 26: sprintf(skin, "male/dashaft"); break;
			case 27: sprintf(skin, "male/deadpl"); break;
			case 28: sprintf(skin, "male/elway"); break;
			case 29: sprintf(skin, "male/GreenBeret"); break;
			case 30: sprintf(skin, "male/grunt"); break;
			case 31: sprintf(skin, "male/habs"); break;
			case 32: sprintf(skin, "male/hall"); break;
			case 33: sprintf(skin, "male/hellspawn"); break;
			case 34: sprintf(skin, "male/hollywood"); break;
			case 35: sprintf(skin, "male/homer"); break;
			case 36: sprintf(skin, "male/hulk2"); break;
			case 37: sprintf(skin, "male/image"); break;
			case 38: sprintf(skin, "male/indy"); break;
			case 39: sprintf(skin, "male/invince"); break;
			case 40: sprintf(skin, "male/jax-jaguar"); break;
			case 41: sprintf(skin, "male/jdredd"); break;
			case 42: sprintf(skin, "male/jewels"); break;
			case 43: sprintf(skin, "male/jules"); break;
			case 44: sprintf(skin, "male/kgb"); break;
			case 45: sprintf(skin, "male/kw_aqua"); break;
			case 46: sprintf(skin, "male/kw_black"); break;
			case 47: sprintf(skin, "male/kw_blue"); break;
			case 48: sprintf(skin, "male/kw_green"); break;
			case 49: sprintf(skin, "male/kw_orange"); break;
			case 50: sprintf(skin, "male/kw_pink"); break;
			case 51: sprintf(skin, "male/kw_red"); break;
			case 52: sprintf(skin, "male/kw_white"); break;
			case 53: sprintf(skin, "male/kw_yellow"); break;
			case 54: sprintf(skin, "male/leaf"); break;
			case 55: sprintf(skin, "male/leservoircat"); break;
			case 56: sprintf(skin, "male/mariom"); break;
			case 57: sprintf(skin, "male/marsell"); break;
			case 58: sprintf(skin, "male/mason"); break;
			case 59: sprintf(skin, "male/MikeLowry"); break;
			case 60: sprintf(skin, "male/mkrain"); break;
			case 61: sprintf(skin, "male/mkrept"); break;
			case 62: sprintf(skin, "male/mkscorp"); break;
			case 63: sprintf(skin, "male/mksub"); break;
			case 64: sprintf(skin, "male/Mohoney"); break;
			case 65: sprintf(skin, "male/mr_t"); break;
			case 66: sprintf(skin, "male/nut"); break;
			case 67: sprintf(skin, "male/nwom"); break;
			case 68: sprintf(skin, "male/optimus"); break;
			case 69: sprintf(skin, "male/oz"); break;
			case 70: sprintf(skin, "male/pimp"); break;
			case 71: sprintf(skin, "male/police"); break;
			case 72: sprintf(skin, "male/resdog"); break;
			case 73: sprintf(skin, "male/rigs"); break;
			case 74: sprintf(skin, "male/robber"); break;
			case 75: sprintf(skin, "male/roger"); break;
			case 76: sprintf(skin, "male/rredneck"); break;
			case 77: sprintf(skin, "male/sabotage"); break;
			case 78: sprintf(skin, "male/santa"); break;
			case 79: sprintf(skin, "male/sdes"); break;
			case 80: sprintf(skin, "male/shaft"); break;
			case 81: sprintf(skin, "male/siris"); break;
			case 82: sprintf(skin, "male/snowcamo"); break;
			case 83: sprintf(skin, "male/Superman"); break;
			case 84: sprintf(skin, "male/t-800m"); break;
			case 85: sprintf(skin, "male/trmntr"); break;
			case 86: sprintf(skin, "male/walter"); break;
			case 87: sprintf(skin, "male/wcwsting3"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_MALE;
	}
	else if (type == 5) // Messiah
	{
		rnd = rand() % messiah;
		switch (rnd)
		{
			case 0: sprintf(skin, "messiah/axe"); break;
			case 1: sprintf(skin, "messiah/badger"); break;
			case 2: sprintf(skin, "messiah/blackman"); break;
			case 3: sprintf(skin, "messiah/blade"); break;
			case 4: sprintf(skin, "messiah/chow"); break;
			case 5: sprintf(skin, "messiah/chowblue"); break;
			case 6: sprintf(skin, "messiah/chowred"); break;
			case 7: sprintf(skin, "messiah/cptahab"); break;
			case 8: sprintf(skin, "messiah/crawler"); break;
			case 9: sprintf(skin, "messiah/ctf_b"); break;
			case 10: sprintf(skin, "messiah/ctf_r"); break;
			case 11: sprintf(skin, "messiah/darkbeing"); break;
			case 12: sprintf(skin, "messiah/evil_axe"); break;
			case 13: sprintf(skin, "messiah/keaneo"); break;
			case 14: sprintf(skin, "messiah/kindig"); break;
			case 15: sprintf(skin, "messiah/leon"); break;
			case 16: sprintf(skin, "messiah/mxr_neo"); break;
			case 17: sprintf(skin, "messiah/neo"); break;
			case 18: sprintf(skin, "messiah/outlaw"); break;
			case 19: sprintf(skin, "messiah/robber"); break;
			case 20: sprintf(skin, "messiah/scuba"); break;
			case 21: sprintf(skin, "messiah/slug"); break;
			case 22: sprintf(skin, "messiah/spectre"); break;
			case 23: sprintf(skin, "messiah/suit"); break;
			case 24: sprintf(skin, "messiah/TheCrow"); break;
			case 25: sprintf(skin, "messiah/thedon"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_MALE;
	}
	else if (type == 6) // Sas
	{
		rnd = rand() % sas;
		switch (rnd)
		{
			case 0: sprintf(skin, "sas/aqmsas"); break;
			case 1: sprintf(skin, "sas/atf"); break;
			case 2: sprintf(skin, "sas/ctf_b"); break;
			case 3: sprintf(skin, "sas/ctf_r"); break;
			case 4: sprintf(skin, "sas/deathreat"); break;
			case 5: sprintf(skin, "sas/fbi"); break;
			case 6: sprintf(skin, "sas/grassland"); break;
			case 7: sprintf(skin, "sas/grunt"); break;
			case 8: sprintf(skin, "sas/mud"); break;
			case 9: sprintf(skin, "sas/nithb"); break;
			case 10: sprintf(skin, "sas/norse1"); break;
			case 11: sprintf(skin, "sas/norse2"); break;
			case 12: sprintf(skin, "sas/sas-urban"); break;
			case 13: sprintf(skin, "sas/sas-woodland"); break;
			case 14: sprintf(skin, "sas/sas"); break;
			case 15: sprintf(skin, "sas/sas2"); break;
			case 16: sprintf(skin, "sas/sasdc"); break;
			case 17: sprintf(skin, "sas/sasdc2"); break;
			case 18: sprintf(skin, "sas/sasjungle"); break;
			case 19: sprintf(skin, "sas/saspolice"); break;
			case 20: sprintf(skin, "sas/sasuc"); break;
			case 21: sprintf(skin, "sas/sasUC2"); break;
			case 22: sprintf(skin, "sas/sasurban"); break;
			case 23: sprintf(skin, "sas/saswc"); break;
			case 24: sprintf(skin, "sas/STARS"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_MALE;
	}
	else if (type == 7) // Sydney
	{
		rnd = rand() % sydney;
		switch (rnd)
		{
			case 0: sprintf(skin, "sydney/blonde"); break;
			case 1: sprintf(skin, "sydney/sydney"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_FEMALE;
	}
	else if (type == 8) // Terror
	{
		rnd = rand() % terror;
		switch (rnd)
		{
			case 0: sprintf(skin, "terror/adf2"); break;
			case 1: sprintf(skin, "terror/black cop"); break;
			case 2: sprintf(skin, "terror/blue cop"); break;
			case 3: sprintf(skin, "terror/csw black"); break;
			case 4: sprintf(skin, "terror/csw blue"); break;
			case 5: sprintf(skin, "terror/csw crazy"); break;
			case 6: sprintf(skin, "terror/csw red"); break;
			case 7: sprintf(skin, "terror/ctf_b"); break;
			case 8: sprintf(skin, "terror/ctf_r"); break;
			case 9: sprintf(skin, "terror/desertterr"); break;
			case 10: sprintf(skin, "terror/fbiterr"); break;
			case 11: sprintf(skin, "terror/j_mafia"); break;
			case 12: sprintf(skin, "terror/jungleterr"); break;
			case 13: sprintf(skin, "terror/kgb"); break;
			case 14: sprintf(skin, "terror/mafia"); break;
			case 15: sprintf(skin, "terror/mafia2"); break;
			case 16: sprintf(skin, "terror/red cop"); break;
			case 17: sprintf(skin, "terror/redterr2"); break;
			case 18: sprintf(skin, "terror/rmafia"); break;
			case 19: sprintf(skin, "terror/skyterr"); break;
			case 20: sprintf(skin, "terror/stormtrooper"); break;
			case 21: sprintf(skin, "terror/swat"); break;
			case 22: sprintf(skin, "terror/swatsnipe"); break;
			case 23: sprintf(skin, "terror/terror"); break;
			case 24: sprintf(skin, "terror/urbanterr"); break;
		}
		//Com_Printf("%s %s type[%d] rnd[%d]\n", __func__, bot->client->pers.netname, type, rnd);
		return GENDER_MALE;
	}

	return GENDER_MALE;
}

// Set the bot's userinfo (name, skin, hand, etc)
void BOTLIB_SetUserinfo(edict_t* bot, const int team, int force_gender, char* force_name, char* force_skin)
{
	int gender = INVALID;
	char name[MAX_QPATH]; // Full bot name ( [prefix/clan/rng]  and/or  [name]  and/or  [postfix] )
	char skin[MAX_INFO_STRING];
	char userinfo[MAX_INFO_STRING];
	memset(userinfo, 0, sizeof(userinfo)); // Init userinfo

	gi.cvar_forceset(stat_logs->name, "0"); // Turning off stat collection since bots are enabled

	if (force_gender != INVALID) // Use previous gender
	{
		gender = force_gender;
	}
	if (force_skin != NULL) // Use previous skin
	{
		Info_SetValueForKey(userinfo, "skin", force_skin);
	}

	// Teamplay and 3TEAMS
	if (teamplay->value && team) // TEAM1, TEAM2, or TEAM3
	{
		// Figure out the gender based on the team skins
		char* femaleSkinDirs[] = { "actionrally", "female", "sydney" };
		char* maleSkinsDirs[] = { "actionmale", "aqmarine", "male", "messiah", "sas", "terror" };
		for (int i = 0; i < sizeof(femaleSkinDirs) / sizeof(femaleSkinDirs[0]); ++i)
		{
			if (Q_strcasestr(teams[team].skin, femaleSkinDirs[i]) != NULL)
			{
				gender = GENDER_FEMALE;
				break;
			}
		}
		if (gender == INVALID)
		{
			for (int i = 0; i < sizeof(maleSkinsDirs) / sizeof(maleSkinsDirs[0]); ++i)
			{
				if (Q_strcasestr(teams[team].skin, maleSkinsDirs[i]) != NULL)
				{
					gender = GENDER_MALE;
					break;
				}
			}
		}
		if (gender == INVALID) // Couldn't find skin gender (perhaps server is using custom skins)
		{
			if (rand() % 2 == 0) // So just randomize the skin gender
				gender = GENDER_MALE;
			else
				gender = GENDER_FEMALE;
		}
	}
	else // Deathmatch
	{
		if (force_skin == NULL) // Not forcing a skin, so pick one at random
		{
			if (gender == GENDER_MALE || gender == GENDER_FEMALE)
				BOTLIB_RandomSkin(bot, skin, gender); // Set random skin based on predefined gender
			else
				gender = BOTLIB_RandomSkin(bot, skin, INVALID); // Set random skin and return its gender
			Info_SetValueForKey(userinfo, "skin", skin);
		}
	}

	Q_strlcpy(name, "AqtionMan", 10); // Set a default name
	if (force_name == NULL) // Not forcing a name, so pick one at random
	{
		BOTLIB_LoadRandomBotName(bot, name, gender);
		Info_SetValueForKey(userinfo, "name", name);
	}
	else // Use previous name
	{
		Info_SetValueForKey(userinfo, "name", force_name);
	}

	// Set the gender
	if (gender == GENDER_MALE)
		Info_SetValueForKey(userinfo, "gender", "male");
	else if (gender == GENDER_FEMALE)
		Info_SetValueForKey(userinfo, "gender", "female");
	else
		Info_SetValueForKey(userinfo, "gender", "none");

	//Set userinfo: hand, spec
	Info_SetValueForKey(userinfo, "hand", "2"); // bot is center handed for now!
	Info_SetValueForKey(userinfo, "spectator", "0"); // NOT a spectator

	ClientConnect(bot, userinfo);
}

///////////////////////////////////////////////////////////////////////
// Called by PutClient in Server to actually release the bot into the game
// Keep from killin' each other when all spawned at once
///////////////////////////////////////////////////////////////////////
void BOTLIB_HoldSpawn(edict_t* self)
{
	if (!KillBox(self))
	{	// could't spawn in?
	}

	gi.linkentity(self);

	self->think = BOTAI_Think;
	self->nextthink = level.framenum + 1;

	// send effect
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_LOGIN);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	gi.bprintf(PRINT_MEDIUM, "%s entered the game\n", self->client->pers.netname);
}

// Modified version of id's code
void BOTLIB_PutClientInServer(edict_t* bot, qboolean respawn, int team)
{
	bot->is_bot = true;
	bot->bot.bot_type = BOT_TYPE_BOTLIB;
	bot->think = BOTLIB_Think;
	bot->nextthink = level.framenum + 1;

	PutClientInServer(bot);
	JoinTeam(bot, team, true);
}

// Spawn a bot*SelectTeamplaySpawnPoint
void ClientBeginDeathmatch(edict_t* ent);
edict_t* BOTLIB_SpawnBot(int team, int force_gender, char* force_name, char* force_skin) //char* userinfo)
{
	edict_t* bot = BOTLIB_FindFreeEntity();
	if (!bot)
	{
		gi.bprintf(PRINT_MEDIUM, "Server is full! To allow more players set the console command 'maxclients' higher than %d\n", game.maxclients);
		return NULL;
	}

	bot->is_bot = true;
	int base_yaw_speed = 1000;
	int aim_speed = (base_yaw_speed * BOTLIB_SkillMultiplier(bot->bot.skill.aim, true));
	bot->yaw_speed = aim_speed;
	//bot->yaw_speed = 1000;  // aiming degrees per second

	if (team == TEAM1)
		bot_connections.total_team1++;
	else if (team == TEAM2)
		bot_connections.total_team2++;
	else if (team == TEAM3)
		bot_connections.total_team3++;

	if(bot_personality->value && (bot_personality_index == game.used_bot_personalities)) {
		if(bot_debug->value)
			gi.dprintf("%s: Ran out of bot personalities, loading random bots now.\n", __func__);
		DeactivateBotPersonality();
	}
	if (bot_debug->value)
		gi.dprintf("%s: trying to load personalities, loaded: %i used: %i\n", __func__, loaded_bot_personalities, game.used_bot_personalities);
	

	// Loading bot personalities and/or random bots
	if (bot_personality->value == 2) {
		if (rand() % 2) { // Randomly choose between 0 and 1, picking a personality or a random bot
			if (!BOTLIB_SetPersonality(bot, team, force_gender)) {
				if (bot_debug->value)
					gi.dprintf("%s: bot_personality value %f BOTLIB_SetPersonality() failed to load bot personality.\n", __func__, bot_personality->value);
				BOTLIB_SetUserinfo(bot, team, force_gender, force_name, force_skin);
			}
		} else {
			BOTLIB_SetUserinfo(bot, team, force_gender, force_name, force_skin);
		}
	} else if (bot_personality->value == 1) { // We want to prioritize bot personalities, then go random if we run out
		if (bot_personality_index > game.used_bot_personalities) {
			if (!BOTLIB_SetPersonality(bot, team, force_gender)) {
				if (bot_debug->value)
					gi.dprintf("%s: bot_personality value %f BOTLIB_SetPersonality() failed to load bot personality.\n", __func__, bot_personality->value);
				BOTLIB_SetUserinfo(bot, team, force_gender, force_name, force_skin);
			}
		} else {
			if (bot_debug->value)
				gi.dprintf("%s: Ran out of bot personalities, loading random bots now.\n", __func__);
			BOTLIB_SetUserinfo(bot, team, force_gender, force_name, force_skin);
		}
	} else { // Use random bot data, no personalities
		if (bot_debug->value)
			gi.dprintf("%s: trying to load random bots\n", __func__);
		BOTLIB_SetUserinfo(bot, team, force_gender, force_name, force_skin);
	}

	// Need to set this here for ClientBeginDeathmatch
	bot->bot.bot_type = BOT_TYPE_BOTLIB;

	ClientBeginDeathmatch(bot);

	BOTLIB_PutClientInServer(bot, true, team);

	//rekkie -- Fake Bot Client -- s
	// Set the average ping this bot will see
	int rng_ping_range = rand() % 5;
	if (rng_ping_range == 0)
		bot->bot.bot_baseline_ping = (int)(3 + (random() * 33)); // Low ping bastard
	else if (rng_ping_range <= 3)
		bot->bot.bot_baseline_ping = (int)(3 + (random() * 66)); // Average pinger
	else
		bot->bot.bot_baseline_ping = (int)(7 + (random() * 227)); // High ping bastard
	gix->SV_BotConnect(bot->client->pers.netname); // So the server can fake the bot as a 'client'
	game.bot_count++;
	//rekkie -- Fake Bot Client -- e

	return bot;
}


// Remove a bot by name or all bots
void BOTLIB_RemoveBot(char* name)
{
	int i;
	qboolean freed = false;
	edict_t* bot;
	
	// ===================================================
	// Kick by NAME, ALL bots, or by team num
	// ===================================================
	if (strlen(name))
	{
		qboolean remove_all = (Q_stricmp(name, "all") == 0) ? true : false;
		int find_team = (strlen(name) == 1) ? atoi(name) : 0; // Kick by team

		//if (name!=NULL)
		for (i = 0; i < game.maxclients; i++)
		{
			bot = g_edicts + i + 1;
			if (bot->inuse)
			{
				if (bot->is_bot && (remove_all || !strlen(name) || Q_stricmp(bot->client->pers.netname, name) == 0 || (find_team && bot->client->resp.team == find_team)))
				{
					//rekkie -- Fake Bot Client -- s
					gix->SV_BotDisconnect(bot->client->pers.netname); // So the server can remove the fake client
					//rekkie -- Fake Bot Client -- e

					bot->health = 0;
					player_die(bot, bot, bot, 100000, vec3_origin);
					// don't even bother waiting for death frames
					//bot->deadflag = DEAD_DEAD;
					//bot->inuse = false;
					freed = true;
					ClientDisconnect(bot);
					game.bot_count--;

					if (bot_personality->value && bot->bot.personality.isActive)
						BOTLIB_FreeBotPersonality(bot);

					if (!remove_all)
						break;
				}
			}
		}
	}

	// ===================================================
	// Kick random bot
	// ===================================================
	int bot_to_kick = 0;
	int num_bots = 0;
	int bot_count = 0;

	// First, try to find a dead bot to remove
	for (int i = 0; i < game.maxclients; i++) {
		bot = g_edicts + i + 1;

		if (bot->inuse && !IS_ALIVE(bot) && bot->is_bot) {
			num_bots++;
		}
	}

	// If no dead bots were found, find any bot to remove
	if (num_bots == 0) {
		for (int i = 0; i < game.maxclients; i++) {
			bot = g_edicts + i + 1;

			if (bot->inuse && bot->is_bot) {
				num_bots++;
			}
		}

		// If still no bots were found, return
		if (num_bots == 0) {
			return;
		}
	}

	// If more than one bot is found, select a random bot to kick
	if (num_bots > 1) {
		bot_to_kick = rand() % num_bots;
	}

	// Iterate again to find the bot to kick
	for (int i = 0; i < game.maxclients; i++) {
		bot = g_edicts + i + 1;

		if (bot->inuse && bot->is_bot) {
			if (bot_to_kick == bot_count) {
				// Bot Chat - Generates chat message (goodbyes)
				if (bot_chat->value) {
					BOTLIB_Chat(bot, CHAT_GOODBYE);
				}

				// Fake Bot Client - Disconnect the bot
				gix->SV_BotDisconnect(bot->client->pers.netname);

				bot->health = 0;
				player_die(bot, bot, bot, 100000, vec3_origin);
				ClientDisconnect(bot);
				game.bot_count--;

				if (bot_personality->value && bot->bot.personality.isActive) {
					BOTLIB_FreeBotPersonality(bot);
				}

				freed = true;
				break;
			}
			bot_count++;
		}
	}

	// If no bot was removed, print debug message
	if (!freed && debug_mode) {
		gi.bprintf(PRINT_MEDIUM, "No bot removed\n");
	}
}

// Remove a bot by team
// Conditions: Bot must be dead or joined a team during an ongoing round
void BOTLIB_RemoveTeamplayBot(int team)
{
	int i;
	edict_t* bot;

	for (i = 0; i < game.maxclients; i++)
	{
		bot = g_edicts + i + 1;
		if (bot->inuse) // Ent in use
		{
			if (bot->is_bot) // Is a bot
			{
				// Only kick when the bot isn't actively in a match
				//if (bot->client->resp.team == team && (team_round_going == 0 || bot->health <= 0 || bot->solid == SOLID_NOT))
				if (bot->client->resp.team == team) // && team_round_going == 0)
				{
					//if (random() < 0.20) // Randomly kick a bot
					{
						//rekkie -- Fake Bot Client -- s
						gix->SV_BotDisconnect(bot->client->pers.netname); // So the server can remove the fake client
						//rekkie -- Fake Bot Client -- e

						if (team == TEAM1)
							bot_connections.total_team1--;
						else if (team == TEAM2)
							bot_connections.total_team2--;
						else if (team == TEAM3)
							bot_connections.total_team3--;

						game.bot_count--;

						if (bot->health)
							player_die(bot, bot, bot, 100000, vec3_origin);
						ClientDisconnect(bot);
						break;
					}
				}
			}
		}
	}
}

// Change bot [from team] ==> [to team]
// Conditions: Bot must be in a team. Change occurs after round ends.
void BOTLIB_ChangeBotTeam(int from_team, int to_team)
{
	int i;
	edict_t* bot;

	for (i = 0; i < game.maxclients; i++)
	{
		bot = g_edicts + i + 1;
		if (bot->inuse) // Ent in use
		{
			if (bot->is_bot) // Is a bot
			{
				// Only kick when the bot isn't actively in a match or dead
				if ((bot->client->resp.team == from_team && team_round_going == 0) || 
					(bot->client->resp.team == from_team && !IS_ALIVE(bot)))
				{
					if (from_team == TEAM1) bot_connections.total_team1--;
					else if (from_team == TEAM2) bot_connections.total_team2--;
					else if (from_team == TEAM3) bot_connections.total_team3--;

					if (to_team == TEAM1) bot_connections.total_team1++;
					else if (to_team == TEAM2) bot_connections.total_team2++;
					else if (to_team == TEAM3) bot_connections.total_team3++;

					JoinTeam(bot, to_team, true);
					break;
				}
			}
		}
	}
}

//rekkie -- DEV_1 -- s

// Returns how many bots and players are playing (returned via struct).
void BOTLIB_GetTotalPlayers(bot_connections_t* bc)
{
	edict_t* ent;

	bc->total_bots = 0;
	bc->total_humans = 0;
	bc->total_humans_playing = 0;

	bc->total_team1 = 0;
	bc->total_team2 = 0;
	bc->total_team3 = 0;

	bc->spec_bots = 0;
	bc->team1_bots = 0;
	bc->team2_bots = 0;
	bc->team3_bots = 0;

	bc->spec_humans = 0;
	bc->team1_humans = 0;
	bc->team2_humans = 0;
	bc->team3_humans = 0;

	for (int i = 0; i < game.maxclients; i++)
	{
		ent = &g_edicts[1 + i];
		if (ent && ent->inuse)
		{
			if (ent->is_bot)
			{
				bc->total_bots++;

				if (teamplay->value)
				{
					if (ent->client->resp.team == NOTEAM)
					{
						bc->spec_bots++;
					}
					else if (ent->client->resp.team == TEAM1)
					{
						bc->team1_bots++;
						bc->total_team1++;
					}
					else if (ent->client->resp.team == TEAM2)
					{
						bc->team2_bots++;
						bc->total_team2++;
					}
					else if (ent->client->resp.team == TEAM3)
					{
						bc->team3_bots++;
						bc->total_team3++;
					}
				}
				else if (deathmatch->value)
				{
					if (ent->solid == SOLID_NOT)
						bc->spec_bots++;
					else
						bc->team1_bots++;
				}
			}
			else
			{
				bc->total_humans++;

				if (teamplay->value)
				{
					if (ent->client->resp.team == NOTEAM)
						bc->spec_humans++;
					else if (ent->client->resp.team == TEAM1)
					{
						bc->team1_humans++;
						bc->total_team1++;
					}
					else if (ent->client->resp.team == TEAM2)
					{
						bc->team2_humans++;
						bc->total_team2++;
					}
					else if (ent->client->resp.team == TEAM3)
					{
						bc->team3_humans++;
						bc->total_team3++;
					}
				}
				else if (deathmatch->value)
				{
					if (ent->solid == SOLID_NOT)
						bc->spec_humans++;
					else
						bc->team1_humans++;
				}
			}
		}
	}
	bc->total_humans_playing = bc->total_humans - bc->spec_humans;
}

static void BOTLIB_TeamBotShuffle(void)
{
	if (use_3teams->value && (bot_connections.total_team1 != bot_connections.total_team2 || bot_connections.total_team1 != bot_connections.total_team3 || bot_connections.total_team2 != bot_connections.total_team3))
	{
		if (abs(bot_connections.total_team1 - bot_connections.total_team2) > 1 && bot_connections.total_team1 < bot_connections.total_team2)
			BOTLIB_ChangeBotTeam(TEAM2, TEAM1);
		if (abs(bot_connections.total_team1 - bot_connections.total_team3) > 1 && bot_connections.total_team1 < bot_connections.total_team3)
			BOTLIB_ChangeBotTeam(TEAM3, TEAM1);

		if (abs(bot_connections.total_team2 - bot_connections.total_team1) > 1 && bot_connections.total_team2 < bot_connections.total_team1)
			BOTLIB_ChangeBotTeam(TEAM1, TEAM2);
		if (abs(bot_connections.total_team2 - bot_connections.total_team3) > 1 && bot_connections.total_team2 < bot_connections.total_team3)
			BOTLIB_ChangeBotTeam(TEAM3, TEAM2);

		if (abs(bot_connections.total_team3 - bot_connections.total_team1) > 1 && bot_connections.total_team3 < bot_connections.total_team1)
			BOTLIB_ChangeBotTeam(TEAM1, TEAM3);
		if (abs(bot_connections.total_team3 - bot_connections.total_team2) > 1 && bot_connections.total_team3 < bot_connections.total_team2)
			BOTLIB_ChangeBotTeam(TEAM2, TEAM3);
	}
	else if (bot_connections.total_team1 != bot_connections.total_team2)
	{
		if (abs(bot_connections.total_team1 - bot_connections.total_team2) > 1)
		{
			if (bot_connections.total_team1 < bot_connections.total_team2)
				BOTLIB_ChangeBotTeam(TEAM2, TEAM1);
			else
				BOTLIB_ChangeBotTeam(TEAM1, TEAM2);
		}
	}
}

// Bots auto join teams to keep them equal in teamplay. 
// Empty servers have a bot join a team upon a player connecting.
int bot_teamcheckfrequency = 0;
int bot_teamchangefrequency = 0;
void BOTLIB_CheckBotRules(void)
{
	if (matchmode->value) // Bots never allowed in matchmode
		return;

	if (ctf->value)
	{
		BOTLIB_Update_Flags_Status();
	}

	// check these rules every 1.0 seconds... (aka 10 frames) -- this will be faster if server hz is faster
	if ((++bot_teamcheckfrequency % 10) != 0)
	{
		return;
	}
	else
	{
		bot_teamcheckfrequency = 0;
		//Com_Printf("%s ltk_teamcheckfrequency\n", __func__);
	}

	BOTLIB_GetTotalPlayers(&bot_connections);

	// All bots are counted as being on team 1 if in DM mode

	//gi.dprintf("%s hs[%d] ht1[%d] ht2[%d] ht3[%d] bs[%d] bt1[%d] bt2[%d] bt3[%d] tb[%d] th[%d]\n", __func__, bot_connections.spec_humans, bot_connections.team1_humans, bot_connections.team2_humans, bot_connections.team3_humans, bot_connections.spec_bots, bot_connections.team1_bots, bot_connections.team2_bots, bot_connections.team3_bots, bot_connections.total_bots, bot_connections.total_humans);

	if (bot_connections.tried_adding_prev_bots == false)
	{
		bot_connections.tried_adding_prev_bots = true; // Only called once per map
		BOTLIB_AddBotsFromPreviousMap(100); // Try get bots from a previous map
		BOTLIB_GetTotalPlayers(&bot_connections); // Update connection stats
	}

	// Turn on team balance if bot_maxteam is used
	if (bot_maxteam->value > 0) bot_connections.auto_balance_bots = true;

	// ========================================================================================================
	// Manually add bots to a select team
	// sv bots <num> <team>
	if (teamplay->value && bot_connections.auto_balance_bots == false)
	{
		// Sanity check
		if (bot_connections.desire_team1 < 0) bot_connections.desire_team1 = 0;
		if (bot_connections.desire_team2 < 0) bot_connections.desire_team2 = 0;
		if (bot_connections.desire_team3 < 0) bot_connections.desire_team3 = 0;

		// Remove bots
		if (bot_connections.desire_team1 < bot_connections.team1_bots)
		{
			while (bot_connections.team1_bots != bot_connections.desire_team1)
			{
				BOTLIB_RemoveTeamplayBot(TEAM1);
				bot_connections.team1_bots--;
			}
		}
		if (bot_connections.desire_team2 < bot_connections.team2_bots)
		{
			while (bot_connections.team2_bots != bot_connections.desire_team2)
			{
				BOTLIB_RemoveTeamplayBot(TEAM2);
				bot_connections.team2_bots--;
			}
		}
		if (bot_connections.desire_team3 < bot_connections.team3_bots)
		{
			while (bot_connections.team3_bots != bot_connections.desire_team3)
			{
				BOTLIB_RemoveTeamplayBot(TEAM3);
				bot_connections.team3_bots--;
			}
		}

		// Add bots
		if (bot_connections.desire_team1 > bot_connections.team1_bots)
		{
			while (bot_connections.team1_bots != bot_connections.desire_team1)
			{
				// If adding bots goes over maxclients, limit what can be added.
				if (bot_connections.total_bots + bot_connections.total_humans + 1 > maxclients->value)
				{
					bot_connections.desire_team1 = bot_connections.team1_bots;
					break;
				}
				BOTLIB_SpawnBot(TEAM1, INVALID, NULL, NULL);
				BOTLIB_GetTotalPlayers(&bot_connections); // Update connection stats
			}
		}
		if (bot_connections.desire_team2 > bot_connections.team2_bots)
		{
			while (bot_connections.team2_bots != bot_connections.desire_team2)
			{
				// If adding bots goes over maxclients, limit what can be added.
				if (bot_connections.total_bots + bot_connections.total_humans + 1 > maxclients->value)
				{
					bot_connections.desire_team2 = bot_connections.team2_bots;
					break;
				}
				BOTLIB_SpawnBot(TEAM2, INVALID, NULL, NULL);
				BOTLIB_GetTotalPlayers(&bot_connections); // Update connection stats
			}
		}
		if (bot_connections.desire_team3 > bot_connections.team3_bots)
		{
			while (bot_connections.team3_bots != bot_connections.desire_team3)
			{
				// If adding bots goes over maxclients, limit what can be added.
				if (bot_connections.total_bots + bot_connections.total_humans + 1 > maxclients->value)
				{
					bot_connections.desire_team3 = bot_connections.team3_bots;
					break;
				}
				BOTLIB_SpawnBot(TEAM3, INVALID, NULL, NULL);
				BOTLIB_GetTotalPlayers(&bot_connections); // Update connection stats
			}
		}
	}
	// ========================================================================================================


	// Manage bot counts in DM mode
	int bots_to_spawn = 0;
	if (bot_playercount->value > 0 && (gameSettings & GS_DEATHMATCH)) {
		// Calculate the desired number of bots based on bot_playercount and total_humans
		bot_connections.desire_bots = (int)bot_playercount->value - bot_connections.total_humans;

		// Ensure desire_bots does not exceed maxclients - total_humans
		if (bot_connections.desire_bots + bot_connections.total_humans > maxclients->value) {
			bot_connections.desire_bots = (int)(maxclients->value - bot_connections.total_humans);
		}

		// Sanity check - safety limits
		if (bot_connections.desire_bots < 0) bot_connections.desire_bots = 0;
		int bots_adj = 0;
		int total_players = bot_connections.total_bots + bot_connections.total_humans;

		if (total_players > bot_playercount->value) {
			//gi.dprintf("I should remove a bot\n");
			bots_adj = -1;
		} else if (total_players < bot_playercount->value) {
			// gi.dprintf("I should add a bot\n");
			// gi.dprintf("total_players: %d, bot_playercount->value: %d\n", total_players, bot_playercount->value);
			bots_adj = 1;
		} else {
			bots_to_spawn = (int)bot_playercount->value;
			bot_connections.desire_bots = bots_to_spawn;
		}

		bots_to_spawn = bots_adj; // Set bots_to_spawn to +1 or -1 based on the adjustment needed
		//gi.dprintf("bots_adj: %d\n", bots_to_spawn);
	} else {
		bots_to_spawn = bot_connections.desire_bots - bot_connections.total_bots;
	}

	// // Auto balance bots
	// if (bot_connections.auto_balance_bots == false)
	// 	return;


	//bot_connections.desire_bots += (bot_connections.desire_team1 + bot_connections.desire_team2 + bot_connections.desire_team3);

	// Percent chance to add/remove bots every 5 seconds...  inc/dec = [min: 1, max: bot_maxteam]
	if ((++bot_teamchangefrequency % 5) == 0)
	{
		bot_teamchangefrequency = 0;

		if (bot_maxteam->value)
		{
			//scale_up
			float percent = (float)bot_connections.total_bots / bot_maxteam->value;
			if (bot_connections.scale_dn == false && bot_connections.scale_up == false)
				bot_connections.scale_up = true;

			float min_percent = 1 / bot_maxteam->value;

			// Reached top end of the scale, so work back down
			if (bot_connections.scale_up && percent >= 1.0)
			{
				bot_connections.scale_up = false;
				bot_connections.scale_dn = true;
			}
			// Reached bottom end of the scale, or randomly at 50%, so work back up
			else if (bot_connections.scale_dn && (percent <= min_percent || random() < 0.05)) // Never drop below this percent
			{
				bot_connections.scale_up = true;
				bot_connections.scale_dn = false;
			}

			// Decrease bots
			if (bot_connections.scale_dn && random() < 0.050)
				bot_connections.desire_bots--;

			// Increase bots
			if (bot_connections.scale_up && random() < 0.250)
				bot_connections.desire_bots++;

			// Don't let bots be greater than bot_maxteam
			if ((float)bot_connections.desire_bots > bot_maxteam->value)
				bot_connections.desire_bots--;

			// Never go below 1 bot when bot_maxteam is active
			if (bot_connections.desire_bots < 1)
				bot_connections.desire_bots = 1;
		}
	}

	// Sanity check - safety limits
	//if (bot_connections.desire_bots < 0) bot_connections.desire_bots = 0;

	//int bots_to_spawn = abs(bot_connections.total_bots - bot_connections.desire_bots);

	// Shuffle bots around
	if (teamplay->value && bot_connections.auto_balance_bots)
		BOTLIB_TeamBotShuffle();

	// Remove ALL bots
	if (bot_connections.total_bots > 0 && bot_connections.desire_bots == 0)
	{
		BOTLIB_RemoveBot("ALL");
		bot_connections.total_team1 = 0;
		bot_connections.total_team2 = 0;
		bot_connections.total_team3 = 0;
	}

	// Time to add or remove bots
	BOTLIB_BotCountManager(bots_to_spawn);
}
//rekkie -- DEV_1 -- e