#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"
#include "../m_player.h" // For waving frame types, i.e: FRAME_flip01

// Bot chat
int last_bot_chat_time;  // So the chat isn't super spammy with lots of bots

//
// Delayed chat, somewhat more realistic
#define MAX_MESSAGES 100

typedef struct {
	edict_t* bot;
    char text[256];
    int frameReceived;
} ChatMessage;

typedef struct {
    ChatMessage messages[MAX_MESSAGES];
    int count;
} MessageQueue;

MessageQueue chatQueue = { .count = 0 };

void AddMessageToChatQueue(edict_t* bot, const char* text, int frameReceived) {
    if (chatQueue.count < MAX_MESSAGES) {
        chatQueue.messages[chatQueue.count].bot = bot;
        strncpy(chatQueue.messages[chatQueue.count].text, text, sizeof(chatQueue.messages[chatQueue.count].text) - 1);
        chatQueue.messages[chatQueue.count].frameReceived = frameReceived;
        chatQueue.count++;
    } else {
        gi.dprintf("Chat queue is full, message dropped\n");
    }
}

void ProcessChatQueue(int currentFrame) {
    for (int i = 0; i < chatQueue.count; i++) {
        if (currentFrame > chatQueue.messages[i].frameReceived + (rand() % HZ) + 4 * HZ) {
            // Send the chat message from the correct bot
            BOTLIB_Say(chatQueue.messages[i].bot, chatQueue.messages[i].text, false);
            //gi.dprintf("Sending delayed chat message from bot: %s\n", chatQueue.messages[i].text);

            // Remove the message from the queue
            for (int j = i; j < chatQueue.count - 1; j++) {
                chatQueue.messages[j] = chatQueue.messages[j + 1];
            }
            chatQueue.count--;
            i--; // Adjust index to account for removed message
        }
    }
}

// Call this function periodically, e.g., in the main game loop
void UpdateBotChat(void) {
    ProcessChatQueue(level.framenum);
}

// Borrowed from LTK bots
#define DBC_WELCOMES 4
char *botchat_welcomes[DBC_WELCOMES] =
{
    "Hey %s, how's it going?",
	"hey %s whats up",
	"yo %s",
	"aw yeah %s is here"
};

#define DBC_GOODBYES 5
char *botchat_goodbyes[DBC_GOODBYES] =
{
    "welp gotta go",
	"food time brb",
	"ggs all",
	"time for a j",
	"sauna time"
};

#define DBC_KILLEDS 20
char *botchat_killeds[DBC_KILLEDS] =
{
	"lol",
	"welp..",
	"aw come on",
	"nice one",
	"prkl",
	"jajajaja",
	"ffffffffff",
	"woowwwww",
	"ran outta ammo",
	"ahhhhhhhh",
	"damn",
	"heh",
	"rip",
	":D",
	":)))",
	">:)",
	"noooooo",
	":<",
	"oops",
	"rofl",
};

#define DBC_INSULTS 11
char *botchat_insults[DBC_INSULTS] =
{
	"lol gottem",
	"pow!",
	"rip",
	"can't hide from me",
	"goteem",
	"he he heee",
	":D",
	"heh",
	"AHAHAHAHA",
	">:)",
	":>"
};

#define DBC_VICTORY 3
char *botchat_victory[DBC_VICTORY] =
{
	"smoked!",
	"whew",
	"too ez"
};

#define DBC_RAGE 9
char *botchat_rage[DBC_RAGE] =
{
	"f this map",
	"i never liked this map anyway gg",
	"nope not today",
	"I'm terrible at this map anyway",
	"yeah gg",
	"that's enough for me today",
	"ASDKDDKJFJK",
	"PERKELEEeeeee",
	"FYFAAAAN"
};

// This function is used to mix up chat so that multiple
// phrases can be combined to create a more unique message
// Currently only doing this for CHAT_KILLED
char* _chatMix(char* msg, bot_chat_types_t chattype, int randval)
{
    char* text = NULL;
	char buffer[1024];
    buffer[0] = '\0';
    int newrandval = 0;
    switch (chattype) {
        case CHAT_KILLED:
            do {
                newrandval = rand() % DBC_KILLEDS;
            } while (newrandval == randval);
            text = botchat_killeds[newrandval];
            break;
        default:
            if (debug_mode)
                gi.bprintf(PRINT_HIGH, "%s: Unknown chat type %d", __func__, chattype);
            return msg; // Just return what was given to us
    }

    // Randomize whether to append or prepend
    if (rand() % 2 == 0) {
        // Prepend
        snprintf(buffer, sizeof(buffer), "%s %s", text, msg);
    } else {
        // Append
        snprintf(buffer, sizeof(buffer), "%s %s", msg, text);
    }

    // Return a dynamically allocated copy of the result
    return strdup(buffer);
}

void BOTLIB_Chat(edict_t* bot, bot_chat_types_t chattype)
{
	// Do nothing if bot_chat is disabled
	if(!bot_chat->value)
		return;

	char* text = NULL;
	qboolean delayed = true;
	// bot_is_target means the bot edict provided to this function is not the talker, but the bot the other
	// bots will talk to, such as 'Welcome <so and so>!'.  Bot chosen to speak will be random from the list
	// of current/inuse bots

	int killedsrandval = 0;
	switch (chattype) {
		//case CHAT_WELCOME:
			// CHAT_WELCOME is handled below outside of this switch statement
		case CHAT_KILLED:
			killedsrandval = rand() % DBC_KILLEDS;
			text = botchat_killeds[killedsrandval];
			break;
		case CHAT_INSULTS:
			text = botchat_insults[rand() % DBC_INSULTS];
			break;
		case CHAT_GOODBYE:
			text = botchat_goodbyes[rand() % DBC_GOODBYES];
			break;
		case CHAT_RAGE:
			text = botchat_rage[rand() % DBC_RAGE];
			break;
		default:
			if (debug_mode)
				gi.bprintf(PRINT_HIGH, "%s: Unknown chat type %d", __func__, chattype);
			return; // Exit if chattype is unknown
	}

	// Ensure text is not NULL before proceeding
	if (text == NULL) {
		if (debug_mode)
			gi.bprintf(PRINT_HIGH, "%s: text is NULL, cannot proceed with BOTLIB_Say.", __func__);
		return; // Optionally, set text to a default value before calling BOTLIB_Say
	}

    // Define the chat interval (e.g., once per minute)
    int chatInterval = 60 * HZ; // 60 seconds * HZ (game ticks per second)
	float randval = random();

    //Check if the bot should chat
	// Only applies to insults and killed messages
	if (chattype == CHAT_INSULTS || chattype == CHAT_KILLED) {
		if (bot->bot.lastChatTime > level.framenum - chatInterval) {
			//gi.dprintf("Skipping chat due to interval limit (%i) needs to be 0 or smaller\n", (bot->bot.lastChatTime - (level.framenum - chatInterval)));
			return;
		} else if (chattype == CHAT_KILLED) { // Mix up the killed messages
			if (rand() % 2 == 0) {
				text = _chatMix(text, chattype, randval);
			}
		}
	}
	// Goodbyes and rages happen without delay
	if (chattype == CHAT_GOODBYE || chattype == CHAT_RAGE) {
		randval = randval - 0.3; // Increase the chance of this type of message
		delayed = false;
	}

	// In this one, the bot edict is not the talker, but the 'target' of the talker
	// Then we switch the bot after we get the target's name, since the talker
	// will be random
	if (chattype == CHAT_WELCOME) {
		int index = rand() % DBC_WELCOMES; // Choose a random message index
		char message[256]; // Assuming 256 bytes is enough for the message
		snprintf(message, sizeof(message), botchat_welcomes[index], bot->client->pers.netname);
		
		// Ensure 'text' buffer is large enough
		char text[256]; // Increase the size of 'text' to match 'message'
		
		// Copy back to 'text'
		snprintf(text, sizeof(text), "%s", message);
		
		if (BOTLIB_GetRandomBot(0, false) != NULL)
			bot = BOTLIB_GetRandomBot(0, false);
	}

	// bot_personality
	// This will let the bot chat based on their propensity to do so
	if (bot_personality->value) {
		if (!BOTLIB_DoIChat(bot)){
			return;
		}
	} else if (randval > 0.2) { // 80% do not chat
		//gi.dprintf("Skipping chat due to random chance (%f)\n", randval);
		return; // Don't chat too often
    } else if (level.framenum - last_bot_chat_time < 5 * HZ) { // Check if the last chat was within the last so many frames
		return;  // Slow down the spam!
	}

    // Add the message to the queue if delayed
	if (delayed) {
    	AddMessageToChatQueue(bot, text, level.framenum);
	} else { // instant message, such as a goodbye before leaving
		BOTLIB_Say(bot, text, false);
		last_bot_chat_time = level.framenum;
	}

    // Sets the current level time as the last chat time so the bot doesn't spam chat
    bot->bot.lastChatTime = level.framenum;

    //gi.dprintf("new LastChatTime: %d\n", bot->bot.lastChatTime);

}

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

void BOTLIB_PrecacheRadioSounds(void)
{
	int i;
	char path[MAX_QPATH];

	//male
	for (i = 0; i < bot_numMaleSnds; i++)
	{
		Q_snprintf(path, sizeof(path), "%s%s.wav", "radio/male/", bot_male_radio_msgs[i].msg);
		bot_male_radio_msgs[i].sndIndex = gi.soundindex(path);
	}

	//female
	for (i = 0; i < bot_numFemaleSnds; i++)
	{
		Q_snprintf(path, sizeof(path), "%s%s.wav", "radio/female/", bot_female_radio_msgs[i].msg);
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

	// Dead shit doesn't broadcast
	if (!IS_ALIVE(ent))
		return;

	// Check if the bot should communicate based on skill level
	if (!BOTLIB_SkillChance((ent->bot.skill.communication + ent->bot.skill.teamwork)))
		return;

	if (!teamplay->value)
	{
		if (!DMFLAGS((DF_MODELTEAMS | DF_SKINTEAMS)))
			return;			// don't allow in a non-team setup...
	}

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


	if (teamplay->value) {

		if (ent->client->resp.team == NOTEAM)
			return;

		if (ent->solid == SOLID_NOT || ent->deadflag == DEAD_DEAD)
		{
			if (team_message)
				Q_snprintf(text, sizeof(text), "[DEAD] (%s): ", ent->client->pers.netname); // Dead, say team
			else
				Q_snprintf(text, sizeof(text), "[DEAD] %s: ", ent->client->pers.netname); // Dead, say all
		}
		else if (team_message)
			Q_snprintf(text, sizeof(text), "(%s): ", ent->client->pers.netname); // Alive, say team
		else
			Q_snprintf(text, sizeof(text), "%s: ", ent->client->pers.netname); // Alive, say all
	} else { // non-teamplay
		if (bot_chat->value) {
			Q_snprintf(text, sizeof(text), "%s: ", ent->client->pers.netname);
		}
	}

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
		sprintf(buf, " %c [ %d ] %dx Enemies Down! [ %c ", '\x06', self->client->resp.streakKills, kills, '\x07'); // Build bandage string
	else
		sprintf(buf, " %c [ %d ] Enemy Down! [ %c ", '\x06', self->client->resp.streakKills, '\x07'); // Build bandage string

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
					char weap_name[WEAP_ITM_NAME_LEN];
					char item_name[WEAP_ITM_NAME_LEN];
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
		if (IS_ALIVE(self) && self->health < 100)
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
