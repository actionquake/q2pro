#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

int num_poi_nodes;
int poi_nodes[MAX_POI_NODES];
node_t *nodes;
static nmesh_t nmesh;

// Free nodes
void BOTLIB_FreeNodes(void)
{
	if (nodes)
	{
		Com_Printf("%s --- Freeing nodes ---\n", __func__);
		free(nodes);
		nodes = NULL; // Nullify dangling pointer
	}

	if (path_table)
	{
		Com_Printf("%s --- Freeing path_table ---\n", __func__);
		for (int r = 0; r < MAX_PNODES; r++) {
			free(path_table[r]);
			path_table[r] = NULL; // Nullify dangling pointer
		}

		free(path_table);
		path_table = NULL; // Nullify dangling pointer
	}
}


// Adds a new or inactive 'inuse == false' flagged node (from deleting a node)
// Returns the node that was added/reused, INVALID if node was not added
int BOTLIB_AddNode(vec3_t origin, vec3_t normal, byte type)
{
	// =================================================================================
	// Avoid placing nodes on 'hurt_touch' trigger areas
	// =================================================================================
	if (0)
	{
		solid_t* trigger_solid = (solid_t*)malloc(sizeof(solid_t) * MAX_EDICTS);
		if (trigger_solid == NULL)
		{
			Com_Printf("%s failed to malloc trigger_solid\n", __func__);
			return INVALID;
		}
		BOTLIB_MakeEntsSolid(trigger_solid); // Save solid state of each ent
		if (BOTLIB_UTIL_CHECK_FOR_HURT(origin))
		{
			Com_Printf("%s ignoring hurt_touch\n", __func__);
			return INVALID;
		}
		BOTLIB_RestoreEntsSolidState(trigger_solid); // Restore solid state, and free memory
	}
	// =================================================================================

	int node = INVALID;
	// Check for any unused nodes first
	for (int i = 0; i < numnodes; i++)
	{
		if (nodes[i].inuse == false)
		{
			node = i; // Free to use this node
			break;
		}
	}

	if (node == INVALID)
	{
		// Absolute max allowed
		if (numnodes + 1 > MAX_PNODES)
		{
			Com_Printf("%s exceeded max nodes, the limit is %d\n", __func__, MAX_PNODES);
			return INVALID;
		}

		// -------------
		// Alloc memory
		// -------------
		node_t* prev = NULL;

		// Nodes
		if (numnodes == 0)
			nodes = (node_t*)malloc(sizeof(node_t));
		else
		{
			prev = nodes; // Keep a copy
			nodes = (node_t*)realloc(nodes, sizeof(node_t) * (numnodes + 1));
		}
		if (nodes == NULL)
		{
			Com_Printf("%s failed to malloc nodes. Out of memory!\n", __func__);
			if (prev)
			{
				free(prev); // Free using the copy, because nodes is null
				nodes = NULL;
				prev = NULL;
			}
			return INVALID;
		}

		node = numnodes;
	}

	// -------------------------------------------------------------------------------------
	// nodes
	// -------------------------------------------------------------------------------------
	nodes[node].area = 0; // Area
	nodes[node].weight = 0; // Weight

	VectorCopy(origin, nodes[node].origin);	// Location
	
	//VectorCopy(origin, nodes[node].origin);	//TODO:::::::::::::::::::::::  Angles (for ladders, sniper spots, etc)

	VectorCopy(normal, nodes[node].normal);	// Surface Normal
	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };
	VectorCopy(mins, nodes[node].mins);		// Box mins
	VectorCopy(maxs, nodes[node].maxs);		// Box maxs
	if (type == NODE_CROUCH)
		nodes[node].maxs[2] = CROUCHING_MAXS2;
	if (type == NODE_BOXJUMP)
	{
		nodes[node].mins[0] = -8;
		nodes[node].mins[1] = -8;
		nodes[node].mins[2] = -12;
		nodes[node].maxs[0] = 8;
		nodes[node].maxs[1] = 8;
		nodes[node].maxs[2] = 16;
	}
	VectorAdd(origin, mins, nodes[node].absmin); // Update absolute box min/max in the world
	VectorAdd(origin, maxs, nodes[node].absmax); // Update absolute box min/max in the world
	nodes[node].type = type;				// Node type
	nodes[node].nodenum = node;				// Node number
	nodes[node].inuse = true;				// Node is inuse
	nodes[node].num_links = 0;				// Node links
	for (int i = 0; i < MAXLINKS; i++) // Init links
	{
		nodes[node].links[i].targetNode = INVALID;						// Link
		nodes[node].links[i].targetNodeType = INVALID;					// Type
		nodes[node].links[i].cost = INVALID;							// Cost
	}

	if (node == numnodes)
		numnodes++; // Update the node counter

	return node;
}

///////////////////////////////////////////////////////////////////////
// Nodes added by the new navigation system
///////////////////////////////////////////////////////////////////////
qboolean DAIC_Add_Node(vec3_t origin, vec3_t normal, byte type)
{
	// Absolute max allowed
	if (numnodes + 1 > MAX_PNODES)
	{
		Com_Printf("%s exceeded max nodes, the limit is %d\n", __func__, MAX_PNODES);
		return false;
	}

	// -------------
	// Alloc memory
	// -------------
	node_t* prev = NULL;

	// Nodes
	if (numnodes == 0)
		nodes = (node_t*)malloc(sizeof(node_t));
	else
	{
		prev = nodes; // Keep a copy
		nodes = (node_t*)realloc(nodes, sizeof(node_t) * (numnodes + 1));
	}
	if (nodes == NULL)
	{
		Com_Printf("%s failed to malloc nodes. Out of memory!\n", __func__);
		if (prev)
		{
			free(prev); // Free using the copy, because nodes is null
			nodes = NULL;
			prev = NULL;
		}
		return false;
	}

	// Unsorted duplicate copy nodes
	if (numnodes == 0)
		unsorted_nodes = (node_t*)malloc(sizeof(node_t));
	else
	{
		prev = unsorted_nodes; // Keep a copy
		unsorted_nodes = (node_t*)realloc(unsorted_nodes, sizeof(node_t) * (numnodes + 1));
	}
	if (unsorted_nodes == NULL)
	{
		Com_Printf("%s failed to malloc unsorted_nodes. Out of memory!\n", __func__);
		if (prev)
		{
			free(prev); // Free using the copy, because unsorted_nodes is null
			unsorted_nodes = NULL;
			prev = NULL;
		}
		return false;
	}

	// -------------------------------------------------------------------------------------
	// nodes
	// -------------------------------------------------------------------------------------
	VectorCopy(origin, nodes[numnodes].origin);	// Location
	VectorCopy(normal, nodes[numnodes].normal);	// Surface Normal
	nodes[numnodes].type = type;				// Node type
	nodes[numnodes].nodenum = numnodes;			// Node number
	nodes[numnodes].num_links = 0;				// Node links
	for (int i = 0; i < MAXLINKS; i++) // Init links
	{
		nodes[numnodes].links[i].targetNode = INVALID;						// Link
		nodes[numnodes].links[i].targetNodeType = INVALID;					// Type
		nodes[numnodes].links[i].cost = INVALID;							// Cost
	}


	// -------------------------------------------------------------------------------------
	// unsorted_nodes
	// -------------------------------------------------------------------------------------
	// Make a duplicate copy of nodes into unsorted_nodes. We'll use this later to optimise links
	VectorCopy(origin, unsorted_nodes[numnodes].origin);	// Location
	VectorCopy(normal, unsorted_nodes[numnodes].normal);	// Surface Normal
	unsorted_nodes[numnodes].type = type;					// Node type
	unsorted_nodes[numnodes].nodenum = numnodes;			// Node number
	unsorted_nodes[numnodes].num_links = 0;					// Node links
	for (int i = 0; i < MAXLINKS; i++) // Init links
	{
		unsorted_nodes[numnodes].links[i].targetNode = INVALID;						// Link
		unsorted_nodes[numnodes].links[i].targetNodeType = INVALID;					// Type
		unsorted_nodes[numnodes].links[i].cost = INVALID;							// Cost
	}

	//if ((numnodes % 1024) == 0)
		//Com_Printf("%s Adding node %i at [%f %f %f]\n", __func__, nodes[numnodes].nodenum, nodes[numnodes].origin[0], nodes[numnodes].origin[1], nodes[numnodes].origin[2]);

	numnodes++; // Update the node counter

	return true;
}


///////////////////////////////////////////////////////////////////////
// Remove node by number, 
// Shifts all the nodes down by one to replace the deleted node
///////////////////////////////////////////////////////////////////////
void ACEND_RemoveNode(edict_t* self, int nodenum)
{
	int i, j, n, l;

	// Sanity
	if (nodenum <= 0 || nodenum + 1 > MAX_PNODES)
		return;

	// Remove links to/from this node
	for (i = 0; i < MAXLINKS; i++)
	{
		// Remove link from other node to our node
		n = nodes[nodenum].links[i].targetNode; // Links from this node to other nodes
		if (n != INVALID)
		{
			for (j = 0; j < MAXLINKS; j++)
			{
				if (nodes[n].links[j].targetNode == nodenum) // If the other node has a link back to our node
				{
					nodes[n].links[j].targetNode = INVALID; // Remove its link to us
				}
			}
		}

		// Remove our link to other node
		nodes[nodenum].links[i].targetNode = INVALID;
	}

	// Update node ent
	edict_t* ent;
	for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
	{
		// Free removed node ent
		if (ent->node_num == nodenum)
		{
			G_FreeEdict(ent);
		}

		// Update node name and number
		if (ent->node_num > nodenum)
		{
			ent->node_num--;

			if (nodes[nodenum].type == NODE_MOVE)
				Q_strncpyz(ent->node_name, va("node move [%d]", ent->node_num), sizeof(ent->node_name));
			else if (nodes[nodenum].type == NODE_WATER)
				Q_strncpyz(ent->node_name, va("node water [%d]", ent->node_num), sizeof(ent->node_name));
			else if (nodes[nodenum].type == NODE_JUMPPAD)
				Q_strncpyz(ent->node_name, va("node jumppad [%d]", ent->node_num), sizeof(ent->node_name));
			else if (nodes[nodenum].type == NODE_SPAWNPOINT)
				Q_strncpyz(ent->node_name, va("node spawn point [%d]", ent->node_num), sizeof(ent->node_name));
			else if (nodes[nodenum].type == NODE_POI)
				Q_strncpyz(ent->node_name, va("node point of interest [%d]", ent->node_num), sizeof(ent->node_name));
			else
				Q_strncpyz(ent->node_name, va("node action [%d]", ent->node_num), sizeof(ent->node_name));
		}
	}

	// Shift all nodes and links down one
	for (n = 0; n < MAX_PNODES; n++)
	{
		if (n != nodenum)
		{
			// Move links down
			for (l = 0; l < MAXLINKS; l++)
			{
				if (nodes[n].links[l].targetNode != INVALID)
				{
					if (nodes[n].links[l].targetNode > nodenum)
						nodes[n].links[l].targetNode--;
				}
			}
		}
	}

	// Shift all nodes and links down one
	for (n = 0; n + 1 < MAX_PNODES; n++)
	{
		// Copy links
		if (n >= nodenum)
		{
			for (l = 0; l < MAXLINKS; l++)
			{
				nodes[n].links[l].targetNode = nodes[n + 1].links[l].targetNode;
			}
		}

		// Shift node down + copy
		if (n >= nodenum)
		{
			nodes[n].type = nodes[n + 1].type;
			VectorCopy(nodes[n + 1].origin, nodes[n].origin);
			node_ents[n] = node_ents[n + 1];
			--nodes[n].nodenum;
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Add/Update (one way) node link
///////////////////////////////////////////////////////////////////////
qboolean BOTLIB_AddNodeLink(int from, int to, byte type, qboolean do_cost)
{
	int i;
	vec3_t	v;
	float cost;

	// Sanity Checks
	if (type == INVALID)
	{
		if (debug_mode) debug_printf("%s cannot add link from %d to %i, targetNodeType: %d \n", __func__, from, to, type);
		return false;
	}
	if (from == INVALID || to == INVALID || from == to)
	{
		if (debug_mode) debug_printf("%s error: cannot add link from %d to %i\n", __func__, from, to);
		return false;
	}
	if (VectorEmpty(nodes[from].origin))
	{
		if (debug_mode) debug_printf("%s error: zero origin from %d\n", __func__, from);
		return false;
	}
	if (VectorEmpty(nodes[to].origin))
	{
		if (debug_mode) debug_printf("%s error: zero origin to %d\n", __func__, to);
		return false;
	}

	// Check max links reached
	if (nodes[from].num_links + 1 >= MAXLINKS)
	{
		if (debug_mode) Com_Printf("%s warning: MAXLINKS was reached, cannot link from %i to %i\n", __func__, from, to);
		return false;
	}

	// Check if already added
	for (i = 0; i < nodes[from].num_links; i++) // Always cycle all links so we can check for existing links
	{
		if (nodes[from].links[i].targetNode == to) // Do we already have a link to this target?
		{
			if (debug_mode) Com_Printf("%s warning: target link %d already link to %i\n", __func__, nodes[from].links[i].targetNode, to);
			return false; // Failed to add link
		}
	}

	// Add link if we can
	for (i = 0; i < MAXLINKS; i++) // Always cycle all links so we can check for existing links
	{
		if (i + 1 == MAXLINKS) // Max links reached
		{
			if (debug_mode) Com_Printf("%s warning: MAXLINKS was reached, cannot link from %i to %i\n", __func__, from, to);
			return false; // Failed to add link
		}

		if (nodes[from].links[i].targetNode == INVALID) // Found a free spot to add a new link
		{
			nodes[from].num_links++; // Update link counter
			nodes[from].links[i].targetNode = to; // Add the link
			nodes[from].links[i].targetNodeType = type; // The type of the target node

			if (do_cost) // Add the cost of this link?
			{
				VectorSubtract(nodes[from].origin, nodes[to].origin, v); // RiEvEr: William uses a time factor here, whereas I use distance. His is possibly more efficient.
				cost = VectorLength(v); // The cost here is distance based
				nodes[from].links[i].cost = cost; // Add cost
				//if (debug_mode) 
				//	Com_Printf("Link node[%d][%d] type[%d][%d] cost[%f]\n", from, to, nodes[from].type, nodes[to].type, cost);
			}
			//else if (debug_mode) debug_printf("Link %d -> %d\n", from, to);

			return true; // Successfully added link
		}
	}
	return false; // Failed to add link
}

///////////////////////////////////////////////////////////////////////
// Add/Update (one way) node connection/path 
///////////////////////////////////////////////////////////////////////
qboolean ACEND_UpdateNodeReach(int from, int to)
{
	int i;
	//trace_t	trace;

	/*
	// Sanity Checks
	if (from == INVALID || to == INVALID || from == to)
		return qfalse;
	if (nodes[from].origin[0] == 0 && nodes[from].origin[1] == 0 && nodes[from].origin[2] == 0)
	{
		if (debug_mode)
			debug_printf("Zero origin from %d\n", from);
		return qfalse;
	}
	if (nodes[to].origin[0] == 0 && nodes[to].origin[1] == 0 && nodes[to].origin[2] == 0)
	{
		if (debug_mode)
			debug_printf("Zero origin to %d\n", to);
		return qfalse;
	}
	*/

	/////////////////////

	// Add the link
	path_table[from][to] = to;

	/*
	// Checks if the link exists and then may create a new one - RiEvEr
	for (i = 0; i < MAXLINKS; i++)
	{
		if (nodes[from].links[i].targetNode == to)
			break;
		if (nodes[from].links[i].targetNode == INVALID)
		{
			// RiEvEr: William uses a time factor here, whereas I use distance. His is possibly more efficient.
			vec3_t	v;
			float thisCost;

			VectorSubtract(nodes[from].origin, nodes[to].origin, v); // subtract first
			thisCost = VectorLength(v);
			nodes[from].links[i].targetNode = to;
			nodes[from].links[i].cost = thisCost;
			if (debug_mode)
				debug_printf("Link %d -> %d\n", from, to);
			break;
		}
	}
	*/

	// Now for the self-referencing part, linear time for each link added
	for (i = 0; i < numnodes; i++)
	{
		if (path_table[i][from] != INVALID)
		{
			if (i == to)
				path_table[i][to] = INVALID; // make sure we terminate
			else
				path_table[i][to] = path_table[i][from];
		}
	}

	return true;
}
//rekkie -- DEV_1 -- e

//rekkie -- DEV_1 -- s
////////////////////////////////////////////////////////////////////////////////////////////
// Remove all node links from a particular node, including links from and to the node
// This is used when deleting a node, so the links get destroyed properly (bi-directional)
////////////////////////////////////////////////////////////////////////////////////////////
void BOTLIB_RemoveAllNodeLinksFrom(int from)
{
	int i, j, to;

	// Other nodes
	int link_other_counter = 0;
	int link_other_to[MAXLINKS];
	int link_other_from[MAXLINKS];

	// Our node
	int link_self_counter = 0;
	int link_self_to[MAXLINKS];
	int link_self_from[MAXLINKS];

	for (i = 0; i < numnodes; i++)
	{
		for (j = 0; j < MAXLINKS; j++)
		{
			if (nodes[i].links[j].targetNode != INVALID && nodes[i].links[j].targetNode == from)
			{
				//gi.dprintf("%s: OtherNode %d -> %d\n", __func__, nodes[i].nodenum, from);
				link_other_to[link_other_counter] = nodes[i].nodenum;
				link_other_from[link_other_counter] = from;
				link_other_counter++;
			}
		}
	}

	// Gather all the link information before doing anything with it
	for (i = 0; i < MAXLINKS; i++)
	{
		if (nodes[from].links[i].targetNode != INVALID)
		{
			to = nodes[from].links[i].targetNode;

			/*
			// Check other node we're connected to
			for (j = 0; j < MAXLINKS; j++)
			{
				if (nodes[to].links[j].targetNode == from)
				{
					//gi.dprintf("%s: OtherNode %d -> %d\n", __func__, to, from);
					link_other_to[link_other_counter] = to;
					link_other_from[link_other_counter] = from;
					link_other_counter++;
				}
			}
			*/

			//gi.dprintf("%s: OurNode %d -> %d\n", __func__, from, to);
			link_self_from[link_self_counter] = from;
			link_self_to[link_self_counter] = to;
			link_self_counter++;
		}
	}

	// Now use the link information we gathered to remove connections between nodes
	//
	// Remove other node links to ours
	for (i = 0; i < link_other_counter; i++)
		ACEND_RemoveNodeEdge(NULL, link_other_to[i], link_other_from[i]);
	// Remove our node link to other nodes
	for (i = 0; i < link_self_counter; i++)
		ACEND_RemoveNodeEdge(NULL, link_self_from[i], link_self_to[i]);
}

#if 0
// TODO: This function isn't ready for use yet
// Removes a node link from all nodes
void BOTLIB_RemoveNodeLink(edict_t* self, int from, int to)
{
	path_table[from][to] = INVALID; // set to invalid
	
	/*
	// Fill the gap caused by the removal by left shifting from the position of the removed link
	for (int i = 0; i < MAXLINKS; i++)
	{
		// Terminate the last link by making it INVALID
		if (i == MAXLINKS - 1)
		{
			nodes[from].links[i].targetNode = INVALID;
			nodes[from].links[i].cost = INVALID;
		}

		// Move all the other info down to fill the gap (left shift).
		// This *MUST* be done because pathing expects links to be contiguous, 
		// with the first INVALID link found becoming the terminator.
		// IE: First link --> [LINK]-[LINK]-[LINK]-[LINK]-[INVALID] <-- terminator [INVALID]-[INVALID] <-- up to MAXLINKS
		if (nodes[from].links[i].targetNode != INVALID)
		{
			nodes[from].links[i].targetNode = nodes[from].links[i + 1].targetNode;
			nodes[from].links[i].cost = nodes[from].links[i + 1].cost;
		}
	}

	// Make sure this gets updated in our path array
	for (int i = 0; i < numnodes; i++)
		if (path_table[from][i] == to)
			path_table[from][i] = INVALID;
	*/
}
#endif

// Checks a node to see if its possible to path to
// Parameters:
// goal_node: the goal node to reach
// path_randomization: Should path be randomized (if true, this can lead to no path being taken)
// check_and_goto: If TRUE check for a valid path AND go there. If FALSE check for valid path but don't go there.
// build_new_path: If we're building a new path or adding to an existing path
qboolean BOTLIB_CanVisitNode(edict_t* self, int goal_node, qboolean path_randomization, int area, qboolean build_new_path)
{
	//gi.dprintf("All parameters: goal_node[%d] path_randomization[%d] area[%d] build_new_path[%d]\n", goal_node, path_randomization, area, build_new_path);
	// Always update current node
	self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	// Invalid node
	if (goal_node == INVALID || self->bot.current_node == INVALID || nodes[goal_node].inuse == false){
		//Com_Printf("%s %s invalid node set: current_node[%d] goal_node[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node);
		return false;
	}

	if (nodes[goal_node].type == NODE_LADDER) // Never visit ladder nodes
		return false;

	// Already at node
	if (goal_node == self->bot.current_node){
		//Com_Printf("%s %s invalid goal set: current_node[%d] goal_node[%d] inuse[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node, nodes[goal_node].inuse);
		return false;
	}

	// NEW PATHING: limit search to same area
	if (area != INVALID && BOTLIB_DijkstraAreaPath(self, self->bot.current_node, goal_node, path_randomization, area, build_new_path) == false) {
		//gi.dprintf("NEW: %s %s failed to find path to node %d\n", __func__, self->client->pers.netname, goal_node);
		return false;
	}
	// OLD: searches all nodes
	//else if (AntStartSearch(self, self->bot.current_node, goal_node, path_randomization) == false)
	else if (BOTLIB_DijkstraPath(self, self->bot.current_node, goal_node, path_randomization) == false){
		//gi.dprintf("OLD: %s %s failed to find path to node %d\n", __func__, self->client->pers.netname, goal_node);
		return false;
	}



	// ================================================================
	// BOTLIB_SetGoal ()
	// ================================================================
	if (self->bot.goal_node == INVALID)
		self->bot.goal_node = goal_node;

	int nodelist[MAX_NODELIST];
	int nodes_touched;
	nodes_touched = BOTLIB_NodeTouchNodes(self->s.origin, vec3_origin, 32, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
	for (int i = 0; i < nodes_touched; i++) // Cycle through all the nodes we touched
	{
		if (nodelist[i] != INVALID)
		{
			//self->bot.current_node = nodelist[i];
			break;
		}
	}

	if (self->bot.current_node == INVALID || goal_node == self->bot.current_node || goal_node == INVALID || nodes[goal_node].inuse == false)
	{
		//Com_Printf("%s %s invalid goal set: current_node[%d] goal_node[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.goal_node);
		self->bot.goal_node = INVALID;
		self->bot.state = BOT_MOVE_STATE_NAV; // BOT_MOVE_STATE_WANDER
		return false;
	}

	//self->bot.next_node = self->bot.current_node; // make sure we get to the nearest node first
	self->node_timeout = 0;

	self->bot.state = BOT_MOVE_STATE_MOVE;

	//Com_Printf("%s curr[%d] goal[%d] state[%d]\n", __func__, self->bot.current_node, self->bot.goal_node, self->state);
	// ================================================================


	return true;
}

///////////////////////////////////////////////////////////////////////
// Set up the goal
///////////////////////////////////////////////////////////////////////

void BOTLIB_SetGoal(edict_t* self, int goal_node)
{
	// This code is now merged inside BOTLIB_CanVisitNode()

	/*
	self->bot.goal_node = goal_node;

	int nodelist[MAX_NODELIST];
	int nodes_touched;
	nodes_touched = BOTLIB_NodeTouchNodes(self->s.origin, tv(0, 0, 0), 32, self->mins, self->maxs, nodelist, MAX_NODELIST, INVALID);
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
		self->bot.state = BOT_MOVE_STATE_NAV;
		//self->wander_timeout = level.framenum + 0.1 * HZ;
		//ACEAI_PickLongRangeGoal(self);
		//Com_Printf("%s %s invalid goal[%i]\n", __func__, self->client->pers.netname, self->bot.goal_node);
		return;
	}

	self->bot.next_node = self->bot.current_node; // make sure we get to the nearest node first
	self->node_timeout = 0;

	//Com_Printf("%s curr[%d] goal[%d] state[%d]\n", __func__, self->bot.current_node, self->bot.goal_node, self->state); //rekkie

	return;
	*/
}



// Run a trace from [start -> end] looking for nodes (hitbox)
// If [min/max] is empty -> Line trace
// If [min/max] not empty -> Box trace
// If a node was hit, return node number
// If nothing was hit, return INVALID
int BOTLIB_TraceNodeBoxLine(vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs)
{
	vec3_t pos, min, max, absmin, absmax;
	vec3_t dir = { 0 };
	VectorCopy(mins, min);
	VectorCopy(maxs, max);
	if (VectorEmpty(min) && VectorEmpty(max))
	{
		min[0] = -0.25; min[1] = -0.25; min[2] = -0.25;
		max[0] = 0.25; max[1] = 0.25; max[2] = 0.25;
	}
	VectorSubtract(end, start, dir);
	float distance = VectorLength(dir);
	VectorCopy(start, pos);
	float tested_distance = 1;
	float normalized_dist = tested_distance / distance; // Normalized distance
	while (tested_distance + 1 < distance)
	{
		tested_distance += 1; // Move distance forward
		VectorMA(pos, normalized_dist, dir, pos); // Origin -> normalized distance -> direction = new position

		//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
		if (0) // Debug draw
		{
			void(*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time) = players[0]->client->pers.draw->DrawBox;
			DrawBox(500 + tested_distance, pos, MakeColor(255, 255, 0, 255), mins, maxs, 5000); // Draw node box
			players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
			//Com_Printf("%s [%d to %d] dist[%f of %f] pos[%f %f %f]\n", __func__, from, to, distance, tested_distance, pos[0], pos[1], pos[2]);
		}
#endif
		//rekkie -- debug drawing -- e

		// Update absolute box min/max in the world
		VectorAdd(pos, min, absmin);
		VectorAdd(pos, max, absmax);

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].inuse == false) continue; // Ignore nodes not in use

			if (BOTLIB_BoxIntersection(nodes[i].absmin, nodes[i].absmax, absmin, absmax)) // Do boxes intersect?
				return i;
		}
	}

	return INVALID; // No intersection
}
/*
// Generate vis data for each node - test node-to-node visibility
void BOTLIB_GenerateNodeVis(edict_t* self)
{
	for (int i = 0; i < numnodes; i++)
		nodes[i].num_vis_nodes = 0;			// zero

	for (int i = 0; i < numnodes; i++)
	{
		for (int j = 0; j < numnodes; j++)
		{
			if (i == j) continue; // skip self

			// Check if the node was already added
			for (int k = 0; k < nodes[i].num_vis_nodes; k++)
			{
				if (nodes[i].vis_nodes[k] == nodes[j].nodenum)
					continue;
			}

			if (nodes[i].num_vis_nodes + 1 >= 1024)
				continue;
			if (nodes[j].num_vis_nodes + 1 >= 1024)
				continue;

			trace_t tr = gi.trace(nodes[i].origin, NULL, NULL, nodes[j].origin, NULL, MASK_SHOT);
			
			if (tr.fraction == 1.0) // Nodes can see each other
			{
				// Store the remote node in the local node
				nodes[i].vis_nodes[nodes[i].num_vis_nodes] = nodes[j].nodenum; // Store the node we can see
				nodes[i].num_vis_nodes++; // Increase the vis nodes in cache

				// Store the local node in the remote node
				nodes[j].vis_nodes[nodes[j].num_vis_nodes] = nodes[i].nodenum; // Store the node we can see
				nodes[j].num_vis_nodes++; // Increase the vis nodes in cache
			}
		}
	}
}
*/

// Trace from node to node, checking to see if we hit other nodes along the path
// Return the node number that was hit, or INVALID if none was hit
int BOTLIB_TraceBoxNode(int from, int to)
{
	if (from == INVALID || to == INVALID) return INVALID;

	vec3_t pos, absmin, absmax;
	vec3_t dir = { 0 };
	VectorSubtract(nodes[to].origin, nodes[from].origin, dir);
	float distance = VectorLength(dir);
	VectorCopy(nodes[from].origin, pos);

	float tested_distance = 1;
	float normalized_dist = tested_distance / distance; // Normalized distance
	while (tested_distance + 1 < distance)
	{
		tested_distance += 1; // Move distance forward
		VectorMA(pos, normalized_dist, dir, pos); // Origin -> normalized distance -> direction = new position

		//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
		if (0) // Debug draw
		{
			void(*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time) = players[0]->client->pers.draw->DrawBox;
			DrawBox(500 + tested_distance, pos, MakeColor(255, 255, 0, 255), nodes[from].mins, nodes[from].maxs, 5000); // Draw node box
			players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
			//Com_Printf("%s [%d to %d] dist[%f of %f] pos[%f %f %f]\n", __func__, from, to, distance, tested_distance, pos[0], pos[1], pos[2]);
		}
#endif
		//rekkie -- debug drawing -- e

		// Update absolute box min/max in the world
		VectorAdd(pos, nodes[from].mins, absmin);
		VectorAdd(pos, nodes[from].maxs, absmax);

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].inuse == false) continue; // Ignore nodes not in use
			if (nodes[i].nodenum == from || nodes[i].nodenum == to) continue; // Ignore source and destination nodes
				
			if (BOTLIB_BoxIntersection(nodes[i].absmin, nodes[i].absmax, absmin, absmax)) // Do boxes intersect?
				return i;
		}
	}

	return INVALID; // No intersection
}

// Test to see if a node is touching any other nodes
// Returns INVALID not touching any nodes
// Returns nodelist if one or more nodes are touching
// numnodes is the total nodes touched
// maxnodes is the maximum nodes that can be stored in nodelist
// ignore_node is optional: ignores testing this node when testing. If ignore_node is INVALID, all nodes are tested
int BOTLIB_NodeTouchNodes(vec_t *origin, vec3_t normal, float size, vec3_t mins, vec3_t maxs, int *nodelist, const int maxnodes, int ignore_node)
{
	int nodelist_count = 0;

	// Adjust the mins/maxs box size based on size
	vec3_t bmins = { mins[0] + -(size), mins[1] + -(size), mins[2] + -(size) };
	vec3_t bmaxs = { maxs[0] + size, maxs[1] + size, maxs[2] + size };

	// Update absolute box min/max in the world
	vec3_t absmin, absmax;
	VectorAdd(origin, bmins, absmin);
	VectorAdd(origin, bmaxs, absmax);

	for (int n = 0; n < numnodes; n++)
	{
		if (nodes[n].inuse == false) continue; // Ignore nodes not in use
		if (nodes[n].nodenum == INVALID) continue; // Ignore invalid nodes
		//if (VectorDistance(nodes[n].origin, origin) > 32) continue; // Skip checking distant nodes
		if (ignore_node != INVALID && nodes[n].nodenum == ignore_node) continue; // Ignore ignored node

		// Line test
		//trace_t tr = gi.trace(origin, NULL, NULL, nodes[n].origin, NULL, MASK_PLAYERSOLID | MASK_OPAQUE);
		//if (tr.fraction == 1.0)
		{
			if (BOTLIB_BoxIntersection(absmin, absmax, nodes[n].absmin, nodes[n].absmax)) // Do boxes intersect?
			{
				//if (normal[2] == 1 && origin[2] != nodes[n].origin[2]) // Check flat ground + height difference
				//	continue; // Ignore nodes that are on a different plane (such as stairs or boxes)

				if (nodelist_count + 1 < maxnodes) // Check we have room to store
					nodelist[nodelist_count++] = n;  //nodes[n].nodenum; // Add to list
				else
					break;
			}
		}
	}

	return nodelist_count; // Return number of nodes touched
}

// Test location to see if taken by an existing node based on distance, return the node (if any) we're too close to
// Returns 'INVALID' if no node(s) are found
// Returns 'nodenum' if node(s) are found
int BOTLIB_TestForNodeDist(vec_t *origin, float distance, vec3_t mins, vec3_t maxs)
{
	trace_t tr;
	//vec3_t v;
	//float dist;

	// Check if we fit
	tr = gi.trace(origin, tv(-2, -2, 0), tv(2, 2, 0), origin, NULL, MASK_PLAYERSOLID);
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		return -2; // Inside solid
	}

	// Test to see if node hits the ceiling
	//tr = gi.trace(tv(origin[0], origin[1], origin[2]), tv(-16, -16, -32), tv(16, 16, 25), tv(origin[0], origin[1], origin[2] + 25), NULL, MASK_PLAYERSOLID);
	tr = gi.trace(tv(origin[0], origin[1], origin[2]), NULL, NULL, tv(origin[0], origin[1], origin[2] + 25), NULL, MASK_PLAYERSOLID);
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		return -2; // Hit our head on something immediately above us
	}


	vec3_t bmins = { mins[0] + -(distance), mins[1] + -(distance), mins[2] + -(distance) };
	vec3_t bmaxs = { maxs[0] + distance, maxs[1] + distance, maxs[2] + distance };

	//for (int n = 0; n < NODE_MAX; n++)
	for (int n = 0; n < numnodes; n++)
	{
		if (nodes[n].inuse == false) continue; // Ignore nodes not in use

		if (nodes[n].nodenum != INVALID)
		{
			// Line test
			tr = gi.trace(origin, NULL, NULL, nodes[n].origin, NULL, MASK_PLAYERSOLID);
			if (tr.fraction == 1.0) // Only distance test origins that can see each other
			{
				// New method of box intersection search
				if (BOTLIB_BoxIntersection(bmins, bmaxs, nodes[n].mins, nodes[n].maxs))
					return nodes[n].nodenum; // Return the first node found too close

				/*
				// Old method of radius search
				VectorSubtract(nodes[n].origin, origin, v); // other node -> our node
				dist = VectorLength(v);
				if (dist < distance)
				{
					//if (dist == 0) // If we find a node at the exact same coordinates
					//	Com_Printf("%s found node %d at the same origin[%f %f %f]\n", __func__, n, origin[0], origin[1], origin[2]);

					//gi.dprintf("-=-=-=-=-=------------[%i] dist[%f]\n", n, dist);
					return nodes[n].nodenum; // Return the first node found too close
				}
				*/
			}
		}
	}

	return INVALID; // Found no nodes within range, safe to place a node here :)
}

// Test all nodes to see they're too close to our node
// Calculation is based on matching the exact X and Y coords, then comparing the height difference
// Returns the first offending node that is too close, otherwise INVALID if no node is found
int BOTLIB_UTIL_NearbyNodeAtHeightDist(vec_t* origin, float distance)
{
	float dist;
	for (int n = 0; n < numnodes; n++)
	{
		if (nodes[n].inuse == false) continue; // Ignore nodes not in use

		if (nodes[n].nodenum != INVALID)
		{
			if (origin[0] == nodes[n].origin[0]) // Match X axis
			{
				if (origin[1] == nodes[n].origin[1]) // Match Y axis
				{
					dist = abs(nodes[n].origin[2] - origin[2]); // Get Z height distance

					if (dist <= distance) // Z axis within distance
						return n; // Offending node that is too close
				}
			}
		}
	}

	return INVALID; // Found no nodes that are too close
}

float HalfPlaneSign(vec2_t p1, vec2_t p2, vec2_t p3)
{
	return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1]);
}

// Check if point is inside triangle. 
// PT = (x,y,z) point location. V1,V2,V3 = 3 points of a triangle (x,y,z)
qboolean PointInTriangle(vec3_t pt, vec3_t v1, vec3_t v2, vec3_t v3)
{
	float d1, d2, d3;
	qboolean has_neg, has_pos;

	d1 = HalfPlaneSign(pt, v1, v2);
	d2 = HalfPlaneSign(pt, v2, v3);
	d3 = HalfPlaneSign(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

// Returns true if the point is inside a convex polygon
qboolean PointInPolygon(vec2_t pt, vec2_t* v, int num_verts)
{
	int i, j, c = 0;
	for (i = 0, j = num_verts - 1; i < num_verts; j = i++)
	{
		if (((v[i][1] > pt[1]) != (v[j][1] > pt[1])) &&
			(pt[0] < (v[j][0] - v[i][0]) * (pt[1] - v[i][1]) / (v[j][1] - v[i][1]) + v[i][0]))
			c = !c;
	}
	return c;
}

// A function that makes a square polygon from a set of vertices
void BOTLIB_UTIL_MakePolySquare(int face, vec3_t out)
{
	int e;
	vec3_t edge[128][2]; // Max edges per face
	for (e = 0; e < nmesh.face[face].num_edges; e++)
	{
		// Copy all the edges into a 2D array
		VectorCopy(nmesh.face[face].edge[e].v[0], edge[e][0]); // v1
		VectorCopy(nmesh.face[face].edge[e].v[1], edge[e][1]); // v2
	}

	/*
	face 101  edges 6
	--------------
	v1[256.000000 -176.000000 -800.000000]  v2[480.000000 -176.000000 -800.000000]
	v1[256.000000 -112.000000 -800.000000]  v2[256.000000 -176.000000 -800.000000]
	v1[256.000000 -33.000000 -800.000000]  v2[256.000000 -112.000000 -800.000000]
	v1[256.000000 0.000000 -800.000000]  v2[256.000000 -33.000000 -800.000000]
	v1[480.000000 0.000000 -800.000000]  v2[256.000000 0.000000 -800.000000]
	v1[480.000000 0.000000 -800.000000]  v2[480.000000 -176.000000 -800.000000]
	--------------
	*/

	// Find each edge that goes in the same direction
	// Try to find the total length of that direction
	vec3_t v1, v2;
	//float highest = 0, lowest = 0;
	float v1_x; // V1 X
	float v1_y; // V1 Y
	float v2_x; // V2 X
	float v2_y; // V2 Y

	float highest_x = 0;
	float lowest_x = 0;
	float highest_y = 0;
	float lowest_y = 0;

	for (e = 0; e < nmesh.face[face].num_edges; e++)
	{
		VectorCopy(edge[e][0], v1); // First vertex
		VectorCopy(edge[e][1], v2); // Second vertex

		for (int e2 = 0; e2 < nmesh.face[face].num_edges; e2++)
		{
			if (e == e2) continue; // Skip self

			// Check the X axis
			v1_x = v1[0]; v2_x = v2[0]; // X
			v1_y = v1[1]; v2_y = v2[1]; // Y

			if (v1_x == v2_x) // Check if v1 and v2 are on the same X axis
			{
				// Find highest on the Y axis
				if (v1_y > highest_y)
					highest_y = v1_y;
				else if (v2_y > highest_y)
					highest_y = v2_y;

				// Find lowest on the Y axis
				if (v1_y < lowest_y)
					lowest_y = v1_y;
				else if (v2_y < lowest_y)
					lowest_y = v2_y;
			}
			if (v1_y == v2_y) // Check if v1 and v2 are on the same Y axis
			{
				// Find highest on the X axis
				if (v1_x > highest_x)
					highest_x = v1_x;
				else if (v2_x > highest_x)
					highest_x = v2_x;

				// Find lowest on the X axis
				if (v1_x < lowest_x)
					lowest_x = v1_x;
				else if (v2_x < lowest_x)
					lowest_x = v2_x;
			}
		}
	}
}


// A function that finds the center of a 3D convex polygon
void BOTLIB_UTIL_PolygonCenter(vec3_t* v, int num_verts, vec3_t center)
{
	vec3_t sum = { 0,0,0 };
	for (int i = 0; i < num_verts; i++)
	{
		VectorAdd(sum, v[i], sum);
	}
	VectorScale(sum, 1.0 / num_verts, center);
}

// Return the BSP face and triangle based on origin
void GetBSPFaceTriangle(vec3_t origin, qboolean* found, int* floor, int* triangle)
{
	vec3_t org = { 0,0,0 };
	vec3_t dist = { 0,0,0 };
	int n;
	float distance;

	for (int f = 0; f < nmesh.total_faces; f++) // Current face
	{
		if (nmesh.face[f].type == FACETYPE_WALL) continue; // Skip walls

		for (int t = 0; t < nmesh.face[f].num_tris; t++) // Current triangle
		{
			if (PointInTriangle(origin, nmesh.face[f].tris[t].v[0], nmesh.face[f].tris[t].v[1], nmesh.face[f].tris[t].v[2]))
			{
				// Ignore tris with invalid nodes
				n = nmesh.face[f].tris[t].node;
				if (n <= 0)
					continue;

				// Ensure the PointInTriangle is roughly within the same z height as origin
				VectorCopy(origin, org); // We only want the height
				org[0] = 0; // Remove X component
				org[1] = 0; // Remove Y component
				VectorSubtract(nodes[n].origin, origin, dist);
				distance = VectorLength(dist);
				if (distance > 64) // Height
					continue;

				//Com_Printf("%s -> f[%i] t[%i] node %i at %f %f %f\n", __func__, f, t, n, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2]);

				*found = true;
				*floor = f;
				*triangle = t;
				return;
			}
		}
	}
}

// Return the BSP face and number of edges that is closest to origin
void BOTLIB_UTIL_NEAREST_BSP_FACE(vec3_t origin, int* floor, int *edges)
{
	vec3_t org = { 0,0,0 };
	vec3_t dist = { 0,0,0 };
	int e; // edge
	float distance;
	float closest = 999999;

	for (int f = 0; f < nmesh.total_faces; f++) // Current face
	{
		if (nmesh.face[f].type == FACETYPE_WALL) continue; // Skip walls

		vec3_t edge[128]; // Max edges per face
		for (e = 0; e < nmesh.face[f].num_edges; e++)
		{
			// Copy all the edges into a 2D array
			VectorCopy(nmesh.face[f].edge[e].v[0], edge[e]);
		}

		BOTLIB_UTIL_PolygonCenter(edge, nmesh.face[f].num_edges, org); // Get the center of the polygon

		// Ensure the polygon is roughly within the same z height as origin
		//VectorCopy(origin, org); // We only want the height
		//org[0] = 0; // Remove X component
		//org[1] = 0; // Remove Y component
		VectorSubtract(org, origin, dist);
		distance = VectorLength(dist);
		//if (distance > 64) // Height
		//	continue;

		if (distance < closest)
		{
			closest = distance;
			*floor = f;
			*edges = e;
		}

	}
}


// Returns the distance from origin to target
float ACEND_Distance(vec3_t origin, vec3_t target)
{
	vec3_t dist;
	VectorSubtract(target, origin, dist);
	return VectorLength(dist);
}

// Using three points of a triangle with origin being the origin calculate the angle between target_a and target_b
float ACEND_AngleBetween(vec3_t origin, vec3_t target_a, vec3_t target_b)
{
	vec3_t a, b;
	VectorSubtract(target_a, origin, a); // Get length
	VectorSubtract(target_b, origin, b); // Get length
	return RAD2DEG(acos(DotProduct(a, b) / (VectorLength(a) * VectorLength(b))));
}


void LaunchP(edict_t* ent, vec3_t origin, vec3_t target)
{
	vec3_t dist;
	vec3_t velocity;
	float gravity = ent->gravity * sv_gravity->value;
	VectorSubtract(target, ent->s.origin, dist);
	float distance = VectorLength(dist);

	// Calculate the jump height to get to the target
	float jump_height = sqrt(2 * (gravity * distance));

	// Calculate the time it will take to get to the target
	float time = distance / (ent->velocity[2] + jump_height / 2.0);

	// Calculate the velocity at the end of the jump
	VectorScale(dist, 1.0 / time, velocity);

	velocity[2] = jump_height / 2.0;

	// If the target is above the player, increase the velocity to get to the target
	float z_height = 0;
	if (target[2] > ent->s.origin[2])
	{
		z_height = (target[2] - ent->s.origin[2]);
		z_height += sqrtf(distance) + NODE_Z_HEIGHT;
		velocity[2] += z_height;
	}
	else if (abs(target[2] - ent->s.origin[2]) <= 4)
	{
		// Do nothing here
	}
	else
	{
		z_height = (ent->s.origin[2] - target[2]) - NODE_Z_HEIGHT;
		velocity[2] -= z_height;
	}

	// Set the velocity of the player to the calculated velocity
	VectorCopy(velocity, ent->velocity);
	VectorCopy(ent->velocity, ent->client->oldvelocity);
	VectorCopy(ent->velocity, ent->avelocity);

	//float speed = VectorLength(velocity);
	//gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);
}

qboolean LaunchPlayer(edict_t* ent, vec3_t target)
{

	LaunchP(ent, ent->s.origin, target);
	return false;

	// Calculate the distance to the target
	vec3_t dist;
	vec3_t velocity;
	float gravity = ent->gravity * sv_gravity->value;
	//float gravity = ent->gravity * sv_gravity->value * FRAMETIME;


	vec3_t targ;
	VectorCopy(target, targ);
	//targ[2] += 64;
	VectorSubtract(targ, ent->s.origin, dist);
	//dist[2] = 0;
	//dist[2] += 64;
	float distance = VectorLength(dist);
	VectorCopy(ent->velocity, velocity);
	//velocity[2] += 64;
	//VectorNormalize(velocity);
	//VectorNormalize(dist);
	//if (DotProduct(velocity, dist) > 0.8)
	//Com_Printf("%s %f\n", __func__, DotProduct(velocity, dist));
	//distance *= 2.0; // Increasing this increases the jump height


	//VectorSubtract(target, ent->s.origin, dist);
	//float distance = VectorLength(dist);
	//distance *= 1.0; // Increasing this increases the jump height

	// Calculate the jump height to get to the target
	float jump_height = sqrt(2 * (gravity * distance));
	// Calculate the time it will take to get to the target
	float time = distance / (ent->velocity[2] + jump_height / 2.0);
	// Calculate the velocity at the end of the jump
	VectorScale(dist, 1.0 / time, velocity);
	velocity[2] = jump_height / 2.0;

	// If the target is above the player, increase the velocity to get to the target
	float z_height;
	if (target[2] > ent->s.origin[2])
	{
		z_height = (target[2] - ent->s.origin[2]) + NODE_Z_HEIGHT;
		velocity[2] += z_height;
	}
	else
	{
		z_height = (ent->s.origin[2] - target[2]) - NODE_Z_HEIGHT;
		velocity[2] -= z_height;
	}


	/*
	// Get the z height distance between origin and target
	vec3_t zdiff, oz, tz;
	// Copy origin to oz
	VectorCopy(ent->s.origin, oz);
	// Copy target to tz
	VectorCopy(target, tz);
	// Set X and Y on oz and tz to zero
	oz[0] = 0; oz[1] = 0;
	tz[0] = 0; tz[1] = 0;
	// Get the z height distance between oz and tz
	VectorSubtract(oz, tz, zdiff);
	// Add the difference to jump velocity
	float z_height = 0;
	if (ent->s.origin[2] < target[2])
	{
		//z_height = VectorLength(zdiff);
		velocity[2] += z_height + 56;
	}
	else
	{
		velocity[2] -= VectorLength(zdiff);
	}
	*/

	// Calculate speed from velocity
	float speed = VectorLength(velocity);
	gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);
	// Limit max speed
	//if (speed < 750 && z_height < 150 && jump_height <= 965)
	{
		// Set the velocity of the player
		VectorCopy(velocity, ent->velocity);
		// Don't take falling damage immediately from this
		if (ent->client)
			VectorCopy(ent->velocity, ent->client->oldvelocity);

		return true;
	}
	return false;
}

int DC_Reachability(int from, int to, vec3_t origin, vec3_t target, vec3_t normal)
{
	trace_t tr;

	//static const float max_dist = 128; // Absolute max distance -- 470
	const float max_dist = 470; // Absolute max distance -- 470
	//float curr_dist = 224;
	//qboolean is_los = true; // Line of Sight
	qboolean is_gap = false; // If there's a gap between origin and target

	// Sanity checks
	if (from == to || from == INVALID || to == INVALID)
		return INVALID;
	if (origin[0] == 0 && origin[1] == 0 && origin[2] == 0)
		return INVALID;
	if (target[0] == 0 && target[1] == 0 && target[2] == 0)
		return INVALID;


	//if (nodes[from].type == NODE_WATER || nodes[to].type == NODE_WATER)
	//	Com_Printf("%s NODE_WATER\n", __func__);

	/*
	// Ignore some water nodes (except ladders)
	// Don't go into the water
	if (nodes[from].type != NODE_WATER && nodes[to].type == NODE_WATER)
		return INVALID;
	// Check node is in water
	if (nodes[from].type == NODE_WATER && (gi.pointcontents(nodes[from].origin) & CONTENTS_WATER) == 1)
		return INVALID;
	// Check node is in water
	if (nodes[to].type == NODE_WATER && (gi.pointcontents(nodes[to].origin) & CONTENTS_WATER) == 1)
		return INVALID;
	//if (nodes[from].type == NODE_WATER)
	//	return INVALID;
	//if (nodes[from].type == NODE_WATER || nodes[to].type == NODE_WATER)
	//	return INVALID;
	*/

	//vec3_t vec_from, vec_to;

	// Calculate the distance to the target
	vec3_t dist;
	vec3_t velocity;

	VectorSubtract(target, origin, dist);
	float xyz_distance = VectorLength(dist); // XYZ distance

	//VectorSubtract(target, origin, dist);
	dist[2] = 0;
	float xy_distance = VectorLength(dist); // XY Distance

	// Max distance
	if (xyz_distance > max_dist) // Ignore very far nodes
		return INVALID;

	// Min distance
	//if (xyz_distance < NODE_Z_HEIGHT) // Ignore very close nodes
	//	return INVALID;

	/*
	//	Player when standing
	vec3_t  player_standing_mins = { -16, -16, -24 }; // With legs
	vec3_t  player_partial_standing_mins = { -16, -16, -0 }; //	Without legs
	vec3_t  player_standing_maxs = { 16, 16, 32 };
	//	Player when crouching/ducking
	vec3_t  player_crouching_mins = { -16, -16, -24 };
	vec3_t  player_crouching_maxs = { 16, 16, 4 };
	//	Player when standing smaller mins
	vec3_t  player_standing_smaller_mins = { -14, -14, -24 };
	vec3_t  player_standing_smaller_maxs = { 14, 14, 32 };
	//	Player width only, no height component
	vec3_t  player_no_height_mins = { -16, -16, -0 };
	vec3_t  player_no_height_maxs = { 16, 16, 0 };
	// Small box test
	vec3_t  small_mins = { -8, -8, -2 };
	vec3_t  small_maxs = { 8, 8, 2 };
	// Tiny box test
	vec3_t  tiny_mins = { -1, -1, -1 };
	vec3_t  tiny_maxs = { 1, 1, 1 };
	*/


	// Do a line trace
	tr = gi.trace(origin, NULL, NULL, target, g_edicts, MASK_DEADSOLID);
	// See if we hit any entities we wish to avoid
	if (tr.ent && tr.ent != g_edicts)
	{
		//Com_Printf("%s %s found - \n", __func__, tr.ent->classname);

		// Avoid these
		if (strcmp(tr.ent->classname, "trigger_hurt") == 0)
		{
			//Com_Printf("%s %s found - invalid reachability\n", __func__, tr.ent->classname);
			return INVALID;
		}

		// Avoid rotating geometry - for now
		if (strcmp(tr.ent->classname, "func_rotating") == 0)
		{
			//Com_Printf("%s %s found - invalid reachability\n", __func__, tr.ent->classname);
			return INVALID;
		}
	}
	// See if we hit any walls or inside a solid
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		return INVALID;
	}






	// Test for gaps/holes between the current and next node
	// Each tested is conducted x units apart
	{
		vec3_t dir, end;
		float tested_distance = 0;

		// Get direction
		VectorSubtract(target, origin, dir); // (end - start) = dir

		// Get XYZ distance
		xyz_distance = VectorLength(dir); // VectorLength of dir (cannot be normalized)

		// Normalize direction vector (must be done after VectorLength otherwise we don't get the correct distance)
		VectorNormalize(dir);

		// Safety check
		if (xyz_distance >= 4096) // Max distance
		{
			Com_Printf("%s WARNING: gap test exceeded > 4096 units from node %d to %d\n", __func__, from, to);
		}
		else
		{
			while (tested_distance + NODE_SIZE < xyz_distance)
			{
				tested_distance += NODE_SIZE; // Move next test forward
				VectorMA(origin, tested_distance, dir, end); // origin -> distance -> direction = end point

				// Check position to see if we hit any solids along the way
				tr = gi.trace(end, tv(-12,-12,-8), tv(12, 12, 8), end, g_edicts, MASK_PLAYERSOLID | MASK_OPAQUE);
				if (tr.startsolid) is_gap = true;

				end[2] -= NODE_Z_HEIGHT; // Move end point to ground
				tr = gi.trace(end, NULL, NULL, tv(end[0], end[1], -4096), g_edicts, MASK_PLAYERSOLID | MASK_OPAQUE); // Trace down until we hit the ground
				if (tr.startsolid) continue;
					//is_gap = true;

				//if (tr.plane.normal[2] < 0.99) // If the ground we hit below is not flat
				{
					//is_gap = true;
					//break;
				}

				// 
				if (end[2] - tr.endpos[2] > (STEPSIZE + 1))
				{
					//if (from == 1740 && to == 1403)
					//	Com_Printf("%s dist %f z-diff %f -- FOUND GAP\n", __func__, tested_distance, end[2] - tr.endpos[2]);
					is_gap = true;
					break;
				}
			}
		}
	}




	// Use smaller node min/max for ladders
	//if (nodes[to].type == NODE_LADDER_DOWN || nodes[to].type == NODE_LADDER_UP)
	//	//tr = gi.trace(origin, tv(-16, -16, -0), tv(16, 16, 0), target, g_edicts, MASK_DEADSOLID); // Player width only, no height component
	//	tr = gi.trace(origin, NULL, NULL, target, g_edicts, MASK_DEADSOLID); // Line trace
	//else
	//	tr = gi.trace(origin, tv(-16, -16, -0), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID); // Test player partial height
	if (nodes[from].type != NODE_LADDER_DOWN && nodes[to].type != NODE_LADDER_DOWN && nodes[from].type != NODE_LADDER_UP && nodes[to].type != NODE_LADDER_UP)
	{
		tr = gi.trace(origin, tv(-16, -16, -0), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID); // Test player partial height
		//tr = gi.trace(origin, tv(-16, -16, -0), tv(16, 16, 0), target, g_edicts, MASK_DEADSOLID); // Player width only, no height component
		// Try to trace from node->node LoS
		// Failing that a second trace is performed with both nodes moved up slightly
		if (tr.fraction < 1)
		{
			// Ignore ladders
			//if (nodes[from].type != NODE_LADDER_DOWN && nodes[to].type != NODE_LADDER_DOWN && nodes[from].type != NODE_LADDER_UP && nodes[to].type != NODE_LADDER_UP)
			{
				// See if target is below and if we can drop down
				if (origin[2] > target[2] && xy_distance < 96)
				{
					//tr = gi.trace(origin, tv(-16, -16, 48), tv(16, 16, 56), target, g_edicts, MASK_DEADSOLID);
					//tr = gi.trace(origin, tv(-16, -16, 25), tv(16, 16, 26), target, g_edicts, MASK_DEADSOLID);
					tr = gi.trace(origin, tv(-8, -8, -0), tv(8, 8, 0), target, g_edicts, MASK_DEADSOLID); // LoS + width to the node below
					if (tr.fraction == 1)
					{
						float lower = (origin[2] - target[2]); // Distance below

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
							return NODE_UNSAFE_DROP; // Can drop while crouching but will take some leg damage
						}
					}
				}

				// Check for crouching
				if (nodes[to].type == NODE_MOVE && is_gap == false && xy_distance < 256)
				{
					tr = gi.trace(tv(origin[0], origin[1], origin[2] - 16), tv(-16, -16, -32), tv(16, 16, 0), tv(target[0], target[1], target[2] - 16), g_edicts, MASK_DEADSOLID); // Test crouch height
					if (tr.fraction == 1)
					{
						return NODE_CROUCH;
					}
				}
			}

			//return INVALID;
		}
	}


	// Invalidate bad ladder links
	// ---------------------------
	// Ladder from DOWN -> UP must be on same X/Y position
	if (nodes[from].type == NODE_LADDER_DOWN && nodes[to].type == NODE_LADDER_UP && (origin[0] != target[0] || origin[1] != target[1]))
		return INVALID;
	// Ladder from UP -> DOWN must be on same X/Y position
	if (nodes[from].type == NODE_LADDER_UP && nodes[to].type == NODE_LADDER_DOWN && (origin[0] != target[0] || origin[1] != target[1]))
		return INVALID;
	// Ladder from DOWN -> DOWN
	if (nodes[from].type == NODE_LADDER_DOWN && nodes[to].type == NODE_LADDER_DOWN)
		return INVALID;
	// Ladder from UP -> UP
	if (nodes[from].type == NODE_LADDER_UP && nodes[to].type == NODE_LADDER_UP)
		return INVALID;

	// Top ladder node -> non-ladder node, test if within z height range +/-
	if (nodes[from].type == NODE_LADDER_UP && nodes[to].type != NODE_LADDER_DOWN)
	{
		//if (abs(origin[2] - target[2]) > STEPSIZE || xyz_distance > 128 || xyz_distance < 48)
		if (fabsf(origin[2] < target[2]) > 8 || xyz_distance > 128 || xyz_distance < 48)
			return INVALID;
		else
			return NODE_MOVE;
		//else
		//	return NODE_JUMPPAD; // Jump to next node
	}
	// Bottom ladder node -> non-ladder node, test if within z height range +/-
	if (nodes[from].type == NODE_LADDER_DOWN && nodes[to].type != NODE_LADDER_UP)
	{
		if (fabsf(origin[2] - target[2]) > STEPSIZE || xyz_distance > 96)
			return INVALID;
	}

	// Non-ladder node -> top ladder node - test if within z height range +/-
	if (nodes[from].type != NODE_LADDER_DOWN && nodes[to].type == NODE_LADDER_UP)
	{
		if (fabsf(origin[2] - target[2]) > NODE_Z_HEIGHT || xyz_distance > 96)
			return INVALID;
	}
	// Non-ladder node -> bottom ladder node - test if within z height range +/-
	if (nodes[from].type != NODE_LADDER_UP && nodes[to].type == NODE_LADDER_DOWN)
	{
		//if (abs(origin[2] - target[2]) > NODE_Z_HEIGHT || xyz_distance > 96)
		//	return INVALID;
		if (fabsf(origin[2] - target[2]) <= NODE_MAX_JUMP_HEIGHT && xyz_distance < 96)
			return NODE_JUMPPAD;
		else
			return INVALID;
	}
	// ---------------------------


	/*
	//if (nodes[from].type == NODE_LADDER || nodes[to].type == NODE_LADDER)
	//	return INVALID;
// (surf->plane->normal[0] >= -MAX_STEEPNESS && surf->plane->normal[0] <= MAX_STEEPNESS && surf->plane->normal[1] >= -MAX_STEEPNESS && surf->plane->normal[1] <= MAX_STEEPNESS && surf->plane->normal[2] >= MAX_STEEPNESS)
	if (nodes[from].type == NODE_LADDER && nodes[to].type == NODE_LADDER)
	{
		if (abs(origin[2] - target[2]) > 4)
		{
			if (abs(origin[0] - target[0]) <= 4 && abs(origin[1] - target[1]) <= 4)
				return NODE_LADDER;
			else if (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS)
				return INVALID;
			else if (normal[0] != 1.0 && normal[1] != 1.0 && origin[0] == target[0] && abs(origin[1] - target[1]) <= 96)
				return NODE_LADDER;
			else if (normal[0] != 1.0 && normal[1] != 1.0 && origin[1] == target[1] && abs(origin[0] - target[0]) <= 96)
				return NODE_LADDER;
		}
		return INVALID;
	}
	else if (nodes[from].type == NODE_LADDER && nodes[to].type != NODE_LADDER && xyz_distance > 64)
		return INVALID;
	else if (nodes[from].type != NODE_LADDER && nodes[to].type == NODE_LADDER && xyz_distance > 64)
		return INVALID;
	*/







	//return INVALID;







	// Water nodes
	if (nodes[from].type == NODE_WATER)
	{
		if (target[2] > origin[2] && xy_distance < 64 && nodes[to].type == NODE_LADDER_UP) // Allow water -> ladder
			return NODE_LADDER_UP;
		//if (nodes[to].type == NODE_LADDER_UP) // Allow water -> ladder
		//	return NODE_LADDER_DOWN;
		//else if (nodes[to].type == NODE_MOVE) // Allow water -> move
		//	return NODE_MOVE;
		else if (nodes[to].type == NODE_WATER) // Allow water -> water
			return NODE_MOVE;
		else
			return INVALID; // Deny water to anything else
	}



	// Steps
	// -----
	// Deal with steps
	if (nodes[from].type == NODE_STEP || nodes[to].type == NODE_STEP)
	{
		// Getting off step to a landing node
		if (nodes[from].type == NODE_STEP && nodes[to].type == NODE_MOVE && fabsf(origin[2] - target[2]) <= NODE_Z_HEIGHT_PLUS_STEPSIZE && xyz_distance <= 80)
		{
			trace_t tr_step = gi.trace(origin, tv(-15,-15,-18), tv(15,15,32), target, g_edicts, MASK_PLAYERSOLID);
			if (tr_step.fraction == 1)
			{
				return NODE_MOVE;
			}
		}
		// Getting on step
		else if (nodes[from].type == NODE_MOVE && nodes[to].type == NODE_STEP && fabsf(origin[2] - target[2]) <= STEPSIZE && xyz_distance <= 80)
		{
			trace_t tr_step = gi.trace(origin, tv(-15,-15,-18), tv(15,15,32), target, g_edicts, MASK_PLAYERSOLID);
			if (tr_step.fraction == 1)
			{
				return NODE_MOVE;
			}
		}
		// Step to Step
		else if (nodes[from].type == NODE_STEP && nodes[to].type == NODE_STEP && fabsf(origin[2] - target[2]) <= STEPSIZE && xyz_distance < 40)
		{

			//is_gap
			trace_t tr_step = gi.trace(origin, NULL, NULL, target, g_edicts, MASK_PLAYERSOLID);
			if (tr_step.fraction == 1)
			{
				return NODE_STEP;
			}
		}
		else
			return INVALID;
	}




	// Calculate max jump height
	// If on flat ground the max is 60 (NODE_MAX_JUMP_HEIGHT)
	// If on slope up, the max is 150 depending on the angle of the slope, higher angle = higher max jump height
	float z_height_max = NODE_MAX_JUMP_HEIGHT;
	//float max_speed = 577;   //(750 / 1.3 = 577)
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

	qboolean target_is_above = false, target_is_below = false, target_is_equal = false;
	float higher = 0;
	float lower = 0;
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

	// Small jump to target
	// -----------------
	//if (target_is_above && distance > STEPSIZE && distance < NODE_Z_HEIGHT)
	//	return NODE_JUMP;

	// Jump nodes
	//if (xyz_distance <= 96 && is_gap && fabs(origin[2] - target[2]) <= 48)
	//{
		//return NODE_JUMP;
	//}

	// Large jump up to target
	// --------------------
	// Jumppad nodes
	//if (target_is_above && distance > STEPSIZE)
	if (xyz_distance >= 16 && is_gap)
	{
		float gravity = sv_gravity->value; // This doesn't take into account the ent's own gravity (ent->gravity)

		//distance *= 1.0; // Increasing this increases the jump height
		// Calculate the jump height to get to the target
		float jump_height = sqrt(2 * gravity * xyz_distance);
		float jump_height_headroom = jump_height / 8; // head space required to make the jump from point to point

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
		tr = gi.trace(origin, tv(-16, -16, 0), tv(16, 16, jump_height_headroom), target, g_edicts, MASK_DEADSOLID); // Player height + some jumping height
		if (tr.fraction == 1.0 && !tr.startsolid)
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
				//if (speed < 550 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 500)
				if (speed < 550 && z_height <= z_height_max + sqrtf(xy_distance) && jump_height <= 450)
					return NODE_JUMPPAD;
			}
			// Roughly equal
			else if (fabsf(origin[2] - target[2]) <= 8) // target node is +/- 8
			{
				float speed = VectorLength(velocity);
				if (speed < 600 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 750)
					return NODE_JUMPPAD;
			}
			// Below
			else
			{
				z_height = (origin[2] - target[2]) - NODE_Z_HEIGHT;
				velocity[2] -= z_height;

				float speed = VectorLength(velocity);
				if (speed < 750 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 965)
					return NODE_JUMPPAD;
			}

			// Calculate speed from velocity
			//float speed = VectorLength(velocity);

			//gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);
			//Com_Printf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);
			//if (distance > 64 && speed < 750 && z_height < z_height_max + 24 && jump_height <= 965)
			//if (speed < 750 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 965) // Works VERY well
			//if (speed < 750 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 500)

			//if (speed < 750 && z_height < z_height_max + NODE_Z_HALF_HEIGHT && jump_height <= 965)
			//	return NODE_JUMPPAD;
		}
	}


	// Limit max speed
	//if (speed < 750 && z_height < 150 && jump_height <= 965) // Maximum
	//if (speed <= 750 && z_height < 48 && jump_height <= 525) // Works well for ground based nodes
	//if (speed <= 750 && z_height < NODE_MAX_JUMP_HEIGHT && jump_height <= 965)
	//if (speed <= 750 && z_height <= z_height_max && jump_height <= 600) // Works really well!!
	//if (speed <= 750 && z_height <= z_height_max && jump_height <= 700)
	//if (speed <= 750 && z_height <= z_height_max && jump_height <= 650) // Works uber well
	//if (speed <= 750 && z_height <= z_height_max && (jump_height <= 650 || lower <= NODE_MAX_CROUCH_FALL_HEIGHT + 64))
	//if (distance <= 224 && (higher <= NODE_MAX_JUMP_HEIGHT + 40 || lower <= NODE_MAX_CROUCH_FALL_HEIGHT + 32) )
	//if (distance <= curr_dist && (higher <= NODE_MAX_JUMP_HEIGHT + 40 || lower <= NODE_MAX_CROUCH_FALL_HEIGHT + 32))
	//if (distance <= curr_dist && (higher <= NODE_MAX_JUMP_HEIGHT + 40 || lower <= NODE_MAX_CROUCH_FALL_HEIGHT + 32))

	// Small jump
	//if (distance < 64 && higher > STEPSIZE && higher <= NODE_MAX_JUMP_HEIGHT)
		//return NODE_JUMP;

	// Jumping up
	//if (distance < 128 && higher > STEPSIZE && higher <= z_height_max)
	//{
		//return NODE_JUMPPAD;
	//}




	// Running to target
	// -----------------
	if (xy_distance < 400 && !is_gap)
	{
		if (origin[2] == target[2])
		{
			//return INVALID;

			//if (from == 1066 && to == 1069)
			//	Com_Printf("%s\n", __func__);

			//tr = gi.trace(origin, tv(-16, -16, -NODE_Z_HEIGHT), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID);
			tr = gi.trace(origin, tv(-15, -15, -NODE_Z_HEIGHT), tv(15, 15, 24), target, g_edicts, MASK_DEADSOLID);
			//tr = gi.trace(origin, tv(-16, -16, -NODE_Z_HEIGHT), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID);
			if (tr.fraction == 1)
				return NODE_MOVE;
		}
		else if (fabsf(origin[2] - target[2]) <= STEPSIZE) // target node is +/- STEPSIZE
		{
			//tr = gi.trace(origin, tv(-16, -16, -(NODE_Z_HEIGHT - STEPSIZE)), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID);
			tr = gi.trace(origin, tv(-15, -15, -(NODE_Z_HEIGHT - STEPSIZE)), tv(15, 15, 24), target, g_edicts, MASK_DEADSOLID);
			//tr = gi.trace(origin, tv(-16, -16, -(NODE_Z_HEIGHT - STEPSIZE)), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID);
			if (tr.fraction == 1)
				return NODE_MOVE;
		}
	}

	/*
	// Jump to target
	// -----------------
	if (xy_distance < 128)
	{
		if (abs(origin[2] - target[2]) <= NODE_MAX_JUMP_HEIGHT) // target node is +/- NODE_MAX_JUMP_HEIGHT
		{
			// We passed a partial height test, now test players step height
			// -32 is ground + 17 (one less than full step height of 18) = -14
			tr = gi.trace(origin, tv(-16, -16, -(NODE_Z_HEIGHT - STEPSIZE)), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID); // Step height
			if (tr.fraction < 1) // We hit something
			{
				// Test if we can jump over
				// mins: -32 is ground + 18 (step height) = -16
				// maxs: mins (-16) + full player size (56) = 40
				//tr = gi.trace(origin, tv(-16, -16, -16), tv(16, 16, 40), target, g_edicts, MASK_DEADSOLID); // Jump height + player height
				//if (tr.fraction == 1.0) // We hit nothing, so we can jump over :)
				//	return NODE_JUMP;

				//VectorCopy(origin, vec_from);
				//VectorCopy(target, vec_to);
				//origin
				
				// Test if we fit inside target area
				tr = gi.trace(target, tv(-16, -16, -32), tv(16, 16, 24), target, g_edicts, MASK_DEADSOLID);
				if (tr.startsolid == false && tr.fraction == 1.0) // We fit
					return NODE_JUMP;
				//return NODE_JUMP;
			}
			else
				return NODE_JUMP;
		}
	}
	*/
	


	// Run up/down slopes and ramps
	if (xy_distance < 512 && !is_gap)
	{
		// If we have LoS
		tr = gi.trace(origin, tv(-16, -16, -14), tv(16, 16, 24), target, g_edicts, MASK_PLAYERSOLID);
		if (tr.fraction > 0.9)
		{
			// Test for slope / stairs at the midpoint
			trace_t tr_50;
			vec3_t pt_50, end_50;
			LerpVector(origin, target, 0.50, pt_50); // Get the two vector midpoint
			VectorCopy(pt_50, end_50);
			end_50[2] -= NODE_Z_HEIGHT_PLUS_STEPSIZE;
			tr_50 = gi.trace(pt_50, NULL, NULL, end_50, g_edicts, MASK_DEADSOLID);
			if (tr_50.fraction < 1.0) // We hit solid
			{
				// Slope
				if (tr_50.plane.normal[0] != 0 || tr_50.plane.normal[1] != 0)
					return NODE_MOVE;
			}
		}
	}


	return INVALID;
}

// Check to see if triangles are coplanar
// Check to see if the triangles are adjacent
//qboolean TrianglesAreCoplanar(vec3_t t1_p1, vec3_t t1_p2, vec3_t t1_p3, vec3_t t2_p1, vec3_t t2_p2, vec3_t t2_p3)

// Explodes all breakable glass :D
void Remove_All_Breakableglass(void)
{
	edict_t* glass;
	for (glass = g_edicts; glass < &g_edicts[globals.num_edicts]; glass++)
	{
		if (!glass)
			continue;
		//if (glass->solid == SOLID_NOT)
		//	continue;
		if (!glass->classname)
			continue;

		if (strcmp(glass->classname, "breakableglass_trigger") == 0 || strcmp(glass->classname, "func_explosive") == 0)
		{
			//gi.dprintf("Found glass at location %f %f %f\n", glass->s.origin[0], glass->s.origin[1], glass->s.origin[2]);
			//glass->takedamage = DAMAGE_YES;
			//glass->die = kill_door;
			//glass->health = 0;
			//glass->max_health = 0;

			// Explode the glass
			T_Damage(glass, glass, glass, vec3_origin, vec3_origin, vec3_origin, 100000, 1, 0, MOD_UNKNOWN);
		}
	}

	/*
	// iterate over all func_explosives
	while ((glass = G_Find(glass, FOFS(classname), "func_explosive")) != NULL)
	{
		// glass is broken if solid != SOLID_BSP
		if (glass->solid != SOLID_BSP)
		{
			T_Damage(glass, glass, glass, vec3_origin, vec3_origin, vec3_origin, 100000, 1, 0, MOD_UNKNOWN);
		}
	}
	*/
}

void kill_door(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
	G_FreeEdict(self);
}
// Removes all door types
void Remove_All_Doors(void)
{
	edict_t* door;
	for (door = g_edicts; door < &g_edicts[globals.num_edicts]; door++)
	{
		if (!door)
			continue;
		if (door->solid == SOLID_NOT)
			continue;
		if (!door->classname)
			continue;

		if (strcmp(door->classname, "func_door_rotating") == 0 || strcmp(door->classname, "func_door") == 0 || strcmp(door->classname, "func_door_secret") == 0)
		{
			//gi.dprintf("Found door at location %f %f %f\n", door->s.origin[0], door->s.origin[1], door->s.origin[2]);
			door->takedamage = DAMAGE_YES;
			door->die = kill_door;
			door->health = 0;
			door->max_health = 0;

			// Open up the door's area portal (otherwise players will see hall-of-mirrors effect
			edict_t* t = NULL;
			if (door->target)
			{
				while ((t = G_Find(t, FOFS(targetname), door->target)) != NULL)
				{
					if (Q_stricmp(t->classname, "func_areaportal") == 0) {
						gi.SetAreaPortalState(t->style, true);
					}
				}
			}

			// Explode the door
			T_Damage(door, door, door, vec3_origin, vec3_origin, vec3_origin, 100000, 1, 0, MOD_UNKNOWN);
		}
	}
}

void Open_All_Doors(edict_t* ent)
{
	if (ent != NULL)
	{
		if (ent->last_door_time > level.framenum - 2.0 * HZ)
			return;
		else
			ent->last_door_time = level.framenum + random() * 2.5 * HZ; // wait!
	}
	// Open all doors and keep them open
	edict_t* door;
	for (door = g_edicts; door < &g_edicts[globals.num_edicts]; door++)
	{
		if (!door)
			continue;
		if (door->solid == SOLID_NOT)
			continue;
		if (!door->classname)
			continue;

		if (strcmp(door->classname, "func_door_rotating") == 0 || strcmp(door->classname, "func_door") == 0 || strcmp(door->classname, "func_door_secret") == 0)
		{
			//gi.dprintf("Found door at location %f %f %f\n", door->s.origin[0], door->s.origin[1], door->s.origin[2]);

			int DOOR_TOGGLE = 32; //#define DOOR_TOGGLE 32
			if ((door->spawnflags & DOOR_TOGGLE) == 0)
				door->spawnflags |= DOOR_TOGGLE; // Bitwise add DOOR_TOGGLE to door->spawnflags
			door->wait = -1;
			door->speed = 9000;
			door->touch_debounce_framenum = 0;
			door->moveinfo.accel = 9000;
			door->moveinfo.decel = 9000;
			door->moveinfo.speed = 9000;
			//door_use(door, ent, ent); // Open the door
		}
	}
}

// Find ladders by searching the center of bottom edges of walls that are CONTENTS_LADDER
#define MAX_LADDER_FACES 3072
vec3_t surface_ladder_faces[MAX_LADDER_FACES] = { 0 }; // Origin
void ACEND_FindEdgeLadders(void)
{
	int f, e, f2, f3, i;
	trace_t tr;
	//vec3_t zero = { 0 };
	//	Player when standing
	//vec3_t  player_standing_mins_up = { -16, -16, -1 };
	//vec3_t  player_standing_mins = { -16, -16, -0 };
	//vec3_t  player_standing_maxs = { 16, 16, 32 };


	//vec3_t *ladder_faces = (vec3_t*)malloc(sizeof(vec3_t) * MAX_LADDER_FACES);
	//if (ladder_faces == NULL)
	{
		//Com_Printf("%s [CONTENTS_LADDER] ladder_faces malloc failed\n", __func__);
		//return;
	}
	//vec3_t* ladder_face;

	//short int ladder_index[MAX_LADDER_FACES] = { 0 }; // Face
	int* ladder_index = malloc(sizeof(int) * MAX_LADDER_FACES);
	if (ladder_index == NULL)
	{
		Com_Printf("%s [CONTENTS_LADDER] ladder_index malloc failed\n", __func__);
		return;
	}


	int num_ladder_faces = 0;
	qboolean foundLadder = true;
	qboolean ladderLimit = false;

	/*
	for (f = 0; f < nmesh.total_faces; f++)
	{
		if (nmesh.face[f].contents & CONTENTS_LADDER)
		{
			Com_Printf("%s face:%i tris:%i contents:0x%x (LADDER)\n", __func__, f, nmesh.face[f].num_tris, nmesh.face[f].contents);
		}
	}
	*/

	// Find all the ladder faces
	for (f = 0; f < nmesh.total_faces; f++)
	{
		if (ladderLimit) break;

		//if (nmesh.face[f].contents & CONTENTS_LADDER)
		//	Com_Printf("%s face:%i tris:%i contents:0x%x (LADDER)\n", __func__, f, nmesh.face[f].num_tris, nmesh.face[f].contents);

		if (nmesh.face[f].type == FACETYPE_WALL && nmesh.face[f].contents & CONTENTS_LADDER) // If wall is Ladder
		{
			for (e = 0; e < nmesh.face[f].num_edges; e++) // Find the bottom of the ladder using the middle of the lowest edge
			{
				// Look for horizontal edges - check if the two edge verts are level
				if (nmesh.face[f].edge[e].v[0][2] == nmesh.face[f].edge[e].v[1][2])
				{
					// Check if we already added the ladder face to ladder_faces[]
					foundLadder = true;
					for (f3 = 0; f3 < num_ladder_faces; f3++)
						//for (f3 = 0, ladder_face = ladder_faces; f3 < num_ladder_faces; f3++, ladder_face++)
					{
						if (surface_ladder_faces[f3][0] == nmesh.face[f].edge[e].center[0] && surface_ladder_faces[f3][1] == nmesh.face[f].edge[e].center[1] && surface_ladder_faces[f3][2] == nmesh.face[f].edge[e].center[2])
							//if (*(ladder_face)[0] == nmesh.face[f].edge[e].center[0] && *(ladder_face)[1] == nmesh.face[f].edge[e].center[1] && *(ladder_face)[2] == nmesh.face[f].edge[e].center[2])
						{
							//Com_Printf("%s  f:%i  e:%i\n", __func__, f, e);
							foundLadder = false;
							break;
						}
					}

					// Found new ladder edge
					if (foundLadder)
					{
						// Check we're not over the limit
						if (num_ladder_faces + 2 > MAX_LADDER_FACES)
						{
							Com_Printf("%s [CONTENTS_LADDER] num_ladder_faces > max_faces\n", __func__);
							ladderLimit = true;
							break;
						}

						//Com_Printf("%s [CONTENTS_LADDER] f:%i e:%d n:%d [%f %f %f]\n", __func__, f, e, nmesh.face[f].edge[e].node, nmesh.face[f].edge[e].center[0], nmesh.face[f].edge[e].center[1], nmesh.face[f].edge[e].center[2]);

						//ladder_index[num_ladder_faces] = f; // Copy face
						VectorCopy(nmesh.face[f].edge[e].center, surface_ladder_faces[num_ladder_faces]); // Copy origin

						*(ladder_index + num_ladder_faces) = f; // Copy face

						//Com_Printf("%s  ladder_index:%i  f:%i\n", __func__, *(ladder_index + num_ladder_faces), f);

						//for (int i = 0; i < 3; i++)
						//	*(ladder_faces + num_ladder_faces * sizeof(float))[i] = nmesh.face[f].edge[e].center[i]; // Copy origin
						//
						//ladder_face = ladder_faces + (num_ladder_faces * sizeof(vec3_t)); // Set pointer to end of array
						//*(ladder_faces + (num_ladder_faces + sizeof(vec3_t)))[0] = nmesh.face[f].edge[e].center[0];
						//*(ladder_faces + (num_ladder_faces + sizeof(vec3_t)))[1] = nmesh.face[f].edge[e].center[1];
						//*(ladder_faces + (num_ladder_faces + sizeof(vec3_t)))[2] = nmesh.face[f].edge[e].center[2];

						num_ladder_faces++;
					}
				}
			}
		}
	}




	for (f = 0; f < num_ladder_faces; f++)
		//for (f = 0, ladder_face = ladder_faces; f < num_ladder_faces; f++, ladder_face++)
	{
		// Com_printf ladder_faces
		//Com_Printf("%s [CONTENTS_LADDER] f:%i [%f %f %f]\n", __func__, ladder_index[f], ladder_faces[f][0], ladder_faces[f][1], ladder_faces[f][2]);
		//Com_Printf("%s [CONTENTS_LADDER] f:%i [%f %f %f]\n", __func__, *(ladder_index + f), *(ladder_face)[0], *(ladder_face)[1], *(ladder_face)[2]);
	}





	// Search all ladder faces looking for the lowest and highest parts of the ladder
	//
	//  |-|   <-- highest
	//  |-|
	//  |-|   <-- lowest
	//
	qboolean tested = false;
	int faces_tested_num = 0;
	//short int faces_tested[MAX_LADDER_FACES];
	int* faces_tested = malloc(sizeof(int) * MAX_LADDER_FACES);
	if (faces_tested == NULL)
	{
		Com_Printf("%s [CONTENTS_LADDER] faces_tested malloc failed\n", __func__);
		return;
	}


	//vec3_t ladder_final[MAX_LADDER_FACES]; // Origin
	//int ladder_final_counter = 0;
	float low_z, high_z; // Height of lowest and highest parts of ladder
	int low_f, high_f; // Face index of lowest and highest parts of ladder
	for (f = 0; f < num_ladder_faces; f++)
	{
		low_z = 99999, high_z = -99999; // Reset lowest and highest height
		low_f = high_f = INVALID; // Reset lowest and highest face index

		tested = false;
		for (int ft = 0; ft < faces_tested_num; ft++)
		{
			//if (faces_tested[ft] == f)
			if (*(faces_tested + ft) == f)
			{
				tested = true; // Skip if already tested
				break;
			}
		}
		if (tested)
			continue;

		for (f2 = 0; f2 < num_ladder_faces; f2++)
		{
			// Check if the next ladder has the same X and Y alignment
			if (surface_ladder_faces[f][0] == surface_ladder_faces[f2][0] && surface_ladder_faces[f][1] == surface_ladder_faces[f2][1])
				//if (*(ladder_faces + f)[0] == *(ladder_faces + f2)[0] && *(ladder_faces + f)[1] == *(ladder_faces + f2)[1])
			{
				//Com_Printf("%s [F %i] [F %i] [%f %f %f] [%f %f %f] \n", __func__, f, f2, ladder_faces[f][0], ladder_faces[f][1], ladder_faces[f][2], ladder_faces[f2][0], ladder_faces[f2][1], ladder_faces[f2][2]);

				//faces_tested[faces_tested_num++] = f2; // Add face to faces_tested
				*(faces_tested + faces_tested_num++) = f2; // Add face to faces_tested



				if (surface_ladder_faces[f2][2] < low_z)
					//if (*(ladder_faces + f2)[2] < low_z)
				{
					//Com_Printf("%s [F %i] [ladder_faces[f2][2] < low_z] %f < %f \n", __func__, f2, ladder_faces[f2][2], low_z);
					low_z = surface_ladder_faces[f2][2]; // Update lowest height
					//low_z = *(ladder_faces + f2)[2]; // Update lowest height
					low_f = f2; // Update lowest face index
				}
				if (surface_ladder_faces[f2][2] > high_z)
					//if (*(ladder_faces + f2)[2] > high_z)
				{
					//Com_Printf("%s [F %i] [ladder_faces[f2][2] > high_z] %f > %f \n", __func__, f2, ladder_faces[f2][2], high_z);
					high_z = surface_ladder_faces[f2][2]; // Update highest height
					//high_z = *(ladder_faces + f2)[2]; // Update highest height
					high_f = f2; // Update highest face index
				}
			}
		}

		foundLadder = false;

		float distance = 0;
		if (surface_ladder_faces[high_f][2] > surface_ladder_faces[low_f][2]) // Make sure the top of the ladder is higher than the bottom
			distance = surface_ladder_faces[high_f][2] - surface_ladder_faces[low_f][2];

		if (distance >= 32 && low_f != INVALID && high_f != INVALID)
		{
			//Com_Printf("%s LO  F:%i L:%f  [F %i][%f %f %f]\n", __func__, low_f, low_z, ladder_index[low_f], ladder_faces[low_f][0], ladder_faces[low_f][1], ladder_faces[low_f][2]);
			//Com_Printf("%s HI  F:%i H:%f  [F %i][%f %f %f]\n", __func__, high_f, high_z, ladder_index[high_f], ladder_faces[high_f][0], ladder_faces[high_f][1], ladder_faces[high_f][2]);


			if (1) // Project the top of the ladder forward, then we test to see if its in the ground, if so we need to move the top node up
			{
				vec3_t ladder_forward;
				VectorCopy(surface_ladder_faces[high_f], ladder_forward);
				//ladder_forward[2] += NODE_Z_HEIGHT;
				// Move the top ladder node closer to the ladder
				int f3 = *(ladder_index + high_f);
				//Com_Printf("%s f3:%i\n", __func__, f3);
				if (nmesh.face[f3].normal[0] != 0) // X
				{
					if (nmesh.face[f3].drawflags & DSURF_PLANEBACK)
						ladder_forward[0] += (NODE_Z_HEIGHT * nmesh.face[f3].normal[0]); // -X (forward wall)
					else
						ladder_forward[0] -= (NODE_Z_HEIGHT * nmesh.face[f3].normal[0]); // +X (back wall) 
				}
				if (nmesh.face[f3].normal[1] != 0) // Y
				{
					if (nmesh.face[f3].drawflags & DSURF_PLANEBACK)
						ladder_forward[1] += (NODE_Z_HEIGHT * nmesh.face[f3].normal[1]); // -Y (left wall)
					else
						ladder_forward[1] -= (NODE_Z_HEIGHT * nmesh.face[f3].normal[1]); // +Y (right wall)
				}
				// Test if we're in the ground
				for (i = 0; i < 100; i++) //NODE_Z_HEIGHT
				{
					tr = gi.trace(ladder_forward, tv(-16, -16, -NODE_Z_HEIGHT), tv(16, 16, 0), ladder_forward, g_edicts, MASK_PLAYERSOLID);
					if (tr.startsolid || tr.fraction < 1)
					{
						ladder_forward[2]++;
					}
					else
						break;
				}

				//Com_Printf("%s Increased top of ladder [%f %f %f] -> [%f %f %f]\n", __func__, ladder_faces[high_f][0], ladder_faces[high_f][1], ladder_faces[high_f][2], ladder_forward[0], ladder_forward[1], ladder_forward[2]);
				surface_ladder_faces[high_f][2] = ladder_forward[2];
			}

			// Move the top of the ladder up before testing, this serves two purposes: test if ladder hits roof, make sure ladder reaches top
			//ladder_faces[high_f][2] += NODE_Z_HEIGHT + 8;
			//ladder_faces[high_f][2] += NODE_Z_HEIGHT;

			if (1)
				// Test if bottom of ladder is in the ground, move it up until its not
				for (i = 0; i < 100; i++) //NODE_Z_HEIGHT
				{
					tr = gi.trace(surface_ladder_faces[low_f], tv(-16, -16, -NODE_Z_HEIGHT), tv(16, 16, 0), surface_ladder_faces[low_f], g_edicts, MASK_PLAYERSOLID);
					if (tr.startsolid || tr.fraction < 1)
					{
						if (surface_ladder_faces[low_f][2] + 1 < surface_ladder_faces[high_f][2])
							surface_ladder_faces[low_f][2]++;
					}
					else
						break;
				}

			//if (ladder_faces[low_f][2] + NODE_Z_HEIGHT < ladder_faces[high_f][2]) // Make sure bottom ladder doesn't go above top of ladder
			//	ladder_faces[low_f][2] += NODE_Z_HEIGHT;






			// Check if ladder ends can be connected, if so add nodes and links
			//tr = gi.trace(ladder_faces[low_f], player_standing_mins_up, player_standing_maxs, ladder_faces[high_f], g_edicts, MASK_PLAYERSOLID);
			tr = gi.trace(surface_ladder_faces[low_f], MINS_PLAYER_STANDING, MAXS_PLAYER_STANDING, surface_ladder_faces[high_f], g_edicts, MASK_PLAYERSOLID);
			if (tr.startsolid == false && tr.fraction == 1.0)
			{
				//ACEND_AutoAddNode(*(ladder_faces + low_f), nmesh.face[low_f].normal, NODE_LADDER_DOWN);
				//ACEND_AutoAddNode(*(ladder_faces + high_f), nmesh.face[high_f].normal, NODE_LADDER_UP);

				int tn = INVALID; // Top node
				int bn = INVALID; // Bottom node

				// Add Top ladder node
				// ----------------------
				// Top ladder doesn't have to worry about existing nodes, so just make a new one
				DAIC_Add_Node(surface_ladder_faces[high_f], nmesh.face[high_f].normal, NODE_LADDER_UP);
				//ACEND_AutoAddNode(ladder_faces[high_f], nmesh.face[high_f].normal, NODE_LADDER_UP);
				tn = numnodes - 1;
				// ----------------------

				// Add bottom ladder node
				// ----------------------
				// Look for an existing node located on the triangle where the bottom of the ladder is, 
				// if so then move that node origin to the bottom ladder origin
				qboolean found = false;
				int f = INVALID;
				int t = INVALID;
				//vec3_t ladder_low;
				//VectorCopy(ladder_faces[low_f], ladder_low);
				//ladder_low[2] -= 24;
				//GetBSPFaceTriangle(ladder_low, &found, &f, &t);
				GetBSPFaceTriangle(surface_ladder_faces[low_f], &found, &f, &t);
				if (found && f != INVALID && t != INVALID) //f = 0; t = 0; found = true;
				{
					found = false;
					bn = nmesh.face[f].tris[t].node;
					if (bn != INVALID) // Successfully found node
					{
						//Com_Printf("%s ladder tris     found [%f %f %f]\n", __func__, ladder_faces[low_f][0], ladder_faces[low_f][1], ladder_faces[low_f][2]);

						found = true;
						VectorCopy(surface_ladder_faces[low_f], nodes[bn].origin); // Move node origin to bottom ladder origin
						nodes[bn].type = NODE_LADDER_DOWN; // Update its type
					}
				}
				// Otherwise we add a new node for the bottom of the ladder
				if (found == false)
				{
					//Com_Printf("%s ladder tris not found [%f %f %f]\n", __func__, ladder_faces[low_f][0], ladder_faces[low_f][1], ladder_faces[low_f][2]);
					DAIC_Add_Node(surface_ladder_faces[low_f], nmesh.face[low_f].normal, NODE_LADDER_DOWN);
					//ACEND_AutoAddNode(ladder_faces[low_f], nmesh.face[low_f].normal, NODE_LADDER_DOWN);
					bn = numnodes - 1;
				}
				// ----------------------

				//Com_Printf("%s NODE_LADDER_DOWN f:%i n:%d [%f %f %f]\n", __func__, ladder_index[low_f], bn, ladder_faces[low_f][0], ladder_faces[low_f][1], ladder_faces[low_f][2]);
				//Com_Printf("%s NODE_LADDER_UP   f:%i n:%d [%f %f %f]\n", __func__, ladder_index[high_f], tn, ladder_faces[high_f][0], ladder_faces[high_f][1], ladder_faces[high_f][2]);

				if (bn != INVALID && tn != INVALID)
				{
					BOTLIB_AddNodeLink(bn, tn, NODE_LADDER_DOWN, true); // Bottom to top
					BOTLIB_AddNodeLink(tn, bn, NODE_LADDER_UP, true); // Top to bottom
					ACEND_UpdateNodeReach(bn, tn);
					ACEND_UpdateNodeReach(tn, bn);
					nmesh.total_reach += 2;
				}
			}
		}
	}

	// Free memory
	free(faces_tested);
	faces_tested = NULL;
}

/*
// Find if origin is touching a ladder
// Can be used to see if node is touching a ladder
qboolean ACEND_FindLadder(vec3_t origin, vec3_t ladder_bottom, vec3_t ladder_top)
{
	float yaw_rad = 0, yaw = 0, depth;
	int loops;
	vec3_t fwd = { 0 }, start = { 0 }, end = { 0 }, final = { 0 }, ground = { 0 };
	vec3_t mins = { -4, -4, -24 }, maxs = { 4, 4, 32 };
	trace_t tr;
	qboolean is_touching = false, found_bottom = false;
	VectorCopy(origin, start);

	//	Player when standing
	vec3_t  player_standing_mins = { -16, -16, -0 };
	vec3_t  player_standing_maxs = { 16, 16, 32 };
	//	Player when crouching/ducking
	vec3_t  player_crouching_mins = { -16, -16, -24 };
	vec3_t  player_crouching_maxs = { 16, 16, 4 };
	//	Player when standing smaller mins
	vec3_t  player_standing_smaller_mins = { -8, -8, -0 };
	vec3_t  player_standing_smaller_maxs = { 8, 8, 0 };

	return false; //////////////////////////////////////////////////////////////////////////////////////////// TEMP DISABLE LADDERS

	depth = 0;
	for (int d = 0; d < 128; d++) // depth
	{
		// For each yaw angle, project forward and see if touching a ladder
		for (int a = 0; a < 64; a++) // Spin around in a 360 circle
		{
			if (yaw >= 360) // Make sure we come back around
				yaw = 0;

			yaw_rad = DEG2RAD(yaw);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);
			VectorMA(start, depth, fwd, end);
			VectorMA(start, (depth - 20), fwd, final); // Back away from the ladder until we're about the distance of the player center (give or take a little)
			tr = gi.trace(start, mins, maxs, end, g_edicts, MASK_PLAYERSOLID);
			//if (tr.fraction < 1 && (tr.contents & CONTENTS_LADDER))
			if (tr.contents & CONTENTS_LADDER)
			{
				// Find the bottom of the ladder
				//--------------------
				// Now find the ground so we know not to go below this point
				VectorCopy(end, ground);
				ground[2] -= 8192;
				tr = gi.trace(ladder_top, NULL, NULL, ground, g_edicts, MASK_PLAYERSOLID);
				if (tr.fraction < 1)
				{
					VectorCopy(tr.endpos, ground);
				}
				loops = 0;
				while (loops++ < 64)
				{
					// Now move down testing until we no longer hit the ladder to find its bottom
					if (end[2] + 32 > ground[2]) // Can we move down?
					{
						// Okay move down
						start[2] -= 32;
						end[2] -= 32;
						final[2] -= 32;

						tr = gi.trace(start, player_standing_smaller_mins, player_standing_smaller_maxs, end, g_edicts, MASK_PLAYERSOLID);
						if ((tr.contents & CONTENTS_LADDER) == 0)
						{
							// Ladder reached the ground so adjust the position so we're not stuck in the ground
							if (tr.startsolid)
							{
								start[2] = tr.endpos[2] + NODE_Z_HEIGHT;
								end[2] = tr.endpos[2] + NODE_Z_HEIGHT;
								final[2] = tr.endpos[2] + NODE_Z_HEIGHT;
							}
							else
							{
								// Hanging ladder, didn't reach the ground
							}
							VectorCopy(final, ladder_bottom);
							Com_Printf("Found bottom ladder at location %f %f %f\n", ladder_bottom[0], ladder_bottom[1], ladder_bottom[2]);
							found_bottom = true;
							break; // end while loop
						}
					}
				}
				//--------------------


				// Find the top of the ladder
				//--------------------
				if (found_bottom)
				{
					// Now move up testing until we no longer hit the ladder to find its top
					loops = 0;
					while (loops++ < 64)
					{
						// move up
						start[2] += 32;
						end[2] += 32;
						final[2] += 32;

						tr = gi.trace(start, player_standing_smaller_mins, player_standing_smaller_maxs, end, g_edicts, MASK_PLAYERSOLID);
						if ((tr.contents & CONTENTS_LADDER) == 0)
						{
							final[2] += NODE_Z_HEIGHT; // Adjust the final height above the top of the ladder
							VectorCopy(final, ladder_top); // Ladder top

							// Make sure the ladder z height length is large enough
							// edge cases: where ladder handles go above the ground we're trying to reach, such as urban2 the very highest ladder
							float difference = ladder_top[2] - ladder_bottom[2];
							if (difference > 96)
							{
								Com_Printf("Found top ladder at location %f %f %f\n", ladder_top[0], ladder_top[1], ladder_top[2]);
								is_touching = true;
								return is_touching;
							}
						}
					}
				}
				//--------------------

				if (is_touching)
					break;
			}
			yaw += 5.625; // Change yaw angle
		}
		depth += 2;
	}

	return is_touching;
}
*/

//rekkie -- walknodes -- s


void BOTLIB_SetAllNodeNormals(void)
{
	vec3_t mins = { -14, -14, -4, };
	vec3_t maxs = { 14, 14, 4, };

	for (int i = 0; i < numnodes; i++)
	{
		// Get normal
		trace_t tr = gi.trace(tv(nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2] + 30), mins, maxs, tv(nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2] - 64), NULL, MASK_SHOT);
		if (tr.fraction == 1.0)
		{
			//Com_Printf("%s failed node[%d] normal [%f %f %f]\n", __func__, nodes[i].nodenum, tr.plane.normal[0], tr.plane.normal[1], tr.plane.normal[2]);
			continue;
		}

		VectorCopy(tr.plane.normal, nodes[i].normal);
	}
}

// Compression -- s
#if USE_ZLIB
#include <zlib.h>

#define CHUNK_SIZE 16384

void BOTLIB_DecompressBuffer(char* inputBuffer, long inputLength, char* outputBuffer, long* outputLength) 
{
	z_stream stream = { 0 }; 
	stream.zalloc = Z_NULL; 
	stream.zfree = Z_NULL; 
	stream.opaque = Z_NULL;
	int status = inflateInit(&stream);
	if (status != Z_OK) {
		Com_Printf("Failed to initialize decompression stream! (error %d)\n", status);
		return;
	}

	stream.avail_in = inputLength;
	stream.next_in = (Bytef*)inputBuffer;
	stream.avail_out = *outputLength;
	stream.next_out = (Bytef*)outputBuffer;

	status = inflate(&stream, Z_FINISH);
	if ((status != Z_OK) && (status != Z_STREAM_END)) {
		Com_Printf("Failed to decompress data! (error %d)\n", status);
		inflateEnd(&stream);
		return;
	}

	*outputLength = stream.total_out;
	inflateEnd(&stream);
}
void BOTLIB_CompressBuffer(char* inputBuffer, long inputLength, char* outputBuffer, long* outputLength) {
	z_stream stream = { 0 };
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	int status = deflateInit(&stream, Z_BEST_COMPRESSION);
	if (status != Z_OK) {
		Com_Printf("Failed to initialize compression stream! (error %d)\n", status);
		return;
	}

	stream.avail_in = inputLength;
	stream.next_in = (Bytef*)inputBuffer;
	stream.avail_out = inputLength; // *outputLength;
	stream.next_out = (Bytef*)outputBuffer;

	status = deflate(&stream, Z_FINISH);
	if ((status != Z_OK) && (status != Z_STREAM_END)) {
		Com_Printf("Failed to compress data! (error %d)\n", status);
		deflateEnd(&stream);
		return;
	}

	*outputLength = stream.total_out;

	deflateEnd(&stream);
}

// Assume the following variables are declared on the other end:
// char* buffer; // Pointer to the outgoing buffer to add data to 
// int bufferLength; // Length of the outgoing buffer 
// int dataLength; // Length of the incoming data 
// char* incomingData; // Pointer to the incoming data to add
void BOTLIB_AddToBuffer(char* incomingData, int dataLength, char** bufferPtr, int* bufferLengthPtr) 
{
	if (*bufferPtr == NULL)
	{ // If the buffer is null, set the buffer to the incoming data )
		char *newBuffer = (char*)malloc(*bufferLengthPtr + dataLength);
		if (newBuffer != NULL)
		{ // If the allocation was successful, copy the incoming data to the end of the buffer 
			memcpy(newBuffer + (*bufferLengthPtr), incomingData, dataLength);
			*bufferPtr = newBuffer;
			*bufferLengthPtr += dataLength;
		} else { // If the allocation was not successful, handle the error 
			Com_Printf("Failed to allocate buffer for incoming data!\n"); 
		}
	}
	else
	{
		// If the buffer is not null, reallocate the buffer to the new size 
		char *newBuffer = (char*)realloc(*bufferPtr, *bufferLengthPtr + dataLength);
		if (newBuffer != NULL)
		{ // If the allocation was successful, copy the incoming data to the end of the buffer 
			memcpy(newBuffer + (*bufferLengthPtr), incomingData, dataLength);
			*bufferPtr = newBuffer;
			*bufferLengthPtr += dataLength;
		} else { // If the allocation was not successful, handle the error 
			Com_Printf("Failed to allocate buffer for incoming data!\n"); 
		}
	}
}


// ============================================================================================
// These group of functions sort the node links from highest to lowest based on their zheight
// difference from the node they're connected to. node -> link(s) zheight difference. We run
// this function upon saving nodes.
// 
// This is done so that when pathing the highest node links are consindered first. Although,
// currently the bots use Dijkstra Pathing, so this is ignored for now.
// ============================================================================================
// Temp link structure
typedef struct {
	int targetNode;
	int targetNodeType;
	double cost;
	double zheight; // Not part of the original link struct. Used by qsort() to order highest -> lowest
} QSLink; // Qsorted Link

// Compare function for qsort()
static int QScompare(const void* a, const void* b) {
	return ((QSLink*)b)->zheight - ((QSLink*)a)->zheight;
}

void QsortLinks(QSLink* links, size_t length) {
	qsort(links, length, sizeof(*links), QScompare);
}

// Sorts node links by height. Highest to lowest.
void QSNodeLinksByHeight(void)
{
	QSLink qslinks[MAXLINKS]; // Temp links

	for (int n = 0; n < numnodes; n++) // Each node
	{
		for (int i = 0; i < nodes[n].num_links; ++i) // Each link
		{
			// Copy original link data into our temp struct
			qslinks[i].targetNode = nodes[n].links[i].targetNode;
			qslinks[i].targetNodeType = nodes[n].links[i].targetNodeType;
			qslinks[i].cost = nodes[n].links[i].cost;

			// The original data struct doesn't use or contain the zheight of a linked node, so we create it here for qsort() to use, 
			int tnode = nodes[n].links[i].targetNode; // Target node
			//
			if ((nodes[tnode].origin[2] - nodes[n].origin[2]) >= 1) // Only sort linked nodes that are higher than the node itself and are >= 1 in height difference
				qslinks[i].zheight = (nodes[tnode].origin[2] - nodes[n].origin[2]);
			else
				qslinks[i].zheight = 0; // Any height < 1 just consider as 0 height
		}

		// Debug print
		/*
		qboolean hasDiffHeight = false;
		for (int i = 0; i < nodes[n].num_links; ++i)
		{
			if (nodes[n].links[i].zheight >= 1) // Only if height diff is >= 1
			{
				hasDiffHeight = true;
				break;
			}
		}
		*/

		// Debug print
		/*
		if (0 && hasDiffHeight)
		{
			Com_Printf("\nBefore sorting:\n");
			for (int i = 0; i < nodes[n].num_links; ++i)
				Com_Printf("{%d %d %f %f}\n", nodes[n].links[i].targetNode, nodes[n].links[i].targetNodeType, nodes[n].links[i].cost, nodes[n].links[i].zheight);
		}
		*/

		// Sort links from highest to lowest
		QsortLinks(qslinks, nodes[n].num_links);

		// Debug print
		/*
		if (0 && hasDiffHeight) // Debug print
		{
			Com_Printf("\nAfter sorting:\n");
			for (int i = 0; i < nodes[n].num_links; ++i)
				Com_Printf("{%d %d %f %f}\n", qslinks[i].targetNode, qslinks[i].targetNodeType, qslinks[i].cost, qslinks[i].zheight);
		}
		*/

		// Override and store the original link data
		for (int i = 0; i < nodes[n].num_links; ++i)
		{
			nodes[n].links[i].targetNode = qslinks[i].targetNode;
			nodes[n].links[i].targetNodeType = qslinks[i].targetNodeType;
			nodes[n].links[i].cost = qslinks[i].cost;
		}
	}
}
// ==============================================
// ==============================================

void BOTLIB_SaveNavCompressed(void)
{
	FILE* fOut;
	char filename[128];
	int fileSize = 0;
	int n, l; // File, nodes, links
	int version = BOT_NAV_VERSION;
	cvar_t* game_dir = gi.cvar("game", "action", 0);
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

#ifdef _WIN32
	int f;
	f = sprintf(filename, ".\\");
	f += sprintf(filename + f, game_dir->string);
	f += sprintf(filename + f, "\\");
	f += sprintf(filename + f, botdir->string);
	f += sprintf(filename + f, "\\nav\\");
	f += sprintf(filename + f, level.mapname);
	f += sprintf(filename + f, ".nav");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/nav/");
	strcat(filename, level.mapname);
	strcat(filename, ".nav");
#endif

	if ((fOut = fopen(filename, "wb")) == NULL) // Open file to write binary
	{
		Com_Printf("%s Failed to write NAV to file %s\n", __func__, filename);
		return;
	}

	Com_Printf("%s Writing compressed NAV to file...\n", __func__);

	QSNodeLinksByHeight(); // Sort node links by height, highest to lowest

	char* buff = NULL;
	int uncompressed_buff_len = 0;
	BOTLIB_AddToBuffer((char*)&numnodes, sizeof(unsigned short), &buff, &uncompressed_buff_len);
	for (n = 0; n < numnodes; n++)
	{
		// Nodes
		BOTLIB_AddToBuffer((char*)&nodes[n].area, sizeof(int), &buff, &uncompressed_buff_len);
		//BOTLIB_AddToBuffer((char*)&nodes[n].normal, sizeof(vec3_t), &buff, &uncompressed_buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].origin, sizeof(vec3_t), &buff, &uncompressed_buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].type, sizeof(byte), &buff, &uncompressed_buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].nodenum, sizeof(short int), &buff, &uncompressed_buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].inuse, sizeof(qboolean), &buff, &uncompressed_buff_len);

		// Links
		BOTLIB_AddToBuffer((char*)&nodes[n].num_links, sizeof(byte), &buff, &uncompressed_buff_len);
		for (l = 0; l < nodes[n].num_links; l++)
		{
			BOTLIB_AddToBuffer((char*)&nodes[n].links[l].targetNode, sizeof(short int), &buff, &uncompressed_buff_len);
			BOTLIB_AddToBuffer((char*)&nodes[n].links[l].targetNodeType, sizeof(byte), &buff, &uncompressed_buff_len);
			BOTLIB_AddToBuffer((char*)&nodes[n].links[l].cost, sizeof(float), &buff, &uncompressed_buff_len);
		}

		// Paths
		//for (p = 0; p < numnodes; p++)
		//{
			//BOTLIB_AddToBuffer((char*)&path_table[n][p], sizeof(short int), &buff, &uncompressed_buff_len); // Path table			
		//}
	}

	// Compress the buffer
	long compressed_buff_len = 0;
	char* compressed_buff = (char*)malloc(uncompressed_buff_len); // Make the compressed buffer as large as the uncompressed buffer
	if (compressed_buff != NULL)
	{
		BOTLIB_CompressBuffer(buff, uncompressed_buff_len, compressed_buff, &compressed_buff_len);

		// Compare and print the size of the buffers
		//Com_Printf("%s uncompressed_buff_len:%i compressed_buff_len:%i\n", __func__, uncompressed_buff_len, compressed_buff_len);

		// Write version, checksum, and buffer sizes
		fileSize += sizeof(byte) * fwrite(&version, sizeof(byte), 1, fOut);									// NAV Version
		//fileSize += sizeof(unsigned) * fwrite(&nmesh.bsp_checksum, sizeof(unsigned), 1, fOut);			// Map checksum
		fileSize += sizeof(int) * fwrite(&uncompressed_buff_len, sizeof(int), 1, fOut);						// Uncompressed buffer size
		fileSize += sizeof(int) * fwrite(&compressed_buff_len, sizeof(int), 1, fOut);						// Compressed buffer size

		// Write compressed node data
		fwrite(compressed_buff, compressed_buff_len, 1, fOut);
		fileSize += compressed_buff_len;
	}
	else
	{
		Com_Printf("%s Failed to allocate memory for compressed nav buffer\n", __func__);
	}

	// Cache all the POI nodes
	if (numnodes)
	{
		num_poi_nodes = 0; // Reset any previous POI nodes

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].type == NODE_POI)
			{
				poi_nodes[num_poi_nodes] = i;
				num_poi_nodes++;
			}
		}
	}

	fclose(fOut);

	if (buff != NULL)
		free(buff);

	if (compressed_buff != NULL)
		free(compressed_buff);

	Com_Printf("%s saved %s containing %d nodes. Nav zlib compressed [%i bytes] to [%i bytes]\n", __func__, filename, numnodes, uncompressed_buff_len, fileSize);
}
void BOTLIB_LoadNavCompressed(void)
{
	FILE* fIn;
	char filename[128];
	int fileSize = 0;
	int n, l; // File, nodes, links
	int version = 0; // Bot nav version
	unsigned bsp_checksum = 0; // Map checksum
	cvar_t* game_dir = gi.cvar("game", "action", 0);
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib
	const vec3_t mins = { -16, -16, -24 };
	const vec3_t maxs = { 16, 16, 32 };

#ifdef _WIN32
	int f;
	f = sprintf(filename, ".\\");
	f += sprintf(filename + f, game_dir->string);
	f += sprintf(filename + f, "\\");
	f += sprintf(filename + f, botdir->string);
	f += sprintf(filename + f, "\\nav\\");
	f += sprintf(filename + f, level.mapname);
	f += sprintf(filename + f, ".nav");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/nav/");
	strcat(filename, level.mapname);
	strcat(filename, ".nav");
#endif

	if ((fIn = fopen(filename, "rb")) == NULL) // See if .nav file exists
	{
		return; // No file
	}
	else
	{
		fileSize += sizeof(byte) * fread(&version, sizeof(byte), 1, fIn); // Bot nav version
		if (version < BOT_NAV_VERSION_1 || version > BOT_NAV_VERSION)
		{
			Com_Printf("%s ERROR: NAV file version mismatch. Got %d, expected value between %d and %d\n", __func__, version, BOT_NAV_VERSION_1, BOT_NAV_VERSION);
			fclose(fIn); // Close the file
			return;
		}
	}

	// Init Nodes
	BOTLIB_FreeNodes(); //Soft map change. Free any existing node memory used
	BOTLIB_InitNodes();
	BOTLIB_FreeAreaNodes(); //Soft map change. Free all area node memory used
	//memset(&nmesh, 0, sizeof(nmesh_t));

	// Remove doors
	Remove_All_Doors();
	Remove_All_Breakableglass();

	Com_Printf("%s Reading NAV file... detected version [%d]\n", __func__, version);

	// Read Compressed and uncompressed buffer sizes
	long uncompressed_buff_len = 0;
	fileSize += sizeof(int) * fread(&uncompressed_buff_len, sizeof(int), 1, fIn); // Uncompressed buffer size
	long compressed_buff_len = 0;
	fileSize += sizeof(int) * fread(&compressed_buff_len, sizeof(int), 1, fIn); // Compressed buffer size
	
	// Read compressed buffer
	char* uncompressed_buff = (char*)malloc(uncompressed_buff_len);
	char* compressed_buff = (char*)malloc(compressed_buff_len);
	if (compressed_buff != NULL && uncompressed_buff != NULL)
	{
		fileSize += compressed_buff_len * fread(compressed_buff, compressed_buff_len, 1, fIn); // Compressed buffer

		BOTLIB_DecompressBuffer(compressed_buff, compressed_buff_len, uncompressed_buff, &uncompressed_buff_len);
		//Com_Printf("%s compressed_buff_len:%i uncompressed_buff_len:%i\n", __func__, compressed_buff_len, uncompressed_buff_len);

		// Read the uncompressed buffer
		{
			int buff_read = 0;

			// Total nodes
			memcpy(&numnodes, uncompressed_buff + buff_read, sizeof(unsigned short));
			buff_read += sizeof(unsigned short);

			if (numnodes + 1 > MAX_PNODES)
			{
				Com_Printf("%s ERROR: Too many nodes in NAV file\n", __func__);
				return;
			}

			// Alloc nodes
			nodes = (node_t*)malloc(sizeof(node_t) * (numnodes + 1)); // Alloc memory
			if (nodes == NULL)
			{
				Com_Printf("%s failed to malloc nodes\n", __func__);
				return;
			}

			// Total areas
			//nav_area.total_areas = 0;

			//Com_Printf("%s buff_version:%i buff_numnodes:%i\n", __func__, version, numnodes);

			// Read node data
			for (n = 0; n < numnodes; n++)
			{
				if (version > BOT_NAV_VERSION_1)
				{
					// Area/chunk/group this node belongs to -- optimise and diversify bot pathing
					memcpy(&nodes[n].area, uncompressed_buff + buff_read, sizeof(int));					// Node area
					buff_read += sizeof(int);

					nodes[n].area_color = 0;
				}
				//VectorClear(nodes[n].normal);
				//if (version > BOT_NAV_VERSION_2)
				//{
					//memcpy(&nodes[n].normal, uncompressed_buff + buff_read, sizeof(vec3_t));			// Surface normal
					//buff_read += sizeof(vec3_t);
				//}

				nodes[n].weight = 0; // Used to help diversify bot pathing

				memcpy(&nodes[n].origin, uncompressed_buff + buff_read, sizeof(vec3_t));			// Location
				buff_read += sizeof(vec3_t);
				memcpy(&nodes[n].type, uncompressed_buff + buff_read, sizeof(byte));				// Node type
				buff_read += sizeof(byte);

				VectorCopy(mins, nodes[n].mins);		// Box mins
				VectorCopy(maxs, nodes[n].maxs);		// Box maxs
				if (nodes[n].type == NODE_CROUCH)
					nodes[n].maxs[2] = CROUCHING_MAXS2;
				else if (nodes[n].type == NODE_BOXJUMP)
				{
					nodes[n].mins[0] = -8;
					nodes[n].mins[1] = -8;
					nodes[n].mins[2] = -12;
					nodes[n].maxs[0] = 8;
					nodes[n].maxs[1] = 8;
					nodes[n].maxs[2] = 16;
				}
				VectorAdd(nodes[n].origin, mins, nodes[n].absmin); // Update absolute box min/max in the world
				VectorAdd(nodes[n].origin, maxs, nodes[n].absmax); // Update absolute box min/max in the world

				memcpy(&nodes[n].nodenum, uncompressed_buff + buff_read, sizeof(short int));		// Node number
				buff_read += sizeof(short int);
				memcpy(&nodes[n].inuse, uncompressed_buff + buff_read, sizeof(qboolean));			// Node inuse
				buff_read += sizeof(qboolean);

				// Init MAXLINKS links
				for (l = 0; l < MAXLINKS; l++)
				{
					nodes[n].links[l].targetNode = INVALID;							// Link
					nodes[n].links[l].targetNodeType = INVALID;						// Type
					nodes[n].links[l].cost = INVALID;								// Cost
				}

				// Read the num_links
				memcpy(&nodes[n].num_links, uncompressed_buff + buff_read, sizeof(byte));
				buff_read += sizeof(byte);
				//Com_Printf("%s origin:%f %f %f type:%i nodenum:%i inuse:%i num_links:%i\n", __func__, nodes[n].origin[0], nodes[n].origin[1], nodes[n].origin[2], nodes[n].type, nodes[n].nodenum, nodes[n].inuse, nodes[n].num_links);

				for (l = 0; l < nodes[n].num_links; l++) // 
				{
					memcpy(&nodes[n].links[l].targetNode, uncompressed_buff + buff_read, sizeof(short int));
					buff_read += sizeof(short int);
					memcpy(&nodes[n].links[l].targetNodeType, uncompressed_buff + buff_read, sizeof(byte));
					buff_read += sizeof(byte);
					memcpy(&nodes[n].links[l].cost, uncompressed_buff + buff_read, sizeof(float));
					buff_read += sizeof(float);
					//Com_Printf("%s targetNode:%i targetNodeType:%i cost:%f\n", __func__, nodes[n].links[l].targetNode, nodes[n].links[l].targetNodeType, nodes[n].links[l].cost);
				}

				// Paths
				//for (p = 0; p < numnodes; p++)
				//{
					//memcpy(&path_table[n][p], uncompressed_buff + buff_read, sizeof(short int)); // Path table
					//buff_read += sizeof(short int);
					//Com_Printf("%s path_table[%i][%i]:%i\n", __func__, n, p, path_table[n][p]);
				//}
			}
		}
	}

	// Free memory
	if (compressed_buff != NULL)
		free(compressed_buff);
	if (uncompressed_buff != NULL)
		free(uncompressed_buff);

	// Fix any dangling or unusual links
	if (numnodes)
	{
		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].inuse == false)
				BOTLIB_RemoveAllNodeLinksFrom(nodes[i].nodenum); // Remove all links to and from this node

			if (nodes[i].origin[0] == 0 && nodes[i].origin[1] == 0 && nodes[i].origin[2] == 0)
				BOTLIB_RemoveAllNodeLinksFrom(nodes[i].nodenum); // Remove all links to and from this node

			/*
			for (int l = 0; l < nodes[i].num_links; l++)
			{
				int tnode = nodes[i].links[l].targetNode;

				if (nodes[tnode].origin[0] == 0 && nodes[tnode].origin[1] == 0 && nodes[tnode].origin[2] == 0)
				{
					BOTLIB_RemoveAllNodeLinksFrom(nodes[tnode].nodenum); // Remove all links to and from this node
				}
			}
			*/
		}
	}

	fclose(fIn);

	Com_Printf("%s loaded %s containing %d nodes.\n", __func__, filename, numnodes);

	// Update old versions to new format
	if (version == BOT_NAV_VERSION_1)
	{
		for (int i = 0; i < numnodes; i++)
		{
			nodes[i].area = 0;
		}
	}
	// Save changes to file
	if (version < BOT_NAV_VERSION)
	{
		Com_Printf("%s Updating NAV file to new version [%d]\n", __func__, BOT_NAV_VERSION);
		BOTLIB_SaveNavCompressed();
	}

	//BOTLIB_SetAllNodeNormals(); // Set all the normals

	// Init Areas (must be done after nodes are loaded)
	if (numnodes)
	{
		BOTLIB_InitAreaNodes();
		BOTLIB_InitAreaConnections();
	}

	// Cache all the POI nodes
	if (numnodes)
	{
		num_poi_nodes = 0; // Reset any previous POI nodes

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].type == NODE_POI)
			{
				poi_nodes[num_poi_nodes] = i;
				num_poi_nodes++;
			}
		}
	}

}
#endif

void BOTLIB_SaveNav(void)
{
	FILE* fOut;
	char filename[128];
	int fileSize = 0;
	int f, n, l, p; // File, nodes, links, paths
	int version = BOT_NAV_VERSION;
	cvar_t* game_dir = gi.cvar("game", "action", 0);
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

#ifdef _WIN32
	f = sprintf(filename, ".\\");
	f += sprintf(filename + f, game_dir->string);
	f += sprintf(filename + f, "\\");
	f += sprintf(filename + f, botdir->string);
	f += sprintf(filename + f, "\\nav\\");
	f += sprintf(filename + f, level.mapname);
	f += sprintf(filename + f, ".nav");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/nav/");
	strcat(filename, level.mapname);
	strcat(filename, ".nav");
#endif

	if ((fOut = fopen(filename, "wb")) == NULL) // Open file to write binary
	{
		Com_Printf("%s Failed to write NAV to file %s\n", __func__, filename);
		return;
	}

	Com_Printf("%s Writing NAV to file...\n", __func__);

#if 0
#ifdef USE_ZLIB
	char* buff = NULL;
	int buff_len = 0;
	//BOTLIB_AddToBuffer((char*)&version, sizeof(int), &buff, &buff_len);
	BOTLIB_AddToBuffer((char*)&numnodes, sizeof(unsigned short), &buff, &buff_len);
	for (n = 0; n < numnodes; n++)
	{
		// Nodes
		BOTLIB_AddToBuffer((char*)&nodes[n].area, sizeof(int), &buff, &buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].origin, sizeof(vec3_t), &buff, &buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].type, sizeof(byte), &buff, &buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].nodenum, sizeof(short int), &buff, &buff_len);
		BOTLIB_AddToBuffer((char*)&nodes[n].inuse, sizeof(qboolean), &buff, &buff_len);

		// Links
		BOTLIB_AddToBuffer((char*)&nodes[n].num_links, sizeof(byte), &buff, &buff_len);
		for (l = 0; l < nodes[n].num_links; l++) // 
		{
			BOTLIB_AddToBuffer((char*)&nodes[n].links[l].targetNode, sizeof(short int), &buff, &buff_len);
			BOTLIB_AddToBuffer((char*)&nodes[n].links[l].targetNodeType, sizeof(byte), &buff, &buff_len);
			BOTLIB_AddToBuffer((char*)&nodes[n].links[l].cost, sizeof(float), &buff, &buff_len);
		}

		// Paths
		for (p = 0; p < numnodes; p++)
		{
			BOTLIB_AddToBuffer((char*)&path_table[n][p], sizeof(short int), &buff, &buff_len); // Path table			
		}
	}
	// Compress the buffer
	int compressed_buff_len = 0;
	char* compressed_buff = (char*)malloc(buff_len);
	if (compressed_buff != NULL)
	{
		//memset(compressed_buff, 0, buff_len);
		compressBuffer(buff, buff_len, compressed_buff, &compressed_buff_len);
		// Compare and print the size of the buffers
		Com_Printf("%s buff_len:%i compressed_buff_len:%i\n", __func__, buff_len, compressed_buff_len);

		int uncompressed_buff_len = buff_len;
		char* uncompressed_buff = (char*)malloc(buff_len);
		decompressBuffer(compressed_buff, compressed_buff_len, uncompressed_buff, &uncompressed_buff_len);
		Com_Printf("%s uncompressed_buff_len:%i\n", __func__, uncompressed_buff_len);

		if (uncompressed_buff != NULL)
		{
			// Read the uncompressed version
			int buff_read = 0;
			//int buff_version = 0;
			//memcpy(&buff_version, uncompressed_buff, sizeof(int));
			//buff_read += sizeof(int);
			// Read the buffer numnodes
			unsigned short buff_numnodes = 0;
			//memcpy(&buff_numnodes, buff + sizeof(int), sizeof(unsigned short));
			memcpy(&buff_numnodes, uncompressed_buff + buff_read, sizeof(unsigned short));
			buff_read += sizeof(unsigned short);
			//Com_Printf("%s buff_version:%i buff_numnodes:%i\n", __func__, buff_version, buff_numnodes);
			for (n = 0; n < buff_numnodes; n++)
			{
				// Read the origin
				vec3_t buff_origin = { 0 };
				memcpy(&buff_origin, uncompressed_buff + buff_read, sizeof(vec3_t));
				buff_read += sizeof(vec3_t);
				// Read the type
				byte buff_type = 0;
				memcpy(&buff_type, uncompressed_buff + buff_read, sizeof(byte));
				buff_read += sizeof(byte);
				// Read the nodenum
				short int buff_nodenum = 0;
				memcpy(&buff_nodenum, uncompressed_buff + buff_read, sizeof(short int));
				buff_read += sizeof(short int);
				// Read the inuse
				qboolean buff_inuse = false;
				memcpy(&buff_inuse, uncompressed_buff + buff_read, sizeof(qboolean));
				buff_read += sizeof(qboolean);
				// Read the num_links
				byte buff_num_links = 0;
				memcpy(&buff_num_links, uncompressed_buff + buff_read, sizeof(byte));
				buff_read += sizeof(byte);
				Com_Printf("%s buff_origin:%f %f %f buff_type:%i buff_nodenum:%i buff_inuse:%i buff_num_links:%i\n", __func__, buff_origin[0], buff_origin[1], buff_origin[2], buff_type, buff_nodenum, buff_inuse, buff_num_links);

				for (l = 0; l < buff_num_links; l++) // 
				{
					// Read the targetNode
					short int buff_targetNode = 0;
					memcpy(&buff_targetNode, uncompressed_buff + buff_read, sizeof(short int));
					buff_read += sizeof(short int);
					// Read the targetNodeType
					byte buff_targetNodeType = 0;
					memcpy(&buff_targetNodeType, uncompressed_buff + buff_read, sizeof(byte));
					buff_read += sizeof(byte);
					// Read the cost
					float buff_cost = 0;
					memcpy(&buff_cost, uncompressed_buff + buff_read, sizeof(float));
					buff_read += sizeof(float);
					Com_Printf("%s buff_targetNode:%i buff_targetNodeType:%i buff_cost:%f\n", __func__, buff_targetNode, buff_targetNodeType, buff_cost);
					break;
				}
				for (p = 0; p < buff_numnodes; p++)
				{
					// Read the path_table
					short int buff_path_table = 0;
					memcpy(&buff_path_table, uncompressed_buff + buff_read, sizeof(short int));
					buff_read += sizeof(short int);
					Com_Printf("%s buff_path_table:%i\n", __func__, buff_path_table);
					break;
				}

				break;
			}
		}


		if (1)
		{
			// Read the buffer version
			int buff_read = 0;
			//int buff_version = 0;
			//memcpy(&buff_version, buff, sizeof(int)); 
			//buff_read += sizeof(int);
			// Read the buffer numnodes
			unsigned short buff_numnodes = 0;
			//memcpy(&buff_numnodes, buff + sizeof(int), sizeof(unsigned short));
			memcpy(&buff_numnodes, buff + buff_read, sizeof(unsigned short)); 
			buff_read += sizeof(unsigned short);
			//Com_Printf("%s buff_version:%i buff_numnodes:%i\n", __func__, buff_version, buff_numnodes);
			for (n = 0; n < buff_numnodes; n++)
			{
				// Read the origin
				vec3_t buff_origin = { 0 };
				memcpy(&buff_origin, buff + buff_read, sizeof(vec3_t)); 
				buff_read += sizeof(vec3_t);
				// Read the type
				byte buff_type = 0;
				memcpy(&buff_type, buff + buff_read, sizeof(byte)); 
				buff_read += sizeof(byte);
				// Read the nodenum
				short int buff_nodenum = 0;
				memcpy(&buff_nodenum, buff + buff_read, sizeof(short int)); 
				buff_read += sizeof(short int);
				// Read the inuse
				qboolean buff_inuse = false;
				memcpy(&buff_inuse, buff + buff_read, sizeof(qboolean));
				buff_read += sizeof(qboolean);
				// Read the num_links
				byte buff_num_links = 0;
				memcpy(&buff_num_links, buff + buff_read, sizeof(byte));
				buff_read += sizeof(byte);
				Com_Printf("%s buff_origin:%f %f %f buff_type:%i buff_nodenum:%i buff_inuse:%i buff_num_links:%i\n", __func__, buff_origin[0], buff_origin[1], buff_origin[2], buff_type, buff_nodenum, buff_inuse, buff_num_links);
				
				for (l = 0; l < buff_num_links; l++) // 
				{
					// Read the targetNode
					short int buff_targetNode = 0;
					memcpy(&buff_targetNode, buff + buff_read, sizeof(short int));
					buff_read += sizeof(short int);
					// Read the targetNodeType
					byte buff_targetNodeType = 0;
					memcpy(&buff_targetNodeType, buff + buff_read, sizeof(byte));
					buff_read += sizeof(byte);
					// Read the cost
					float buff_cost = 0;
					memcpy(&buff_cost, buff + buff_read, sizeof(float));
					buff_read += sizeof(float);
					Com_Printf("%s buff_targetNode:%i buff_targetNodeType:%i buff_cost:%f\n", __func__, buff_targetNode, buff_targetNodeType, buff_cost);
					break;
				}
				for (p = 0; p < buff_numnodes; p++)
				{
					// Read the path_table
					short int buff_path_table = 0;
					memcpy(&buff_path_table, buff + buff_read, sizeof(short int));
					buff_read += sizeof(short int);
					Com_Printf("%s buff_path_table:%i\n", __func__, buff_path_table);
					break;
				}

				break;
			}
		}

	}
	else
	{
		Com_Printf("%s Failed to allocate memory for compressed buffer\n", __func__);
	}

	if (buff != NULL)
		free(buff);

	if (compressed_buff != NULL)
		free(compressed_buff);

#endif
#endif

	// Write general data
	fileSize += sizeof(byte) * fwrite(&version, sizeof(byte), 1, fOut);									// Version
	//fileSize += sizeof(unsigned) * fwrite(&nmesh.bsp_checksum, sizeof(unsigned), 1, fOut);				// Map checksum

	// Write node data
	fileSize += sizeof(unsigned short) * fwrite(&numnodes, sizeof(unsigned short), 1, fOut);			// Total Nodes
	for (n = 0; n < numnodes; n++)
	{
		// Nodes
		fileSize += sizeof(vec3_t) * fwrite(&nodes[n].origin, sizeof(vec3_t), 1, fOut);				// Node origin
		fileSize += sizeof(byte) * fwrite(&nodes[n].type, sizeof(byte), 1, fOut);					// Node type
		fileSize += sizeof(short int) * fwrite(&nodes[n].nodenum, sizeof(short int), 1, fOut);		// Node number
		fileSize += sizeof(qboolean) * fwrite(&nodes[n].inuse, sizeof(qboolean), 1, fOut);			// Node inuse

		// Links
		fileSize += sizeof(byte) * fwrite(&nodes[n].num_links, sizeof(byte), 1, fOut);				// Total node links
		if (nodes[n].num_links > MAXLINKS) Com_Printf("WARNING: Node %d has %d links, only %d will be saved\n", n, nodes[n].num_links, MAXLINKS);
		for (l = 0; l < nodes[n].num_links; l++) // 
		{
			fileSize += sizeof(short int) * fwrite(&nodes[n].links[l].targetNode, sizeof(short int), 1, fOut);		// Link target node
			fileSize += sizeof(byte) * fwrite(&nodes[n].links[l].targetNodeType, sizeof(byte), 1, fOut);			// Link target type
			fileSize += sizeof(float) * fwrite(&nodes[n].links[l].cost, sizeof(float), 1, fOut);					// Link cost
		}

		// Paths
		for (p = 0; p < numnodes; p++)
		{
			fileSize += sizeof(short int) * fwrite(&path_table[n][p], sizeof(short int), 1, fOut); // Path table			
		}
	}

	// Cache all the POI nodes
	if (numnodes)
	{
		num_poi_nodes = 0; // Reset any previous POI nodes

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].type == NODE_POI)
			{
				poi_nodes[num_poi_nodes] = i;
				num_poi_nodes++;
			}
		}
	}


	fclose(fOut);

	Com_Printf("%s Saved %s [%i bytes] to disk\n", __func__, filename, fileSize);
}

void BOTLIB_LoadNav(void)
{
	FILE* fIn;
	char filename[128];
	int fileSize = 0;
	int f, n, l, p; // File, nodes, links, paths
	int version = 0; // Bot nav version
	unsigned bsp_checksum = 0; // Map checksum
	cvar_t* game_dir = gi.cvar("game", "action", 0);
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib
	const vec3_t mins = { -16, -16, -24 };
	const vec3_t maxs = { 16, 16, 32 };

#ifdef _WIN32
	f = sprintf(filename, ".\\");
	f += sprintf(filename + f, game_dir->string);
	f += sprintf(filename + f, "\\");
	f += sprintf(filename + f, botdir->string);
	f += sprintf(filename + f, "\\nav\\");
	f += sprintf(filename + f, level.mapname);
	f += sprintf(filename + f, ".nav");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/nav/");
	strcat(filename, level.mapname);
	strcat(filename, ".nav");
#endif

	if ((fIn = fopen(filename, "rb")) == NULL) // See if .nav file exists
	{
		return; // No file
	}
	else
	{

		fileSize += sizeof(byte) * fread(&version, sizeof(byte), 1, fIn); // Bot nav version
		if (version != BOT_NAV_VERSION)
		{
			fclose(fIn); // Close the file
			return;
		}
	}

	// Init
	BOTLIB_FreeNodes(); //Soft map change. Free any existing node memory used
	BOTLIB_InitNodes();
	BOTLIB_FreeAreaNodes(); //Soft map change. Free all area node memory used
	//memset(&nmesh, 0, sizeof(nmesh_t));

	// Remove doors
	Remove_All_Doors();
	Remove_All_Breakableglass();

	Com_Printf("%s Reading NAV file...\n", __func__);

	fileSize += sizeof(unsigned short) * fread(&numnodes, sizeof(unsigned short), 1, fIn);			// Total Nodes

	if (numnodes + 1 > MAX_PNODES)
	{
		Com_Printf("%s ERROR: Too many nodes in NAV file\n", __func__);
		return;
	}

	// Main nodes
	nodes = (node_t*)malloc(sizeof(node_t) * (numnodes + 1)); // Alloc memory
	if (nodes == NULL)
	{
		Com_Printf("%s failed to malloc nodes\n", __func__);
		return;
	}

	// Read node data
	for (n = 0; n < numnodes; n++)
	{
		nodes[n].weight = 0; // Used to help diversify bot pathing

		// Nodes
		fileSize += sizeof(int) * fread(&nodes[n].area, sizeof(int), 1, fIn);				// Area
		fileSize += sizeof(vec3_t) * fread(&nodes[n].origin, sizeof(vec3_t), 1, fIn);		// Location
		fileSize += sizeof(byte) * fread(&nodes[n].type, sizeof(byte), 1, fIn);				// Node type

		VectorClear(nodes[n].normal);			// Surface normal
		VectorCopy(mins, nodes[n].mins);		// Box mins
		VectorCopy(maxs, nodes[n].maxs);		// Box maxs
		if (nodes[n].type == NODE_CROUCH)
			nodes[n].maxs[2] = CROUCHING_MAXS2;
		else if (nodes[n].type == NODE_BOXJUMP)
		{
			nodes[n].mins[0] = -8;
			nodes[n].mins[1] = -8;
			nodes[n].mins[2] = -12;
			nodes[n].maxs[0] = 8;
			nodes[n].maxs[1] = 8;
			nodes[n].maxs[2] = 16;
		}
		VectorAdd(nodes[n].origin, mins, nodes[n].absmin); // Update absolute box min/max in the world
		VectorAdd(nodes[n].origin, maxs, nodes[n].absmax); // Update absolute box min/max in the world

		nodes[n].weight = 0; // Used to help diversify bot pathing

		fileSize += sizeof(unsigned short) * fread(&nodes[n].nodenum, sizeof(unsigned short), 1, fIn);	// Node number
		fileSize += sizeof(qboolean) * fread(&nodes[n].inuse, sizeof(qboolean), 1, fIn);	// Node inuse

		// Init MAXLINKS links
		for (l = 0; l < MAXLINKS; l++)
		{
			nodes[n].links[l].targetNode = INVALID;							// Link
			nodes[n].links[l].targetNodeType = INVALID;						// Type
			nodes[n].links[l].cost = INVALID;								// Cost
		}

		// Copy num_links links
		fileSize += sizeof(byte) * fread(&nodes[n].num_links, sizeof(byte), 1, fIn);							// Total node links
		for (l = 0; l < nodes[n].num_links; l++)
		{
			fileSize += sizeof(short int) * fread(&nodes[n].links[l].targetNode, sizeof(short int), 1, fIn);	// Link target node
			fileSize += sizeof(byte) * fread(&nodes[n].links[l].targetNodeType, sizeof(byte), 1, fIn);			// Link target type
			fileSize += sizeof(float) * fread(&nodes[n].links[l].cost, sizeof(float), 1, fIn);					// Link cost
		}

		// Paths
		for (p = 0; p < numnodes; p++)
		{
			fileSize += sizeof(short int) * fread(&path_table[n][p], sizeof(short int), 1, fIn); // Path table
		}
	}

	// Init Areas (must be done after nodes are loaded)
	if (numnodes)
	{
		BOTLIB_InitAreaNodes();
		BOTLIB_InitAreaConnections();
	}

	// Cache all the POI nodes
	if (numnodes)
	{
		num_poi_nodes = 0; // Reset any previous POI nodes

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].type == NODE_POI)
			{
				poi_nodes[num_poi_nodes] = i;
				num_poi_nodes++;
			}
		}
	}

	fclose(fIn);

	Com_Printf("%s Loaded %s [%i bytes] from disk\n", __func__, filename, fileSize);
}

//rekkie -- walknodes -- e

///////////////////////////////////////////////////////////////////////
// Save to disk file
///////////////////////////////////////////////////////////////////////
void ACEND_SaveAAS(qboolean update)
{
	FILE* fOut;
	char filename[128];
	int fileSize = 0;
	int f, v, e, t, r;	// BSP: Faces, verts, edges, tris, reach
	int n, l, p; // Nodes, links, paths
	int version = BOT_AAS_VERSION;
	cvar_t *game_dir = gi.cvar("game", "action", 0);
	cvar_t *botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

#ifdef _WIN32
	f = sprintf(filename, ".\\");
	f += sprintf(filename + f, game_dir->string);
	f += sprintf(filename + f, "\\");
	f += sprintf(filename + f, botdir->string);
	f += sprintf(filename + f, "\\nav\\");
	f += sprintf(filename + f, level.mapname);
	f += sprintf(filename + f, ".aas");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/nav/");
	strcat(filename, level.mapname);
	strcat(filename, ".aas");
#endif

	// If not updating, process from scratch
	if (update == false)
	{
		// Proccess BSP data so we can save it to disk
		ACEND_BSP(NULL);
	}

	if ((fOut = fopen(filename, "wb")) == NULL) // Open file to write binary
	{
		Com_Printf("%s Failed to write AAS to file %s\n", __func__, filename);
		return;
	}

	Com_Printf("%s Writing AAS to file...\n", __func__);

	// Write general data
	fileSize += sizeof(byte) * fwrite(&version, sizeof(byte), 1, fOut);									// AAS Version
	fileSize += sizeof(unsigned) * fwrite(&nmesh.bsp_checksum, sizeof(unsigned), 1, fOut);				// Map checksum
	fileSize += sizeof(unsigned short) * fwrite(&nmesh.total_faces, sizeof(unsigned short), 1, fOut);	// Total Faces
	fileSize += sizeof(unsigned short) * fwrite(&nmesh.total_edges, sizeof(unsigned short), 1, fOut);	// Total Edges
	fileSize += sizeof(unsigned short) * fwrite(&nmesh.total_verts, sizeof(unsigned short), 1, fOut);	// Total Verticies
	fileSize += sizeof(unsigned short) * fwrite(&nmesh.total_tris, sizeof(unsigned short), 1, fOut);	// Total Triangles
	fileSize += sizeof(int) * fwrite(&nmesh.total_reach, sizeof(int), 1, fOut);							// Total Reachabilities
	fileSize += sizeof(unsigned short) * fwrite(&numnodes, sizeof(unsigned short), 1, fOut);			// Total Nodes

	// Write BSP Data
	for (f = 0; f < nmesh.total_faces; f++)
	{
		// Write face verts
		fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].num_verts, sizeof(unsigned short), 1, fOut);
		for (v = 0; v < nmesh.face[f].num_verts; v++)
		{
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].v[v], sizeof(vec3_t), 1, fOut);
		}

		// Write edges
		fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].num_edges, sizeof(unsigned short), 1, fOut);
		for (e = 0; e < nmesh.face[f].num_edges; e++)
		{
			// Write edge verts
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].edge[e].v[0], sizeof(vec3_t), 1, fOut);
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].edge[e].v[1], sizeof(vec3_t), 1, fOut);

			// Write edge center
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].edge[e].center, sizeof(vec3_t), 1, fOut);

			// Write edge node
			fileSize += sizeof(short int) * fwrite(&nmesh.face[f].edge[e].node, sizeof(short int), 1, fOut);
		}

		// Write triangles
		fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].num_tris, sizeof(unsigned short), 1, fOut);
		for (t = 0; t < nmesh.face[f].num_tris; t++)
		{
			// Write triangle verts
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].tris[t].v[0], sizeof(vec3_t), 1, fOut);
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].tris[t].v[1], sizeof(vec3_t), 1, fOut);
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].tris[t].v[2], sizeof(vec3_t), 1, fOut);

			// Write triangle center
			fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].tris[t].center, sizeof(vec3_t), 1, fOut);

			// Write triangle face
			fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].tris[t].face, sizeof(unsigned short), 1, fOut);

			// Write triangle node
			fileSize += sizeof(short int) * fwrite(&nmesh.face[f].tris[t].node, sizeof(short int), 1, fOut);

			// Write triangle reachabilities
			fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].tris[t].reach_num, sizeof(unsigned short), 1, fOut);
			for (r = 0; r < nmesh.face[f].tris[t].reach_num; r++)
			{
				fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].tris[t].reach_triangle[r], sizeof(unsigned short), 1, fOut);
				fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].tris[t].reach_origin[r], sizeof(vec3_t), 1, fOut);
				fileSize += sizeof(unsigned short) * fwrite(&nmesh.face[f].tris[t].reach_face[r], sizeof(unsigned short), 1, fOut);
			}
		}

		fileSize += sizeof(byte) * fwrite(&nmesh.face[f].type, sizeof(byte), 1, fOut);			// Surface type
		fileSize += sizeof(int) * fwrite(&nmesh.face[f].drawflags, sizeof(int), 1, fOut);		// Surface flags
		fileSize += sizeof(int) * fwrite(&nmesh.face[f].contents, sizeof(int), 1, fOut);		// Surface contents
		fileSize += sizeof(vec3_t) * fwrite(&nmesh.face[f].normal, sizeof(vec3_t), 1, fOut);	// Surface normal
	}


	// Write node data
	for (n = 0; n < numnodes; n++)
	{
		// Nodes
		fileSize += sizeof(vec3_t) * fwrite(&nodes[n].origin, sizeof(vec3_t), 1, fOut);				// Location
		fileSize += sizeof(vec3_t) * fwrite(&nodes[n].normal, sizeof(vec3_t), 1, fOut);				// Surface normal
		fileSize += sizeof(byte) * fwrite(&nodes[n].type, sizeof(byte), 1, fOut);					// Node type
		//fileSize += sizeof(short int) * fwrite(&nodes[n].nodenum, sizeof(short int), 1, fOut);	// Node number
		fileSize += sizeof(qboolean) * fwrite(&nodes[n].inuse, sizeof(qboolean), 1, fOut);			// Node inuse


		// Links
		fileSize += sizeof(byte) * fwrite(&nodes[n].num_links, sizeof(byte), 1, fOut);		// Total node links
		if (nodes[n].num_links > MAXLINKS) Com_Printf("WARNING: Node %d has %d links, only %d will be saved\n", n, nodes[n].num_links, MAXLINKS);
		for (l = 0; l < MAXLINKS; l++) // Include all links, even invalid ones
		//for (l = 0; l < nodes[numnodes].num_links; l++) // Only include valid links
		{
			fileSize += sizeof(short int) * fwrite(&nodes[n].links[l].targetNode, sizeof(short int), 1, fOut);		// Link target node
			fileSize += sizeof(byte) * fwrite(&nodes[n].links[l].targetNodeType, sizeof(byte), 1, fOut);			// Link target type
			fileSize += sizeof(float) * fwrite(&nodes[n].links[l].cost, sizeof(float), 1, fOut);					// Link cost
		}

		// Paths
		for (p = 0; p < numnodes; p++)
		{
			fileSize += sizeof(short int) * fwrite(&path_table[n][p], sizeof(short int), 1, fOut); // Path table			
		}
	}


	fclose(fOut);

	Com_Printf("%s Saved %s [%i bytes] to disk\n", __func__, filename, fileSize);
}

///////////////////////////////////////////////////////////////////////
// Load Area Awareness System (from file or generative)
// Parameter: force - generates a new AAS instead of loading from file
///////////////////////////////////////////////////////////////////////
void ACEND_LoadAAS(qboolean force)
{
	FILE* fIn;
	char filename[128];
	int fileSize = 0;
	qboolean generateAAS = false;
	int f, v, e, t, r;	// BSP: Faces, verts, edges, tris, reach
	int n, l, p; // Nodes, links, paths
	int version = 0; // Bot nav version
	unsigned bsp_checksum = 0; // Map checksum
	cvar_t *game_dir = gi.cvar("game", "action", 0);
	cvar_t *botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

#ifdef _WIN32
	f = sprintf(filename, ".\\");
	f += sprintf(filename + f, game_dir->string);
	f += sprintf(filename + f, "\\");
	f += sprintf(filename + f, botdir->string);
	f += sprintf(filename + f, "\\nav\\");
	f += sprintf(filename + f, level.mapname);
	f += sprintf(filename + f, ".aas");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/nav/");
	strcat(filename, level.mapname);
	strcat(filename, ".aas");
#endif

	if ((fIn = fopen(filename, "rb")) == NULL) // See if AAS file exists
	{
		generateAAS = true; // Generate new AAS
	}
	else
	{
		fileSize += sizeof(byte) * fread(&version, sizeof(byte), 1, fIn); // Bot nav version
		if (version != BOT_AAS_VERSION)
		{
			generateAAS = true; // Generate new AAS
			fclose(fIn); // Close the file
		}

		rektek_bots_api_v1_t *bsp_api = gex.GetExtension(REKTEK_BOTS_API_V1);
		bsp_t* bsp = bsp_api->Bsp();
		fileSize += sizeof(unsigned) * fread(&bsp_checksum, sizeof(unsigned), 1, fIn); // Map checksum
		if (bsp_checksum != bsp->checksum)
		{
			generateAAS = true; // Generate new AAS
			fclose(fIn); // Close the file
		}

	}

	if (force)
		generateAAS = true; // Force: generate a new AAS instead of loading from file

	// Generate a new AAS?
	if (generateAAS)
	{
		Com_Printf("%s Generating botnav. Please wait this might take a while\n", __func__);
		ACEND_SaveAAS(false);
		return;
	}

	// Init
	BOTLIB_FreeNodes(); //Soft map change. Free any existing node memory used
	BOTLIB_InitNodes();
	BOTLIB_FreeAreaNodes(); //Soft map change. Free all area node memory used
	memset(&nmesh, 0, sizeof(nmesh_t));

	// Remove doors
	Remove_All_Doors();

	Com_Printf("%s Reading AAS file...\n", __func__);

	fileSize += sizeof(unsigned short) * fread(&nmesh.total_faces, sizeof(unsigned short), 1, fIn);	// Total Faces
	fileSize += sizeof(unsigned short) * fread(&nmesh.total_edges, sizeof(unsigned short), 1, fIn);	// Total Edges
	fileSize += sizeof(unsigned short) * fread(&nmesh.total_verts, sizeof(unsigned short), 1, fIn);	// Total Verticies
	fileSize += sizeof(unsigned short) * fread(&nmesh.total_tris, sizeof(unsigned short), 1, fIn);	// Total Triangles
	fileSize += sizeof(int) * fread(&nmesh.total_reach, sizeof(int), 1, fIn);						// Total Reachabilities
	fileSize += sizeof(unsigned short) * fread(&numnodes, sizeof(unsigned short), 1, fIn);			// Total Nodes
	
	if (numnodes + 1 > MAX_PNODES)
	{
		Com_Printf("%s ERROR: Too many nodes in AAS file\n", __func__);
		return;
	}

	// Read BSP Data
	for (f = 0; f < nmesh.total_faces; f++)
	{
		// Read face verts
		fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].num_verts, sizeof(unsigned short), 1, fIn);
		for (v = 0; v < nmesh.face[f].num_verts; v++)
		{
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].v[v], sizeof(vec3_t), 1, fIn);
		}

		// Read edges
		fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].num_edges, sizeof(unsigned short), 1, fIn);
		for (e = 0; e < nmesh.face[f].num_edges; e++)
		{
			// Read edge verts
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].edge[e].v[0], sizeof(vec3_t), 1, fIn);
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].edge[e].v[1], sizeof(vec3_t), 1, fIn);

			// Read edge center
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].edge[e].center, sizeof(vec3_t), 1, fIn);

			// Read edge node
			fileSize += sizeof(short int) * fread(&nmesh.face[f].edge[e].node, sizeof(short int), 1, fIn);
		}

		// Read triangles
		fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].num_tris, sizeof(unsigned short), 1, fIn);
		for (t = 0; t < nmesh.face[f].num_tris; t++)
		{
			// Read triangle verts
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].tris[t].v[0], sizeof(vec3_t), 1, fIn);
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].tris[t].v[1], sizeof(vec3_t), 1, fIn);
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].tris[t].v[2], sizeof(vec3_t), 1, fIn);

			// Read triangle center
			fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].tris[t].center, sizeof(vec3_t), 1, fIn);

			// Read triangle face
			fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].tris[t].face, sizeof(unsigned short), 1, fIn);

			// Read triangle node
			fileSize += sizeof(short int) * fread(&nmesh.face[f].tris[t].node, sizeof(short int), 1, fIn);

			// Read triangle reachabilities
			fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].tris[t].reach_num, sizeof(unsigned short), 1, fIn);
			for (r = 0; r < nmesh.face[f].tris[t].reach_num; r++)
			{
				fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].tris[t].reach_triangle[r], sizeof(unsigned short), 1, fIn);
				fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].tris[t].reach_origin[r], sizeof(vec3_t), 1, fIn);
				fileSize += sizeof(unsigned short) * fread(&nmesh.face[f].tris[t].reach_face[r], sizeof(unsigned short), 1, fIn);
			}
		}

		fileSize += sizeof(byte) * fread(&nmesh.face[f].type, sizeof(byte), 1, fIn);		// Surface type
		fileSize += sizeof(int) * fread(&nmesh.face[f].drawflags, sizeof(int), 1, fIn);		// Surface flags
		fileSize += sizeof(int) * fread(&nmesh.face[f].contents, sizeof(int), 1, fIn);		// Surface contents
		fileSize += sizeof(vec3_t) * fread(&nmesh.face[f].normal, sizeof(vec3_t), 1, fIn);	// Surface normal
	}


	// Main nodes
	nodes = (node_t*)malloc(sizeof(node_t) * (numnodes + 1)); // Alloc memory
	if (nodes == NULL)
	{
		Com_Printf("%s failed to malloc nodes\n", __func__);
		return;
	}

	/*
	// Path table
	path_table = (short int**)malloc(sizeof(short int*) * MAX_PNODES);
	if (path_table == NULL)
	{
		Com_Printf("%s failed to malloc path_table[]\n", __func__);
		return;
	}
	else
	{
		// Dynamically allocate memory of size MAX_PNODES for each row
		for (int r = 0; r < numnodes; r++)
		{
			path_table[r] = (short int*)malloc(sizeof(short int) * MAX_PNODES);
			if (path_table[r] == NULL)
			{
				Com_Printf("%s failed to malloc path_table[][]\n", __func__);
				return;
			}
		}
	}

	Com_Printf("%s allocing MBytes[%ld] for nodes\n", __func__, ((sizeof(node_t) * numnodes) / 1024000));
	Com_Printf("%s allocing MBytes[%ld] for path_table\n", __func__, ((sizeof(short int) * MAX_PNODES * MAX_PNODES) / 1024000));
	*/


	// Read node data
	for (n = 0; n < numnodes; n++)
	{	
		// Nodes
		fileSize += sizeof(vec3_t) * fread(&nodes[n].origin, sizeof(vec3_t), 1, fIn);		// Location
		fileSize += sizeof(vec3_t) * fread(&nodes[n].normal, sizeof(vec3_t), 1, fIn);		// Surface normal
		fileSize += sizeof(byte) * fread(&nodes[n].type, sizeof(byte), 1, fIn);				// Node type
		//fileSize += sizeof(unsigned short) * fread(&nodes[n].nodenum, sizeof(unsigned short), 1, fIn);	// Node number
		fileSize += sizeof(qboolean) * fread(&nodes[n].inuse, sizeof(qboolean), 1, fIn);	// Node inuse
		nodes[n].nodenum = n; // Node number

		for (l = 0; l < MAXLINKS; l++) // Include all links, even invalid ones
		{
			nodes[n].links[l].targetNode = INVALID;							// Link
			nodes[n].links[l].targetNodeType = INVALID;						// Type
			nodes[n].links[l].cost = INVALID;								// Cost
		}

		// Links
		fileSize += sizeof(byte) * fread(&nodes[n].num_links, sizeof(byte), 1, fIn);	// Total node links
		if (nodes[n].num_links > MAXLINKS) Com_Printf("%s node[%d] has more than MAXLINKS[%d] links\n", __func__, n, MAXLINKS);
		//else Com_Printf("%s node[%d] has links[%d]\n", __func__, n, nodes[n].num_links);
		//for (l = 0; l < nodes[numnodes].num_links; l++) // Include all links, even invalid ones
		for (l = 0; l < MAXLINKS; l++) // Include all links, even invalid ones
		{
			fileSize += sizeof(short int) * fread(&nodes[n].links[l].targetNode, sizeof(short int), 1, fIn);	// Link target node
			fileSize += sizeof(byte) * fread(&nodes[n].links[l].targetNodeType, sizeof(byte), 1, fIn);			// Link target type
			fileSize += sizeof(float) * fread(&nodes[n].links[l].cost, sizeof(float), 1, fIn);					// Link cost
		}

		// Paths
		for (p = 0; p < numnodes; p++)
		{
			fileSize += sizeof(short int) * fread(&path_table[n][p], sizeof(short int), 1, fIn); // Path table
		}
	}



	/*
	fread(&num_items, sizeof(int), 1, fIn); // read facts count
	fread(nodes, sizeof(node_t), numnodes, fIn);
	fread(item_table, sizeof(item_table_t), num_items, fIn);
	// Raptor007: Do not trust saved pointers!
	for (i = 0; i < MAX_EDICTS; i++)
		item_table[i].ent = NULL;
	*/

	fclose(fIn);

	Com_Printf("%s Loaded %s [%i bytes] from disk\n", __func__, filename, fileSize);

	/*
	// Try placing POI nodes on high locations
	if (1)
	{
		float highest_origin = -9999;
		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].origin[2] > highest_origin)
				highest_origin = nodes[i].origin[2];
		}
		float above_height = (highest_origin - 256);

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].type == NODE_MOVE && nodes[i].origin[2] > above_height && random() < 0.1)
			{
				nodes[i].type = NODE_POI;
			}
		}

		ACEND_CachePointOfInterestNodes();
	}

	ACEND_BuildVisibilityNodes(); // Rebuild node vis -- generate LOS nodes to target enemies
	*/
}

// Find the reachability for each node
void BOTLIB_NODE_REACHABILITY(edict_t* ent)
{
	node_t* from_node;	// From node
	node_t* to_node;	// To node
	int s, result;
	vec3_t zero = { 0 };
	//qboolean found_ladder;
	for (s = 0; s < numnodes; s++)
	{
		if (nodes[s].inuse == false) continue; // Ignore nodes not in use
		
		//if (nodes[s].nodenum != INVALID) continue;

		// Get the next 'from' node
		from_node = (unsorted_nodes + s); // From Node

		// Zero the link counter
		from_node->num_links = 0;

		for (int o = 0; o < numnodes; o++)
		{
			if (nodes[o].inuse == false) continue; // Ignore nodes not in use
			//if (nodes[o].nodenum != INVALID) continue;

			if (s == o) // skip self
				continue;

			to_node = (unsorted_nodes + o); // Node A

			result = DC_Reachability(from_node->nodenum, to_node->nodenum, from_node->origin, to_node->origin, from_node->normal);

			//TEST CODE -- s
			//if (result != NODE_MOVE && result != NODE_STEP) // Force only MOVE nodes
			//	result = INVALID;
			//if (from_node->nodenum == 1740 && to_node->nodenum == 1403)
			//	result = NODE_JUMPPAD;
			//TEST CODE -- e

			if (result != INVALID)
			{
				for (int lk = 0; lk < MAXLINKS; lk++)
				{
					if (from_node->links[lk].targetNode != o && from_node->links[lk].targetNode == INVALID) // Find a spare target link
					{
						from_node->links[lk].targetNodeType = result; // Set target node type

						// Add link
						//DC_AddNodeLink(unsorted_nodes, s, o, zero, false, result, true);
						//break;


						//DC_AddNodeLink(NULL, s_node->nodenum, a_node->nodenum, zero, false, s_node->links[lk1].targetNodeType, true); // Copy the sorted nodes
						if (BOTLIB_AddNodeLink(s, o, result, true))
						{
							ACEND_UpdateNodeReach(from_node->nodenum, to_node->nodenum); // Update path_table
							//from_node->num_links++; // Update number of links this node has
							nmesh.total_reach++;
						}
						break;
					}
				}
			}
		}
	}
}

// Similar to gi.pointcontents, but returns the contents within the bounding box around the origin
// Useful when checking for multiple contents within the local area, specially when checking water
int BOTLIB_UTIL_BOXCONTENTS(vec3_t origin)
{
	trace_t tr = gi.trace(origin, tv(-0.1, -0.1, -0.1), tv(0.1, 0.1, 0.1), origin, NULL, MASK_ALL);
	return tr.contents;
}

// Rotate the center of an edge around a center point, then extend the center point out by a distance
// 
//            (end) <--- desired result
//              |
//              |
//              |   <--- distance
//              |\
//              | \ <-- rotated edge (90 degrees)
//              |  \
//   (pt1) -----+----- (pt2)
//           (center)
//
// pt1 and pt2 are the start and end points of an edge
// float rotate the degrees of rotation
// float distance is the distance from the center point
// vec3_t end is the rotated end point
void BOTLIB_UTIL_ROTATE_CENTER(vec3_t pt1, vec3_t pt2, float rotate, float distance, vec3_t end)
{
	vec3_t center, angle, forward; // , right, offset;

	// print warning if distance is > 3000
	if (distance > 3000)
	{
		Com_Printf("%s WARNING: distance > 3000. Cannot rotate accurately.\n", __func__);
	}

	// Find the middle between two points of an edge
	LerpVector(pt1, pt2, 0.50, center);

	// Get direction vector
	VectorSubtract(pt2, pt1, forward);

	// Normalize the direction vector
	VectorNormalize(forward);

	// Get the angle of the direction vector
	vectoangles(forward, angle);

	//Com_Printf("%s vectoangles()->angle[YAW] = %f\n", __func__, angle[YAW]);

	//if (direction == MOVE_LEFT)
	//	angles[1] += 90;
	//else if (direction == MOVE_RIGHT)
	//	angles[1] -= 90;
	//else if (direction == MOVE_BACK)
	//	angles[1] -= 180;

	// Rotation
	//angle[PITCH] = 0;
	angle[YAW] += rotate; // Rotation angle
	//angle[ROLL] = 0;

	// Wrap around
	//if (angle[YAW] >= 180)
	//	angle[YAW] = angle[YAW] - 360;
	//else if (angle[YAW] <= -180)
	//	angle[YAW] = angle[YAW] + 360;

	//if (angle[YAW] > 360)
	//	angle[YAW] -= 360;
	//else if (angle[YAW] < 0)
	//	angle[YAW] += 360;

	// Rotate the direction vector
	AngleVectors(angle, forward, NULL, NULL);

	//VectorSet(offset, distance, 0, 0);
	//G_ProjectSource(center, offset, forward, right, end);

	// Get the rotated end point and extend it out based on distance
	VectorMA(center, distance, forward, end);
	//VectorMA(center, distance, right, end);
}

// Test for a 'trigger_hurt' entity
// Returns true if we're touching a 'trigger_hurt', otherwise false
qboolean BOTLIB_UTIL_CHECK_FOR_HURT(vec3_t origin)
{
	trace_t tr;

	// Test point contents from -20 to -30 checking for lava or slime below the proposed origin
	int contents;
	for (int i = 0; i < 10; i++)
	{
		contents = gi.pointcontents(tv(origin[0], origin[1], origin[2] - (20+i) ));
		if (contents & (CONTENTS_LAVA | CONTENTS_SLIME))
			return true;
	}

	// Trace above center to see if we're touching a 'trigger_hurt' designed to kill a player who touches it
	tr = gi.trace(tv(origin[0], origin[1], origin[2]), NULL, NULL, tv(origin[0], origin[1], origin[2] + NODE_Z_HEIGHT), g_edicts, MASK_ALL);
	if (tr.ent && tr.ent != g_edicts) // is ent and not worldspawn (g_edicts[0] = worldspawn)
	{
		//Com_Printf("%s face:%i tris:%i ent:%s\n", __func__, f, nmesh.face[f].num_tris, tr.ent->classname);
		if (strcmp(tr.ent->classname, "trigger_hurt") == 0 || tr.ent->dmg)
		{
			//Com_Printf("%s +SKIPPING %s\n", __func__, tr.ent->classname);
			return true;
		}

		if (tr.contents & (CONTENTS_LAVA | CONTENTS_SLIME))
			return true;
	}
	// Skip if we started inside of a solid
	if (tr.startsolid)
	{
		/*
		if (nmesh.face[f].drawflags)
		{
			if (nmesh.face[f].drawflags & SURF_SKY)
				Com_Printf("%s START SOLID, drawflags: SURF_SKY\n", __func__, nmesh.face[f].drawflags);
			else
				Com_Printf("%s START SOLID, drawflags: %d\n", __func__, nmesh.face[f].drawflags);
		}
		*/

		return true;
	}
	// Trace below center to see if we're touching a 'trigger_hurt' designed to kill a player who touches it
	tr = gi.trace(tv(origin[0], origin[1], origin[2]), NULL, NULL, tv(origin[0], origin[1], origin[2] - NODE_Z_HEIGHT), g_edicts, MASK_ALL);
	if (tr.ent && tr.ent != g_edicts) // is ent and not worldspawn (g_edicts[0] = worldspawn)
	{
		//Com_Printf("%s face:%i tris:%i ent:%s\n", __func__, f, nmesh.face[f].num_tris, tr.ent->classname);
		if (strcmp(tr.ent->classname, "trigger_hurt") == 0 || tr.ent->dmg)
		{
			//Com_Printf("%s -SKIPPING %s\n", __func__, tr.ent->classname);
			return true;
		}
		if (tr.contents & (CONTENTS_LAVA | CONTENTS_SLIME))
			return true;
	}

	return false;
}

// Turn selective map entities into solids, so we can trace them
// Used for ents like 'trigger_hurt' so we can avoid placing nodes on those surfaces
//solid_t	trigger_solid[MAX_EDICTS];
// make trigger_solid a malloc
int BOTLIB_MakeEntsSolid(solid_t* trigger_solid)
{
	if (trigger_solid == NULL)
	{
		return -1;
	}

	int edict_num = 1 + game.maxclients; // skip worldclass & players
	//int edict_cur = edict_num;
	int edict_cur = 0;
	edict_t* map_ents;
	for (map_ents = g_edicts + edict_num; map_ents < &g_edicts[globals.num_edicts]; map_ents++)
	{
		if (map_ents->inuse == false)
		{
			//edict_cur++;
			continue;
		}

		trigger_solid[edict_cur] = map_ents->solid; // keep a copy of the solid state of each ent
		/*
			SOLID_NOT,          // no interaction with other objects
			SOLID_TRIGGER,      // only touch when inside, after moving
			SOLID_BBOX,         // touch on edge
			SOLID_BSP           // bsp clip, touch on edge
		*/
		/*
		if (map_ents->solid == SOLID_NOT)
			Com_Printf("%s [%i] SOLID_NOT classname: %s\n", __func__, edict_cur, map_ents->classname);
		else if (map_ents->solid == SOLID_TRIGGER)
			Com_Printf("%s [%i] SOLID_TRIGGER classname: %s\n", __func__, edict_cur, map_ents->classname);
		else if (map_ents->solid == SOLID_BBOX)
			Com_Printf("%s [%i] SOLID_BBOX classname: %s\n", __func__, edict_cur, map_ents->classname);
		else if (map_ents->solid == SOLID_BSP)
			Com_Printf("%s [%i] SOLID_BSP classname: %s\n", __func__, edict_cur, map_ents->classname);
		else
			Com_Printf("%s [%i] SOLID_??? classname: %s\n", __func__, edict_cur, map_ents->classname);
		*/



		//if (strlen(map_ents->classname) > 1)
		//	Com_Printf("%s %s found\n", __func__, map_ents->classname);

		if (strcmp(map_ents->classname, "trigger_hurt") == 0)
		{
			map_ents->solid = SOLID_BBOX;
			gi.linkentity(map_ents); // Relink ent because solidity changed
			//Com_Printf("%s %s found\n", __func__, map_ents->classname, map_ents->s.solid);
		}

		//if (strcmp(map_ents->classname, "func_rotating") == 0)
		//{
			//map_ents->solid = SOLID_BBOX;
			//gi.linkentity(map_ents); // Relink ent because solidity changed
			//Com_Printf("%s %s found\n", __func__, map_ents->classname, map_ents->s.solid);
		//}

		edict_cur++;
	}
	return edict_cur;
}
void BOTLIB_RestoreEntsSolidState(solid_t* trigger_solid)
{
	if (trigger_solid == NULL)
	{
		Com_Printf("%s trigger_solid is NULL\n", __func__);
		return;
	}
	// Restore solid state of each ent
	int edict_cur = 0;
	edict_t* map_ents;
	int edict_num = 1 + game.maxclients; // skip worldclass & players
	for (map_ents = g_edicts + edict_num; map_ents < &g_edicts[globals.num_edicts]; map_ents++)
	{
		if (map_ents->inuse == false)
		{
			//edict_cur++;
			continue;
		}

		map_ents->solid = trigger_solid[edict_cur]; // Restore the original solid state
		gi.linkentity(map_ents); // Relink ent because solidity changed

		edict_cur++;
	}
	if (trigger_solid)
		free(trigger_solid);
}

// Add nodes at item locations
void BOTLIB_AddItemNodes(void)
{
	// ===========================
	// Add nodes at item locations
	// ===========================
	int foundItems = 0; // How many items were found
	vec3_t item_origin = { 0,0,0 };
	edict_t* map_ents;
	int edict_num = 1 + game.maxclients; // skip worldclass & players
	trace_t tr;
	for (map_ents = g_edicts + edict_num; map_ents < &g_edicts[globals.num_edicts]; map_ents++)
	{
		// Skip ents that are not in used and not an item
		if (map_ents->inuse == false || map_ents->item == NULL)
			continue;

		// Find the ground under the item
		// Some ents drop from the sky, so we need to trace down to the ground to find their landing spot (tr.endpos)
		tr = gi.trace(map_ents->s.origin, NULL, NULL, tv(map_ents->s.origin[0], map_ents->s.origin[1], -4096), g_edicts, MASK_PLAYERSOLID);

		// See if the item will fit (player size) on the ground
		// Some ents are placed on shelfs (urban2) so we cannot place a node on the shelf, so look for a spot to place the node nearby
		tr = gi.trace(tr.endpos, tv(-12, -12, 0), tv(12, 12, 56), tr.endpos, g_edicts, MASK_PLAYERSOLID);

		VectorCopy(tr.endpos, item_origin); // Update the origin

		// If it won't fit, then look for a spot nearby
		if (tr.startsolid || tr.fraction < 1.0)
		{
			float offset = 30;	// How far to offset from the original item location (30 allows just enough room to grab the item, but not too far away)
			// This number '30' was picked because of the shelves on urban2. The bot needs to walk up to the shelf to grab the item.

// Try 24 units forward (X + 24)
			tr = gi.trace(tv(item_origin[0] + offset, item_origin[1], item_origin[2]), tv(-12, -12, 0), tv(12, 12, 56), tv(item_origin[0] + offset, item_origin[1], item_origin[2]), g_edicts, MASK_PLAYERSOLID);
			if (tr.startsolid == false && tr.fraction == 1.0)
				goto VIABLE_ITEM_NODE; // Success!

			// Try 24 units back (X - 24)
			tr = gi.trace(tv(item_origin[0] - offset, item_origin[1], item_origin[2]), tv(-12, -12, 0), tv(12, 12, 56), tv(item_origin[0] - offset, item_origin[1], item_origin[2]), g_edicts, MASK_PLAYERSOLID);
			if (tr.startsolid == false && tr.fraction == 1.0)
				goto VIABLE_ITEM_NODE; // Success!

			// Try 24 units right (Y + 24)
			tr = gi.trace(tv(item_origin[0], item_origin[1] + offset, item_origin[2]), tv(-12, -12, 0), tv(12, 12, 56), tv(item_origin[0], item_origin[1] + offset, item_origin[2]), g_edicts, MASK_PLAYERSOLID);
			if (tr.startsolid == false && tr.fraction == 1.0)
				goto VIABLE_ITEM_NODE; // Success!

			// Try 24 units left (Y - 24)
			tr = gi.trace(tv(item_origin[0], item_origin[1] - offset, item_origin[2]), tv(-12, -12, 0), tv(12, 12, 56), tv(item_origin[0], item_origin[1] - offset, item_origin[2]), g_edicts, MASK_PLAYERSOLID);
			if (tr.startsolid == false && tr.fraction == 1.0)
				goto VIABLE_ITEM_NODE; // Success!

			// If we get here, then we could not find a spot to place the node
			Com_Printf("%s WARNING: could not add node for item: %s\n", __func__, map_ents->classname);
			continue; // Skip adding a node for this item because we cannot reach it

			// We found a spot to place the node
		VIABLE_ITEM_NODE:
			VectorCopy(tr.endpos, item_origin); // Update the origin
			tr = gi.trace(item_origin, NULL, NULL, tv(item_origin[0], item_origin[1], -4096), g_edicts, MASK_PLAYERSOLID); // Find the ground under the item
			VectorCopy(tr.endpos, item_origin); // Update the origin again
		}


		item_origin[2] += NODE_Z_HEIGHT; // Raise the node up to standard height


		Com_Printf("%s found node for item: %s [typeNum: %d]\n", __func__, map_ents->classname, map_ents->item->typeNum);

		// Weapons
		if (strcmp(map_ents->classname, "weapon_Sniper") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "weapon_M4") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "weapon_MP5") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "weapon_M3") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "weapon_HC") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "weapon_Grenade") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}

		// Ammo
		else if (strcmp(map_ents->classname, "ammo_clip") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "ammo_mag") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "ammo_m4") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "ammo_m3") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
		else if (strcmp(map_ents->classname, "ammo_sniper") == 0)
		{
			foundItems++;
			DAIC_Add_Node(item_origin, tr.plane.normal, NODE_ITEM);
		}
	}
	if (foundItems == 0)
	{
		// TODO: Get map ents without needing deathmatch enabled

		Com_Printf("%s WARNING: bot navigation was generated without item nodes. Please ensure the map is generated in deathmatch mode.\n", __func__);
	}
}

// Find the reachability for each node
void BOTLIB_ProcesssReachabilities(void)
{
	vec3_t zero = { 0 };
	node_t* from_node;	// From node
	node_t* to_node;	// To node
	int s, result;
	//qboolean found_ladder;
	for (s = 0; s < numnodes; s++) // From node
	{
		if (nodes[s].inuse == false) continue; // Ignore nodes not in use

		//if (nodes[s].nodenum != INVALID) continue;

		// Get the next 'from' node
		from_node = (unsorted_nodes + s); // From Node

		// Zero the link counter
		from_node->num_links = 0;

		for (int o = 0; o < numnodes; o++) // To node
		{
			if (nodes[o].inuse == false) continue; // Ignore nodes not in use
			//if (nodes[o].nodenum != INVALID) continue;

			if (s == o) // skip self
				continue;

			to_node = (unsorted_nodes + o); // Node A

			result = DC_Reachability(from_node->nodenum, to_node->nodenum, from_node->origin, to_node->origin, from_node->normal);

			//TEST CODE -- s
			//if (result == NODE_JUMPPAD || result == NODE_LADDER_UP || result == NODE_LADDER_DOWN)
			//	result = INVALID;
			//if (result != NODE_MOVE && result != NODE_STEP) // Force only MOVE nodes
			//	result = INVALID;
			//if (from_node->nodenum == 1740 && to_node->nodenum == 1403)
			//	result = NODE_JUMPPAD;
			//TEST CODE -- e

			if (BOTLIB_AddNodeLink(s, o, result, true))
			{
				ACEND_UpdateNodeReach(from_node->nodenum, to_node->nodenum); // Update path_table
				nmesh.total_reach++;
			}

			/*
			if (result != INVALID)
			{
				for (int lk = 0; lk < MAXLINKS; lk++)
				{
					if (from_node->links[lk].targetNode != o && from_node->links[lk].targetNode == INVALID) // Find a spare target link
					{
						from_node->links[lk].targetNodeType = result; // Set target node type

						// Add link
						//DC_AddNodeLink(unsorted_nodes, s, o, zero, false, result, true);
						//break;


						//DC_AddNodeLink(NULL, s_node->nodenum, a_node->nodenum, zero, false, s_node->links[lk1].targetNodeType, true); // Copy the sorted nodes
						if (DC_AddNodeLink(NULL, s, o, zero, false, result, true))
						{
							ACEND_UpdateNodeReach(from_node->nodenum, to_node->nodenum); // Update path_table
							//if (from_node->num_links + 1 > MAXLINKS) Com_Printf("%s too many links added to node[%d]\n", __func__, s);
							//from_node->num_links++; // Update number of links this node has
							nmesh.total_reach++;
						}
						break;
					}
				}
				//Com_Printf("%s node[%d] added %i links\n", __func__, s, nodes[s].num_links);
			}
			*/
		}
	}
}

void BOTLIB_BSP_SURFACES(bsp_t* bsp)
{
	int f, e;
	int nv = 0; // Current number of verts
	mface_t* surf;
	msurfedge_t* src_surfedge;

	// Copy data from the BSP tree
	for (f = 0, surf = bsp->faces; f < bsp->numfaces; f++, surf++)
	{
		if (surf == NULL)
		{
			gi.dprintf("%s null faces found\n", __func__);
			return;
		}

		nmesh.face[f].drawflags = surf->drawflags; // Copy draw flags (DSURF_PLANEBACK, etc)
		VectorCopy(surf->plane->normal, nmesh.face[f].normal); // Copy plane normal
		nmesh.face[f].num_edges = surf->numsurfedges; // Copy total edges on this face

		// Don't exclude sky because some walkable surfaces are sky
		//if (surf->drawflags & SURF_SKY)
		//	continue;

		// If surf plane is a flat floor
		if (surf->plane->normal[2] == 1)
		{
			if (surf->drawflags & DSURF_PLANEBACK) // Skip flat roof surfaces
				continue;

			nmesh.face[f].type = FACETYPE_FLOOR; // Perfectly flat
		}
		else if (surf->plane->normal[0] >= -MAX_STEEPNESS && surf->plane->normal[0] <= MAX_STEEPNESS && surf->plane->normal[1] >= -MAX_STEEPNESS && surf->plane->normal[1] <= MAX_STEEPNESS && surf->plane->normal[2] >= MAX_STEEPNESS)
		{
			nmesh.face[f].type = FACETYPE_SLOPE; // Any climbable sloped surface
		}
		else //if (surf->plane->normal[2] == 0) // Wall with normal[2] == 0 means the wall is perfectly upright
			nmesh.face[f].type = FACETYPE_WALL; // Anything too steep to climb becomes a wall	 (needs to yet be tested for CONTENTS_LADDER, this will be processed further down)


		for (e = 0, src_surfedge = surf->firstsurfedge; e < surf->numsurfedges; e++, src_surfedge++)
		{
			// Copy edge verts
			VectorCopy(bsp->vertices[bsp->edges[src_surfedge->edge].v[0]].point, nmesh.face[f].edge[e].v[0]);
			VectorCopy(bsp->vertices[bsp->edges[src_surfedge->edge].v[1]].point, nmesh.face[f].edge[e].v[1]);


			// Copy face verts
			nv = nmesh.face[f].num_verts;
			VectorCopy(nmesh.face[f].edge[e].v[0], nmesh.face[f].v[nv]);
			VectorCopy(nmesh.face[f].edge[e].v[1], nmesh.face[f].v[nv + 1]);
			nmesh.face[f].num_verts += 2;
		}

		//f = ++nmesh.total_faces; // Total faces
		nmesh.total_faces++; // Total faces


		nmesh.total_verts += (surf->numsurfedges * 2); // Total verts
		nmesh.total_edges += surf->numsurfedges; // Total edges
	}
}

void BOTLIB_Process_NMesh(edict_t* ent)
{
	trace_t tr;

	//	Player when standing
	vec3_t  player_standing_mins = { -16, -16, -24 };
	vec3_t  player_standing_maxs = { 16, 16, 32 };
	//	Player when crouching/ducking
	vec3_t  player_crouching_mins = { -16, -16, -24 };
	vec3_t  player_crouching_maxs = { 16, 16, 4 };
	//	Player when standing
	// vec3_t  player_standing_smaller_mins = { -14, -14, -24 };
	// vec3_t  player_standing_smaller_maxs = { 14, 14, 32 };

	//const int MIN_STEP_WIDTH = 32; // min step width
	//qboolean hit_step = false; // If we hit a suspected step (not 100% sure)
	//qboolean hit_ledge = false; // If we hit a suspected ledge (not 100% sure)

	// vec3_t mid_point = { 0 };
	// vec3_t ladder_bottom = { 0 };
	// vec3_t ladder_top = { 0 };

	int f, e;

	float normal_0, normal_1, normal_2;
	//int num_tris = 0;
	//int vert_count = 0; // Vertex count
	//int total_verts = 0;
	vec3_t pt0 = { 0 }; // Point 1 of a triangle
	vec3_t pt1 = { 0 }; // Point 2 of a triangle
	vec3_t pt2 = { 0 }; // Point 3 of a triangle
	vec3_t center = { 0 }; // Center point of either an edge or a triangle

	// Proccess the mesh data
	for (f = 0; f < nmesh.total_faces; f++)
	{
		normal_0 = nmesh.face[f].normal[0];
		normal_1 = nmesh.face[f].normal[1];
		normal_2 = nmesh.face[f].normal[2];

		int num_tris = 0;
		int vert_count = 0;
		int total_verts = nmesh.face[f].num_verts;
		memset(pt0, 0, sizeof(vec3_t));
		memset(pt1, 0, sizeof(vec3_t));
		memset(pt2, 0, sizeof(vec3_t));
		memset(center, 0, sizeof(vec3_t));

		// Edges - (only the outside edges of a face)
		//-----------------------------------------------------
		for (e = 0; e < nmesh.face[f].num_edges; e++)
		{
			// Copy the edge vectors
			VectorCopy(nmesh.face[f].edge[e].v[0], pt1);
			VectorCopy(nmesh.face[f].edge[e].v[1], pt2);

			// Get the distance between pt1 and pt2
			vec3_t pt1_to_pt2_vec;
			VectorSubtract(pt2, pt1, pt1_to_pt2_vec);			  // Direction vector from pt1 to pt2
			float pt1_to_pt2_dist = VectorLength(pt1_to_pt2_vec); // Distance between pt1 and pt2

			// Skip excessively small edges
			if (pt1_to_pt2_dist < 4)
			{
				nmesh.face[f].edge[e].node = INVALID;
				continue;
			}

			// Get the normalized node height based on the face type -- floor, slope, or wall
			vec3_t adjusted_height = { 0 };
			if (nmesh.face[f].type == FACETYPE_FLOOR)
				adjusted_height[2] += NODE_Z_HEIGHT; // Move the center so it is somewhat above the ground, instead of being stuck inside the ground
			else if (nmesh.face[f].type == FACETYPE_SLOPE)
				adjusted_height[2] += (NODE_Z_HEIGHT * normal_2); // Calculate the correct height based on the normal of the slope
			else if (nmesh.face[f].type == FACETYPE_WALL) // Push out from wall
			{
				adjusted_height[2] += NODE_Z_HEIGHT; // Move up from ground (note this isn't using the normal direction, its directly up from the ground)

				// Calculate the correct wall distance based on the normal angle
				if (nmesh.face[f].drawflags & DSURF_PLANEBACK)
				{
					if (normal_0 != 0)
						adjusted_height[0] -= (NODE_Z_HEIGHT * normal_0); // -X (forward wall)
					if (normal_1 != 0)
						adjusted_height[1] -= (NODE_Z_HEIGHT * normal_1); // -Y (left wall)
				}
				else
				{
					if (normal_0 != 0)
						adjusted_height[0] += (NODE_Z_HEIGHT * normal_0); // +X (back wall)
					if (normal_1 != 0)
						adjusted_height[1] += (NODE_Z_HEIGHT * normal_1); // +Y (right wall)
				}
			}

			// Get the center of the edge
			LerpVector(pt1, pt2, 0.50, center);

			// Record it
			VectorCopy(center, nmesh.face[f].edge[e].center); // Copy the center of the edge

			// Apply the adjusted height
			nmesh.face[f].edge[e].center[0] += adjusted_height[0];
			nmesh.face[f].edge[e].center[1] += adjusted_height[1];
			nmesh.face[f].edge[e].center[2] += adjusted_height[2];

			// Test for edges
			// Some edges are on a floor, others from a wall, therefore we test both here
			if (0)
			{
				qboolean hit_step_left = false, hit_step_right = false;
				vec3_t end_left, end_right;

				BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 90, 1, end_left);	// Turn 90 degrees left and move 1 unit
				BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, -90, 1, end_right); // Turn 90 degrees right and move 1 unit

				// Test just above center to see if we're in a solid
				trace_t tr_edge_center = gi.trace(tv(center[0], center[1], center[2] + 1), NULL, NULL, tv(center[0], center[1], center[2] + 1), ent, MASK_PLAYERSOLID);
				if (tr_edge_center.startsolid == false && tr_edge_center.fraction == 1.0) // Not in a solid (usually a wall)
				{
					// Test if we're on an edge with a dropoff
					trace_t tr_edge_left = gi.trace(end_left, NULL, NULL, tv(end_left[0], end_left[1], end_left[2] - 1), ent, MASK_PLAYERSOLID);
					trace_t tr_edge_right = gi.trace(end_right, NULL, NULL, tv(end_right[0], end_right[1], end_right[2] - 1), ent, MASK_PLAYERSOLID);
					if (tr_edge_left.startsolid == false && tr_edge_left.fraction == 1.0)
						hit_step_left = true;
					if (tr_edge_right.startsolid == false && tr_edge_right.fraction == 1.0)
						hit_step_right = true;

					//DAIC_Add_Node(center, nmesh.face[f].normal, NODE_STEP);

					if (hit_step_left && hit_step_right) // A double edge on either side
					{
						//DAIC_Add_Node(center, nmesh.face[f].normal, NODE_STEP);
					}
					else if (hit_step_left || hit_step_right) // A single edge -- ledge / stairs
					{
						//if (BOTLIB_UTIL_NearbyNodeAtHeightDist(center, NODE_SIZE) == INVALID)
						if (1)
						{
							//Com_Printf("%s f:%i  e:%i  node:%d  hit_step_left:%i  hit_step_right:%i\n", __func__, f, e, numnodes, hit_step_left, hit_step_right);

							//if (BOTLIB_TestForNodeDist(tv(center[0], center[1], center[2] + NODE_Z_HEIGHT), 2) == INVALID)
							DAIC_Add_Node(tv(center[0], center[1], center[2] + NODE_Z_HEIGHT), nmesh.face[f].normal, NODE_STEP);

							// Expand nodes from the top and bottom landing steps. Try adding a node 90 degrees left and -90 degrees right.
							// We try both directions because the top step is usually flat, and the landing step has a dropoff.
							// This will help the bot get around corners, and connect the top and bottom of stairs to the node network
							// 
							//           [left node]                    [sn_1]
							// -------------------------------------------- 
							// |                                          |      [sn_2]
							// |                                          ----------
							// | sn_x = step node                                  |       [sn_3]
							// | left node = move node                              ----------
							// | right node = move node                                       |               [right node]
							// |                                                              --------------------------------
							// The [left node] is added to the left of [sn_1]
							// The [right node] is added to the right of [sn_3]
							// 
							if (1)
							{
								vec3_t end;

								// Set player waist height
								const int player_waist_height = fabs(player_standing_mins[2]); // 24 units

								if (1) // Left step
								{
									BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 90, 32, end); //2048

									// Lift the end points up a bit so they don't get stuck in the ground
									end[2] += player_waist_height + 1;

									// Test to see if end is in a solid
									trace_t tr_end_step = gi.trace(end, tv(-15, -15, -24), tv(15, 15, 32), end, ent, MASK_PLAYERSOLID);
									if (tr_end_step.startsolid == false)
									{
										// Test to see if end is touching the ground
										trace_t tr_end_ground = gi.trace(end, NULL, NULL, tv(end[0], end[1], end[2] - (player_waist_height + 2 + STEPSIZE)), ent, MASK_PLAYERSOLID);

										// If the end is touching the ground, then we can add a node there
										if (tr_end_ground.fraction < 1.0)
										{
											// Bring height up to NODE_Z_HEIGHT
											tr_end_ground.endpos[2] += NODE_Z_HEIGHT;

											if (BOTLIB_TestForNodeDist(tr_end_ground.endpos, NODE_SIZE, tv(-16,-16,-16), tv(-16, -16, -16)) == INVALID)
												DAIC_Add_Node(tr_end_ground.endpos, tr_end_ground.plane.normal, NODE_MOVE);
										}
									}
								}

								if (1) // Right step
								{
									BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, -90, 32, end); //2048

									// Lift the end points up a bit so they don't get stuck in the ground
									end[2] += player_waist_height + 1;

									// Test to see if end is in a solid
									trace_t tr_end_step = gi.trace(end, tv(-15, -15, -24), tv(15, 15, 32), end, ent, MASK_PLAYERSOLID);
									if (tr_end_step.startsolid == false)
									{
										// Test to see if end is touching the ground
										trace_t tr_end_ground = gi.trace(end, NULL, NULL, tv(end[0], end[1], end[2] - (player_waist_height + 2 + STEPSIZE)), ent, MASK_PLAYERSOLID);

										// If the end is touching the ground, then we can add a node there
										if (tr_end_ground.fraction < 1.0)
										{
											// Bring height up to NODE_Z_HEIGHT
											tr_end_ground.endpos[2] += NODE_Z_HEIGHT;

											if (BOTLIB_TestForNodeDist(tr_end_ground.endpos, NODE_SIZE, tv(-16, -16, -16), tv(-16, -16, -16)) == INVALID)
												DAIC_Add_Node(tr_end_ground.endpos, tr_end_ground.plane.normal, NODE_MOVE);
										}
									}
								}
							}
						}
					}
					else // Floor edge (no dropoff on either side -- the floor is a flat plane)
					{
						// Expand additional nodes from the center of the edge.
						// Heading forward, back, left, right.
						if (1)
						{
							for (int d = 0; d < 4; d++)
							{
								vec3_t end;

								if (d == 0) // forwards
								{
									BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 0, 1024, end); // Forward 0 degrees
									//VectorCopy(pt1, end);
									//end[2] += 1;
								}
								else if (d == 1) // backwards
								{
									BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 180, 1024, end); // Backwards 180 or -180 degrees
									//VectorCopy(pt2, end);
									//end[2] += 1;
								}
								else if (d == 2) // left
								{
									BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 90, 1024, end); // Left (pt1 -> pt2 -> 90 degrees)
								}
								else if (d == 3) // right
								{
									BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, -90, 1024, end); // Right (pt1 -> pt2 -> -90 degrees)
								}

								// Get player waist height
								const int player_waist_height = fabs(player_standing_mins[2]); // 24 units

								// Lift the end points up a bit so they don't get stuck in the ground
								end[2] += player_waist_height + 1;

								// Test to see where end meets a wall
								// Center is lifted up a bit so it doesn't get stuck in the ground, for the same reason as end_left and end_right
								trace_t tr_end_wall = gi.trace(tv(center[0], center[1], center[2] + player_waist_height), player_standing_mins, player_standing_maxs, end, ent, MASK_PLAYERSOLID);


								// Test to see if end is touching the ground
								// From the wall hit down to the ground which is -(player_waist_height + 1)
								trace_t tr_end_ground = gi.trace(tr_end_wall.endpos, NULL, NULL, tv(tr_end_wall.endpos[0], tr_end_wall.endpos[1], tr_end_wall.endpos[2] - (player_waist_height + 1)), ent, MASK_PLAYERSOLID);


								// If the end is touching the ground, then we can add a node there
								if (tr_end_ground.fraction < 1.0)
								{
									if (BOTLIB_TestForNodeDist(tr_end_ground.endpos, NODE_SIZE * 4, tv(-16, -16, -16), tv(-16, -16, -16)) == INVALID)
									{
										// Bring height up to NODE_Z_HEIGHT
										tr_end_ground.endpos[2] += NODE_Z_HEIGHT;

										DAIC_Add_Node(tr_end_ground.endpos, tr_end_ground.plane.normal, NODE_MOVE);
									}
								}
							}
						}
					}
				}
			}


			// Skip wall edges
			if (nmesh.face[f].type == FACETYPE_WALL)
			{
				nmesh.face[f].edge[e].node = INVALID;
				continue;
			}

			// Find the middle between two points of an edge
			//LerpVector(pt1, pt2, 0.50, center); // 50% (center)


			// Ignore edges that are up in the air
			// Trace just below the floor, if we didn't hit the ground, then the node is up in the air. We want to ignore these because some are 'trigger' and other ents
			//if (nmesh.face[f].type == FACETYPE_FLOOR || nmesh.face[f].type == FACETYPE_SLOPE)
			{
				//tr = gi.trace(center, tv(-16, -16, -(NODE_Z_HEIGHT + 1)), tv(16, 16, 24), center, ent, MASK_SOLID);
				// move off ground slightly, then line trace back to ground, if we don't hit ground, then we're in the air
				tr = gi.trace(tv(center[0], center[1], center[2] + 1), NULL, NULL, tv(center[0], center[1], center[2] - 1), ent, MASK_PLAYERSOLID);
				//tr = gi.trace(center, tv(-0.1, -0.1, -0.1), tv(0.1, 0.1, 0.1), center, ent, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0)
				{
					//Com_Printf("%s face:%i HIT failed to find ground\n", __func__, f);
					nmesh.face[f].edge[e].node = INVALID;
					continue;
				}
			}

			// Check for trigger_hurt
			if (BOTLIB_UTIL_CHECK_FOR_HURT(center))
			{
				//Com_Printf("%s face:%i HIT trigger_hurt\n", __func__, f);
				nmesh.face[f].edge[e].node = INVALID;
				continue;
			}

			// Depending on the edge tested (within the same face) we may encounter different multiple groups of contents,
			// therefore we need to add them all together. This is because edge can be shared within the same face, or other faces.
			int box_contents = BOTLIB_UTIL_BOXCONTENTS(center);
			if (nmesh.face[f].contents > 0 && box_contents > 0 && box_contents != nmesh.face[f].contents)
			{
				// before
				//Com_Printf("%s face:%i tris:%i contents:0x%x contents:0x%x\n", __func__, f, nmesh.face[f].num_tris, nmesh.face[f].contents, tr.contents);
				//
				// Add tr.contents to nmesh.face[f].contents
				//
				//    0x18000002
				// +  0x08030000
				//    ----------
				// =  0x18030002
				//
				nmesh.face[f].contents |= box_contents;
				//
				// after
				//Com_Printf("%s face:%i tris:%i contents:0x%x\n", __func__, f, nmesh.face[f].num_tris, nmesh.face[f].contents);
			}
			// Add the first contents found
			if (nmesh.face[f].contents == 0)
				nmesh.face[f].contents = box_contents;

			// Copy it back
			VectorCopy(nmesh.face[f].edge[e].center, center);

			// Test for steps / edges
#if 0
			if (0)
			{

				// Check if trace hit a step - STEPSIZE
				// 
				//  e1 = edge 1
				//  e2 = edge 2
				//  c = center of bounding box
				// 
				// roof -------------------------------
				//
				//             ____________
				//             |          |
				//			   | player   |
				//             | bounding |
				//             | box      |
				//             |          |
				//             -----|------
				//				    c
				// 
				// floor ___________e1 (step)
				//		            |
				//                  |___e2 (step)
				//	                    |
				//		                |
				//	                    ------------- floor
				//
				// The center of the players bounding box will fit on (e1), but might not on (e2) as it could overlap against (e1)
				// We do the same floor trace as normal, except move the bounding box upward by STEPSIZE 
				// and check if player will fit, as if they were standing on the step while crouching. This is because
				// a player can crouch walk up stairs, thereby covering normal + edge cases.
				//

				hit_step = false; // If we hit a suspected step (not 100% sure)
				hit_ledge = false; // If we hit a suspected ledge (not 100% sure)


				vec3_t dist;
				VectorSubtract(pt1, pt2, dist); // vector from pt1 to pt2
				dist[2] = 0; // remove height
				vec3_t center_step;
				VectorCopy(center, center_step);
				center_step[2] -= (adjusted_height[2] - 0.1);
				trace_t tr_edge = gi.trace(center_step, tv(-16, -16, -24), tv(16, 16, 32), center_step, ent, MASK_PLAYERSOLID);
				if (tr_edge.startsolid && tr_edge.fraction < 1 && VectorLength(dist) > MIN_STEP_WIDTH/*calc step width*/) // inside step and we're larger than min step width
				{
					//*
					//vec3_t center_step;
					//VectorCopy(center, center_step);
					//center_step[2] += STEPSIZE;
					//center_step[2] += adjusted_height[2];
					center_step[2] += STEPSIZE + 0.1;

					//trace_t tr_step = gi.trace(center_step, player_crouching_mins, player_crouching_maxs, center_step, ent, MASK_DEADSOLID);
					trace_t tr_step = gi.trace(center_step, tv(-15.9, -15.9, 0), tv(15.9, 15.9, 0), center_step, ent, MASK_DEADSOLID);
					//trace_t tr_step = gi.trace(center_step, tv(-0.1, -0.1, 0), tv(0.1, 0.1, 0), center_step, ent, MASK_DEADSOLID);
					//trace_t tr_step = gi.trace(center_step, NULL, NULL, center_step, ent, MASK_DEADSOLID);
					if (!tr_step.startsolid && tr_step.fraction == 1)
					{
						// Find the floor
						tr_step = gi.trace(center_step, tv(-15.9, -15.9, 0), tv(15.9, 15.9, 0), tv(center_step[0], center_step[1], center_step[2] - (STEPSIZE + 1)), ent, MASK_DEADSOLID);
						tr_step.endpos[2] += adjusted_height[2];

						if (BOTLIB_UTIL_NearbyNodeAtHeightDist(tr_step.endpos, NODE_SIZE) == INVALID)
						{
							DAIC_Add_Node(tr_step.endpos, nmesh.face[f].normal, NODE_STEP);
						}
						//hit_step = true;
						//VectorCopy(center_step, center); // increase node z loc from NODE_Z_HEIGHT to (NODE_Z_HEIGHT + STEPSIZE)
						//Com_Printf("%s f:%d e:%d [center_step %f %f %f] [center %f %f %f]\n", __func__, f, e, center_step[0], center_step[1], center_step[2], center[0], center[1], center[2]);

					}
					//*/

					/*
					// Try again but move the bounding box slightly, this works for small edges that go along a wall (urban, urban2, etc)
					// Move the bounding box to each corner, forward/back/right/left. Then trace again.
					trace_t tr_edge;
					vec3_t center_edge;

					// forward
					if (hit_step == false)
					{
						VectorCopy(center, center_edge);
						center_edge[0] += 16; // move node forward
						tr_edge = gi.trace(center_edge, player_crouching_mins, player_crouching_maxs, center_edge, ent, MASK_DEADSOLID);
						if (!tr_edge.startsolid && tr_edge.fraction == 1)
						{
							hit_ledge = true;
							VectorCopy(center_edge, center);
						}
					}

					// back
					if (hit_ledge == false)
					{
						VectorCopy(center, center_edge);
						center_edge[0] -= 16; // move node back
						tr_edge = gi.trace(center_edge, player_crouching_mins, player_crouching_maxs, center_edge, ent, MASK_DEADSOLID);
						if (!tr_edge.startsolid && tr_edge.fraction == 1)
						{
							hit_ledge = true;
							VectorCopy(center_edge, center);
						}
					}

					// right
					if (hit_ledge == false)
					{
						VectorCopy(center, center_edge);
						center_edge[1] += 16; // move node right
						tr_edge = gi.trace(center_edge, player_crouching_mins, player_crouching_maxs, center_edge, ent, MASK_DEADSOLID);
						if (!tr_edge.startsolid && tr_edge.fraction == 1)
						{
							hit_ledge = true;
							VectorCopy(center_edge, center);
						}
					}

					// left
					if (hit_ledge == false)
					{
						VectorCopy(center, center_edge);
						center_edge[1] -= 16; // move node left
						tr_edge = gi.trace(center_edge, player_crouching_mins, player_crouching_maxs, center_edge, ent, MASK_DEADSOLID);
						if (!tr_edge.startsolid && tr_edge.fraction == 1)
						{
							hit_ledge = true;
							VectorCopy(center_edge, center);
						}
					}
					*/
				}
			}
#endif

			//if (nmesh.face[f].type == FACETYPE_WALL && (nmesh.face[f].contents & CONTENTS_LADDER) == 0)


			if (nmesh.face[f].type == FACETYPE_SLOPE)
				tr = gi.trace(center, tv(-16, -16, -6), player_crouching_maxs, center, ent, MASK_PLAYERSOLID);
			else // floor
				tr = gi.trace(center, player_crouching_mins, player_crouching_maxs, center, ent, MASK_PLAYERSOLID);



			//tr = gi.trace(center, tv(-8, -8, -20), tv(8, 8, 0), center, ent, MASK_SOLID);
			if ((!tr.startsolid && tr.fraction == 1))// || hit_step || hit_ledge)
			{
				//if (tr.contents < CONTENTS_AREAPORTAL || (tr.contents & CONTENTS_LADDER) == 0) // Look for visible brushes, including non-solid such as liquids, also ladders
				{
					if (BOTLIB_TestForNodeDist(center, NODE_SIZE, tv(-16, -16, -16), tv(-16, -16, -16)) == INVALID) // || hit_step || hit_ledge) // hit_step and hit_ledge ignore node size (for now)
					{
						// Test again, but just for MASK_WATER
						if (nmesh.face[f].type == FACETYPE_SLOPE)
							tr = gi.trace(center, tv(-16, -16, -6), player_crouching_maxs, center, ent, MASK_WATER);
						else
							tr = gi.trace(center, player_crouching_mins, player_crouching_maxs, center, ent, MASK_WATER);

						if ((nmesh.face[f].contents & CONTENTS_LAVA) || (nmesh.face[f].contents & CONTENTS_SLIME))
							continue;
						else if ((tr.contents & CONTENTS_WATER) || nmesh.face[f].contents & CONTENTS_WATER)
							//continue; // Don't add edge water nodes 
							DAIC_Add_Node(center, nmesh.face[f].normal, NODE_WATER);
						else
						{
							//if (hit_step) // hit_step
							//	DAIC_Add_Node(center, nmesh.face[f].normal, NODE_STEP);
							//else if (hit_ledge)
							//	DAIC_Add_Node(center, nmesh.face[f].normal, NODE_STEP);
							//else
							DAIC_Add_Node(center, nmesh.face[f].normal, NODE_MOVE);
						}

						//if (nmesh.face[f].edge[e].node)
						//	Com_Printf("WARNING: edge node already exists face:%d edge:%d node:%d\n", f, e, nmesh.face[f].edge[e].node);

						nmesh.face[f].edge[e].node = numnodes - 1;


						// Expand additional nodes from the center of the edge, one heading 90 degrees left, and one heading 90 degrees right.
						if (0 && nmesh.face[f].type == FACETYPE_FLOOR)
						{
							vec3_t end_left, end_right;

							BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 90, 2000, end_left);
							BOTLIB_UTIL_ROTATE_CENTER(pt1, pt2, 270, 2000, end_right);

							// Lift the end points up a bit so they don't get stuck in the ground
							end_left[2] += 1;
							end_right[2] += 1;

							// Find the middle between two points of an edge
							LerpVector(pt1, pt2, 0.50, center);
							//center[2] += NODE_Z_HEIGHT;
							//center[2] += 25;

							// Test to see where the left and right meet a wall
							// Center is lifted up a bit so it doesn't get stuck in the ground, for the same reason as end_left and end_right
							trace_t tr_left_wall = gi.trace(tv(center[0], center[1], center[2] + 1), player_standing_mins, player_standing_maxs, end_left, ent, MASK_PLAYERSOLID);
							trace_t tr_right_wall = gi.trace(tv(center[0], center[1], center[2] + 1), player_standing_mins, player_standing_maxs, end_right, ent, MASK_PLAYERSOLID);

							//DAIC_Add_Node(tr_left_wall.endpos, tr_left_wall.plane.normal, NODE_MOVE);
							//DAIC_Add_Node(tr_right_wall.endpos, tr_right_wall.plane.normal, NODE_MOVE);

							// Test to see if left and right are touching the ground
							//trace_t tr_left_ground = gi.trace(tr_left_wall.endpos, NULL, NULL, tv(tr_left_wall.endpos[0], tr_left_wall.endpos[1], tr_left_wall.endpos[2] - NODE_Z_HEIGHT), ent, MASK_PLAYERSOLID);
							//trace_t tr_right_ground = gi.trace(tr_right_wall.endpos, NULL, NULL, tv(tr_right_wall.endpos[0], tr_right_wall.endpos[1], tr_right_wall.endpos[2] - NODE_Z_HEIGHT), ent, MASK_PLAYERSOLID);
							trace_t tr_left_ground = gi.trace(tr_left_wall.endpos, NULL, NULL, tv(tr_left_wall.endpos[0], tr_left_wall.endpos[1], tr_left_wall.endpos[2] - 1.2), ent, MASK_PLAYERSOLID);
							trace_t tr_right_ground = gi.trace(tr_right_wall.endpos, NULL, NULL, tv(tr_right_wall.endpos[0], tr_right_wall.endpos[1], tr_right_wall.endpos[2] - 1.2), ent, MASK_PLAYERSOLID);



							// If the left is touching the ground, then we can add a node there
							//if (tr_left_ground.fraction < 1.0)
							{
								tr_left_wall.endpos[2] += (NODE_Z_HEIGHT - 1);

								//if (BOTLIB_TestForNodeDist(tr_left_wall.endpos, NODE_SIZE) == INVALID)
								DAIC_Add_Node(tr_left_wall.endpos, tr_left_ground.plane.normal, NODE_MOVE);
							}
							// If the right is touching the ground, then we can add a node there
							//if (tr_right_ground.fraction < 1.0)
							{
								tr_right_wall.endpos[2] += (NODE_Z_HEIGHT - 1);

								//if (BOTLIB_TestForNodeDist(tr_right_wall.endpos, NODE_SIZE) == INVALID)
								DAIC_Add_Node(tr_right_wall.endpos, tr_right_ground.plane.normal, NODE_MOVE);
							}
						}



					}
				}
			}
		}
	//-----------------------------------------------------
	}
}

#define BOTLIB_OrthogonalToVectors(v1, v2, res) \
	(res)[0] = ((v1)[1] * (v2)[2]) - ((v1)[2] * (v2)[1]);\
	(res)[1] = ((v1)[2] * (v2)[0]) - ((v1)[0] * (v2)[2]);\
	(res)[2] = ((v1)[0] * (v2)[1]) - ((v1)[1] * (v2)[0]);

// Verts must be unwound and in clockwise order
qboolean BOTLIB_InsideFace(vec3_t *verts, int num_verts, vec3_t point, vec3_t normal, float epsilon)
{
	vec3_t v1, v2;
	vec3_t edgevec, pointvec, sepnormal;

	for (int v = 0; v < num_verts; v += 2) // edges
	{
		VectorCopy(verts[v + 0], v1);
		VectorCopy(verts[v + 1], v2);
		//edge vector
		VectorSubtract(v2, v1, edgevec);
		//vector from first edge point to point possible in face
		VectorSubtract(point, v1, pointvec);
		//get a vector pointing inside the face orthogonal to both the
		//edge vector and the normal vector of the plane the face is in
		//this vector defines a plane through the origin (first vertex of
		//edge) and through both the edge vector and the normal vector
		//of the plane
		BOTLIB_OrthogonalToVectors(edgevec, normal, sepnormal);
		//check on which side of the above plane the point is
		//this is done by checking the sign of the dot product of the
		//vector orthogonal vector from above and the vector from the
		//origin (first vertex of edge) to the point 
		//if the dotproduct is smaller than zero the point is outside the face
		if (DotProduct(pointvec, sepnormal) < -epsilon) return false;
	}

	return true; // Found point
}

void BOTLIB_InitNavigation(edict_t* ent)
{
	rektek_bots_api_v1_t *bsp_api = gex.GetExtension(REKTEK_BOTS_API_V1);
	bsp_t* bsp = bsp_api->Bsp();
	if (bsp == NULL)
	{
		gi.dprintf("%s failed to import BSP data\n", __func__);
		return;
	}

	if (bsp->checksum == 0)
	{
		gi.dprintf("%s bsp->checksum is zero\n", __func__);
		return;
	}

	if (ent == NULL)
	{
		ent = g_edicts;
		if (ent == NULL)
			return;
	}

	BOTLIB_InitNodes();

	nmesh.bsp_checksum = bsp->checksum; // Save the map checksum

	Remove_All_Doors(); // Make sure all doors are open before making any nodes
}

void ACEND_BSP(edict_t* ent)
{
	rektek_bots_api_v1_t *bsp_api = gex.GetExtension(REKTEK_BOTS_API_V1);
	bsp_t* bsp = bsp_api->Bsp();
	if (bsp == NULL)
	{
		gi.dprintf("%s failed to import BSP data\n", __func__);
		return;
	}

	if (ent == NULL)
	{
		ent = g_edicts;
		if (ent == NULL)
			return;
	}

	clock_t clock_begin = clock();	// Start performance clock

	BOTLIB_InitNodes();
	nmesh.bsp_checksum = bsp->checksum; // Save the map checksum
	
	Remove_All_Doors(); // Make sure all doors are open before making any nodes

	solid_t *trigger_solid = (solid_t*)malloc(sizeof(solid_t) * MAX_EDICTS);
	if (trigger_solid == NULL)
	{
		Com_Printf("%s failed to malloc trigger_solid\n", __func__);
		return;
	}
	if (BOTLIB_MakeEntsSolid(trigger_solid) == -1)
		return;
	BOTLIB_AddItemNodes(); // Add nodes at item locations

	BOTLIB_BSP_SURFACES(bsp);

	//BOTLIB_Process_NMesh(ent);

	if (1)
	{
		rektek_bots_api_v1_t *nav_api = gex.GetExtension(REKTEK_BOTS_API_V1);
		nav_t* nav = nav_api->Nav();
		//ent->nav = gi.Nav(); // Grant access to navigation data
		if (ent->nav)
		{
			for (int f = 0; f < ent->nav->faces_total; f++)
			{
				if (ent->nav->surface_data_faces[f].face_type != FACETYPE_WALK && ent->nav->surface_data_faces[f].face_type != FACETYPE_TINYWALK) continue;

				/*
				for (int e = 0; e < ent->nav->surface_data_faces[f].num_verts; e += 2) // Edges
				{
					// Get center of edge
					vec3_t center;
					VectorAdd(ent->nav->surface_data_faces[f].verts[e], ent->nav->surface_data_faces[f].verts[e + 1], center);
					VectorScale(center, 0.5, center);
					
					// Move center toward face center by 4 units
					vec3_t dir;
					VectorSubtract(ent->nav->surface_data_faces[f].center_poly, center, dir);
					VectorNormalize(dir);
					VectorMA(center, 8, dir, center);

					// Check if we're still in the same face
					//BOTLIB_InsideFace

					// Move center up by 32 units
					center[2] += 32;

					// Test to see if the center is valid
					if (BOTLIB_TestForNodeDist(center, NODE_SIZE) == INVALID)
					{
						// Add the node
						DAIC_Add_Node(center, ent->nav->surface_data_faces[f].normal, NODE_MOVE);
					}
				}
				*/


				for (int c = 0; c < ent->nav->surface_data_faces[f].snode_counter; c++)
				{
					vec3_t pos;
					VectorCopy(ent->nav->surface_data_faces[f].snodes[c].start, pos);
					pos[2] += NODE_Z_HEIGHT;
					if (BOTLIB_TestForNodeDist(pos, NODE_SIZE, tv(-16, -16, -16), tv(-16, -16, -16)) == INVALID)
						DAIC_Add_Node(pos, ent->nav->surface_data_faces[f].normal, NODE_MOVE);
				}
			}
		}
	}

	// Find ladders by searching the center of bottom edges of walls that are CONTENTS_LADDER
	ACEND_FindEdgeLadders();

	// Find the reachability for each node
	BOTLIB_ProcesssReachabilities();

	// Try placing POI nodes on high locations
	if (0)
	{
		/*
		float highest_origin = -9999;
		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].origin[2] > highest_origin)
				highest_origin = nodes[i].origin[2];
		}
		float above_height = (highest_origin - 256);

		for (int i = 0; i < numnodes; i++)
		{
			if (nodes[i].origin[2] > above_height)// && random() < 0.1)
				nodes[i].type = NODE_POI;
		}
		*/

		//ACEND_BuildSpawnPointNodes();
		//ACEND_CachePointOfInterestNodes();
	}

	// Restore solid state of each ent
	BOTLIB_RestoreEntsSolidState(trigger_solid);



	gi.dprintf("BSP f[%i] t[%i] e[%i] v[%i] r[%i] n[%i]\n", nmesh.total_faces, nmesh.total_tris, nmesh.total_edges, nmesh.total_verts, nmesh.total_reach, numnodes);
	clock_t clock_end = clock();
	Com_Printf("%s execution took %f seconds to process bsp reachability\n", __func__, (double)(clock_end - clock_begin) / CLOCKS_PER_SEC);

	//ACEND_BuildVisibilityNodes(); // Rebuild node vis -- generate LOS nodes to target enemies
	// Free the memory
	if (unsorted_nodes)
	{
		free(unsorted_nodes);
		unsorted_nodes = NULL; // Nullify dangling pointer
	}
}
//rekkie -- BSP -- e