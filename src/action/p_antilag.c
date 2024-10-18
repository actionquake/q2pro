
#include "g_local.h"

cvar_t *sv_antilag;
cvar_t *sv_antilag_interp;

void antilag_update(edict_t *ent)
{
	antilag_t *state = &(ent->client->antilag_state);
	float time_stamp;

	state->seek++;
	state->curr_timestamp = level.time;
	
	time_stamp = level.time;
	if (sv_antilag_interp->value) // offset by 1 server frame to account for interpolation
		time_stamp += FRAMETIME;

	state->hist_timestamp[state->seek & ANTILAG_MASK] = time_stamp;
	VectorCopy(ent->s.origin, state->hist_origin[state->seek & ANTILAG_MASK]);
	VectorCopy(ent->mins, state->hist_mins[state->seek & ANTILAG_MASK]);
	VectorCopy(ent->maxs, state->hist_maxs[state->seek & ANTILAG_MASK]);
}


void antilag_clear(edict_t *ent)
{
	memset(&ent->client->antilag_state, 0, sizeof(antilag_t));
}


float antilag_findseek(edict_t *ent, float time_stamp)
{
	antilag_t *state = &(ent->client->antilag_state);

	int offs = 0;
	while (offs < ANTILAG_MAX)
	{
		if (state->hist_timestamp[(state->seek - offs) & ANTILAG_MASK] && state->hist_timestamp[(state->seek - offs) & ANTILAG_MASK] <= time_stamp)
		{
			if (offs == 0) {
				float corrected_leveltime = level.time - FRAMETIME;
				//need to do this because level.time gets updated after antilag_update during sv frame
				//Com_Printf("###SHOT REWIND###\n");
				//Com_Printf("hitbox curr_timestamp %f\n", state->curr_timestamp);
				//Com_Printf("antilag curr_timestamp %f\n", time_stamp);
				//Com_Printf("level time %f\n", corrected_leveltime);
				float advanced_since_svframe = state->curr_timestamp - corrected_leveltime;
				//Com_Printf("advanced_since_svframe %f ms\n", advanced_since_svframe*1000);
				if (advanced_since_svframe <= 0) return 0;

				float timestamp_since_svframe = time_stamp - corrected_leveltime;
				//Com_Printf("antilag_timestamp_since_svframe %f ms\n", timestamp_since_svframe*1000);
				if (timestamp_since_svframe <= 0) return state->seek;
				if (timestamp_since_svframe >= advanced_since_svframe) return state->seek;
				return - ((float)(state->seek) + (timestamp_since_svframe / advanced_since_svframe));
			}

			float frac = 1;
			float stamp_last = state->hist_timestamp[(state->seek - offs) & ANTILAG_MASK];
			float stamp_next = state->hist_timestamp[(state->seek - (offs - 1)) & ANTILAG_MASK];

			time_stamp -= stamp_last;
			frac = time_stamp / (stamp_next - stamp_last);

			return (float)(state->seek - offs) + frac;
		}

		offs++;
	}

	return -1;
}


void antilag_rewind_all(edict_t *ent)
{
	if (!sv_antilag->value)
		return;

	if (ent->client->pers.antilag_optout)
		return;

	float time_to_seek = ent->client->antilag_state.curr_timestamp;

	time_to_seek -= ((float)ent->client->ping) / 1000;
	if (time_to_seek < level.time - ANTILAG_REWINDCAP)
		time_to_seek = level.time - ANTILAG_REWINDCAP;

	edict_t *who;
	antilag_t *state;
	int i;
	for (i = 1; i < game.maxclients; i++)
	{
		who = g_edicts + i;
		if (!who->inuse)
			continue;

		state = &who->client->antilag_state;
		state->rewound = false;

		if (who == ent)
			continue;

		if (who->deadflag != DEAD_NO)
			continue;

		float rewind_seek = antilag_findseek(who, time_to_seek);      
		//Com_Printf("rewind seek %f\n", rewind_seek);
		if (rewind_seek == 0)
			continue;

		state->rewound = true;
		VectorCopy(who->s.origin, state->hold_origin);
		VectorCopy(who->mins, state->hold_mins);
		VectorCopy(who->maxs, state->hold_maxs);

		//Com_Printf("seek diff %f\n", (float)state->seek - rewind_seek);
		//LerpVector(state->hist_origin[((int)rewind_seek) & ANTILAG_MASK], state->hist_origin[((int)(rewind_seek+1)) & ANTILAG_MASK], rewind_seek - ((float)(int)rewind_seek), who->s.origin);
		
        int lerp_latest = 0;
		if (rewind_seek < 0) {
			lerp_latest = 1;
			rewind_seek = -rewind_seek;
		}
		float lerpfrac = rewind_seek - (int)rewind_seek;
		//Com_Printf("lerpfrac %f\n", lerpfrac);
		vec3_t prev, next;
		VectorCopy(state->hist_origin[(int)rewind_seek & ANTILAG_MASK], prev);
		if (lerp_latest) {
			VectorCopy(who->s.origin, next);
			//Com_Printf("Using current origin as next\n");
		} else {
			VectorCopy(state->hist_origin[((int)(rewind_seek+1)) & ANTILAG_MASK], next);
			//Com_Printf("Using hist_origin as next\n");
		}
		LerpVector(prev, next, lerpfrac, who->s.origin);
    
		VectorCopy(state->hist_mins[(int)rewind_seek & ANTILAG_MASK], who->mins);
		VectorCopy(state->hist_maxs[(int)rewind_seek & ANTILAG_MASK], who->maxs);

		gi.linkentity(who);
	}
}


void antilag_unmove_all(void)
{
	if (!sv_antilag->value)
		return;

	edict_t *who;
	antilag_t *state;
	int i;
	for (i = 1; i < game.maxclients; i++)
	{
		who = g_edicts + i;
		if (!who->inuse)
			continue;

		state = &who->client->antilag_state;

		if (!state->rewound)
			continue;

		state->rewound = false;
		VectorCopy(state->hold_origin, who->s.origin);
		VectorCopy(state->hold_mins, who->mins);
		VectorCopy(state->hold_maxs, who->maxs);

		gi.linkentity(who);
	}
}
