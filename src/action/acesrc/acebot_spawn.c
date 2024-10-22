///////////////////////////////////////////////////////////////////////
//
//  ACE - Quake II Bot Base Code
//
//  Version 1.0
//
//  Original file is Copyright(c), Steve Yeager 1998, All Rights Reserved
//
//
//	All other files are Copyright(c) Id Software, Inc.
////////////////////////////////////////////////////////////////////////
/*
 * $Header: /LTK2/src/acesrc/acebot_spawn.c 6     24/02/00 20:05 Riever $
 *
 * $Log: /LTK2/src/acesrc/acebot_spawn.c $
 * 
 * 6     24/02/00 20:05 Riever
 * Team Radio use fixed up. (Using ACE to test)
 * 
 * Manual addition: Radio 'treport' support added.
 * User: Riever       Date: 21/02/00   Time: 23:42
 * Changed initial spawn state to BS_ROAM
 * User: Riever       Date: 21/02/00   Time: 15:16
 * Bot now has the ability to roam on dry land. Basic collision functions
 * written. Active state skeletal implementation.
 * User: Riever       Date: 20/02/00   Time: 20:27
 * Added new members and definitions ready for 2nd generation of bots.
 * User: Riever       Date: 17/02/00   Time: 17:10
 * Radiogender now set for bots.
 * 
 */

///////////////////////////////////////////////////////////////////////
//
//  acebot_spawn.c - This file contains all of the 
//                   spawing support routines for the ACE bot.
//
///////////////////////////////////////////////////////////////////////

// Define this for an ACE bot
#define ACE_SPAWN
// ----

#include "../g_local.h"
#include "../m_player.h"

#include "acebot.h"
#include "botchat.h"
#include "botscan.h"

//AQ2 ADD
#define	CONFIG_FILE_VERSION 1

void	ClientBeginDeathmatch( edict_t *ent );
void	AllItems( edict_t *ent );
void	AllWeapons( edict_t *ent );
void	EquipClient( edict_t *ent );
char	*TeamName(int team);
void	LTKsetBotName( char	*bot_name );
void	ACEAI_Cmd_Choose( edict_t *ent, char *s);

//==========================================
// Joining a team (hacked from AQ2 code)
//==========================================
/*	defined in a_team.h
#define NOTEAM          0
#define TEAM1           1
#define TEAM2           2
*/

//==============================
// Get the number of the next team a bot should join
//==============================
int GetNextTeamNumber(void)
{
        int i, onteam1 = 0, onteam2 = 0, onteam3 = 0;
        edict_t *e;

        // only use this function during [2]team games...
        if (!teamplay->value )
                return 0;

//		gi.bprintf(PRINT_HIGH, "Checking team balance..\n");
        for (i = 1; i <= game.maxclients; i++)
        {
                e = g_edicts + i;
                if (e->inuse)
                {
                    if (e->client->resp.team == TEAM1)
                        onteam1++;
                    else if (e->client->resp.team == TEAM2)
                        onteam2++;
                    else if (e->client->resp.team == TEAM3)
                        onteam3++;
                }
        }

	// Return the team number that needs the next bot
        if (use_3teams->value && (onteam3 < onteam1) && (onteam3 < onteam2))
                return (3);
        else if (onteam2 < onteam1)
                return (2);
        //default
        return (1);
}

//==========================
// Join a Team
//==========================
void ACESP_JoinTeam(edict_t *ent, int desired_team)
{
	JoinTeam( ent, desired_team, true );
	CheckForUnevenTeams( ent );
}

//======================================
// ACESP_LoadBotConfig()
//======================================
// Using RiEvEr's new config file
//
void ACESP_LoadBotConfig(void)
{
    FILE	*pIn;
	cvar_t	*game_dir = NULL, *botdir = NULL;
#ifdef _WIN32
	int		i;
#endif
	char	filename[60];
	// Scanner stuff
	int		fileVersion = 0;
	char	inString[81];
	char	tokenString[81];
	char	*sp, *tp;
	int		ttype;

	game_dir = gi.cvar ("game", "action", 0);
	botdir = gi.cvar ("botdir", "bots", 0);

if (ltk_loadbots->value){

	// Turning off stat collection since bots are enabled
	gi.cvar_forceset(stat_logs->name, "0");
	// Try to load the file for THIS level
	#ifdef	_WIN32
		i =  sprintf(filename, ".\\");
		i += sprintf(filename + i, game_dir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, botdir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, level.mapname);
		i += sprintf(filename + i, ".cfg");
	#else
		strcpy(filename, "./");
		strcat(filename, game_dir->string);
		strcat(filename, "/");
		strcat(filename, botdir->string);
		strcat(filename, "/");
		strcat(filename, level.mapname);
		strcat(filename, ".cfg");
	#endif

		// If there's no specific file for this level, then
		// load the file name from value ltk_botfile (default is botdata.cfg)
		if((pIn = fopen(filename, "rb" )) == NULL)
		{
	#ifdef	_WIN32
			i =  sprintf(filename, ".\\");
			i += sprintf(filename + i, game_dir->string);
			i += sprintf(filename + i, "\\");
			i += sprintf(filename + i, botdir->string);
			i += sprintf(filename + i, "\\");
			i += sprintf(filename + i, ltk_botfile->string);
	#else
			strcpy(filename, "./");
			strcat(filename, game_dir->string);
			strcat(filename, "/");
			strcat(filename, botdir->string);
			strcat(filename, "/");
			strcat(filename, ltk_botfile->string);
	#endif

			// No bot file available, get out of here!
			if((pIn = fopen(filename, "rb" )) == NULL) {
				gi.dprintf("WARNING: No file containing bot data was found, no bots loaded.\n");
				gi.dprintf("ltk_botfile value is %s\n", ltk_botfile->string);
				return; // bail
			}
		}

		// Now scan each line for information
		// First line should be the file version number
		fgets( inString, 80, pIn );
		sp = inString;
		tp = tokenString;
		ttype = UNDEF;

		// Scan it for the version number
		scanner( &sp, tp, &ttype );
		if(ttype == BANG)
		{
			scanner( &sp, tp, &ttype );
			if(ttype == INTLIT)
			{
				fileVersion = atoi( tokenString );
			}
			if( fileVersion != CONFIG_FILE_VERSION )
			{
				// ERROR!
				gi.bprintf(PRINT_HIGH, "Bot Config file is out of date!\n");
				fclose(pIn);
				return;
			}
		}

		// Now process each line of the config file
		while( fgets(inString, 80, pIn) )
		{
			ACESP_SpawnBotFromConfig( inString );
		}

			

	/*	fread(&count,sizeof (int),1,pIn); 

		for(i=0;i<count;i++)
		{
			fread(userinfo,sizeof(char) * MAX_INFO_STRING,1,pIn); 
			ACESP_SpawnBot (NULL, NULL, NULL, userinfo);
		}*/
			
		fclose(pIn);
	}
}

//===========================
// ACESP_SpawnBotFromConfig
//===========================
// Input string is from the config file and should have
// all the data we need to spawn a bot
//
edict_t *ACESP_SpawnBotFromConfig( char *inString )
{
	edict_t	*bot;
	char	userinfo[MAX_INFO_STRING];
	int		count=1;
	char	name[32];
	char	modelskin[80];
	int		team=0, weaponchoice=0, equipchoice=0;
	char	gender[2] = "m";
	char	team_str[2] = "0";

	// Scanner stuff
	char	tokenString[81];
	char	*sp, *tp;
	int		ttype;

	sp = inString;
	ttype = UNDEF;

	while( ttype != EOL )
	{
		tp = tokenString;
		scanner(&sp, tp, &ttype);

		// Check for comments
		if( ttype == HASH || ttype == LEXERR)
			return NULL;

		// Check for semicolon (end of input marker)
		if( ttype == SEMIC || ttype == EOL)
			continue;

		// Keep track of which parameter we are reading
		if( ttype == COMMA )
		{
			count++;
			continue;
		}

		// NAME (parameter 1)
		if(count == 1 && ttype == STRLIT)
		{
//			strncpy( name, tokenString, 32 );
			strcpy( name, tokenString);
			continue;
		}

		// MODELSKIN (parameter 2)
		if(count == 2 && ttype == STRLIT)
		{
//			strncpy( modelskin, tokenString, 32 );
			strcpy( modelskin, tokenString);
			continue;
		}

		if(count == 3 && ttype == INTLIT )
		{
			team = atoi(tokenString);
			continue;
		}

		if(count == 4 && ttype == INTLIT )
		{
			weaponchoice = atoi(tokenString);
			continue;
		}

		if(count == 5 && ttype == INTLIT )
		{
			equipchoice = atoi(tokenString);
			continue;
		}

		if(count == 6 && ttype == SYMBOL )
		{
			gender[0] = tokenString[0];
			continue;
		}

	}// End while
	
	// To allow bots to respawn
	// initialise userinfo
	memset( userinfo, 0, sizeof(userinfo) );
	
	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey( userinfo, "name", name );
	Info_SetValueForKey( userinfo, "skin", modelskin );
	Info_SetValueForKey( userinfo, "hand", "2" ); // bot is center handed for now!
	Info_SetValueForKey( userinfo, "spectator", "0" ); // NOT a spectator
	Info_SetValueForKey( userinfo, "gender", gender );
	
	Q_snprintf( team_str, 2, "%i", team );
	
	bot = ACESP_SpawnBot( team_str, name, modelskin, userinfo );
	
	// FIXME: This might have to happen earlier to take effect.
	if( bot )
	{
		bot->weaponchoice = weaponchoice;
		bot->equipchoice = equipchoice;
	}
	
	return bot;
}
//AQ2 END


///////////////////////////////////////////////////////////////////////
// Called by PutClient in Server to actually release the bot into the game
// Keep from killin' each other when all spawned at once
///////////////////////////////////////////////////////////////////////
void ACESP_HoldSpawn(edict_t *self)
{
	if (!KillBox (self))
	{	// could't spawn in?
	}

	gi.linkentity (self);

#ifdef	ACE_SPAWN
	// Use ACE bots
	self->think = ACEAI_Think;
#else
	self->think = BOTAI_Think;
#endif

	self->nextthink = level.framenum + 1;

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

/*	if(ctf->value)
	gi.bprintf(PRINT_MEDIUM, "%s joined the %s team.\n",
		self->client->pers.netname, CTFTeamName(self->client->resp.ctf_team));
	else*/
		gi.bprintf (PRINT_MEDIUM, "%s entered the game\n", self->client->pers.netname);

}

///////////////////////////////////////////////////////////////////////
// Modified version of id's code
///////////////////////////////////////////////////////////////////////
void ACESP_PutClientInServer( edict_t *bot, qboolean respawn, int team )
{
	bot->is_bot = true;
	
	// Use 'think' to pass the value of respawn to PutClientInServer.
	if( ! respawn )
	{
		bot->think = ACESP_HoldSpawn;
		bot->nextthink = level.framenum + random() * 3 * HZ;
	}
	else
	{
		#ifdef ACE_SPAWN
			bot->think = ACEAI_Think;
		#else
			bot->think = BOTAI_Think;
		#endif
		bot->nextthink = level.framenum + 1;
	}
	
	PutClientInServer( bot );

	
	
	JoinTeam( bot, team, true );
}

///////////////////////////////////////////////////////////////////////
// Respawn the bot
///////////////////////////////////////////////////////////////////////
void ACESP_Respawn (edict_t *self)
{
	respawn( self );
	
	if( random() < 0.15)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		if( self->lastkilledby)
		{
			// Have a comeback line!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( self, self->lastkilledby, DBC_KILLED);
			self->lastkilledby = NULL;
		}
		else
		{
			// Pick someone at random to insult
			for(i=0;i<=num_players;i++)
			{
				// Find all available enemies to insult
				if(players[i] == NULL || players[i] == self || 
				   players[i]->solid == SOLID_NOT)
				   continue;
			
				if(teamplay->value && OnSameTeam( self, players[i]) )
				   continue;
				myplayer[counter++] = players[i];
			}
			if(counter > 0)
			{
				// Say something insulting to them!
				if(ltk_chat->value)	// Some people don't want this *sigh*
					LTK_Chat( self, myplayer[rand()%counter], DBC_INSULT);
			}
		}
	}	
}

///////////////////////////////////////////////////////////////////////
// Find a free client spot
///////////////////////////////////////////////////////////////////////
edict_t *ACESP_FindFreeClient (void)
{
	edict_t *bot = NULL;
	int i = 0;
	int max_count = 0;
	
	// This is for the naming of the bots
	for (i = game.maxclients; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		
		if(bot->count > max_count)
			max_count = bot->count;
	}
	
	// Check for free spot
	for (i = game.maxclients; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if (!bot->inuse)
			break;
	}
	
	if (bot->inuse)
		return NULL;
	
	bot->count = max_count + 1; // Will become bot name...
	
	return bot;
}

///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
void ACESP_SetName(edict_t *bot, char *name, char *skin, char *team)
{
	float rnd;
	char userinfo[MAX_INFO_STRING];
	char bot_skin[MAX_INFO_STRING];
	
	// initialise userinfo
	memset(userinfo, 0, sizeof(userinfo));

	//rekkie -- DEV_1 -- s
	//
	// Original code
	/*
	char bot_name[MAX_INFO_STRING];
	// Set the name for the bot.
	// name
	if( (!name) || !strlen(name) )
	{
		// RiEvEr - new code to get random bot names
		LTKsetBotName(bot_name);
	}
	else
		strcpy(bot_name,name);
	*/
	//
	// Modified code
	//
	DC_LoadRandomBotName(userinfo);
	//rekkie -- DEV_1 -- e

	gi.cvar_forceset(stat_logs->name, "0"); // Turning off stat collection since bots are enabled

	// skin
	if( (!skin) || !strlen(skin) )
	{
		// randomly choose skin 
		rnd = random();
		if(rnd  < 0.05)
			sprintf(bot_skin,"male/bluebeard");
		else if(rnd < 0.1)
			sprintf(bot_skin,"female/brianna");
		else if(rnd < 0.15)
			sprintf(bot_skin,"male/blues");
		else if(rnd < 0.2)
			sprintf(bot_skin,"female/ensign");
		else if(rnd < 0.25)
			sprintf(bot_skin,"female/jezebel");
		else if(rnd < 0.3)
			sprintf(bot_skin,"female/jungle");
		else if(rnd < 0.35)
			sprintf(bot_skin,"sas/sasurban");
		else if(rnd < 0.4)
			sprintf(bot_skin,"terror/urbanterr");
		else if(rnd < 0.45)
			sprintf(bot_skin,"female/venus");
		else if(rnd < 0.5)
			sprintf(bot_skin,"sydney/sydney");
		else if(rnd < 0.55)
			sprintf(bot_skin,"male/cajin");
		else if(rnd < 0.6)
			sprintf(bot_skin,"male/commando");
		else if(rnd < 0.65)
			sprintf(bot_skin,"male/grunt");
		else if(rnd < 0.7)
			sprintf(bot_skin,"male/mclaine");
		else if(rnd < 0.75)
			sprintf(bot_skin,"male/robber");
		else if(rnd < 0.8)
			sprintf(bot_skin,"male/snowcamo");
		else if(rnd < 0.85)
			sprintf(bot_skin,"terror/swat");
		else if(rnd < 0.9)
			sprintf(bot_skin,"terror/jungleterr");
		else if(rnd < 0.95)
			sprintf(bot_skin,"sas/saspolice");
		else 
			sprintf(bot_skin,"sas/sasuc");
	}
	else
		strcpy(bot_skin,skin);

	

	// add bot's name/skin/hand to userinfo
	//Info_SetValueForKey (userinfo, "name", bot_name);
	
	Info_SetValueForKey (userinfo, "skin", bot_skin);
	Info_SetValueForKey (userinfo, "hand", "2"); // bot is center handed for now!
//AQ2 ADD
	Info_SetValueForKey (userinfo, "spectator", "0"); // NOT a spectator
//AQ2 END

	ClientConnect (bot, userinfo);

//	ACESP_SaveBots(); // make sure to save the bots
}
//RiEvEr - new global to enable bot self-loading of routes
extern char current_map[55];
//

char *LocalTeamNames[4] = { "spectator", "1", "2", "3" };

///////////////////////////////////////////////////////////////////////
// Spawn the bot
///////////////////////////////////////////////////////////////////////
edict_t *ACESP_SpawnBot( char *team_str, char *name, char *skin, char *userinfo )
{
	int team = 0;
	edict_t	*bot = ACESP_FindFreeClient();

	if( ! bot )
	{
		gi.bprintf( PRINT_MEDIUM, "Server is full, increase Maxclients.\n" );
		return NULL;
	}
	
	bot->is_bot = true;
	bot->yaw_speed = 1000;  // deg/sec
	
	// To allow bots to respawn
	if( ! userinfo )
		ACESP_SetName( bot, name, skin, team_str );  // includes ClientConnect
	else
		ClientConnect( bot, userinfo );
	
	ClientBeginDeathmatch( bot );
	
	// Balance the teams!
	if( teamplay->value )
	{
		if( team_str )
			team = atoi( team_str );
		if( (team < TEAM1) || (team > teamCount) )
			team = GetNextTeamNumber();
		team_str = LocalTeamNames[ team ];
	}
	
	ACESP_PutClientInServer( bot, true, team );

	if (bot_reportasclient->value) {
		//rekkie -- Fake Bot Client -- s
		// Set the average ping this bot will see
		if (random() < 0.85)
			bot->bot.bot_baseline_ping = (int)(3 + (random() * 60)); // Country ping
		else
			bot->bot.bot_baseline_ping = (int)(7 + (random() * 227)); // Country + overseas ping

		
		SV_BotConnect(bot->client->pers.netname); // So the server can fake the bot as a 'client'
		//gi.SV_BotConnect(bot->client->pers.netname); // So the server can fake the bot as a 'client'
	}
	game.bot_count++;
	//rekkie -- Fake Bot Client -- e
	
	//bot->wander_timeout = level.framenum + 60.0 * HZ;
	//ACEAI_PickLongRangeGoal(bot); // pick a new goal
	
	// LTK chat stuff
	if(0 && random() < 0.33)
	{
		// Store current enemies available
		int		i, counter = 0;
		edict_t *myplayer[MAX_BOTS];

		for(i=0;i<=num_players;i++)
		{
			// Find all available enemies to insult
			if(players[i] == NULL || players[i] == bot || 
			   players[i]->solid == SOLID_NOT)
			   continue;
		
			if(teamplay->value && OnSameTeam( bot, players[i]) )
			   continue;
			myplayer[counter++] = players[i];
		}
		if(counter > 0)
		{
			// Say something insulting to them!
			if(ltk_chat->value)	// Some people don't want this *sigh*
				LTK_Chat( bot, myplayer[rand()%counter], DBC_WELCOME);
		}
	}	
	
	return bot;
}

void	ClientDisconnect( edict_t *ent );

///////////////////////////////////////////////////////////////////////
// Remove a bot by name or all bots
///////////////////////////////////////////////////////////////////////
void ACESP_RemoveBot(char *name)
{
	int i;
	qboolean freed=false;
	edict_t *bot;
	qboolean remove_all = (Q_stricmp(name,"all")==0) ? true : false;
	int find_team = (strlen(name)==1) ? atoi(name) : 0;

//	if (name!=NULL)
	for(i=0;i<game.maxclients;i++)
	{
		bot = g_edicts + i + 1;
		if(bot->inuse)
		{
			if( bot->is_bot && (remove_all || !strlen(name) || Q_stricmp(bot->client->pers.netname,name)==0 || (find_team && bot->client->resp.team==find_team)) )
			{
				//rekkie -- Fake Bot Client -- s
				
				if (bot_reportasclient->value) {
					SV_BotDisconnect(bot->client->pers.netname); // So the server can fake the bot as a 'client'
				}
				//gi.SV_BotDisconnect(bot->client->pers.netname); // So the server can remove the fake client
				//rekkie -- Fake Bot Client -- e
				
				bot->health = 0;
				player_die (bot, bot, bot, 100000, vec3_origin);
				// don't even bother waiting for death frames
//				bot->deadflag = DEAD_DEAD;
//				bot->inuse = false;
				freed = true;
				ClientDisconnect( bot );
				game.bot_count--;
//				ACEIT_PlayerRemoved (bot);
//				gi.bprintf (PRINT_MEDIUM, "%s removed\n", bot->client->pers.netname);

				if (bot_personality->value && bot->bot.personality.isActive)
					BOTLIB_FreeBotPersonality(bot);
				if( ! remove_all )
					break;
			}
		}
	}

/*
	//Werewolf: Remove a random bot if no name given
	if(!freed)	
//		gi.bprintf (PRINT_MEDIUM, "%s not found\n", name);
	{
		do
		{
			i = (int)(rand()) % (int)(game.maxclients);
			bot = g_edicts + i + 1;
		}	while ( (!bot->inuse) || (!bot->is_bot) );
		bot->health = 0;
		player_die (bot, bot, bot, 100000, vec3_origin);
		freed = true;
		ClientDisconnect( bot );
	}
*/
	if (!freed) {
		if (debug_mode) {
			gi.bprintf(PRINT_MEDIUM, "No bot removed\n");
		}
	}

//	ACESP_SaveBots(); // Save them again
}

//rekkie -- DEV_1 -- s
///////////////////////////////////////////////////////////////////////
// Remove a bot by team
// Conditions: Bot must be dead or joined a team during an ongoing round
///////////////////////////////////////////////////////////////////////
void ACESP_RemoveTeamplayBot(int team)
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
				if (bot->client->resp.team == team && team_round_going == 0)
				{
					if (random() < 0.20) // Randomly kick a bot
					{
						//rekkie -- Fake Bot Client -- s
						if (bot_reportasclient->value) {
							SV_BotDisconnect(bot->client->pers.netname); // So the server can fake the bot as a 'client'
						}
						//gi.SV_BotDisconnect(bot->client->pers.netname); // So the server can remove the fake client
						//rekkie -- Fake Bot Client -- e
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
//rekkie -- DEV_1 -- e

//====================================
// Stuff to generate pseudo-random names
//====================================
#define NUMNAMES	10
char	*names1[NUMNAMES] = {
	"Bad", "Death", "L33t", "Fast", "Real", "Lethal", "Hyper", "Hard", "Angel", "Red"};

char	*names2[NUMNAMES] = {
	"Moon", "evil", "master", "dude", "killa", "dog", "chef", "dave", "Zilch", "Amator" };

char	*names3[NUMNAMES] = {
	"Ana", "Bale", "Calen", "Cor", "Fan", "Gil", "Hali", "Line", "Male", "Pero"};

char	*names4[NUMNAMES] = {
	"ders", "rog", "born", "dor", "fing", "galad", "bon", "loss", "orch", "riel" };

qboolean	nameused[NUMNAMES][NUMNAMES];

//====================================
// AQ2World Staff Names -- come shoot at us!
// Find time to implement this!  Or better yet, 
// load names from a file rather than this array
//====================================
// #define AQ2WORLDNUMNAMES	14
// char	*aq2names[AQ2WORLDNUMNAMES] = {
// 	"bAron", "darksaint", "FragBait", "matic", "stan0x", "TgT", "dmc", "dox", "KaniZ", "keffo", "QuimBy", "Rezet", "Royce", "vrol"
// 	};
//qboolean	adminnameused[AQ2WORLDNUMNAMES];
// END AQ2World Staff Names //

//====================================
// New random bot naming routine
//====================================
void	LTKsetBotName( char	*bot_name )
{
	int	part1,part2;
	part1 = part2 = 0;

	do // Load random bot names from NUMNAMES lists
	{
		part1 = rand()% NUMNAMES;
		part2 = rand()% NUMNAMES;
	}while( nameused[part1][part2]);

	// Mark that name as used
	nameused[part1][part2] = true;
	// Now put the name together
	if( random() < 0.5 )
	{
		strcpy( bot_name, names1[part1]);
		strcat( bot_name, names2[part2]);
	}
	else
	{
		strcpy( bot_name, names3[part1]);
		strcat( bot_name, names4[part2]);
	}
}

//rekkie -- DEV_1 -- s
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
// DC_GetRandomClanSymbol()
// Gets a random clan tag char symbol: ie --> [ ], ( ), { }, < >, etc
// Returns char
//======================================
int DC_GetRandomClanSymbol(void)
{	
	int sym = rand() % 27;
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
		case 12: return '/';
		case 13: return ':';
		case 14: return '<';
		case 15: return '=';
		case 16: return '>';
		case 17: return '?';
		case 18: return '@';
		case 19: return '[';
		case 20: return ']';
		case 21: return '\\';
		case 22: return '^';
		case 23: return '_';
		case 24: return '{';
		case 25: return '|';
		case 26: return '}';
		default: return ' ';
	}
}

//======================================
// DC_GetOpposingClanSymbol()
// If a symbol has an opposite: ie --> [ ], ( ), { }, < >, etc
// Returns opposite side, else returns the input symbol
//======================================
int DC_GetOpposingClanSymbol(char symbol)
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
// DC_GetRandomClanLetter()
// Gets a random clan letter
// Returns char
//======================================
int DC_GetRandomClanLetter(void)
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


//======================================
// DC_LoadRandomBotName()
// Gets a random bot name from file; bot_name is limited to a maximum of 16 chars, and the size of the list can be up to random()'s maximum of 32,767
// Genders can be: GENDER_MALE, GENDER_FEMALE, GENDER_NEUTRAL
// Returns bot_name
//======================================
void DC_LoadRandomBotName(char *userinfo)
{	
	// File stuff
	FILE* f;
	cvar_t* game_dir = gi.cvar("game", "action", 0); // Directory of the gamelib
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);	 // Directory of the bot files in the gamelib
	char	 filename[MAX_QPATH];	// Filename to load from
	int		 comments_num = 0;		// Keep track of how many comments the file has
	int		 line_num = 0;			// Keep track of lines
	int		 curr_len;				// Current length of the line
	char	 curr_line[1024];		// Accounts for reading lines that could be fairly long (comments)
	curr_line[0] = '\0';
#ifdef _WIN32
	int		i; // Keep track where we are in the filename array
#endif
	
	// Bot stuff
	char	 bot_name[MAX_QPATH];		// Full bot name
	qboolean name_has_prefix = false;	// If the name gained a prefix
	qboolean name_has_clan = false;		// If the name gained a clan
	int		 gender;					// Bot gender
	
	// Try to apply either a clan tag, or a prefix to the start of the name
	float prefix = random();
	if (prefix < 0.2)
	{
		name_has_prefix = true;
		
#ifdef	_WIN32
		i = sprintf(filename, ".\\");
		i += sprintf(filename + i, game_dir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, botdir->string);
		i += sprintf(filename + i, "\\");
		
		if (prefix < 0.1) // Clan tag
			i += sprintf(filename + i, "clans.txt");
		else // Prefix
			i += sprintf(filename + i, "prefix.txt");
#else
		strcpy(filename, "./");
		strcat(filename, game_dir->string);
		strcat(filename, "/");
		strcat(filename, botdir->string);
		strcat(filename, "/");
		if (prefix < 0.1) // Clan tag
			strcat(filename, "clans.txt");
		else // Prefix
			strcat(filename, "prefix.txt");
		
#endif

		// Open and check for success
		if ((f = fopen(filename, "r")) == NULL) // Read text file
		{
			gi.dprintf("%s failed to open bot file: %s\n", __func__, filename);
			return;
		}

		// Read each line
		while (fgets(curr_line, sizeof(curr_line), f))
		{
			if (curr_line[0] == '/') // Skip comment lines
			{
				comments_num++;
				continue;
			}

			line_num++;
		}
		fclose(f);

		int random_line;
		if (line_num - comments_num > 0)
			random_line = rand() % (line_num - comments_num);
		else
			return; // File has no names

		// Open and check for success
		if ((f = fopen(filename, "r")) != NULL) // Read text file
		{
			line_num = 0;

			// Read each line
			while (fgets(curr_line, sizeof(curr_line), f))
			{
				if (feof(f))
				{
					// Could not get a prefix name
					break;
				}
				// If we're at or above the correct line, not a comment, not empty
				if (line_num >= random_line && curr_line[0] != '/' && curr_line[0])
				{
					curr_len = strlen(curr_line);
					Q_strlcpy(bot_name, curr_line, curr_len);
					break;
				}
				else
					line_num++; // Either we're not at random_line, or we're on a comment line
			}
			fclose(f);
		}
	}
	else if (prefix < 0.3) // Random gen
	{
		name_has_clan = true;
		
		byte tag = 0;
		char outer_symbol = 0;
		char inner_symbol = 0;
				
		outer_symbol = DC_GetRandomClanSymbol();
		if (random() < 0.5) // 50% chance to add an inner symbol
			inner_symbol = DC_GetRandomClanSymbol();
		
		// Add the prefix tag
		bot_name[tag] = outer_symbol;
		if (inner_symbol)
			bot_name[++tag] = inner_symbol;
		
		// Add the clan acronym
		bot_name[++tag] = DC_GetRandomClanLetter();
		bot_name[++tag] = DC_GetRandomClanLetter();
		if (random() < 0.25) bot_name[++tag] = DC_GetRandomClanLetter(); // 50% chance to add another clan letter
		
		// Add the suffix tag
		if (inner_symbol)
			bot_name[++tag] = DC_GetOpposingClanSymbol(inner_symbol); // Add the opposing symbol, if any
		bot_name[++tag] = DC_GetOpposingClanSymbol(outer_symbol);
		
		// print the bot name
		bot_name[++tag] = '\0';
		//Com_Printf("CLAN TAG  %s\n", bot_name);
	}


	

	// Pick a random name
	float rnd_gender = random();
	if (rnd_gender < 0.15)
	{
		gender = GENDER_MALE;
		Info_SetValueForKey(userinfo, "gender", "male");
	}
	else if (rnd_gender < 0.30)
	{
		gender = GENDER_FEMALE;
		Info_SetValueForKey(userinfo, "gender", "female");
	}
	else
	{
		gender = GENDER_NEUTRAL;
		Info_SetValueForKey(userinfo, "gender", "none");
	}

#ifdef	_WIN32
	i = sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, botdir->string);
	i += sprintf(filename + i, "\\");
	if (gender == GENDER_MALE)
		i += sprintf(filename + i, "males.txt");
	else if (gender == GENDER_FEMALE)
		i += sprintf(filename + i, "females.txt");
	else
		i += sprintf(filename + i, "other.txt");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/");
	if (gender == GENDER_MALE)
		strcat(filename, "males.txt");
	else if (gender == GENDER_FEMALE)
		strcat(filename, "females.txt");
	else
		strcat(filename, "other.txt");
#endif


	// Open and check for success
	if ((f = fopen(filename, "r")) == NULL) // Read text file
	{
		gi.dprintf("%s failed to open bot name file: %s\n", __func__, filename);
		return;
	}

	// Read each line
	while (fgets(curr_line, sizeof(curr_line), f))
	{
		if (curr_line[0] == '/') // Skip comment lines
		{
			comments_num++;
			continue;
		}

		line_num++;
	}
	fclose(f);

	int random_line;
	if (line_num - comments_num > 0)
		random_line = rand() % (line_num - comments_num);
	else
		return; // File has no names

	// Open and check for success
	if ((f = fopen(filename, "r")) != NULL) // Read text file
	{
		line_num = 0;

		// Read each line
		while (fgets(curr_line, sizeof(curr_line), f))
		{
			if (feof(f))
			{
				//gi.dprintf("%s could not generate a bot name, reached EOF on \n", __func__, filename);
				Q_strlcpy(bot_name, "AqtionMan", 9);
				break;
			}
			// If we're at or above the correct line and its not a comment
			if (line_num >= random_line && curr_line[0] != '/')
			{
				curr_len = strlen(curr_line);
				
				if (name_has_prefix)
				{
					Q_strncpyz(bot_name, va("%s %s", bot_name, curr_line), sizeof(bot_name)); // Space between prefix and name
				}
				else if (name_has_clan)
				{
					if (random() < 0.5)
						Q_strncpyz(bot_name, va("%s%s", bot_name, curr_line), sizeof(bot_name)); // Space between clan and name
					else
						Q_strncpyz(bot_name, va("%s%s", bot_name, curr_line), sizeof(bot_name)); // No space between clan and name
				}
				else
					Q_strncpyz(bot_name, curr_line, sizeof(bot_name));
					
				
				break;
			}
			else
				line_num++; // Either we're not at random_line, or we're on a comment line
		}
		fclose(f);
	}
	


	// Try to apply a postfix to the end of the name
	float postfix = random();
	if (postfix < 0.1)
	{
		//name_has_prefix = true;

#ifdef	_WIN32
		i = sprintf(filename, ".\\");
		i += sprintf(filename + i, game_dir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, botdir->string);
		i += sprintf(filename + i, "\\");
		i += sprintf(filename + i, "postfix.txt");
#else
		strcpy(filename, "./");
		strcat(filename, game_dir->string);
		strcat(filename, "/");
		strcat(filename, botdir->string);
		strcat(filename, "/");
		strcat(filename, "prefix.txt");

#endif

		// Open and check for success
		if ((f = fopen(filename, "r")) == NULL) // Read text file
		{
			gi.dprintf("%s failed to open bot file: %s\n", __func__, filename);
			return;
		}

		// Read each line
		while (fgets(curr_line, sizeof(curr_line), f))
		{
			if (curr_line[0] == '/') // Skip comment lines
			{
				comments_num++;
				continue;
			}

			line_num++;
		}
		fclose(f);

		int random_line;
		if (line_num - comments_num > 0)
			random_line = rand() % (line_num - comments_num);
		else
			return; // File has no names

		// Open and check for success
		if ((f = fopen(filename, "r")) != NULL) // Read text file
		{
			line_num = 0;

			// Read each line
			while (fgets(curr_line, sizeof(curr_line), f))
			{
				if (feof(f))
				{
					// Could not get a prefix name
					break;
				}
				// If we're at or above the correct line and its not a comment
				if (line_num >= random_line && curr_line[0] != '/')
				{
					Q_strncpyz(bot_name, va("%s %s", bot_name, curr_line), sizeof(bot_name));
					//Q_strlcat(bot_name, curr_line, curr_len);
					break;
				}
				else
					line_num++; // Either we're not at random_line, or we're on a comment line
			}
			fclose(f);
		}
	}
	
	

	Info_SetValueForKey(userinfo, "name", bot_name);
	

	/*
	curr_len = strlen(curr_line);
	if (curr_len > 0) // && bot_name[curr_len - 1] == '\n')
	{
		if (curr_len > name_length)
		{
			curr_line[curr_len] = '\0';
			Com_Printf("%s bot name %s is limited to 16 chars, found %i\n", __func__, curr_line, curr_len);
			bot_name[name_length] = '\0';
			//Com_Printf("%s using bot name %s\n", __func__, bot_name);
		}
		else
		{
			bot_name[curr_len] = '\0';
		}
	}
	else
	{
		gi.dprintf("%s could not generate a bot name\n", __func__);
	}
	*/
}
//rekkie -- DEV_1 -- e