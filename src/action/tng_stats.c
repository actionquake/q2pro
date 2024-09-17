//-----------------------------------------------------------------------------
// Statistics Related Code
//
// $Id: tng_stats.c,v 1.33 2004/05/18 20:35:45 slicerdw Exp $
//
//-----------------------------------------------------------------------------
// $Log: tng_stats.c,v $
// Revision 1.33  2004/05/18 20:35:45  slicerdw
// Fixed a bug on stats command
//
// Revision 1.32  2002/04/03 15:05:03  freud
// My indenting broke something, rolled the source back.
//
// Revision 1.30  2002/04/01 16:08:59  freud
// Fix in hits/shots counter for each weapon
//
// Revision 1.29  2002/04/01 15:30:38  freud
// Small stat fix
//
// Revision 1.28  2002/04/01 15:16:06  freud
// Stats code redone, tng_stats now much more smarter. Removed a few global
// variables regarding stats code and added kevlar hits to stats.
//
// Revision 1.27  2002/03/28 12:10:12  freud
// Removed unused variables (compiler warnings).
// Added cvar mm_allowlock.
//
// Revision 1.26  2002/03/15 19:28:36  deathwatch
// Updated with stats rifle name fix
//
// Revision 1.25  2002/02/26 23:09:20  freud
// Stats <playerid> not working, fixed.
//
// Revision 1.24  2002/02/21 23:38:39  freud
// Fix to a BAD stats bug. CRASH
//
// Revision 1.23  2002/02/18 23:47:33  freud
// Fixed FPM if time was 0
//
// Revision 1.22  2002/02/18 19:31:40  freud
// FPM fix.
//
// Revision 1.21  2002/02/18 17:21:14  freud
// Changed Knife in stats to Slashing Knife
//
// Revision 1.20  2002/02/17 21:48:56  freud
// Changed/Fixed allignment of Scoreboard
//
// Revision 1.19  2002/02/05 09:27:17  freud
// Weapon name changes and better alignment in "stats list"
//
// Revision 1.18  2002/02/03 01:07:28  freud
// more fixes with stats
//
// Revision 1.14  2002/01/24 11:29:34  ra
// Cleanup's in stats code
//
// Revision 1.13  2002/01/24 02:24:56  deathwatch
// Major update to Stats code (thanks to Freud)
// new cvars:
// stats_afterround - will display the stats after a round ends or map ends
// stats_endmap - if on (1) will display the stats scoreboard when the map ends
//
// Revision 1.12  2001/12/31 13:29:06  deathwatch
// Added revision header
//
//
//-----------------------------------------------------------------------------

#include "g_local.h"
#include <time.h>
#include <errno.h>

/* Stats Command */

void ResetStats(edict_t *ent)
{
	int i;

	if(!ent->client)
		return;

	ent->client->resp.shotsTotal = 0;
	ent->client->resp.hitsTotal = 0;

	for (i = 0; i<LOC_MAX; i++)
		ent->client->resp.hitsLocations[i] = 0;

	memset(ent->client->resp.gunstats, 0, sizeof(ent->client->resp.gunstats));
}

void Stats_AddShot( edict_t *ent, int gun )
{
	if( in_warmup )
		return;

	if ((unsigned)gun >= MAX_GUNSTAT) {
		gi.dprintf( "Stats_AddShot: Bad gun number!\n" );
		return;
	}

	if (!teamplay->value || team_round_going || stats_afterround->value) {
		ent->client->resp.shotsTotal += 1;	// TNG Stats, +1 hit
		ent->client->resp.gunstats[gun].shots += 1;	// TNG Stats, +1 hit
	}
}

void Stats_AddHit( edict_t *ent, int gun, int hitPart )
{
	int headShot = (hitPart == LOC_HDAM || hitPart == LOC_KVLR_HELMET) ? 1 : 0;

	ent->client->last_damaged_part = hitPart;

	if( in_warmup )
		return;

	// Adjusted logic to be inclusive rather than exclusive
	if (
	((unsigned)gun <= MAX_GUNSTAT) || 
	((unsigned)gun == MOD_KICK) || 
	((unsigned)gun == MOD_PUNCH) || 
	((unsigned)gun == MOD_GRENADE_IMPACT)
	) {

		if (!teamplay->value || team_round_going || stats_afterround->value) {
			ent->client->resp.hitsTotal++;
			ent->client->resp.gunstats[gun].hits++;
			ent->client->resp.hitsLocations[hitPart]++;

			if (headShot)
				ent->client->resp.gunstats[gun].headshots++;
		}
		if (!headShot) {
			ent->client->resp.streakHS = 0;
		}
	}
	else {
		gi.dprintf( "Stats_AddHit: Bad gun number!\n" );
		return;
	}
}


void Cmd_Stats_f (edict_t *targetent, char *arg)
{
/* Variables Used:                              *
* stats_shots_t - Total nr of Shots             *
* stats_shots_h - Total nr of Hits              *
* headshots     - Total nr of Headshots         *
*                                               */
	
	double	perc_hit;
	int		total, hits, i, y, len, locHits;
	char		*string, stathead[64];
	edict_t		*ent, *cl_ent;
	gunStats_t	*gun;

	if (!targetent->inuse)
		return;


	if (arg[0] != '\0') {
		if (strcmp (arg, "list") == 0) {
			gi.cprintf (targetent, PRINT_HIGH, "\n\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F\n");
			gi.cprintf (targetent, PRINT_HIGH, "PlayerID  Name                  Accuracy\n");

			for (i = 0; i < game.maxclients; i++)
			{
				cl_ent = &g_edicts[1 + i];

				if (!cl_ent->inuse || cl_ent->client->pers.mvdspec)
					continue;

				hits = total = 0;
				gun = cl_ent->client->resp.gunstats;
				for (y = 0; y < MAX_GUNSTAT; y++, gun++) {
					hits += gun->hits;
					total += gun->shots;
				}

				if (total > 0)
					perc_hit = (double)hits * 100.0 / (double)total;
				else
					perc_hit = 0.0;

				gi.cprintf (targetent, PRINT_HIGH, "   %-3i    %-16s        %6.2f\n", i, cl_ent->client->pers.netname, perc_hit);
			}
			gi.cprintf (targetent, PRINT_HIGH, "\n  Use \"stats <PlayerID>\" for\n  individual stats\n\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F\n");
			return;
		}

		ent = LookupPlayer(targetent, gi.args(), true, true);
		if (!ent)
			return;

	} else {
		ent = targetent;
	}

	// Global Stats:
	hits = total = 0;
	gun = ent->client->resp.gunstats;
	for (y = 0; y < MAX_GUNSTAT; y++, gun++) {
		hits += gun->hits;
		total += gun->shots;
	}

	sprintf(stathead, "\n\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F Statistics for %s \x9D", ent->client->pers.netname);
	len = strlen(stathead);
	for (i = len; i < 55; i++) {
		stathead[i] = '\x9E';
	}
	stathead[i] = 0;
	strcat(stathead, "\x9F\n");

	gi.cprintf (targetent, PRINT_HIGH, "%s", stathead);

	if (!total) {
		gi.cprintf (targetent, PRINT_HIGH, "\n  Player has not fired a shot.\n");
		gi.cprintf (targetent, PRINT_HIGH, "\n\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F\n\n");
		return;
	}

	gi.cprintf (targetent, PRINT_HIGH, "Weapon            Accuracy Hits/Shots Kills Headshots\n");		

	gun = ent->client->resp.gunstats;
	for (y = 0; y < MAX_GUNSTAT; y++, gun++) {

		if (gun->shots <= 0)
			continue;

		switch (y) {
		case MOD_MK23:
			string = "Pistol";
			break;
		case MOD_DUAL:
			string = "Dual Pistols";
			break;
		case MOD_KNIFE:
			string = "Slashing Knife";
			break;
		case MOD_KNIFE_THROWN:
			string = "Throwing Knife";
			break;
		case MOD_M4:
			string = "M4 Assault Rifle";
			break;
		case MOD_MP5:
			string = "MP5 Submachinegun";
			break;
		case MOD_SNIPER:
			string = "Sniper Rifle";
			break;
		case MOD_HC:
			string = "Handcannon";
			break;
		case MOD_M3:
			string = "M3 Shotgun";
			break;
		default:
			string = "Unknown Weapon";
			break;
		}

		perc_hit = (double)gun->hits * 100.0 / (double)gun->shots; // Percentage of shots that hit
		gi.cprintf( targetent, PRINT_HIGH, "%-17s  %6.2f  %4i/%-4i  %3i   %5i\n",
			string, perc_hit, gun->hits, gun->shots, gun->kills, gun->headshots );
	}

	gi.cprintf (targetent, PRINT_HIGH, "\n\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F\n");


	// Final Part
	gi.cprintf (targetent, PRINT_HIGH, "Location                          Hits     (%%)\n");		

	for (y = 0; y < LOC_MAX; y++) {
		locHits = ent->client->resp.hitsLocations[y];

		if (locHits <= 0)
			continue;

		switch (y) {
		case LOC_HDAM:
			string = "Head";
			break;
		case LOC_CDAM:
			string = "Chest";
			break;
		case LOC_SDAM:
			string = "Stomach";
			break;
		case LOC_LDAM:
			string = "Legs";
			break;
		case LOC_KVLR_HELMET:
			string = "Kevlar Helmet";
			break;
		case LOC_KVLR_VEST:
			string = "Kevlar Vest";
			break;
		case LOC_NO:
			string = "Spread (Shotgun/Handcannon)";
			break;
		default:
			string = "Unknown";
			break;
		}

		perc_hit = (double)locHits * 100.0 / (double)hits;
		gi.cprintf( targetent, PRINT_HIGH, "%-27s %10i  (%6.2f)\n", string, locHits, perc_hit );
	}
	gi.cprintf (targetent, PRINT_HIGH, "\n");

	if (total > 0)
		perc_hit = (double)hits * 100.0 / (double)total;
	else
		perc_hit = 0.0;

	gi.cprintf (targetent, PRINT_HIGH, "Average Accuracy:                         %.2f\n", perc_hit); // Average
	gi.cprintf (targetent, PRINT_HIGH, "\n\x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F\n\n");
	gi.cprintf(targetent, PRINT_HIGH, "Highest streaks:  kills: %d headshots: %d\n", ent->client->resp.streakKillsHighest, ent->client->resp.streakHSHighest);
}

void A_ScoreboardEndLevel (edict_t * ent, edict_t * killer)
{
	char string[2048];
	gclient_t *sortedClients[MAX_CLIENTS], *cl;
	int maxsize = 1000, i, line_y;
	int totalClients, secs, shots;
	double accuracy, fpm;
	int totalplayers[TEAM_TOP] = {0};
	int totalscore[TEAM_TOP] = {0};
	int name_pos[TEAM_TOP] = {0};

	totalClients = G_SortedClients(sortedClients);

	ent->client->ps.stats[STAT_TEAM_HEADER] = level.pic_teamtag;

	for (i = 0; i < totalClients; i++) {
		cl = sortedClients[i];

		totalscore[cl->resp.team] += cl->resp.score;
		totalplayers[cl->resp.team]++;
	}

	for (i = TEAM1; i <= teamCount; i++) {
		name_pos[i] = ((20 - strlen(teams[i].name)) / 2) * 8;
		if (name_pos[TEAM1] < 0)
			name_pos[TEAM1] = 0;
	}


	if (teamCount == 3)
	{
		sprintf(string,
			// TEAM1
			"if 24 xv -80 yv 8 pic 24 endif "
			"if 22 xv -48 yv 8 pic 22 endif "
			"xv -48 yv 28 string \"%4d/%-3d\" "
			"xv 10 yv 12 num 2 26 "
			"xv %d yv 0 string \"%s\" ",
			totalscore[TEAM1], totalplayers[TEAM1], name_pos[TEAM1] - 80,
			teams[TEAM1].name);
		sprintf(string + strlen (string),
			// TEAM2
			"if 25 xv 80 yv 8 pic 25 endif "
			"if 22 xv 112 yv 8 pic 22 endif "
			"xv 112 yv 28 string \"%4d/%-3d\" "
			"xv 168 yv 12 num 2 27 "
			"xv %d yv 0 string \"%s\" ",
			totalscore[TEAM2], totalplayers[TEAM2], name_pos[TEAM2] + 80,
			teams[TEAM2].name);
		sprintf(string + strlen (string),
			// TEAM3
			"if 30 xv 240 yv 8 pic 30 endif "
			"if 22 xv 272 yv 8 pic 22 endif "
			"xv 272 yv 28 string \"%4d/%-3d\" "
			"xv 328 yv 12 num 2 31 "
			"xv %d yv 0 string \"%s\" ",
			totalscore[TEAM3], totalplayers[TEAM3], name_pos[TEAM3] + 240,
			teams[TEAM3].name);
	}
	else
	{
		sprintf (string,
			// TEAM1
			"if 24 xv 0 yv 8 pic 24 endif "
			"if 22 xv 32 yv 8 pic 22 endif "
			"xv 32 yv 28 string \"%4d/%-3d\" "
			"xv 90 yv 12 num 2 26 " "xv %d yv 0 string \"%s\" "
			// TEAM2
			"if 25 xv 160 yv 8 pic 25 endif "
			"if 22 xv 192 yv 8 pic 22 endif "
			"xv 192 yv 28 string \"%4d/%-3d\" "
			"xv 248 yv 12 num 2 27 "
			"xv %d yv 0 string \"%s\" ",
			totalscore[TEAM1], totalplayers[TEAM1], name_pos[TEAM1],
			teams[TEAM1].name, totalscore[TEAM2], totalplayers[TEAM2],
			name_pos[TEAM2] + 160, teams[TEAM2].name);
	}

	line_y = 56;
	sprintf(string + strlen (string),
		"xv 0 yv 40 string2 \"Frags Player          Shots   Acc   FPM \" "
		"xv 0 yv 48 string2 \"\x9D\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9F\" ");

//        strcpy (string, "xv 0 yv 32 string2 \"Frags Player          Time Ping Damage Kills\" "
//                "xv 0 yv 40 string2 \"\x9D\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9F \x9D\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9E\x9F \x9D\x9E\x9E\x9E\x9F\" ");
  /*
     {
     strcpy (string, "xv 0 yv 32 string2 \"Player          Time Ping\" "
     "xv 0 yv 40 string2 \"--------------- ---- ----\" ");
     }
     else
     {
     strcpy (string, "xv 0 yv 32 string2 \"Frags Player          Time Ping Damage Kills\" "
     "xv 0 yv 40 string2 \"----- --------------- ---- ---- ------ -----\" ");
     }
   */
  // AQ2:TNG END

	for (i = 0; i < totalClients; i++)
	{
		cl = sortedClients[i];

		if (!cl->resp.team)
			continue;

		shots = min( cl->resp.shotsTotal, 9999 );

		if (shots)
			accuracy = (double)cl->resp.hitsTotal * 100.0 / (double)cl->resp.shotsTotal;
		else
			accuracy = 0;

		secs = (level.framenum - cl->resp.enterframe) / HZ;
		if (secs > 0)
			fpm = (double)cl->resp.score * 60.0 / (double)secs;
		else
			fpm = 0.0;

		sprintf(string + strlen(string),
			"yv %d string \"%5d %-15s  %4d %5.1f  %4.1f\" ",
			line_y,
			cl->resp.score,
			cl->pers.netname, shots, accuracy, fpm );
		
		line_y += 8;

		if (strlen (string) > (maxsize - 100) && i < (totalClients - 2))
		{
			sprintf(string + strlen (string),
				"yv %d string \"..and %d more\" ",
				line_y, (totalClients - i - 1) );
			line_y += 8;
			break;
		}
	}
	
	if (strlen(string) > 1023)	// for debugging...
	{
		gi.dprintf("Warning: scoreboard string neared or exceeded max length\nDump:\n%s\n---\n", string);
		string[1023] = '\0';
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

void Cmd_Statmode_f(edict_t* ent)
{
	int i;
	char stuff[32], *arg;


	// Ignore if there is no argument.
	arg = gi.argv(1);
	if (!arg || !arg[0])
		return;

	// Numerical
	i = atoi (arg);

	if (i > 2 || i < 0) {
		gi.dprintf("Warning: stat_mode set to %i by %s\n", i, ent->client->pers.netname);

		// Force the old mode if it is valid else force 0
		if (ent->client->resp.stat_mode > 0 && ent->client->resp.stat_mode < 3)
			sprintf(stuff, "set stat_mode \"%i\"\n", ent->client->resp.stat_mode);
		else
			sprintf(stuff, "set stat_mode \"0\"\n");
	} else {
		sprintf(stuff, "set stat_mode \"%i\"\n", i);
		ent->client->resp.stat_mode = i;
	}
	stuffcmd(ent, stuff);
}

/*
==================
High Scores List

Inspired and based on the original code by skullernet from OpenFFA
==================
*/


typedef struct {
    int nb_lines;
    char **lines;
    char path[1];
} load_file_t;

q_printf(1, 2)
static load_file_t *G_LoadFile(const char *fmt, ...)
{
    char path[MAX_OSPATH];
    size_t pathlen;
    va_list argptr;
    int err = 0;

    va_start(argptr, fmt);
    pathlen = Q_vsnprintf(path, sizeof(path), fmt, argptr);
    va_end(argptr);

    if (pathlen >= sizeof(path)) {
        err = ENAMETOOLONG;
        goto fail0;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        err = errno;
        goto fail0;
    }

    Q_STATBUF st;
    if (os_fstat(os_fileno(fp), &st)) {
        err = errno;
        goto fail1;
    }

    if (st.st_size >= INT_MAX / sizeof(char *)) {
        err = EFBIG;
        goto fail1;
    }

    char *buf = gi.TagMalloc(st.st_size + 1, TAG_GAME);

    if (!fread(buf, st.st_size, 1, fp)) {
        err = EIO;
        goto fail2;
    }

    buf[st.st_size] = 0;

    load_file_t *f = gi.TagMalloc(sizeof(*f) + pathlen, TAG_GAME);
    f->nb_lines = 0;
    f->lines = NULL;
    memcpy(f->path, path, pathlen + 1);

    int max_lines = 0;
    while (1) {
        char *p = strchr(buf, '\n');
        if (p) {
            if (p > buf && *(p - 1) == '\r')
                *(p - 1) = 0;
            *p = 0;
        }

        if (f->nb_lines == max_lines) {
            void *tmp = f->lines;
            f->lines = gi.TagMalloc(sizeof(char *) * (max_lines += 32), TAG_GAME);
            if (tmp) {
                memcpy(f->lines, tmp, sizeof(char *) * f->nb_lines);
                gi.TagFree(tmp);
            }
        }
        f->lines[f->nb_lines++] = buf;

        if (!p)
            break;
        buf = p + 1;
    }

    fclose(fp);
    return f;

fail2:
    gi.TagFree(buf);
fail1:
    fclose(fp);
fail0:
    gi.dprintf("Couldn't load '%s': %s\n", path, strerror(err));
    return NULL;
}

static void G_FreeFile(load_file_t *f)
{
    gi.TagFree(f->lines[0]);
    gi.TagFree(f->lines);
    gi.TagFree(f);
}

static int G_CreatePath(char *path)
{
    char *p;
    int ret;

    // skip leading slash(es)
    for (p = path; *p == '/'; p++)
        ;

    for (; *p; p++) {
        if (*p == '/') {
            // create the directory
            *p = 0;
            ret = os_mkdir(path);
            *p = '/';
            if (ret == -1 && errno != EEXIST)
                return -1;
        }
    }
    return 0;
}

static int ScoreCmp(const void *p1, const void *p2)
{
    highscore_t *a = (highscore_t *)p1;
    highscore_t *b = (highscore_t *)p2;

    if (a->score > b->score) {
        return -1;
    }
    if (a->score < b->score) {
        return 1;
    }
    if (a->time > b->time) {
        return -1;
    }
    if (a->time < b->time) {
        return 1;
    }
    return 0;
}

#define SD(s)   *s ? "/" : "", s

static void G_SaveScores(void)
{
    char path[MAX_OSPATH];
    highscore_t *s;
    FILE *fp;
    int i;
    size_t len;

    len = Q_snprintf(path, sizeof(path), "%s%s%s/%s/%s.txt",
                     GAMEVERSION, SD(g_highscores_dir->string), gm->string, level.mapname);
    if (len >= sizeof(path)) {
        return;
    }

    G_CreatePath(path);

    fp = fopen(path, "w");
    if (!fp) {
        gi.dprintf("Couldn't open '%s': %s\n", path, strerror(errno));
        return;
    }

    for (i = 0; i < level.numscores; i++) {
        s = &level.scores[i];
        fprintf(fp, "\"%s\" %d %f %f %lu\n",
                s->name, s->score, s->fragsper, s->accuracy, (unsigned long)s->time);
    }

    fclose(fp);
}

static int G_PlayerCmp(const void *p1, const void *p2)
{
    gclient_t *a = *(gclient_t * const *)p1;
    gclient_t *b = *(gclient_t * const *)p2;

    int r = b->resp.score - a->resp.score;
    if (!r)
        r = a->resp.deaths - b->resp.deaths;
    if (!r)
        r = (byte *)a - (byte *)b;
    return r;
}

int G_CalcRanks(gclient_t **ranks)
{
    int i, total;

    // sort the clients by score, then by eff
    total = 0;
    for (i = 0; i < game.maxclients; i++) {
        if (game.clients[i].pers.connected == true) {
            if (ranks) {
                ranks[total] = &game.clients[i];
            }
            total++;
        }
    }

    if (ranks) {
        qsort(ranks, total, sizeof(gclient_t *), G_PlayerCmp);
    }

    return total;
}

void G_RegisterScore(void)
{
    gclient_t    *ranks[MAX_CLIENTS];
    gclient_t    *c;
    highscore_t  *s;
    int total;
    int score;
	int sec;
	float accuracy, fragsper;

    total = G_CalcRanks(ranks);
    if (!total) {
        return;
    }

	if (teamplay->value && game.roundNum == 0){
		gi.dprintf("No rounds were played, so no highscore will be recorded\n");
		return; // No rounds were played, so skip
	}

    // grab our champion
    c = ranks[0];

	// Just straight up score
	score = c->resp.score;

	// Calculate FPR, if mode is teamplay, else FPH
	if (teamplay->value && game.roundNum > 0){
		fragsper = c->resp.score / game.roundNum;
	} else {
		sec = (level.framenum - c->resp.enterframe) / HZ;
		if (!sec)
            sec = 1;
		fragsper = c->resp.score * 3600 / sec;
	}

	int shots = min(c->resp.shotsTotal, 9999 );
	if (shots)
			accuracy = (double)c->resp.hitsTotal * 100.0 / (double)c->resp.shotsTotal;
		else
			accuracy = 0;

    if (score < 1) {
        return; // do not record bogus results
    }

    if (level.numscores < MAX_HIGH_SCORES) {
        s = &level.scores[level.numscores++];
    } else {
        s = &level.scores[ level.numscores - 1 ];
        if (score < s->score) {
            return; // result not impressive enough
        }
    }

    strcpy(s->name, c->pers.netname);
    s->score = score;
	s->fragsper = fragsper;
	s->accuracy = accuracy;
    time(&s->time);

    level.record = s->time;

    qsort(level.scores, level.numscores, sizeof(highscore_t), ScoreCmp);

    gi.dprintf("Added highscore entry for %s with %d score\n",
               c->pers.netname, score);

	CenterPrintAll(va("New Highscore for map %s: %s with %i score and %.2f%% accuracy", level.mapname, c->pers.netname, score, accuracy));
    G_SaveScores();
}

void G_LoadScores(void)
{
    char *token;
    const char *data;
    highscore_t *s;
    load_file_t *f;
    int i;

    f = G_LoadFile("%s%s%s/%s/%s.txt", GAMEVERSION, SD(g_highscores_dir->string), gm->string, level.mapname);
    if (!f) {
		gi.dprintf("No high scores file loaded for %s\n", level.mapname);
        return;
    }

    for (i = 0; i < f->nb_lines && level.numscores < MAX_HIGH_SCORES; i++) {
        data = f->lines[i];

        if (data[0] == '#' || data[0] == '/') {
            continue;
        }

        token = COM_Parse(&data);
        if (!*token) {
            continue;
        }

        s = &level.scores[level.numscores++];
        Q_strlcpy(s->name, token, sizeof(s->name));

        token = COM_Parse(&data);
        s->score = strtoul(token, NULL, 10);

		token = COM_Parse(&data);
		s->fragsper = strtoul(token, NULL, 10);  // Load fragsper value

		token = COM_Parse(&data);
		s->accuracy = strtoul(token, NULL, 10);  // Load accuracy value


        token = COM_Parse(&data);
        s->time = strtoul(token, NULL, 10);
    }

    qsort(level.scores, level.numscores, sizeof(highscore_t), ScoreCmp);

    gi.dprintf("Loaded %d scores from '%s'\n", level.numscores, f->path);

    G_FreeFile(f);
}


#if USE_AQTION

#ifndef NO_BOTS
/*
=================
Bot Check
=================
*/
void StatBotCheck(void)
{
	int i;
    for (i = 0; i < num_players; i++)
    {
        if (players[i]->is_bot)
        {
            game.ai_ent_found = true;
            if (stat_logs->value) {
                gi.dprintf("Bot detected, forcing stat_logs off\n");
                gi.cvar_forceset(stat_logs->name, "0");    // Turn off stat collection
            }
            return;
        }
    }
    game.ai_ent_found = false;
}
#endif

cvar_t* logfile_name;
void Write_Stats(const char* msg, ...)
{
	va_list	argptr;
	char	stat_cpy[1024];
	char	logpath[MAX_QPATH];
	FILE* 	f;

	va_start(argptr, msg);
	vsprintf(stat_cpy, msg, argptr);
	va_end(argptr);

	logfile_name = gi.cvar("logfile_name", "", CVAR_NOSET);
	sprintf(logpath, "action/logs/%s.stats", logfile_name->string);

	if ((f = fopen(logpath, "a")) != NULL)
	{
		fprintf(f, "%s", stat_cpy);
		fclose(f);
	}
	else
		gi.dprintf("Error writing to %s.stats\n", logfile_name->string);

}

/*
==================
LogKill
=================
*/
void LogKill(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int mod;
	int loc;
	int chosenItem = 0;
	int gametime = 0;
	int roundNum;
	int eventtime;
	int vt = 0; //Default victim team is 0 (no team)
	int kt = 0; //Default killer team is 0 (no team)
	int ttk = 0; //Default TTK (time to kill) is 0
	int vl = 0; //Default victimleader is 0, 1 if leader
	int kl = 0; //Default killerleader is 0, 1 if leader
	char msg[1024]; // Whole stat line in JSON format
	char v[24]; // Victim's Steam ID
	char vn[128]; // Victim's name
	char vip[24]; // Victim's IP:port
	char vd[24]; // Victim's Discord ID
	char *vi; // Victim's IP (without port)
	char vloc[18]; // Victim's location
	char k[24]; // Killer's Steam ID
	char kn[128]; // Killer's name
	char kip[24]; // Killer's IP:port
	char kd[24]; // Killer's Discord ID
	char *ki; // Killer's IP (without port)
	char kloc[18]; // Killer's location

	// Check if there's an AI bot in the game, if so, do nothing
	if (game.ai_ent_found) {
		return;
	}

	// Only record stats if there's more than one opponent
    if (gameSettings & GS_DEATHMATCH) // Only check if in DM
    {
        int oc = 0; // Opponent count
        int i;
        for (i = 0; i < game.maxclients; i++)
        {
            // If player is connected and not spectating, add them as an opponent
            if (game.clients[i].pers.connected && game.clients[i].pers.spectator == false)
            {
                if (++oc > 1) // Two or more opponents are active, so log kills
                    break;
            }
        }
        if (oc == 1) // Only one opponent active, so don't log kills
            return;
    }

	if (esp->value){
		if (IS_LEADER(self))
			vl = 1;
		if (IS_LEADER(attacker))
			kl = 1;
	}

	if ((team_round_going && !in_warmup) || (gameSettings & GS_DEATHMATCH)) // If round is active OR if deathmatch
	{
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		loc = locOfDeath;

		if (gameSettings & GS_TEAMPLAY) // Define these if the game is teamplay
		{
			vt = self->client->resp.team;
			kt = attacker->client->resp.team;
			ttk = current_round_length / 10;
		}
		
		Q_strncpyz(v, Info_ValueForKey(self->client->pers.userinfo, "steamid"), sizeof(v));
		Q_strncpyz(vn, Info_ValueForKey(self->client->pers.userinfo, "name"), sizeof(vn));
		Q_strncpyz(vip, Info_ValueForKey(self->client->pers.userinfo, "ip"), sizeof(vip));
		Q_strncpyz(vd, Info_ValueForKey(self->client->pers.userinfo, "cl_discord_id"), sizeof(vd));
		Q_strncpyz(k, Info_ValueForKey(attacker->client->pers.userinfo, "steamid"), sizeof(k));
		Q_strncpyz(kn, Info_ValueForKey(attacker->client->pers.userinfo, "name"), sizeof(kn));
		Q_strncpyz(kip, Info_ValueForKey(attacker->client->pers.userinfo, "ip"), sizeof(kip));
		Q_strncpyz(kd, Info_ValueForKey(attacker->client->pers.userinfo, "cl_discord_id"), sizeof(kd));

		// Separate IP from Port
		char* portSeperator = ":";
		vi = strtok(vip, portSeperator);
		ki = strtok(kip, portSeperator);

		gametime = level.matchTime;
		eventtime = (int)time(NULL);
		roundNum = game.roundNum + 1;

		// Location data
		sprintf(vloc, "%5.0f,%5.0f,%5.0f", self->s.origin[0], self->s.origin[1], self->s.origin[2]);
		sprintf(kloc, "%5.0f,%5.0f,%5.0f", attacker->s.origin[0], attacker->s.origin[1], attacker->s.origin[2]);

		// Item identifier, taking item kit mode into account
		if (!item_kit_mode->value) {
			chosenItem = attacker->client->pers.chosenItem->typeNum;
		} else {
			if (attacker->client->pers.chosenItem->typeNum == KEV_NUM) {
				chosenItem = KEV_NUM;
			} else {
				// Commando Kit
				if (attacker->client->pers.chosenItem->typeNum == BAND_NUM &&
					attacker->client->pers.chosenItem2->typeNum == HELM_NUM ) {
						chosenItem = C_KIT_NUM;
				// Stealth Kit
				} else if (attacker->client->pers.chosenItem->typeNum == SLIP_NUM &&
						attacker->client->pers.chosenItem2->typeNum == SIL_NUM ) {
						chosenItem = S_KIT_NUM;
				// Assassin Kit
				} else if (attacker->client->pers.chosenItem->typeNum == LASER_NUM &&
						attacker->client->pers.chosenItem2->typeNum == SIL_NUM ) {
						chosenItem = A_KIT_NUM;	
				}
			}
		}

		Q_snprintf(
			msg, sizeof(msg),
			"{\"frag\":{\"sid\":\"%s\",\"mid\":\"%s\",\"v\":\"%s\",\"vn\":\"%s\",\"vi\":\"%s\",\"vt\":%i,\"vl\":%i,\"k\":\"%s\",\"kn\":\"%s\",\"ki\":\"%s\",\"kt\":%i,\"kl\":%i,\"w\":%i,\"i\":%i,\"l\":%i,\"ks\":%i,\"gm\":%i,\"gmf\":%i,\"ttk\":%d,\"t\":%d,\"gt\":%d,\"m\":\"%s\",\"r\":%i,\"vd\":\"%s\",\"kd\":\"%s\",\"vloc\":\"%s\",\"kloc\":\"%s\"}}\n",
			server_id->string,
			game.matchid,
			v,
			vn,
			vi,
			vt,
			vl,
			k,
			kn,
			ki,
			kt,
			kl,
			mod,
			chosenItem,
			loc,
			attacker->client->resp.streakKills + 1,
			Gamemode(),
			Gamemodeflag(),
			ttk,
			eventtime,
			gametime,
			level.mapname,
			roundNum,
			vd,
			kd,
			vloc,
			kloc
		);
		Write_Stats(msg);
	}
}

/*
==================
LogWorldKill
=================
*/
void LogWorldKill(edict_t *self)
{
	int mod;
	int loc = 16;
	int gametime = 0;
	int roundNum;
	int eventtime;
	int vt = 0; //Default victim team is 0 (no team)
	int ttk = 0; //Default TTK (time to kill) is 0
	int vl = 0; //Is victim leader? 0 = false, 1 = true
	char msg[1024];
	char v[24];
	char vn[128];
	char vip[24];
	char vd[24];
	char *vi;
	char vloc[18];

	// Check if there's an AI bot in the game, if so, do nothing
	if (game.ai_ent_found) {
		return;
	}

	// Only record stats if there's more than one opponent
    if (gameSettings & GS_DEATHMATCH) // Only check if in DM
    {
        int oc = 0; // Opponent count
        int i;
        for (i = 0; i < game.maxclients; i++)
        {
            // If player is connected and not spectating, add them as an opponent
            if (game.clients[i].pers.connected && game.clients[i].pers.spectator == false)
            {
                if (++oc > 1) // Two or more opponents are active, so log kills
                    break;
            }
        }
        if (oc == 1) // Only one opponent active, so don't log kills
            return;
    }

	if (esp->value && IS_LEADER(self)){
		vl = 1;
	}

	if ((team_round_going && !in_warmup) || (gameSettings & GS_DEATHMATCH)) // If round is active OR if deathmatch
	{
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		loc = locOfDeath;

		if (gameSettings & GS_TEAMPLAY) // Define these if the game is teamplay, else default to 0
		{
			vt = self->client->resp.team;
			ttk = current_round_length / 10;
		}
		
		Q_strncpyz(v, Info_ValueForKey(self->client->pers.userinfo, "steamid"), sizeof(v));
		Q_strncpyz(vn, Info_ValueForKey(self->client->pers.userinfo, "name"), sizeof(vn));
		Q_strncpyz(vip, Info_ValueForKey(self->client->pers.userinfo, "ip"), sizeof(vip));
		Q_strncpyz(vd, Info_ValueForKey(self->client->pers.userinfo, "cl_discord_id"), sizeof(vd));

		// Separate IP from Port
		char* portSeperator = ":";
		vi = strtok(vip, portSeperator);

		gametime = level.matchTime;
		eventtime = (int)time(NULL);
		roundNum = game.roundNum + 1;

		// Location data
		sprintf(vloc, "%5.0f,%5.0f,%5.0f", self->s.origin[0], self->s.origin[1], self->s.origin[2]);

		Q_snprintf(
			msg, sizeof(msg),
			"{\"frag\":{\"sid\":\"%s\",\"mid\":\"%s\",\"v\":\"%s\",\"vn\":\"%s\",\"vi\":\"%s\",\"vt\":%i,\"vl\":%i,\"k\":\"%s\",\"kn\":\"%s\",\"ki\":\"%s\",\"kt\":%i,\"kl\":%i,\"w\":%i,\"i\":%i,\"l\":%i,\"ks\":%i,\"gm\":%i,\"gmf\":%i,\"ttk\":%d,\"t\":%d,\"gt\":%d,\"m\":\"%s\",\"r\":%i,\"vd\":\"%s\",\"kd\":\"%s\",\"vloc\":\"%s\",\"kloc\":\"%s\"}}\n",
			server_id->string,
			game.matchid,
			v,
			vn,
			vi,
			vt,
			vl,
			v,
			vn,
			vi,
			vt,
			vl,
			mod,
			16, // No attacker for world death, setting to 16 as a 'dummy' value
			loc,
			0, // No killstreak for world death, setting to 0 permanently as we aren't tracking world death kill streaks
			Gamemode(),
			Gamemodeflag(),
			ttk,
			eventtime,
			gametime,
			level.mapname,
			roundNum,
			vd,
			vd,
			vloc,
			vloc
		);
		Write_Stats(msg);
	}
}

/*
==================
LogCapture
=================
*/
void LogCapture(edict_t *capturer)
{
	int eventtime;
	char msg[1024];
	eventtime = (int)time(NULL);
	int caps, capstreak;
	int ttc = current_round_length / 10; // TimeToCapture (how many seconds in the round)

	// Check if there's an AI bot in the game, if so, do nothing
	if (game.ai_ent_found) {
		return;
	}

	int mode = Gamemode();
	switch (mode) {
		case GM_CTF:
			caps = capturer->client->resp.ctf_caps;
			capstreak = capturer->client->resp.ctf_capstreak;
			break;
		case GM_ESCORT_THE_VIP:
			caps = capturer->client->resp.esp_caps;
			capstreak = capturer->client->resp.esp_capstreak;
			break;
		case GM_DOMINATION:
			caps = capturer->client->resp.dom_caps;
			capstreak = capturer->client->resp.dom_capstreak;
			break;
		default:
			// Safety measure in case something isn't right
			caps = 0;
			capstreak = 0;
			break;
	}

	// (matchid,capturer,capturername,team,gamemode,gamemodeflag,cap,capstreak)
	// (mid,steamid,cn,t,team,gm,gmf,c,cs,ttc)

	Q_snprintf(
		msg, sizeof(msg),
		"{\"capture\":{\"mid\":\"%s\",\"steamid\":\"%s\",\"cn\":\"%s\",\"t\":\"%d\",\"team\":\"%i\",\"gm\":%i,\"gmf\":%i,\"c\":%i,\"cs\":%i,\"ttc\":%i}}\n",
		game.matchid,
		capturer->client->pers.steamid,
		capturer->client->pers.netname,
		eventtime,
		capturer->client->resp.team,
		Gamemode(),
		Gamemodeflag(),
		caps,
		capstreak,
		ttc
	);
	Write_Stats(msg);
}

/*
==================
LogMatch
=================
*/
void LogMatch(void)
{
	int eventtime;
	char msg[1024];
	int t1 = teams[TEAM1].score;
	int t2 = teams[TEAM2].score;
	int t3 = teams[TEAM3].score;
	eventtime = (int)time(NULL);

	// Check if there's an AI bot in the game, if so, do nothing
	if (game.ai_ent_found) {
		return;
	}

	// Check for scoreless teamplay, don't log
	if (teamplay->value && t1 == 0 && t2 == 0 && t3 == 0) {
		return;
	}

	Q_snprintf(
		msg, sizeof(msg),
		"{\"gamematch\":{\"mid\":\"%s\",\"sid\":\"%s\",\"t\":\"%d\",\"m\":\"%s\",\"gm\":%i,\"gmf\":%i,\"t1\":%i,\"t2\":%i,\"t3\":%i}}\n",
		game.matchid,
		server_id->string,
		eventtime,
		level.mapname,
		Gamemode(),
		Gamemodeflag(),
		t1,
		t2,
		t3
	);
	Write_Stats(msg);
}

/*
==================
LogAward
=================
*/
void LogAward(edict_t *ent, int award)
{
	int gametime = 0;
	int eventtime;
	char msg[1024];
	int mod;
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;

	gametime = level.matchTime;
	eventtime = (int)time(NULL);

	// Check if there's an AI bot in the game, if so, do nothing
	if (game.ai_ent_found) {
		return;
	}

	Q_snprintf(
		msg, sizeof(msg),
		"{\"award\":{\"sid\":\"%s\",\"mid\":\"%s\",\"t\":\"%d\",\"gt\":\"%d\",\"a\":%i,\"k\":\"%s\",\"w\":%i,\"d\":\"%s\"}}\n",
		server_id->string,
		game.matchid,
		eventtime,
		gametime,
		award,
		ent->client->pers.steamid,
		mod,
		ent->client->pers.discordid
	);
	Write_Stats(msg);
}

/*
==================
WriteLogEndMatchStats

This function is called from LogEndMatchStats because we're writing
real clients and ghost clients, to reduce code duplication.
=================
*/

void WriteLogEndMatchStats(gclient_t *cl)
{
	int secs, shots;
	double accuracy, fpm;
	char msg[1024];

	shots = min( cl->resp.shotsTotal, 9999 );
	secs = (level.framenum - cl->resp.enterframe) / HZ;

	if (shots)
			accuracy = (double)cl->resp.hitsTotal * 100.0 / (double)cl->resp.shotsTotal;
		else
			accuracy = 0;
		if (secs > 0)
			fpm = (double)cl->resp.score * 60.0 / (double)secs;
		else
			fpm = 0.0;

	Q_snprintf(
			msg, sizeof(msg),
			"{\"matchstats\":{\"sid\":\"%s\",\"mid\":\"%s\",\"s\":\"%s\",\"sc\":%i,\"sh\":%i,\"a\":%f,\"f\":%f,\"dd\":%i,\"d\":%i,\"k\":%i,\"ctfc\":%i,\"ctfcs\":%i,\"ht\":%i,\"tk\":%i,\"t\":%i,\"hks\":%i,\"hhs\":%i,\"dis\":\"%s\",\"pt\":%i,\"hlh\":%i,\"hlc\":%i,\"hls\":%i,\"hll\":%i,\"hlkh\":%i,\"hlkv\":%i,\"hln\":%i,\"gss1\":%i,\"gss2\":%i,\"gss3\":%i,\"gss4\":%i,\"gss5\":%i,\"gss6\":%i,\"gss7\":%i,\"gss8\":%i,\"gss9\":%i,\"gss13\":%i,\"gss14\":%i,\"gss35\":%i,\"gsh1\":%i,\"gsh2\":%i,\"gsh3\":%i,\"gsh4\":%i,\"gsh5\":%i,\"gsh6\":%i,\"gsh7\":%i,\"gsh8\":%i,\"gsh9\":%i,\"gsh13\":%i,\"gsh14\":%i,\"gsh35\":%i,\"gshs1\":%i,\"gshs2\":%i,\"gshs3\":%i,\"gshs4\":%i,\"gshs5\":%i,\"gshs6\":%i,\"gshs7\":%i,\"gshs8\":%i,\"gshs9\":%i,\"gshs13\":%i,\"gshs14\":%i,\"gshs35\":%i,\"gsk1\":%i,\"gsk2\":%i,\"gsk3\":%i,\"gsk4\":%i,\"gsk5\":%i,\"gsk6\":%i,\"gsk7\":%i,\"gsk8\":%i,\"gsk9\":%i,\"gsk13\":%i,\"gsk14\":%i,\"gsk35\":%i,\"gsd1\":%i,\"gsd2\":%i,\"gsd3\":%i,\"gsd4\":%i,\"gsd5\":%i,\"gsd6\":%i,\"gsd7\":%i,\"gsd8\":%i,\"gsd9\":%i,\"gsd13\":%i,\"gsd14\":%i,\"gsd35\":%i,\"ecdc\":%i,\"ecs\":%i,\"ecsb\":%i,\"elfc\":%i,\"elks\":%i,\"elksb\":%i,\"elpc\":%i}}\n",
			server_id->string,
			game.matchid,
			cl->pers.steamid,
			cl->resp.score,
			shots,
			accuracy,
			fpm,
			cl->resp.damage_dealt,
			cl->resp.deaths,
			cl->resp.kills,
			cl->resp.ctf_caps,
			cl->resp.ctf_capstreak,
			cl->resp.hitsTotal,
			cl->resp.team_kills,
			cl->resp.team,
			cl->resp.streakKillsHighest,
			cl->resp.streakHSHighest,
			cl->pers.discordid,
			secs,
			cl->resp.hitsLocations[LOC_HDAM],
			cl->resp.hitsLocations[LOC_CDAM],
			cl->resp.hitsLocations[LOC_SDAM],
			cl->resp.hitsLocations[LOC_LDAM],
			cl->resp.hitsLocations[LOC_KVLR_HELMET],
			cl->resp.hitsLocations[LOC_KVLR_VEST],
			cl->resp.hitsLocations[LOC_NO],
			cl->resp.gunstats[MOD_MK23].shots,
			cl->resp.gunstats[MOD_MP5].shots,
			cl->resp.gunstats[MOD_M4].shots,
			cl->resp.gunstats[MOD_M3].shots,
			cl->resp.gunstats[MOD_HC].shots,
			cl->resp.gunstats[MOD_SNIPER].shots,
			cl->resp.gunstats[MOD_DUAL].shots,
			cl->resp.gunstats[MOD_KNIFE].shots,
			cl->resp.gunstats[MOD_KNIFE_THROWN].shots,
			cl->resp.gunstats[MOD_HG_SPLASH].shots,
			cl->resp.gunstats[MOD_PUNCH].shots,
			cl->resp.gunstats[MOD_KICK].shots,
			cl->resp.gunstats[MOD_MK23].hits,
			cl->resp.gunstats[MOD_MP5].hits,
			cl->resp.gunstats[MOD_M4].hits,
			cl->resp.gunstats[MOD_M3].hits,
			cl->resp.gunstats[MOD_HC].hits,
			cl->resp.gunstats[MOD_SNIPER].hits,
			cl->resp.gunstats[MOD_DUAL].hits,
			cl->resp.gunstats[MOD_KNIFE].hits,
			cl->resp.gunstats[MOD_KNIFE_THROWN].hits,
			cl->resp.gunstats[MOD_HG_SPLASH].hits,
			cl->resp.gunstats[MOD_PUNCH].hits,
			cl->resp.gunstats[MOD_KICK].hits,
			cl->resp.gunstats[MOD_MK23].headshots,
			cl->resp.gunstats[MOD_MP5].headshots,
			cl->resp.gunstats[MOD_M4].headshots,
			cl->resp.gunstats[MOD_M3].headshots,
			cl->resp.gunstats[MOD_HC].headshots,
			cl->resp.gunstats[MOD_SNIPER].headshots,
			cl->resp.gunstats[MOD_DUAL].headshots,
			cl->resp.gunstats[MOD_KNIFE].headshots,
			cl->resp.gunstats[MOD_KNIFE_THROWN].headshots,
			cl->resp.gunstats[MOD_HG_SPLASH].headshots,
			cl->resp.gunstats[MOD_PUNCH].headshots,
			cl->resp.gunstats[MOD_KICK].headshots,
			cl->resp.gunstats[MOD_MK23].kills,
			cl->resp.gunstats[MOD_MP5].kills,
			cl->resp.gunstats[MOD_M4].kills,
			cl->resp.gunstats[MOD_M3].kills,
			cl->resp.gunstats[MOD_HC].kills,
			cl->resp.gunstats[MOD_SNIPER].kills,
			cl->resp.gunstats[MOD_DUAL].kills,
			cl->resp.gunstats[MOD_KNIFE].kills,
			cl->resp.gunstats[MOD_KNIFE_THROWN].kills,
			cl->resp.gunstats[MOD_HG_SPLASH].kills,
			cl->resp.gunstats[MOD_PUNCH].kills,
			cl->resp.gunstats[MOD_KICK].kills,
			cl->resp.gunstats[MOD_MK23].damage,
			cl->resp.gunstats[MOD_MP5].damage,
			cl->resp.gunstats[MOD_M4].damage,
			cl->resp.gunstats[MOD_M3].damage,
			cl->resp.gunstats[MOD_HC].damage,
			cl->resp.gunstats[MOD_SNIPER].damage,
			cl->resp.gunstats[MOD_DUAL].damage,
			cl->resp.gunstats[MOD_KNIFE].damage,
			cl->resp.gunstats[MOD_KNIFE_THROWN].damage,
			cl->resp.gunstats[MOD_HG_SPLASH].damage,
			cl->resp.gunstats[MOD_PUNCH].damage,
			cl->resp.gunstats[MOD_KICK].damage,
			cl->resp.esp_capdefendercount,
			cl->resp.esp_capstreak,
			cl->resp.esp_capstreakbest,
			cl->resp.esp_leaderfragcount,
			cl->resp.esp_leaderkillstreak,
			cl->resp.esp_leaderkillstreakbest,
			cl->resp.esp_leaderprotectcount
	);
	Write_Stats(msg);
}


/*
=================
LogEndMatchStats
=================
*/
void LogEndMatchStats(void)
{
	int i;
	gclient_t *sortedClients[MAX_CLIENTS], *cl;
	int totalClients;

	totalClients = G_SortedClients(sortedClients);

	// Check if there's an AI bot in the game, if so, do nothing
	if (game.ai_ent_found) {
		return;
	}

	// Write out the stats for each client still connected to the server
	for (i = 0; i < totalClients; i++){
		cl = sortedClients[i];

		//gi.dprintf("Writing real stats for %s\n", cl->pers.netname);
		WriteLogEndMatchStats(cl);
	}

	// Write out the stats for each ghost client
	if (!use_ghosts->value || num_ghost_players == 0) {
		// Ghostbusters got em all
		return;
	} else {
		gghost_t *ghost = NULL;
		edict_t *ent = NULL;
		gclient_t *ghlient = NULL;
		
		// Initialize the ghost client
		ent = G_Spawn();
		ent->client = gi.TagMalloc(sizeof(gclient_t), TAG_LEVEL);
			if (!ent->client) {
				gi.error("%s: Couldn't allocate client memory\n", __FUNCTION__);
				return;
			}
		ghlient = ent->client;

		for (i = 0, ghost = ghost_players; i < num_ghost_players; i++, ghost++) {
			//gi.dprintf("I found %i ghosts, Writing ghost stats for %s\n", num_ghost_players, ghost->netname);

			// Copy ghost data to this dummy entity
			strcpy(ghlient->pers.ip, ghost->ip);
			strcpy(ghlient->pers.netname, ghost->netname);
			strcpy(ghlient->pers.steamid, ghost->steamid);
			strcpy(ghlient->pers.discordid, ghost->discordid);

			ghlient->resp.enterframe = ghost->enterframe;
			ghlient->resp.score = ghost->score;
			ghlient->resp.kills = ghost->kills;
			ghlient->resp.deaths = ghost->deaths;
			ghlient->resp.damage_dealt = ghost->damage_dealt;
			ghlient->resp.ctf_caps = ghost->ctf_caps;
			ghlient->resp.ctf_capstreak = ghost->ctf_capstreak;
			ghlient->resp.team_kills = ghost->team_kills;
			ghlient->resp.streakKillsHighest = ghost->streakKillsHighest;
			ghlient->resp.streakHSHighest = ghost->streakHSHighest;
			ghlient->resp.shotsTotal = ghost->shotsTotal;
			ghlient->resp.hitsTotal = ghost->hitsTotal;

			if (teamplay->value && ghost->team)
				ghlient->resp.team = ghost->team;

			// Copy all gunstats and hit locations to this dummy entity
			memcpy(ghlient->resp.hitsLocations, ghost->hitsLocations, sizeof(ghlient->resp.hitsLocations));
			memcpy(ghlient->resp.gunstats, ghost->gunstats, sizeof(ghlient->resp.gunstats));

		WriteLogEndMatchStats(ghlient);
		}
		// Loop is over, set this to zero
		num_ghost_players = 0;
		// Be free, little dummy entity
		G_FreeEdict(ent);
	}
}
#endif
