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
 * $Header: /LTK2/src/acesrc/acebot_nodes.c 9     29/02/00 22:59 Riever $
 *
 * $Log: /LTK2/src/acesrc/acebot_nodes.c $
 * 
 * 9     29/02/00 22:59 Riever
 * Updated removelink to work with antpath.
 * 
 * 8     27/02/00 13:08 Riever
 * Enabled sensible jump linking for humans.
 * 
 * 7     24/02/00 20:03 Riever
 * Reverse link code re-enabled.
 * 
 * 6     24/02/00 3:05 Riever
 * Updated LTK nodes version to 4
 * 
 * User: Riever       Date: 23/02/00   Time: 23:19
 * New door node creation (a node either side).
 * Nodes raised to allow better linking.
 * Temp nodes at routing only exist for 60 seconds.
 * User: Riever       Date: 23/02/00   Time: 17:24
 * Added support for 'sv shownodes on/off'
 * Enabled creation of nodes for ALL doors. (Stage 1 of new method)
 * User: Riever       Date: 21/02/00   Time: 15:16
 * Bot now has the ability to roam on dry land. Basic collision functions
 * written. Active state skeletal implementation.
 * User: Riever       Date: 20/02/00   Time: 20:27
 * Added new members and definitions ready for 2nd generation of bots.
 * 
 */

///////////////////////////////////////////////////////////////////////	
//
//  acebot_nodes.c -   This file contains all of the 
//                     pathing routines for the ACE bot.
// 
///////////////////////////////////////////////////////////////////////

#include "../g_local.h"

#include "acebot.h"
#include "botnav.h"

//rekkie -- DEV_1 -- s
// original code
//#define LTK_NODEVERSION 4
//
// modified code
//#define LTK_NODEVERSION 5 // Updated to new version 5
//rekkie -- DEV_1 -- e

// flags
qboolean newmap=true;

// Total number of nodes that are items
int numitemnodes; 

// Total number of nodes
int numnodes;

// For debugging paths
int show_path_from = INVALID;
int show_path_to = INVALID;

// array for node data
//rekkie -- DEV_1 -- s
node_t *unsorted_nodes; // Used to generate all links, so they can be sorted, then copied to nodes
edict_t *node_ents[MAX_EDICTS]; // Define the node_ents variable
int num_vis_nodes = 0; // Number of visible nodes
int node_vis[10][10]; // Cached node visibily. node_vis[X][Y] <-- can X see Y? If Y == INVALID, then false. Otherwise Y == NODE NUM
int node_vis_list[10][MAX_VIS_NODES]; // Cached node visibility list. node_vis_list[X][list-of-nodes-x-can-see]  <-- All the nodes that X can see.

//node_t *nodes = NULL; // Memory allocated nodes
//node_t nodes[MAX_PNODES];
//#define MAX_PNODES 65536 // Absolute max nodes 64k

//short int **path_table = NULL;
////short int path_table[MAX_PNODES][MAX_PNODES];

//rekkie -- DEV_1 -- e
//node_t nodes[MAX_NODES];
//short int path_table[MAX_NODES][MAX_NODES];

///////////////////////////////////////////////////////////////////////
// NODE INFORMATION FUNCTIONS
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Determin cost of moving from one node to another
///////////////////////////////////////////////////////////////////////
int ACEND_FindCost(int from, int to)
{
	// RiEvEr - Bug Hunting
	int curnode = INVALID;
	int cost=1; // Shortest possible is 1

	// If we can not get there then return invalid
	if( (from == INVALID) || (to == INVALID) || 
		(path_table[from][to] == INVALID)	)
		return INVALID;

	// Otherwise check the path and return the cost
	curnode = path_table[from][to];

	// Find a path (linear time, very fast)
	while(curnode != to)
	{
		curnode = path_table[curnode][to];
		if(curnode == INVALID) // something has corrupted the path abort
			return INVALID;
		if(cost > numnodes) // Sanity check to avoid infinite loop.
		        return INVALID;
		cost++;
	}
	
	return cost;
}

///////////////////////////////////////////////////////////////////////
// Find a close node to the player within dist.
//
// Faster than looking for the closest node, but not very 
// accurate.
///////////////////////////////////////////////////////////////////////
int ACEND_FindCloseReachableNode(edict_t *self, int range, int type)
{
	vec3_t v;
	int i;
	trace_t tr;
	float dist;
	vec3_t maxs,mins;

	VectorCopy(self->mins,mins);
	mins[2] += 16;
	VectorCopy(self->maxs,maxs);
	maxs[2] -= 16;

	range *= range;

	for(i=0;i<numnodes;i++)
	{
		//rekkie -- DEV_1 -- s
		// Prevent bots from running in circles by trying to target the same node
		if (i == self->bot.goal_node)
			continue;

		if (nodes[i].inuse == false) continue; // Ignore nodes not in use
		//rekkie -- DEV_1 -- e

		if(type == NODE_ALL || type == nodes[i].type) // check node type
		{
		
			VectorSubtract(nodes[i].origin,self->s.origin,v); // subtract first

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

			if(dist < range) // square range instead of sqrt
			{
				// make sure it is visible
				//AQ2 ADDED MASK_SOLID
				//trace = gi.trace (self->s.origin, vec3_origin, vec3_origin, nodes[i].origin, self, MASK_SOLID|MASK_OPAQUE);
				tr = gi.trace (self->s.origin, mins, maxs, nodes[i].origin, self, MASK_ALL);

				if(tr.fraction == 1.0)
					return i;
			}
		}
	}

	return -1;
}


int ACEND_DistanceToTargetNode(edict_t *self)
{
	float dist;
	//int node=-1;
	vec3_t v;

	VectorSubtract(nodes[self->bot.goal_node].origin, self->s.origin,v); // subtract first
	dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

	return dist;
}


///////////////////////////////////////////////////////////////////////
// Find the closest node to the player within a certain range
///////////////////////////////////////////////////////////////////////
int ACEND_FindClosestReachableNode(edict_t *self, int range, int type)
{
	// When updating current node, only update when we're touching the ground, 
	// otherwise if in the air it will return INVALID node because we need to be near the ground to find a node
	if (self && self->bot.current_node != INVALID && self->groundentity == NULL)
		return self->bot.current_node;

	int node = INVALID;
	//rekkie -- DEV_1 -- s
	// Try to find node using the level geometry first
	/*
	if (nmesh.total_reach)
	{
		qboolean found;
		int f = INVALID;
		int t = INVALID;
		GetBSPFaceTriangle(self->s.origin, &found, &f, &t);
		if (found && f != INVALID && t != INVALID) //f = 0; t = 0; found = true;
		{
			int n = nmesh.face[f].tris[t].node;
			//trace_t tr = gi.trace(self->s.origin, tv(-16,-16,-24), tv(16,16,32), nodes[n].origin, self, MASK_PLAYERSOLID);
			trace_t tr = gi.trace(self->s.origin, tv(-16,-16,STEPSIZE), tv(16,16,32), nodes[n].origin, self, MASK_PLAYERSOLID);
			//Com_Printf("%s -> [%s] f[%i] t[%i] node %i at %f %f %f\n", __func__, self->client->pers.netname, f, t, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);
			if (n != INVALID && tr.fraction == 1)
				node = nmesh.face[f].tris[t].node; // Successfully found node
		}
	}
	*/
	//Com_Printf("%s -> [%s] failed to GetBSPFaceTriangle\n", __func__, self->client->pers.netname);



	// Geometry lookup failed - Look for nodes by distance
	
	//rekkie -- DEV_1 -- e
	if (node == INVALID)
	{
		int i;
		float closest = 99999;
		float dist;
		vec3_t v;
		trace_t tr;
		float rng;
		vec3_t maxs, mins;

		VectorCopy(self->mins, mins);
		VectorCopy(self->maxs, maxs);
		mins[2] += 18; // Stepsize
		maxs[2] -= 16; // Duck a little.. 

		rng = (float)(range * range); // square range for distance comparison (eliminate sqrt)	

		for (i = 0; i < numnodes; i++)
		{
			if (nodes[i].inuse == false) continue; // Ignore nodes not in use

			if (type == NODE_ALL || type == nodes[i].type) // check node type
			{
				// Get Height Diff
				float height = fabs(nodes[i].origin[2] - self->s.origin[2]);
				if (height > 60) // Height difference high
					continue;

				VectorSubtract(nodes[i].origin, self->s.origin, v); // subtract first

				dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

				/*
				// Try to see if bot is close to the next node
				{
					tr = gi.trace(self->s.origin, tv(-16, -16, STEPSIZE), tv(16, 16, 32), nodes[i].origin, self, MASK_PLAYERSOLID); //rekkie
					if ((tr.fraction == 1.0) ||
						((tr.fraction > 0.9) // may be blocked by the door itself!
						&& (Q_stricmp(tr.ent->classname, "func_door_rotating") == 0))
						)
					{
						//if (node != INVALID)
							return node;
					}
				}
				else // Otherwise just closest node
				*/
				{
					if (dist < closest && dist < rng)
					{
						// make sure it is visible
						//AQ2 added MASK_SOLID
						//tr = gi.trace(self->s.origin, mins, maxs, nodes[i].origin, self, MASK_SOLID | MASK_OPAQUE);
						tr = gi.trace(self->s.origin, tv(-16, -16, STEPSIZE), tv(16, 16, 32), nodes[i].origin, self, MASK_PLAYERSOLID); //rekkie
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
		}
	}

	//Com_Printf("%s [%s] node %d\n", __func__, self->client->pers.netname, node);
	
	return node;
}

///////////////////////////////////////////////////////////////////////
// Find the closest node to the player within a certain range that doesn't have LOS to an enemy
///////////////////////////////////////////////////////////////////////
int ACEND_FindClosestReachableSafeNode(edict_t *self, int range, int type)
{
	int i;
	float closest = 99999;
	float dist;
	int node=-1;
	vec3_t v;
	trace_t tr;
	float rng;
	vec3_t maxs,mins;
	int curplayer;
	int is_safe=1;

	VectorCopy(self->mins,mins);
	VectorCopy(self->maxs,maxs);
	mins[2] += 18; // Stepsize
	maxs[2] -= 16; // Duck a little.. 

	rng = (float)(range * range); // square range for distance comparison (eliminate sqrt)	
	
	for(i=0;i<numnodes;i++)
	{
		if (nodes[i].inuse == false) continue; // Ignore nodes not in use

		if(type == NODE_ALL || type == nodes[i].type) // check node type
		{
			VectorSubtract(nodes[i].origin, self->s.origin,v); // subtract first

			dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
		
			if(dist < closest && dist < rng && dist > 200) 
			{
				// make sure it is visible
				//AQ2 added MASK_SOLID
				tr = gi.trace (self->s.origin, mins, maxs, nodes[i].origin, self, MASK_SOLID|MASK_OPAQUE);
//				tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, nodes[i].origin, self, MASK_ALL);
				if( (tr.fraction == 1.0) ||
					(	(tr.fraction > 0.9) // may be blocked by the door itself!
						&& (Q_stricmp(tr.ent->classname, "func_door_rotating") == 0)	)	
					)
				{

					is_safe=1;
					for (curplayer=0; curplayer<num_players; curplayer++)
					if( (players[curplayer]->team != self->team) && (players[curplayer]->solid != SOLID_NOT) )
					{
						tr = gi.trace (self->s.origin, tv(-8,-8,-8), tv(8,8,8), players[curplayer]->s.origin, self, MASK_SOLID|MASK_OPAQUE);
						if (tr.fraction == 1.0)
						{
							is_safe=0;
							break;
						}
					}

					if (is_safe==1)
					{
						//rekkie -- DEV_1 -- s
						// Pick a node which isn't our goal node (prevent bots going in circles trying to target the same safe node)
						if (i == self->bot.goal_node)
							break;
						//rekkie -- DEV_1 -- e
							
						node = i;
						closest = dist;
					}

				}
			}
		}
	}

	return node;
}


///////////////////////////////////////////////////////////////////////
// BOT NAVIGATION ROUTINES
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Set up the goal
///////////////////////////////////////////////////////////////////////
void ACEND_SetGoal(edict_t *self, int goal_node)
{
	self->bot.goal_node = goal_node;
	
	int nodelist[MAX_NODELIST];
	int nodes_touched;
	nodes_touched = BOTLIB_NodeTouchNodes(self->s.origin, vec3_origin, 32, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
	for (int i = 0; i < nodes_touched; i++) // Cycle through all the nodes we touched
	{
		if (nodelist[i] != INVALID)
		{
			self->bot.current_node = nodelist[i];
			break;
		}
	}

	if (self->bot.current_node == INVALID || goal_node == self->bot.current_node || goal_node == INVALID || nodes[goal_node].inuse == false)
	{
		//Com_Printf("%s %s invalid goal set: current_node[%d] goal_node[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node);

		self->bot.goal_node = INVALID;
		self->bot.state = BOT_MOVE_STATE_NAV; // BOT_MOVE_STATE_WANDER
		//self->wander_timeout = level.framenum + 0.1 * HZ;
		//ACEAI_PickLongRangeGoal(self);
		//Com_Printf("%s %s invalid goal[%i]\n", __func__, self->client->pers.netname, self->bot.goal_node);
		return;
	}

	self->bot.next_node = self->bot.current_node; // make sure we get to the nearest node first
	self->node_timeout = 0;

	//Com_Printf("%s curr[%d] goal[%d] state[%d]\n", __func__, self->bot.current_node, self->bot.goal_node, self->state); //rekkie

	return;
}

///////////////////////////////////////////////////////////////////////
// Advances to the selected node
///////////////////////////////////////////////////////////////////////
qboolean BOTLIB_AdvanceToSelectNode(edict_t* self, int node)
{
	qboolean bot_hit_next_node = false;
	if (self->bot.next_node == node)
		bot_hit_next_node = true;

	do
	{
		self->bot.prev_node = self->bot.current_node;
		self->bot.current_node = self->bot.next_node;

		// Remove the front entry from the list
		if (self->bot.next_node == SLLfront(&self->pathList))
			SLLpop_front(&self->pathList);

		// Get the next node - if there is one!
		if (!SLLempty(&self->pathList))
		{
			self->bot.next_node = SLLfront(&self->pathList);
			//return true;

			if (bot_hit_next_node) return true;
		}
		else
		{
			// We messed up...
			//Com_Printf("%s Trying to read an empty SLL nodelist!\n", __func__);
			self->bot.next_node = INVALID;
			return false;
		}
	} while (self->bot.next_node != node);

	return true;
}
//rekkie -- goto next node -- s
///////////////////////////////////////////////////////////////////////
// Advance to the next node, if any
///////////////////////////////////////////////////////////////////////
qboolean NODES_AdvanceToNextNode(edict_t* self)
{
	self->bot.prev_node = self->bot.current_node;
	self->bot.current_node = self->bot.next_node;

	// Remove the front entry from the list
	if (self->bot.next_node == SLLfront(&self->pathList))
		SLLpop_front(&self->pathList);

	// Get the next node - if there is one!
	if (!SLLempty(&self->pathList))
	{
		self->bot.next_node = SLLfront(&self->pathList);
		return true;
	}
	else
	{
		// We messed up...
		//if (debug_mode) gi.bprintf(PRINT_HIGH, "Trying to read an empty SLL nodelist!\n");
		//Com_Printf("%s Trying to read an empty SLL nodelist!\n", __func__);
		self->bot.next_node = INVALID;
		return false;
	}

	//Com_Printf("%s\n", self->client->pers.netname);
	return false;
}
//rekkie -- goto next node -- e

//rekkie -- walknodes -- s
// Cycle through node selection/function types
// Called via a_cmds.c Cmd_New_Reload_f() function. The user can press their reload key.
// Alternatively called via BOTLIB_MouseControlNodes. The user can hold down their attack key.
void BOTLIB_ChangeNodeFunction(edict_t* ent)
{
	if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_NONE)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_SELECT; // Select nodes
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Select nodes (area is [%d]) (use nav_area <num> to change selected node area)\n", "ChangeNodeFunction:", ent->bot.walknode.selection_area);
		VectorClear(ent->bot.walknode.selection_start);
		VectorClear(ent->bot.walknode.selection_end);
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_SELECT_SMART; // Smart node selection
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Smart node selection (area is [%d]) (use nav_area <num> to change selected node area)\n", "ChangeNodeFunction:", ent->bot.walknode.selection_area);
		VectorClear(ent->bot.walknode.selection_start);
		VectorClear(ent->bot.walknode.selection_end);
		BOTLIB_SetAllNodeNormals(); // Set all the normals
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT_SMART)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_ADD; // Add nodes
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Add node\n", "ChangeNodeFunction:");
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_ADD)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_LINK; // Link nodes
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Link/Unlink node\n", "ChangeNodeFunction:");
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINK)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_MOVE; // Move nodes
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Move node\n", "ChangeNodeFunction:");
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_MOVE)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_TYPE; // Change node type
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Change node type\n", "ChangeNodeFunction:");
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_TYPE)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_LINKTYPE; // Change node-to-node link type
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Change node-to-node link type\n", "ChangeNodeFunction:");
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINKTYPE)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_DEL; // Delete nodes
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Delete node\n", "ChangeNodeFunction:");
	}
	else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_DEL)
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_FLOODFILL; // Flood fill
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: Flood fill area with nodes\n", "ChangeNodeFunction:");
	}
	else
	{
		ent->bot.walknode.highlighted_node_type = HIGHLIGHTED_NODE_NONE; // Disable interaction with nodes
		gi.cprintf(ent, PRINT_MEDIUM, "%s Node selection changed to: No interaction with nodes\n", "ChangeNodeFunction:");
	}

	// Reset highlighted nodes
	ent->bot.walknode.highlighted_node = INVALID;
	ent->bot.walknode.prev_highlighted_node = INVALID;
}

// Self expanding nodes attempt to expand out from the source node.
void BOTLIB_SelfExpandingNodes(edict_t* ent, int node)
{
	trace_t tr;
	int node_added; // Last node added
	vec3_t exp; // Expand
	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };

	/*
	// Keep a copy of all the nodes added - Max 4096 nodes
	int node_counter = 0;
	int * nodes_added = (int *)malloc(sizeof(int) * 4096);
	if (nodes_added == NULL) // Failed to allocate memory
		return;
	nodes_added[node_counter++] = node; // Add the initial node
	*/

	int curr_node = node; // Current node in the list

	botlib_sll_t openList; // SLL openlist
	openList.head = openList.tail = NULL; // Nullify the list ends

	SLLpush_back(&openList, node); // Current node in the list

	int nodes_touched; // Number of nodes touched
	int nodelist[MAX_NODELIST]; // Nodes touched

	// While there are nodes on the OPEN list
	while (SLLempty(&openList) == false)
	{
		// Get next node from the list
		curr_node = SLLfront(&openList);

		// Safety check
		if (curr_node <= INVALID)
			break;

		const byte multi = 3; // Multiplier for expanding nodes
		for (int i = 0; i < 8; i++) // Cycle through all 8 directions
		{
			VectorCopy(nodes[curr_node].origin, exp);
			if (i == 0)
				exp[0] += (mins[0] * multi); // Move left
			else if (i == 1)
				exp[0] += (maxs[0] * multi); // Move right
			else if (i == 2)
				exp[1] += (mins[1] * multi); // Move forward
			else if (i == 3)
				exp[1] += (maxs[1] * multi); // Move back

			else if (i == 4) // Move left and forward
			{
				exp[0] += (mins[0] * multi);
				exp[1] += (mins[1] * multi);
			}
			else if (i == 5) // Move right and forward
			{
				exp[0] += (maxs[0] * multi);
				exp[1] += (mins[1] * multi);
			}
			else if (i == 6) // Move left and back
			{
				exp[0] += (mins[0] * multi);
				exp[1] += (maxs[1] * multi);
			}
			else if (i == 7) // Move right and back
			{
				exp[0] += (maxs[0] * multi);
				exp[1] += (maxs[1] * multi);
			}

			/*
			else if (i == 4)
			{
				exp[0] += (mins[0] * 4);
				exp[1] += (maxs[0] * 4);
			}
			else if (i == 5)
			{
				exp[0] += (mins[0] * 4);
				exp[1] += (maxs[1] * 4);
			}
			else if (i == 6)
			{
				exp[0] += (mins[1] * 4);
				exp[1] += (maxs[0] * 4);
			}
			else if (i == 7)
			{
				exp[0] += (mins[1] * 4);
				exp[1] += (maxs[1] * 4);
			}
			*/

			if (0)
			{
				// Check if new location touches another node
				nodes_touched = BOTLIB_NodeTouchNodes(exp, vec3_origin, 8, mins, maxs, nodelist, MAX_NODELIST, INVALID);
				if (nodes_touched == 0)
				{
					// Check if we fit
					tr = gi.trace(exp, mins, maxs, exp, ent, MASK_PLAYERSOLID);
					if (tr.startsolid == false)
					{
						// Try crawling down
						tr = gi.trace(exp, mins, maxs, tv(exp[0], exp[1], exp[2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
						if (tr.startsolid || tr.fraction == 1.0 || tr.plane.normal[2] < 0.7) // 
							continue; // Skip
					}
					else // Try crawling up
					{
						continue;
					}

					nodes_touched = BOTLIB_NodeTouchNodes(tr.endpos, tr.plane.normal, 0, mins, maxs, nodelist, MAX_NODELIST, INVALID);
					if (nodes_touched == 0)
					{
						// Contents check
						int type = NODE_MOVE;
						int contents = gi.pointcontents(tv(tr.endpos[0], tr.endpos[1], tr.endpos[2] - 23));
						if (contents & MASK_WATER)
							type = NODE_WATER;

						node_added = BOTLIB_AddNode(tr.endpos, tr.plane.normal, type);
						if (node_added != INVALID)
						{
							// Add node to the openlist
							SLLpush_back(&openList, node_added);
							//nodes_added[node_counter++] = node_added;

							//char typename[32] = { '\0' }; // Length of the longest node type name
							//NodeTypeToString(ent, nodes[node_added].type, typename, sizeof(typename));
							//Com_Printf("%s %s added node [%d] type [%s]\n", __func__, ent->client->pers.netname, node_added, typename);
						}
					}
				}
			}

			if (1)
			{
				exp[2] += 0.01; // Move up a little

				// Check if we fit
				tr = gi.trace(exp, mins, maxs, exp, ent, MASK_PLAYERSOLID);
				if (tr.startsolid == false)
				{
					// Try crawling down
					tr = gi.trace(exp, mins, maxs, tv(exp[0], exp[1], exp[2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
					if (tr.startsolid || tr.plane.normal[2] < 0.7 || tr.surface->flags & SURF_SKY)
						continue;
				}
				else
				{
					// Try crawling up
					exp[2] += 24; // 57
					tr = gi.trace(exp, mins, maxs, exp, ent, MASK_PLAYERSOLID);
					if (tr.startsolid)
					{
						// Try crawling up more
						exp[2] += 24;
						tr = gi.trace(exp, mins, maxs, exp, ent, MASK_PLAYERSOLID);
						if (tr.startsolid)
						{
							// Try crawling up once more
							exp[2] += 16;
							tr = gi.trace(exp, mins, maxs, exp, ent, MASK_PLAYERSOLID);
							if (tr.startsolid)
								continue;
						}
					}

					// And then crawl back down
					tr = gi.trace(exp, mins, maxs, tv(exp[0], exp[1], exp[2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
					if (tr.startsolid || tr.plane.normal[2] < 0.7 || tr.surface->flags & SURF_SKY)
						continue;
				}

				// Ignore nodes that are too far on an edge
				trace_t tr_edge = gi.trace(tr.endpos, NULL, NULL, tv(tr.endpos[0], tr.endpos[1], tr.endpos[2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
				if (tr_edge.fraction == 1.0 || tr_edge.surface->flags & SURF_SKY)
					continue;

				/*
				// Check for ground -60 units
				tr = gi.trace(exp, NULL, NULL, tv(exp[0], exp[1], exp[2] - (24 + NODE_MAX_JUMP_HEIGHT)), ent, MASK_PLAYERSOLID);
				if (tr.startsolid || tr.fraction == 1.0) // Inside solid, or didn't touch the ground
					continue; // Skip
				//tr.endpos[2] += 24;

				// If the node is on a slope, raise it up depending on the slope normal and the node's mins/maxs hit box
				// This is done so the node hitbox is not inside the slope face
				{
					vec3_t exp_up;
					VectorCopy(tr.plane.normal, exp_up);
					exp_up[2] = 0;
					VectorNormalize(exp_up);
					exp_up[2] = 1;
					VectorScale(exp_up, 24, exp_up);
					VectorAdd(tr.endpos, exp_up, tr.endpos);
				}

				// Check if location can fit a node
				tr = gi.trace(tr.endpos, tv(-16, -16, -24), tv(16, 16, 32), tr.endpos, ent, MASK_PLAYERSOLID);
				if (tr.startsolid) // Inside solid, or didn't touch the ground
					continue; // Skip
				*/

				/*
				// Slime, lava, trigger_hurt, or skybox
				//trace_t tr_avoid = gi.trace(exp, VEC_ORIGIN, VEC_ORIGIN, tv(exp[0], exp[1], exp[2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID | MASK_DEADLY);
				trace_t tr_avoid = gi.trace(tv(exp[0], exp[1], exp[2]), VEC_ORIGIN, VEC_ORIGIN, tv(exp[0], exp[1], exp[2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID | MASK_DEADLY);
				if (tr_avoid.contents & MASK_DEADLY)
				{
					Com_Printf("%s %s ignoring MASK_DEADLY\n", __func__, ent->client->pers.netname);
					continue; // Skip
				}
				if (tr_avoid.ent && (tr_avoid.ent->touch == hurt_touch))
				{
					Com_Printf("%s %s ignoring hurt_touch\n", __func__, ent->client->pers.netname);
					continue; // Skip
				}
				if (tr_avoid.surface && (tr_avoid.surface->flags & SURF_SKY))
				{
					Com_Printf("%s %s ignoring SURF_SKY\n", __func__, ent->client->pers.netname);
					continue; // Skip
				}
				*/

				// Check if new location touches another node
				nodes_touched = BOTLIB_NodeTouchNodes(tr.endpos, tr.plane.normal, 0, mins, maxs, nodelist, MAX_NODELIST, INVALID); // 8
				if (nodes_touched == 0)
				{
					// Contents check
					int type = NODE_MOVE;
					int contents = gi.pointcontents(tv(tr.endpos[0], tr.endpos[1], tr.endpos[2] - 23));
					if (contents & MASK_WATER)
						type = NODE_WATER;

					node_added = BOTLIB_AddNode(tr.endpos, tr.plane.normal, type);
					if (node_added != INVALID)
					{
						// Add node to the openlist
						SLLpush_back(&openList, node_added);
						//nodes_added[node_counter++] = node_added;

						//char typename[32] = { '\0' }; // Length of the longest node type name
						//NodeTypeToString(ent, nodes[node_added].type, typename, sizeof(typename));
						//Com_Printf("%s %s added node [%d] type [%s]\n", __func__, ent->client->pers.netname, node_added, typename);
					}
				}
			}
		}

		// Remove current node from the list
		SLLpop_front(&openList);
	}

	// Free up the memory we allocated
	SLLdelete(&openList);

	
	/*
	// Link all the added nodes together
	int from, to;
	float node_to_node_dist;
	if (node_counter + 1 < 4096)
	for (int i = 0; i < node_counter; i++)
	{
		for (int j = 0; j < node_counter; j++)
		{
			if (i == j)
				continue;

			from = nodes_added[i];
			to = nodes_added[j];
			node_to_node_dist = VectorDistance(nodes[from].origin, nodes[to].origin);

			if (node_to_node_dist <= 128)
			{
				tr = gi.trace(nodes[from].origin, tv(-16, -16, -24), tv(16, 16, 32), nodes[to].origin, ent, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0) // Don't link if we can't see the node
				{
					if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
					{
						if (BOTLIB_AddNodeLink(from, to, NODE_MOVE, true))
						{
							//Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, from, to, node_type);
							ACEND_UpdateNodeReach(from, to); // Update path_table
						}
					}
				}
			}
		}
	}

	if (nodes_added)
		free(nodes_added);
	*/
}

// Links
void BOTLIB_LinkNodesNearbyNode(edict_t* ent, int from)
{
	trace_t tr;
	int to, type;

	for (int j = 0; j < numnodes; j++)
	{
		if (from == j)	continue; // Skip self

		to = nodes[j].nodenum;

		if (to == INVALID) continue; // Skip invalid
		//if (nodes[to].num_links >= 8) continue; // Skip checking nodes with 8+ links

		if (VectorDistance(nodes[from].origin, nodes[to].origin) >= 96) continue;

		if (fabs(nodes[from].origin[2] - nodes[to].origin[2]) <= NODE_MAX_JUMP_HEIGHT)
		{
			//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
			{
				tr = gi.trace(nodes[from].origin, tv(-16, -16, -12), tv(16, 16, 32), nodes[to].origin, ent, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0) // Don't link if we can't see the node
				{
					type = BOTLIB_Reachability(from, to);
					if (type != NODE_CROUCH && type != NODE_JUMPPAD)
					{
						type = NODE_MOVE;

						// Contents check
						int contents = gi.pointcontents(tv(nodes[from].origin[0], nodes[from].origin[1], nodes[from].origin[2]));
						if (contents & MASK_WATER)
							type = NODE_WATER;
					}
					else
						continue;

					//if (type != NODE_CROUCH && type != NODE_JUMPPAD)
					{
						if (BOTLIB_AddNodeLink(from, to, type, true))
						{
							Com_Printf("%s linking node[%d -> %d] type[%d]\n", __func__, from, to, type);

							// Add reverse link
							if (BOTLIB_AddNodeLink(to, from, type, true))
								Com_Printf("%s linking node[%d -> %d] type[%d]\n", __func__, from, to, type);
						}
					}
				}
			}
		}
	}
}

void BOTLIB_LinkAllNodesTogether(edict_t* ent)
{
	trace_t tr;
	int from, to, type;
	//float height_diff;
	//float node_to_node_dist_xyz;
	//float node_to_node_dist_xy;
	for (int i = 0; i < numnodes; i++)
	{
		from = nodes[i].nodenum;
		if (from == INVALID) continue; // Skip invalid
		if (nodes[from].num_links >= 8) continue; // Skip checking nodes with 8+ links

		for (int j = 0; j < numnodes; j++)
		{
			if (i == j)	continue; // Skip self

			to = nodes[j].nodenum;
			if (to == INVALID) continue; // Skip invalid
			if (nodes[to].num_links >= 8) continue; // Skip checking nodes with 8+ links

			//if ( (node_to_node_dist_xyz = VectorDistance(nodes[from].origin, nodes[to].origin)) > 256) continue; // Skip checking distant nodes
			if (VectorDistance(nodes[from].origin, nodes[to].origin) >= 96) continue; 
			//node_to_node_dist_xy = VectorDistanceXY(nodes[from].origin, nodes[to].origin);

			/*
			height_diff = fabs(nodes[from].origin[2] - nodes[to].origin[2]);

			qboolean target_is_above = false, target_is_below = false;
			float higher = 0, lower = 0;
			if (nodes[from].origin[2] > nodes[to].origin[2]) // We're above the target
			{
				target_is_below = true;
				lower = (nodes[from].origin[2] - nodes[to].origin[2]);
			}
			else if (nodes[from].origin[2] < nodes[to].origin[2]) // We're below the target
			{
				target_is_above = true;
				higher = (nodes[to].origin[2] - nodes[from].origin[2]);
			}
			*/

			//if (node_to_node_dist_xyz <= 128 && height_diff <= NODE_MAX_JUMP_HEIGHT)
			if (fabs(nodes[from].origin[2] - nodes[to].origin[2]) <= NODE_MAX_JUMP_HEIGHT)
			{
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					tr = gi.trace(nodes[from].origin, tv(-16, -16, -12), tv(16, 16, 32), nodes[to].origin, ent, MASK_PLAYERSOLID);
					if (tr.fraction == 1.0) // Don't link if we can't see the node
					{
						type = BOTLIB_Reachability(from, to);
						if (type != NODE_CROUCH && type != NODE_JUMPPAD)
						{
							type = NODE_MOVE;

							// Contents check
							int contents = gi.pointcontents(tv(nodes[from].origin[0], nodes[from].origin[1], nodes[from].origin[2]));
							if (contents & MASK_WATER)
								type = NODE_WATER;
						}
						else
							continue;

						//if (type != NODE_CROUCH && type != NODE_JUMPPAD)
						{
							if (BOTLIB_AddNodeLink(from, to, type, true))
							{
								//Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, from, to, node_type);
								////ACEND_UpdateNodeReach(from, to); // Update path_table
							}
						}
					}
				}
			}
			/*
			// Link crouching nodes
			else if (node_to_node_dist_xy <= 64 && target_is_below && height_diff <= NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE) // 192
			{
				tr = gi.trace(nodes[from].origin, tv(-16, -16, 31), tv(16, 16, 32), nodes[to].origin, ent, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0) // Don't link if we can't see the node
				{
					// Contents check
					int contents = gi.pointcontents(tv(nodes[from].origin[0], nodes[from].origin[1], nodes[from].origin[2] - 32));
					if (contents & MASK_WATER)
						continue;

					if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
					{
						if (BOTLIB_Reachability(from, to) == NODE_CROUCH)
						if (BOTLIB_AddNodeLink(from, to, NODE_CROUCH, true))
						{
							//Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, from, to, node_type);
							////ACEND_UpdateNodeReach(from, to); // Update path_table
						}
					}
				}
			}
			// Link jumppad nodes
			else if (node_to_node_dist_xy <= 256 && target_is_above && height_diff <= NODE_MAX_JUMP_HEIGHT)
			{
				tr = gi.trace(nodes[from].origin, tv(-16, -16, 31), tv(16, 16, 32), nodes[to].origin, ent, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0) // Don't link if we can't see the node
				{
					// Contents check
					int contents = gi.pointcontents(tv(nodes[from].origin[0], nodes[from].origin[1], nodes[from].origin[2] - 32));
					if (contents & MASK_WATER)
						continue;

					if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
					{
						if (BOTLIB_Reachability(from, to) == NODE_JUMPPAD)
						if (BOTLIB_AddNodeLink(from, to, NODE_JUMPPAD, true))
						{
							//Com_Printf("%s %s linking node[%d -> %d] type[%d]\n", __func__, ent->client->pers.netname, from, to, node_type);
							////ACEND_UpdateNodeReach(from, to); // Update path_table
						}
					}
				}
			}
			*/
		}
	}
}

int SortByHeight(const void* a, const void* b)
{
	float vecA[3] = { ((float*)a)[0], ((float*)a)[1], ((float*)a)[2] }; // typecast the void pointers to vec3_t pointers
	float vecB[3] = { ((float*)b)[0], ((float*)b)[1], ((float*)b)[2] }; // typecast the void pointers to vec3_t pointers
	
	if (vecA[2] < vecB[2]) { // If the first vector is lower than the second vector in the Z-axis, it needs to be sorted to a higher position in the array.
		return 1; // Return positive value signifying we want to move vecA higher in the array.
	}
	else if (vecA[2] > vecB[2]) { // If the first vector is higher than the second vector in the Z-axis, it needs to be sorted to a lower position in the array.
		return -1; // Return negative value signifying we want to move vecA lower in the array.
	}
	else {
		return 0; // Return 0 if the two vectors are of the same height.
	}
}

// Self expand nodes from all spawn points
void BOTLIB_SelfExpandNodesFromSpawnpoints(edict_t *ent)
{
	trace_t tr;
	int nodelist[MAX_NODELIST];
	int nodes_touched;
	int node_added;
	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };

	int sp_counter = 0;
	vec3_t sp_origin[128];
	edict_t* spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		VectorCopy(spot->s.origin, sp_origin[sp_counter]);

		// Try crawling down
		sp_origin[sp_counter][2] += 0.01;
		tr = gi.trace(sp_origin[sp_counter], mins, maxs, tv(sp_origin[sp_counter][0], sp_origin[sp_counter][1], sp_origin[sp_counter][2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
		if (tr.startsolid)
			continue;
		else
			sp_origin[sp_counter][2] = tr.endpos[2];

		sp_counter++;
	}
	while ((spot = G_Find(spot, FOFS(classname), "info_player_team1")) != NULL)
	{
		VectorCopy(spot->s.origin, sp_origin[sp_counter]);

		// Try crawling down
		sp_origin[sp_counter][2] += 0.01;
		tr = gi.trace(sp_origin[sp_counter], mins, maxs, tv(sp_origin[sp_counter][0], sp_origin[sp_counter][1], sp_origin[sp_counter][2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
		if (tr.startsolid)
			continue;
		else
			sp_origin[sp_counter][2] = tr.endpos[2];

		sp_counter++;
	}
	while ((spot = G_Find(spot, FOFS(classname), "info_player_team2")) != NULL)
	{
		VectorCopy(spot->s.origin, sp_origin[sp_counter]);

		// Try crawling down
		sp_origin[sp_counter][2] += 0.01;
		tr = gi.trace(sp_origin[sp_counter], mins, maxs, tv(sp_origin[sp_counter][0], sp_origin[sp_counter][1], sp_origin[sp_counter][2] - NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE), ent, MASK_PLAYERSOLID);
		if (tr.startsolid)
			continue;
		else
			sp_origin[sp_counter][2] = tr.endpos[2];

		sp_counter++;
	}

	// Qsort in order of highest to lowest z value
	qsort(sp_origin, sp_counter, sizeof(vec3_t), SortByHeight);

	// Show custom user spawn points
	//if (dc_sp_count && dc_sp != NULL)
	{
		//for (int i = 0; i < dc_sp_count; i++)
		for (int i = 0; i < sp_counter; i++)
		{
			//if (dc_sp[i].inuse) // Ignore deleted spawn spots
			{
				/*
				// Add node to spawn point - ensure it touched the ground and not touching another node
				trace_t tr = gi.trace(tv(sp_origin[i][0], sp_origin[i][1], sp_origin[i][2] + 1), mins, maxs, tv(sp_origin[i][0], sp_origin[i][1], sp_origin[i][2] - 2), ent, MASK_PLAYERSOLID);
				if (tr.fraction == 1) // If up in the air (like one of the teamjungle spawnpoints)
				{
					// Find ground
					tr = gi.trace(sp_origin[i], mins, maxs, tv(sp_origin[i][0], sp_origin[i][1], sp_origin[i][2] - 256), ent, MASK_PLAYERSOLID);
					if (tr.fraction < 1 && tr.startsolid == false)
					{
						// Check if new location touches another node
						nodes_touched = BOTLIB_NodeTouchNodes(tr.endpos, tr.plane.normal, 0, mins, maxs, nodelist, MAX_NODELIST, INVALID);
						if (nodes_touched == 0)
						{
							node_added = BOTLIB_AddNode(tr.endpos, tr.plane.normal, NODE_MOVE);
							if (node_added != INVALID)
								BOTLIB_SelfExpandingNodes(NULL, node_added);
						}
					}
				}
				else // Most spawn points need no adjustment; node can use spawn origin :)
				*/
				{
					// Check if new location touches another node
					nodes_touched = BOTLIB_NodeTouchNodes(sp_origin[i], vec3_origin, 0, mins, maxs, nodelist, MAX_NODELIST, INVALID);
					if (nodes_touched == 0)
					{
						// Contents check
						int type = NODE_MOVE;
						int contents = gi.pointcontents(tv(sp_origin[i][0], sp_origin[i][1], sp_origin[i][2] - 24));
						if (contents & MASK_WATER)
							type = NODE_WATER;

						node_added = BOTLIB_AddNode(sp_origin[i], vec3_origin, type);
						if (node_added != INVALID)
							BOTLIB_SelfExpandingNodes(ent, node_added);
					}
				}
			}
		}
	}
}

//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
// Use mouse to control node: link, move, add, delete, etc
void BOTLIB_MouseControlNodes(edict_t* ent, usercmd_t* ucmd)
{
	const int grid_size = 2; // When adding/moving nodes, align their XY coordinates to a grid
	int nodelist[MAX_NODELIST];
	int nodes_touched;
	char type_name[64] = { '\0' }; // Length of the node type name
	int from, to;

	int i;
	int latched_buttons = ent->client->latched_buttons;
	int oldbuttons = ent->client->buttons;
	int buttons = ucmd->buttons;
	latched_buttons |= buttons & ~oldbuttons;
	
	/*
	// Press and hold down the attack button to change highlight functionality
	// This is an alternative to pressing the 'reload' key
	if (ucmd->buttons & BUTTON_ATTACK)
	{
		ent->bot.walknode.highlighted_counter++; // Counter

		// Reset highlighted_counter
		if (ent->bot.walknode.highlighted_time < level.framenum)
		{
			ent->bot.walknode.highlighted_time = level.framenum + 1 * HZ;
			ent->bot.walknode.highlighted_counter = 0;
		}

		if (ent->bot.walknode.highlighted_counter >= 50) // Only change functionality once every 50 frames (0.5 seconds)
		{
			// Reset timer and counter
			ent->bot.walknode.highlighted_time = level.framenum + 1 * HZ;
			ent->bot.walknode.highlighted_counter = 0;

			// Reset highlighted nodes
			ent->bot.walknode.highlighted_node = INVALID;
			ent->bot.walknode.prev_highlighted_node = INVALID;

			// Change selection type
			BOTLIB_ChangeNodeFunction(ent);
		}
	}
	*/

	// Highlight type is move and we've got a node selected. Show an example of where the node will be, if successfully moved
	if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_MOVE && ent->bot.walknode.highlighted_node != INVALID)
	{
		int node = ent->bot.walknode.highlighted_node;
		void(*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time) = ent->client->pers.draw->DrawBox;


		// Project trace from the player weapon POV
		vec3_t      start, end;
		vec3_t      forward, right;
		vec3_t      offset;
		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 0, ent->viewheight);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		VectorMA(start, 8192, forward, end);
		trace_t tr;
		tr = gi.trace(start, nodes[node].mins, nodes[node].maxs, end, ent, MASK_SOLID);

		// Try to align XY to a grid, otherwise just use the hit location
		int iorigin = 0; // Int based origin vec
		vec3_t grid_origin;
		VectorCopy(tr.endpos, grid_origin);
		iorigin = (int)grid_origin[0]; // Drop the decimal
		iorigin = iorigin - (iorigin % grid_size); // Align to grid
		grid_origin[0] = (float)iorigin; // Convert back to float
		iorigin = (int)grid_origin[1]; // Drop the decimal
		iorigin = iorigin - (iorigin % grid_size); // Align to grid
		grid_origin[1] = (float)iorigin; // Convert back to float
		tr = gi.trace(grid_origin, nodes[node].mins, nodes[node].maxs, grid_origin, ent, MASK_PLAYERSOLID);
		if (tr.startsolid || tr.fraction < 1.0) // Failed to find grid location, so just use hit location
			VectorCopy(tr.endpos, grid_origin);

		// Check if new location touches another node
		nodes_touched = BOTLIB_NodeTouchNodes(grid_origin, tr.plane.normal, 0, nodes[node].mins, nodes[node].maxs, nodelist, MAX_NODELIST, node);
		if (nodes_touched == 0) // Position OKAY
		{
			DrawBox(node, grid_origin, MakeColor(0, 255, 0, 255), nodes[node].mins, nodes[node].maxs, 200); // Draw node box

			VectorCopy(grid_origin, nodes[node].origin); // Move node

			VectorAdd(nodes[node].origin, nodes[node].mins, nodes[node].absmin); // Update absolute box min/max in the world
			VectorAdd(nodes[node].origin, nodes[node].maxs, nodes[node].absmax); // Update absolute box min/max in the world
		}
		else
			DrawBox(node, grid_origin, MakeColor(255, 0, 0, 255), nodes[node].mins, nodes[node].maxs, 200); // Draw RED node box

		/*
		if (tr.startsolid) // Position BAD
			DrawBox(node, grid_origin, MakeColor(255, 0, 0, 255), nodes[node].mins, nodes[node].maxs, 200); // Draw RED node box
		else if (nodes_touched == 0) // Position OKAY
		{
			DrawBox(node, grid_origin, MakeColor(0, 255, 0, 255), nodes[node].mins, nodes[node].maxs, 200); // Draw node box

			VectorCopy(grid_origin, nodes[node].origin); // Move node

			VectorAdd(nodes[node].origin, nodes[node].mins, nodes[node].absmin); // Update absolute box min/max in the world
			VectorAdd(nodes[node].origin, nodes[node].maxs, nodes[node].absmax); // Update absolute box min/max in the world
		}
		*/
	}

	/*
	// Select
	if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT)
	{
		if ((latched_buttons & BUTTON_ATTACK) == 0)
		{
			if (VectorEmpty(ent->bot.walknode.selection_start) == false && VectorEmpty(ent->bot.walknode.selection_end) == true)
			{
				// Project trace from the player weapon POV
				vec3_t      start, end;
				vec3_t      forward, right;
				vec3_t      offset;
				AngleVectors(ent->client->v_angle, forward, right, NULL);
				VectorSet(offset, 0, 0, ent->viewheight);
				G_ProjectSource(ent->s.origin, offset, forward, right, start);
				VectorMA(start, 8192, forward, end);
				trace_t tr;
				tr = gi.trace(start, NULL, NULL, end, ent, MASK_PLAYERSOLID);

				VectorCopy(tr.endpos, ent->bot.walknode.selection_start); // Set the end point
			}
		}
	}
	*/

	if (ucmd->buttons & BUTTON_ATTACK) // When attack button is held down (keeps triggering while depressed)
	{
		// Project trace from the player weapon POV
		vec3_t      start, end;
		vec3_t      forward, right;
		vec3_t      offset;
		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 0, ent->viewheight);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		VectorMA(start, 8192, forward, end);
		trace_t tr;
		//tr = gi.trace(start, tv(-16,-16,-24), tv(16,16,32), end, ent, MASK_PLAYERSOLID);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_PLAYERSOLID);

		tr.endpos[2] += 28; // half the height of a node - [player size (32 + 24) / 2] which is [56 / 2] == 28;

		// Select
		if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT || ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT_SMART)
		{
			ent->bot.walknode.selection_node_count = 0;

			if (latched_buttons & BUTTON_ATTACK) // When attack button is initially depressed (single trigger)
			{

				VectorCopy(tr.endpos, ent->bot.walknode.selection_start); // Set the selection start point

				// Reset highest and lowest points selected
				ent->bot.walknode.selection_max = -99999;
				ent->bot.walknode.selection_min = 99999;

				// Remember the first node selected
				//ent->bot.walknode.selection_node_first = INVALID;
			}
			else // When attack button is held down (keeps triggering while depressed)
			{
				VectorCopy(tr.endpos, ent->bot.walknode.selection_end); // Set the selection end point

				// Set the min/max - the lowest and highest selection
				if (tr.endpos[2] > ent->bot.walknode.selection_max)
					ent->bot.walknode.selection_max = tr.endpos[2];
				else if (tr.endpos[2] < ent->bot.walknode.selection_min)
					ent->bot.walknode.selection_min = tr.endpos[2];

				// Update absolute box min/max in the world
				vec3_t absmin, absmax;
				if (ent->bot.walknode.selection_start[0] < ent->bot.walknode.selection_end[0])
				{
					absmin[0] = ent->bot.walknode.selection_start[0];
					absmax[0] = ent->bot.walknode.selection_end[0];
				}
				else
				{
					absmin[0] = ent->bot.walknode.selection_end[0];
					absmax[0] = ent->bot.walknode.selection_start[0];
				}
				if (ent->bot.walknode.selection_start[1] < ent->bot.walknode.selection_end[1])
				{
					absmin[1] = ent->bot.walknode.selection_start[1];
					absmax[1] = ent->bot.walknode.selection_end[1];
				}
				else
				{
					absmin[1] = ent->bot.walknode.selection_end[1];
					absmax[1] = ent->bot.walknode.selection_start[1];
				}

				/*
				if (ent->bot.walknode.selection_start[2] < ent->bot.walknode.selection_end[2])
				{
					absmin[2] = ent->bot.walknode.selection_start[2];
					absmax[2] = ent->bot.walknode.selection_end[2];
				}
				else
				{
					absmin[2] = ent->bot.walknode.selection_end[2];
					absmax[2] = ent->bot.walknode.selection_start[2];
				}
				*/

				if (ent->bot.walknode.selection_min > 99990 || ent->bot.walknode.selection_max < -99990)
				{
					absmin[2] = ent->bot.walknode.selection_start[2];
					absmax[2] = ent->bot.walknode.selection_start[2];
				}
				else
				{
					absmin[2] = ent->bot.walknode.selection_min;
					absmax[2] = ent->bot.walknode.selection_max;
				}
			

				//absmin[2] -= NODE_SIZE;
				//absmax[2] += NODE_SIZE;

				// Remember the first node selected
				ent->bot.walknode.selection_node_first = INVALID;
				{
					vec3_t absmin_2, absmax_2;
					VectorCopy(ent->bot.walknode.selection_start, absmin_2);
					VectorCopy(ent->bot.walknode.selection_start, absmax_2);
					absmin_2[0] -= 16;
					absmin_2[1] -= 16;
					//absmin_2[2] -= NODE_SIZE;
					
					absmax_2[0] += 16;
					absmax_2[1] += 16;
					//absmax_2[2] += NODE_SIZE;

					for (int i = 0; i < numnodes; i++)
					{
						if (nodes[i].inuse == false) continue; // Ignore nodes not in use

						if (BOTLIB_BoxIntersection(nodes[i].absmin, nodes[i].absmax, absmin_2, absmax_2)) // Do boxes intersect?
						{
							//if (nodes[i].area > 0)
							{
								ent->bot.walknode.selection_node_first = i;
								//Com_Printf("%s %s [area: %d]\n", __func__, ent->client->pers.netname, nodes[i].area);
								break;
							}
						}
					}
				}

				if (ent->bot.walknode.selection_node_first != INVALID && ent->bot.walknode.selection_min < 99998 && ent->bot.walknode.selection_max > -99998)
				{
					// Copy all the nodes we're selecting
					for (int i = 0; i < numnodes; i++)
					{
						if (nodes[i].inuse == false) continue; // Ignore nodes not in use

						if (BOTLIB_BoxIntersection(nodes[i].absmin, nodes[i].absmax, absmin, absmax)) // Do boxes intersect?
						{
							if (ent->bot.walknode.selection_node_count >= MAX_NAV_AREAS_NODES)
								break;

							// Check if node already included in selection
							qboolean added = false;
							for (int s = 0; s < ent->bot.walknode.selection_node_count; s++)
							{
								if (ent->bot.walknode.selection_nodes[s] == i)
								{
									added = true;
									break;
								}
							}
							if (added)
								continue; // Skip adding because we already have it

							ent->bot.walknode.selection_nodes[ent->bot.walknode.selection_node_count] = i;
							ent->bot.walknode.selection_node_count++;

							if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_SELECT_SMART)
							{
								BOTLIB_GroupConnectedNodeArea(ent, ent->bot.walknode.selection_nodes[ent->bot.walknode.selection_node_count - 1]);
							}
						}
					}
				}

				// Inform user of how many nodes are selected - only update upon change
				if (ent->bot.walknode.selection_node_count && ent->bot.walknode.selected_node_count_prev != ent->bot.walknode.selection_node_count)
				{
					ent->bot.walknode.selected_node_count_prev = ent->bot.walknode.selection_node_count; // Update change

					if (ent->bot.walknode.selection_node_first == INVALID)
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s nodes selected %d [area: INVALID]\n", __func__, ent->client->pers.netname, ent->bot.walknode.selection_node_count);
					else
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s nodes selected %d [area: %d]\n", __func__, ent->client->pers.netname, ent->bot.walknode.selection_node_count, nodes[ent->bot.walknode.selection_node_first].area);
				}
			}
		}
	}

	if (latched_buttons & BUTTON_ATTACK)
	{
		// Project trace from the player weapon POV
		vec3_t      start, end;
		vec3_t      forward, right;
		vec3_t      offset;
		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 0, ent->viewheight);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		VectorMA(start, 8192, forward, end);
		trace_t tr;
		//tr = gi.trace(start, tv(-16,-16,-24), tv(16,16,32), end, ent, MASK_PLAYERSOLID);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_PLAYERSOLID);

		// Check if we're touching a node at the traced endpos
		int node = BOTLIB_TraceNodeBoxLine(ent->s.origin, tr.endpos, VEC_ORIGIN, VEC_ORIGIN);
		if (node != INVALID)
		{
			// Move
			if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_MOVE && ent->bot.walknode.highlighted_node == INVALID)
			{
				ent->bot.walknode.highlighted_node = node;
				return;
			}

			// Delete
			else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_DEL && ent->bot.walknode.highlighted_node == INVALID)
			{
				ent->bot.walknode.highlighted_node = node;
			}

			// Link type
			else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINKTYPE)
			{
				if (ent->bot.walknode.highlighted_node == INVALID)
				{
					ent->bot.walknode.highlighted_node = node;
					return;
				}
				if (ent->bot.walknode.prev_highlighted_node == INVALID)
				{
					if (ent->bot.walknode.highlighted_node == node) // Same node selected twice
					{
						// Unselect nodes
						ent->bot.walknode.highlighted_node = INVALID;
						ent->bot.walknode.prev_highlighted_node = INVALID;
						return;
					}

					ent->bot.walknode.prev_highlighted_node = ent->bot.walknode.highlighted_node;
					ent->bot.walknode.highlighted_node = node;

					// When first selecting the link type: inform the player of the current link type
					from = ent->bot.walknode.prev_highlighted_node;
					to = ent->bot.walknode.highlighted_node;

					// Check if a link exists between the two nodes
					for (i = 0; i < nodes[from].num_links; i++)
					{
						if (nodes[from].links[i].targetNode == to)
						{
							// Print curent link type
							NodeTypeToString(ent, nodes[from].links[i].targetNodeType, type_name, sizeof(type_name));
							gi.cprintf(ent, PRINT_MEDIUM, "%s %s current link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);
							return;
						}
					}
				}
			}

			// Link
			else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINK)
			{
				if (ent->bot.walknode.highlighted_node == INVALID)
				{
					ent->bot.walknode.highlighted_node = node;
					return;
				}
				if (ent->bot.walknode.prev_highlighted_node == INVALID)
				{
					if (ent->bot.walknode.highlighted_node == node) // Same node selected twice
					{
						// Unselect nodes
						ent->bot.walknode.highlighted_node = INVALID;
						ent->bot.walknode.prev_highlighted_node = INVALID;
						return;
					}

					ent->bot.walknode.prev_highlighted_node = ent->bot.walknode.highlighted_node;
					ent->bot.walknode.highlighted_node = node;
				}
			}

			// When first selecting the node type: inform the player of the current node type
			if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_TYPE && ent->bot.walknode.highlighted_node == INVALID)
			{
				// Print current node type
				NodeTypeToString(ent, nodes[node].type, type_name, sizeof(type_name));
				gi.cprintf(ent, PRINT_MEDIUM, "%s Current node [%d] of type: %s\n", __func__, nodes[node].nodenum, type_name);

				// Print all links from this node to other nodes
				for (i = 0; i < nodes[node].num_links; i++)
				{
					NodeTypeToString(ent, nodes[node].links[i].targetNodeType, type_name, sizeof(type_name));
					gi.cprintf(ent, PRINT_MEDIUM, "%s node-to-node[%d -> %d] link type: %s\n", __func__, nodes[node].nodenum, nodes[node].links[i].targetNode, type_name);
				}

				ent->bot.walknode.highlighted_node = node;
				ent->bot.walknode.prev_highlighted_node = node;
				//if (ent->bot.walknode.prev_highlighted_node != ent->bot.walknode.highlighted_node)
				//	ent->bot.walknode.prev_highlighted_node = node;
				return;
			}

			//gi.cprintf(ent, PRINT_MEDIUM, "%s [%d] [LINK] Node touched prev_and_curr[%d %d] type[%d %d] \n", __func__, level.framenum, ent->bot.walknode.prev_highlighted_node, node, nodes[ent->bot.walknode.prev_highlighted_node].type, nodes[node].type);
		}
		else // No node touched
		{
			//gi.cprintf(ent, PRINT_MEDIUM, "%s [%d] [LINK] No node touched prev_and_curr[%d %d] type[%d %d] \n", __func__, level.framenum, ent->bot.walknode.prev_highlighted_node, node, nodes[ent->bot.walknode.prev_highlighted_node].type, nodes[node].type);

			// Unselect nodes
			ent->bot.walknode.highlighted_node = INVALID;
			ent->bot.walknode.prev_highlighted_node = INVALID;
			if (ent->bot.walknode.highlighted_node_type != HIGHLIGHTED_NODE_ADD && ent->bot.walknode.highlighted_node_type != HIGHLIGHTED_NODE_FLOODFILL)
				return;
		}

		// Check if new location touches another node
		nodes_touched = BOTLIB_NodeTouchNodes(tr.endpos, tr.plane.normal, 0, tv(-16, -16, -24), tv(16, 16, 32), nodelist, MAX_NODELIST, node);

		// Add node
		if ((ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_ADD || ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_FLOODFILL) && nodes_touched == 0) // No node touched
		{
			tr = gi.trace(start, tv(-16, -16, -24), tv(16, 16, 32), end, ent, MASK_PLAYERSOLID);

			// Try to align XY to a grid, otherwise just use the hit location
			int iorigin = 0; // Int based origin vec
			vec3_t grid_origin;
			VectorCopy(tr.endpos, grid_origin);
			iorigin = (int)grid_origin[0]; // Drop the decimal
			iorigin = iorigin - (iorigin % grid_size); // Align to grid
			grid_origin[0] = (float)iorigin; // Convert back to float
			iorigin = (int)grid_origin[1]; // Drop the decimal
			iorigin = iorigin - (iorigin % grid_size); // Align to grid
			grid_origin[1] = (float)iorigin; // Convert back to float
			tr = gi.trace(grid_origin, tv(-16, -16, -24), tv(16, 16, 32), grid_origin, ent, MASK_PLAYERSOLID);
			if (tr.startsolid || tr.fraction < 1.0) // Failed to find grid location, so just use hit location
				VectorCopy(tr.endpos, grid_origin);

			// Check if new location touches another node
			nodes_touched = BOTLIB_NodeTouchNodes(grid_origin, tr.plane.normal, 0, tv(-16, -16, -24), tv(16, 16, 32), nodelist, MAX_NODELIST, INVALID);
			if (nodes_touched == 0)
			{
				int node_added = BOTLIB_AddNode(grid_origin, tr.plane.normal, NODE_MOVE);
				if (node_added != INVALID)
				{
					//char typename[32] = { '\0' }; // Length of the longest node type name
					//NodeTypeToString(ent, nodes[node_added].type, typename, sizeof(typename));
					//gi.cprintf(ent, PRINT_MEDIUM, "%s %s added node [%d] type [%s]\n", __func__, ent->client->pers.netname, node_added, typename);

					// Try to self expand nodes outward from this node
					if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_FLOODFILL)
					{
						BOTLIB_SelfExpandingNodes(ent, node_added);
						//BOTLIB_LinkAllNodesTogether(ent);
					}
				}
			}
			// Try to self expand nodes outward from this node
			if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_FLOODFILL)
			{
				BOTLIB_LinkAllNodesTogether(ent);
			}
			return;
		}

		// Move
		if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_MOVE)
		{
			// Unselect node
			ent->bot.walknode.highlighted_node = INVALID;
			ent->bot.walknode.prev_highlighted_node = INVALID;
			return;
		}

		// Delete
		if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_DEL && ent->bot.walknode.highlighted_node != INVALID)
		{
			BOTLIB_RemoveAllNodeLinksFrom(ent->bot.walknode.highlighted_node); // Remove all links to and from this node
			nodes[ent->bot.walknode.highlighted_node].inuse = false; // Flags as unused
			VectorClear(nodes[ent->bot.walknode.highlighted_node].origin); // Clear origin

			// Unselect node
			ent->bot.walknode.highlighted_node = INVALID;
			ent->bot.walknode.prev_highlighted_node = INVALID;
			return;
		}

		// Change node type
		if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_TYPE && ent->bot.walknode.highlighted_node != INVALID)//
		{
			if (node == INVALID) // If unselecting, nodes will be invalid, so unhighlight and return
			{
				ent->bot.walknode.highlighted_node = INVALID;
				ent->bot.walknode.prev_highlighted_node = INVALID;
				return;
			}

			// Player changed to a new node from the previous node: inform the player of the current node type
			if (ent->bot.walknode.highlighted_node != node)
			{
				// Print current node type
				NodeTypeToString(ent, nodes[node].type, type_name, sizeof(type_name));
				gi.cprintf(ent, PRINT_MEDIUM, "%s Current node [%d] of type: %s\n", __func__, nodes[node].nodenum, type_name);

				// Print all links from this node to other nodes
				for (i = 0; i < nodes[node].num_links; i++)
				{
					NodeTypeToString(ent, nodes[node].links[i].targetNodeType, type_name, sizeof(type_name));
					gi.cprintf(ent, PRINT_MEDIUM, "%s node-to-node[%d -> %d] link type: %s\n", __func__, nodes[node].nodenum, nodes[node].links[i].targetNode, type_name);
				}

				ent->bot.walknode.highlighted_node = node;
				ent->bot.walknode.prev_highlighted_node = node;
				return;
			}

			// Player wishes to change type: a limited selection here so we don't go through the entire list.
			int nodetype = (nodes[node].type + 1); // Increment type so the switch statement will use the next type in the nodetype_t enum
			switch (nodetype) {
				case NODE_JUMPPAD:
					nodetype = NODE_JUMPPAD; // NODE_JUMPPAD enum == 2
					break;
				case NODE_LADDER:
					nodetype = NODE_LADDER; // NODE_LADDER enum == 3
					break;
				case NODE_WATER:
					nodetype = NODE_WATER; // NODE_WATER enum == 4
					break;
				case NODE_CROUCH:
					nodetype = NODE_CROUCH; // NODE_CROUCH enum == 5
					break;
				case NODE_BOXJUMP:
					nodetype = NODE_BOXJUMP; // NODE_BOXJUMP enum == 6
					break;
				case NODE_POI:
					nodetype = NODE_POI; // NODE_POI enum == 7
					break;
				default:
					nodetype = NODE_MOVE; // NODE_MOVE enum == 1
					break;
			}

			//char typename[32] = { '\0' }; // Length of the longest node type name
			if (NodeTypeToString(ent, nodetype, type_name, sizeof(type_name))) // Only change if valid
			{
				
				if (nodetype == NODE_CROUCH) // Update crouch min/max
				{
					nodes[node].mins[0] = -16;
					nodes[node].mins[1] = -16;
					nodes[node].mins[2] = -24;
					nodes[node].maxs[0] = 16;
					nodes[node].maxs[1] = 16;
					nodes[node].maxs[2] = CROUCHING_MAXS2;
				}
				else if (nodetype == NODE_BOXJUMP) // Update boxjump move min/max
				{
					nodes[node].mins[0] = -8;
					nodes[node].mins[1] = -8;
					nodes[node].mins[2] = -12;
					nodes[node].maxs[0] = 8;
					nodes[node].maxs[1] = 8;
					nodes[node].maxs[2] = 16;
				}
				else // Update all other node min/max to default
				{
					nodes[node].mins[0] = -16;
					nodes[node].mins[1] = -16;
					nodes[node].mins[2] = -24;
					nodes[node].maxs[0] = 16;
					nodes[node].maxs[1] = 16;
					nodes[node].maxs[2] = 32;
				}
				VectorAdd(nodes[node].origin, nodes[node].mins, nodes[node].absmin); // Update absolute box min/max in the world
				VectorAdd(nodes[node].origin, nodes[node].maxs, nodes[node].absmax); // Update absolute box min/max in the world

				BOTLIB_RemoveAllNodeLinksFrom(nodes[node].nodenum); // Remove all links to and from this node

				nodes[node].type = nodetype; // Update the node type
				gi.cprintf(ent, PRINT_MEDIUM, "%s Changed node to type: %s\n", __func__, type_name);
			}
			return;
		}

		// Link type
		else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINKTYPE && ent->bot.walknode.highlighted_node != INVALID && ent->bot.walknode.prev_highlighted_node != INVALID)
		{
			from = ent->bot.walknode.prev_highlighted_node;
			to = ent->bot.walknode.highlighted_node;

			if (node != to) // Changed target node, so unselect
			{
				ent->bot.walknode.highlighted_node = INVALID;
				ent->bot.walknode.prev_highlighted_node = INVALID;
				return;
			}

			// Check if a link exists between the two nodes
			for (i = 0; i < nodes[from].num_links; i++)
			{
				if (nodes[from].links[i].targetNode == to)
				{
					// Change link type
					int linktype = (nodes[from].links[i].targetNodeType + 1); // Increment type so the switch statement will use the next type in the nodetype_t enum
					switch (linktype) {
						case NODE_MOVE:
							linktype = NODE_MOVE;
							break;
						case NODE_JUMPPAD:
							linktype = NODE_JUMPPAD;
							break;
						case NODE_LADDER:
							linktype = NODE_LADDER;
							break;
						case NODE_WATER:
							linktype = NODE_WATER;
							break;
						case NODE_CROUCH:
							linktype = NODE_CROUCH;
							break;
						case NODE_BOXJUMP:
							linktype = NODE_BOXJUMP;
							break;
						case NODE_POI:
							linktype = NODE_POI;
							break;
						case NODE_POI_LOOKAT:
							linktype = NODE_POI_LOOKAT; // NODE_POI_LOOKAT enum == 7
							break;
						default:
							linktype = NODE_MOVE; // NODE_MOVE enum == 1
							break;
					}

					if (NodeTypeToString(ent, linktype, type_name, sizeof(type_name))) // Only change if valid
					{
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s changing link from node[%d -> %d] to type [%s]\n", __func__, ent->client->pers.netname, from, to, type_name);
						nodes[from].links[i].targetNodeType = linktype; // Update the link type
					}
					return;
				}
			}
		}

		// Link
		else if (ent->bot.walknode.highlighted_node_type == HIGHLIGHTED_NODE_LINK && ent->bot.walknode.highlighted_node != INVALID && ent->bot.walknode.prev_highlighted_node != INVALID)
		{
			from = ent->bot.walknode.prev_highlighted_node;
			to = ent->bot.walknode.highlighted_node;	

			// Check if link already exists, if so remove link
			for (i = 0; i < nodes[from].num_links; i++)
			{
				if (nodes[from].links[i].targetNode == to)
				{
					// Unselect node
					ent->bot.walknode.highlighted_node = INVALID;
					ent->bot.walknode.prev_highlighted_node = INVALID;

					// Print removing link
					NodeTypeToString(ent, nodes[from].links[i].targetNodeType, type_name, sizeof(type_name));
					gi.cprintf(ent, PRINT_MEDIUM, "%s %s removing link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

					// Remove link
					ACEND_RemoveNodeEdge(ent, from, to); // Removes one side of the link
					return;
				}
			}

			// Link ladder nodes
			if (nodes[from].type == NODE_LADDER && nodes[to].type == NODE_LADDER)
			{
				int node_type = NODE_LADDER;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}

			// Link water nodes
			if (nodes[from].type == NODE_WATER || nodes[to].type == NODE_WATER)
			{
				int node_type = NODE_WATER;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}

			// Link crouch nodes
			else if (nodes[from].type == NODE_CROUCH)
			{
				int node_type = NODE_CROUCH;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}

			// Link crouch drop nodes
			else if (BOTLIB_Reachability(from, to) == NODE_CROUCH)
			{
				int node_type = NODE_CROUCH;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}
			/*
			// Link jump nodes
			else if (BOTLIB_Reachability(from, to) == NODE_JUMP)
			{
				int node_type = NODE_JUMP;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}
			*/
			// Link jumppad nodes
			else if (BOTLIB_Reachability(from, to) == NODE_JUMPPAD)
			{
				int node_type = NODE_JUMPPAD;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}
			// Link move nodes
			else if (nodes[from].type == NODE_MOVE || nodes[to].type == NODE_MOVE)
			{
				int node_type = NODE_MOVE;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}
			// Link small move nodes
			else if (nodes[from].type == NODE_BOXJUMP || nodes[to].type == NODE_BOXJUMP)
			{
				int node_type = NODE_BOXJUMP;
				//if (BOTLIB_TraceBoxNode(from, to) == INVALID) // Don't link if our trace hit other nodes
				{
					if (BOTLIB_AddNodeLink(from, to, node_type, true))
					{
						// Print adding link
						NodeTypeToString(ent, node_type, type_name, sizeof(type_name));
						gi.cprintf(ent, PRINT_MEDIUM, "%s %s adding link from node[%d -> %d] type[%s]\n", __func__, ent->client->pers.netname, from, to, type_name);

						////ACEND_UpdateNodeReach(from, to); // Update path_table
					}
				}
			}

			// Unselect nodes
			ent->bot.walknode.highlighted_node = INVALID;
			ent->bot.walknode.prev_highlighted_node = INVALID;
			return;
		}
	}
}
#endif
//rekkie -- debug drawing -- e

// Test reachability between two nodes
int BOTLIB_Reachability(int from, int to)
{
	trace_t tr;
	const float max_dist = 1024; // Absolute max distance - 470
	qboolean is_gap = false; // If there's a gap between origin and target

	// Sanity checks
	if (from == to || from == INVALID || to == INVALID)
		return INVALID;

	vec3_t origin, target;
	VectorCopy(nodes[from].origin, origin);
	VectorCopy(nodes[to].origin, target);

	// Calculate the distance to the target
	vec3_t dist;
	vec3_t velocity;
	VectorSubtract(target, origin, dist);
	float xyz_distance = VectorLength(dist); // XYZ distance
	dist[2] = 0;
	float xy_distance = VectorLength(dist); // XY Distance

	// Max distance
	if (xyz_distance > max_dist) // Ignore very far nodes
		return INVALID;

	// Get direction
	vec3_t dir;
	VectorSubtract(target, origin, dir); // (end - start) = dir
	//VectorNormalize(dir); // Normalize direction vector (must be done after VectorLength otherwise we don't get the correct distance)

	// Test for gaps/holes between the current and next node
	// Each tested is conducted x units apart
	{
		vec3_t pos;
		VectorCopy(origin, pos);

		float tested_distance = 4;
		float normalized_dist = tested_distance / xyz_distance; // Normalized distance	
		while (tested_distance + 1 < xyz_distance)
		{
			tested_distance += 4; // Move next test forward
			VectorMA(pos, normalized_dist, dir, pos); // Origin -> normalized distance -> direction = new position

			//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
			if (0) // Debug draw
			{
				void(*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time) = players[0]->client->pers.draw->DrawBox;
				DrawBox(500 + tested_distance, pos, MakeColor(255, 255, 0, 255), nodes[from].mins, nodes[from].maxs, 500); // Draw node box
				//gi.cprintf(ent, PRINT_MEDIUM, "%s [%d to %d] dist[%f of %f] pos[%f %f %f]\n", __func__, from, to, distance, tested_distance, pos[0], pos[1], pos[2]);
			}
#endif
			//rekkie -- debug drawing -- e

			// Check position to see if we hit any solids along the way
			//tr = gi.trace(end, tv(-12, -12, -8), tv(12, 12, 8), end, g_edicts, MASK_PLAYERSOLID | MASK_OPAQUE);
			//if (tr.startsolid) is_gap = true;
			//if (tr.startsolid) continue;

			tr = gi.trace(pos, tv(-16, -16, -1), tv(16, 16, 1), tv(pos[0], pos[1], -4096), g_edicts, MASK_PLAYERSOLID | MASK_OPAQUE); // Trace down until we hit the ground
			if (tr.startsolid) continue;

			if ( fabs(pos[2] - tr.endpos[2]) > (61)) // Greater than max jump height //STEPSIZE
			{
				//Com_Printf("%s moved[%f] height[%f] xyz_distance[%f] -- FOUND GAP\n", __func__, tested_distance, fabs(pos[2] - tr.endpos[2]), xyz_distance);
				is_gap = true;
				break;
			}
		}
	}

	// Calculate max jump height
	// If on flat ground the max is 60 (NODE_MAX_JUMP_HEIGHT)
	// If on slope up, the max is 150 depending on the angle of the slope, higher angle = higher max jump height
	float z_height_max = NODE_MAX_JUMP_HEIGHT;
	//float max_speed = 577;   //(750 / 1.3 = 577)
	/*
	if (normal[0] != 1.0 && normal[1] != 1.0)
	{
		float z_height_max_x = 1.0;
		float z_height_max_y = 1.0;
		if (normal[0] >= MAX_STEEPNESS) // Player is standing on a slope going up //else if (surf->plane->normal[0] >= -MAX_STEEPNESS && surf->plane->normal[0] <= MAX_STEEPNESS && surf->plane->normal[1] >= -MAX_STEEPNESS && surf->plane->normal[1] <= MAX_STEEPNESS && surf->plane->normal[2] >= MAX_STEEPNESS)
		{
			z_height_max_x += (1 - normal[0]);
		}
		if (normal[1] >= MAX_STEEPNESS)
		{
			z_height_max_y += (1 - normal[1]);
		}
		z_height_max = (z_height_max_x + z_height_max_y) / 2;
		if (z_height_max > 1.3)
			z_height_max = 1.3;
		else if (z_height_max < 1.0)
			z_height_max = 1.0;

		z_height_max *= 116; // Max jump height is 150, and max z_height is 1.3 ... so 116 * 1.3 = 150   (150/1.3 = 116)
		if (z_height_max > 150)
			z_height_max = 150;

		//max_speed *= z_height_max;
	}
	*/

	qboolean target_is_above = false, target_is_below = false, target_is_equal = false;
	float higher = 0, lower = 0;
	if (origin[2] > target[2]) // We're above the target
	{
		target_is_below = true;
		lower = (origin[2] - target[2]);
	}
	else if (origin[2] < target[2]) // We're below the target
	{
		target_is_above = true;
		higher = (target[2] - origin[2]);
	}
	else
		target_is_equal = true;



	// Can drop while standing
	if (target_is_below && xy_distance < 128) //256, 228
	{
		// Crouch drop down + leg damage (actcity2 uses this for the one spawn that can cause leg damage when dropping after LCA)
		if (lower > 32 && lower <= NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE && xyz_distance <= NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE + NODE_Z_HALF_HEIGHT)
		{
			return NODE_CROUCH; // Can drop while crouching but will take some minor leg damage
		}
	}

	/*
	// Dropping down to target
	// -----------------------
	// Can drop while standing
	if (target_is_below && xy_distance < 228) //256
	{
		if (lower <= NODE_MAX_FALL_HEIGHT && xyz_distance <= NODE_MAX_FALL_HEIGHT + NODE_Z_HALF_HEIGHT)
		{
			return NODE_STAND_DROP; // Can drop while standing
		}

		// Crouch drop down
		if (lower <= NODE_MAX_CROUCH_FALL_HEIGHT && xyz_distance <= NODE_MAX_CROUCH_FALL_HEIGHT + NODE_Z_HALF_HEIGHT)
		{
			return NODE_CROUCH_DROP; // Can drop but need to crouch
		}

		// Crouch drop down + leg damage (actcity2 uses this for the one spawn that can cause leg damage when dropping after LCA)
		if (lower <= NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE && xyz_distance <= NODE_MAX_CROUCH_FALL_HEIGHT_UNSAFE + NODE_Z_HALF_HEIGHT)
		{
			return NODE_UNSAFE_DROP; // Can drop while crouching but will take some minor leg damage
		}
	}
	*/

	//if (xyz_distance <= 128 && is_gap)
	//	return NODE_JUMP;


	//if (xyz_distance >= 64 && is_gap)
	//	return NODE_JUMPPAD;

	if (xyz_distance >= 32 && is_gap)
	{
		float gravity = sv_gravity->value; // This doesn't take into account the ent's own gravity (ent->gravity)
		float jump_height = sqrt(2 * gravity * xyz_distance); // Calculate the jump height to get to the target
		float time = xyz_distance / (jump_height / 2.0); // Calculate the time it will take to get to the target
		VectorScale(dist, 1.0 / time, velocity); // Calculate the velocity at the end of the jump
		velocity[2] = jump_height / 2.0;

		// Test if speed is too high
		qboolean speed_within_spec = false;
		//float required_speed = VectorLength(velocity);
		if (target[2] > origin[2])
		{
			//Com_Printf("%s ABOVE speed[%f] jump_height[%f]\n", __func__, required_speed, jump_height);
			//if (required_speed < 550)
				speed_within_spec = true;
		}
		else if (abs(origin[2] - target[2]) <= 8) // target is +/- 8
		{
			//Com_Printf("%s EQUAL speed[%f] jump_height[%f]\n", __func__, required_speed, jump_height);
			//if (required_speed < 600)
				speed_within_spec = true;
		}
		else
		{
			//Com_Printf("%s BELOW speed[%f] jump_height[%f]\n", __func__, required_speed, jump_height);
			//if (required_speed < 750)
				speed_within_spec = true;
		}

		// Result
		if (speed_within_spec)
			return NODE_JUMPPAD;
		else
			return INVALID;
	}


	// Large jump up to target
	// --------------------
	// Jumppad nodes
	//if (target_is_above && distance > STEPSIZE)
	if (xyz_distance >= 64 && is_gap)
	{
		float gravity = sv_gravity->value; // This doesn't take into account the ent's own gravity (ent->gravity)

		//distance *= 1.0; // Increasing this increases the jump height
		// Calculate the jump height to get to the target
		float jump_height = sqrt(2 * gravity * xyz_distance);
		//float jump_height_headroom = jump_height / 8; // head space required to make the jump from point to point

		// Move the middle point up to the jump height (maximum parabola height)
		//end_50[2] += NODE_Z_HEIGHT_PLUS_STEPSIZE + jump_height;
		// Next test from start to mid, then mid to end
		//tr_25 = gi.trace(origin, tv(-16, -16, -30), tv(16, 16, 24), end_50, g_edicts, MASK_DEADSOLID);
		//tr_75 = gi.trace(target, tv(-16, -16, -30), tv(16, 16, 24), end_50, g_edicts, MASK_DEADSOLID);
		// If the path from [start -> mid jump -> end] is clear of obsticles, then allow the jump
		//if (tr_25.fraction == 1.0 && tr_75.fraction == 1.0)

		// Do we have room to jump without hitting our head?
		//tr = gi.trace(tv(origin[0], origin[1], origin[2] + NODE_Z_HEIGHT), tv(-16, -16, -6), tv(16, 16, 48), tv(target[0], target[1], target[2] + NODE_Z_HEIGHT) , g_edicts, MASK_DEADSOLID); // Player height + some jumping height
		//tr = gi.trace(tv(origin[0], origin[1], origin[2] + NODE_Z_HEIGHT), NULL, NULL, tv(target[0], target[1], target[2] + NODE_Z_HEIGHT), g_edicts, MASK_DEADSOLID); // Player height + some jumping height
		//tr = gi.trace(tv(origin[0], origin[1], origin[2] + NODE_Z_HEIGHT), tv(-16, -16, -24), tv(16, 16, 32), tv(target[0], target[1], target[2] + NODE_Z_HEIGHT), g_edicts, MASK_DEADSOLID); // Player height + some jumping height
		//tr = gi.trace(origin, tv(-16, -16, -24), tv(16, 16, jump_height_headroom), target, g_edicts, MASK_DEADSOLID); // Player height + some jumping height
		//tr = gi.trace(origin, tv(-16, -16, 0), tv(16, 16, jump_height_headroom), target, g_edicts, MASK_DEADSOLID); // Player height + some jumping height
		//if (tr.fraction == 1.0 && !tr.startsolid)
		{
			//Com_Printf("jump_height[%f] jump_height_headroom[%f]\n", jump_height, jump_height_headroom);

			// Calculate the time it will take to get to the target
			float time = xyz_distance / (jump_height / 2.0);
			// Calculate the velocity at the end of the jump
			VectorScale(dist, 1.0 / time, velocity);
			velocity[2] = jump_height / 2.0;

			//
			// If the target is above the player, increase the velocity to get to the target
			float z_height = 0;

			// Above
			if (target[2] > origin[2])
			{
				z_height = (target[2] - origin[2]) + NODE_Z_HEIGHT;
				velocity[2] += z_height;

				float speed = VectorLength(velocity);

				Com_Printf("%s ABOVE speed[%f] z_height[%f] z_height_max[%f] jump_height[%f]\n", __func__, speed, z_height, (z_height_max + NODE_Z_HALF_HEIGHT), jump_height);

				//if (speed < 550 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 500)
				if (speed < 550 && z_height <= z_height_max + sqrtf(xy_distance) && jump_height <= 450)
					return NODE_JUMPPAD;
			}
			// Roughly equal
			else if (abs(origin[2] - target[2]) <= 8) // target node is +/- 8
			{
				float speed = VectorLength(velocity);

				Com_Printf("%s EQUAL speed[%f] jump_height[%f]\n", __func__, speed, jump_height);

				if (speed < 600 && z_height < z_height_max + NODE_Z_HEIGHT && jump_height <= 750)
					return NODE_JUMPPAD;
			}
			// Below
			else
			{
				z_height = (origin[2] - target[2]) - NODE_Z_HEIGHT;
				velocity[2] -= z_height;

				float speed = VectorLength(velocity);
				
				Com_Printf("%s BELOW speed[%f] z_height[%f] z_height_max[%f] jump_height[%f]\n", __func__, speed, z_height, (z_height_max + NODE_Z_HALF_HEIGHT), jump_height);

				if (speed < 750 && z_height < z_height_max + NODE_Z_HEIGHT && jump_height <= 965)
					return NODE_JUMPPAD;
			}
		}
	}

	return INVALID;
}
//rekkie -- Walknodes -- e


// Move closer to goal by pointing the bot to the nearest next node that is closer to the goal
qboolean BOTLIB_FollowPath(edict_t *self)
{
	if (self->bot.current_node == INVALID || self->bot.next_node == INVALID || self->bot.goal_node == INVALID) // Invalid pathing
	{
		self->bot.node_travel_time++;
		return false;
	}

	// Start holding the POI
	if (nodes[self->bot.next_node].type == NODE_POI && self->bot.node_poi_holding == false)
	{
		// See if we touched the goal (POI) node
		float distance = 4;
		for (int i = self->bot.node_list_current; i < self->bot.node_list_count; i++)
		{
			int node = self->bot.node_list[i]; // Next node in the list
			if (node != INVALID)
			{
				vec3_t bmins = { nodes[node].absmin[0] + -(distance), nodes[node].absmin[1] + -(distance), nodes[node].absmin[2] + -(distance) };
				vec3_t bmaxs = { nodes[node].absmax[0] + distance, nodes[node].absmax[1] + distance, nodes[node].absmax[2] + distance };
				if (BOTLIB_BoxIntersection(self->absmin, self->absmax, bmins, bmaxs))
				{
					if (node == self->bot.goal_node) // Reached POI
					{
						// If we're not holding the POI, then hold it
						if (self->bot.node_poi_holding == false && self->bot.node_poi_time < level.framenum)
						{
							self->bot.prev_node = self->bot.current_node;
							self->bot.current_node = self->bot.goal_node;
							self->bot.next_node = self->bot.goal_node;

							self->bot.node_poi_holding = true;

							if (teamplay->value && lights_camera_action) // Hold spot longer if LCA is active
								self->bot.node_poi_time = level.framenum + 30 * HZ; // Set the time to spend at the POI
							else
								self->bot.node_poi_time = level.framenum + 7 * HZ; // Set the time to spend at the POI

							break;
						}
					}
				}
			}
		}
	}
	// Continue holding the POI
	if (self->bot.node_poi_holding && self->bot.node_poi_time > level.framenum && self->bot.see_enemies == false && self->bot.enemy_in_xhair == false)
	{
		//Com_Printf("%s [%s] at POI, hold pos\n", __func__, self->client->pers.netname);
		self->bot.bi.actionflags = ACTION_HOLDPOS; // Hold
		//self->bot.bi.actionflags |= ACTION_CROUCH; // Crouch
		return true;
	}
	// Stop holding the POI
	else if (self->bot.node_poi_holding)
	{
		self->bot.node_poi_holding = false;
		//Com_Printf("%s [%s] at POI, free to leave\n", __func__, self->client->pers.netname);

		// Kill velocity, give the bot a chance to adjust to the next goal path
		self->velocity[0] = 0;
		self->velocity[1] = 0;
		self->velocity[2] = -200;

		self->bot.bi.speed = 10;

		//Com_Printf("%s %s GOAL\n", __func__, self->client->pers.netname);
		self->bot.node_travel_time = 0;
		self->bot.state = BOT_MOVE_STATE_NAV;
		self->bot.goal_node = INVALID;
		return false;
	}

	if (ctf->value)
	{
		// Allow flag carrying bots to idle around their home flag
		if (BOTLIB_Carrying_Flag(self))
		{
			if (self->client->resp.team == TEAM1 && VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, self->s.origin) <= 128)
			{
				self->bot.node_travel_time = 0;
			}
			else if (self->client->resp.team == TEAM2 && VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, self->s.origin) <= 128)
			{
				self->bot.node_travel_time = 0;
			}
		}

		// Allow flag bots to idle around enemy flag home
		if (BOTLIB_Carrying_Flag(self) == false)
		{
			if (self->client->resp.team == TEAM1 && VectorDistance(nodes[bot_ctf_status.flag2_home_node].origin, self->s.origin) <= 128)
			{
				self->bot.node_travel_time = 0;
			}
			else if (self->client->resp.team == TEAM2 && VectorDistance(nodes[bot_ctf_status.flag1_home_node].origin, self->s.origin) <= 128)
			{
				self->bot.node_travel_time = 0;
			}
		}
	}

	//if (teamplay->value && lights_camera_action > 0)
	//	return true;

	//rekkie -- Quake3 -- s
	// Count travel time between nodes, only if bot is not holding position or LCA countdown is active
	if ((self->bot.bi.actionflags & ACTION_HOLDPOS) == 0)
	{
		// Set node_travel_time to zero if the round isn't active
		if (teamplay->value && (team_round_going == false || lights_camera_action > 0))
			self->bot.node_travel_time = 0;

		// If we stopped moving
		//if (VectorDistance(self->s.origin, self->lastPosition) < FRAMETIME)
		self->bot.node_travel_time++;

		// Check bot direction vs node direction
		vec3_t node_vec = { 0 };
		VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, node_vec);
		vec3_t move_vec = { 0 };
		VectorSubtract(self->s.origin, self->lastPosition, move_vec);
		VectorNormalize(node_vec);
		VectorNormalize(move_vec);
		float dot = DotProduct(node_vec, move_vec);
		//Com_Printf("%s [%s] dot[%f] \n", __func__, self->client->pers.netname, dot);
		if (dot < 0.3f)
		{
			//Com_Printf("%s [%s] [%f] node_travel_time: %d \n", __func__, self->client->pers.netname, dot, self->bot.node_travel_time);
			self->bot.node_travel_time++;
		}

		//Com_Printf("%s [%s] [%f] node_travel_time: %d \n", __func__, self->client->pers.netname, dot, self->bot.node_travel_time);
	}
	//rekkie -- Quake3 -- e


	


	// Boot in our new pathing algorithm - this will fill self->pathList with the information we need
	// We have no path (SLLempty) and we're not at our destination (current_node != goal_node)
	if (SLLempty(&self->pathList))
	{
		//Com_Printf("%s %s: SLLempty curr[%d] goal[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node);

		self->bot.node_list_count = 0;

		/*
		if (!AntStartSearch(self, self->bot.current_node, self->bot.goal_node)) // Set up our pathList
		{
			// Failed to find a path
			return false;
		}
		*/
	}
	
	// Check if bot is touching a node that isn't on the path
	int nodelist[MAX_NODELIST]; // Nodes touched
	int nodes_touched; // Number of nodes touched
	nodes_touched = BOTLIB_NodeTouchNodes(self->s.origin, vec3_origin, 0, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
	qboolean external_node_touched = true;
	for (int i = 0; i < nodes_touched; i++)
	{
		if (self->groundentity == NULL)
		{
			external_node_touched = false;
			break;
		}
		/*
		// Check if touching any of the connected links
		int node = self->bot.node_list[i];
		for (int j = 0; j < nodes[node].num_links; j++)
		{
			{
				external_node_touched = false;
				break;
			}
		}
		*/

		// Check if node very close
		if (VectorDistance(self->s.origin, nodes[self->bot.current_node].origin) <= 64 || VectorDistance(self->s.origin, nodes[self->bot.next_node].origin) <= 64)
		{
			external_node_touched = false;
			break;
		}

		for (int j = self->bot.node_list_current; j < self->bot.node_list_count; j++)
		{
			if (nodelist[i] == self->bot.node_list[j])
			{
				external_node_touched = false;
				break;
			}
		}
	}
	if (self->groundentity == NULL)
	{
		external_node_touched = false;
	}
	if (external_node_touched)
	{
		//Com_Printf("%s %s touching ext node\n", __func__, self->client->pers.netname);

		// Check how far we've moved
		qboolean moved = true;
		vec3_t lastdir;
		VectorSubtract(self->s.origin, self->lastPosition, lastdir);
		float move_dist = VectorLength(lastdir);
		if (move_dist < FRAMETIME)
			moved = false; // We've not moved

		if (moved == false && self->bot.node_list_count && self->bot.goal_node)
		{
			//Com_Printf("%s %s touched a node that isn't on its path\n", __func__, self->client->pers.netname);
			//if (BOTLIB_CanGotoNode(self, self->bot.goal_node, false))
			if (BOTLIB_CanVisitNode(self, self->bot.goal_node, false, INVALID, false))
			{
				//self->bot.state = BOT_MOVE_STATE_MOVE;
				//BOTLIB_SetGoal(self, self->bot.goal_node);
				//Com_Printf("%s %s redirecting bot to its goal node %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
				return true;
			}
		}
		if (self->bot.node_list_count == 0 || self->bot.goal_node == INVALID)
		{
			//Com_Printf("%s %s redirecting bot to a new goal\n", __func__, self->client->pers.netname);
			//self->bot.state = BOT_MOVE_STATE_NAV;
			//return true;
		}
	}



	if (self->bot.node_list_count == 0)
	{
		//Com_Printf("%s %s node_list_count is empty. Wandering!\n", __func__, self->client->pers.netname);

		////if (nav_area.total_areas > 0)
		{
			////self->bot.state = BOT_MOVE_STATE_NAV_NEXT;
		}
		////else
		{
			self->bot.state = BOT_MOVE_STATE_NAV;
			self->bot.goal_node = INVALID;
		}
		
		return false;
	}

	if (bot_showpath->value && dedicated->value == 0)
	{
		BOTLIB_DrawPath(self);
	}


	/*
	// Check if bot is touching a node that isn't on the path
	int nodes_touched; // Number of nodes touched
	int nodelist[MAX_NODELIST]; // Nodes touched
	nodes_touched = BOTLIB_NodeTouchNodes(self->s.origin, tv(0,0,0), 0, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
	qboolean external_node_touched = false;
	for (int i = 0; i < nodes_touched; i++)
	{
		for (int j = self->bot.node_list_current; j < self->bot.node_list_count; j++)
		{
			external_node_touched = true;
			if (nodelist[i] == self->bot.node_list[j])
			{
				external_node_touched = false;
				break;
			}
		}
	}
	if (external_node_touched)
	{
		//Com_Printf("%s %s touched a node that isn't on our path\n", __func__, self->client->pers.netname);
	}
	*/


	// Check if bot is at last node
	if (self->bot.node_list_current == self->bot.node_list_count - 1)
	{
		int node = self->bot.node_list[self->bot.node_list_count - 1]; // Last node (goal node)
		if (node != INVALID && node == self->bot.goal_node)
		{
			float dist = VectorDistance(nodes[self->bot.current_node].origin, nodes[node].origin);

			//float distance = 32; // extend node box size by
			//vec3_t bmins = { nodes[node].absmin[0] + -(distance), nodes[node].absmin[1] + -(distance), nodes[node].absmin[2] + -(distance) };
			//vec3_t bmaxs = { nodes[node].absmax[0] + distance, nodes[node].absmax[1] + distance, nodes[node].absmax[2] + distance };
			//if (BOTLIB_BoxIntersection(self->absmin, self->absmax, bmins, bmaxs))
			if (dist < 128)
			{
				// Reached goal
				//if (node == self->bot.goal_node)
				{
					// Kill velocity, give the bot a chance to adjust to the next goal path
					self->velocity[0] = 0;
					self->velocity[1] = 0;
					self->velocity[2] = -200;

					self->bot.bi.speed = 100;
					//self->bot.bi.actionflags = 0;

					//Com_Printf("%s %s +++++++++ SUCCESSFULLY REACHED GOAL +++++++++ curr_node[%d] goal_node[%d] dist[%f]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node, VectorDistance(nodes[self->bot.current_node].origin, nodes[node].origin));
					self->bot.node_travel_time = 0;

					////if (nav_area.total_areas > 0)
					{
						////self->bot.state = BOT_MOVE_STATE_NAV_NEXT;
					}
					////else
					{
						self->bot.state = BOT_MOVE_STATE_NAV; // Flag bot to get a new goal
						self->bot.goal_node = INVALID;
					}

					return false;
				}
			}
		}
	}


	// See if we touched any of the nodes in our path, if so, update our current node and next node
	float distance = 4; // extend node box size by
	for (int i = self->bot.node_list_current; i < self->bot.node_list_count; i++)
	{
		int node = self->bot.node_list[i]; // Next node in the list
		if (node != INVALID)
		{
			vec3_t bmins = { nodes[node].absmin[0] + -(distance), nodes[node].absmin[1] + -(distance), nodes[node].absmin[2] + -(distance) };
			vec3_t bmaxs = { nodes[node].absmax[0] + distance, nodes[node].absmax[1] + distance, nodes[node].absmax[2] + distance };
			if (BOTLIB_BoxIntersection(self->absmin, self->absmax, bmins, bmaxs))
			{
				// Reached goal
				if (node == self->bot.goal_node)
				{
					// Kill velocity, give the bot a chance to adjust to the next goal path
					self->velocity[0] = 0;
					self->velocity[1] = 0;
					self->velocity[2] = -200;

					self->bot.bi.speed = 100;
					//self->bot.bi.actionflags = 0;

					// This is a complete success, well done
					//Com_Printf("%s %s SUCCESSFULLY REACHED GOAL - curr_node[%d] goal_node[%d] dist[%f]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node, VectorDistance(nodes[self->bot.current_node].origin, nodes[node].origin));
					self->bot.node_travel_time = 0;

					////if (nav_area.total_areas > 0)
					{
						////self->bot.state = BOT_MOVE_STATE_NAV_NEXT;
					}
					////else
					{
						self->bot.state = BOT_MOVE_STATE_NAV; // Flag bot to get a new goal
						self->bot.goal_node = INVALID;
					}

					return false;
				}
				else // Update current and next node // THIS IS IT HERE!!!
				{
					self->bot.node_travel_time = 0;

					self->bot.node_list_current = i + 1;

					self->bot.prev_node = self->bot.current_node;

					self->bot.current_node = self->bot.node_list[i];
					self->bot.next_node = self->bot.node_list[i + 1];
				}
			}
		}
	}
	// Backup check: if our list is at the end, then we're at the goal
	//Com_Printf("%s %s count[%d] curr[%d]\n", __func__, self->client->pers.netname, self->bot.node_list_count, self->bot.node_list_current);
	if (self->bot.node_list_current > self->bot.node_list_count)
	{
		// Kill velocity, give the bot a chance to adjust to the next goal path
		self->velocity[0] = 0;
		self->velocity[1] = 0;
		self->velocity[2] = -200;

		self->bot.bi.speed = 10;
		//self->bot.bi.actionflags = 0;

		Com_Printf("%s %s GOAL [FIX]\n", __func__, self->client->pers.netname);
		self->bot.node_travel_time = 0;

		////if (nav_area.total_areas > 0)
		{
			////Com_Printf("%s %s GOAL [FIX]\n", __func__, self->client->pers.netname);
			////self->bot.state = BOT_MOVE_STATE_NAV_NEXT;
		}
		////else
		{
			self->bot.state = BOT_MOVE_STATE_NAV; // Flag bot to get a new goal
			self->bot.goal_node = INVALID;
		}

		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
// MAPPING CODE
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Capture when the grappling hook has been fired for mapping purposes.
///////////////////////////////////////////////////////////////////////
void ACEND_GrapFired(edict_t *self)
{
/*	int closest_node;
	
	if(!self->owner)
		return; // should not be here
	
	// Check to see if the grapple is in pull mode
	if(self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
	{
		// Look for the closest node of type grapple
		closest_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_GRAPPLE);
		if(closest_node == -1 ) // we need to drop a node
		{	
			closest_node = ACEND_AddNode(self,NODE_GRAPPLE);
			 
			// Add an edge
			ACEND_UpdateNodeEdge(self, self->owner->last_node,closest_node);
		
			self->owner->last_node = closest_node;
		}
		else
			self->owner->last_node = closest_node; // zero out so other nodes will not be linked
	}*/
}

/*
///////////////////////////////////////////////////////////////////////
// Check for adding ladder nodes
///////////////////////////////////////////////////////////////////////
qboolean ACEND_CheckForLadder(edict_t *self)
{
	int closest_node;

	// If there is a ladder and we are moving up, see if we should add a ladder node
	if (gi.pointcontents(self->s.origin) & CONTENTS_LADDER && self->velocity[2] > 0)
	{
		//debug_printf("contents: %x\n",tr.contents);

		closest_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_LADDER); 
		if(closest_node == -1)
		{
			closest_node = ACEND_AddNode(self,NODE_LADDER);
	
			// Now add link
		    ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			
			// Set current to last
			self->last_node = closest_node;
		}
		else
		{
			ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			self->last_node = closest_node; // set visited to last
		}
		return true;
	}
	return false;
}

//=======================================================
// LadderForward
//=======================================================
//
// The ACE code version of this doesn't work!

qboolean	ACEND_LadderForward( edict_t *self )//, vec3_t angles )
{
	vec3_t	dir, angle, dest, min, max;
	trace_t	trace;
	int closest_node;


	VectorClear(angle);
	angle[1] = self->s.angles[1];

	AngleVectors(angle, dir, NULL, NULL);
	VectorCopy(self->mins,min);
	min[2] += 22;
	VectorCopy(self->maxs,max);
	VectorMA(self->s.origin, TRACE_DIST_LADDER, dir, dest);

	trace = gi.trace(self->s.origin, min, max, dest, self, MASK_ALL);

	//BOTUT_TempLaser(self->s.origin, dest);
	if (trace.fraction == 1.0)
		return (false);

//	gi.bprintf(PRINT_HIGH,"Contents forward are %d\n", trace.contents);
	if (trace.contents & CONTENTS_LADDER || trace.contents &CONTENTS_DETAIL)
	{
		// Debug print
//		gi.bprintf(PRINT_HIGH,"contents: %x\n",trace.contents);

		closest_node = ACEND_FindClosestReachableNode(self,NODE_DENSITY,NODE_LADDER); 
		if(closest_node == -1)
		{
			closest_node = ACEND_AddNode(self,NODE_LADDER);
	
			// Now add link
		    ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			
			// Set current to last
			self->last_node = closest_node;
		}
		else
		{
			ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
			self->last_node = closest_node; // set visited to last
		}
		return (true);
	}		
	return (false);
}


///////////////////////////////////////////////////////////////////////
// This routine is called to hook in the pathing code and sets
// the current node if valid.
///////////////////////////////////////////////////////////////////////
void ACEND_PathMap(edict_t *self)
{
	int closest_node;
	// Removed last_update checks since this stopped multiple node files being built
	vec3_t v;

	// Special node drawing code for debugging
	if( ltk_showpath->value )
	{
		if(show_path_to != -1)
			ACEND_DrawPath( self );
	}
	
	// Just checking lightlevels - uncomment to use
//	if( debug_mode && !self->is_bot)
//		gi.bprintf(PRINT_HIGH,"LightLevel = %d\n", self->light_level);

	////////////////////////////////////////////////////////
	// Special check for ladder nodes
	///////////////////////////////////////////////////////
	// Replace non-working ACE version with mine.
//	if(ACEND_CheckForLadder(self)) // check for ladder nodes
	if(ACEND_LadderForward(self)) // check for ladder nodes
		return;

	// Not on ground, and not in the water, so bail
    if(!self->groundentity && !self->waterlevel)
		return;

	////////////////////////////////////////////////////////
	// Lava/Slime
	////////////////////////////////////////////////////////
	VectorCopy(self->s.origin,v);
	v[2] -= 18;
	if(gi.pointcontents(v) & (CONTENTS_LAVA|CONTENTS_SLIME))
		return; // no nodes in slime
	
    ////////////////////////////////////////////////////////
	// Jumping
	///////////////////////////////////////////////////////
	if(self->is_jumping)
	{
	   // See if there is a closeby jump landing node (prevent adding too many)
		closest_node = ACEND_FindClosestReachableNode(self, 64, NODE_JUMP);

		if(closest_node == INVALID)
			closest_node = ACEND_AddNode(self,NODE_JUMP);
		
		// Now add link
		if(self->last_node != -1)
			ACEND_UpdateNodeEdge(self, self->last_node, closest_node);	   

		self->is_jumping = false;
		return;
	}

	// Werewolf:
    ////////////////////////////////////////////////////////
	// Switches, etc. - uses the "grapple" nodetype
	///////////////////////////////////////////////////////
	if(self->is_triggering)
	{
	   // See if there is a closeby grapple node (prevent adding too many)
//		closest_node = ACEND_FindClosestReachableNode(self, 64, NODE_GRAPPLE);

//		if(closest_node == INVALID)
			closest_node = ACEND_AddNode(self,NODE_GRAPPLE);
		
		// Now add link
		if(self->last_node != -1)
			ACEND_UpdateNodeEdge(self, self->last_node, closest_node);	   

		self->is_triggering = false;
		return;
	}


	////////////////////////////////////////////////////////////
	// Grapple
	// Do not add nodes during grapple, added elsewhere manually
	////////////////////////////////////////////////////////////
//	if(ctf->value && self->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
//		return;
	 
	// Iterate through all nodes to make sure far enough apart
	closest_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	////////////////////////////////////////////////////////
	// Special Check for Platforms
	////////////////////////////////////////////////////////
	if(self->groundentity && self->groundentity->use == Use_Plat)
	{
		if(closest_node == INVALID)
			return; // Do not want to do anything here.

		// Here we want to add links
		if(closest_node != self->last_node && self->last_node != INVALID)
			ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   

		self->last_node = closest_node; // set visited to last
		return;
	}
	 
	 ////////////////////////////////////////////////////////
	 // Add Nodes as needed
	 ////////////////////////////////////////////////////////
	 if(closest_node == INVALID)
	 {
		// Add nodes in the water as needed
		if(self->waterlevel)
			closest_node = ACEND_AddNode(self,NODE_WATER);
		else
		    closest_node = ACEND_AddNode(self,NODE_MOVE);
		
		// Now add link
		if(self->last_node != -1)
			ACEND_UpdateNodeEdge(self, self->last_node, closest_node);	   
			
	 }
	 else if(closest_node != self->last_node && self->last_node != INVALID)
	 	ACEND_UpdateNodeEdge(self, self->last_node,closest_node);	   
	
	 self->last_node = closest_node; // set visited to last
	
}
*/

///////////////////////////////////////////////////////////////////////
// Init node array (set all to INVALID)
///////////////////////////////////////////////////////////////////////
void BOTLIB_InitNodes(void)
{
	numnodes = 0; // Nodes start at 0
	numitemnodes = 1;
	
	//memset(&nmesh, 0, sizeof(nmesh_t));

	//BOTLIB_FreeNodes(); // Free any existing node data

	//memset(nodes,0,sizeof(node_t) * MAX_NODES);
	//memset(path_table,INVALID,sizeof(short int)*MAX_NODES*MAX_NODES);

	//rekkie -- DEV_1 -- s
	memset(node_ents, 0, sizeof(node_ents));
	//rekkie -- DEV_1 -- e

	// -------------
	// Alloc memory
	// -------------
	/*
	// Main nodes
	for (int n = 0; n < MAX_PNODES; n++)
	{
		nodes[n] = (node_t*)malloc(sizeof(node_t));
		if (nodes[n] == NULL)
		{
			Com_Printf("%s failed to malloc nodes\n", __func__);
			return;
		}
		memset(nodes[n], 0, sizeof(node_t));
		//nodes[n]->nodenum = INVALID;
	}
	//nodes = (node_t*)malloc(sizeof(node_t) * MAX_PNODES);
	//if (nodes == NULL)
	//{
	//	Com_Printf("%s failed to malloc nodes\n", __func__);
	//	return;
	//}
	//memset(nodes, 0, sizeof(node_t) * MAX_PNODES);

	// Unsorted duplicate copy nodes
	unsorted_nodes = (node_t*)malloc(sizeof(node_t) * MAX_PNODES);
	if (unsorted_nodes == NULL)
	{
		Com_Printf("%s failed to malloc unsorted_nodes\n", __func__);
		return;
	}
	*/
	
	/*
	// Path table
	// Uses a 2D array of dynamic memory consisting of an array of pointers -- https://www.techiedelight.com/dynamically-allocate-memory-for-2d-array/
	// This doubles the memory requirements versus a traditional 2D array, but it allows for larger array sizes
	path_table = (short int**)malloc(sizeof(short int*) * MAX_PNODES);
	if (path_table == NULL)
	{
		Com_Printf("%s failed to malloc path_table[]\n", __func__);
		return;
	}
	else
	{
		// Dynamically allocate memory of size MAX_PNODES for each row
		for (int r = 0; r < MAX_PNODES; r++)
		{
			path_table[r] = (short int*)malloc(sizeof(short int) * MAX_PNODES);
			if (path_table[r] == NULL)
			{
				Com_Printf("%s failed to malloc path_table[][]\n", __func__);
				return;
			}
			else
			{
				for (int c = 0; c < MAX_PNODES; c++)
				{
					path_table[r][c] = INVALID;
				}
			}
		}
	}

	Com_Printf("%s allocing MBytes[%ld] for path_table\n", __func__, ((sizeof(short int) * MAX_PNODES * MAX_PNODES) / 1024000));
	*/
}

//rekkie -- DEV_1 -- s
/*
///////////////////////////////////////////////////////////////////////
// Show spawn NODE_SPAWNPOINT nodes
///////////////////////////////////////////////////////////////////////
edict_t* ACEND_ShowSpawnPointNode(int node, qboolean isGoodNode)
{
	edict_t* ent;

	ent = G_Spawn();

	ent->movetype = MOVETYPE_NONE;

	//ent->solid = SOLID_NOT;
	ent->solid = SOLID_TEST_NOT;	// Used in gi.TraceAll (but not gi.Trace) - ensures non-solid ents are findable
	ent->svflags |= SVF_ENT;		// Ensures other entities (inc players) cannot clip against this entity
	ent->svflags &= ~SVF_NOCLIENT;	// Always send to client

	ent->classname = ent->node_name;	//botdebug
	ent->node_num = node;

	VectorSet(ent->mins, -4, -4, -4);
	VectorSet(ent->maxs, 4, 4, 4);

	ent->owner = ent;
	ent->dmg = 0;

	// Deathmatch spawn point
	ent->model = "models/objects/dmspot/tris.md2";
	ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
	if (isGoodNode)
		ent->s.renderfx = RF_SHELL_BLUE;
	else
		ent->s.renderfx = RF_SHELL_RED;
	Q_strncpyz(ent->node_name, va("node spawn point [%d]", node), sizeof(ent->node_name));

	if (debug_mode)
	{
		ent->nextthink = level.framenum + 6000 * HZ; // Extra long
		ent->think = G_FreeEdict;
	}

	VectorCopy(nodes[node].origin, ent->s.origin);
	gi.linkentity(ent);
	return ent;
}
*/
///////////////////////////////////////////////////////////////////////
// Ensures that spawn points all have NODE_SPAWNPOINT nodes on them
// Bad nodes occur because they were either moved or the map was changed
///////////////////////////////////////////////////////////////////////
void ACEND_BuildSpawnPointNodes(void)
{
	// Counter, Spot, Node
	int i, s, n;

	// Save
	qboolean requiresSave = false;		// If ACEND_SaveNodes() needs to be called

	// Spawn Point (SP)
	vec3_t spots_mod[MAX_SP_NODES];		// Spawn points (SP) (info_player_deathmatch) - modified from a downward endpos trace
	short sp_counter = 0;				// Total spawn points found
	qboolean sp_has_spn[MAX_SP_NODES];	// If the SP has an SPN
	edict_t* spot = NULL;
	trace_t tr;
	vec3_t maxs = { 1,1,1 };
	vec3_t mins = { 1,1,1 };

	// Spawn Point Node (SPN)
	short spn_nodes[MAX_SP_NODES];		// Track spawn point nodes (SPN)
	short spn_total = 0;				// Total spawn point nodes (SPN) found on map
	vec3_t spn_loc[MAX_SP_NODES];		// All SPN locations
	qboolean spn_inuse[MAX_SP_NODES];	// If the SPN is in use

	edict_t* ent;

	// Init
	for (i = 0; i < MAX_SP_NODES; i++)
	{
		spn_nodes[i] = INVALID;
		sp_has_spn[i] = false;
		spn_inuse[i] = false;
	}

	// Find all SPs
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if (sp_counter >= MAX_SP_NODES) // Security check for array bounds - what map would need more than 256 spawns?
		{
			Com_Printf("%s Failed build spawn point nodes. Increase MAX_SPAWN_SPOTS beyond %i.\n", __func__, MAX_SP_NODES);
			return;
		}

		// Trace down and find the ground
		VectorCopy(spot->s.origin, spots_mod[sp_counter]);
		spots_mod[sp_counter][2] -= 1024;
		tr = gi.trace(spot->s.origin, mins, maxs, spots_mod[sp_counter], spot, MASK_SOLID | MASK_OPAQUE);
		spots_mod[sp_counter][2] = tr.endpos[2] + 32;
		//gi.bprintf(PRINT_MEDIUM, "%s node %i at %f vs %f\n", __func__, i, spot->s.origin[2], spots_mod[sp_counter][2]);

		sp_counter++;
	}

	// Exit if no DM SPs found
	if (!sp_counter)
	{
		Com_Printf("%s Failed build spawn point nodes. No deathmatch spawn points found.\n", __func__);
		return;
	}

	// Find all SPNs
	for (i = 0; i < numnodes; i++)
	{
		if (nodes[i].inuse == false) continue; // Ignore nodes not in use

		if (nodes[i].type == NODE_SPAWNPOINT)
		{
			VectorCopy(nodes[i].origin, spn_loc[spn_total]);	// Locatiom
			spn_nodes[spn_total] = i;							// Node num
			spn_total++;										// Total
		}
	}

	// Find any non-SPN nodes sitting on a SP and move them up so they're out of the way and can be used elsewhere on the map
	for (s = 0; s < sp_counter; s++)
	{
		for (i = 0; i < numnodes; i++)
		{
			if (nodes[i].inuse == false) continue; // Ignore nodes not in use

			if (nodes[i].type != NODE_SPAWNPOINT)
			{
				if (nodes[i].origin[0] == spots_mod[s][0] && nodes[i].origin[1] == spots_mod[s][1] && nodes[i].origin[2] == spots_mod[s][2])
				{
					nodes[i].origin[2] += 32; // Move node up a little
					//Com_Printf("%s Moving node %i of type %i to %f %f %f\n", __func__, i, nodes[i].type, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);
					requiresSave = true;
				}
			}
		}
	}

	// Map has SPNs - A) Find if a SP has an SPN - B) Flag used SPNs - C) Move unused SPNs to 0,0,0
	if (spn_total)
	{
		for (s = 0; s < sp_counter; s++)
		{
			for (n = 0; n < spn_total; n++)
			{
				if (spn_loc[n][0] == spots_mod[s][0] && spn_loc[n][1] == spots_mod[s][1] && spn_loc[n][2] == spots_mod[s][2])
				{
					//Com_Printf("%s Found good node %i at %f %f %f\n", __func__, spn_nodes[n], spots_mod[s][0], spots_mod[s][1], spots_mod[s][2]);
					sp_has_spn[s] = true;
					spn_inuse[n] = true;
				}
			}
		}

		// SP has no SPN - lets give it one
		for (s = 0; s < sp_counter; s++)
		{
			if (sp_has_spn[s] == false) // If no SPN on SP
			{
				// Try using an existing SPN (if there is one free)
				for (n = 0; n < spn_total; n++)
				{
					if (spn_inuse[n] == false)
					{
						sp_has_spn[s] = true;
						spn_inuse[n] = true;

						i = spn_nodes[n];
						//Com_Printf("%s Moving SPN %i from %f %f %f to %f %f %f\n", __func__, spn_nodes[n], nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2], spots_mod[s][0], spots_mod[s][1], spots_mod[s][2]);

						BOTLIB_RemoveAllNodeLinksFrom(i);			// Break all connections to spawn point node
						VectorCopy(spots_mod[s], nodes[i].origin);	// Then move node to spot location

						// Also move the entitiy location
						for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
						{
							if (ent && ent->node_num == i)
							{
								VectorCopy(spots_mod[s], ent->s.origin);
								break;
							}
						}

						break;
					}
				}

				// Otherwise make a new SPN
				if (sp_has_spn[s] == false)
				{
					if (numnodes < MAX_PNODES)
					{
						VectorCopy(spots_mod[s], nodes[numnodes].origin);
						nodes[numnodes].type = NODE_SPAWNPOINT;
						nodes[numnodes].nodenum = numnodes;

						// Init links
						for (i = 0; i < MAXLINKS; i++)
						{
							nodes[numnodes].links[i].targetNode = INVALID;
						}

						//Com_Printf("%s Adding new node %i at %f %f %f\n", __func__, numnodes, spots_mod[s][0], spots_mod[s][1], spots_mod[s][2]);

						if (debug_mode)
							ACEND_ShowNode(numnodes);

						numnodes++;
					}
				}
			}
		}

		// Lastly, deal with situations where we have more SPNs than SPs - so move the unused SPN to origin 0,0,0
		for (n = 0; n < spn_total; n++)
		{
			if (spn_inuse[n] == false)
			{
				vec3_t spn_unused = { 0,0,0 };
				i = spn_nodes[n];
				//Com_Printf("%s Moving unused SPN %i from %f %f %f to %f %f %f\n", __func__, spn_nodes[n], nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2], spn_unused[0], spn_unused[1], spn_unused[2]);

				BOTLIB_RemoveAllNodeLinksFrom(i);			// Break all connections to spawn point node
				VectorCopy(spn_unused, nodes[i].origin);	// Then move node to spot location

				// Also move the entitiy location
				for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
				{
					if (ent && ent->node_num == i)
					{
						VectorCopy(spn_unused, ent->s.origin);
						break;
					}
				}
			}
		}
	}

	// Map has no SPNs - so lets add them
	else
	{
		requiresSave = true;
		for (s = 0; s < sp_counter; s++)
		{
			// Add spawn point node
			if (numnodes < MAX_PNODES)
			{
				VectorCopy(spots_mod[s], nodes[numnodes].origin);
				nodes[numnodes].type = NODE_SPAWNPOINT;
				nodes[numnodes].nodenum = numnodes;

				// Init links
				for (i = 0; i < MAXLINKS; i++)
				{
					nodes[numnodes].links[i].targetNode = INVALID;
				}

				//Com_Printf("%s [New Map] Adding new node %i at %f %f %f\n", __func__, numnodes, spots_mod[s][0], spots_mod[s][1], spots_mod[s][2]);

				if (debug_mode)
					ACEND_ShowNode(numnodes);

				numnodes++;
			}
		}
	}

	// Save changes
	//if (requiresSave)
	//	ACEND_SaveNodes();
}

///////////////////////////////////////////////////////////////////////
// Cache all the Point of Interest (PoI) nodes for faster access
// Also include Spawn Point Nodes (SPNs) so bots visit spawn locs
///////////////////////////////////////////////////////////////////////
void ACEND_CachePointOfInterestNodes(void)
{
	num_poi_nodes = 0;
	// Init
	for (int i = 0; i < numnodes; i++)
	{
		if (nodes[i].inuse == false) continue; // Ignore nodes not in use

		poi_nodes[num_poi_nodes] = INVALID;
	}
	for (int i = 0; i < numnodes; i++)
	{
		if (nodes[i].inuse == false) continue; // Ignore nodes not in use

		if (num_poi_nodes + 1 >= MAX_POI_NODES)
		{
			//Com_Printf("%s Failed to cache POI nodes. Maximum POI allowed %i\n", __func__, MAX_POI_NODES);
			return;
		}

		if (nodes[i].type == NODE_POI)
		{
			//Com_Printf("%s Caching POI node %i at %f %f %f\n", __func__, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);
			poi_nodes[num_poi_nodes] = i;
			num_poi_nodes++;
		}
		else if (nodes[i].type == NODE_SPAWNPOINT)
		{
			//Com_Printf("%s Caching SPAWN POINT node %i at %f %f %f\n", __func__, i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]);
			poi_nodes[num_poi_nodes] = i;
			num_poi_nodes++;
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Builds an array of nodes that are attached to an entity 
// (such as a NODE_DOOR being attached to a func_door_rotating or func_door entity)
///////////////////////////////////////////////////////////////////////
void ACEND_BuildEntNodeTable(void)
{
	// General counter, nodes
	int i, n;

	// Door nodes
	qboolean is_bad_door_node;					// If door node is missing
	vec3_t bad_door_node_pos[MAX_DOOR_NODES];	// Missing door positions

	edict_t* ent;								// Entities

	// Door counters
	byte doors_total = 0;						// Total doors found
	byte doors_good = 0;						// Total good doors
	byte doors_bad = 0;							// How many doors are missing nodes
	byte doors_fixed = 0;						// Total bad doors we've fixed

	byte door_nodes[MAX_DOOR_NODES];			// Cache door nodes
	//byte bad_door_nodes[MAX_DOOR_NODES];		// Cache bad door nodes

	for (i = 0; i < numnodes; i++)
	{
		if (nodes[i].inuse == false) continue; // Ignore nodes not in use

		if (nodes[i].type == NODE_DOOR)
		{
			if (doors_total + 1 >= MAX_DOOR_NODES)
			{
				Com_Printf("%s Failed build door point nodes. Increase MAX_DOOR_NODES beyond %i.\n", __func__, MAX_DOOR_NODES);
				return;
			}
			door_nodes[doors_total] = i;
			doors_total++;
		}
	}

	// Existing doors
	for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
	{
		// filter out crap
		if (ent->solid == SOLID_NOT)
			continue;

		if (!ent->classname)
			continue;

		// Doors
		if (strcmp(ent->classname, "func_door_rotating") == 0 || strcmp(ent->classname, "func_door") == 0)
		{
			// First lets find any existing doors nodes without entities attached
			// The nodes must be found in the correct place
			//==============================================================
			//
			vec3_t position;
			// Find mid point of door max and min and put the node there
			VectorClear(position);
			// find center of door
			position[0] = ent->absmin[0] + ((ent->maxs[0] - ent->mins[0]) / 2);
			position[1] = ent->absmin[1] + ((ent->maxs[1] - ent->mins[1]) / 2);
			position[2] = ent->absmin[2] + 32;
			// We now have to create TWO nodes, one each side of the door
			// See if the 'door' is wider in the x or y direction
			// Create in the 'y' direction
			if ((ent->absmax[0] - ent->absmin[0]) > (ent->absmax[1] - ent->absmin[1]))
			{
				is_bad_door_node = true;
				position[1] += 48;
				for (i = 0; i < doors_total; i++)
				{
					n = door_nodes[i];
					if (nodes[n].origin[0] == position[0] && nodes[n].origin[1] == position[1] && nodes[n].origin[2] == position[2])
					{
						doors_good++;
						is_bad_door_node = false;
						node_ents[n] = ent;	// Add door as an entity reference
						Com_Printf("%s Attaching ent to [in place] door node %i at %f %f %f\n", __func__, n, position[0], position[1], position[2]);
					}
				}
				if (is_bad_door_node)
				{
					VectorCopy(position, bad_door_node_pos[doors_bad]);
					doors_bad++;
				}

				is_bad_door_node = true;
				position[1] -= 82;
				for (i = 0; i < doors_total; i++)
				{
					n = door_nodes[i];
					if (nodes[n].origin[0] == position[0] && nodes[n].origin[1] == position[1] && nodes[n].origin[2] == position[2])
					{
						doors_good++;
						is_bad_door_node = false;
						node_ents[n] = ent;	// Add door as an entity reference
						Com_Printf("%s Attaching ent to [in place] door node %i at %f %f %f\n", __func__, n, position[0], position[1], position[2]);
					}
				}
				if (is_bad_door_node)
				{
					VectorCopy(position, bad_door_node_pos[doors_bad]);
					doors_bad++;
				}
			}
			// Create in the 'x' direction
			else
			{
				is_bad_door_node = true;
				position[0] += 48;
				for (i = 0; i < doors_total; i++)
				{
					n = door_nodes[i];
					if (nodes[n].origin[0] == position[0] && nodes[n].origin[1] == position[1] && nodes[n].origin[2] == position[2])
					{
						doors_good++;
						is_bad_door_node = false;
						node_ents[n] = ent;	// Add door as an entity reference
						Com_Printf("%s Attaching ent to [in place] door node %i at %f %f %f\n", __func__, n, position[0], position[1], position[2]);
					}
				}
				if (is_bad_door_node)
				{
					VectorCopy(position, bad_door_node_pos[doors_bad]);
					doors_bad++;
				}

				is_bad_door_node = true;
				position[0] -= 82;
				for (i = 0; i < doors_total; i++)
				{
					n = door_nodes[i];
					if (nodes[n].origin[0] == position[0] && nodes[n].origin[1] == position[1] && nodes[n].origin[2] == position[2])
					{
						doors_good++;
						is_bad_door_node = false;
						node_ents[n] = ent;	// Add door as an entity reference
						Com_Printf("%s Attaching ent to [in place] door node %i at %f %f %f\n", __func__, n, position[0], position[1], position[2]);
					}
				}
				if (is_bad_door_node)
				{
					VectorCopy(position, bad_door_node_pos[doors_bad]);
					doors_bad++;
				}
			}
			//
			//==============================================================
		}
	}

	// Door fixes
	// For door nodes that are out of place and without entities attached
	//===================================================================
	if (doors_bad)
	{
		Com_Printf("%s GD:%i BD:%i TD:%i (GD+BD:%i)\n", __func__, doors_good, doors_bad, doors_total, doors_good + doors_bad);

		// Add new door node
		if (doors_good + doors_bad < doors_total)
		{
			return;
		}

		for (i = 0; i < doors_total; i++)
		{
			n = door_nodes[i];

			if (nodes[n].type == NODE_DOOR)
			{
				if (node_ents[n] == 0) // NULL
				{
					BOTLIB_RemoveAllNodeLinksFrom(n);			// Break all connections to this node

					// Now move the node into place of the missing door node position
					if (doors_fixed != doors_bad)
					{
						VectorCopy(bad_door_node_pos[doors_fixed], nodes[n].origin);
						doors_fixed++;
					}

					Com_Printf("%s Attaching ent to [out of place] door node %i at %f %f %f\n", __func__, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);
				}
			}
		}

		// Too many door nodes - convert to node type 0 and send to location 0,0,0
		if (doors_good + doors_bad > doors_total)
		{
			/*
			vec3_t zero = { 0,0,0 };
			for (i = doors_fixed; i < doors_bad; i++)
			{
				//bad_door_nodes
				//VectorCopy(zero, bad_door_node_pos[i]);
				//i++;
			}
			*/
		}

		// Run once more to attach all door nodes to door ents
		//ACEND_BuildEntNodeTable();
	}
	//===================================================================
}
///////////////////////////////////////////////////////////////////////
// Build an array of visibility nodes, i.e. If node X can see node Y
// These nodes will be calculated once, saved/loaded to/from file, 
// and cached in memory for quick lookup access.
///////////////////////////////////////////////////////////////////////
void ACEND_BuildVisibilityNodes(void)
{
	short sn, dn; // Source node, destination node
	short nvl_counter; // Node vis list counter
	trace_t tr; // Trace
	//vec3_t down; // Looking for ground
	//edict_t* ent; // Entities

	clock_t begin = clock();

	Com_Printf("%s is working...\n", __func__);

	// Init node vis list
	for (sn = 0; sn < numnodes; sn++)
	{
		num_vis_nodes = 0; // Reset global vis node counter
		nvl_counter = 0;
		for (dn = 0; dn < MAX_VIS_NODES; dn++)
		{
			//node_vis_list[sn][nvl_counter++] = INVALID;
			node_vis_list[sn][dn] = INVALID;
		}

		/*
		// Moves selective nodes to eye level
		// For all the move nodes lets make sure that they're at the player eye level
		if (nodes[sn].type == NODE_MOVE || nodes[sn].type == NODE_POI)
		{
			VectorCopy(nodes[sn].origin, down);
			down[2] -= 128; // Distance downward, keeping the distance shallow because a node might be up high, so leave it alone
			tr = gi.trace(nodes[sn].origin, NULL, NULL, down, NULL, MASK_SOLID | MASK_OPAQUE); // Trace down and find the ground

			tr.endpos[2] += 48;	// self->viewheight (Eye position)
			//Com_Printf("%s Adjusting node %i [%f] to [%f]\n", __func__, sn, nodes[sn].origin[2], tr.endpos[2]);
			VectorCopy(tr.endpos, nodes[sn].origin); // Adjust the node to eye pos

			// Also move the entitiy location in realtime
			for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
			{
				if (ent && ent->node_num == sn)
				{
					VectorCopy(tr.endpos, ent->s.origin);
					break;
				}
			}
		}
		*/
	}

	// Build it
	for (sn = 0; sn < numnodes; sn++)
	{
		nvl_counter = 0;
		for (dn = 0; dn < numnodes; dn++)
		{
			if (sn == dn) // Skip self
				continue;

			tr = gi.trace(nodes[sn].origin, NULL, NULL, nodes[dn].origin, NULL, MASK_SOLID | MASK_OPAQUE); // Trace from source to destination node
			if (tr.fraction == 1.0)
			{
				// Build the array
				node_vis[sn][dn] = dn;

				// Build the list
				if (nvl_counter + 1 < MAX_VIS_NODES)
					node_vis_list[sn][nvl_counter++] = dn;

				num_vis_nodes++; // Total visibility nodes
			}
			else
			{
				node_vis[sn][dn] = INVALID; // X cannot see Y
			}
		}
	}

	clock_t end = clock();
	Com_Printf("%s execution took %f seconds to build visibility for %i nodes [%i * %i]\n", __func__, (double)(end - begin) / CLOCKS_PER_SEC, numnodes * numnodes, numnodes, numnodes);
}
///////////////////////////////////////////////////////////////////////
// Can node X can see node Y
// Returns true/false based on precalculated cached tracelines
///////////////////////////////////////////////////////////////////////
qboolean ACEND_IsNodeVisibleToNodes(short x, short y)
{
	if (x < numnodes && y < numnodes) // Bounce check
	{
		if (node_vis[x][y] != INVALID) // Is the node visible?
			return true;
	}

	return false;
}
///////////////////////////////////////////////////////////////////////
// Returns a random node that X can see
// Otherwise return INVALID if no visibility
///////////////////////////////////////////////////////////////////////
short ACEND_GetRandomVisibleNode(short x)
{
    short i;
    short dn; // Destination node
    short vis_size = 0;

    if (x + 1 < numnodes) // Bounds check
    {
        // Work out the size of the vis list
        for (i = 0; i < MAX_VIS_NODES; i++)
        {
            if (node_vis_list[x][i] != INVALID)
                vis_size++;
        }

        if (vis_size == 0) // No visible nodes
            return INVALID;

        dn = (short)(random() * vis_size); // Pick a random destination node

        if (dn >= 0 && dn < vis_size) // Ensure dn is within bounds
            return node_vis_list[x][dn];
    }

    return INVALID;
}
//rekkie -- DEV_1 -- e

///////////////////////////////////////////////////////////////////////
// Show the node for debugging (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_ShowNode(int node)
{
	edict_t *ent = NULL;

//	return; // commented out for now. uncommend to show nodes during debugging,
	        // but too many will cause overflows. You have been warned.

	//rekkie -- adjust node placement height for visual aid - NODE_Z_HEIGHT places @ ground level -- s
	float adjusted_node_height = 0;
	if (0)
		adjusted_node_height = NODE_Z_HEIGHT;
	//rekkie -- adjust node placement height for visual aid - NODE_Z_HEIGHT places @ ground level -- e

	//rekkie -- DEV_1 -- s
	// Ignore invalid nodes
	if (node < 0)
		return;
	if (nodes[node].nodenum < 0)
		return;

	// Ignore placement of nodes on top of an existing node
	for (ent = g_edicts + 1; ent < &g_edicts[globals.num_edicts]; ent++)
	{
		if (ent->inuse == false || ent->client) // skip these
			continue;

		// Check if ent has same origin as node
		if (nodes[node].origin[0] == ent->s.origin[0] && nodes[node].origin[1] == ent->s.origin[1] && nodes[node].origin[2] == (ent->s.origin[2] + adjusted_node_height))
		{
			//Com_Printf("%s already showing node %i\n", __func__, node);
			return;
		}
	}
	//rekkie -- DEV_1 -- e

	ent = G_Spawn();

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if(nodes[node].type == NODE_MOVE)
		ent->s.renderfx = RF_SHELL_BLUE;
	else if (nodes[node].type == NODE_WATER)
		ent->s.renderfx = RF_SHELL_RED;
	else			
		ent->s.renderfx = RF_SHELL_GREEN; // action nodes

	ent->s.modelindex = gi.modelindex ("models/items/ammo/grenades/medium/tris.md2");
	ent->owner = ent;
	//ent->nextthink = level.framenum + 60 * HZ; // 1 minute (60*hz) is long enough!
	ent->think = G_FreeEdict;                
	ent->dmg = 0;

	//rekkie -- DEV_1 -- s
	//ent->nextthink = level.framenum + 6000 * HZ; // rekkie -- extra long
	ent->nextthink = level.framenum + 6 * HZ; // rekkie -- very short

	VectorSet(ent->mins, -4, -4, -4);
	VectorSet(ent->maxs, 4, 4, 4);

	ent->model = "models/objects/grenades/tris.md2";
	ent->s.modelindex = gi.modelindex("models/objects/grenades/tris.md2");

	ent->classname = ent->node_name;
	ent->node_num = node;

	if (nodes[node].type == NODE_MOVE)
	{
		ent->s.renderfx = RF_SHELL_BLUE;
		Q_strncpyz(ent->node_name, va("node move [%d]", node), sizeof(ent->node_name));
	}
	else if (nodes[node].type == NODE_WATER)
	{
		ent->s.renderfx = RF_SHELL_RED;
		Q_strncpyz(ent->node_name, va("node water [%d]", node), sizeof(ent->node_name));
	}
	//rekkie -- DEV_1 -- s
	else if (nodes[node].type == NODE_JUMPPAD)
	{
		ent->s.renderfx = RF_SHELL_GREEN;
		Q_strncpyz(ent->node_name, va("node jumppad [%d]", node), sizeof(ent->node_name));
	}
	else if (nodes[node].type == NODE_SPAWNPOINT)
	{
		VectorSet(ent->mins, -16, -16, -24);
		VectorSet(ent->maxs, 16, 16, 32);
		ent->model = "models/objects/dmspot/tris.md2";								// deathmatch spawn point
		ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
		//ent->s.renderfx = RF_SHELL_BLUE;
		Q_strncpyz(ent->node_name, va("node spawn point [%d]", node), sizeof(ent->node_name));
	}
	else if (nodes[node].type == NODE_POI)
	{
		ent->s.renderfx = RF_SHELL_RED;
		Q_strncpyz(ent->node_name, va("node point of interest [%d]", node), sizeof(ent->node_name));
	}
	//rekkie -- DEV_1 -- e
	else
	{
		ent->s.renderfx = RF_SHELL_GREEN; // action nodes
		Q_strncpyz(ent->node_name, va("node action [%d]", node), sizeof(ent->node_name));
	}
	//rekkie -- DEV_1 -- e

	VectorCopy(nodes[node].origin, ent->s.origin);

	ent->s.origin[2] -= adjusted_node_height; //rekkie -- place node to ground level for visual aid

	gi.linkentity (ent);
}
/*
///////////////////////////////////////////////////////////////////////
// Draws the current path (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_DrawPath(edict_t *self)
{
	int current_node, goal_node, next_node;

	current_node = show_path_from;
	goal_node = show_path_to;

	// RiEvEr - rewritten to use Ant system
	AntStartSearch( self, current_node, goal_node);

	next_node = SLLfront(&self->pathList);

	// Now set up and display the path
	while( current_node != goal_node && current_node != INVALID && next_node != INVALID)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (nodes[current_node].origin);
		gi.WritePosition (nodes[next_node].origin);
		gi.multicast (nodes[current_node].origin, MULTICAST_PVS);
		current_node = next_node;
		SLLpop_front( &self->pathList);
		next_node = SLLfront(&self->pathList);
	}
}
*/
///////////////////////////////////////////////////////////////////////
// Turns on showing of the path, set goal to -1 to 
// shut off. (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_ShowPath(edict_t *self, int goal_node)
{
	show_path_from = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
	show_path_to = goal_node;
}

/*
///////////////////////////////////////////////////////////////////////
// Add a node of type ?
///////////////////////////////////////////////////////////////////////
int ACEND_AddNode(edict_t *self, int type)
{
	vec3_t v1,v2;
	int i;

	// Block if we exceed maximum
	if (numnodes + 1 > MAX_PNODES)
		return false;
	
	// Set location
	VectorCopy(self->s.origin, nodes[numnodes].origin);
	nodes[numnodes].origin[2] += 8;

	// Set type
	nodes[numnodes].type = type;
	// Set number - RiEvEr
	nodes[numnodes].nodenum = numnodes;

	// Clear out the link information - RiEvEr
	for( i = 0; i< MAXLINKS; i++)
	{
		nodes[numnodes].links[i].targetNode = INVALID;
	}

	/////////////////////////////////////////////////////
	// ITEMS
	// Move the z location up just a bit.
	if(type == NODE_ITEM)
	{
		nodes[numnodes].origin[2] += 16;
		numitemnodes++;
	}

	// Teleporters
	if(type == NODE_TELEPORTER)
	{
		// Up 32
		nodes[numnodes].origin[2] += 32;
	}

	// Doors
	if(type == NODE_DOOR)
	{
		vec3_t	position;
		// Find mid point of door max and min and put the node there
		VectorClear(position);
        // find center of door
		position[0] = self->absmin[0] + ((self->maxs[0] - self->mins[0]) /2);
		position[1] = self->absmin[1] + ((self->maxs[1] - self->mins[1]) /2);
		position[2] = self->absmin[2] + 32;
		// We now have to create TWO nodes, one each side of the door
		// See if the 'door' is wider in the x or y direction
		if( (self->absmax[0] - self->absmin[0]) > (self->absmax[1] - self->absmin[1]) )
		{
			// Create in the 'y' direction

			// First node (Duplication deliberate!)
			// Set type
			nodes[numnodes].type = type;
			// Set number - RiEvEr
			nodes[numnodes].nodenum = numnodes;
			// Set position
			position[1] +=48;
			VectorCopy(position, nodes[numnodes].origin);

			// Second node
			numnodes++;
			// Set type
			nodes[numnodes].type = type;
			// Set number - RiEvEr
			nodes[numnodes].nodenum = numnodes;
			// Set position
			position[1] -=82;
			VectorCopy(position, nodes[numnodes].origin);
		}
		else
		{
			// Create in the 'x' direction

			// First node (Duplication deliberate!)
			// Set type
			nodes[numnodes].type = type;
			// Set number - RiEvEr
			nodes[numnodes].nodenum = numnodes;
			// Set position
			position[0] +=48;
			VectorCopy(position, nodes[numnodes].origin);

			// Second node
			numnodes++; 
			// Set type
			nodes[numnodes].type = type;
			// Set number - RiEvEr
			nodes[numnodes].nodenum = numnodes;
			// Set position
			position[0] -=82;
			VectorCopy(position, nodes[numnodes].origin);
		}
		numnodes++;
		return numnodes-1; // return the second node added

	}

	if(type == NODE_LADDER)
	{
		nodes[numnodes].type = NODE_LADDER;
				
		if(debug_mode)
		{
			debug_printf("Node added %d type: Ladder\n",numnodes);
			ACEND_ShowNode(numnodes);
		}
		
		numnodes++;
		return numnodes-1; // return the node added

	}

	// For platforms drop two nodes one at top, one at bottom
	if(type == NODE_PLATFORM)
	{
		VectorCopy(self->maxs,v1);
		VectorCopy(self->mins,v2);
		
		// To get the center
		nodes[numnodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
		nodes[numnodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
		nodes[numnodes].origin[2] = self->maxs[2];
			
		if(debug_mode)	
			ACEND_ShowNode(numnodes);
		
		numnodes++;

		nodes[numnodes].origin[0] = nodes[numnodes-1].origin[0];
		nodes[numnodes].origin[1] = nodes[numnodes-1].origin[1];
		nodes[numnodes].origin[2] = self->mins[2]+64;
		
		nodes[numnodes].type = NODE_PLATFORM;

		// Add a link 
		//RiEvEr modified to pass in calling entity
		ACEND_UpdateNodeEdge(self, numnodes, numnodes-1);			
		
		if(debug_mode)
		{
			debug_printf("Node added %d type: Platform\n",numnodes);
			ACEND_ShowNode(numnodes);
		}

		numnodes++;

		return numnodes -1;
	}
		
	if(debug_mode)
	{
		if(nodes[numnodes].type == NODE_MOVE)
			debug_printf("Node added %d type: Move\n",numnodes);
		else if(nodes[numnodes].type == NODE_TELEPORTER)
			debug_printf("Node added %d type: Teleporter\n",numnodes);
		else if(nodes[numnodes].type == NODE_ITEM)
			debug_printf("Node added %d type: Item\n",numnodes);
		else if(nodes[numnodes].type == NODE_WATER)
			debug_printf("Node added %d type: Water\n",numnodes);
		else if(nodes[numnodes].type == NODE_GRAPPLE)
			debug_printf("Node added %d type: Grapple\n",numnodes);

		ACEND_ShowNode(numnodes);
	}
		
	numnodes++;
	
	return numnodes-1; // return the node added
}
*/



// RiEvEr
//=======================================
// ReverseLink
//=======================================
// Takes the path BACK to where we came from
// and tries to link the two nodes
// This helps make good path files
//
void ACEND_ReverseLink( edict_t *self, int from, int to )
{
	int	i;
	trace_t	trace;
	vec3_t	min,max;
	
	if(from == INVALID || to == INVALID || from == to)
		return; // safety

	// Need to trace from -> to and check heights
	// if from is much lower than to, forget it
	if( (nodes[from].origin[2]+32.0) < (nodes[to].origin[2]) )
	{
		// May not be able to jump that high so do not allow the return link
		return;
	}
//	VectorCopy(self->mins, min);
//	if( (nodes[from].origin[2]) < (nodes[2].origin[2]) )
//		min[2]  =0;	// Allow for steps etc.
//	VectorCopy(self->maxs, max);
//	if( (nodes[from].origin[2]) > (nodes[2].origin[2]) )
//		max[2] =0;	// Could be a downward sloping feature above our head
	VectorCopy( vec3_origin, min);
	VectorCopy( vec3_origin, max);



	// This should not be necessary, but I've heard that before!
	// Now trace it again
	trace = gi.trace( nodes[from].origin, min, max, nodes[to].origin, self, MASK_SOLID);
//	trace = gi.trace( nodes[from].origin, tv(-8,-8,0), tv(8,8,0), nodes[to].origin, self, MASK_SOLID);
	if( trace.fraction < 1.0)
	{
		// can't get there for some reason
		return;
	}
	// Add the link
	path_table[from][to] = to;

	// Checks if the link exists and then may create a new one - RiEvEr
	for( i=0; i<MAXLINKS; i++)
	{
		if ( nodes[from].links[i].targetNode == to)
			break;
		if ( nodes[from].links[i].targetNode == INVALID)
		{
			// RiEvEr
			// William uses a time factor here, whereas I use distance
			// His is possibly more efficient
			vec3_t	v;
			float thisCost;

			VectorSubtract(nodes[from].origin, nodes[to].origin, v); // subtract first
			thisCost = VectorLength(v);
			nodes[from].links[i].targetNode = to;
			nodes[from].links[i].cost = thisCost;
			if(debug_mode)
				debug_printf("ReverseLink %d -> %d\n", from, to);
			break;
		}
	}

	// Now for the self-referencing part, linear time for each link added
	for(i=0;i<numnodes;i++)
	{
		if (path_table[i][from] != INVALID)
		{
			if (i == to)
				path_table[i][to] = INVALID; // make sure we terminate
			else
				path_table[i][to] = path_table[i][from];
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Remove a node edge
///////////////////////////////////////////////////////////////////////
void ACEND_RemoveNodeEdge(edict_t *self, int from, int to)
{
	int i;

	if (from == INVALID || to == INVALID)
		return; // safety

	//if(debug_mode) 
	//	debug_printf("%s: Removing Edge %d -> %d\n", self->client->pers.netname, from, to);
		
	////path_table[from][to] = INVALID; // set to invalid

	// RiEvEr
	// Now we must remove the link from the antpath system
	// Get the link information
	for( i=0; i<MAXLINKS; i++)
	{
		// If it matches our link remove it
		if ( nodes[from].links[i].targetNode == to)
			break;
	}
	// Fill the gap caused by the removal by left shifting from the position of the removed link
	for( ; i<MAXLINKS; i++)
	{
		// Terminate the last link by making it INVALID
		if( i == MAXLINKS-1)
		{
			nodes[from].links[i].targetNode = INVALID;
			nodes[from].links[i].targetNodeType = INVALID;
			nodes[from].links[i].cost = INVALID;

		}

		// Move all the other info down to fill the gap (left shift).
		// This *MUST* be done because pathing expects links to be contiguous, 
		// with the first INVALID link found becoming the terminator.
		// IE: First link --> [LINK]-[LINK]-[LINK]-[LINK]-[INVALID] <-- terminator [INVALID]-[INVALID] <-- up to MAXLINKS
		//if(nodes[from].links[i].targetNode != INVALID)
		if (i + 1 < MAXLINKS)
		{
			nodes[from].links[i].targetNode = nodes[from].links[i + 1].targetNode;
			nodes[from].links[i].targetNodeType = nodes[from].links[i + 1].targetNodeType;
			nodes[from].links[i].cost = nodes[from].links[i + 1].cost;
		}
	}
	nodes[from].num_links--; // Reduce number of links

	//R

	// Make sure this gets updated in our path array
	////for(i=0;i<numnodes;i++)
	////{
	////	if (path_table[from][i] == to)
	////		path_table[from][i] = INVALID;
	////}
}


/*
///////////////////////////////////////////////////////////////////////
// This function will resolve all paths that are incomplete
// usually called before saving to disk
///////////////////////////////////////////////////////////////////////
void ACEND_ResolveAllPaths()
{
	int i, from, to;
	int num=0;

//	return;	// RiEvEr - disabled since it will interfere with the optimiser

	gi.bprintf(PRINT_HIGH,"Resolving all paths...");

	for(from=0;from<numnodes;from++)
	{
		for(to=0;to<numnodes;to++)
		{
			// update unresolved paths
			// Not equal to itself, not equal to -1 and equal to the last link
			if(from != to && path_table[from][to] == to)
			{
				num++;

				// Now for the self-referencing part linear time for each link added
				for(i=0;i<numnodes;i++)
					if(path_table[i][from] != -1)
					{
						if(i == to)
							path_table[i][to] = -1; // make sure we terminate
						else
							path_table[i][to] = path_table[i][from];
					}
			}
		}
	}

	gi.bprintf(PRINT_MEDIUM,"done (%d updated)\n",num);
}


///////////////////////////////////////////////////////////////////////
// Save to disk file
//
// Since my compression routines are one thing I did not want to
// release, I took out the compressed format option. Most levels will
// save out to a node file around 50-200k, so compression is not really
// a big deal.
///////////////////////////////////////////////////////////////////////
void ACEND_SaveNodes()
{
	FILE *pOut;
	char filename[60];
	int i,j;
	int version;
	cvar_t	*game_dir;

	if (dedicated->value) return; // Ignore if dedicated server //rekkie -- DEV_1

	version = LTK_NODEVERSION;

	game_dir = gi.cvar ("game", "action", 0);

	//@@ change 'nav' to 'terrain' to line up with William
#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\terrain\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".ltk");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/terrain/");
	strcat(filename,level.mapname);
	strcat(filename,".ltk");
#endif
	
	// Resolve paths
	ACEND_ResolveAllPaths();

	gi.bprintf(PRINT_MEDIUM,"Saving node table...");

	//strcpy(filename,"action\\nav\\");
	//strcat(filename,level.mapname);
	//strcat(filename,".nod");

	if((pOut = fopen(filename, "wb" )) == NULL)
		return; // bail
	
	fwrite(&version,sizeof(int),1,pOut); // write version
	fwrite(&numnodes,sizeof(int),1,pOut); // write count
	fwrite(&num_items,sizeof(int),1,pOut); // write facts count
	
	fwrite(nodes,sizeof(node_t),numnodes,pOut); // write nodes
	
	for(i=0;i<numnodes;i++)
		for(j=0;j<numnodes;j++)
			fwrite(&path_table[i][j],sizeof(short int),1,pOut); // write count
		
	//fwrite(item_table,sizeof(item_table_t),num_items,pOut); 		// write out the fact table

	// Write out the fact table with sanitized pointers.
	for( i = 0; i < num_items; i ++ )
	{
		item_table_t item;
		memcpy( &item, &(item_table[i]), sizeof(item) );
		item.ent = NULL;
		fwrite( &item, sizeof(item), 1, pOut );
	}

	//rekkie -- DEV_1 -- s
	// Could compress all of this using ZLIB or LZW https://github.com/radekstepan/LZW
	//ACEND_BuildVisibilityNodes(); // Rebuild node vis

	for (i = 0; i < numnodes; i++)
		for (j = 0; j < numnodes; j++)
			fwrite(&node_vis[i][j], sizeof(short int), 1, pOut); // write node visibility

	for (i = 0; i < numnodes; i++)
		for (j = 0; j < MAX_VIS_NODES; j++)
			fwrite(&node_vis_list[i][j], sizeof(short int), 1, pOut); // write node visibility list
	//rekkie -- DEV_1 -- e

	fclose(pOut);
	
	gi.bprintf(PRINT_MEDIUM,"done.\n");
}

///////////////////////////////////////////////////////////////////////
// Read from disk file
///////////////////////////////////////////////////////////////////////
void ACEND_LoadNodes(void)
{
	FILE *pIn;
	int i,j;
	char filename[60];
	int version;
	cvar_t	*game_dir;

	game_dir = gi.cvar ("game", "action", 0);

#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\terrain\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".ltk");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/terrain/");
	strcat(filename,level.mapname);
	strcat(filename,".ltk");
#endif

	//strcpy(filename,"action\\nav\\");
	//strcat(filename,level.mapname);
	//strcat(filename,".nod");

	if((pIn = fopen(filename, "rb" )) == NULL)
    {
		// Create item table
		gi.bprintf(PRINT_MEDIUM, "ACE: No node file found, creating new one...");
		ACEIT_BuildItemNodeTable(false);
		gi.bprintf(PRINT_MEDIUM, "done.\n");
		return; 
	}

	// determin version
	fread(&version,sizeof(int),1,pIn); // read version
	
	if(version == LTK_NODEVERSION) 
	{
		gi.bprintf(PRINT_MEDIUM,"ACE: Loading node table...");

		fread(&numnodes,sizeof(int),1,pIn); // read count
		fread(&num_items,sizeof(int),1,pIn); // read facts count
		
		fread(nodes,sizeof(node_t),numnodes,pIn);

		for(i=0;i<numnodes;i++)
			for(j=0;j<numnodes;j++)
				fread(&path_table[i][j],sizeof(short int),1,pIn); // write count
	
		fread(item_table,sizeof(item_table_t),num_items,pIn);
		fclose(pIn);

		//rekkie -- DEV_1 -- s
		for (i = 0; i < numnodes; i++)
			for (j = 0; j < numnodes; j++)
				fread(&node_vis[i][j], sizeof(short int), 1, pIn); // read node visibility

		for (i = 0; i < numnodes; i++)
			for (j = 0; j < MAX_VIS_NODES; j++)
				fread(&node_vis_list[i][j], sizeof(short int), 1, pIn); // read node visibility list
		//rekkie -- DEV_1 -- e

		// Raptor007: Do not trust saved pointers!
		for(i=0;i<MAX_EDICTS;i++)
			item_table[i].ent = NULL;
	}
	else
	{
		// Create item table
		gi.bprintf(PRINT_MEDIUM, "ACE: No node file found, creating new one...");
		ACEIT_BuildItemNodeTable(false);
		gi.bprintf(PRINT_MEDIUM, "done.\n");
		return; // bail
	}
	
	gi.bprintf(PRINT_MEDIUM, "done.\n");
	
	ACEIT_BuildItemNodeTable(true);

	//rekkie -- DEV_1 -- s
	//ACEND_BuildSpawnPointNodes();
	//ACEND_BuildEntNodeTable();
	//ACEND_CachePointOfInterestNodes();
	//rekkie -- DEV_1 -- e
}
*/
//rekkie -- BSP -- s


////////////////////////////////////////////////////////////////////////////////////////////
// Free unsorted_nodes
////////////////////////////////////////////////////////////////////////////////////////////
/*
void ACEND_FreeUnsortedNodes()
{
	if (unsorted_nodes != NULL) free(unsorted_nodes); // Free it
	unsorted_nodes = NULL; // Nullify dangling/wild pointer
}
qboolean ACEND_InitUnsortedNodes()
{
	ACEND_FreeUnsortedNodes(); // Free if already exists
	
	unsorted_nodes = (node_t*)malloc(sizeof(node_t) * MAX_PNODES);
	if (unsorted_nodes == NULL) 
	{
		Com_Printf("%s failed to alloc memory for unsorted_nodes\n", __func__);
		return false;
	}
	memset(unsorted_nodes, 0, sizeof(node_t) * MAX_PNODES);
	return true;
}
*/
