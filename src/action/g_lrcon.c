/*
 * g_lrcon.c
 *
 * Limited Remote Console (LRCON) - Client-side admin command system
 * Allows players to claim temporary admin rights and execute restricted commands.
 *
 * Features:
 * - Claim/release system: One player can claim admin access at a time
 * - Persistence: Claim persists across map changes and reconnects (name+IP matched)
 * - Commands: Player management, map control, cvar control, mode switching
 * - Config: INI-format configuration file for allowed cvars and modes
 * - Broadcast: All admin actions announced to all players
 */

#include "g_local.h"
#include "g_lrcon.h"

/* External declarations */
extern cvar_t *lrcon_config;
extern cvar_t *lrcon_claimer_name;
extern cvar_t *lrcon_claimer_ip;
extern int dosoft;

/*
 * Lrcon_CheckClaimer
 *
 * Verify that the caller is the current server claimer.
 * Returns true if caller has admin rights, false otherwise.
 */
qboolean Lrcon_CheckClaimer(edict_t *ent)
{
	if (!level.lrcon.claimed) {
		gi.cprintf(ent, PRINT_HIGH,
				   "Server not claimed. Use 'lrcon claim' first.\n");
		return false;
	}

	/* Check if this is the claimer (by name + IP) */
	if (Q_stricmp(ent->client->pers.netname, level.lrcon.claimer_name) != 0 ||
		Q_stricmp(ent->client->pers.ip, level.lrcon.claimer_ip) != 0) {
		gi.cprintf(ent, PRINT_HIGH,
				   "Server is claimed by %s. Only they can use admin commands.\n",
				   level.lrcon.claimer_name);
		return false;
	}

	/* Update entity pointer (in case of reconnect) */
	level.lrcon.claimer_ent = ent;
	return true;
}

/*
 * Lrcon_ClearClaim
 *
 * Clear the server claim and reset state.
 */
void Lrcon_ClearClaim(void)
{
	level.lrcon.claimed = false;
	level.lrcon.claimer_name[0] = '\0';
	level.lrcon.claimer_ip[0] = '\0';
	level.lrcon.claimer_ent = NULL;
	level.lrcon.claim_time = 0;

	gi.cvar_forceset("lrcon_claimer_name", "");
	gi.cvar_forceset("lrcon_claimer_ip", "");
}

/*
 * Lrcon_PrintHelp
 *
 * Display help text with all available commands.
 */
void Lrcon_PrintHelp(edict_t *ent)
{
	gi.cprintf(ent, PRINT_HIGH, "LRCON Commands:\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon claim             - Claim server control\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon release           - Release control\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon status            - Show player list with IPs\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon kick <id>         - Kick player\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon teamnone <id>     - Remove from team\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon map <mapname>     - Change map\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon softmap <mapname> - Change map keeping state\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon mode <mode|list>  - Change server mode\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon stuffcmd <id|all> - Send command to client(s)\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon <cvar>            - Query cvar value\n");
	gi.cprintf(ent, PRINT_HIGH, "  lrcon <cvar> <value>    - Set cvar value\n");
}

/*
 * Lrcon_Claim
 *
 * Claim server control for this player.
 */
void Lrcon_Claim(edict_t *ent)
{
	if (level.lrcon.claimed) {
		gi.cprintf(ent, PRINT_HIGH,
				   "Server already claimed by %s. Wait for them to release.\n",
				   level.lrcon.claimer_name);
		return;
	}

	level.lrcon.claimed = true;
	Q_strncpyz(level.lrcon.claimer_name, ent->client->pers.netname,
			   sizeof(level.lrcon.claimer_name));
	Q_strncpyz(level.lrcon.claimer_ip, ent->client->pers.ip,
			   sizeof(level.lrcon.claimer_ip));
	level.lrcon.claim_time = level.framenum;
	level.lrcon.claimer_ent = ent;

	gi.cprintf(ent, PRINT_HIGH, "You have claimed server control!\n");
	gi.bprintf(PRINT_HIGH, "%s claimed server control\n",
			   ent->client->pers.netname);

	/* Store in cvars for persistence across map changes */
	gi.cvar_forceset("lrcon_claimer_name", level.lrcon.claimer_name);
	gi.cvar_forceset("lrcon_claimer_ip", level.lrcon.claimer_ip);
}

/*
 * Lrcon_Release
 *
 * Release server control.
 */
void Lrcon_Release(edict_t *ent)
{
	if (!Lrcon_CheckClaimer(ent)) return;

	gi.cprintf(ent, PRINT_HIGH, "You released server control.\n");
	gi.bprintf(PRINT_HIGH, "%s released server control\n",
			   ent->client->pers.netname);

	Lrcon_ClearClaim();
}

/*
 * Lrcon_Status
 *
 * Display list of connected players with IPs.
 */
void Lrcon_Status(edict_t *ent)
{
	int i;
	edict_t *cl;

	if (!Lrcon_CheckClaimer(ent)) return;

	gi.cprintf(ent, PRINT_HIGH, "ID  Name            IP Address\n");
	gi.cprintf(ent, PRINT_HIGH, "--- --------------- ---------------\n");

	for (i = 0; i < game.maxclients; i++) {
		cl = g_edicts + 1 + i;
		if (!cl->inuse || !cl->client) continue;

		gi.cprintf(ent, PRINT_HIGH, "%-3d %-15s %s\n",
				   i, cl->client->pers.netname, cl->client->pers.ip);
	}
}

/*
 * Lrcon_Kick
 *
 * Kick a player from the server.
 */
void Lrcon_Kick(edict_t *ent)
{
	edict_t *target;

	if (!Lrcon_CheckClaimer(ent)) return;

	if (gi.argc() < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: lrcon kick <id>\n");
		return;
	}

	target = LookupPlayer(ent, gi.argv(2), true, false);
	if (!target) return;

	if (target == ent) {
		gi.cprintf(ent, PRINT_HIGH, "You cannot kick yourself\n");
		return;
	}

	gi.bprintf(PRINT_HIGH, "%s was kicked by %s\n",
			   target->client->pers.netname, ent->client->pers.netname);

	Kick_Client(target);
}

/*
 * Lrcon_Teamnone
 *
 * Remove a player from their team.
 */
void Lrcon_Teamnone(edict_t *ent)
{
	edict_t *target;

	if (!Lrcon_CheckClaimer(ent)) return;

	if (gi.argc() < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: lrcon teamnone <id>\n");
		return;
	}

	target = LookupPlayer(ent, gi.argv(2), true, false);
	if (!target) return;

	if (target->client->resp.team == NOTEAM) {
		gi.cprintf(ent, PRINT_HIGH, "%s is not on a team\n",
				   target->client->pers.netname);
		return;
	}

	gi.bprintf(PRINT_HIGH, "%s removed %s from their team\n",
			   ent->client->pers.netname, target->client->pers.netname);

	JoinTeam(target, NOTEAM, 0);
}

/*
 * Lrcon_Map
 *
 * Change to a new map.
 */
void Lrcon_Map(edict_t *ent)
{
	const char *mapname;

	if (!Lrcon_CheckClaimer(ent)) return;

	if (gi.argc() < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: lrcon map <mapname>\n");
		return;
	}

	mapname = gi.argv(2);

	gi.bprintf(PRINT_HIGH, "%s is changing map to %s\n",
			   ent->client->pers.netname, mapname);

	Q_strncpyz(level.nextmap, mapname, sizeof(level.nextmap));
	dosoft = 1;  /* Map changes via lrcon use soft map (preserves state) */
	EndDMLevel();
}

/*
 * Lrcon_Softmap
 *
 * Soft map change - changes map while keeping score/state.
 */
void Lrcon_Softmap(edict_t *ent)
{
	const char *mapname;

	if (!Lrcon_CheckClaimer(ent)) return;

	if (gi.argc() < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: lrcon softmap <mapname>\n");
		return;
	}

	mapname = gi.argv(2);

	gi.bprintf(PRINT_HIGH, "%s is soft-changing map to %s\n",
			   ent->client->pers.netname, mapname);

	/* Use existing softmap functionality from g_svcmds.c */
	Q_strncpyz(level.nextmap, mapname, sizeof(level.nextmap));
	dosoft = 1;  /* Global flag for soft map change */
	EndDMLevel();
}

/*
 * Lrcon_Mode
 *
 * Change server configuration mode.
 */
void Lrcon_Mode(edict_t *ent)
{
	const char *mode_arg;
	int i;

	if (!Lrcon_CheckClaimer(ent)) return;

	if (gi.argc() < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: lrcon mode <mode|list>\n");
		return;
	}

	mode_arg = gi.argv(2);

	if (!Q_stricmp(mode_arg, "list")) {
		gi.cprintf(ent, PRINT_HIGH, "Available modes:\n");
		for (i = 0; i < game.lrcon_config.modes_count; i++) {
			gi.cprintf(ent, PRINT_HIGH, "  %s\n",
					   game.lrcon_config.modes[i].name);
		}
		return;
	}

	/* Find and execute mode */
	for (i = 0; i < game.lrcon_config.modes_count; i++) {
		if (!Q_stricmp(mode_arg, game.lrcon_config.modes[i].name)) {
			gi.bprintf(PRINT_HIGH, "%s is changing mode to %s\n",
					   ent->client->pers.netname, mode_arg);
			gi.AddCommandString(game.lrcon_config.modes[i].command);
			gi.AddCommandString("\n");
			return;
		}
	}

	gi.cprintf(ent, PRINT_HIGH, "Unknown mode '%s'. Use 'lrcon mode list'\n",
			   mode_arg);
}

/*
 * Lrcon_Stuffcmd
 *
 * Send a command to a specific client or all clients.
 */
void Lrcon_Stuffcmd(edict_t *ent)
{
	const char *target_arg;
	const char *command;
	const char *cmd_start;
	edict_t *target;
	int i, skip_count;

	if (!Lrcon_CheckClaimer(ent)) return;

	if (gi.argc() < 4) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: lrcon stuffcmd <id|all> <command>\n");
		return;
	}

	target_arg = gi.argv(2);
	command = gi.args();

	/* Skip "stuffcmd <target>" to get the actual command */
	cmd_start = command;
	skip_count = 0;
	while (*cmd_start && skip_count < 2) {
		if (*cmd_start == ' ') skip_count++;
		cmd_start++;
	}

	if (!Q_stricmp(target_arg, "all")) {
		/* Send to all clients */
		for (i = 0; i < game.maxclients; i++) {
			target = g_edicts + 1 + i;
			if (!target->inuse || !target->client) continue;
			stuffcmd(target, va("%s\n", cmd_start));
		}
		gi.bprintf(PRINT_HIGH, "%s sent command to all players\n",
				   ent->client->pers.netname);
	} else {
		/* Send to specific client */
		target = LookupPlayer(ent, target_arg, true, false);
		if (!target) return;

		stuffcmd(target, va("%s\n", cmd_start));
		gi.bprintf(PRINT_HIGH, "%s sent command to %s\n",
				   ent->client->pers.netname, target->client->pers.netname);
	}
}

/*
 * Lrcon_Cvar
 *
 * Query or set a whitelisted cvar value.
 */
void Lrcon_Cvar(edict_t *ent, const char *cvar_name)
{
	cvar_t *var;
	int i;
	qboolean allowed = false;
	const char *new_value;

	if (!Lrcon_CheckClaimer(ent)) return;

	/* Check if cvar is whitelisted */
	for (i = 0; i < game.lrcon_config.allowed_cvars_count; i++) {
		if (!Q_stricmp(cvar_name, game.lrcon_config.allowed_cvars[i])) {
			allowed = true;
			break;
		}
	}

	if (!allowed) {
		gi.cprintf(ent, PRINT_HIGH, "Cvar '%s' is not whitelisted\n",
				   cvar_name);
		return;
	}

	var = gi.cvar(cvar_name, "", 0);

	if (gi.argc() < 3) {
		/* Query cvar value */
		gi.cprintf(ent, PRINT_HIGH, "%s = \"%s\"\n", cvar_name, var->string);
	} else {
		/* Set cvar value */
		new_value = gi.argv(2);
		gi.cvar_forceset(cvar_name, new_value);

		gi.cprintf(ent, PRINT_HIGH, "%s -> \"%s\"\n", cvar_name, new_value);
		gi.bprintf(PRINT_HIGH, "%s changed %s to \"%s\"\n",
				   ent->client->pers.netname, cvar_name, new_value);
	}
}

/*
 * Cmd_Lrcon_f
 *
 * Main LRCON command router. Dispatches to appropriate handler based on subcommand.
 */
void Cmd_Lrcon_f(edict_t *ent)
{
	const char *subcmd;

	if (!game.lrcon_config.enabled) {
		gi.cprintf(ent, PRINT_HIGH,
				   "LRCON is not enabled on this server.\n");
		return;
	}

	if (gi.argc() < 2) {
		Lrcon_PrintHelp(ent);
		return;
	}

	subcmd = gi.argv(1);

	if (!Q_stricmp(subcmd, "claim")) {
		Lrcon_Claim(ent);
	} else if (!Q_stricmp(subcmd, "release")) {
		Lrcon_Release(ent);
	} else if (!Q_stricmp(subcmd, "status")) {
		Lrcon_Status(ent);
	} else if (!Q_stricmp(subcmd, "kick")) {
		Lrcon_Kick(ent);
	} else if (!Q_stricmp(subcmd, "teamnone")) {
		Lrcon_Teamnone(ent);
	} else if (!Q_stricmp(subcmd, "map")) {
		Lrcon_Map(ent);
	} else if (!Q_stricmp(subcmd, "softmap")) {
		Lrcon_Softmap(ent);
	} else if (!Q_stricmp(subcmd, "mode")) {
		Lrcon_Mode(ent);
	} else if (!Q_stricmp(subcmd, "stuffcmd")) {
		Lrcon_Stuffcmd(ent);
	} else {
		/* Try cvar get/set */
		Lrcon_Cvar(ent, subcmd);
	}
}
