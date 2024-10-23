#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

ctf_status_t bot_ctf_status;

qboolean ctf_goal_node_warning = false;

// Does this bot have the flag?
qboolean BOTLIB_Carrying_Flag(edict_t* self)
{
	if (!ctf->value)
		return false;

	if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag2 == self)
		return true;

	if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag1 == self)
		return true;

	//if (self->client->inventory[items[FLAG_T1_NUM].index] || self->client->inventory[items[FLAG_T2_NUM].index])
	//	return true;
	return false;
}

//#undef CTF_AUTO_FLAG_RETURN_TIMEOUT
//#define CTF_AUTO_FLAG_RETURN_TIMEOUT 30

// Get the 
int BOTLIB_CTF_Get_Flag_Node(edict_t *ent)
{
	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };
	vec3_t bmins = { 0,0,0 };
	vec3_t bmaxs = { 0,0,0 };

	if (ent == NULL)
		return INVALID;

	int cloest_node_num = INVALID;
	float cloest_node_dist = 99999999;

	for (int j = 0; j < numnodes; j++)
	{
		VectorAdd(ent->s.origin, mins, bmins); // Update absolute box min/max in the world
		VectorAdd(ent->s.origin, maxs, bmaxs); // Update absolute box min/max in the world

		float dist = VectorDistance(nodes[j].origin, ent->s.origin);

		// If ent is touching a node
		//if (BOTLIB_BoxIntersection(bmins, bmaxs, nodes[j].absmin, nodes[j].absmax) || VectorDistance(nodes[j].origin, ent->s.origin) <= 128)
		if (dist <= 128)
		{
			trace_t tr = gi.trace(nodes[j].origin, tv(-16, -16, -8), tv(16, 16, 8), ent->s.origin, NULL, MASK_PLAYERSOLID);
			if (tr.fraction == 1.0)
			{
				//return nodes[j].nodenum;

				if (dist < cloest_node_dist)
				{
					cloest_node_dist = dist;
					cloest_node_num = nodes[j].nodenum;
				}
			}
		}
	}
	if (cloest_node_num != INVALID)
		return cloest_node_num;

	// If not touching a box, try searching via cloest distance to a node
	if (1)
	{
		int i;
		float closest = 99999;
		float dist;
		vec3_t v;
		trace_t tr;
		float rng;
		int node = INVALID;
		//vec3_t maxs, mins;

		//VectorCopy(self->mins, mins);
		//VectorCopy(self->maxs, maxs);
		//mins[2] += 18; // Stepsize
		//maxs[2] -= 16; // Duck a little.. 

		rng = (float)(NODE_DENSITY * NODE_DENSITY); // square range for distance comparison (eliminate sqrt)	

		for (i = 0; i < numnodes; i++)
		{
			if (nodes[i].inuse == false) continue; // Ignore nodes not in use

			//if (type == NODE_ALL || type == nodes[i].type) // check node type
			{
				// Get Height Diff
				float height = fabs(nodes[i].origin[2] - ent->s.origin[2]);
				if (height > 60) // Height difference high
					continue;

				VectorSubtract(nodes[i].origin, ent->s.origin, v); // subtract first

				dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

				if (dist < closest && dist < rng)
				{
					tr = gi.trace(ent->s.origin, tv(-16, -16, STEPSIZE), tv(16, 16, 32), nodes[i].origin, ent, MASK_PLAYERSOLID); //rekkie
					if ((tr.fraction == 1.0) ||
						((tr.fraction > 0.9) // may be blocked by the door itself!
							&& (Q_stricmp(tr.ent->classname, "func_door_rotating") == 0))
						)
					{
						node = i;
						closest = dist;
					}
				}
			}
		}
		if (node != INVALID)
			return nodes[node].nodenum;
	}

	return INVALID;
}

// Intercept flag carrier (good guy and bad guy)
// [OPTIONAL] distance: if bot is within distance
// Returns the node nearest to the carrier
int BOTLIB_InterceptFlagCarrier(edict_t* self, int team, float distance)
{
	if (distance > 0)
	{
		if (team == TEAM1)
		{
			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T2_NUM) <= distance)
			{
				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
				return bot_ctf_status.player_has_flag2->bot.current_node;
			}
			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
			{
				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
				return bot_ctf_status.player_has_flag1->bot.current_node;
			}
		}

		if (team == TEAM2)
		{
			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
			{
				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
				return bot_ctf_status.player_has_flag1->bot.current_node;
			}
			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T2_NUM) <= distance)
			{
				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
				return bot_ctf_status.player_has_flag2->bot.current_node;
			}
		}
	}
	else
	{
		if (team == TEAM1)
		{
			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false)
			{
				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
				return bot_ctf_status.player_has_flag2->bot.current_node;
			}
			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false)
			{
				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
				return bot_ctf_status.player_has_flag1->bot.current_node;
			}
		}
		if (team == TEAM2)
		{
			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false)
			{
				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
				return bot_ctf_status.player_has_flag1->bot.current_node;
			}
			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false)
			{
				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
				return bot_ctf_status.player_has_flag2->bot.current_node;
			}
		}
	}

	return INVALID;
}

// Is flag at home?
// [OPTIONAL] distance: if bot is within distance
qboolean BOTLIB_IsFlagHome(edict_t* self, int team, float distance)
{
	if (team == TEAM1 && self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag1_is_home == true)
	{
		if (distance > 0)
		{
			if (BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
			{
				//Com_Printf("%s %s RED flag is home\n", __func__, self->client->pers.netname);
				return true;
			}
		}
		else
		{
			//Com_Printf("%s %s RED flag is home\n", __func__, self->client->pers.netname);
			return true;
		}
	}
	if (team == TEAM2 && self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home == true)
	{
		if (distance > 0)
		{
			if (BOTLIB_DistanceToFlag(self, FLAG_T2_NUM) <= distance)
			{
				//Com_Printf("%s %s BLUE flag is home\n", __func__, self->client->pers.netname);
				return true;
			}
		}
		else
		{
			//Com_Printf("%s %s BLUE flag is home\n", __func__, self->client->pers.netname);
			return true;
		}
	}

	return false;
}

// Is Red or Blue flag dropped to ground?
// <team> Which team flag to check if dropped
// [OPTIONAL] if distance > 0: Checks bot is within distance
qboolean BOTLIB_IsFlagDropped(edict_t* self, int team, float distance)
{
	if (self->client->resp.team == TEAM1)
	{
		if (distance > 0)
		{
			if (team == TEAM1 && bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag1_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
			{
				//Com_Printf("%s %s our flag is dropped nearby %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM));
				return true;
			}
			if (team == TEAM2 && bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T2_NUM) <= distance)
			{
				//Com_Printf("%s %s enemy flag is dropped nearby %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM));
				return true;
			}
		}
		else
		{
			if (team == TEAM1 && bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag1_is_home == false)
			{
				//Com_Printf("%s %s our flag is dropped %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM));
				return true;
			}
			if (team == TEAM2 && bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home == false)
			{
				//Com_Printf("%s %s enemy flag is dropped %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM));
				return true;
			}
		}
	}
	else if (self->client->resp.team == TEAM2)
	{
		if (distance > 0)
		{
			if (team == TEAM2 && bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T2_NUM) <= distance)
			{
				//Com_Printf("%s %s our flag is dropped nearby %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM));
				return true;
			}
			if (team == TEAM1 && bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag1_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
			{
				//Com_Printf("%s %s enemy flag is dropped nearby %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM));
				return true;
			}
		}
		else
		{
			if (team == TEAM2 && bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home == false)
			{
				//Com_Printf("%s %s our flag is dropped %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM));
				return true;
			}
			if (team == TEAM1 && bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag1_is_home == false)
			{
				//Com_Printf("%s %s enemy flag is dropped %f\n", __func__, self->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM));
				return true;
			}
		}
	}

	return false;
}

// Returns the distance to flagType (FLAG_T1_NUM, FLAG_T2_NUM)
float BOTLIB_DistanceToFlag(edict_t* self, int flagType)
{
	// If carried by a player
	if (bot_ctf_status.player_has_flag1)
		return VectorDistance(bot_ctf_status.player_has_flag1->s.origin, self->s.origin);
	if (bot_ctf_status.player_has_flag2)
		return VectorDistance(bot_ctf_status.player_has_flag2->s.origin, self->s.origin);

	// If on ground
	if (flagType == FLAG_T1_NUM && bot_ctf_status.flag1_curr_node > INVALID)
		return VectorDistance(nodes[bot_ctf_status.flag1_curr_node].origin, self->s.origin);
	if (flagType == FLAG_T2_NUM && bot_ctf_status.flag2_curr_node > INVALID)
		return VectorDistance(nodes[bot_ctf_status.flag2_curr_node].origin, self->s.origin);

	return 9999999; // Could not find distance
}

// Gets the nearest flag
int BOTLIB_NearestFlag(edict_t *self)
{
	if (bot_ctf_status.flag1_home_node == INVALID || bot_ctf_status.flag2_home_node == INVALID)
		return 0;

	float flag1_dist = 999999999;
	float flag2_dist = 999999999;

	if (bot_ctf_status.flag1_is_dropped)
		flag1_dist = VectorDistance(nodes[bot_ctf_status.flag1_curr_node].origin, self->s.origin);
	else
		flag1_dist = VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, self->s.origin);

	if (bot_ctf_status.flag2_is_dropped)
		flag2_dist = VectorDistance(nodes[bot_ctf_status.flag2_curr_node].origin, self->s.origin);
	else
		flag2_dist = VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, self->s.origin);

	qboolean closer_to_flag1 = flag1_dist < flag1_dist ? true : false;
	if (closer_to_flag1)
		return FLAG_T1_NUM;
	else
		return FLAG_T2_NUM;

	/*
	float dist_to_flag1_home = VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, self->s.origin);
	float dist_to_flag2_home = VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, self->s.origin);
	qboolean closer_to_flag1 = dist_to_flag1_home < dist_to_flag2_home ? true : false;
	if (closer_to_flag1) 
		return FLAG_T1_NUM;
	else
		return FLAG_T2_NUM;
	*/
}

// Get the status of flags
void BOTLIB_Update_Flags_Status(void)
{
	// Check if any players are carrying a flag
	// Returns INVALID if no enemy has the flag
	bot_ctf_status.player_has_flag1 = NULL;
	bot_ctf_status.player_has_flag2 = NULL;
	for (int p = 0; p < num_players; p++)
	{
		if (players[p]->client->inventory[items[FLAG_T1_NUM].index])
			bot_ctf_status.player_has_flag1 = players[p];

		if (players[p]->client->inventory[items[FLAG_T2_NUM].index])
			bot_ctf_status.player_has_flag2 = players[p];
	}

	bot_ctf_status.flag1 = NULL;
	bot_ctf_status.flag2 = NULL;
	bot_ctf_status.flag1_is_home = true;
	bot_ctf_status.flag2_is_home = true;
	bot_ctf_status.flag1_is_dropped = false;
	bot_ctf_status.flag2_is_dropped = false;
	bot_ctf_status.flag1_is_carried = false;
	bot_ctf_status.flag2_is_carried = false;

	int base = 1 + game.maxclients + BODY_QUEUE_SIZE;
	edict_t* ent = g_edicts + base;
	for (int i = base; i < globals.num_edicts; i++, ent++)
	{
		if (ent->inuse == false) continue;
		if (!ent->classname) continue;

		// When at home:
		// Spawned:	 0 0 1 0			-- 0, 0, SOLID_TRIGGER, 0
		// Returned: 80000000 0 1 40000 -- FL_RESPAWN, 0, SOLID_TRIGGER, ITEM_TARGETS_USED
		// 
		// When picked up:
		// Picked:	 80000000 1 0 40000	-- FL_RESPAWN, SVF_NOCLIENT, SOLID_NOT, ITEM_TARGETS_USED
		// 
		// When dropped after being picked up:
		// Dropped:  80000000 1 0 40000	-- FL_RESPAWN, SVF_NOCLIENT, SOLID_NOT, ITEM_TARGETS_USED
		// Dropped:  0 0 1 10000		-- 0, 0, SOLID_TRIGGER, DROPPED_ITEM
		

		if (ent->typeNum == FLAG_T1_NUM)
		{
			//Com_Printf("%s flag FLAG_T1_NUM %x %x %d %x\n", __func__, ent->flags, ent->svflags, ent->solid, ent->spawnflags);

			bot_ctf_status.flag1 = ent;
			bot_ctf_status.flag1_curr_node = BOTLIB_CTF_Get_Flag_Node(ent);

			// Home - never touched (set once per map)
			if (ent->flags == 0 && ent->svflags == 0 && ent->solid == SOLID_TRIGGER && ent->spawnflags == 0 && bot_ctf_status.flag1_home_node == INVALID)
			{
				/*
				// Add a flag node here
				trace_t tr = gi.trace(ent->s.origin, tv(-16, -16, -24), tv(16, 16, 32), ent->s.origin, ent, MASK_PLAYERSOLID);
				int node_added = BOTLIB_AddNode(ent->s.origin, tr.plane.normal, NODE_MOVE);
				if (node_added != INVALID)
				{
					BOTLIB_LinkNodesNearbyNode(ent, node_added);
					bot_ctf_status.flag1_home_node = node_added;
					Com_Printf("%s Red flag home to node %i\n", __func__, bot_ctf_status.flag1_home_node);
				}
				*/

				bot_ctf_status.flag1_is_home = true;
				bot_ctf_status.flag1_home_node = BOTLIB_CTF_Get_Flag_Node(ent);
				//Com_Printf("%s Red flag home to node %i\n", __func__, bot_ctf_status.flag1_home_node);
			}
			// Returned
			if (ent->flags == FL_RESPAWN && ent->svflags == 0 && ent->solid == SOLID_TRIGGER && ent->spawnflags == ITEM_TARGETS_USED) 
			{
				bot_ctf_status.flag1_is_home = true;
			}

			// Picked up
			if (ent->flags == FL_RESPAWN && ent->svflags == SVF_NOCLIENT && ent->solid == SOLID_NOT && ent->spawnflags == ITEM_TARGETS_USED) 
			{
				bot_ctf_status.flag1_is_home = false;
				bot_ctf_status.flag1_is_carried = true;
				if (bot_ctf_status.flag2_home_node != INVALID && bot_ctf_status.player_has_flag1 != NULL)
					bot_ctf_status.team2_carrier_dist_to_home = VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, bot_ctf_status.player_has_flag1->s.origin);
			}
			// Dropped
			if (ent->flags == 0 && ent->svflags == 0 && ent->solid == SOLID_TRIGGER && ent->spawnflags == DROPPED_ITEM && VectorLength(ent->velocity) == 0)
			{
				bot_ctf_status.flag1_is_home = false;
				bot_ctf_status.flag1_is_dropped = true;
			}
		}

		if (ent->typeNum == FLAG_T2_NUM)
		{
			//Com_Printf("%s flag FLAG_T2_NUM %x %x %d %x\n", __func__, ent->flags, ent->svflags, ent->solid, ent->spawnflags);

			bot_ctf_status.flag2 = ent;
			bot_ctf_status.flag2_curr_node = BOTLIB_CTF_Get_Flag_Node(ent);

			// Home - never touched (set once per map)
			if (ent->flags == 0 && ent->svflags == 0 && ent->solid == SOLID_TRIGGER && ent->spawnflags == 0 && bot_ctf_status.flag2_home_node == INVALID)
			{
				/*
				// Add a flag node here
				trace_t tr = gi.trace(ent->s.origin, tv(-16, -16, -24), tv(16, 16, 32), ent->s.origin, ent, MASK_PLAYERSOLID);
				int node_added = BOTLIB_AddNode(ent->s.origin, tr.plane.normal, NODE_MOVE);
				if (node_added != INVALID)
				{
					BOTLIB_LinkNodesNearbyNode(ent, node_added);
					bot_ctf_status.flag2_home_node = node_added;
					Com_Printf("%s Blue flag home to node %i\n", __func__, bot_ctf_status.flag2_home_node);
				}
				*/

				bot_ctf_status.flag2_is_home = true;
				bot_ctf_status.flag2_home_node = BOTLIB_CTF_Get_Flag_Node(ent);
				Com_Printf("%s Blue flag home to node %i\n", __func__, bot_ctf_status.flag2_home_node);
			}
			// Returned
			if (ent->flags == FL_RESPAWN && ent->svflags == 0 && ent->solid == SOLID_TRIGGER && ent->spawnflags == ITEM_TARGETS_USED) 
			{
				bot_ctf_status.flag2_is_home = true;
			}

			// Picked
			if (ent->flags == FL_RESPAWN && ent->svflags == SVF_NOCLIENT && ent->solid == SOLID_NOT && ent->spawnflags == ITEM_TARGETS_USED)
			{
				bot_ctf_status.flag2_is_home = false;
				bot_ctf_status.flag2_is_carried = true;
				if (bot_ctf_status.flag1_home_node != INVALID && bot_ctf_status.player_has_flag2 != NULL)
					bot_ctf_status.team1_carrier_dist_to_home = VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, bot_ctf_status.player_has_flag2->s.origin);
			}
			// Dropped
			if (ent->flags == 0 && ent->svflags == 0 && ent->solid == SOLID_TRIGGER && ent->spawnflags == DROPPED_ITEM && VectorLength(ent->velocity) == 0)
			{
				bot_ctf_status.flag2_is_home = false;
				bot_ctf_status.flag2_is_dropped = true;
			}

			// Flag home moved
			if (bot_ctf_status.flag2_is_home && bot_ctf_status.flag2_home_node != BOTLIB_CTF_Get_Flag_Node(ent))
			{
				//Com_Printf("%s Blue flag home was moved to node %i\n", __func__, bot_ctf_status.flag2_home_node);
				bot_ctf_status.flag2_home_node = BOTLIB_CTF_Get_Flag_Node(ent); // Update home node
			}
		}



		//if (ent->solid == SOLID_NOT) continue; // picked up
	}
	//Com_Printf("%s\n", __func__);
}

void BOTLIB_CTF_Goals(edict_t* self)
{
	if (team_round_going == false || lights_camera_action) return; // Only allow during a real match (after LCA and before win/loss announcement)

	// Get flag
	if (self->bot.bot_ctf_state != BOT_CTF_STATE_GET_ENEMY_FLAG && 
		self->bot.bot_ctf_state != BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG &&
		self->bot.bot_ctf_state != BOT_CTF_STATE_RETURN_TEAM_FLAG)
	{
		if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag2 == NULL && bot_ctf_status.flag2_is_home)
		{
			int n = bot_ctf_status.flag2_home_node;
			if (BOTLIB_CanGotoNode(self, n, false)) // Make sure we can visit the node they're at
			{
				//Com_Printf("%s %s heading for enemy blue flag at node %i dist[%f]\n", __func__, self->client->pers.netname, n, VectorDistance(bot_ctf_status.flag2->s.origin, self->s.origin));
				self->bot.bot_ctf_state = BOT_CTF_STATE_GET_ENEMY_FLAG;
				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, n);
				return;
			}
		}
		if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag1 == NULL && bot_ctf_status.flag2_is_home)
		{
			int n = bot_ctf_status.flag1_home_node;
			if (BOTLIB_CanGotoNode(self, n, false)) // Make sure we can visit the node they're at
			{
				//Com_Printf("%s %s heading for enemy red flag at node %i dist[%f]\n", __func__, self->client->pers.netname, n, VectorDistance(bot_ctf_status.flag1->s.origin, self->s.origin));
				self->bot.bot_ctf_state = BOT_CTF_STATE_GET_ENEMY_FLAG;
				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, n);
				return;
			}
		}
	}

	// Take flag home, if home flag is home
	if (BOTLIB_Carrying_Flag(self))
	{
		if (self->bot.bot_ctf_state != BOT_CTF_STATE_CAPTURE_ENEMY_FLAG && 
			self->bot.bot_ctf_state != BOT_CTF_STATE_ATTACK_ENEMY_CARRIER && 
			self->bot.bot_ctf_state != BOT_CTF_STATE_RETURN_TEAM_FLAG &&
			self->bot.bot_ctf_state != BOT_CTF_STATE_FORCE_MOVE_TO_FLAG)
		{
			int n = INVALID;
			if (self->client->resp.team == TEAM1 && VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, self->s.origin) > 128)
				n = bot_ctf_status.flag1_home_node;
			else if (self->client->resp.team == TEAM2 && VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, self->s.origin) > 128)
				n = bot_ctf_status.flag2_home_node;

			// If team flag is dropped and nearby, don't head to home. Allow bot to pickup the dropped team flag.
			if (self->client->resp.team == TEAM1 && bot_ctf_status.flag1_is_dropped && VectorDistance(bot_ctf_status.flag1->s.origin, self->s.origin) < 768)
			{
				n = INVALID;
				//Com_Printf("%s %s abort capturing flag, red flag was dropped nearby\n", __func__, self->client->pers.netname);
			}
			if (self->client->resp.team == TEAM2 && bot_ctf_status.flag2_is_dropped && VectorDistance(bot_ctf_status.flag2->s.origin, self->s.origin) < 768)
			{
				n = INVALID;
				//Com_Printf("%s %s abort capturing flag, blue flag was dropped nearby\n", __func__, self->client->pers.netname);
			}

			// If both teams have the flag, and there's no other players/bots to go attack the flag carrier
			if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag1 && bot_connections.total_team1 <= 1)
			{
				n = INVALID;
				//Com_Printf("%s %s abort capturing flag, other team has our red flag and there's no support to go get it back\n", __func__, self->client->pers.netname);
			}
			// If both teams have the flag, and there's no other players/bots to go attack the flag carrier
			if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag2 && bot_connections.total_team2 <= 1)
			{
				n = INVALID;
				//Com_Printf("%s %s abort capturing flag, other team has our red flag and there's no support to go get it back\n", __func__, self->client->pers.netname);
			}

			//Com_Printf("%s %s goal_node[%d]  flag1_home_node[%i]  state[%d]\n", __func__, self->client->pers.netname, self->bot.goal_node, bot_ctf_status.flag1_home_node, self->state);
			//if (n != INVALID) Com_Printf("%s %s self->bot.goal_node %i  flag node %i\n", __func__, self->client->pers.netname, self->bot.goal_node, n);

			if (BOTLIB_CanGotoNode(self, n, false))
			{
				if (self->client->resp.team == TEAM1)
				{
					//Com_Printf("%s %s Taking blue flag home to node %i dist[%f]\n", __func__, self->client->pers.netname, n, VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, self->s.origin));
				}
				else
				{
					//Com_Printf("%s %s Taking red flag home to node %i dist[%f]\n", __func__, self->client->pers.netname, n, VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, self->s.origin));
				}
				self->bot.bot_ctf_state = BOT_CTF_STATE_CAPTURE_ENEMY_FLAG;
				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, n);
				return;
			}
		}
	}

	// Bot was on its way to the enemy flag, but got picked up by a team member before the bot could reach it. Therefore, try to support the flag carrrier who got it.
	if (self->bot.bot_ctf_state == BOT_CTF_STATE_GET_ENEMY_FLAG && BOTLIB_Carrying_Flag(self) == false)
	{
		if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag2)
		{
			//Com_Printf("%s %s Red flag was picked first up by another player [%s]\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname);

			// Support ally if they're close
			if (VectorDistance(self->s.origin, bot_ctf_status.player_has_flag2->s.origin) < 1024)
			{
				int n = bot_ctf_status.player_has_flag2->bot.current_node;
				if (BOTLIB_CanGotoNode(self, n, false))
				{
					//Com_Printf("%s %s supporting blue flag carrier %s at node %i\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, n);
					self->bot.bot_ctf_state = BOT_CTF_STATE_COVER_FLAG_CARRIER;
					self->bot.ctf_support_time = level.framenum + 10.0 * HZ;
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, n);
					return;
				}
			}

			//self->bot.bot_ctf_state = BOT_CTF_STATE_NONE;
			//self->bot.state = BOT_MOVE_STATE_NAV;
		}
		if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag1)
		{
			//Com_Printf("%s %s Blue flag was picked first up by another player [%s]\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname);

			// Support ally if they're close
			if (VectorDistance(self->s.origin, bot_ctf_status.player_has_flag1->s.origin) < 1024)
			{
				int n = bot_ctf_status.player_has_flag1->bot.current_node;
				if (BOTLIB_CanGotoNode(self, n, false))
				{
					//Com_Printf("%s %s supporting red flag carrier %s at node %i\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, n);
					self->bot.bot_ctf_state = BOT_CTF_STATE_COVER_FLAG_CARRIER;
					self->bot.ctf_support_time = level.framenum + 10.0 * HZ;
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, n);
					return;
				}
			}

			//self->bot.bot_ctf_state = BOT_CTF_STATE_NONE;
			//self->bot.state = BOT_MOVE_STATE_NAV;
		}
	}

	// Continue supporting flag carrier
	if (self->bot.bot_ctf_state != BOT_CTF_STATE_COVER_FLAG_CARRIER && BOTLIB_Carrying_Flag(self) == false)
	{
		if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag2 && self->bot.ctf_support_time < level.framenum)
		{
			self->bot.ctf_support_time = level.framenum + 5.0 * HZ;

			//if (bot_ctf_status.team1_carrier_dist_to_home > 512)
			//	Com_Printf("%s %s continue supporting blue flag carrier %s dist from home [%f]\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, bot_ctf_status.team1_carrier_dist_to_home);


			// Support flag carrier if they're away from the home flag
			float dist = VectorDistance(self->s.origin, bot_ctf_status.player_has_flag2->s.origin);
			if (dist > 256 && dist < 4024 && bot_ctf_status.team1_carrier_dist_to_home > 512)
			{
				int n = bot_ctf_status.player_has_flag2->bot.current_node;
				if (BOTLIB_CanGotoNode(self, n, false))
				{
					//Com_Printf("%s %s continue supporting blue flag carrier %s at node %i\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, n);
					self->bot.bot_ctf_state = BOT_CTF_STATE_COVER_FLAG_CARRIER;
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, n);
					return;
				}
			}
		}

		if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag1 && self->bot.ctf_support_time < level.framenum)
		{
			self->bot.ctf_support_time = level.framenum + 5.0 * HZ;

			//if (bot_ctf_status.team2_carrier_dist_to_home > 512)
			//	Com_Printf("%s %s continue supporting red flag carrier %s dist from home [%f]\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, bot_ctf_status.team2_carrier_dist_to_home);

			// Support flag carrier if they're around
			float dist = VectorDistance(self->s.origin, bot_ctf_status.player_has_flag1->s.origin);
			if (dist > 256 && dist < 4024 && bot_ctf_status.team1_carrier_dist_to_home > 512)
			{
				int n = bot_ctf_status.player_has_flag1->bot.current_node;
				if (BOTLIB_CanGotoNode(self, n, false))
				{
					//Com_Printf("%s %s continue supporting red flag carrier %s at node %i\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, n);
					self->bot.bot_ctf_state = BOT_CTF_STATE_COVER_FLAG_CARRIER;
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, n);
					return;
				}
			}
		}
	}



	
	// Retrieve dropped T1/T2 flags if nearby
	if (self->client->resp.team == TEAM1)
	{
		int flag_to_get = 0; // Flag that is dropped

		// If both team and ememy flag is dropped, go for closest flag
		if (bot_ctf_status.flag1_is_dropped && bot_ctf_status.flag2_is_dropped)
		{
			// If team flag is closer
			if (VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin) < VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin))
			{
				flag_to_get = FLAG_T1_NUM; // Dropped team flag
				//Com_Printf("%s %s [team] dropped red flag is closer. [%f] is less than [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin), VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin));
			}
			else
			{
				flag_to_get = FLAG_T2_NUM; // Dropped enemy flag
				//Com_Printf("%s %s [enemy] dropped blue flag is closer. [%f] is less than [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin), VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin));
			}
		}
		else if (bot_ctf_status.flag1_is_dropped) // Dropped team flag
			flag_to_get = FLAG_T1_NUM;
		else if (bot_ctf_status.flag2_is_dropped) // Dropped enemy flag
			flag_to_get = FLAG_T2_NUM;

		if (flag_to_get && self->bot.bot_ctf_state != BOT_CTF_STATE_RETURN_TEAM_FLAG
			&& self->bot.bot_ctf_state != BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG
			&& self->bot.bot_ctf_state != BOT_CTF_STATE_FORCE_MOVE_TO_FLAG)
		{
			int n = INVALID;
			if (flag_to_get == FLAG_T1_NUM)
				n = bot_ctf_status.flag1_curr_node;
			if (flag_to_get == FLAG_T2_NUM)
				n = bot_ctf_status.flag2_curr_node;

			if (n == INVALID && ctf_goal_node_warning == false){
				ctf_goal_node_warning = true;
				gi.dprintf("%s: Warning: Flag location is at an INVALID node.\n", __func__);
				return;
			}

			// Extra safety check in case ctf_goal_node_warning is true and isn't caught above
			// this is a segfault preventor
			if (n == INVALID)
				return;

			if (n != INVALID && BOTLIB_CanGotoNode(self, nodes[n].nodenum, false))
			{
				ctf_goal_node_warning = false;
				if (flag_to_get == FLAG_T1_NUM) // Dropped team flag
				{
					self->bot.bot_ctf_state = BOT_CTF_STATE_RETURN_TEAM_FLAG;
					//Com_Printf("%s %s retrieving [team] dropped red flag %s at node %d dist[%f]\n", __func__, self->client->pers.netname, bot_ctf_status.flag1->classname, nodes[n].nodenum, VectorDistance(self->s.origin, nodes[n].origin));
				}
				if (flag_to_get == FLAG_T2_NUM) // Dropped enemy flag
				{
					self->bot.bot_ctf_state = BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG;
					//Com_Printf("%s %s retrieving [enemy] dropped blue flag %s at node %d dist[%f]\n", __func__, self->client->pers.netname, bot_ctf_status.flag2->classname, nodes[n].nodenum, VectorDistance(self->s.origin, nodes[n].origin));
				}

				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, nodes[n].nodenum);
				return;
			}
		}
	}
	if (self->client->resp.team == TEAM2)
	{
		int flag_to_get = 0; // Flag that is dropped

		// If both team and ememy flag is dropped, go for closest flag
		if (bot_ctf_status.flag1_is_dropped && bot_ctf_status.flag2_is_dropped)
		{
			// If team flag is closer
			if (VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin) < VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin))
			{
				flag_to_get = FLAG_T2_NUM; // Dropped team flag
				if (bot_debug->value)
					Com_Printf("%s %s [team] dropped blue flag is closer. [%f] is less than [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin), VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin));
			}
			else
			{
				flag_to_get = FLAG_T1_NUM; // Dropped enemy flag
				if (bot_debug->value)
					Com_Printf("%s %s [enemy] dropped red flag is closer. [%f] is less than [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin), VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin));
			}
		}
		else if (bot_ctf_status.flag2_is_dropped) // Dropped team flag
			flag_to_get = FLAG_T2_NUM;
		else if (bot_ctf_status.flag1_is_dropped) // Dropped enemy flag
			flag_to_get = FLAG_T1_NUM;

		if (flag_to_get && self->bot.bot_ctf_state != BOT_CTF_STATE_RETURN_TEAM_FLAG
			&& self->bot.bot_ctf_state != BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG
			&& self->bot.bot_ctf_state != BOT_CTF_STATE_FORCE_MOVE_TO_FLAG)
		{
			int n = INVALID;
			if (flag_to_get == FLAG_T2_NUM)
				n = bot_ctf_status.flag2_curr_node;
			if (flag_to_get == FLAG_T1_NUM)
				n = bot_ctf_status.flag1_curr_node;

			if (BOTLIB_CanGotoNode(self, nodes[n].nodenum, false))
			{
				if (flag_to_get == FLAG_T2_NUM) // Dropped team flag
				{
					self->bot.bot_ctf_state = BOT_CTF_STATE_RETURN_TEAM_FLAG;
					//Com_Printf("%s %s retrieving [team] dropped blue flag %s at node %d dist[%f]\n", __func__, self->client->pers.netname, bot_ctf_status.flag2->classname, nodes[n].nodenum, VectorDistance(self->s.origin, nodes[n].origin));
				}
				if (flag_to_get == FLAG_T1_NUM) // Dropped enemy flag
				{
					self->bot.bot_ctf_state = BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG;
					//Com_Printf("%s %s retrieving [enemy] dropped red flag %s at node %d dist[%f]\n", __func__, self->client->pers.netname, bot_ctf_status.flag1->classname, nodes[n].nodenum, VectorDistance(self->s.origin, nodes[n].origin));
				}

				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, nodes[n].nodenum);
				return;
			}
		}
	}


	// When the bot is close to the flag from following nodes, flags are not always on a node exactly,
	// therefore when the bot gets close enough to the flag, force move the bot toward the flag.
	if (self->bot.goal_node == INVALID)
	{

		//if (self->bot.bot_ctf_state == BOT_CTF_STATE_GET_ENEMY_FLAG || self->bot.bot_ctf_state == BOT_CTF_STATE_GET_DROPPED_ENEMY_FLAG || self->bot.bot_ctf_state == BOT_CTF_STATE_CAPTURE_ENEMY_FLAG || self->bot.bot_ctf_state == BOT_CTF_STATE_RETURN_TEAM_FLAG)
		{
			// TEAM 1 - Walk to home flag (Capture)
			if (self->client->resp.team == TEAM1 && BOTLIB_Carrying_Flag(self) && bot_ctf_status.flag1_is_home
				&& bot_ctf_status.flag1 && VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin) < 256)
			{
				//Com_Printf("%s %s is being forced toward home red flag [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin));
				vec3_t walkdir;
				VectorSubtract(bot_ctf_status.flag1->s.origin, self->s.origin, walkdir); // Head to flag
				VectorNormalize(walkdir);
				vec3_t hordir; //horizontal direction
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
				self->bot.bi.speed = 400;
				self->bot.bot_ctf_state = BOT_CTF_STATE_FORCE_MOVE_TO_FLAG;
				return;
			}
			// TEAM 2 - Walk to home flag (Capture)
			if (self->client->resp.team == TEAM2 && BOTLIB_Carrying_Flag(self) && bot_ctf_status.flag2_is_home
				&& bot_ctf_status.flag2 && VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin) < 256)
			{
				//Com_Printf("%s %s is being forced toward home blue flag [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin));
				vec3_t walkdir;
				VectorSubtract(bot_ctf_status.flag2->s.origin, self->s.origin, walkdir); // Head to flag
				VectorNormalize(walkdir);
				vec3_t hordir; //horizontal direction
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
				self->bot.bi.speed = 400;
				self->bot.bot_ctf_state = BOT_CTF_STATE_FORCE_MOVE_TO_FLAG;
				return;
			}
			// TEAM 1 - Walk to enemy flag (Take)
			if (self->client->resp.team == TEAM1 && BOTLIB_Carrying_Flag(self) == false && bot_ctf_status.player_has_flag2 == false && bot_ctf_status.flag2_is_home
				&& bot_ctf_status.flag2 && VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin) < 256)
			{
				//Com_Printf("%s %s is being forced toward enemy home blue flag [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin));
				vec3_t walkdir;
				VectorSubtract(bot_ctf_status.flag2->s.origin, self->s.origin, walkdir); // Head to flag
				VectorNormalize(walkdir);
				vec3_t hordir; //horizontal direction
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
				self->bot.bi.speed = 400;
				self->bot.bot_ctf_state = BOT_CTF_STATE_FORCE_MOVE_TO_FLAG;
				return;
			}
			// TEAM 2 - Walk to enemy flag (Take)
			if (self->client->resp.team == TEAM2 && BOTLIB_Carrying_Flag(self) == false && bot_ctf_status.player_has_flag1 == false && bot_ctf_status.flag1_is_home
				&& bot_ctf_status.flag1 && VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin) < 256)
			{
				//Com_Printf("%s %s is being forced toward enemy home red flag [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin));
				vec3_t walkdir;
				VectorSubtract(bot_ctf_status.flag1->s.origin, self->s.origin, walkdir); // Head to flag
				VectorNormalize(walkdir);
				vec3_t hordir; //horizontal direction
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
				self->bot.bi.speed = 400;
				self->bot.bot_ctf_state = BOT_CTF_STATE_FORCE_MOVE_TO_FLAG;
				return;
			}
			// Walk to dropped RED flag
			if (bot_ctf_status.flag1_is_dropped && bot_ctf_status.flag1 && VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin) < 256)
			{
				//Com_Printf("%s %s is being forced toward dropped red flag [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin));
				vec3_t walkdir;
				VectorSubtract(bot_ctf_status.flag1->s.origin, self->s.origin, walkdir); // Head to flag
				VectorNormalize(walkdir);
				vec3_t hordir; //horizontal direction
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
				self->bot.bi.speed = 400;
				self->bot.bot_ctf_state = BOT_CTF_STATE_FORCE_MOVE_TO_FLAG;
				return;
			}
			// Walk to dropped BLUE flag
			if (bot_ctf_status.flag2_is_dropped && bot_ctf_status.flag2 && VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin) < 256)
			{
				//Com_Printf("%s %s is being forced toward dropped blue flag [%f]\n", __func__, self->client->pers.netname, VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin));
				vec3_t walkdir;
				VectorSubtract(bot_ctf_status.flag2->s.origin, self->s.origin, walkdir); // Head to flag
				VectorNormalize(walkdir);
				vec3_t hordir; //horizontal direction
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
				self->bot.bi.speed = 400;
				self->bot.bot_ctf_state = BOT_CTF_STATE_FORCE_MOVE_TO_FLAG;
				return;
			}
		}

		// Check if bot is closer to enemy flag carrier or the enemy flag at home
		int flag_to_get = 0;
		if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag1)
		{
			if (bot_ctf_status.flag2 && bot_ctf_status.flag2_is_home) // If enemy flag is home
			{
				// If we're closer to the enemy flag, go after that instead of intercepting enemy flag carrier
				if (VectorDistance(self->s.origin, bot_ctf_status.flag2->s.origin) < VectorDistance(self->s.origin, bot_ctf_status.player_has_flag1->s.origin))
				{
					flag_to_get = FLAG_T2_NUM; // Go after enemy flag
					//Com_Printf("%s %s closer to blue flag than blue enemy flag carrier %s\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname);
				}
				else
					flag_to_get = FLAG_T1_NUM; // Go after enemy carrier
			}
			else
				flag_to_get = FLAG_T1_NUM; // If both flags gone, go after enemy carrier
		}
		else if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag2)
		{
			if (bot_ctf_status.flag1 && bot_ctf_status.flag1_is_home) // If enemy flag is home
			{
				// If we're closer to the enemy flag, go after that instead of intercepting enemy flag carrier
				if (VectorDistance(self->s.origin, bot_ctf_status.flag1->s.origin) < VectorDistance(self->s.origin, bot_ctf_status.player_has_flag2->s.origin))
				{
					flag_to_get = FLAG_T1_NUM; // Go after enemy flag
					//Com_Printf("%s %s closer to red flag than red enemy flag carrier %s\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname);
				}
				else
					flag_to_get = FLAG_T2_NUM; // Go after enemy carrier
			}
			else
				flag_to_get = FLAG_T2_NUM; // If both flags gone, go after enemy carrier
		}

		// Intercept enemy flag carrier if we're closer to them than the enemy flag
		if (self->client->resp.team == TEAM1 && bot_ctf_status.player_has_flag1 && flag_to_get == FLAG_T1_NUM)
		{
			// If both teams have the flag, and there's no other players/bots to go attack the flag carrier
			qboolean force_incercept = false;
			if (BOTLIB_Carrying_Flag(self) && bot_connections.total_team1 <= 1)
			{
				force_incercept = true;
				//Com_Printf("%s %s abort capturing flag, other team has our red flag and there's no support to go get it back\n", __func__, self->client->pers.netname);
			}

			self->bot.state = BOT_MOVE_STATE_MOVE;

			float dist = 999999999;
			dist = VectorDistance(self->s.origin, bot_ctf_status.player_has_flag1->s.origin);
			if (dist > 256 && (BOTLIB_Carrying_Flag(self) == false || force_incercept))
			{
				int n = bot_ctf_status.player_has_flag1->bot.current_node; //bot_ctf_status.flag2_curr_node;
				if (BOTLIB_CanGotoNode(self, n, false)) // Make sure we can visit the node they're at
				{
					//Com_Printf("%s %s intercepting blue flag carrier %s at node %i\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, n);
					self->bot.bot_ctf_state = BOT_CTF_STATE_ATTACK_ENEMY_CARRIER;
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, n);
					return;
				}
			}
		}
		if (self->client->resp.team == TEAM2 && bot_ctf_status.player_has_flag2 && flag_to_get == FLAG_T2_NUM)
		{
			// If both teams have the flag, and there's no other players/bots to go attack the flag carrier
			qboolean force_incercept = false;
			if (BOTLIB_Carrying_Flag(self) && bot_connections.total_team2 <= 1)
			{
				force_incercept = true;
				//Com_Printf("%s %s abort capturing flag, other team has our blue flag and there's no support to go get it back\n", __func__, self->client->pers.netname);
			}

			self->bot.state = BOT_MOVE_STATE_MOVE;

			float dist = 999999999;
			dist = VectorDistance(self->s.origin, bot_ctf_status.player_has_flag2->s.origin);
			if (dist > 256 && (BOTLIB_Carrying_Flag(self) == false || force_incercept))
			{
				int n = bot_ctf_status.player_has_flag2->bot.current_node; //bot_ctf_status.flag1_curr_node;
				if (BOTLIB_CanGotoNode(self, n, false)) // Make sure we can visit the node they're at
				{
					//Com_Printf("%s %s intercepting red flag carrier %s at node %i\n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, n);
					self->bot.bot_ctf_state = BOT_CTF_STATE_ATTACK_ENEMY_CARRIER;
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, n);
					return;
				}
			}
		}

		self->bot.bot_ctf_state = BOT_CTF_STATE_NONE;
		self->bot.ctf_support_time = 0;

	}

	/*
	// Gather nearby weapons, ammo, and items
	if (self->bot.bot_ctf_state == BOT_CTF_STATE_NONE && self->bot.bot_ctf_state != BOT_CTF_STATE_GET_DROPPED_ITEMS)
	{
		int item_node = BOTLIB_GetEquipment(self);
		if (item_node != INVALID)
		{
			{
				self->bot.state = BOT_MOVE_STATE_MOVE;
				BOTLIB_SetGoal(self, nodes[item_node].nodenum);
				self->bot.bot_ctf_state == BOT_CTF_STATE_GET_DROPPED_ITEMS;
				//Com_Printf("%s %s going for %s at node %d\n", __func__, self->client->pers.netname, self->bot.get_item->classname, nodes[item_node].nodenum);
				return;
			}
		}
	}
	*/

	
}