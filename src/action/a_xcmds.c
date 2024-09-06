//-----------------------------------------------------------------------------
// PG BUND
// a_xcmds.c
//
// contains all new non standard command functions
//
// $Id: a_xcmds.c,v 1.15 2004/04/08 23:19:51 slicerdw Exp $
//
//-----------------------------------------------------------------------------
// $Log: a_xcmds.c,v $
// Revision 1.15  2004/04/08 23:19:51  slicerdw
// Optimized some code, added a couple of features and fixed minor bugs
//
// Revision 1.14  2004/01/18 11:20:14  igor_rock
// added flashgrenades
//
// Revision 1.13  2002/04/01 15:16:06  freud
// Stats code redone, tng_stats now much more smarter. Removed a few global
// variables regarding stats code and added kevlar hits to stats.
//
// Revision 1.12  2002/03/26 21:49:01  ra
// Bufferoverflow fixes
//
// Revision 1.11  2001/11/08 20:56:24  igor_rock
// - changed some things related to wp_flags
// - corrected use_punch bug when player only has an empty weapon left
//
// Revision 1.10  2001/11/08 13:22:18  igor_rock
// added missing parenthises (use_punch didn't function correct)
//
// Revision 1.9  2001/11/03 17:21:57  deathwatch
// Fixed something in the time command, removed the .. message from the voice command, fixed the vote spamming with mapvote, removed addpoint command (old pb command that wasnt being used). Some cleaning up of the source at a few points.
//
// Revision 1.8  2001/09/28 13:48:34  ra
// I ran indent over the sources. All .c and .h files reindented.
//
// Revision 1.7  2001/09/28 13:44:23  slicerdw
// Several Changes / improvements
//
// Revision 1.6  2001/08/15 14:50:48  slicerdw
// Added Flood protections to Radio & Voice, Fixed the sniper bug AGAIN
//
// Revision 1.5  2001/07/13 00:34:52  slicerdw
// Adjusted Punch command
//
// Revision 1.4  2001/05/31 16:58:14  igor_rock
// conflicts resolved
//
// Revision 1.3.2.2  2001/05/31 06:47:51  igor_rock
// - removed crash bug with non exisitng flag files
// - added new commands "setflag1", "setflag2" and "saveflags" to create
//   .flg files
//
// Revision 1.3.2.1  2001/05/25 18:59:52  igor_rock
// Added CTF Mode completly :)
// Support for .flg files is still missing, but with "real" CTF maps like
// tq2gtd1 the ctf works fine.
// (I hope that all other modes still work, just tested DM and teamplay)
//
// Revision 1.3  2001/05/13 01:23:01  deathwatch
// Added Single Barreled Handcannon mode, made the menus and scoreboards
// look nicer and made the voice command a bit less loud.
//
// Revision 1.2  2001/05/11 12:21:18  slicerdw
// Commented old Location support ( ADF ) With the ML/ETE Compatible one
//
// Revision 1.1.1.1  2001/05/06 17:25:16  igor_rock
// This is the PG Bund Edition V1.25 with all stuff laying around here...
//
//-----------------------------------------------------------------------------

#include "g_local.h"
#include "m_player.h"

//AQ2:TNG - Slicer Old Location support
//loccube_t *setcube = NULL;
//AQ2:TNG End

//
void _Cmd_Rules_f (edict_t * self, char *argument)
{
	char section[32], mbuf[1024], *p, buf[30][INI_STR_LEN];
	int i, j = 0;
	ini_t ini;

	strcpy (mbuf, "\n");
	if (*argument)
		Q_strncpyz(section, argument, sizeof(section));
	else
		strcpy (section, "main");

	if (OpenIniFile (GAMEVERSION "/prules.ini", &ini))
	{
		i = ReadIniSection (&ini, section, buf, 30);
		while (j < i)
		{
			p = buf[j++];
			if (*p == '.')
				p++;
			Q_strncatz(mbuf, p, sizeof(mbuf));
			Q_strncatz(mbuf, "\n", sizeof(mbuf));
		}
		CloseIniFile (&ini);
	}
	if (!j)
		gi.cprintf (self, PRINT_MEDIUM, "No rules on %s available\n", section);
	else
		gi.cprintf (self, PRINT_MEDIUM, "%s", mbuf);
}

void Cmd_Rules_f (edict_t * self)
{
	char *s;

	s = gi.args ();
	_Cmd_Rules_f (self, s);
}

//
void Cmd_Menu_f (edict_t * self)
{
	char *s;

	s = gi.args ();
	vShowMenu (self, s);
}

//
void Cmd_Punch_f (edict_t * self)
{
	if (!use_punch->value || !IS_ALIVE(self) || self->client->resp.sniper_mode != SNIPER_1X)
		return;

	if (self->client->weaponstate != WEAPON_READY && self->client->weaponstate != WEAPON_END_MAG)
		return;

	// animation moved to punch_attack() in a_xgame.c
	// punch_attack is now called in ClientThink after evaluation punch_desired
	// for "no punch when firing" stuff - TempFile
	if (level.framenum > self->client->punch_framenum + PUNCH_DELAY) {
		self->client->punch_framenum = level.framenum;	// you aren't Bruce Lee! :)
		self->client->punch_desired = true;
	}
}

/*
//Adds a point with name to location file - cheats must be enabled!
void
Cmd_Addpoint_f (edict_t * self)
{
  gi.cprintf (self, PRINT_MEDIUM,
	      "\nLocation point feature was dropped in 1.20 and\n"
	      "replaced by location area cubes.\nSee readme.txt for details.\n");
  //FILE *pntlist;
     char *s, buf[1024];

     s = gi.args();
     if (!*s)  
     { 
     gi.cprintf(self, PRINT_MEDIUM, "\nCommand needs argument, use addpoint <description>.\n");
     return;
     }
     sprintf(buf, "%s/maps/%s%s", GAMEVERSION, level.mapname, PG_LOCEXT);
     pntlist = fopen(buf, "a");
     if (pntlist == NULL)
     {
     gi.cprintf(self, PRINT_MEDIUM, "\nError accessing loc file %s.\n", buf);
     return;
     }
     sprintf(buf, "%.2f %.2f %.2f %s\n", self->s.origin[0], self->s.origin[1], self->s.origin[2], s);
     fputs(buf, pntlist);
     fclose(pntlist);
     gi.cprintf(self, PRINT_MEDIUM, "\nPoint added.\n"); 
}
*/

//Plays a sound file
void Cmd_Voice_f (edict_t * self)
{
	char *s;
	char fullpath[MAX_QPATH];

	if (!use_voice->value)
		return;

	s = gi.args ();
	//check if no sound is given
	if (!*s)
	{
		gi.cprintf (self, PRINT_MEDIUM,
			"\nCommand needs argument, use voice <soundfile.wav>.\n");
		return;
	}
	if (strlen (s) > 32)
	{
		gi.cprintf (self, PRINT_MEDIUM,
			"\nArgument is too long. Maximum length is 32 characters.\n");
		return;
	}
	// AQ2:TNG Disabled this message: why? -M
	if (strstr (s, ".."))
	{
		gi.cprintf (self, PRINT_MEDIUM,
			"\nArgument must not contain \"..\".\n");
		return;
	}
	
	//check if player is dead
	if (!IS_ALIVE(self))
		return;

	strcpy(fullpath, PG_SNDPATH);
	strcat(fullpath, s);
	// SLIC2 Taking this out.
	/*if (radio_repeat->value)
	{
	if ((d = CheckForRepeat (self, s)) == false)
	return;
	}*/
	if (radio_max->value)
	{
		if (CheckForFlood (self)== false)
			return;
	}
	// AQ2:TNG Deathwatch - This should be IDLE not NORM
	gi.sound (self, CHAN_VOICE, gi.soundindex (fullpath), 1, ATTN_IDLE, 0);
	// AQ2:TNG END
}

//AQ2:TNG SLicer - Old location support
// TempFile - BEGIN
/*
void
Cmd_BeginCube_f (edict_t * self)
{
  if (setcube)
    {
      gi.cprintf (self, PRINT_MEDIUM, "There is already a cube allocated at 0x%p.\n",
		  (void *) setcube);
      return;
    }

  setcube = gi.TagMalloc (sizeof (loccube_t), TAG_GAME);
  if (!setcube)
    return;

  gi.cprintf (self, PRINT_MEDIUM, "New cube successfully allocated at 0x%p.\n", (void *) setcube);
  gi.cprintf (self, PRINT_MEDIUM, "Please set lower left and upper right corner.\n");
}

void
Cmd_SetCubeLL_f (edict_t * self)
{
  if (!setcube)
    {
      gi.cprintf (self, PRINT_MEDIUM, "Please allocate a cube first by executing BeginCube.\n");
      return;
    }

  memcpy (setcube->lowerleft, self->s.origin, sizeof (vec3_t));
  gi.cprintf (self, PRINT_MEDIUM, "Lower left has been set to <%.2f %.2f %.2f>.\n",
       setcube->lowerleft[0], setcube->lowerleft[1], setcube->lowerleft[2]);
}

void
Cmd_SetCubeUR_f (edict_t * self)
{
  if (!setcube)
    {
      gi.cprintf (self, PRINT_MEDIUM, "Please allocate a cube first by executing BeginCube.\n");
      return;
    }

  memcpy (setcube->upperright, self->s.origin, sizeof (vec3_t));
  gi.cprintf (self, PRINT_HIGH, "Upper right has been set to <%.2f %.2f %.2f>.\n",
    setcube->upperright[0], setcube->upperright[1], setcube->upperright[2]);
}

void
Cmd_AbortCube_f (edict_t * self)
{
  if (!setcube)
    {
      gi.cprintf (self, PRINT_MEDIUM, "No cube to deallocate.\n");
      return;
    }

  gi.TagFree (setcube);
  gi.cprintf (self, PRINT_MEDIUM, "Cube at 0x%p successfully deallocated.\n",
	      (void *) setcube);
  setcube = NULL;
}

//Adds a cube with name to location file - cheats must be enabled!
void
Cmd_AddCube_f (edict_t * self)
{
  FILE *pntlist;
  char *s, buf[1024];

  if (!setcube)
    {
      gi.cprintf (self, PRINT_MEDIUM, "\nPlease allocate a cube first by executing BeginCube.\n");
      return;
    }

  if (!setcube->lowerleft[0] || !setcube->lowerleft[1] || !setcube->lowerleft[2] ||
      !setcube->upperright[0] || !setcube->upperright[1] || !setcube->upperright[2])
    {
      gi.cprintf (self, PRINT_MEDIUM, "\nPlease set cube corners first using SetCubeLL and SetCubeUR.\n");
      return;
    }

  FixCubeData (setcube);
  s = gi.args ();
  strcpy (setcube->desc, s);

  if (!*s)
    {
      gi.cprintf (self, PRINT_MEDIUM, "\nCommand needs argument, use addcube <description>.\n");
      return;
    }
  sprintf (buf, "%s/location/%s%s", GAMEVERSION, level.mapname, PG_LOCEXTEX);
  pntlist = fopen (buf, "a");
  if (pntlist == NULL)
    {
      gi.cprintf (self, PRINT_MEDIUM, "\nError accessing adf file %s.\n", buf);
      return;
    }

  sprintf (buf, "<%.2f %.2f %.2f> <%.2f %.2f %.2f> %s\n",
	setcube->lowerleft[0], setcube->lowerleft[1], setcube->lowerleft[2],
	   setcube->upperright[0], setcube->upperright[1], setcube->upperright[2], setcube->desc);
  fputs (buf, pntlist);
  fclose (pntlist);
  memcpy (&mapdescex[num_loccubes], setcube, sizeof (*setcube));
  num_loccubes++;

  gi.TagFree (setcube);
  setcube = NULL;

  gi.cprintf (self, PRINT_MEDIUM, "\nCube added.\n");
}

void
Cmd_PrintCubeState_f (edict_t * self)
{
  if (!setcube)
    {
      gi.cprintf (self, PRINT_MEDIUM, "\nPlease allocate a cube first by executing BeginCube.\n");
      return;
    }

  gi.cprintf (self, PRINT_MEDIUM, "\nTemporary cube allocated at %p.\nLower left corner: "
	      "<%.2f %.2f %.2f>\nUpper right corner: <%.2f %.2f %.2f>\n", (void *) setcube,
	setcube->lowerleft[0], setcube->lowerleft[1], setcube->lowerleft[2],
    setcube->upperright[1], setcube->upperright[2], setcube->upperright[2]);
}


// TempFile - END
*/
//AQ2:TNG END

// Show your location name, plus position and heading if cheats are enabled.
void Cmd_WhereAmI_f( edict_t * self )
{
	char location[ 128 ] = "";
	qboolean found = GetPlayerLocation( self, location );

	if( found )
		gi.cprintf( self, PRINT_MEDIUM, "Location: %s\n", location );
	else if( ! sv_cheats->value )
		gi.cprintf( self, PRINT_MEDIUM, "Location unknown.\n" );

	if( sv_cheats->value )
	{
		gi.cprintf( self, PRINT_MEDIUM, "Origin: %5.0f,%5.0f,%5.0f  Facing: %3.0f\n",
			self->s.origin[0], self->s.origin[1], self->s.origin[2], self->s.angles[1] );
	}
}

// Variables for new flags

static char flagpos1[64] = { 0 };
static char flagpos2[64] = { 0 };

//sets red flag position - cheats must be enabled!
void Cmd_SetFlag1_f (edict_t * self)
{
	Q_snprintf (flagpos1, sizeof(flagpos1), "<%.2f %.2f %.2f>", self->s.origin[0], self->s.origin[1],
		self->s.origin[2]);
	gi.cprintf (self, PRINT_MEDIUM, "\nRed Flag added at %s.\n", flagpos1);
}

//sets blue flag position - cheats must be enabled!
void Cmd_SetFlag2_f (edict_t * self)
{
	Q_snprintf (flagpos2, sizeof(flagpos2), "<%.2f %.2f %.2f>", self->s.origin[0], self->s.origin[1],
		self->s.origin[2]);
	gi.cprintf (self, PRINT_MEDIUM, "\nBlue Flag added at %s.\n", flagpos2);
}

//Save flag definition file - cheats must be enabled!
void Cmd_SaveFlags_f (edict_t * self)
{
	FILE *fp;
	char buf[128];

	if (!(flagpos1[0] && flagpos2[0]))
	{
		gi.cprintf (self, PRINT_MEDIUM,
			"You only can save flag positions when you've already set them\n");
		return;
	}

	sprintf (buf, "%s/tng/%s.flg", GAMEVERSION, level.mapname);
	fp = fopen (buf, "w");
	if (fp == NULL)
	{
		gi.cprintf (self, PRINT_MEDIUM, "\nError accessing flg file %s.\n", buf);
		return;
	}
	sprintf (buf, "# %s\n", level.mapname);
	fputs (buf, fp);
	sprintf (buf, "%s\n", flagpos1);
	fputs (buf, fp);
	sprintf (buf, "%s\n", flagpos2);
	fputs (buf, fp);
	fclose (fp);

	flagpos1[0] = 0;
	flagpos2[0] = 0;

	gi.cprintf (self, PRINT_MEDIUM, "\nFlag File saved.\n");
}
//SLIC2
/*
void Cmd_FlashGrenade_f(edict_t *ent)
{
  if (ent->client->grenadeType == GRENADE_NORMAL) {
    gi.cprintf(ent, PRINT_HIGH, "Flash grenades selected.\n");
    ent->client->grenadeType = GRENADE_FLASH;
  } else {
    gi.cprintf(ent, PRINT_HIGH, "Standard grenades selected.\n");
    ent->client->grenadeType = GRENADE_NORMAL;
  }
}*/

// static void SV_AddLrconCmd_f(void)
// {
//     if (Cmd_Argc() < 2) {
//         Com_Printf("Usage: %s <command>\n", Cmd_Argv(0));
//         return;
//     }

//     SV_AddStuffCmd(&sv_lrconlist, 1, "Lrcon");
// }

// static void SV_DelLrconCmd_f(void)
// {
//     if (Cmd_Argc() < 2) {
//         Com_Printf("Usage: %s <id|cmd|all>\n", Cmd_Argv(0));
//         return;
//     }

//     SV_DelStuffCmd(&sv_lrconlist, 1, "lrcon");
// }

// static void SV_ListLrconCmds_f(void)
// {
//     SV_ListStuffCmds(&sv_lrconlist, "lrcon");
// }

// Assuming each node in the list contains a command string
typedef struct command_node_s {
    list_t sv_lrconlist;
    char *command;
} command_node_t;

// Function to check if a command is in the list of lrcon commands
static qboolean SV_IsLrconCommand(const char* cmd) {
    list_t *node;
    for (node = sv_lrconlist.next; node != &sv_lrconlist; node = node->next) {
        command_node_t *cmd_node = (command_node_t *)node;
        if (strcmp(cmd_node->command, cmd) == 0) {
            return true;
        }
    }
    return false;
}

void Cmd_Lrcon_f (edict_t* self)
{
	char* cmd = gi.args();
	if (!*cmd)
	{
		gi.cprintf(self, PRINT_HIGH, "Usage: lrcon <command> [parameters]\nType \"lrcon help\" for more information.\n");
		return;
	}
	if (strcmp(cmd, "help") == 0)
	{
		gi.cprintf(self, PRINT_HIGH, 
			"Limited rcon usage:\n"
			"lrcon claim             - claim the server\n"
			"lrcon release           - release the server to be re-claimed\n"
			"lrcon <cvar>            - query cvar value\n"
			"lrcon <cvar> <value>    - set cvar value\n"
			"lrcon status            - get client status information\n"
			"lrcon kick <id>         - kick a player\n"
			"lrcon map <mapname>     - change map\n");
		return;
	}
	// The following requires the server to be claimed

	// Check if we have a claimer
	if (game.lrcon_claimer != self) {
			// Check if the claimer is the one who is trying to use lrcon
			gi.cprintf(self, PRINT_HIGH, "Server is already claimed by %s\n", game.lrcon_claimer->client->pers.netname);
			return;
	} else if (game.lrcon_claimer == NULL){
		// Check if the claimer is the one who is trying to use lrcon
		if (strcmp(cmd, "claim") == 0)
		{
			game.lrcon_claimer = self;
			gi.cprintf(self, PRINT_HIGH, "You have claimed the server! Type lrcon <command> to set appropriate settings.\n");
			gi.bprintf(PRINT_HIGH, "Server claimed by %s\n", self->client->pers.netname);
			return;
		}
	}

	// If you haven't claimed, you can go no further
	if (game.lrcon_claimer != self) {
		gi.cprintf(self, PRINT_HIGH, "You have not claimed the server.\n");
		return;
	}

	if (strcmp(cmd, "release") == 0)
	{
		if (game.lrcon_claimer == NULL) {
			gi.cprintf(self, PRINT_HIGH, "You released the server.\n");
			gi.bprintf(PRINT_HIGH, "%s released the server, use \"lrcon claim\" to re-claim.\n", self->client->pers.netname);
			return;
		}
	} else if (strcmp(cmd, "status") == 0) {
		if (game.lrcon_claimer != self) {
			gi.cprintf(self, PRINT_HIGH, "You have not claimed the server.\n");
			return;
		} else {
			SV_Status_f(); 
			return;
		}
	} else if (strcmp(cmd, "kick") == 0) {
		char* id = gi.args();
		if (!*id)
		{
			gi.cprintf(self, PRINT_HIGH, "Usage: kick <id>\n");
			return;
		}

		// Check if the id is a valid number
		char* endptr;
		int playerID = strtol(id, &endptr, 10);
		if (*endptr != '\0' || playerID < 0)
		{
			gi.cprintf(self, PRINT_HIGH, "Invalid player ID: %s\n", id);
			return;
		}

		// Check if playerID is within a valid range
		if (playerID >= maxclients->value)
		{
			gi.cprintf(self, PRINT_HIGH, "Player ID out of range: %d\n", playerID);
			return;
		}

		// Kick player with id
		Kick_Client(playerID);
		return;
	} else if (strcmp(cmd, "map") == 0) {
		char* mapname = gi.args();
		if (!*mapname)
		{
			gi.cprintf(self, PRINT_HIGH, "Usage: map <mapname>\n");
			return;
		}

		// Set the command arguments
		char command[256];
		snprintf(command, sizeof(command), "map %s", mapname);
		Cmd_SetArgs(command);

		// Call SV_Map_f function
		SV_Map_f();
		return;
	}




	else { // Unknown command
		gi.cprintf(self, PRINT_HIGH, "Unknown or disallowed lrcon command: %s\n", cmd);
		return;
	}

}