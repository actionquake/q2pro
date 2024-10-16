#include "../g_local.h"

// Math helper functions, utilities and other useful math based tools


// Functionally this works the same as #define VectorCompare(), with the exception of an epsilon
// Compare two vectors and return true if they are the same within a certain epsilon
qboolean BOTLIB_VectorCompare(vec3_t v1, vec3_t v2, const float epsilon)
{
	for (int i = 0; i < 3; i++)
	{
		if (fabs(v1[i] - v2[i]) > epsilon)
		{
			return false;
		}
	}
	return true;
}

// Originally from Quake 2 Rerelease source code
// Returns true if two boxes intersect
bool BOTLIB_BoxIntersection(vec3_t amins, vec3_t amaxs, vec3_t bmins, vec3_t bmaxs)
{
	return	amins[0] <= bmaxs[0] && amaxs[0] >= bmins[0] &&
		amins[1] <= bmaxs[1] && amaxs[1] >= bmins[1] &&
		amins[2] <= bmaxs[2] && amaxs[2] >= bmins[2];
}


qboolean BOTLIB_Infront(edict_t* self, edict_t* other, float amount)
{
	vec3_t vec = { 0.f,0.f,0.f };
	float dot = 0.f;
	vec3_t forward = { 0.f,0.f,0.f };

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(other->s.origin, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (amount == 0)
		amount = 0.3f;

	if (dot > amount)
		return true;

	return false;
}

// Test if bot is facing toward an origin
qboolean BOTLIB_OriginInfront(edict_t* self, vec3_t origin, float amount)
{
	vec3_t vec = { 0.f,0.f,0.f };
	float dot = 0.f;
	vec3_t forward = { 0.f,0.f,0.f };

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(origin, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (amount == 0)
		amount = 0.3f;

	if (dot > amount)
		return true;

	return false;
}

// Test if bot is walking toward an origin
qboolean BOTLIB_MovingToward(edict_t* self, vec3_t origin, float amount)
{
	vec3_t vec = { 0.f,0.f,0.f };
	float dot = 0.f;
	//vec3_t forward = { 0.f,0.f,0.f };

	// Get the player's direction based from their velocity
	vec3_t walkdir;
	VectorSubtract(self->s.origin, self->bot.last_position, walkdir);
	//VectorCopy(self->s.origin, self->bot.last_position);
	VectorNormalize(walkdir);
	vec3_t angle, forward, right;
	vectoangles(walkdir, angle);
	//VectorCopy(self->s.angles, angle); // Use the player's view angles (not their walk direction)
	angle[0] = 0;
	AngleVectors(angle, forward, right, NULL);


	//AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorSubtract(origin, self->s.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if (amount == 0)
		amount = 0.3f;

	if (dot > amount)
	{
		//Com_Printf("%s %s moving toward [%f]\n", __func__, self->client->pers.netname, dot);
		return true;
	}

	return false;
}


// True if [self] is aiming at [other]
// This func uses box intersection to determine if the player is aiming at the other entity
qboolean BOTLIB_IsAimingAt(edict_t* self, edict_t* other)
{
	// Project trace from the player's weapon POV
	vec3_t      start, end;
	vec3_t      forward, right;
	vec3_t      offset;
	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, self->viewheight - 8);
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	float dist = VectorDistance(self->s.origin, other->s.origin);
	VectorMA(start, dist, forward, end);

	// Adjust the mins/maxs box size based on size
	//vec3_t bmins = { -24, -24, +6 };
	//vec3_t bmaxs = { 24, 24, +10 };
	vec3_t bmins = { -32, -32, -16 };
	vec3_t bmaxs = { 32, 32, +16 };

	// Update absolute box min/max in the world
	vec3_t absmin, absmax;
	VectorAdd(end, bmins, absmin);
	VectorAdd(end, bmaxs, absmax);

	//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
	if (0) // Debug draw predicted enemy origin - blue is predicted, yellow is actual
	{
		uint32_t blue = MakeColor(0, 0, 255, 255); // Blue
		uint32_t yellow = MakeColor(255, 255, 0, 255); // Yellow
		void (*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = NULL;
		DrawBox = players[0]->client->pers.draw->DrawBox;
		players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
		DrawBox(other->s.number + (rand() % 256 + 1), other->s.origin, blue, other->mins, other->maxs, 100, false); // Player box
		DrawBox(self->s.number + (rand() % 256 + 1), end, yellow, bmins, bmaxs, 100, false); // Projectile box
	}
#endif
	//rekkie -- debug drawing -- e

	if (BOTLIB_BoxIntersection(absmin, absmax, other->absmin, other->absmax)) // Do boxes intersect?
	{
		//Com_Printf("%s %s can hit %s\n", __func__, self->client->pers.netname, other->client->pers.netname);
		return true;
	}
	return false;
}