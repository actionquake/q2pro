

//
// Reki
// API extensions from AQTION_EXTENSION
//

#include "g_local.h"
#include "q_ghud.h"

#if AQTION_EXTENSION
//
// Reki
// Setup struct and macro for defining engine-callable entrypoints
// basically a copy-paste of the same system for game-to-engine calls,
// just in reverse
typedef struct extension_func_s
{
	char		name[MAX_QPATH];
	void*		func;
	struct extension_func_s *n;
} extension_func_t;
extension_func_t *g_extension_funcs;

// the do {} while here is a bizarre C-ism to allow for our local variable, probably not the best way to do this
#define g_addextension(ename, efunc) \
				do { \
				extension_func_t *ext = gi.TagMalloc(sizeof(extension_func_t), TAG_GAME); \
				strcpy(ext->name, ename); \
				ext->func = efunc; \
				ext->n = g_extension_funcs; \
				g_extension_funcs = ext; \
				} while (0);

//
// declare engine trap pointers, to make the compiler happy
int(*engine_Client_GetVersion)(edict_t *ent);
int(*engine_Client_GetProtocol)(edict_t *ent);

void(*engine_Ghud_ClearForClient)(edict_t *ent);
int(*engine_Ghud_NewElement)(edict_t *ent, int type);
void(*engine_Ghud_RemoveElement)(edict_t *ent, int i);
void(*engine_Ghud_SetFlags)(edict_t *ent, int i, int val);
void(*engine_Ghud_SetInt)(edict_t *ent, int i, int val);
void(*engine_Ghud_SetText)(edict_t *ent, int i, char *text);
void(*engine_Ghud_SetPosition)(edict_t *ent, int i, int x, int y, int z);
void(*engine_Ghud_SetAnchor)(edict_t *ent, int i, float x, float y);
void(*engine_Ghud_SetColor)(edict_t *ent, int i, int r, int g, int b, int a);
void(*engine_Ghud_SetSize)(edict_t *ent, int i, int x, int y);

void(*engine_CvarSync_Set)(int index, const char *name, const char *val);

// botlib
bsp_t* (*SV_BSP)(void);
nav_t* (*CS_NAV)(void);
debug_draw_t* (*Draw)(void);
void (*SV_BotUpdateInfo)(char* name, int ping, int score);
void (*SV_BotConnect)(char* name);
void (*SV_BotDisconnect)(char* name);
void (*SV_BotClearClients)(void);


//
// optional new entrypoints the engine may want to call
edict_t *xerp_ent;
trace_t q_gameabi XERP_trace(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end)
{
	return gi.trace(start, mins, maxs, end, xerp_ent, MASK_PLAYERSOLID);
}

int G_customizeentityforclient(edict_t *clent, edict_t *ent, entity_state_t *state)
{
	if (!clent) // don't touch mvds
	{
		if (!strcmp(ent->classname, "ind_arrow"))
			return false;
		return true;
	}

	// first check visibility masks
	if (!(max(1, ent->dimension_visible) & max(1, clent->client->dimension_observe)))
		return false;
	
	if (ent->client) // client specific changes
	{
		if ((int)use_newirvision->value)
		{
			// don't show teammates in irvision
			if (teamplay->value && (ent->client->resp.team == clent->client->resp.team))
				state->renderfx &= ~RF_IR_VISIBLE;
		}
	}

	if (!strcmp(ent->classname, "ind_arrow"))
	{
		if (clent == ent->owner || ent->owner == clent->client->chase_target)
			return false;

		if (!clent->client->pers.cl_indicators) // never show if client doesn't want them
			return false;

		if (!teamplay->value) // don't use indicators in non-teamplay
			return false;

		// Espionage allows indicators for leaders if set to 2
		if (esp->value && use_indicators->value == 2 && esp_showleader->value && clent->client->resp.team) {
			// Quad is a blue glow, pent is a red glow
			if (clent->client->resp.team == TEAM1 && IS_LEADER(clent))
				ent->s.effects = EF_PENT;
			else if (clent->client->resp.team == TEAM2 && IS_LEADER(clent))
				ent->s.effects = EF_QUAD;
		}
		if ((use_indicators->value == 2 && clent->client->resp.team) || (clent->client->resp.team && clent->client->pers.cl_indicators != 2)) // disallow indicators for players in use_indicators 2, and don't use them for players unless cl_indicators 2
			return false;

		trace_t trace;
		vec3_t view_pos, arrow_pos, vnull;
		VectorClear(vnull);

		VectorCopy(ent->owner->s.origin, arrow_pos);
		arrow_pos[2] += ent->owner->maxs[2] - 4;

		VectorCopy(clent->s.origin, view_pos);
		view_pos[2] += clent->viewheight;
		
		trace = gi.trace(view_pos, vnull, vnull, arrow_pos, ent->owner, CONTENTS_SOLID);
		if (trace.fraction < 1)
			return false;

		VectorCopy(clent->client->v_angle, state->angles);
	}

	// objective arrow in Espionage
	if (!strcmp(ent->classname, "ind_arrow_objective"))
	{
		if (clent == ent->owner || ent->owner == clent->client->chase_target) // if the objective marker is on the viewer, don't send it
			return false;

		if (!clent->client->pers.cl_indicators) // never show if client doesn't want them
			return false;

		if (use_indicators->value <= 0) // if the server disallows them, don't send
			return false;

		trace_t trace;
		vec3_t view_pos, arrow_pos, vnull;
		VectorClear(vnull);

		VectorCopy(ent->owner->s.origin, arrow_pos);
		arrow_pos[2] += ent->owner->maxs[2] - 4;

		VectorCopy(clent->s.origin, view_pos);
		view_pos[2] += clent->viewheight;
		
		//if (0) // this stops it from being show through walls
		//{
		trace = gi.trace(view_pos, vnull, vnull, arrow_pos, ent->owner, CONTENTS_SOLID);
		if (trace.fraction < 1)
			return false;
		//}

		VectorCopy(clent->client->v_angle, state->angles);
	}

	// extrapolation, if we want that kind of thing (client and server both want it, client not a spectator)
	if (clent->client->pers.cl_xerp && use_xerp->value && clent->solid != SOLID_NOT)
	{
		if (ent->client) // extrapolate clients
		{
			if (sv_antilag_interp->value) // don't extrapolate when our antilag accounts for lerp
				return true;

			float xerp_amount = FRAMETIME;
			if (!sv_antilag->value) // we don't add extra xerp with antilag
				xerp_amount += clent->client->ping / 1000;

			if (clent->client->pers.cl_xerp == 2)
				xerp_amount = min(xerp_amount, FRAMETIME/2);
			else
				xerp_amount = min(xerp_amount, 0.25);

			pmove_t pm;
			xerp_ent = ent;
			memcpy(&pm.s, &ent->client->ps.pmove, sizeof(pmove_state_t));

			pm.trace = XERP_trace;
			pm.pointcontents = gi.pointcontents;
			pm.s.pm_type = PM_NORMAL;

			pm.cmd = ent->client->cmd_last; // just assume client is gonna keep pressing the same buttons for next frame
			// we need to reign in A/D spamming at low velocities
			if ((VectorLength(pm.s.velocity)/8) < 280)
			{
				pm.cmd.sidemove = 0;
				pm.cmd.forwardmove = 0;
			}

			pm.cmd.msec = xerp_amount * 1000;

			gi.Pmove(&pm);

			VectorScale(pm.s.origin, 0.125, state->origin);
		}
		else // extrapolate other physics entities
		{
			if (ent->movetype)
			{
				float xerp_amount = FRAMETIME;
				xerp_amount += clent->client->ping / 1000;

				if (clent->client->pers.cl_xerp == 2)
					xerp_amount = min(xerp_amount, FRAMETIME / 2);
				else
					xerp_amount = min(xerp_amount, 0.25);

				vec3_t start, end, velocity;
				VectorCopy(state->origin, start);
				VectorCopy(ent->velocity, velocity);

				switch (ent->movetype)
				{
				case MOVETYPE_BOUNCE:
				case MOVETYPE_TOSS:
					velocity[2] -= ent->gravity * sv_gravity->value * xerp_amount;
				case MOVETYPE_FLY:
					VectorMA(start, xerp_amount, velocity, end);
					break;
				default:
					return true;
				}

				trace_t trace;
				trace = gi.trace(start, ent->mins, ent->maxs, end, ent, ent->clipmask ? ent->clipmask : MASK_SOLID);
				VectorCopy(trace.endpos, state->origin);
			}
		}
	}

	return true;
}

void G_CvarSync_Updated(int index, edict_t *clent)
{
	gclient_t *client = clent->client;

	if (!client) // uh... this shouldn't happen
		return;
	
	const char *val = client->cl_cvar[index];
	int val_i = atoi(val);
	int val_f = atof(val);

	switch (index)
	{
		case clcvar_cl_antilag:;
			int antilag_value = client->pers.antilag_optout;
			if (atoi(val) > 0)
				client->pers.antilag_optout = qfalse;
			else if (atoi(val) <= 0)
				client->pers.antilag_optout = qtrue;

			if (sv_antilag->value && antilag_value != client->pers.antilag_optout)
				gi.cprintf(clent, PRINT_MEDIUM, "YOUR CL_ANTILAG IS NOW SET TO %i\n", !client->pers.antilag_optout);
			break;

		case clcvar_cl_xerp:
			client->pers.cl_xerp = val_i;
			break;

		case clcvar_cl_indicators:
			client->pers.cl_indicators = val_i;
			break;

		case clcvar_cl_spectatorhud:
			if (val_i)
				client->pers.spec_flags |= (SPECFL_SPECHUD | SPECFL_SPECHUD_NEW);
			else
				client->pers.spec_flags &= ~(SPECFL_SPECHUD | SPECFL_SPECHUD_NEW);

			if (Client_GetProtocol(clent) >= 38)
				client->pers.spec_flags &= ~SPECFL_SPECHUD; // don't use old spec hud
			break;

		case clcvar_cl_spectatorkillfeed:
			if (val_i)
				client->pers.spec_flags |= SPECFL_KILLFEED;
			else
				client->pers.spec_flags &= ~SPECFL_KILLFEED;
			break;
	}
}

void G_InitExtEntrypoints(void)
{
	g_addextension("customizeentityforclient", G_customizeentityforclient);
	g_addextension("CvarSync_Updated", G_CvarSync_Updated);
}


void* G_FetchGameExtension(char *name)
{
	Com_Printf("Game: G_FetchGameExtension for %s\n", name);
	extension_func_t *ext;
	for (ext = g_extension_funcs; ext != NULL; ext = ext->n)
	{
		if (strcmp(ext->name, name))
			continue;

		return ext->func;
	}

	Com_Printf("Game: Extension not found.\n");
	return NULL;
}

// 
// new engine functions we can call from the game

//
// client network querying
//
int Client_GetVersion(edict_t *ent)
{
	if (!engine_Client_GetVersion)
		return 0;

	return engine_Client_GetVersion(ent);
}


int Client_GetProtocol(edict_t *ent)
{
	if (!engine_Client_GetProtocol)
		return 34;

	return engine_Client_GetProtocol(ent);
}



//
// game hud
//
void Ghud_ClearForClient(edict_t *ent)
{
	if (!engine_Ghud_ClearForClient)
		return;

	engine_Ghud_ClearForClient(ent);
}

int Ghud_NewElement(edict_t *ent, int type)
{
	if (!engine_Ghud_NewElement)
		return 0;

	return engine_Ghud_NewElement(ent, type);
}

void Ghud_RemoveElement(edict_t *ent, int i)
{
	if (!engine_Ghud_RemoveElement)
		return;

	engine_Ghud_RemoveElement(ent, i);
}

void Ghud_SetFlags(edict_t *ent, int i, int val)
{
	if (!engine_Ghud_SetFlags)
		return;

	engine_Ghud_SetFlags(ent, i, val);
}

void Ghud_SetInt(edict_t *ent, int i, int val)
{
	if (!engine_Ghud_SetInt)
		return;

	engine_Ghud_SetInt(ent, i, val);
}

void Ghud_SetText(edict_t *ent, int i, char *text)
{
	if (!engine_Ghud_SetText)
		return;

	engine_Ghud_SetText(ent, i, text);
}

void Ghud_SetPosition(edict_t *ent, int i, int x, int y)
{
	if (!engine_Ghud_SetPosition)
		return;

	engine_Ghud_SetPosition(ent, i, x, y, 0);
}

void Ghud_SetPosition3D(edict_t *ent, int i, int x, int y, int z)
{
	if (!engine_Ghud_SetPosition)
		return;

	engine_Ghud_SetPosition(ent, i, x, y, z);
}

void Ghud_SetSize(edict_t *ent, int i, int x, int y)
{
	if (!engine_Ghud_SetSize)
		return;

	engine_Ghud_SetSize(ent, i, x, y);
}

void Ghud_SetAnchor(edict_t *ent, int i, float x, float y)
{
	if (!engine_Ghud_SetAnchor)
		return;

	engine_Ghud_SetAnchor(ent, i, x, y);
}

void Ghud_SetColor(edict_t *ent, int i, int r, int g, int b, int a)
{
	if (!engine_Ghud_SetColor)
		return;

	engine_Ghud_SetColor(ent, i, r, g, b, a);
}


//
// game hud abstractions,
// makes code a bit cleaner
//
int Ghud_AddIcon(edict_t *ent, int x, int y, int image, int sizex, int sizey)
{
	int index = Ghud_NewElement(ent, GHT_IMG);
	Ghud_SetPosition(ent, index, x, y);
	Ghud_SetSize(ent, index, sizex, sizey);
	Ghud_SetInt(ent, index, image);

	return index;
}


int Ghud_AddText(edict_t *ent, int x, int y, char *text)
{
	int index = Ghud_NewElement(ent, GHT_TEXT);
	Ghud_SetPosition(ent, index, x, y);
	Ghud_SetText(ent, index, text);

	return index;
}

void Ghud_SetTextFlags(edict_t *ent, int i, int uiflags)
{
	Ghud_SetSize(ent, i, (uiflags & 0xFFFF), (uiflags >> 16) & 0xFFFF);
}


int Ghud_AddNumber(edict_t *ent, int x, int y, int value)
{
	int index = Ghud_NewElement(ent, GHT_NUM);
	Ghud_SetPosition(ent, index, x, y);
	Ghud_SetInt(ent, index, value);

	return index;
}

void CvarSync_Set(int index, const char *name, const char *val)
{
	if (!engine_CvarSync_Set)
		return;

	engine_CvarSync_Set(index, name, val);
}

#endif