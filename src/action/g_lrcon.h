/*
 * g_lrcon.h
 *
 * Limited Remote Console (LRCON) - Client-side admin command system
 * Allows players to claim temporary admin rights and execute restricted commands
 */

#ifndef G_LRCON_H
#define G_LRCON_H

/* Data structures defined in g_local.h:
 * - lrcon_state_t: Runtime state (claimed, claimer info)
 * - lrcon_config_t: Configuration (enabled, allowed cvars, modes)
 * - lrcon_mode_t: Server mode definition
 */

/* Main command entry point */
void Cmd_Lrcon_f(edict_t *ent);

/* Claim/Release operations */
void Lrcon_Claim(edict_t *ent);
void Lrcon_Release(edict_t *ent);

/* Admin commands (require claim) */
void Lrcon_Status(edict_t *ent);      /* Show player list */
void Lrcon_Kick(edict_t *ent);        /* Kick a player */
void Lrcon_Teamnone(edict_t *ent);    /* Remove from team */
void Lrcon_Map(edict_t *ent);         /* Change map */
void Lrcon_Softmap(edict_t *ent);     /* Soft map change (keeps state) */
void Lrcon_Mode(edict_t *ent);        /* Change server mode */
void Lrcon_Stuffcmd(edict_t *ent);    /* Send command to client(s) */
void Lrcon_Cvar(edict_t *ent, const char *cvar_name);  /* Query/set cvar */

/* Helper functions */
qboolean Lrcon_CheckClaimer(edict_t *ent);  /* Verify caller is claimer */
void Lrcon_ClearClaim(void);                /* Clear claim state */
void Lrcon_PrintHelp(edict_t *ent);         /* Display help text */

/* Config loading (in a_game.c) */
void ReadLrconConfig(void);

#endif /* G_LRCON_H */
