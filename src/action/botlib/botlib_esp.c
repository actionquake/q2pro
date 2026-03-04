#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

esp_status_t bot_esp_status;

// Get the target in ETV mode
int BOTLIB_ESPGetTargetNode(edict_t *ent, edict_t* leader)
{
	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };
	vec3_t bmins = { 0,0,0 };
	vec3_t bmaxs = { 0,0,0 };

	if (ent == NULL)
		return INVALID;

	edict_t *target = NULL;

	// Reset escortcap value
	//espsettings.escortcap = false;

	// If leader is null, it means we're looking for an ETV target
	if(leader == NULL) {
		target = etvTarget;
	} else { // Target is leader
		target = leader;
	}

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

	if(leader == NULL)
		BOTLIB_Debug("%s: there are no nodes near ETV target\n", __func__);
	else
		BOTLIB_Debug("%s: there are no nodes near the leader\n", __func__);
	return INVALID;
}

// Returns the distance to leader (friendly or enemy)
float BOTLIB_DistanceToLeader(edict_t* self, edict_t* leader)
{
	if (leader == NULL)
		return 9999999;
	
	float distanceToLeader = VectorDistance(leader->s.origin, self->s.origin);

	if(distanceToLeader < 9999999)
		return distanceToLeader;
	else
		return 9999999;// Could not find distance
}

// int BOTLIB_WhereIsTheLeader(edict_t* self, edict_t* leader)
// {
// 	int myTeam = self->client->resp.team;
// 	return BOTLIB_DistanceToLeader(self, leader);

// 		if (myTeam == TEAM1)
// 		{
// 			if (BOTLIB_DistanceToLeader(self, FLAG_T2_NUM) <= distance)
// 			{
// 				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
// 				return bot_ctf_status.player_has_flag2->bot.current_node;
// 			}
// 			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
// 			{
// 				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
// 				return bot_ctf_status.player_has_flag1->bot.current_node;
// 			}
// 		}

// 		if (myTeam == TEAM2)
// 		{
// 			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T1_NUM) <= distance)
// 			{
// 				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
// 				return bot_ctf_status.player_has_flag1->bot.current_node;
// 			}
// 			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false && BOTLIB_DistanceToFlag(self, FLAG_T2_NUM) <= distance)
// 			{
// 				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
// 				return bot_ctf_status.player_has_flag2->bot.current_node;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (myTeam == TEAM1)
// 		{
// 			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false)
// 			{
// 				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
// 				return bot_ctf_status.player_has_flag2->bot.current_node;
// 			}
// 			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false)
// 			{
// 				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
// 				return bot_ctf_status.player_has_flag1->bot.current_node;
// 			}
// 		}
// 		if (myTeam == TEAM2)
// 		{
// 			if (bot_ctf_status.player_has_flag1 && bot_ctf_status.flag1_is_home == false)
// 			{
// 				//Com_Printf("%s %s intercepting RED flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag1->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T1_NUM), bot_ctf_status.player_has_flag1->bot.current_node);
// 				return bot_ctf_status.player_has_flag1->bot.current_node;
// 			}
// 			if (bot_ctf_status.player_has_flag2 && bot_ctf_status.flag2_is_home == false)
// 			{
// 				//Com_Printf("%s %s intercepting BLUE flag carrier %s dist[%f] node[%i] \n", __func__, self->client->pers.netname, bot_ctf_status.player_has_flag2->client->pers.netname, BOTLIB_DistanceToFlag(self, FLAG_T2_NUM), bot_ctf_status.player_has_flag2->bot.current_node);
// 				return bot_ctf_status.player_has_flag2->bot.current_node;
// 			}
// 		}
// 	}

// 	return INVALID;
// }



int BOTLIB_FindMyLeaderNode(edict_t* self)
{
	int myTeam = self->client->resp.team;
	edict_t* leader = teams[myTeam].leader;
	//float distance;

	if (leader != NULL && IS_ALIVE(leader)) {
		if (leader->is_bot) {  // No need to do fancy stuff if leader is a bot
			return leader->bot.current_node;
		} else {
			//distance = BOTLIB_DistanceToLeader(self, leader);
			return BOTLIB_ESPGetTargetNode(self, leader);
		}
	}
	return INVALID;
}

int BOTLIB_FindEnemyLeaderNode(edict_t* self, int teamNum)
{
	edict_t* enemy_leader = teams[teamNum].leader;

	if (IS_ALIVE(enemy_leader)) {
		if (enemy_leader->is_bot) {  // No need to do fancy stuff if leader is a bot
			return enemy_leader->bot.current_node;
		} else {
			//distance = BOTLIB_DistanceToLeader(self, leader);
			return BOTLIB_ESPGetTargetNode(self, enemy_leader);
		}
	}
	return INVALID;
}

int BOTLIB_InterceptLeader_3Team(edict_t* self)
{
    int teamTarget = -1;
    int myTeam = self->client->resp.team;
    int teams[] = {TEAM1, TEAM2, TEAM3};
    int aliveTeams[2];
    int aliveCount = 0;

    // Check which teams' leaders are alive
    for (int i = 0; i < 3; i++)
    {
        if (teams[i] != myTeam && IS_ALIVE(EspGetLeader(teams[i])))
        {
            aliveTeams[aliveCount++] = teams[i];
        }
    }

    // Determine the target team
    if (aliveCount == 2)
    {
        teamTarget = aliveTeams[rand() % 2]; // Randomize between the two alive teams
    }
    else if (aliveCount == 1)
    {
        teamTarget = aliveTeams[0]; // Only one team leader is alive
    }
    else
    {
        return INVALID; // No team leaders are alive, so no node to intercept
    }

	return BOTLIB_FindEnemyLeaderNode(self, teamTarget);
}

int BOTLIB_InterceptLeader_2Team(edict_t* self)
{
    int myTeam = self->client->resp.team;
    edict_t* leader;

    if (espsettings.esp_mode == ESPMODE_ATL) {
        if (myTeam == TEAM1) {
            leader = EspGetLeader(TEAM2);
            if (leader != NULL && IS_ALIVE(leader)) {
                return BOTLIB_FindEnemyLeaderNode(self, TEAM2);
            }
        } else {
            leader = EspGetLeader(TEAM1);
            if (leader != NULL && IS_ALIVE(leader)) {
                return BOTLIB_FindEnemyLeaderNode(self, TEAM1);
            }
        }
    } else if (espsettings.esp_mode == ESPMODE_ETV) {
        if (myTeam == TEAM2) {
            leader = EspGetLeader(TEAM1);
            if (leader != NULL && IS_ALIVE(leader)) {
                return BOTLIB_ESPGetTargetNode(self, NULL);
            }
        }
    }
    return INVALID; // No team leaders are alive, so no node to intercept
}

int BOTLIB_InterceptLeader_ETV(edict_t* self)
{
	return BOTLIB_InterceptLeader_2Team(self);
}

// Intercept enemy team leader (bad guy)
// [OPTIONAL] distance: if bot is within distance
// Returns the node nearest to the leader
int BOTLIB_InterceptEnemyLeader(edict_t* self)
{
	if (espsettings.esp_mode == ESPMODE_ATL) {
		if (use_3teams->value) {
			return BOTLIB_InterceptLeader_3Team(self);
		} else {
			return BOTLIB_InterceptLeader_2Team(self);
		}
	} else if (espsettings.esp_mode == ESPMODE_ETV) {
		return BOTLIB_InterceptLeader_ETV(self);
	} else {
		return INVALID;
	}
}

float BOTLIB_DistanceToEnemyLeader(edict_t* self, int flagType)
{
	// If target is leader
	edict_t* team1leader = teams[TEAM1].leader;
	edict_t* team2leader = teams[TEAM2].leader;
	edict_t* team3leader = teams[TEAM3].leader;

	if (use_3teams->value) {
		// Array of team leaders
		edict_t *teamLeaders[3] = {team1leader, team2leader, team3leader};
		float minDistance = 999999;
		edict_t *targetLeader = NULL;

		// Iterate over each team leader
		for (int i = 0; i < 3; i++) {
			// Skip if the leader is null or if it's the same team as self
			if (!teamLeaders[i] || self->client->resp.team == i + 1) {
				continue;
			}
			// Calculate the distance to the team leader
			float distance = VectorDistance(teamLeaders[i]->s.origin, self->s.origin);
			if (distance < minDistance) {
				minDistance = distance;
				targetLeader = teamLeaders[i];
			}
		}

		if (targetLeader) {
			// Perform the attack or any other logic here
			// attack(targetLeader);
			return minDistance;
		}
	} else { // Teamplay
		// Array of team leaders
		edict_t *teamLeaders[2] = {team1leader, team2leader};
		float minDistance = 999999;
		edict_t *targetLeader = NULL;

		// Iterate over each team leader
		for (int i = 0; i < 2; i++) {
			// Skip if the leader is null or if it's the same team as self
			if (!teamLeaders[i] || self->client->resp.team == i + 1) {
				continue;
			}
			// Calculate the distance to the team leader
			float distance = VectorDistance(teamLeaders[i]->s.origin, self->s.origin);
			if (distance < minDistance) {
				minDistance = distance;
				targetLeader = teamLeaders[i];
			}
		}

		if (targetLeader) {
			// Perform the attack or any other logic here
			// attack(targetLeader);
			return minDistance;
		}
	}

	return 9999999; // Could not find distance
}


/*

BOT Espionage Behaviors

*/

void _EspWander(edict_t* self)
{
	self->bot.bot_esp_state = BOT_ESP_WANDER;
	BOTLIB_PickLongRangeGoal(self);
	BOTLIB_Debug("%s: ATL %s is wandering\n", __func__, self->client->pers.netname);
}

qboolean _EspFlee(edict_t* self, int closestSpawnPointNode)
{
	char* debugName = "leader";

	if (!IS_LEADER(self)) {
		debugName = "crew";
	}

	self->bot.bot_esp_state = BOT_ESP_RETREAT;
	if (BOTLIB_CanVisitNode(self, closestSpawnPointNode, true, INVALID, false)) {
		BOTLIB_Debug("%s: ATL %s %s is fleeing back to %i)\n", __func__, debugName, self->client->pers.netname, closestSpawnPointNode);
		return true;
	} else {
		BOTLIB_Debug("%s: ATL %s %s could not flee back to %i, picking another goal\n", __func__, debugName, self->client->pers.netname, closestSpawnPointNode);
		BOTLIB_PickLongRangeGoal(self);
		return false;
	}
	return false; // Could not flee
}

qboolean _EspAttackLeader(edict_t* self, int enemyNode)
{
	char* debugName = "leader";

	if (!IS_LEADER(self)) {
		debugName = "crew";
	}

	self->bot.bot_esp_state = BOT_ESP_ATTACK_TARGET;
	if (BOTLIB_CanVisitNode(self, enemyNode, true, INVALID, false)) {
		BOTLIB_Debug("%s: %s %s going for enemy leader at node %i\n", __func__, debugName, self->client->pers.netname, enemyNode);
		return true;
	} else {
		BOTLIB_Debug("%s: %s %s could not attack enemy leader at %i, picking another goal\n", __func__, debugName, self->client->pers.netname, enemyNode);
		BOTLIB_PickLongRangeGoal(self);
		return false;
	}
	return false;
}

qboolean _EspAttackTarget(edict_t* self, int targetNode)
{
	self->bot.bot_esp_state = BOT_ESP_ATTACK_TARGET;
	if (BOTLIB_CanVisitNode(self, targetNode, true, INVALID, false)) {
		BOTLIB_Debug("%s: ETV Leader %s going for ETV target at node %i\n", __func__, self->client->pers.netname, targetNode);
		return true;
	} else {
		BOTLIB_Debug("%s: %s could attack ETV target at %i, picking another goal\n", __func__, self->client->pers.netname, targetNode);
		BOTLIB_PickLongRangeGoal(self);
		return false;
	}
	return false;
}

qboolean _EspDefendLeader(edict_t* self, int leaderNode)
{

	self->bot.bot_esp_state = BOT_ESP_ATTACK_TARGET;
	if (BOTLIB_CanVisitNode(self, leaderNode, true, INVALID, false)) {
		BOTLIB_Debug("%s: ATL %s crew going for enemy leader at node %i\n", __func__, self->client->pers.netname, leaderNode);
		return true;
	} else {
		BOTLIB_Debug("%s: %s could reach leader at %i, picking another goal\n", __func__, self->client->pers.netname, leaderNode);
		BOTLIB_PickLongRangeGoal(self);
		return false;
	}
	return false;
}


void BOTLIB_ESP_Goals(edict_t* self)
{
	if (!lights_camera_action && !espsettings.esp_live_round) return; // Only allow during a real match (after LCA and before win/loss announcement)

	// Team related variables
	int myTeam = self->client->resp.team;
	int totalTeammates = TotalPlayersOnTeam(myTeam);
	int aliveTeammates = TotalPlayersAliveOnTeam(myTeam);
	float percentAlive = (float)aliveTeammates / (float)totalTeammates * 100;

	// Refactor this so it is only calculated once, at the beginning of each round
	int closestSpawnPointNode = teamplay_spawn_node[myTeam];
	// End Refactor

	// Relevant node info
	int targetNode = BOTLIB_ESPGetTargetNode(self, NULL);
	int enemyNode = BOTLIB_InterceptEnemyLeader(self);
	int leaderNode = BOTLIB_FindMyLeaderNode(self);

	// Logic time: Discerning between leader vs non-leader
	// TODO: Better logic later on when we utilize bot.skill!
	if (IS_LEADER(self))
		goto bot_leader_think;
	else
		goto bot_crew_think;

// Logic splits here, between ATL and ETV modes
bot_leader_think:
	if (espsettings.esp_mode == ESPMODE_ATL) {
		// If percentage of teammates alive is < 50% then :
		if (percentAlive < 50) {
			// 50% chance to go for the enemy leader
			if (rand() % 2 == 0) {
				if (enemyNode != INVALID) {
					_EspAttackLeader(self, enemyNode);
					return;
				} else {
					// Can't find enemy leader?  Wander time
					_EspWander(self);
					return;
				}
			} else {
				// 50% chance to flee (run back to original spawnpoint)
				if (chosenSpawnpoint[myTeam] != NULL) {
					_EspFlee(self, closestSpawnPointNode);
				}
			}
		} else {
			// 33% chance to go for the enemy leader
			if (rand() % 3 == 0) {
				if (enemyNode != INVALID) {
					_EspAttackLeader(self, enemyNode);
					return;
				}
			} else if (rand() % 3 == 1) {
				// 33% chance to flee
				if (chosenSpawnpoint[myTeam] != NULL) {
					_EspFlee(self, closestSpawnPointNode);
					return;
				}
			} else if (rand() % 3 == 2) {
				// 33% chance to wander
				if (chosenSpawnpoint[myTeam] != NULL) {
					_EspWander(self);
					return;
				}
			}
		}
	} else if (espsettings.esp_mode == ESPMODE_ETV && myTeam == TEAM1) {
		// If percentage of teammates alive is < 50% then :
		if (percentAlive < 50) {
			// 50% chance to go for the ETV target
			if (rand() % 2 == 0) {
				if (targetNode != INVALID) {
					_EspAttackTarget(self, targetNode);
					return;
				}
			} else {
				// 50% chance to flee
				if (chosenSpawnpoint[myTeam] != NULL) {
					_EspFlee(self, closestSpawnPointNode);
					return;
				} else {
					_EspWander(self);
					return;
				}
			}
		} else { // More than 50% alive, let's attack the target!
			if (targetNode != INVALID) {
				_EspAttackTarget(self, targetNode);
				return;
			}
		}
		// We shouldn't get here but in case we do, do something
			_EspWander(self);
			return;
	}

// Main difference between crew and leader is that the crew are
// expendable, protect the leader at all costs!
bot_crew_think:
	if (espsettings.esp_mode == ESPMODE_ATL) {
		// If percentage of teammates alive is < 50% then :
		if (percentAlive < 50) {
			// 50% chance to stay with team leader
			if (rand() % 2 == 0) {
				if (leaderNode != INVALID) {
					_EspDefendLeader(self, leaderNode);
					return;
				}
			} else {
				// 50% chance to attack enemy leader
				if (enemyNode != INVALID) {
					_EspAttackLeader(self, enemyNode);
				}
			}
		} else { // More than 50% alive, let's figure something out
			// 33% chance to stay with team leader
			if (rand() % 3 == 0) {
				if (leaderNode != INVALID) {
					_EspDefendLeader(self, leaderNode);
					return;
				}
			} else if (rand() % 3 == 1) {
				// 33% chance to attack enemy leader
				if (enemyNode != INVALID) {
					_EspAttackLeader(self, enemyNode);
					return;
				}
			} else if (rand() % 3 == 2) {
				// 33% chance to wander
				_EspWander(self);
				return;
			}
		}
	} else if (espsettings.esp_mode == ESPMODE_ETV && myTeam == TEAM1) {
		// If percentage of teammates alive is < 50% then :
		if (percentAlive < 50) {
			// 50% chance to go for the ETV target
			if (rand() % 2 == 0) {
				if (targetNode != INVALID) {
					_EspAttackTarget(self, targetNode);
					return;
				}
			} else{
				// 50% chance to stay with team leader
				if (leaderNode != INVALID) {
					_EspDefendLeader(self, leaderNode);
					return;
				}
			}
		} else { // More than 50% alive, let's attack the target!
			if (targetNode != INVALID) {
				_EspAttackTarget(self, targetNode);
				return;
			} else {
				_EspWander(self);
				return;
			}
		}
	} else if (espsettings.esp_mode == ESPMODE_ETV && myTeam == TEAM2) {
		// If percentage of teammates alive is < 50% then :
		if (percentAlive < 50) {
			// 50% chance to go for the ETV target
			if (rand() % 2 == 0) {
				if (targetNode != INVALID) {
					_EspAttackTarget(self, targetNode);
					return;
				}
			} else {
				// 50% chance to attack enemy leader
				if (enemyNode != INVALID) {
					_EspAttackLeader(self, enemyNode);
					return;
				}
			}
		} else { // More than 50%, attack that leader!
			if (enemyNode != INVALID) {
				_EspAttackLeader(self, enemyNode);
				return;
			}
		}
	} else {
		// We shouldn't get here but in case we do, do something
		_EspWander(self);
		return;
	}

	/*
	// Gather nearby weapons, ammo, and items
	if (self->bot.bot_ctf_state == BOT_CTF_STATE_NONE && self->bot.bot_ctf_state != BOT_CTF_STATE_GET_DROPPED_ITEMS)
	{
		int item_node = BOTLIB_GetEquipment(self);
		if (item_node != INVALID)
		{
			{
				self->bot.state = BOT_MOVE_STATE_NAV;
				BOTLIB_SetGoal(self, nodes[item_node].nodenum);
				self->bot.bot_ctf_state == BOT_CTF_STATE_GET_DROPPED_ITEMS;
				//Com_Printf("%s %s going for %s at node %d\n", __func__, self->client->pers.netname, self->bot.get_item->classname, nodes[item_node].nodenum);
				return;
			}
		}
	}
	*/

	
}