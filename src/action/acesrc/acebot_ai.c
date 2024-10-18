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
 * $Header: /LTK2/src/acesrc/acebot_ai.c 6     29/02/00 11:14 Riever $
 *
 * $History: acebot_ai.c $
 * 
 * *****************  Version 6  *****************
 * User: Riever       Date: 29/02/00   Time: 11:14
 * Updated in $/LTK2/src/acesrc
 * Made bot ChooseWeapon function qboolean so it can signal when no weapon
 * was ready for use.
 * Moved weapon selection call to be inside attack function so bot knows
 * whether it has to fire a weapon or use a kick attack.
 * 
 * *****************  Version 5  *****************
 * User: Riever       Date: 24/02/00   Time: 3:05
 * Updated in $/LTK2/src/acesrc
 * Added bot say command support. Changed LTG weighting method.
 * 
 * *****************  Version 4  *****************
 * User: Riever       Date: 23/02/00   Time: 23:16
 * Updated in $/LTK2/src/acesrc
 * Removed random door opening code - bots now MUST have a routefile to
 * get through doors.
 * Added support for response to 'treport' radio message.
 * 
 * *****************  Version 3  *****************
 * User: Riever       Date: 17/02/00   Time: 17:53
 * Updated in $/LTK2/src/acesrc
 * Fixed item list to be in the right order!
 * 
 */
///////////////////////////////////////////////////////////////////////
//
//  acebot_ai.c -      This file contains all of the 
//                     AI routines for the ACE II bot.
//
//
// NOTE: I went back and pulled out most of the brains from
//       a number of these functions. They can be expanded on 
//       to provide a "higher" level of AI. 
////////////////////////////////////////////////////////////////////////

#include "../g_local.h"
#include "acebot.h"
#include "botchat.h"
#include "../botlib/botlib.h"

void	ACEAI_Cmd_Choose( edict_t *ent, char *s);
void	ACEMV_ChangeBotAngle (edict_t *ent);
//RiEvEr AQ2 Radio use
//void RadioBroadcast(edict_t *ent, int partner, char *msg);
//R AQ2


///////////////////////////////////////////////////////////////////////
// Main Think function for bot
///////////////////////////////////////////////////////////////////////
void ACEAI_Think(edict_t* self) 
{ 
	//Com_Printf("%s\n", __func__);
} // hehe I think not

qboolean BOTLIB_ChooseRandomNode(edict_t* self, int iter)
{
	// Random node
	for (int i = 0; i < iter; i++)
	{
		int n = rand() % numnodes; // pick a random node
		//Com_Printf("%s %s RNG Node[%i]\n", __func__, self->client->pers.netname, nodes[n].nodenum);
		if (BOTLIB_CanGotoNode(self, nodes[n].nodenum, rand() % 2))
		{
			//Com_Printf("%s %s visiting [RNG] node[%i]\n", __func__, self->client->pers.netname, nodes[n].nodenum);
			return true;
		} else {
			//Com_Printf("%s %s visiting [RNG] node[%i] failed\n", __func__, self->client->pers.netname, nodes[n].nodenum);
			return false;
		}
	}
	return false;
}

void BOTLIB_PickLongRangeGoal(edict_t* self)
{
	int i = 0;
	int node = 0;
	//float weight = 0.f, best_weight = 0.0;
	//int goal_node = 0;
	//edict_t* goal_ent = NULL;
	//float cost = INVALID;
	//int counter = 0;
	int max_random_retries = 10;

	// Clear old Node -> Node movement types
	//self->prev_to_curr_node_type = INVALID;
	//self->curr_to_next_node_type = INVALID;

	// Clear old nodes
	self->bot.prev_node = INVALID;
	self->bot.next_node = INVALID;
	self->bot.goal_node = INVALID;
	self->current_link = INVALID; // Clear old link

	// Get new current node
	int nodelist[MAX_NODELIST];
	int nodes_touched;
	/*
	// Check if spawn point location touches any nodes
	nodes_touched = BOTLIB_NodeTouchNodes(self->s.origin, tv(0, 0, 0), 64, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
	for (i = 0; i < nodes_touched; i++) // Cycle through all the nodes we touched
	{
		if (nodelist[i] != INVALID) // Make sure we can visit the spawn point node
		{
			current_node = nodelist[i];
			break;
		}
	}
	*/
	//current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
	self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	//gi.dprintf("%s is currently at node %d\n", self->client->pers.netname, self->bot.current_node);

	if (ctf->value) // CTF has it's own goals
		return;

	//Com_Printf("%s %s [%d] self->bot.state == %d\n", __func__, self->client->pers.netname, level.framenum, self->bot.state);

	if (teamplay->value && self->just_spawned_go == false)
		self->bot.state = BOT_MOVE_STATE_NAV;

	//Com_Printf("\n%s %s\n", __func__, self->client->pers.netname);

	// Couldn't find a current node so try to wander the map
	if (self->bot.current_node == INVALID)
	{
		//Com_Printf("%s %s [%d] current_node == INVALID\n", __func__, self->client->pers.netname, level.framenum);
		self->bot.state = BOT_MOVE_STATE_NAV; // BOT_MOVE_STATE_WANDER
		//self->wander_timeout = level.framenum + 1.0 * HZ;
		self->bot.goal_node = INVALID;
		return;
	}

	//=======================
	// Get navigation
	//=======================
	if (self->bot.state == BOT_MOVE_STATE_NAV)
	{
		//self->bot.get_item = NULL;
		//if (BOTLIB_NeedWeaponOrAmmo(self))
		//if (random() < 0.75 && self->client->weapon == FindItem(MK23_NAME) || self->client->weapon == FindItem(DUAL_NAME))
		if (self->client->weapon == FindItem(MK23_NAME) || self->client->weapon == FindItem(DUAL_NAME))
		{
			//self->bot.get_item = NULL;
			//node = BOTLIB_FindDroppedItems(self);
			node = BOTLIB_GetEquipment(self);
			if (node != INVALID)
			{
				//if (BOTLIB_CanGotoNode(self, nodes[node].nodenum, rand() % 2))
				//{
					//Com_Printf("%s %s going for %s at node %d\n", __func__, self->client->pers.netname, self->bot.get_item->classname, nodes[node].nodenum);
					return;
				//}
			}
		}

		// Random zero weight nodes (make bot goto nodes rarely visitied)
		if (random() < 0.2)
		{
			for (int j = 0; j < numnodes; j++)
			{
				i = rand() % numnodes; // Pick a random node
				if (nodes[i].weight == 0) // Look for zero weight
					break;
			}
			if (BOTLIB_CanGotoNode(self, nodes[i].nodenum, false))
			{
				//Com_Printf("%s %s visiting [RNG] node[%i] counter[%d]\n", __func__, self->client->pers.netname, nodes[i].nodenum, counter);
				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, nodes[i].nodenum);
				return;
			}
		}

		// Check how many bots are sitting on a POI
		for (int p = 0; p < num_players; p++) // Cycle based on how many players we find
		{
			if (players[p]->is_bot && players[p]->bot.goal_node == poi_nodes[i] && players[p]->solid == SOLID_BBOX)
			{
				//self->bot.state = BOT_MOVE_STATE_NAV;
			}
		}

		if ((random() < 0.2 || (teamplay->value && lights_camera_action)) && nodes[self->bot.current_node].type != NODE_POI)
		{
			if (num_poi_nodes)
			{
				int i = rand() % num_poi_nodes; // Pick a random POI node

				// Check if any other bots have already picked this POI node
				for (int p = 0; p < num_players; p++) // Cycle based on how many players we find
				{
					if (players[p]->is_bot && players[p]->bot.goal_node == poi_nodes[i] && players[p]->solid == SOLID_BBOX)
					{
						//Com_Printf("%s %s POI node[%i] already picked by %s\n", __func__, self->client->pers.netname, poi_nodes[i], players[p]->client->pers.netname);
						i = INVALID;
						break;
					}
				}

				if (BOTLIB_CanGotoNode(self, nodes[poi_nodes[i]].nodenum, rand() % 2)) // Make sure we can visit the spawn point node
				{
					//Com_Printf("%s %s visiting POI node[%i]\n", __func__, self->client->pers.netname, nodes[poi_nodes[i]].nodenum);
					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, nodes[poi_nodes[i]].nodenum);
					return;
				}
			}
		}

		// At LCA, randomly pick a spawn spot. This breaks up the bots at LCA, so they spread out.
		// BOTLIB_Spawnrush will take into account bot personality and current weaponry
		if (random() < 0.2 || (teamplay->value && lights_camera_action) || BOTLIB_SpawnRush(self)) // Greater than LCA!
		{
			// Find all the spawn points
			int sp_counter = 0;
			vec3_t sp_origin[128];
			edict_t* spot = NULL;
			while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
			{
				VectorCopy(spot->s.origin, sp_origin[sp_counter]);
				sp_counter++;
			}
			while ((spot = G_Find(spot, FOFS(classname), "info_player_team1")) != NULL)
			{
				VectorCopy(spot->s.origin, sp_origin[sp_counter]);
				sp_counter++;
			}
			while ((spot = G_Find(spot, FOFS(classname), "info_player_team2")) != NULL)
			{
				VectorCopy(spot->s.origin, sp_origin[sp_counter]);
				sp_counter++;
			}
			if (sp_counter) // If we found spawn points
			{
				byte spot_picked = rand() % sp_counter; // Pick a random spot

				// Check if spawn point location touches any nodes
				nodes_touched = BOTLIB_NodeTouchNodes(sp_origin[spot_picked], vec3_origin, 32, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
				for (i = 0; i < nodes_touched; i++) // Cycle through all the nodes we touched
				{
					if (BOTLIB_CanGotoNode(self, nodelist[i], true)) // Make sure we can visit the spawn point node
					{
						//if (debug_mode)
						//	Com_Printf("%s %s visiting spawn spot node[%i] [%f %f %f]\n", __func__, self->client->pers.netname, nodelist[i], nodes[nodelist[i]].origin[0], nodes[nodelist[i]].origin[1], nodes[nodelist[i]].origin[2]);

						//self->bot.state = BOT_MOVE_STATE_MOVE;
						//BOTLIB_SetGoal(self, nodelist[i]);
						return;
					}
				}
			}

			return;
		}

		// If we have an enemy, go directly for them
		if (self->enemy != NULL)
		{
			qboolean random_path = false;
			//if (self->client->pers.chosenItem == FindItemByNum(HC_NUM) || self->client->weapon == FindItemByNum(HC_NUM))
			//	random_path = false;

			if (self->bot.see_enemies && self->enemy != NULL && self->enemy->current_node != INVALID)
			{
				if (BOTLIB_CanGotoNode(self, self->enemy->current_node, random_path))
				{
					//if (debug_mode)
					//	Com_Printf("%s %s visiting enemy %s node %i\n", __func__, self->client->pers.netname, self->enemy->client->pers.netname, self->enemy->current_node);

					//self->bot.state = BOT_MOVE_STATE_MOVE;
					//BOTLIB_SetGoal(self, self->enemy->current_node);
					return;
				}
			}

		}

		// Try going after a random enemy based on the last node they visited
		//if (random() < 0.33 || bot_rush->value == 1 || BOTLIB_EnemiesAlive(self) <= 3) // Just go after them if there's only a few left
		{
			for (int p = 0; p < num_players; p++) // Cycle based on how many players we find
			{
				//i = (int)(random() * num_players); // Pick a random player
				i = rand() % num_players; // Pick a random player
				if (ACEAI_IsEnemy(self, players[i])) // Are they our enemy
				{
					// Get enemy node
					int n = players[i]->bot.current_node;	// This is for bots and humans. For humans their current node is now updated in p_client.c ClientThink()
					if (n == INVALID)
						continue;

					if (BOTLIB_CanGotoNode(self, n, true))
					{
						//if (debug_mode)
						//	Com_Printf("%s %s visiting enemy %s node %i at %f %f %f\n", __func__, self->client->pers.netname, players[i]->client->pers.netname, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);

						//self->bot.state = BOT_MOVE_STATE_MOVE;
						//BOTLIB_SetGoal(self, n);
						return;
					}
				}
			}
		}
		
		// // Random node
		int retries = 0;
			while (retries < max_random_retries)
			{
				if (!BOTLIB_ChooseRandomNode(self, 128))
					retries++;
			}
	}

	//Com_Printf("%s %s BOT_MOVE_STATE_NAV couldn't find a good path [%d]\n", __func__, self->client->pers.netname, level.framenum);
	self->bot.goal_node = INVALID;
	self->bot.state = BOT_MOVE_STATE_NAV; // Get new nav
	return; // no path? 
}

///////////////////////////////////////////////////////////////////////
// Evaluate the best long range goal and send the bot on
// its way. This is a good time waster, so use it sparingly. 
// Do not call it for every think cycle.
///////////////////////////////////////////////////////////////////////
void ACEAI_PickLongRangeGoal(edict_t *self)
{
#if 0
	int i = 0;
	int node = 0;
	float weight = 0.f, best_weight = 0.0;
	int current_node = 0, goal_node = 0;
	edict_t *goal_ent = NULL;
	float cost = 0.f;

	int n, p;

	// Clear old Node -> Node movement types
	self->prev_to_curr_node_type = INVALID;
	self->curr_to_next_node_type = INVALID;

	// Clear old nodes
	self->prev_node = INVALID;
	self->next_node = INVALID;
	self->goal_node = INVALID;
	self->current_link = INVALID; // Clear old link
	
	// Get new current node
	current_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_ALL);
	self->current_node = current_node;

	// Even in teamplay, we wander if no valid node
	if(current_node == INVALID)
	{
		self->state = STATE_WANDER;
		self->wander_timeout = level.framenum + 1.0 * HZ;
		self->goal_node = INVALID;
		return;
	}

	//Added by Werewolf
	if( self->state == STATE_WANDER )
		self->state = STATE_POSITION;

	//======================
	// Teamplay POSITION state
	//======================
	if( self->state == STATE_POSITION )
	{
		int counter = 0;
		cost = INVALID;
		self->goal_node = INVALID;

		//rekkie -- DEV_1 -- s

		if (0)
		{
			self->state = STATE_MOVE;
			self->tries = 0;
			if (random() < 0.5)
				BOTLIB_SetGoal(self, 5);
			else
				BOTLIB_SetGoal(self, 0);
			self->wander_timeout = level.framenum + 1.0 * HZ;
			return;
		}
		
		// At LCA, randomly pick a spawn spot. This breaks up the bots at LCA, so they spread out.
		if (0 && teamplay->value && lights_camera_action > 1) // Greater than LCA!
		{
			byte spot_count = 0, spot_picked;
			edict_t* spot = NULL;
			while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
			{
				spot_count++;
			}
			if (spot_count)
			{
				spot_picked = (int)(random() * (spot_count - 1)); // Pick a random spot
				spot_count = 0;
				while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
				{
					spot_count++;
					if (spot_count == spot_picked)
						break;
				}

				if (spot != NULL)
				{
					i = ACEND_FindClosestReachableNode(spot, NODE_DENSITY, NODE_ALL);
					{
						if (debug_mode)
							Com_Printf("%s %s visiting spawn spot node[%i] [%f %f %f]\n", __func__, self->client->pers.netname, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);

						self->state = STATE_MOVE;
						self->tries = 0; // Reset the count of how many times we tried this goal
						BOTLIB_SetGoal(self, i);
						self->wander_timeout = level.framenum + 1.0 * HZ;
						return;
					}
				}
			}
		}

		// Friendly bots sometimes visit allied humans during a match
		if (0 && random() < 0.50 && ltk_rush->value == 0)
		{
			for (p = 0; p < num_players; p++) // Cycle based on how many players we find
			{
				i = (int)(random() * (num_players - 1)); // Pick a random player
				if (players[i]->is_bot == false && players[i]->bot.current_node != 0 && players[i]->solid == SOLID_BBOX && players[i]->deadflag == DEAD_NO && ACEAI_IsEnemy(self, players[i]) == false) // Are they a friend and human?
				{
					// Get friendly node
					n = players[i]->bot.current_node;	// This is for bots and humans. For humans their current node is now updated in p_client.c ClientThink()
					if (n == INVALID)
						continue;

					{
						if (debug_mode)
							Com_Printf("%s %s visiting friendly %s node %i at %f %f %f\n", __func__, self->client->pers.netname, players[i]->client->pers.netname, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);

						self->state = STATE_MOVE;
						self->tries = 0; // Reset the count of how many times we tried this goal
						BOTLIB_SetGoal(self, n);
						self->wander_timeout = level.framenum + 1.0 * HZ;
						return;
					}
				}
			}
		}
		// Try going after a random enemy based on the last node they visited
		if (0 && random() < 0.33 || ltk_rush->value == 1 || BOTLIB_EnemiesAlive(self) <= 3) // Just go after them if there's only a few left
		{
			for (p = 0; p < num_players; p++) // Cycle based on how many players we find
			{
				i = (int)(random() * num_players); // Pick a random player
				if (ACEAI_IsEnemy(self, players[i])) // Are they our enemy
				{
					// Get enemy node
					n = players[i]->bot.current_node;	// This is for bots and humans. For humans their current node is now updated in p_client.c ClientThink()
					if (n == INVALID)
						continue;

					if (random() < 0.50 && BOTLIB_CanVisitNode(self, n)) // Make sure we can visit the node they're at
					{
						if (debug_mode)
							Com_Printf("%s %s visiting enemy %s node %i at %f %f %f\n", __func__, self->client->pers.netname, players[i]->client->pers.netname, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);

						self->state = STATE_MOVE;
						self->tries = 0; // Reset the count of how many times we tried this goal
						BOTLIB_SetGoal(self, n);
						self->wander_timeout = level.framenum + 1.0 * HZ;
						return;
					}
					else if (num_vis_nodes) // Otherwise try to find a visibility node that has sight on the player node
					{
						int sn = ACEND_GetRandomVisibleNode(n);
						{
							if (debug_mode)
								Com_Printf("%s %s visiting enemy %s [LOS] to node %i from node %i at %f %f %f\n", __func__, self->client->pers.netname, players[i]->client->pers.netname, n, sn, nodes[sn].origin[0], nodes[sn].origin[1], nodes[sn].origin[2]);

							self->state = STATE_MOVE;
							self->tries = 0; // Reset the count of how many times we tried this goal
							BOTLIB_SetGoal(self, sn);
							self->wander_timeout = level.framenum + 1.0 * HZ;
							return;
						}
					}
				}
			}
		}
		// Try going to a Point of Interest (PoI) node
		if (0 && num_poi_nodes) // && random() < 0.33)
		{
			for (int rn = 0; rn < 8; rn++)
			{
				i = (int)(random() * num_poi_nodes - 1);	// Any of the POI nodes will do
				if (poi_nodes[i] != INVALID)
				{
					{
						if (debug_mode)
							debug_printf("%s %s visiting [POI] node %i at %f %f %f\n", __func__, self->client->pers.netname, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);

						self->state = STATE_MOVE;
						self->tries = 0; // Reset the count of how many times we tried this goal
						BOTLIB_SetGoal(self, poi_nodes[i]);
						self->wander_timeout = level.framenum + 1.0 * HZ;
						return;
					}
				}
			}
		}
		//rekkie -- DEV_1 -- e

		// Pick a random node to go to
		while( cost == INVALID && counter < 20) // Don't look for too many
		{
			counter++;
			// pick a random node
			i = rand () % numnodes;
			//i = (int)(random() * numnodes -1);	// Any of the current nodes will do

			if (nodes[i].inuse == false) continue; // Don't go to nodes not in use

			//rekkie -- DEV_1 -- s
			{
				cost = INVALID;
				i = INVALID;
				continue;
			}
			//
			// Prevent bots from stopping on anything other than a move, spn, or poi node
			//if (nodes[i].type != NODE_MOVE && nodes[i].type != NODE_POI && nodes[i].type != NODE_SPAWNPOINT)
			//{
				//debug_printf("%s nodes[%i].type != NODE_MOVE\n", __func__, i);
			//	continue;
			//}
			//
			//if (debug_mode)
			//	debug_printf("%s Visiting random node %i at %f %f %f\n", __func__, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);
			//rekkie -- DEV_1 -- e

			cost = ACEND_FindCost(current_node, i);

			if(cost == INVALID || cost < 2) // ignore invalid and very short hops
			{
				cost = INVALID;
				i = INVALID;
				continue;
			}
		}

		//rekkie -- DEV_1 -- s
		//if (i != INVALID && nodes[i].origin[0] == 0 && nodes[i].origin[1] == 0 && nodes[i].origin[2] == 0)
		//{
		//	;
		//}
		//else
		//rekkie -- DEV_1 -- e

		// We have a target node - just go there!
		if( i != INVALID )
		{
			//rekkie -- DEV_1 -- s
			//if (debug_mode)
				Com_Printf("%s %s visiting [RNG] node %i at %f %f %f\n", __func__, self->client->pers.netname, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);
			//rekkie -- DEV_1 -- e

			self->state = STATE_MOVE;
			self->tries = 0; // Reset the count of how many times we tried this goal
			BOTLIB_SetGoal(self,i);
			self->wander_timeout = level.framenum + 1.0 * HZ;
			return;
		}
	}

	Com_Printf("%s %s ------ \n", __func__, self->client->pers.netname);

	///////////////////////////////////////////////////////
	// Items
	///////////////////////////////////////////////////////
	for(i=0;i<num_items;i++)
	{
		if(item_table[i].ent == NULL || item_table[i].ent->solid == SOLID_NOT) // ignore items that are not there.
			continue;
		
		cost = ACEND_FindCost(current_node,item_table[i].node);
		
		if(cost == INVALID || cost < 2) // ignore invalid and very short hops
			continue;
	
		weight = ACEIT_ItemNeed( self, item_table[i].ent );

		if( weight <= 0 )  // Ignore items we can't pick up.
			continue;

		weight *= ( (rand()%5) +1 ); // Allow random variations
		weight /= cost; // Check against cost of getting there
				
		if(weight > best_weight && item_table[i].node != INVALID)
		{
			best_weight = weight;
			goal_node = item_table[i].node;
			goal_ent = item_table[i].ent;
		}
	}

	///////////////////////////////////////////////////////
	// Players
	///////////////////////////////////////////////////////
	// This should be its own function and is for now just
	// finds a player to set as the goal.
	for(i=0;i<num_players;i++)
	{
		if( (players[i] == self) || (players[i]->solid == SOLID_NOT) )
			continue;

		// If it's dark and he's not already our enemy, ignore him
		if( self->enemy && players[i] != self->enemy)
		{
//Disabled by Werewolf
//			if( players[i]->light_level < 30)
//				continue;
		}

		node = ACEND_FindClosestReachableNode(players[i],NODE_DENSITY,NODE_ALL);
		// RiEvEr - bug fixing
		if( node == INVALID)
			cost = INVALID;
		else
			cost = ACEND_FindCost(current_node, node);

		if(cost == INVALID || cost < 3) // ignore invalid and very short hops
			continue;

		// Stop the centipede effect in teamplay
		if( teamplay->value )
		{
			// Check it's an enemy
			// If not an enemy, don't follow him
			if( OnSameTeam( self, players[i]))
				weight = 0.0;
			else
				weight = 1.5;	//Werewolf: Was 0.3
		}
		else
		  weight = 0.8;		//Werewolf: Was 0.3
		
		weight *= ( (rand()%5) +1 ); // Allow random variations
		weight /= cost; // Check against cost of getting there
		
		if(weight > best_weight && node != INVALID)
		{		
			best_weight = weight;
			goal_node = node;
			goal_ent = players[i];
		}	
	}

	///////////////////////////////////////////////////////
	// Domination Flags
	///////////////////////////////////////////////////////
	if( dom->value )
	{
		edict_t *flag = NULL;
		while(( flag = G_Find( flag, FOFS(classname), "item_flag" ) ))
		{
			// Only chase flags we don't already control.
			if( DomFlagOwner(flag) == self->client->resp.team )
				continue;

			node = ACEND_FindClosestReachableNode( flag, NODE_DENSITY, NODE_ALL );
			if( node == INVALID )
				continue;

			// Always prioritize flags we don't own, so only use cost to differentiate them.
			cost = ACEND_FindCost( current_node, node );
			weight = 10000 - cost;

			if( weight > best_weight )
			{		
				best_weight = weight;
				goal_node = node;
				goal_ent = flag;
			}
		}
	}
	
//Added by Werewolf
//-------------------------------
	if(best_weight == 0.0 || goal_node == INVALID )
	{
		for(i=0;i<num_players;i++)
		{
			if( (players[i] == self) || (players[i]->solid == SOLID_NOT) || (goal_node != INVALID))
				continue;
			if( teamplay->value )
			{
				// Check it's an enemy
				// If not an enemy, don't follow him
				if(!OnSameTeam( self, players[i]))
				{
					goal_node = ACEND_FindClosestReachableNode(players[i],NODE_DENSITY*50,NODE_ALL);
					goal_ent = players[i];
				}
			}
			else
			{
				goal_node = ACEND_FindClosestReachableNode(players[i],NODE_DENSITY*50,NODE_ALL);
				goal_ent = players[i];
			}
		}
	}
//-------------------------------

	// If do not find a goal, go wandering....
	if(best_weight == 0.0 || goal_node == INVALID )
	{
		//rekkie -- DEV_1 -- s
		// Try going to a random node
		/*
		int tries = 0;
		while (tries < 256)
		{
			i = (int)(random() * numnodes - 1);	// Any of the POI nodes will do
			{
				if (debug_mode)
					debug_printf("%s did not find a LR goal, visiting random node %i at %f %f %f\n", self->client->pers.netname, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);

				self->state = STATE_MOVE;
				self->tries = 0; // Reset the count of how many times we tried this goal
				BOTLIB_SetGoal(self, i);
				self->wander_timeout = level.framenum * HZ;
				return;
			}
			tries++;
		}
		*/
		//rekkie -- DEV_1 -- e

		self->goal_node = INVALID;
		self->state = STATE_WANDER;
		self->wander_timeout = level.framenum + 1.0 * HZ;
		//if(debug_mode)
		Com_Printf("%s did not find a LR goal, wandering.\n",self->client->pers.netname);
		return; // no path? 
	}
	
	// OK, everything valid, let's start moving to our goal.
	self->state = STATE_MOVE;
	self->tries = 0; // Reset the count of how many times we tried this goal
	 
	if(goal_ent != NULL && debug_mode)
		Com_Printf("%s selected a %s at node %d for LR goal.\n",self->client->pers.netname, goal_ent->classname, goal_node);

	BOTLIB_SetGoal(self,goal_node);
#endif
}


///////////////////////////////////////////////////////////////////////
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
///////////////////////////////////////////////////////////////////////
void ACEAI_PickShortRangeGoal(edict_t *self)
{
	edict_t *target = NULL;
	float weight = 0.f, best_weight = 0.0;
	edict_t *best = NULL;
	
	// look for a target (should make more efficient later)
	target = findradius(NULL, self->s.origin, 200);
	
	while(target)
	{
		if(target->classname == NULL)
			return;
		
		/*
		// Missle avoidance code
		// Set our movetarget to be the rocket or grenade fired at us. 
		if(strcmp(target->classname,"rocket")==0 || strcmp(target->classname,"grenade")==0)
		{
			if(debug_mode) 
				debug_printf("ROCKET ALERT!\n");

			self->movetarget = target;
			return;
		}
		*/
	
		if (ACEIT_IsReachable(self,target->s.origin))
		{
			if (infront(self, target))
			{
				weight = ACEIT_ItemNeed( self, target );
				
				if(weight > best_weight)
				{
					best_weight = weight;
					best = target;
				}
			}
		}

		// next target
		target = findradius(target, self->s.origin, 200);
	}

	if(best_weight)
	{
		self->movetarget = best;
		
		//if(debug_mode && self->goalentity != self->movetarget)
		//	debug_printf("%s selected a %s for SR goal (weight %.1f).\n",self->client->pers.netname, self->movetarget->classname, best_weight);
		
		//self->goalentity = best;
	}

}



//Werewolf
void ACEAI_PickSafeGoal(edict_t *self)
{

	int i;
	float best_weight=0.0;
	int current_node,goal_node = INVALID;
	
	// look for a target 
	current_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_ALL);

	self->bot.current_node = current_node;

	//rekkie -- DEV_1 -- s
	// Wander if no valid node
	if (current_node == INVALID)
	{
		//debug_printf("%s current_node %i is invalid\n", self->client->pers.netname, current_node);
		self->state = STATE_WANDER;
		self->wander_timeout = level.framenum + 1.0 * HZ;
		self->bot.goal_node = INVALID;
		return;
	}
	//rekkie -- DEV_1 -- e

	i = INVALID;
	i = ACEND_FindClosestReachableSafeNode(self,NODE_DENSITY,NODE_ALL);
	if( i != INVALID )
	{
		self->state = STATE_FLEE;
		self->tries = 0; // Reset the count of how many times we tried this goal

		// This func has been deprecated in favor of BOTLIB_CanVisitNode
		BOTLIB_CanVisitNode(self, nodes[goal_node].nodenum, false, INVALID, false);
		//BOTLIB_SetGoal(self,i);
		self->wander_timeout = level.framenum + 2.0 * HZ;
//		LTK_Say (self, "Under fire, extracting!");

		return;
	}

	

	// If do not find a goal, go wandering....
	if(best_weight == 0.0 || goal_node == INVALID )
	{
		self->bot.goal_node = INVALID;
		self->state = STATE_WANDER;
		self->wander_timeout = level.framenum + 1.0 * HZ;
		if(debug_mode)
			debug_printf("%s did not find a LR goal, wandering.\n",self->client->pers.netname);
		return; // no path? 
	}
	
	// OK, everything valid, let's start moving to our goal.
	self->state = STATE_FLEE;
	self->tries = 0; // Reset the count of how many times we tried this goal
	 
//	if(goal_ent != NULL && debug_mode)
//		debug_printf("%s selected a %s at node %d for LR goal.\n",self->client->pers.netname, goal_ent->classname, goal_node);

	// This func has been deprecated in favor of BOTLIB_CanVisitNode, but it's unusable here
	//BOTLIB_SetGoal(self,goal_node);

}
//-----------------------------------------------------------

//rekkie -- DEV_1 -- s
///////////////////////////////////////////////////////////////////////
// Returns true if 'other' is currently our enemy
///////////////////////////////////////////////////////////////////////
qboolean ACEAI_IsEnemy(edict_t* self, edict_t* other)
{
	if (other == NULL || other == self || other->client == NULL || other->solid == SOLID_NOT || (other->flags & FL_NOTARGET) || other->deadflag != DEAD_NO)
		return false;

	if (teamplay->value && (team_round_going || lights_camera_action || !ff_afterround->value) && OnSameTeam(self, other))
		return false;

	return true;
}
///////////////////////////////////////////////////////////////////////
// Returns how many enemies are alive
///////////////////////////////////////////////////////////////////////
short BOTLIB_EnemiesAlive(edict_t* self)
{
	short enemies_alive = 0;
	for (int i = 0; i <= num_players; i++)
	{
		if (ACEAI_IsEnemy(self, players[i]))
			enemies_alive++;
	}
	return enemies_alive;
}
///////////////////////////////////////////////////////////////////////
// Returns how many allies are alive
///////////////////////////////////////////////////////////////////////
short BOTLIB_AlliesAlive(edict_t* self)
{
	short allies_alive = 0;
	for (int i = 0; i <= num_players; i++)
	{
		if (players[i] == NULL || players[i] == self || players[i]->client == NULL || players[i]->solid == SOLID_NOT || (players[i]->flags & FL_NOTARGET) || players[i]->deadflag != DEAD_NO)
			continue;

		if (OnSameTeam(self, players[i]) == false)
			continue;

		allies_alive++;
	}
	return allies_alive;
}
//rekkie -- DEV_1 -- e

// Find visiable allies
short BOTLIB_FindVisibleAllies(edict_t* self)
{
	int i;
	vec3_t eyes, ally_eyes;
	VectorCopy(self->s.origin, eyes);
	eyes[2] += self->viewheight; // Get our eye level (standing and crouching)

	//int allies[128]; // Store the allies we find
	self->bot.allies_num = 0;

	// Gather visible allies we can engage
	for (i = 0; i <= num_players; i++)
	{
		if (players[i] == NULL || players[i] == self || players[i]->solid == SOLID_NOT || (players[i]->flags & FL_NOTARGET))
			continue;

		if (OnSameTeam(self, players[i]) == false)
			continue;

		//if (teamplay->value && (team_round_going || lights_camera_action || !ff_afterround->value))
		//	continue;

		VectorCopy(players[i]->s.origin, ally_eyes);
		ally_eyes[2] += players[i]->viewheight; // Get our ally eye level (standing and crouching)
		trace_t tr = gi.trace(eyes, NULL, NULL, ally_eyes, self, MASK_SHOT);
		if (tr.ent && tr.ent->health > 0 && tr.ent->solid == SOLID_BBOX)
		{
			self->bot.allies[self->bot.allies_num] = i;
			self->bot.allies_num++;
		}
	}
	return self->bot.allies_num;
}

// Find all viable enemies
qboolean BOTLIB_FindEnemy(edict_t *self)
{
	int i;
	const int INFRONT_WEIGHT = 50;
	const int VISIBLE_WEIGHT = 100;
	edict_t	*bestenemy = NULL;
	vec3_t eyes, enemy_eyes;
	VectorCopy(self->s.origin, eyes);
	eyes[2] += self->viewheight; // Get our eye level (standing and crouching)

	//int enemies[128]; // Store the enemies we find
	self->bot.enemies_num = 0;

	// If a player enables notarget, disable search for ALL enemies
	for (i = 0; i <= num_players; i++)
	{
		if (players[i] != NULL && players[i]->flags & FL_NOTARGET)
			return false;
	}

	// Update previous enemy
	if (self->enemy != NULL && self->bot.old_enemy != self->enemy)
		self->bot.old_enemy = self->enemy;


	// Add random element by allowing bots to RNG attack teammates after a TP round ends
	qboolean can_kill_teammates = false;
	if (self->bot.ff_allies_after_rnd && team_round_going == 0 && lights_camera_action == 0 && ((rand() % (3*HZ)) == 1)) // Add some randomness
		can_kill_teammates = true;
	

	// ALWAYS gather a list of potential enemies. Required for internal and external functions (like BOTLIB_Radio)
	// This must come before we check if bot can still attack existing enemy
	for (i = 0; i <= num_players; i++)
	{
		if (players[i] == NULL || players[i] == self || players[i]->solid == SOLID_NOT || (players[i]->flags & FL_NOTARGET) || players[i]->client->uvTime)
			continue;

		// If attaked by a team mate after the round ends
		if (ff_afterround->value && team_round_going == 0 && lights_camera_action == 0)
		{
			if (self->client->attacker && self->client->attacker == players[i])
			{
				can_kill_teammates = true; // Flag team member as unfriendly
			}
		}

		// Sometimes attack team mates randomly after a round ends, or if we get attacked by a team mate,
		// or the teammate has ff_allies_after_rnd enabled
		if (OnSameTeam(self, players[i]))
		{
			if (players[i]->is_bot && players[i]->bot.ff_allies_after_rnd == true 
				&& team_round_going == 0 && lights_camera_action == 0 && ((rand() % (3*HZ)) == 1)) // Add some randomness
				; // Allow this bot to target other bots that FF after the round ends
			else if (can_kill_teammates == false)
				continue; // Don't target friendly team members
		}

		VectorCopy(players[i]->s.origin, enemy_eyes);
		enemy_eyes[2] += players[i]->viewheight; // Get our enemy eye level (standing and crouching)

		if (gi.inPVS(eyes, enemy_eyes)) // Only consider enemies that are in the same PVS as our bot
		{
			float weight = 0;

			// Target is in front (might not be visible)
			//if (infront(self, players[i]))
			//	weight += INFRONT_WEIGHT;

			// Target is visible
			trace_t tr = gi.trace(eyes, NULL, NULL, enemy_eyes, self, MASK_SHOT);
			if (tr.ent && tr.ent->health > 0 && tr.ent->solid == SOLID_BBOX)
			{
				weight += VISIBLE_WEIGHT;

				// Target is in front and visible
				if (infront(self, tr.ent))
					weight += INFRONT_WEIGHT;
			}

			/*
			// Listen for weapon noises. Firing, Bursting, Reloading, Activating
			if (INV_AMMO(players[i], SIL_NUM))  // Silencer
			{
				weight -= 10; // Quiet enemy
			}
			else // Unsilenced
			{
				if (players[i]->client->weaponstate == WEAPON_FIRING || players[i]->client->weaponstate == WEAPON_BURSTING)
					weight += 10;
				if (players[i]->client->weaponstate == WEAPON_RELOADING || players[i]->client->weaponstate == WEAPON_ACTIVATING)
					weight += 5;
			}



			// Listen for footsteps, limited by distance 
			if (VectorDistance(self->s.origin, players[i]->s.origin) < 300)
			{
				if (INV_AMMO(players[i], SLIP_NUM)) // Slippers
					weight -= 10; // Quiet enemy
				else
					weight += 10; 
			}
			*/

			if (weight > 0)
			{
				// Add target to the list of other possibles
				self->bot.enemies[self->bot.enemies_num] = i;
				self->bot.enemies_weight[self->bot.enemies_num] = weight;
				self->bot.enemies_num++;
			}
		}
	}

	// Check if bot can still attack existing enemy
	if (self->enemy && self->enemy->solid != SOLID_NOT && (self->enemy->flags & FL_NOTARGET) == 0 && self->enemy->client && self->enemy->client->uvTime == 0)
	{
		VectorCopy(self->enemy->s.origin, enemy_eyes);
		enemy_eyes[2] += self->enemy->viewheight; // Get our enemy eye level (standing and crouching)

		// Check if target is visible
		trace_t tr = gi.trace(eyes, NULL, NULL, enemy_eyes, self, MASK_SHOT);
		if (tr.ent && tr.ent->health > 0 && tr.ent->solid == SOLID_BBOX)
		{
			// Minimum time to remember an enemy is 15 seconds, maximum is 30 seconds
			float remember_time = min((BOTLIB_SkillMultiplier(self->bot.skill.overall, true) * 15), 15);
			if (remember_time > 30)
				remember_time = 30;
				
			VectorCopy(self->enemy->s.origin, self->bot.enemy_seen_loc); // Update last seen location
			self->bot.enemy_seen_time = level.framenum + remember_time * HZ; // Update the last time we saw the enemy
			self->bot.enemy_in_xhair = BOTLIB_IsAimingAt(self, self->enemy);
			self->bot.enemy_dist = VectorDistance(self->s.origin, self->enemy->s.origin);
			self->bot.enemy_height_diff = fabs(self->s.origin[2] - self->enemy->s.origin[2]);

			//Com_Printf("%s %s see %s R[%d vs %d]L\n", __func__, self->client->pers.netname, self->enemy->client->pers.netname, self->bot.reaction_time, level.framenum);

			if (self->bot.skill.reaction > level.framenum)
			{
				//Com_Printf("%s %s no fire R[%d vs %d]L\n", __func__, self->client->pers.netname, self->bot.reaction_time, level.framenum);
				return false;
			}
			else
			{
				//Com_Printf("%s %s can fire R[%d vs %d]L\n", __func__, self->client->pers.netname, self->bot.reaction_time, level.framenum);
				return true;
			}

			//return true; // Go get 'em tiger!
		}

		else
		{
			VectorClear(self->bot.enemy_seen_loc);
			self->bot.enemy_in_xhair = false;
			self->bot.enemy_dist = 0;
			self->bot.enemy_height_diff = 0;
			self->bot.enemy_seen_time = 0;
			self->bot.reaction_time = 0;
			self->enemy = NULL;
		}
		


		
		/*
		if (gi.inPVS(eyes, enemy_eyes)) // Only consider enemies that are in the same PVS as our bot
		{
			qboolean possible_target = false;

			// Listen for (unsilenced) footsteps, limited by distance 
			if (INV_AMMO(self->enemy, SLIP_NUM) == 0 && VectorDistance(self->s.origin, self->enemy) < 300)
			{
				possible_target = true;
			}

			// Listen for (unsilenced) weapon noises
			else if (INV_AMMO(self->enemy, SIL_NUM) == 0 && (self->enemy->client->weaponstate == WEAPON_FIRING || self->enemy->client->weaponstate == WEAPON_BURSTING))
			{
				possible_target = true;
			}
			// Target is in front
			else if (infront(self, self->enemy)) // Visible enemies
			{
				possible_target = true;
			}

			if (possible_target)
			{
				// Check if target is visible
				trace_t tr = gi.trace(eyes, NULL, NULL, enemy_eyes, self, MASK_SHOT);
				if (tr.ent && tr.ent->health > 0 && tr.ent->solid == SOLID_BBOX)
				{
					self->bot.enemy_seen_time = level.framenum + bot_remember->value * HZ; // Update the last time we saw the enemy
					self->bot.enemy_in_xhair = BOTLIB_IsAimingAt(self, self->enemy);
					self->bot.enemy_dist = VectorDistance(self->s.origin, self->enemy->s.origin);
					self->bot.enemy_height_diff = fabs(self->s.origin[2] - self->enemy->s.origin[2]);
					return true; // Go get 'em tiger!
				}
			}
		}
		*/

		/*
		int mask = (MASK_SHOT | MASK_WATER); // Default mask - don't let bot see enemies through water
		int pc_self = gi.pointcontents(self->s.origin); // Bot in contents
		int pc_enemy = gi.pointcontents(self->enemy->s.origin); // Player in contents
		if ( (pc_self & MASK_WATER) && (pc_enemy & MASK_WATER) ) // Bot and enemy are both underwater
		{
			mask = MASK_SHOT; // Don't mask through water so the bot can see the enemy underwater
		}
		trace_t tr = gi.trace(eyes, NULL, NULL, enemy_eyes, self, mask);
		if (tr.ent && tr.ent->health > 0 && tr.ent->solid == SOLID_BBOX)
		{
			self->bot.enemy_seen_time = level.framenum + bot_remember->value * HZ; // Update the last time we saw the enemy
			self->bot.enemy_in_xhair = BOTLIB_IsAimingAt(self, self->enemy);
			self->bot.enemy_dist = VectorDistance(self->s.origin, self->enemy->s.origin);
			self->bot.enemy_height_diff = fabs(self->s.origin[2] - self->enemy->s.origin[2]);
			return true;
		}
		*/
	}
	else
	{
		self->bot.enemy_chase_time = 0; // Reset chase delay set for any previous enemies
	}

	if (self->bot.enemies_num)
	{
		// 1) Target high priority enemies (flag carriers, briefcase, vip, etc)
		for (i = 0; i < self->bot.enemies_num; i++)
		{
			if ((self->bot.enemies_weight[i] > 300 && INV_AMMO(players[self->bot.enemies[i]], FLAG_T1_NUM)) || INV_AMMO(players[self->bot.enemies[i]], FLAG_T2_NUM))
				bestenemy = players[self->bot.enemies[i]];
		}

		// 2) Target enemy based on their weight
		if (bestenemy == NULL)
		{
			float best_weight = 0;
			for (i = 0; i < self->bot.enemies_num; i++)
			{
				if (self->bot.enemies_weight[i] >= (INFRONT_WEIGHT + VISIBLE_WEIGHT) && self->bot.enemies_weight[i] > best_weight)
				{
					best_weight = self->bot.enemies_weight[i];
					bestenemy = players[self->bot.enemies[i]];
				}
			}
		}

		// 3) Target enemies that get too close
		//if (bestenemy == NULL)
		{
			float dist;
			float closest = 9999999;
			int closest_enemy = 0;
			for (i = 0; i < self->bot.enemies_num; i++)
			{
				dist = VectorDistance(self->s.origin, players[self->bot.enemies[i]]->s.origin);
				if (dist < closest)
				{
					closest_enemy = i;
					closest = dist;
				}
			}
			// If enemy is super close, go after them
			if (closest < 64)
			{
				bestenemy = players[self->bot.enemies[closest_enemy]];
			}
		}

		/*
		// 4) Target a random enemy
		if (bestenemy == NULL)
		{
			int rnd_enemy = rand() % self->bot.enemies_num;
			if (self->bot.enemies_num)
			{
				bestenemy = players[self->bot.enemies[rnd_enemy]];
			}
		}
		*/
	}

	// If we found an enemy
	if (bestenemy != NULL)
	{
		// Minimum time to remember an enemy is 15 seconds, maximum is 30 seconds
		float remember_time = min((BOTLIB_SkillMultiplier(self->bot.skill.overall, true) * 15), 15);
		if (remember_time > 30)
			remember_time = 30;

		self->enemy = bestenemy;
		self->bot.enemy_seen_time = level.framenum + remember_time * HZ; // Update the last time we saw the enemy
		self->bot.enemy_in_xhair = BOTLIB_IsAimingAt(self, self->enemy);
		self->bot.enemy_dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		self->bot.enemy_height_diff = fabs(self->s.origin[2] - self->enemy->s.origin[2]);

		self->bot.reaction_time = BOTLIB_SKILL_Reaction(self->bot.skill.reaction) * HZ;
		//Com_Printf("%s %s see %s R[%d vs %d]L\n", __func__, self->client->pers.netname, self->enemy->client->pers.netname, self->bot.reaction_time, level.framenum);

		/*
		// The bot's reaction time when facing a new enemy
		self->bot.reaction_time -= FRAMETIME; // Reduce reaction time
		if (self->bot.reaction_time < 0)
			self->bot.reaction_time = 0;
		if (self->bot.reaction_time > bot_reaction->value)
			self->bot.reaction_time = bot_reaction->value;
		//Com_Printf("%s %s remember enemy [%s] reaction_time[%f]\n", __func__, self->client->pers.netname, self->enemy->client->pers.netname, self->bot.reaction_time);

		if (self->bot.reaction_time == 0) // If we have no reaction time left, we can shoot
			return true;
		else
			return false; // Waiting for reaction time to expire.
		*/
	}
	return false;


#if 0

	int i;
	edict_t* bestenemy = NULL;
	//float		bestweight = 99999;
	//float		weight;
	//vec3_t		dist;
	//vec3_t		eyes;

	int enemies[128]; // Store the enemies we find
	self->bot.enemies_num = 0;

	//VectorCopy( self->s.origin, eyes );
	//eyes[2] += self->viewheight;

/*	// If we already have an enemy and it is the last enemy to hurt us
	if (self->enemy && 
		(self->enemy == self->client->attacker) &&
		(!self->enemy->deadflag) &&
		(self->enemy->solid != SOLID_NOT)
		)
	{
		return true;
	}
*/
	for(i=0;i<=num_players;i++)
	{
		if(players[i] == NULL || players[i] == self || 
		   players[i]->solid == SOLID_NOT || (players[i]->flags & FL_NOTARGET) )
		   continue;
	
		// If it's dark and he's not already our enemy, ignore him
		if( self->enemy && players[i] != self->enemy)
		{
			if( players[i]->light_level < 30)
				continue;
		}

/*		if(ctf->value && 
		   self->client->resp.ctf_team == players[i]->client->resp.ctf_team)
		   continue;*/
// AQ2 ADD
		if(teamplay->value && (team_round_going || lights_camera_action || ! ff_afterround->value) && OnSameTeam( self, players[i]) )
		   continue;
// AQ2 END

		if((players[i]->deadflag == DEAD_NO) && ai_visible(self, players[i]) && 
			gi.inPVS(eyes, players[i]->s.origin) )
		{
// RiEvEr
			// Now we assess this enemy
			qboolean visible = infront( self, players[i] );
			VectorSubtract(self->s.origin, players[i]->s.origin, dist);
			weight = VectorLength( dist );

			if( ! visible )
			{
				// Can we hear their footsteps?
				visible = (weight < 300) && !INV_AMMO( players[i], SLIP_NUM );
			}

			if( ! visible )
			{
				// Can we hear their weapon firing?
				if( players[i]->client->weaponstate == WEAPON_FIRING || players[i]->client->weaponstate == WEAPON_BURSTING )
				{
					switch( players[i]->client->weapon->typeNum )
					{
						case DUAL_NUM:
						case M4_NUM:
						case M3_NUM:
						case HC_NUM:
							visible = true;
							break;
						case KNIFE_NUM:
						case GRENADE_NUM:
							break;
						default:
							visible = !INV_AMMO( players[i], SIL_NUM );
					}
				}
			}

			if( ! visible )
			{
				// Can we see their flashlight?
				edict_t *fl = players[i]->client->flashlight;
				visible = fl && infront( self, fl );
				if( fl && ! visible )
				{
					VectorSubtract( self->s.origin, fl->s.origin, dist );
					visible = (VectorLength(dist) < 100);
				}
			}

			if( ! visible )
			{
				// Can we see their laser sight?
				edict_t *laser = players[i]->client->lasersight;
				visible = laser && (laser->s.modelindex != level.model_null) && infront( self, laser ) && ai_visible( self, laser );
			}

			// Can we see this enemy, or are they calling attention to themselves?
			if( visible )
			{
				self->bot.enemies_num++;

				// If we can see the enemy flag carrier, always shoot at them.
				if( INV_AMMO( players[i], FLAG_T1_NUM ) || INV_AMMO( players[i], FLAG_T2_NUM ) )
					weight = 0;

				// See if it's better than what we have already
				if (weight < bestweight)
				{
					bestweight = weight;
					bestenemy = players[i];
				}
			}
		}
	}
	// If we found a good enemy set it up
	if( bestenemy)
	{
		self->bot.enemy_seen_time = level.framenum + bot_remember->value * HZ; // Update the last time we saw them
		self->enemy = bestenemy;
		return true;
	}
	// Check if we've been shot from behind or out of view
	if( self->client->attacker && self->client->attacker->inuse && (self->client->attacker != self) )
	{
		// Check if it was recent
		if( self->client->push_timeout > 0)
		{
			if( (self->client->attacker->solid != SOLID_NOT) && ai_visible(self, self->client->attacker) &&
				gi.inPVS(eyes, self->client->attacker->s.origin) )
			{
				self->bot.enemy_seen_time = level.framenum + bot_remember->value * HZ; // Update the last time we saw them
				self->enemy = self->client->attacker;
				return true;
			}
		}
	}
//R
#endif

	float reaction_multiplier = BOTLIB_SKILL_Reaction(self->bot.skill.reaction);
	if (0)
	{
		self->bot.enemy_in_xhair = false;
		self->bot.enemy_dist = 0;
		self->bot.enemy_height_diff = 0;
		self->bot.enemy_seen_time = 0;
		self->bot.reaction_time = reaction_multiplier * HZ;
		self->enemy = NULL; // Reset any enemies, including any the bot remembered...
		return false;
	}


	// Check if we have an existing enemy, try to remember them and follow their movements even when behind a wall
	if (self->enemy && self->enemy->deadflag == DEAD_NO && self->enemy->health > 0 && self->enemy->solid == SOLID_BBOX)
	{
		if (self->bot.enemy_seen_time > level.framenum) // Remember enemy
		{
			self->bot.enemy_in_xhair = false;
			self->bot.enemy_dist = 0;
			self->bot.enemy_height_diff = 0;
			self->bot.reaction_time += FRAMETIME; // Gradually forget
			//Com_Printf("%s %s remember enemy [%s] reaction_time[%f]\n", __func__, self->client->pers.netname, self->enemy->client->pers.netname, self->bot.reaction_time);
			return false;
		}
	}
	// Otherwise forget them
	else
	{
		VectorClear(self->bot.enemy_seen_loc);
		self->bot.enemy_in_xhair = false;
		self->bot.enemy_dist = 0;
		self->bot.enemy_height_diff = 0;
		self->bot.enemy_seen_time = 0;
		self->bot.reaction_time = reaction_multiplier;
		self->enemy = NULL; // Reset any enemies, including any the bot remembered...
	}


	// Otherwise signal no enemy
	return false;
  
}

///////////////////////////////////////////////////////////////////////
// Hold fire with RL/BFG?
///////////////////////////////////////////////////////////////////////
//@@ Modify this to check for hitting teammates in teamplay games.
//Modification request gladly completed by Werewolf :)
qboolean ACEAI_CheckShot(edict_t *self)
{
	trace_t tr;

//AQ2	tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), self->enemy->s.origin, self, MASK_OPAQUE);
//	tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), self->enemy->s.origin, self, MASK_SOLID|MASK_OPAQUE);
//	tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, self->enemy->s.origin, self, MASK_SOLID|MASK_OPAQUE); // Werewolf: was enabled
	tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), self->enemy->s.origin, self, MASK_PLAYERSOLID); //Check for friendly fire

	//We would hit something
	if (tr.fraction < 0.9)
		//If we're in teamplay the the accidentally hit player is a teammate, hold fire
		if( (teamplay->value) && team_round_going && (OnSameTeam( self, tr.ent)) )
			return false;
		//In deathmatch, don't shoot through glass and stuff
//		else if ((!teamplay->value) && (tr.ent->solid==SOLID_BSP))
//			return false;

//	tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, self->enemy->s.origin, self, MASK_SOLID|MASK_OPAQUE); // Check for Hard stuff
	tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), self->enemy->s.origin, self, MASK_SOLID|MASK_OPAQUE);
	if( (tr.fraction < 0.9) && (tr.ent->solid==SOLID_BSP) )
	{
		// If we'd hit something solid, see if that's also true aiming for the head.
		vec3_t enemy_head;
		VectorCopy( self->enemy->s.origin, enemy_head );
		enemy_head[2] += 12.f;  // g_combat.c HEAD_HEIGHT
		tr = gi.trace( self->s.origin, tv(-2,-2,-2), tv(2,2,2), enemy_head, self, MASK_SOLID|MASK_OPAQUE );
		if( (tr.fraction < 0.9) && (tr.ent->solid==SOLID_BSP) )
			return false;
	}

	//else shoot
	return true;
}


///////////////////////////////////////////////////////////////////////
// Choose the best weapon for bot (simplified)
// Modified by Werewolf to use sniper zoom
///////////////////////////////////////////////////////////////////////
//RiEvEr - Changed to boolean.
qboolean ACEAI_ChooseWeapon(edict_t *self)
{		
	// if no enemy, then what are we doing here?
	if(!self->enemy)
		return (true);

	if (self->bot.last_weapon_change_time > level.framenum)
		return true;
	self->bot.last_weapon_change_time = level.framenum + 6 * HZ;

	//if (self->client->weaponstate != WEAPON_READY)
	//	return true;

	// Let bots use the grenades if already using
	if (self->client->weapon == FindItemByNum(GRENADE_NUM))
		return true;
	// Let bots use the knives if already using
	if (self->client->weapon == FindItemByNum(KNIFE_NUM))
		return true;

//AQ2 CHANGE	
	// Currently always favor the dual pistols!
	//@@ This will become the "bot choice" weapon
//	if(ACEIT_ChangeDualSpecialWeapon(self,FindItem(DUAL_NAME)))
//   	   return;
//AQ2 END

		
	/*
	// Friendly fire after round should be fought with honor.
	if( team_round_countdown && ff_afterround->value )
	{
		if( ACEIT_ChangeWeapon(self,FindItem(KNIFE_NAME)) )
			return true;
		if( ACEIT_ChangeHCSpecialWeapon(self,FindItem(HC_NAME)) )
			return true;
		if( ACEIT_ChangeWeapon(self,FindItem(GRENADE_NAME)) )
		{
			self->client->pers.grenade_mode = (	self->bot.enemy_dist > 500) ? 1 : 0;
			return true;
		}
	}
	*/

	/*
	if (self->client->weapon == FindItemByNum(GRENADE_NUM))
	{
		if (self->client->weaponstate == WEAPON_ACTIVATING || self->client->weaponstate == WEAPON_FIRING)
			return true;
	}
	*/

	//rekkie -- DEV_1 -- s
	// Force bots use grenades only
	/*
	if (ACEIT_ChangeWeapon(self, FindItemByNum(GRENADE_NUM)))
	{
		self->client->pers.grenade_mode = 2;
		return true;
	}
	*/
	//rekkie -- DEV_1 -- e



	// Extreme range
	if(self->bot.enemy_dist > 1300 )
	{
		if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
		{
			//ACEAI_SetSniper( self, 2 );
			return (true);
		}
	}

	// Longest range 
	if(self->bot.enemy_dist > 1000)
	{
		// Random chance to pull out grenades
		if ((rand() % 3) == 0 && ACEIT_ChangeWeapon(self, FindItem(GRENADE_NAME)))
		{
			self->client->pers.grenade_mode = 2;
			return true;
		}
		// Random chance to just use knives
		if ((rand() % 6) == 0 && (INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (ACEIT_ChangeSniperSpecialWeapon(self, FindItem(SNIPER_NAME)))
		{
			//ACEAI_SetSniper(self, 2);
			return (true);
		}

		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
			return (true);
		
		//if( INV_AMMO(self,SIL_NUM) && ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)) )
		//	return (true);

		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return (true);

		// Allow bots that picked throwing knives to use them at a distance
		if (INV_AMMO(self, KNIFE_NUM) >= 2 && self->client->pers.chosenItem == FindItemByNum(KNIFE_NUM) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (ACEIT_ChangeDualSpecialWeapon(self, FindItem(DUAL_NAME)))
			return (true);

		// If bot has a HC and healthy: 
		// keep HC equipped so the bot can try get closer to its target, instead of pulling out the pistol
		if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME) 
			&& self->client->leg_damage == 0 && self->client->bleeding == 0 
			&& ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);

		if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   		   return (true);
	}
	
	// Longer range 
	if(self->bot.enemy_dist > 700)
	{
		// Random chance to pull out grenades
		if ((rand() % 2) == 0 && ACEIT_ChangeWeapon(self, FindItem(GRENADE_NAME)))
		{
			self->client->pers.grenade_mode = 2;
			return (true);
		}
		// Random chance to pull out knives
		if ((rand() % 3) == 0 && (INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (ACEIT_ChangeSniperSpecialWeapon(self, FindItem(SNIPER_NAME)))
		{
			//ACEAI_SetSniper(self, 2);
			return (true);
		}

		if( INV_AMMO(self,SIL_NUM) )
		{
			if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
				return (true);

			if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
				return (true);
		}

		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return (true);
		
		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
			return (true);

		if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
   		   return (true);

		// If bot has a HC and healthy: 
		// keep HC equipped so the bot can try get closer to its target, instead of pulling out the pistol
		if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME)
			&& self->client->leg_damage == 0 && ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);

		if ((INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return (true);
		}

		if (ACEIT_ChangeDualSpecialWeapon(self, FindItem(DUAL_NAME)))
			return (true);

		if (ACEIT_ChangeMK23SpecialWeapon(self, FindItem(MK23_NAME)))
			return (true);

		if (ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);
	}
	
	// Long range 
	if(self->bot.enemy_dist > 500)
	{
		// Random chance to pull out grenades
		if ((rand() % 4) == 0 && ACEIT_ChangeWeapon(self, FindItem(GRENADE_NAME)))
		{
			self->client->pers.grenade_mode = 1;
			return (true);
		}
		// Random chance to pull out knives
		if ((rand() % 4) == 0 && (INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if( INV_AMMO(self,LASER_NUM) )
		{
			if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
				return (true);

			if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
				return (true);
		}

		if(ACEIT_ChangeM3SpecialWeapon(self,FindItem(M3_NAME)))
			return (true);
		
		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
			return (true);
		
		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
			return (true);

		if(ACEIT_ChangeSniperSpecialWeapon(self,FindItem(SNIPER_NAME)))
		{
			//ACEAI_SetSniper( self, 2 );
			return (true);
		}

		// If bot has a HC and healthy: 
		// keep HC equipped so the bot can try get closer to its target, instead of pulling out the pistol
		if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME)
			&& self->client->leg_damage == 0 && ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);

		if ((INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return (true);
		}

		if (ACEIT_ChangeDualSpecialWeapon(self, FindItem(DUAL_NAME)))
			return (true);

		if (ACEIT_ChangeMK23SpecialWeapon(self, FindItem(MK23_NAME)))
			return (true);

		if (ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);
	}
	
	// Medium range 
	if(self->bot.enemy_dist > 200)
	{
		//if( INV_AMMO(self,SLIP_NUM) && ACEIT_ChangeHCSpecialWeapon(self,FindItem(HC_NAME)) )
			//return (true);

		// Always HC at this range
		if (ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);

		// Random chance to pull out grenades
		if ((rand() % 10) == 0 && ACEIT_ChangeWeapon(self, FindItem(GRENADE_NAME)))
		{
			self->client->pers.grenade_mode = 1;
			return (true);
		}
		// Random chance to pull out knives
		if ((rand() % 5) == 0 && (INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (ACEIT_ChangeM3SpecialWeapon(self, FindItem(M3_NAME)))
			return (true);

		if( INV_AMMO(self,LASER_NUM) )
		{
			if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
				return (true);

			if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
				return (true);
		}

		if(ACEIT_ChangeM4SpecialWeapon(self,FindItem(M4_NAME)))
   		   return (true);

		if(ACEIT_ChangeMP5SpecialWeapon(self,FindItem(MP5_NAME)))
			return (true);

		//rekkie -- DEV_1 -- s
		// Force bots to sniper up close
		if (ACEIT_ChangeSniperSpecialWeapon(self, FindItem(SNIPER_NAME)))
		{
			//ACEAI_SetSniper(self, 1);
			return (true);
		}
		//rekkie -- DEV_1 -- e

		// Raptor007: Throw knives at medium range if we have extras.
		if ((INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return (true);
		}

		if(ACEIT_ChangeDualSpecialWeapon(self,FindItem(DUAL_NAME)))
			return (true);

		if(ACEIT_ChangeMK23SpecialWeapon(self,FindItem(MK23_NAME)))
   		   return (true);

		if (ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
			return (true);
	}
	
	// Short range	   
	{
		if (ACEIT_ChangeHCSpecialWeapon(self, FindItem(HC_NAME)))
			return (true);

		if ((rand() % 5) == 0 && ACEIT_ChangeWeapon(self, FindItem(GRENADE_NAME)))
		{
			self->client->pers.grenade_mode = 0;
			return (true);
		}

		if (INV_AMMO(self, LASER_NUM))
		{
			if (ACEIT_ChangeM4SpecialWeapon(self, FindItem(M4_NAME)))
				return (true);

			if (ACEIT_ChangeMP5SpecialWeapon(self, FindItem(MP5_NAME)))
				return (true);
		}

		if (ACEIT_ChangeM3SpecialWeapon(self, FindItem(M3_NAME)))
			return (true);

		if (ACEIT_ChangeM4SpecialWeapon(self, FindItem(M4_NAME)))
			return (true);

		if (ACEIT_ChangeMP5SpecialWeapon(self, FindItem(MP5_NAME)))
			return (true);

		//rekkie -- DEV_1 -- s
		// Force bots to knife and sniper up close
		if (ACEIT_ChangeSniperSpecialWeapon(self, FindItem(SNIPER_NAME)))
		{
			//ACEAI_SetSniper(self, 1);
			return (true);
		}
		//rekkie -- DEV_1 -- e

		if ((INV_AMMO(self, KNIFE_NUM) >= 2) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return (true);
		}

		if (ACEIT_ChangeDualSpecialWeapon(self, FindItem(DUAL_NAME)))
			return (true);

		if (ACEIT_ChangeMK23SpecialWeapon(self, FindItem(MK23_NAME)))
			return (true);

		if (ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
			return (true);

		// Should punch as a last resort?
	}

	
	// We have no weapon available for use.
	if(debug_mode)
		gi.bprintf(PRINT_HIGH,"%s: No weapon available...\n",self->client->pers.netname);
	return (false);
}

void ACEAI_Cmd_Choose (edict_t *ent, char *s)
{
    // only works in teamplay
    if (!teamplay->value)
            return;
    
    if ( Q_stricmp(s, MP5_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(MP5_NAME);
    else if ( Q_stricmp(s, M3_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(M3_NAME);
    else if ( Q_stricmp(s, M4_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(M4_NAME);
    else if ( Q_stricmp(s, HC_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(HC_NAME);
    else if ( Q_stricmp(s, SNIPER_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(SNIPER_NAME);
    else if ( Q_stricmp(s, KNIFE_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(KNIFE_NAME);
    else if ( Q_stricmp(s, DUAL_NAME) == 0 )
            ent->client->pers.chosenWeapon = FindItem(DUAL_NAME);
    else if ( Q_stricmp(s, KEV_NAME) == 0 )
            ent->client->pers.chosenItem = FindItem(KEV_NAME);
    else if ( Q_stricmp(s, LASER_NAME) == 0 )
            ent->client->pers.chosenItem = FindItem(LASER_NAME);
    else if ( Q_stricmp(s, SLIP_NAME) == 0 )
            ent->client->pers.chosenItem = FindItem(SLIP_NAME);
    else if ( Q_stricmp(s, SIL_NAME) == 0 )
            ent->client->pers.chosenItem = FindItem(SIL_NAME);
    else if ( Q_stricmp(s, BAND_NAME) == 0 )
            ent->client->pers.chosenItem = FindItem(BAND_NAME);
}

void ACEAI_Cmd_Choose_Weapon_Num( edict_t *ent, int num )
{
	if( !( num && WPF_ALLOWED(num) ) )
	{
		int list[ WEAPON_COUNT ] = {0};
		int total = 0;

		// Preferred primary weapons.
		if( WPF_ALLOWED(MP5_NUM) )
		{
			list[ total ] = MP5_NUM;
			total ++;
		}
		if( WPF_ALLOWED(M4_NUM) )
		{
			list[ total ] = M4_NUM;
			total ++;
		}
		if( WPF_ALLOWED(SNIPER_NUM) )
		{
			list[ total ] = SNIPER_NUM;
			total ++;
		}
		if( WPF_ALLOWED(M3_NUM) )
		{
			list[ total ] = M3_NUM;
			total ++;
		}
		if( WPF_ALLOWED(HC_NUM) )
		{
			list[ total ] = HC_NUM;
			total ++;
		}

		if( total )
			num = list[ rand() % total ];
		// Last resorts if nothing else is allowed.
		else if( WPF_ALLOWED(DUAL_NUM) )
			num = DUAL_NUM;
		else if( WPF_ALLOWED(KNIFE_NUM) )
			num = KNIFE_NUM;
		else if( WPF_ALLOWED(MK23_NUM) )
			num = MK23_NUM;
	}

	if( num )
		ent->client->pers.chosenWeapon = FindItemByNum(num);
}

void ACEAI_Cmd_Choose_Item_Num( edict_t *ent, int num )
{
	if( !( num && ITF_ALLOWED(num) ) )
	{
		int list[ ITEM_COUNT ] = {0};
		int total = 0;

		if( ITF_ALLOWED(KEV_NUM) )
		{
			list[ total ] = KEV_NUM;
			total ++;
		}
		if( ITF_ALLOWED(BAND_NUM) )
		{
			list[ total ] = BAND_NUM;
			total ++;
		}
		if( ITF_ALLOWED(LASER_NUM) )
		{
			list[ total ] = LASER_NUM;
			total ++;
		}
		if( ITF_ALLOWED(SIL_NUM) )
		{
			list[ total ] = SIL_NUM;
			total ++;
		}
		if( ITF_ALLOWED(SLIP_NUM) )
		{
			list[ total ] = SLIP_NUM;
			total ++;
		}
		if( ITF_ALLOWED(HELM_NUM) )
		{
			list[ total ] = HELM_NUM;
			total ++;
		}

		if( total )
			num = list[ rand() % total ];
	}

	if( num )
		ent->client->pers.chosenItem = FindItemByNum(num);
}

void ACEAI_Cmd_Choose_ItemKit_Num( edict_t *ent, int num )
{
	// Item Kits ignore item bans, for simplicity

	if( num == KEV_NUM ) {
		ent->client->pers.chosenItem = FindItemByNum(KEV_NUM);
	} else if ( num == C_KIT_NUM ) {
		ent->client->pers.chosenItem = FindItemByNum(BAND_NUM);
		ent->client->pers.chosenItem2 = FindItemByNum(HELM_NUM);
	} else if ( num == S_KIT_NUM ) {
		ent->client->pers.chosenItem = FindItemByNum(SLIP_NUM);
		ent->client->pers.chosenItem2 = FindItemByNum(SIL_NUM);
    } else if ( num == A_KIT_NUM ) {
		ent->client->pers.chosenItem = FindItemByNum(LASER_NUM);
		ent->client->pers.chosenItem2 = FindItemByNum(SIL_NUM);
	} else { // For safety
		gi.dprintf("%s: num value here is not an item kit or kevlar vest (%i)\n", __func__, num);
		ent->client->pers.chosenItem = FindItemByNum(KEV_NUM);
	}
}
