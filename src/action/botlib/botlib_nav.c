#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"


 //== GLOBAL SEMAPHORE ==
int	antSearch;

short int** path_table;
nav_area_t nav_area;

qboolean	nodeused[MAX_PNODES]; // This is used for a FAST check if the node has been used
short int	nodefrom[MAX_PNODES]; // Stores how we got here once the node is closed
float		nodeweight[MAX_PNODES]; // The weight of the node based on distance

/* =========================================================
The basic system works by using a single linked list and accessing information from the node array

1) The current node is found
2) All links from it are propogated - if not already done by another node
3) If we haven't found the target node then we get the next open node and go to 1
4) If we have the target node we return the path to it
5) If we run out of nodes then we return INVALID

This idea is based on "Path Finding Via 'Ant Races'  (a floodfill algorithm)"
by Richard Wesson. It is easy to optimise this code to make it even more efficient.

============================================================= */


//=========================
// Init the search path variables
//=========================
void AntInitSearch(edict_t* ent)
{
	//Make sure the lists and arrays used are all set to the correct values and EMPTY!
	memset(nodeused, 0, sizeof(nodeused));
	memset(nodefrom, INVALID, sizeof(nodefrom));
	while (!SLLempty(&ent->pathList))
	{
		SLLpop_front(&ent->pathList);
	}
}

/////////////////////////////////////////////////////////
// Check if any bot is navigating to the selected node //
/////////////////////////////////////////////////////////
qboolean Botlib_Nav_NodeInUse(edict_t* self, int node)
{
	//Com_Printf("%s ===============\n", __func__);

	if (node == INVALID)
		return false;

	short int i;
	for (i = 0; i < num_players; i++)
	{
		if (players[i]->is_bot == false || players[i] == self || players[i]->solid == SOLID_NOT || ACEAI_IsEnemy(self, players[i]) == false)
			continue;

		botlib_sll_t* list = &players[i]->pathList;
		if (list == NULL || list->head == NULL) // No path, or no head
			continue;

		botlib_sll_nodes_t* sll_node = list->head;

		// Setup nodes
		int current_node = players[i]->bot.current_node;
		int next_node = sll_node->node;

		// Loop through the linked list
		while (sll_node != NULL) // If no next
		{
			// Check if we're done, also sanity check
			if (current_node == players[i]->bot.goal_node || current_node == INVALID || next_node == INVALID)
				break;

			if (next_node == node)
			{
				//if (players[i]->client)
				//	Com_Printf("%s node[%d] is in use by %s\n", __func__, node, players[i]->client->pers.netname);
				return true;
			}

			//Com_Printf("%s curr[%d] next[%d] goal[%d]\n", __func__, current_node, next_node, self->bot.goal_node);


			// Get next node
			current_node = next_node;
			if ((sll_node = sll_node->next) == NULL) // No next node
				break;
			next_node = sll_node->node;

		}
	}

	//Com_Printf("%s ===============\n\n", __func__);

	return false;
}

//=========================
// StartSearch
//=========================
//
// returns true if a path is found
// false otherwise
//
int quick = 0;
int normal = 0;
qboolean	AntStartSearch(edict_t* ent, int from, int to, qboolean path_randomization)
{
	// Safety first!
	if (from == INVALID || to == INVALID)
		return false;

	//rekkie -- Dijkstra pathing -- s
	if (1)
	{
		//clock_t clock_begin = clock();	// Start performance clock
		//if (AntQuickPath(ent, from, to))
		{
			//Com_Printf("%s AntQuickPath()\n", __func__);

			//clock_t clock_end = clock();
			//double res = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
			//if (res == 0) quick++;
			//Com_Printf("%s execution took %f seconds. Quick %d\n", __func__, res, quick);

			//return true;
		}
		//else
		{
			if (BOTLIB_DijkstraPath(ent, from, to, path_randomization))
			//if (BOTLIB_DijkstraHeightPath(ent, from, to, path_randomization))
			//if (BOTLIB_HighestPath(ent, from, to, path_randomization))
			{
				//Com_Printf("%s BOTLIB_DijkstraPath()\n", __func__);

				//clock_t clock_end = clock();
				//double res = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
				//if (res == 0) normal++;
				//Com_Printf("%s execution took %f seconds. Normal %d\n", __func__, res, normal);

				return true;
			}
		}

		return false;
	}
	//rekkie -- Dijkstra pathing -- e


	//@@ TESTING ONLY!! antSearch always available
	antSearch = 1;
	// Check we're allowed to search - if so, do it
	if (1)// (antSearch > 0) && (ent->antLastCallTime < level.framenum - ANT_FREQ) )
	{
		//Com_Printf("%s AntFindPath()\n", __func__);
		// Decrement the semaphore to limit calls to this function
		//@@ If we ever get multithreading then we can increment later
		antSearch--;
		// make a note of when this bot last made a path call
		ent->antLastCallTime = level.framenum;
		// Set up the lists
		AntInitSearch(ent);
		// If we found a path
		if (AntFindPath(ent, from, to))
		{
			// pathList now contains the links in reverse order
			return true;
		}
	}

	// We can use the quick node search method here to get our path and put it in the pathList
	// the same way we do with the AntSearch mode. This will have the side effect of finding
	// bad paths and removing them.
	if (AntQuickPath(ent, from, to))
	{
		//Com_Printf("%s AntQuickPath()\n", __func__);
		return true;
	}
	// If not allowed to search and no path
	AntInitSearch(ent);	// Clear out the path storage
	return false;
}

//=================================
// QuickPath
//=================================
//
// Uses the old path array to get a quick answer and removes bad paths
//

qboolean AntQuickPath(edict_t* ent, int from, int to)
{
	int	newNode = from;
	int	oldNode = 0;
	int loopProtection = 0; //rekkie -- DEV_1

	// Clean out the arrays, etc.
	AntInitSearch(ent);
	nodeused[from] = true;
	// Check we can get from->to and that the path is complete
	while (newNode != INVALID)
	{
		oldNode = newNode;
		// get next node
		newNode = path_table[newNode][to];
		if (newNode == to)
		{
			// We're there - store it then build the path
			nodeused[newNode] = true;
			nodefrom[newNode] = oldNode;
			break;
		}
		else if (newNode == INVALID)
		{
			// We have a bad path
			break;
		}
		else if (!nodeused[newNode])
		{
			// Not been here yet - store it!
			nodeused[newNode] = true;
			nodefrom[newNode] = oldNode;
		}
		else
			break; // LOOP encountered
	}

	// If successful, build the pathList
	if (newNode == to)
	{
		SLLpush_front(&ent->pathList, to);
		while (newNode != from)
		{
			//rekkie -- s
			if (newNode >= numnodes || newNode < 0)
			{
				if (debug_mode)
					gi.dprintf("%s newNode out of bounds!\n", __func__);
				return false;
			}
			//rekkie -- e

			// Push the
			SLLpush_front(&ent->pathList, nodefrom[newNode]);
			newNode = nodefrom[newNode];
		}
		return true;
	}
	// else wipe out the bad path!
	else
	{
		newNode = oldNode;
		while (newNode != from)
		{
			path_table[nodefrom[newNode]][to] = INVALID;

			//rekkie -- DEV_1 -- s
			if (loopProtection < numnodes)
				loopProtection++;
			else
			{
				if (debug_mode)
					debug_printf("%s ------------------- Bad while loop. Forcing exit.\n", __func__);
				break;
			}
			//rekkie -- DEV_1 -- e
		}
		path_table[from][to] = INVALID;
	}
	return false;
}

//rekkie -- ZigZag pathing -- s
// The bot will zig and zag from node to node, trying to avoid enemy fire
qboolean BOTLIB_ZigZagPath(edict_t* ent, int from, int to)
{
	//Com_Printf("%s [%d]\n", __func__, level.framenum);

	// Sanity check
	if (from == INVALID || to == INVALID)
		return false;

	AntInitSearch(ent); // Clear out the path storage
	for (int i = 0; i < numnodes; i++)
		nodeweight[i] = NAV_INFINITE;

	int	newNode = INVALID; // Stores the node being tested
	int atNode = from; // Current node we're visiting
	botlib_sll_t openList; // Locally declared OPEN list
	openList.head = openList.tail = NULL; // Init the list

	float weight; // Weight of the node we're visiting
	nodeweight[atNode] = 0; // Set the weight to 0 (distance from self to self is 0)
	nodefrom[atNode] = atNode; // Set the parent node to itself
	//nodeused[atNode] = true; // Add the starting node to the visited CLOSED list
	SLLpush_back(&openList, from); // Store it


	while (!SLLempty(&openList)) // While there are nodes on the OPEN list
	{
		atNode = SLLfront(&openList); // Get the next node
		if (atNode == to) // If the next node is the goal node
		{
			//Com_Printf("%s [%d] next node found a path\n", __func__, level.framenum);
			break; // We found a path
		}


		// Update node weights, but don't add them to the list
		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			if (nodes[atNode].links[i].targetNode == INVALID)
				continue;

			newNode = nodes[atNode].links[i].targetNode; // Get the next linked node
			if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
			{
				weight = nodeweight[atNode] + nodes[atNode].links[i].cost;

				if (nodeweight[newNode] == NAV_INFINITE || nodeweight[newNode] < weight)
				{
					nodeweight[newNode] = weight; // Update the weight
					nodefrom[newNode] = atNode; // Update the parent node (open list)
					SLLpush_back(&openList, newNode); // Add it to the open list
				}
			}
		}

		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)
	}


	// Free up the memory we allocated
	SLLdelete(&openList);

	// Optimise stored path with this new information
	if (newNode == to)
	{
		// Make the path using the fromnode array pushing node numbers on in reverse order
		// so we can SLLpop_front them back later
		SLLpush_front(&ent->pathList, newNode);

		//rekkie -- DEV_1 -- s
		// This happens when a bot dies and respawns, nodefrom[to] spits out INVALID
		// Check if INVALID to avoid accessing [-1] array
		if (nodefrom[to] != INVALID)
			//rekkie -- DEV_1 -- e
			path_table[nodefrom[to]][to] = to; // Set the to path in node array because this is shortest path

		//Com_Printf("%s path_table[ nodefrom[%d][%d] = %d\n", __func__, to, to, to);
		//int prev_newNode = newNode;

		// We earlier set our start node to INVALID to set up the termination
		// Check if there is a path, and it's not the node we're standing on (safety check)
		while ((newNode = nodefrom[newNode]) != INVALID && (newNode != from))
		{
			//Com_Printf("%s %d = nodefrom[%d]\n", __func__, newNode, prev_newNode);
			//prev_newNode = newNode;

			// Push it onto the pathlist
			SLLpush_front(&ent->pathList, newNode);
			// Set the path in the node array to match this shortest path
			path_table[nodefrom[newNode]][to] = newNode;

			//Com_Printf("%s path_table[ nodefrom[%d][%d] = %d\n", __func__, newNode, to, newNode);
		}

		//Com_Printf("%s EXIT PATH\n\n", __func__);

		// Each time a path is found, make a copy
		ent->bot.node_list_count = BOTLIB_SLL_Query_All_Nodes(ent, &ent->pathList, ent->bot.node_list, MAX_NODELIST); // Retrieve the nodes in the list
		ent->bot.node_list_current = 0;
		if (ent->bot.node_list_count) // Set the current and next nodes
		{
			ent->bot.current_node = ent->bot.node_list[0];
			ent->bot.next_node = ent->bot.node_list[1];
		}

		if (0)
		{
			Com_Printf("%s [%d] s[%d] g[%d] node_list[", __func__, level.framenum, from, to);
			for (int i = 0; i < ent->bot.node_list_count; i++)
			{
				Com_Printf(" %d ", ent->bot.node_list[i]);
			}
			Com_Printf(" ]\n");
		}

		//Com_Printf("%s [%d] found a path\n", __func__, level.framenum);
		return true;
	}

	//Com_Printf("%s failed to find a path\n", __func__);
	return false;
}

//rekkie -- Dijkstra height pathing -- s
#if 0
// Function unfinished. Wanted bots to prefer taking higher paths.
qboolean BOTLIB_DijkstraHeightPath(edict_t* ent, int from, int to, qboolean path_randomization)
{
	//Com_Printf("%s from[%d] to[%d]\n", __func__, from, to);

	// Sanity check
	if (from == INVALID || to == INVALID)
		return false;

	

	AntInitSearch(ent); // Clear out the path storage
	for (int i = 0; i < numnodes; i++)
		nodeweight[i] = NAV_INFINITE;

	int	newNode = INVALID; // Stores the node being tested
	int atNode = from; // Current node we're visiting
	botlib_sll_t openList; // Locally declared OPEN list
	openList.head = openList.tail = NULL; // Init the list

	float weight; // Weight of the node we're visiting
	nodeweight[atNode] = 0; // Set the weight to 0 (distance from self to self is 0)
	nodefrom[atNode] = atNode; // Set the parent node to itself
	SLLpush_back(&openList, from); // Store it

	while (!SLLempty(&openList) && newNode != to) // While there are nodes on the OPEN list
	{
		atNode = SLLfront(&openList); // Get the next node
		if (atNode == to) // If the next node is the goal node
		{
			break; // We found a path
		}

		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			if (nodes[atNode].links[i].targetNode == INVALID)
				continue;
			if (nodes[atNode].links[i].targetNodeType == NODE_POI_LOOKAT) // POI look at nodes are not real links
				continue;

			newNode = nodes[atNode].links[i].targetNode; // Get the next linked node
			weight = nodeweight[atNode] + nodes[atNode].links[i].cost; // Get the weight of the node we're visiting + the cost to the new node

			if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
			{
				// If weight is less than the weight of the new node
				if (weight < nodeweight[newNode])
				{
					nodeweight[newNode] = weight; // Update the weight
					nodefrom[newNode] = atNode; // Update the parent node (open list)
					SLLpush_back(&openList, newNode); // Store it
				}
			}

			if (newNode == to) // If node being linked is the goal node
			{
				//Com_Printf("%s [%d] normal found a path\n", __func__, level.framenum);
				break; // We found a path
			}
		}

		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)
	}

	// Free up the memory we allocated
	SLLdelete(&openList);

	// Optimise stored path with this new information
	if (newNode == to)
	{
		// Make the path using the fromnode array pushing node numbers on in reverse order so we can SLLpop_front them back later
		SLLpush_front(&ent->pathList, newNode);

		// We earlier set our start node to INVALID to set up the termination
		// Check if there is a path, and it's not the node we're standing on (safety check)
		while ((newNode = nodefrom[newNode]) != INVALID && (newNode != from))
		{
			SLLpush_front(&ent->pathList, newNode); // Push it onto the pathlist
		}

		// Each time a path is found, make a copy
		ent->bot.node_list_count = BOTLIB_SLL_Query_All_Nodes(ent, &ent->pathList, ent->bot.node_list, MAX_NODELIST); // Retrieve the nodes in the list
		ent->bot.node_list_current = 0;
		if (ent->bot.node_list_count) // Set the current and next nodes
		{
			ent->bot.current_node = ent->bot.node_list[0];
			ent->bot.next_node = ent->bot.node_list[1];
		}

		return true;
	}

	//Com_Printf("%s failed to find a path\n", __func__);
	return false;
}
#endif
//rekkie -- Dijkstra Height Pathing -- e

#if 0
qboolean BOTLIB_NodeCanSeeEnemyPath(edict_t* ent, int atNode)
{
	// Check if node path can be seen by enemies
	if (nodes[atNode].num_vis_nodes) // && random() <= 0.2) // && ent->bot.enemies_num && random() <= 0.2)
	{
		for (int v = 0; v < nodes[atNode].num_vis_nodes; v++) // each vis node
		{
			/*
			for (int e = 0; e < num_players; e++) // each enemy
			{
				if (OnSameTeam(ent, players[e])) continue;

				for (int nl = 0; nl < players[e]->bot.node_list_count; nl++) // each node within the enemy's path
				{
					if (players[e]->bot.node_list[nl] == nodes[atNode].vis_nodes[v]) // does our node see the enemy path?
					{
						return true;
					}
				}
			}
			*/

			/*
			for (int e = 0; e < ent->bot.enemies_num; e++) // each enemy
			{
				for (int nl = 0; nl < players[ent->bot.enemies[e]]->bot.node_list_count; nl++) // each node within the enemy's path
				{
					if (players[ent->bot.enemies[e]]->bot.node_list[nl] == nodes[atNode].vis_nodes[v]) // does our node see the enemy path?
					{
						return true;
					}
				}
			}
			*/

			if (ent->enemy)
			{
				for (int nl = 0; nl < ent->enemy->bot.node_list_count; nl++) // each node within the enemy's path
				{
					if (ent->enemy->bot.node_list[nl] == nodes[atNode].vis_nodes[v]) // does our node see the enemy path?
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}
#endif


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Pathing is working, however some things to add,fix,do...
// 
// 2) Also need to test ALL failure points, and try to trigger failures and test if its handled correctly.
// 
// 3) Cache all nodes in each area. Leave room for an extra node (explained below what we use the extra for)
// 
// 4) Currently when BOTLIB_CanVisitNode() is called, it will search all nodes. I need this to search and path only the
//		areas which are involved in the search. Currently the next_node/edge node is located outside the current area of the bot,
//		and just inside the next area (the border). Therefore, I should include that extra node in the search. So if we're in Area 0, 
//		and the next node is in Area 1 (next to Area 0), then search will be all nodes in Area 0 + the single node from Area 1. 
//		Once the search + path is conducted, remove the node from the search.
// 
// 5) Deal with situtations where the areas are connected, but the goal node is not accessible in the last area. Perhaps after checking
//		if the areas connect, run a BOTLIB_CanVisitNode() search on the last area to see if it connects from the last edge node 
//		to the final goal node. ALSO, there might a scenario where the random selected last edge node does not connect, but one of the
//		many other edge nodes might? I guess in such a case, a connection might path through the other edge nodes anyway.
// 
// 6) When selecting a path from nav_area.dfs_paths[self->bot.path_taken][i], the paths are a series of contigous areas A>B>C>D,etc
//		I need an algorithm (similar to the heightmap) to see if areas are being overly used, and/or taking the higher area is preferred
//		over a lower area. I could also cache area combos and check agains them. Going back to the heightmap idea, if area C (middle TJ)
//		is being overly used, just check if any of the paths contain 'C' and avoid using them?
// 
// 7) When picking a random edge node, perhaps I could pick the closest edge node to the current node, thereby making it a shortest path
//		by direct line. However, this isn't the shortest 'path', only a direct line.
// 
// 8) Check if other bots are using the edge node, if so, pick another edge node if available.
// 
// 9) Move some of the arrays over to dynamic allocated memory, build the array as a single block (single malloc call)
// 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int DFS_area_nodes[MAX_NAV_AREAS][MAX_NAV_AREAS_NODES]; // Area nodes - [(32 * 1024) * 4 bytes = 132k]
int DFS_area_edges[MAX_NAV_AREAS][MAX_NAV_AREAS_EDGES]; // Area edge nodes (area edges that connect to other areas) - [(32 * 64) * 4 bytes = 8k]

// Init data used when selecting nodes
void BOTLIB_InitAreaNodes(void)
{
	for (int a = 0; a < MAX_NAV_AREAS; a++)
	{
		for (int n = 0; n < MAX_NAV_AREAS_NODES; n++)
		{
			DFS_area_nodes[a][n] = INVALID;
		}
	}

	for (int a = 0; a < MAX_NAV_AREAS; a++)
	{
		for (int e = 0; e < MAX_NAV_AREAS_EDGES; e++)
		{
			DFS_area_edges[a][e] = INVALID;
		}
	}
}

void BOTLIB_InitAreaConnections(void)
{
	// For all areas, find all their connecting neighbors.
	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		for (int a = 0; a < MAX_NAV_AREAS; a++)
		{
			nav_area.adjacency_matrix[i][a] = INVALID; // Adjacency matrix
		}
	}
	int targetNode = INVALID;
	int targetArea = INVALID;
	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		for (int n = 0; n < numnodes; n++)
		{
			if (nodes[n].area == i)
			{
				for (int l = 0; l < nodes[n].num_links; l++)
				{
					targetNode = nodes[n].links[l].targetNode;
					targetArea = nodes[targetNode].area;

					//nav_area.adjacency_matrix[i][targetArea] = targetArea;
					//gi.dprintf("Node[%d] Area[%d] connects to Node[%d] Area[%d]\n", n, i, targetNode, targetArea);
					if (targetArea != INVALID && targetArea != i) {
						if (targetArea != i)
						{
							nav_area.adjacency_matrix[i][targetArea] = targetArea;
						}
					}
				}
			}
		}
	}

	BOTLIB_UpdateAllAreaEdges(); // Update edge connections

	// Find total areas
	nav_area.total_areas = 0;
	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		for (int a = 0; a < MAX_NAV_AREAS; a++)
		{
			if (nav_area.adjacency_matrix[i][a] != INVALID)
			{
				nav_area.total_areas++;
				break;
			}
		}
	}

	if (nav_area.total_areas == 0)
	{
		Com_Printf("%s WARNING: Map has no area nodes. Bot pathing will be less optimal and large maps not supported.\n", __func__);
		return;
	}

	// Print connections
	/*
	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		for (int a = 0; a < MAX_NAV_AREAS; a++)
		{
			if (nav_area.adjacency_matrix[i][a] != INVALID)
			{
				Com_Printf("Nav area [%d] connects to [%d]\n", i, a);
			}
		}
	}
	*/


	// Cache all the nodes in each area
	BOTLIB_MallocAreaNodes(); // Alloc memory for area nodes
	// Check if memory was allocated
	if (nav_area.area_nodes && nav_area.area_nodes[0])
	{
		// Init areas with INVALID
		for (int i = 0; i < MAX_NAV_AREAS; ++i)
		{
			for (int n = 0; n < MAX_NAV_AREAS_NODES; n++)
			{
				nav_area.area_nodes[i][n] = INVALID;
			}
		}

		// Cache all the nodes in each area -- currently a max of (MAX_NAV_AREAS_NODES - 1) nodes per area is allowed
		qboolean reached_max_area_nodes;
		//qboolean notified = false;
		for (int i = 0; i < numnodes; i++)
		{
			reached_max_area_nodes = true;
			for (int n = 0; n < (MAX_NAV_AREAS_NODES - 1); n++) // Cache up to (MAX_NAV_AREAS_NODES - 1)
			{
				if (nav_area.area_nodes[nodes[i].area][n] == INVALID)
				{
					reached_max_area_nodes = false;
					nav_area.area_nodes[nodes[i].area][n] = nodes[i].nodenum;
					break;
				}
			}
			if (reached_max_area_nodes)// && notified == false)
			{
				//notified = true;
				Com_Printf("%s Nav area [%d] reached_max_area_nodes [%d]\n", __func__, nodes[i].area, MAX_NAV_AREAS_NODES);
				break;
			}
		}

		BOTLIB_RandomizeAreaColors();

		/*
		// Print out area nodes
		int total_nodes;
		for (int i = 0; i < MAX_NAV_AREAS; ++i)
		{
			if (nav_area.area_nodes[i][0] == INVALID)
				continue;

			total_nodes = 0;

			Com_Printf("%s Area [%d] nodes: ", __func__, i);
			for (int n = 0; n < MAX_NAV_AREAS_NODES; n++)
			{
				if (nav_area.area_nodes[i][n] == INVALID)
					break;

				Com_Printf(" %d ", nav_area.area_nodes[i][n]);
				total_nodes++;
			}
			Com_Printf("\nTotal nodes: %d\n", total_nodes);
		}
		*/
	}
}

// Create area nodes mallocs
void BOTLIB_MallocAreaNodes(void)
{
	// Dynamically cache all the nodes in each area -- currently a max of 1024 nodes per area is allowed
	//area_nodes[MAX_NAV_AREAS][MAX_NAV_AREAS_NODES]; // A collection of all nodes in an area
	nav_area.area_nodes = (int**)malloc(sizeof(*nav_area.area_nodes) * MAX_NAV_AREAS);
	if (!nav_area.area_nodes) {
		Com_Printf("%s Memory allocation failed for nav_area.area_nodes\n", __func__);
		nav_area.area_nodes = NULL;
	}
	else
	{
		for (int i = 0; i < MAX_NAV_AREAS; ++i)
		{
			nav_area.area_nodes[i] = (int*)malloc(sizeof(*(nav_area.area_nodes[i])) * MAX_NAV_AREAS_NODES);
			if (!nav_area.area_nodes[i])
			{
				Com_Printf("%s Memory allocation failed for nav_area.area_nodes\n", __func__);
				//nav_area.area_nodes = NULL;
			}
		}
		// Now we can use nav_area.area_nodes[i][j] to access any element :)
	}
}
// Free area nodes mallocs
void BOTLIB_FreeAreaNodes(void)
{
	if (nav_area.area_nodes)
	{
		for (int i = 0; i < MAX_NAV_AREAS; ++i)
		{
			if (nav_area.area_nodes[i])
			{
				free(nav_area.area_nodes[i]); // Freeing memory allocated for each row separately
				nav_area.area_nodes[i] = NULL;
			}
		}
		free(nav_area.area_nodes); // And then freeing the pointer to the pointers
		nav_area.area_nodes = NULL;
	}
}

// This initiates bot travel from current node to goal node
// 1) The bot will use area nodes if map has areas (Efficient),
// 2) Alternatively it will fallback to doing node-to-node pathing (Less efficient)
// 3) path_randomization only applies to the fallback method
qboolean BOTLIB_CanGotoNode(edict_t* self, int goal_node, qboolean path_randomization)
{
	if (goal_node == INVALID)
		return false;

	self->bot.node_list_count = 0;

	// New pathing - if area nodes are supported
	if (nav_area.total_areas > 0 && goal_node)
	{
		Com_Printf("%s %s goal_node[%i]\n", __func__, self->client->pers.netname, goal_node);
		if (BOTLIB_CanVisitAreaNode(self, goal_node)) // Get area-to-area (low resolution) path
		{
			while (BOTLIB_GetNextAreaNode(self)) // Get node-to-node (full resolution) path using area-to-area path
			{
			}

			// Add the goal to the end of the node list + terminate
			if (self->bot.node_list_count)
			{
				self->bot.node_list[self->bot.node_list_count++] = self->bot.goal_node; // Save goal node
				self->bot.node_list[self->bot.node_list_count] = INVALID; // Terminate
			}

			return true; // Success
		}
		else
		{
			Com_Printf("%s %s failed #1 \n", __func__, self->client->pers.netname);
			return false; // Failure
		}
	}
	else // Fallback to old pathing
	{
		self->bot.goal_node = INVALID;
		//self->bot.goal_node = goal_node;
		qboolean canVisit = BOTLIB_CanVisitNode(self, goal_node, path_randomization, INVALID, false);

		//gi.dprintf("BOTLIB_CanVisitNode? %d\n", canVisit);
		if (canVisit)
		{
			// Add the goal to the end of the node list + terminate
			if (self->bot.node_list_count)
			{
				self->bot.node_list[self->bot.node_list_count++] = self->bot.goal_node; // Save goal node
				self->bot.node_list[self->bot.node_list_count] = INVALID; // Terminate
			}

			return true; // Success
		}
	}

	// Failed #2 is super common, it just means that the bot cannot reach the goal node within the
	// bounds of how long distances can be calculated. It's not a failure, just a limitation.
	//Com_Printf("%s %s failed #2 \n", __func__, self->client->pers.netname);
	return false; // Failure to find path
}

// Checks if goal is reachable from current node
qboolean BOTLIB_CanVisitAreaNode(edict_t* self, int goal_node)
{
	if (goal_node == INVALID)
	{
		Com_Printf("%s INVALID PATH: goal_node is invalid\n", __func__);
		return false;
	}

	// Com_Printf("\n\n========================================================\n", __func__);
	// Com_Printf("%s Heading for goal node[%d] area[%d]\n", __func__, goal_node, self->bot.goal_area);
	// Com_Printf("========================================================\n", __func__);

	//BOTLIB_InitAreaConnections();

	// Init area
	self->bot.start_area = INVALID;
	//self->bot.current_area = INVALID;
	//self->bot.goal_area = INVALID;

	self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	
	if (self->bot.current_node == INVALID)
	{
		//Com_Printf("%s INVALID PATH: Cannot find current_node\n", __func__);
		self->bot.next_area_node = INVALID;
		return false;
	}
	else
	{
		self->bot.start_area = nodes[self->bot.current_node].area; // Area we first started from
		self->bot.current_area = nodes[self->bot.current_node].area; // Current area we're in
		self->bot.goal_area = nodes[goal_node].area; // Goal area to reach
		self->bot.next_area_counter = 0; // Keep track of which area is next, starting from the first area (current area)
		self->bot.next_area_node = self->bot.current_node;




		// If node is inside of the same (or final) area try to see if we can navigation there
		// Otherwise continue with trying to route to the goal_node's area
		if (self->bot.current_area == self->bot.goal_area)
		{
			{
				self->bot.next_area_nodes[0] = goal_node; // Last node
				self->bot.next_area_nodes_counter = 0; // Total nodes in next_area_nodes[]
				self->bot.next_area_nodes[1] = INVALID; // Terminate

				self->bot.goal_node = goal_node; // Set the goal node to reach
				self->bot.path_taken = 0; // Get rand path
				////self->bot.state = BOT_MOVE_STATE_NAV_NEXT;

				Com_Printf("%s Goal node[%d] is in the same area[%d] as your area [%d]\n", __func__, goal_node, self->bot.goal_area, self->bot.current_area);
				return true; // Success
			}
		}

	}




	/*
	// Pick a random area to visit
	// DO NOT DELETE, it will be useful later on
	//while (nav_area.goal_area == nav_area.start_area) // Pick a goal that isn't the same area as bot is in
	//	nav_area.goal_area = rand() % nav_area.total_areas; // Get rand goal
	// Get goal node within goal area
	for (int i = 0; i < numnodes; i++)
	{
		if (nodes[i].area == nav_area.goal_area)
		{
			nav_area.goal_node = nodes[i].nodenum; // Get first node in area
			//nav_area.goal_node = DFS_area_nodes[nav_area.goal_area][0]; // Get first node in area
			Com_Printf("%s Goal node [%d]\n", __func__, nav_area.goal_node);
			break;
		}
	}
	*/


	// Calculate all paths
	for (int i = 0; i < MAX_NAV_AREAS_PATHS; i++)
	{
		for (int a = 0; a < MAX_NAV_AREAS; a++)
		{
			nav_area.dfs_paths[i][a] = INVALID;
		}
	}
	nav_area.dfs_path_count = 0;
	memset(nav_area.dfs_visited, false, sizeof(nav_area.dfs_visited));

	nav_area.dfs_path[0] = 0;
	nav_area.dfs_len = 0;
	//nav_area.dfs_visited[0] = false;
	BOTLIB_DepthFirstSearch(self, self->bot.current_area, self->bot.goal_area); // Calculate all possible paths

	//Com_Printf("%s DFS_path_count >= %d\n", __func__, nav_area.dfs_path_count);

	//Com_Printf("%s total_areas[%d]  start[%d]  curr[%d]  goal[%d]  path_taken[%d]\n", __func__, nav_area.total_areas, nav_area.start_area, nav_area.current_area, nav_area.goal_area, nav_area.path_taken);

	// Print all paths
	/*
	if (nav_area.dfs_path_count)
	{
		for (int i = 0; i < nav_area.dfs_path_count; i++)
		{
			Com_Printf("%s [%d]Path: ", __func__, i);
			for (int a = 0; a < MAX_NAV_AREAS; a++)
			{
				if (nav_area.dfs_paths[i][a] == INVALID)
					break;

				Com_Printf("%d ", nav_area.dfs_paths[i][a]);
			}
			Com_Printf("\n");
		}
	}
	*/

	// Instead of picking a random path, pick the less used path...

	// Pick a random path
	if (nav_area.dfs_path_count) // Success
	{
		/*
		self->bot.goal_node = goal_node; // Set the goal node to reach
		self->bot.path_taken = rand() % nav_area.dfs_path_count; // Get rand path
		self->bot.state = BOT_MOVE_STATE_NAV_NEXT;

		// Precalculate all edge-to-edge nodes
		Com_Printf("%s Found a valid edge path [AREA][NODE]: [%d][%d] ", __func__, self->bot.current_area, self->bot.current_node);
		int prev_area = self->bot.current_area;
		int next_area;
		int next_node;
		for (int i = 0; i < MAX_NAV_AREAS; i++)
		{
			next_area = nav_area.dfs_paths[self->bot.path_taken][i];

			if (next_area == INVALID) // End of path
			{
				Com_Printf("[%d][%d] ", nodes[goal_node].area, goal_node);
				self->bot.next_area_nodes[i] = goal_node; // Last node
				self->bot.next_area_nodes_counter = i; // Total nodes in next_area_nodes[]
				self->bot.next_area_nodes[i + 1] = INVALID; // Terminate
				break;
			}
			else
			{
				next_node = BOTLIB_GetRandomEdgeConnection(prev_area, next_area);
				self->bot.next_area_nodes[i] = next_node;
				Com_Printf("[%d][%d] ", next_area, next_node);
			}

			prev_area = next_area;
		}
		Com_Printf("\n");
		*/

		//self->bot.next_area_node = BOTLIB_GetRandomEdgeConnection(self->bot.current_area, self->bot.next_area); // Get a random edge node
		//next_area_nodes

		BOTLIB_GetAreaPath(self, goal_node);

		return true; // Success
	}
	else // Failure
	{
		self->bot.path_taken = INVALID;
		//Com_Printf("%s NO VALID PATH from area %d to %d\n", __func__, self->bot.current_area, self->bot.goal_area);
		return false; // Faulure
	}

	return true; // Success
}

// 
qboolean BOTLIB_MakeAreaPath(edict_t* self, int goal_node, qboolean use_heatmap)
{
	self->bot.next_area_nodes_counter = 0;
	int prev_area = self->bot.current_area;
	int next_area;
	int next_node;
	qboolean debug = false;

	if (debug) Com_Printf("%s Found a valid edge path [AREA][NODE]: [%d][%d] ", __func__, self->bot.current_area, self->bot.current_node);
	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		next_area = nav_area.dfs_paths[self->bot.path_taken][i];

		self->bot.area_heatmap[next_area] += 1; // Increase heatmap
		if (use_heatmap) // If we're avoiding an area
		{
			if (self->bot.area_heatmap[next_area] >= 3)
			{
				//Com_Printf("\n");
				//self->bot.area_heatmap[next_area] = 0; // Reset heatmap
				//return false; // Failure
			}
		}

		if (next_area == INVALID) // End of path
		{
			if (debug) Com_Printf("[%d][%d] ", nodes[goal_node].area, goal_node);
			self->bot.next_area_nodes[i] = goal_node; // Last node
			self->bot.next_area_nodes_counter = i + 1; // Total nodes in next_area_nodes[]
			self->bot.next_area_nodes[i + 1] = INVALID; // Terminate
			break;
		}
		else
		{
			next_node = BOTLIB_GetRandomEdgeConnection(prev_area, next_area);
			if (next_node == INVALID)
			{
				if (debug) Com_Printf("[%d][%d] ", next_area, next_node);
				if (debug) Com_Printf(" -- INVALID PATH\n");
				return false; // Failure to find connected edge connection
			}
			self->bot.next_area_nodes[i] = next_node;
			if (debug) Com_Printf("[%d][%d] ", next_area, next_node);
		}

		prev_area = next_area;
	}
	if (debug) Com_Printf("\n");

	return true; // Success
}


// Precalculate all edge-to-edge nodes
void BOTLIB_GetAreaPath(edict_t* self, int goal_node)
{
	self->bot.goal_node = goal_node; // Set the goal node to reach
	////self->bot.state = BOT_MOVE_STATE_NAV_NEXT;

	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		self->bot.path_taken = rand() % nav_area.dfs_path_count; // Get rand path
		if (BOTLIB_MakeAreaPath(self, goal_node, false) == true) // Success
			break;
	}

	/*
	qboolean success = false;
	for (int i = 0; i < nav_area.dfs_path_count; i++)
	{
		self->bot.path_taken = i; // Cycle through paths
		success = BOTLIB_MakeAreaPath(self, goal_node, true);
		if (success)
			break;
	}
	if (success == false)
	{
		self->bot.path_taken = rand() % nav_area.dfs_path_count; // Get rand path
		BOTLIB_MakeAreaPath(self, goal_node, false);
	}
	*/
}

qboolean BOTLIB_GetNextAreaNode(edict_t *self)
{
	if (self->bot.next_area_nodes_counter == self->bot.next_area_counter)
	{
		Com_Printf("%s Successfully made a path to goal node [%d]\n", __func__, self->bot.goal_node);
		return false;
	}

	self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update current node
	if (self->bot.current_node == INVALID || self->bot.goal_node == INVALID)
	{
		Com_Printf("%s INVALID PATH\n", __func__);
		self->bot.goal_node = INVALID;
		self->bot.state = BOT_MOVE_STATE_NAV; // Get new nav goal
		return false;
	}
	int prev_node = self->bot.current_node;
	if (self->bot.next_area_counter)
	{
		prev_node = self->bot.node_list[self->bot.node_list_count - 1];
		//prev_node = self->bot.next_area_nodes[self->bot.next_area_counter - 1];
	}
	int next_node = self->bot.next_area_nodes[self->bot.next_area_counter];
	if (next_node != INVALID)
	{
		//if (BOTLIB_DijkstraAreaPath(self, prev_node, next_node, false, nodes[prev_node].area, self->bot.next_area_counter))
		if (BOTLIB_DijkstraPath(self, prev_node, next_node, false))
		{
			self->bot.next_area_node = next_node;
			self->bot.next_area_counter++;

			//Com_Printf("%s [%d] prev_node[%d] prev_area[%d] next_node[%d] next_area[%d] curr[%d] goal[%d]\n", __func__, self->bot.next_area_counter, prev_node, nodes[prev_node].area, next_node, nodes[next_node].area, self->bot.current_node, self->bot.goal_node);

			//if (self->bot.goal_node == nodes[next_node].nodenum)
			//	Com_Printf("%s Successfully made a path to goal node [%d]\n", __func__, self->bot.goal_node);

			if (next_node == self->bot.goal_node) // Reached goal node
			{
				self->bot.current_node = self->bot.node_list[0];
				self->bot.next_node = self->bot.node_list[1];
				self->node_timeout = 0;
				self->bot.state = BOT_MOVE_STATE_MOVE;
				return false;
			}

			return true;
		}
		else
		{
			//Com_Printf("%s Could not visit node -- failed to reach prev_node[%d] prev_area[%d] next_node[%d] next_area[%d] goal[%d]\n", __func__, prev_node, nodes[prev_node].area, next_node, nodes[next_node].area, self->bot.goal_node);
			self->bot.goal_node = INVALID;
			self->bot.state = BOT_MOVE_STATE_NAV; // Get new nav goal
			return false;
		}
	}

	return false;

	/*
	int last_node = self->bot.next_area_nodes[self->bot.next_area_nodes_counter];
	float dist = VectorDistance(nodes[self->bot.current_node].origin, nodes[last_node].origin);
	if ( dist < 128 )
	{
		//Com_Printf("%s Success, reached goal node[%d] from node[%d] dist[%f]\n", __func__, self->bot.goal_node, self->bot.current_node, dist);
		self->bot.goal_node = INVALID;
		self->bot.state = BOT_MOVE_STATE_NAV; // Get new nav goal
		return true;
	}
	*/

	//Com_Printf("%s Failed to reach next node [%d] curr_node[%d] goal[%d]\n", __func__, next_node, self->bot.current_node, self->bot.goal_node);


#if 0



	if (self->bot.path_taken == INVALID)
	{
		Com_Printf("%s No valid paths found\n", __func__);
		self->bot.goal_node = INVALID;
		self->bot.state = BOT_MOVE_STATE_NAV; // Get new nav goal
		return;
	}

	/*
	float dist = 999999; 
	if (nav_area.next_area_node != INVALID)
	{
		dist = VectorDistance(nodes[nav_area.next_area_node].origin, nodes[self->bot.current_node].origin);
		if (dist > 256.0f)
			return;
	}
	*/

	// Update current area
	self->bot.current_area = nodes[self->bot.current_node].area;

	 // Update next area
	self->bot.next_area = nav_area.dfs_paths[self->bot.path_taken][self->bot.next_area_counter];
	self->bot.next_area_counter++;

	// Check if final area reached
	if (self->bot.next_area == INVALID)
	{
		if (self->bot.goal_node != INVALID && BOTLIB_CanVisitNode(self, nodes[self->bot.goal_node].nodenum, false, nodes[self->bot.current_node].area))
		{
			Com_Printf("%s Goal area reached. Heading for goal node [%d] within the final area [%d]\n", __func__, self->bot.goal_node, self->bot.current_area);
			return;
		}
		else
		{
			Com_Printf("%s Failed to reach final goal node[%d]: g_area[%d], c_area[%d], n_area[%d]\n", __func__, self->bot.goal_node, self->bot.goal_area, self->bot.current_area, self->bot.next_area);
			self->bot.goal_node = INVALID;
			self->bot.state = BOT_MOVE_STATE_NAV; // Get new nav goal
			return;
		}
	}

	//	Com_Printf("%s c_area[%d] n_area[%d] g_area[%d]\n", __func__, self->bot.current_area, self->bot.next_area, self->bot.goal_area);

	/*
	Com_Printf("%s Path taken: ", __func__);
	for (int a = 0; a < MAX_NAV_AREAS; a++)
	{
		if (nav_area.dfs_paths[self->bot.path_taken][a] == INVALID)
			break;

		Com_Printf("%d ", nav_area.dfs_paths[self->bot.path_taken][a]);
	}
	Com_Printf("\n");
	*/


	//BOTLIB_UpdateAllAreaEdges(); // Update edge connections
	self->bot.next_area_node = BOTLIB_GetRandomEdgeConnection(self->bot.current_area, self->bot.next_area); // Get a random edge node
	//Com_Printf("%s c_node[%d] n_node[%d] c_area[%d] n_area[%d] g_area[%d] path_taken[%d]\n", __func__, self->bot.current_node, self->bot.next_area_node, self->bot.current_area, self->bot.next_area, self->bot.goal_area, self->bot.path_taken);
	//Com_Printf("%s goal_area[%d]  current_area[%d] --> next_area[%d]\n", __func__, self->bot.goal_area, self->bot.current_area, self->bot.next_area);

	if (self->bot.next_area_node != INVALID)
	{
		{
			Com_Printf("%s heading to next node[%d] area[%d] goal[%d]\n", __func__, nodes[self->bot.next_area_node].nodenum, self->bot.next_area, self->bot.goal_node);
			return;
		}
	}

	Com_Printf("%s Failed to reach NextAreaNode -- goal N[%d] A[%d] -- curr N[%d] A[%d] -- next N[%d] A[%d] \n", __func__, self->bot.goal_node, self->bot.goal_area, self->bot.current_node, self->bot.current_area, self->bot.next_area_node, self->bot.next_area);
#endif
}

// Stores all the nodes within an area that connect to an external area (edge nodes)
// 1) Wipe old edge data
// 2) Find and store edge nodes
void BOTLIB_UpdateAllAreaEdges(void)
{
	for (int a = 0; a < MAX_NAV_AREAS; a++)
	{
		for (int e = 0; e < MAX_NAV_AREAS_EDGES; e++)
		{
			DFS_area_edges[a][e] = INVALID;
		}
	}

	int targetNode;
	for (int i = 0; i < numnodes; i++)
	{
		for (int l = 0; l < nodes[i].num_links; l++)
		{
			targetNode = nodes[i].links[l].targetNode;
			if (nodes[i].area != nodes[targetNode].area) // If node links to a node outside of its own area
			{
				for (int e = 0; e < MAX_NAV_AREAS_EDGES; e++) // Search area edges
				{
					if (DFS_area_edges[nodes[i].area][e] == INVALID) // Find free spot to store a new edge
					{
						//Com_Printf("%s area[%d to %d] node[%d to %d]\n", __func__, nodes[i].area, nodes[targetNode].area, i, targetNode);
						DFS_area_edges[nodes[i].area][e] = targetNode; // Store the edge node
						break;
					}
				}
			}
		}
	}
}

// Get a random edge node from two connected areas
// Returns a random edge node that is inside the second area (so the bot traverses just inside the next area)
int BOTLIB_GetRandomEdgeConnection(int area_1, int area_2)
{
	int a2n; // Area 2 node
	int ec = 0; // Edge counter
	int edges[MAX_NAV_AREAS_EDGES] = { INVALID }; // Store the edges that connect from A1 to A2

	// Find and store all connected nodes from A1 to A2
	for (int e = 0; e < MAX_NAV_AREAS_EDGES; e++) // Search area edges
	{
		if (DFS_area_edges[area_1][e] == INVALID) // End of edges stored
			break;

		a2n = DFS_area_edges[area_1][e];
		if (nodes[a2n].area == area_2) // Find all areas that connect from A1 to A2
		{
			//Com_Printf("%s area[%d to %d] node[%d] counter[%d]\n", __func__, area_1, area_2, a2n, ec);
			edges[ec++] = a2n; // Store the area_2 node
		}
	}

	if (ec)
		return edges[rand() % ec]; // Return a random edge node
	else
		return INVALID; // No connected edge nodes found
}

// Finds ALL possible paths from current to destination using the Depth-First Search (DFS) algorithm
void BOTLIB_DepthFirstSearch(edict_t* self, int current, int destination)
{
	if (current == destination)
	{
		if (nav_area.dfs_path_count >= MAX_NAV_AREAS_PATHS)
		{
			//Com_Printf("%s ERROR: Cannot add to DFS_paths[] because DFS_path_count >= %d\n", __func__, MAX_NAV_AREAS_PATHS);
			return;
		}
		if (nav_area.dfs_len >= MAX_NAV_AREAS)
		{
			Com_Printf("%s ERROR: Cannot add to DFS_paths[] because DFS_len >= %d\n", __func__, MAX_NAV_AREAS);
			return;
		}

		if (nav_area.dfs_len) // If there's an actual path
		{
			// Check if path passes through itself (loops back and touches the starting area again)
			// Don't cross the streams! It would be very bad! Egon.
			for (int i = 0; i < nav_area.dfs_len; i++)
			{
				if (self->bot.start_area == nav_area.dfs_path[i]) // If path crosses start_area again
				{
					return; // Reject path
				}

				for (int j = 0; j < nav_area.dfs_len; j++)
				{
					if (i == j) continue;

					if (nav_area.dfs_path[j] == nav_area.dfs_path[i]) // If path crosses same area twice
					{
						return; // Reject path
					}
				}
			}


			for (int i = 0; i < nav_area.dfs_len; i++)
			{
				nav_area.dfs_paths[nav_area.dfs_path_count][i] = nav_area.dfs_path[i]; // Copy from tmp path to full path array
			}
			nav_area.dfs_path_count++;
		}
		
		return;
	}
	for (int i = 0; i < MAX_NAV_AREAS; i++)
	{
		if (nav_area.adjacency_matrix[current][i] != INVALID && !nav_area.dfs_visited[i])
		{
			nav_area.dfs_path[nav_area.dfs_len++] = i; // Copy to tmp path
			nav_area.dfs_visited[i] = true; // Visited
			BOTLIB_DepthFirstSearch(self, i, destination); // Recursive search (cycles)
			nav_area.dfs_len--; // Backtrack
			nav_area.dfs_visited[i] = false; // Unvisit
		}
	}
}

// Check if area already exists
qboolean BOTLIB_AreaExists(int area_num)
{
	for (int i = 0; i < numnodes; i++)
	{
		if (nodes[i].area == area_num)
		{
			return true; // Area exists
		}
	}
	return false; // Area does not exist
}
// Attempts to automatically add areas to map
void BOTLIB_AutoArea(edict_t* self)
{
	int curr_area_num = 0;
	for (int i = 0; i < numnodes; i++)
	{
		if (nodes[i].area == 0)
		{
			self->bot.walknode.selection_node_count = 0;
			BOTLIB_GroupConnectedNodeArea(self, i); // Try grouping nodes from this node

			if (self->bot.walknode.selection_node_count)
			{
				//Check if area exists
				for (; curr_area_num < MAX_NAV_AREAS; curr_area_num++)
				{
					if (BOTLIB_AreaExists(curr_area_num) == false)
						break;
				}
				if (curr_area_num + 1 >= MAX_NAV_AREAS)
				{
					Com_Printf("%s WARNING: Cannot add more areas, maximum allowed %d\n", __func__, MAX_NAV_AREAS);
					break;
				}
				// Found a free area
				for (int s = 0; s < self->bot.walknode.selection_node_count; s++)
				{
					nodes[self->bot.walknode.selection_nodes[s]].area = curr_area_num;
					nodes[self->bot.walknode.selection_nodes[s]].area = curr_area_num;
				}
			}
		}
	}

	if (curr_area_num)
	{
		BOTLIB_FreeAreaNodes();
		BOTLIB_InitAreaNodes();
		BOTLIB_InitAreaConnections();
	}
}

// Randomize area colors
void BOTLIB_RandomizeAreaColors(void)
{
	int color;
	for (int i = 0; i < MAX_NAV_AREAS; ++i)
	{
		// Try to randomize colors while keeping at least one of the R,G,B values above 128+
		int rng = rand() % 3;
		if (rng == 0)
			color = MakeColor((rand() % 255), (rand() % 255), (rand() % 128) + 128, 255); // Randomize color
		else if (rng == 1)
			color = MakeColor((rand() % 255), (rand() % 128) + 128, (rand() % 255), 255); // Randomize color
		else
			color = MakeColor((rand() % 128) + 128, (rand() % 255), (rand() % 255), 255); // Randomize color

		for (int n = 0; n < MAX_NAV_AREAS_NODES; n++)
		{
			if (nav_area.area_nodes[i][n] == INVALID)
				break;

			nodes[nav_area.area_nodes[i][n]].area_color = color;
		}
	}
}

// ====================================================================================
// ====================================================================================
// Create local area nodes memory alloc
int total_local_area_nodes;
qboolean* local_area_nodes;
qboolean BOTLIB_MallocLocalAreaNodes(void)
{
	total_local_area_nodes = 0;
	local_area_nodes = NULL;

	//nodes = (node_t*)malloc(sizeof(node_t) * (numnodes + 1)); // Alloc memory
	local_area_nodes = (qboolean*)malloc(sizeof(local_area_nodes) * (numnodes + 1));
	if (local_area_nodes == NULL) 
	{
		Com_Printf("%s Memory allocation failed for local_area_nodes\n", __func__);
		local_area_nodes = NULL;
		return false;
	}

	for (int i = 0; i < numnodes; i++)
	{
		local_area_nodes[i] = false; // Init to false
	}

	return true; // Success
}
// Free local area nodes memory alloc
void BOTLIB_FreeLocalAreaNodes(void)
{
	if (local_area_nodes)
	{
		free(local_area_nodes);
		local_area_nodes = NULL; // Nullify dangling pointer
	}
}
// A recurrsive function that finds all nodes that are connected and pathed to the initial `start_node` and are of the same area num
void BOTLIB_GroupConnectedNodeArea_r(edict_t* self, int next_node) //, vec3_t normal)
{
	for (int i = 0; i < numnodes; i++)
	{
		if (local_area_nodes[i] == true)
			continue;

		if (nodes[i].area == nodes[next_node].area)
		{
			for (int l = 0; l < nodes[i].num_links; l++)
			{
				if (nodes[i].links[l].targetNode == next_node)
				{
					if (self->bot.walknode.selection_node_count >= MAX_NAV_AREAS_NODES)
						break;

					// Check if node already included in selection
					qboolean added = false;
					for (int s = 0; s < self->bot.walknode.selection_node_count; s++)
					{
						if (self->bot.walknode.selection_nodes[s] == i)
						{
							added = true;
							break;
						}
					}
					if (added)
						continue; // Skip adding because we already have it

					// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
					// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

					// Need to test if the two nodes are on the same plane/normal/height/distance

					if (VectorDistance(nodes[i].origin, nodes[self->bot.walknode.selection_nodes[0]].origin) > 1024)
						continue;

					// Check hight diff of connected node - ignore nodes with large differences
					//Com_Printf("%s FABS[%f]\n", __func__, fabs(nodes[i].origin[2] - nodes[next_node].origin[2])); 
					if (fabs(nodes[i].origin[2] - nodes[next_node].origin[2]) > 32)
						continue;

					// Only nodes on the same normal - with a small epsilon
					if (BOTLIB_VectorCompare(nodes[i].normal, nodes[next_node].normal, 0.1) == false)
						continue;


					//Com_Printf("%s [%f %f %f] vs [%f %f %f]\n", __func__, nodes[next_node].normal[0], nodes[next_node].normal[1], nodes[next_node].normal[2], nodes[i].normal[0], nodes[i].normal[1], nodes[i].normal[2]);
					
					// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
					// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

					local_area_nodes[next_node] = true; // Flag node as true
					//total_local_area_nodes++;

					self->bot.walknode.selection_nodes[self->bot.walknode.selection_node_count] = i;
					self->bot.walknode.selection_node_count++;

					BOTLIB_GroupConnectedNodeArea_r(self, i); // Scan the connected node
				}
			}
		}
	}
}

void BOTLIB_GroupConnectedNodeArea(edict_t* self, int start_node)
{
	if (BOTLIB_MallocLocalAreaNodes() == false) // Alloc memory
		return; // Failure

	local_area_nodes[start_node] = true; // Initiate starting node to true
	//total_local_area_nodes++;

	/*
	trace_t tr = gi.trace(tv(nodes[start_node].origin[0], nodes[start_node].origin[1], nodes[start_node].origin[2] + 32), NULL, NULL, tv(nodes[start_node].origin[0], nodes[start_node].origin[1], nodes[start_node].origin[2] - 256), NULL, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorCopy(tr.plane.normal, nodes[start_node].normal);
	}

	Com_Printf("%s [%f %f %f]\n", __func__, tr.plane.normal[0], tr.plane.normal[1], tr.plane.normal[2]);
	*/

	//BOTLIB_SetAllNodeNormals(); // Set all the normals

	BOTLIB_GroupConnectedNodeArea_r(self, start_node); // Recurrsively check all nodes

	//total_local_area_nodes
	//Com_Printf("%s total_local_area_nodes[%d]\n", __func__, total_local_area_nodes);

	//for (int i = 0; i < numnodes; i++)
	{
		//if (local_area_nodes[i] == true)
		{
		}
	}

	BOTLIB_FreeLocalAreaNodes(); // Free memory
}
// ====================================================================================
// ====================================================================================

//rekkie -- Dijkstra Area Pathing -- s
// Currently if path_randomization is enabled, the bot may not be able to reach its target,
// therefore it's only really used when heading to a random node.
// Area: specify the area we're working with; only searching nodes within this area
// build_new_path: If we're building a new path or adding to an existing path
qboolean BOTLIB_DijkstraAreaPath(edict_t* ent, int from, int to, qboolean path_randomization, int area, qboolean build_new_path)
{
	//Com_Printf("%s from[%d] to[%d]\n", __func__, from, to);

	// Sanity check
	if (from == INVALID || to == INVALID || area == INVALID)
		return false;

	// Check for null pointers
	if (nav_area.area_nodes == NULL || nav_area.area_nodes[0] == NULL)
		return false;

	//if (build_new_path)
	{
		//ent->bot.node_list_count = 0; // Reset the list of nodes to zero
		//ent->bot.node_list_current = 0; // Set the starting point to zero
	}

	AntInitSearch(ent); // Clear out the path storage

	for (int i = 0; i < numnodes; i++)
		nodeweight[i] = NAV_INFINITE;

	int	newNode = INVALID; // Stores the node being tested
	int atNode = from; // Current node we're visiting
	botlib_sll_t openList; // Locally declared OPEN list
	openList.head = openList.tail = NULL; // Init the list

	float weight; // Weight of the node we're visiting
	nodeweight[atNode] = 0; // Set the weight to 0 (distance from self to self is 0)
	nodefrom[atNode] = atNode; // Set the parent node to itself
	//nodeused[atNode] = true; // Add the starting node to the visited CLOSED list
	SLLpush_back(&openList, from); // Store it

	//int searched_nodes = 0;

	while (!SLLempty(&openList) && newNode != to) // While there are nodes on the OPEN list
	{
		atNode = SLLfront(&openList); // Get the next node
		if (atNode == to) // If the next node is the goal node
		{
			//Com_Printf("%s [%d] next node found a path\n", __func__, level.framenum);
			break; // We found a path
		}

		//searched_nodes++;
		//Com_Printf("%s NODE[%d] AREAS: ", __func__, atNode);
//#if 0
		// Find the node with the lowest weight
		float lowest_cost = NAV_INFINITE;
		int lowest_node = INVALID;
		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			newNode = nodes[atNode].links[i].targetNode; // Get the next linked node

			if (newNode == INVALID)
				continue;

			// Only search in current area
			if (newNode != to && nodes[newNode].area != area)
				continue;

			//Com_Printf(" %d ", nodes[newNode].area);

			if (nodes[atNode].links[i].targetNodeType == NODE_POI_LOOKAT) // POI look at nodes are not real links
				continue;

			// Check if the node can be seen by enemy's path
			//if (BOTLIB_NodeCanSeeEnemyPath(ent, atNode)) continue;

			if (path_randomization)
			{
				// Consider skipping this node if it has many links and its weight is high
				if (nodes[newNode].num_links > 3 && nodes[newNode].weight > random())
					continue;

				// [1] Less expensive
				// Randomize paths
				if (newNode != to && random() <= 0.2) // how often we skip checking a linked node
					continue;

				// [2] Expensive
				// Try to occasionally break up paths, expensive operation
				/*
				if (random() < 0.01)
				{
					// Check if node is free, otherwise check next link
					if (Botlib_Nav_NodeInUse(ent, newNode) == false)
						if (random() < 0.1)
							continue;
				}
				*/
			}

			if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
			{
				if (nodeweight[newNode] == NAV_INFINITE)
				{
					lowest_node = INVALID;
					break;
				}

				if (nodeweight[newNode] < lowest_cost)
				{
					lowest_cost = nodeweight[newNode];
					lowest_node = newNode;
					weight = nodeweight[atNode] + nodes[atNode].links[i].cost;
				}
			}
		}
		//Com_Printf("\n");

		// Use the shortest path
		if (lowest_node != INVALID)
		{
			// If weight is less than the weight of the new node
			if (weight < nodeweight[lowest_node])
			{
				if (lowest_node != from && lowest_node != to)
				{
					// Check if the node can be seen by enemy's path
					//if (BOTLIB_NodeCanSeeEnemyPath(ent, atNode)) continue;

					if (path_randomization)
					{
						// [1] Less expensive
						// Randomize paths
						if (lowest_node != to && random() <= 0.2) // how often we skip checking a linked node
							continue;
					}
				}

				nodeweight[lowest_node] = weight; // Update the weight
				nodefrom[lowest_node] = atNode; // Update the parent node (open list)
				SLLpush_back(&openList, lowest_node); // Store it
				//Com_Printf("%s lowest_node[%d]\n", __func__, lowest_node);
			}

			if (lowest_node == to) // If node being linked is the goal node
			{
				//Com_Printf("%s [%d] lowest_node found a path\n", __func__, level.framenum);
				break; // We found a path
			}
		}
		else
//#endif
		{
			for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
			{
				newNode = nodes[atNode].links[i].targetNode; // Get the next linked node

				if (newNode == INVALID)
					continue;

				// Only search in current area
				if (newNode != to && nodes[newNode].area != area)
					continue;

				if (nodes[atNode].links[i].targetNodeType == NODE_POI_LOOKAT) // POI look at nodes are not real links
					continue;
				

				// Get the weight of the node we're visiting + the cost to the new node
				weight = nodeweight[atNode] + nodes[atNode].links[i].cost;

				if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
				{
					// If weight is less than the weight of the new node
					if (weight < nodeweight[newNode])
					{
						// Check if the node can be seen by enemy's path
						//if (BOTLIB_NodeCanSeeEnemyPath(ent, atNode)) continue;

						// Consider skipping this node if it has many links and its weight is high
						if (nodes[newNode].num_links > 3 && nodes[newNode].weight > random())
							continue;

						if (path_randomization)
						{
							// [1] Less expensive
							// Randomize paths
							if (newNode != to && random() <= 0.2) // how often we skip checking a linked node
								continue;
						}

						nodeweight[newNode] = weight; // Update the weight
						nodefrom[newNode] = atNode; // Update the parent node (open list)
						SLLpush_back(&openList, newNode); // Store it
					}
				}

				if (newNode == to) // If node being linked is the goal node
				{
					//Com_Printf("%s [%d] normal found a path\n", __func__, level.framenum);
					break; // We found a path
				}

			}
		}
		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)
	}

	//Com_Printf("%s searched_nodes[%d]\n", __func__, searched_nodes);

	// Free up the memory we allocated
	SLLdelete(&openList);

	// Optimise stored path with this new information
	if (newNode == to)
	{
		// Make the path using the fromnode array pushing node numbers on in reverse order so we can SLLpop_front them back later
		SLLpush_front(&ent->pathList, newNode);

		// This happens when a bot dies and respawns, nodefrom[newNode] spits out INVALID. Check if INVALID to avoid accessing [-1] array
		////if (nodefrom[newNode] != INVALID)
		{
			////path_table[nodefrom[newNode]][newNode] = newNode; // Set the 'newNode' path in node array because this is shortest path
			//Com_Printf("%s path_table[ nodefrom[%d] ] [%d] = %d\n", __func__, newNode, newNode, newNode);
		}

		//Com_Printf("%s path_table[ nodefrom[%d] ] [%d] = %d\n", __func__, newNode, newNode, newNode);
		//int prev_newNode = newNode;

		// We earlier set our start node to INVALID to set up the termination
		// Check if there is a path, and it's not the node we're standing on (safety check)
		while ((newNode = nodefrom[newNode]) != INVALID && (newNode != from))
		{
			//Com_Printf("%s %d = nodefrom[%d]\n", __func__, newNode, prev_newNode);
			//prev_newNode = newNode;

			// The bot has taken this node so increase its weight so other bots will consider skipping this node if its weight is high
			if (nodes[newNode].num_links > 3)
			{
				nodes[newNode].weight += 0.1;
				if (nodes[newNode].weight > 0.5) // Limit max weight
					nodes[newNode].weight = 0; // Reset back to normal weight
				if (nodes[newNode].weight < 0) // Sanity check
					nodes[newNode].weight = 0; // Reset back to normal weight
			}

			SLLpush_front(&ent->pathList, newNode); // Push it onto the pathlist
			////path_table[nodefrom[newNode]][to] = newNode; // Set the path in the node array to match this shortest path

			//Com_Printf("%s path_table[ nodefrom[%d][%d] = %d\n", __func__, newNode, to, newNode);
		}

		//Com_Printf("%s EXIT PATH\n\n", __func__);

		// Each time a path is found, make a copy
		ent->bot.node_list_count = BOTLIB_SLL_Query_All_Nodes(ent, &ent->pathList, ent->bot.node_list, MAX_NODELIST); // Retrieve the nodes in the list
		ent->bot.node_list_current = 0;
		if (ent->bot.node_list_count) // Set the current and next nodes
		{
			ent->bot.current_node = ent->bot.node_list[0];
			ent->bot.next_node = ent->bot.node_list[1];
		}		
		ent->bot.node_random_path = path_randomization; // Note down if the bot was taking a random or direct path

		if (0)
		{
			if (1)
			{
				Com_Printf("%s s[%d] g[%d] node_list[", __func__, from, to);
				for (int i = 0; i < ent->bot.node_list_count; i++)
				{
					//Com_Printf(" %d[%.1f] ", ent->bot.node_list[i], nodes[ent->bot.node_list[i]].weight);
					Com_Printf(" %d ", ent->bot.node_list[i]);
				}
				Com_Printf(" ]\n");
			}
			else
				Com_Printf("%s from[%d] to[%d]\n", __func__, from, to);
			//Com_Printf("%s found a path\n", __func__);
		}

		if (SLLempty(&ent->pathList))
			return false; // Failure

		return true; // Success
	}

	// If we get to this point, it means our path failed
	// If path_randomization was true, then try again without it
	if (path_randomization) //If using rand pathing, and it fails, try again *ONCE* without it
	{
		BOTLIB_DijkstraAreaPath(ent, from, to, false, area, build_new_path);
	}
	// Else: just fail it completely
	{
		//Com_Printf("%s failed to find a path from %d to %d\n", __func__, from, to);
		return false; // Failure
	}
}
//rekkie -- Dijkstra Area Pathing -- e

//rekkie -- Dijkstra pathing -- s
// Currently if path_randomization is enabled and the bot fails to path to its target,
// BOTLIB_DijkstraPath() is run again with path_randomization turned off
qboolean BOTLIB_DijkstraPath(edict_t* ent, int from, int to, qboolean path_randomization)
{
	//Com_Printf("%s from[%d] to[%d] for %s, random: %d\n", __func__, from, to, ent->client->pers.netname, path_randomization);

	// Sanity check
	if (from == INVALID || to == INVALID) {
		gi.dprintf("both from and to nodes were invalid, to: %i, from: %i\n", to, from);
		return false;
	}

	AntInitSearch(ent); // Clear out the path storage
	for (int i = 0; i < numnodes; i++)
		nodeweight[i] = NAV_INFINITE;


	int	newNode = INVALID; // Stores the node being tested
	int atNode = from; // Current node we're visiting
	botlib_sll_t openList; // Locally declared OPEN list
	openList.head = openList.tail = NULL; // Init the list

	float weight; // Weight of the node we're visiting
	nodeweight[atNode] = 0; // Set the weight to 0 (distance from self to self is 0)
	nodefrom[atNode] = atNode; // Set the parent node to itself
	//nodeused[atNode] = true; // Add the starting node to the visited CLOSED list
	SLLpush_back(&openList, from); // Store it

	while (!SLLempty(&openList) && newNode != to) // While there are nodes on the OPEN list
	{
		atNode = SLLfront(&openList); // Get the next node
		if (atNode == to) // If the next node is the goal node
		{
			//Com_Printf("%s [%d] next node found a path\n", __func__, level.framenum);
			break; // We found a path
		}

		/*
		// Update node weights, but don't add them to the list
		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			if (nodes[atNode].links[i].targetNode == INVALID)
				continue;

			newNode = nodes[atNode].links[i].targetNode; // Get the next linked node
			if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
			{
				weight = nodeweight[atNode] + nodes[atNode].links[i].cost;

				if (nodeweight[newNode] == NAV_INFINITE || nodeweight[newNode] < weight)
				{
					nodeweight[newNode] = weight; // Update the weight
					nodefrom[newNode] = atNode; // Update the parent node (open list)
					//SLLpush_back(&openList, newNode); // Add it to the open list
				}
			}
		}

		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)

		// Find the node link with the shortest path
		float lowest_cost = NAV_INFINITE;
		int lowest_node = INVALID;
		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			if (nodes[atNode].links[i].targetNode == INVALID)
				continue;

			newNode = nodes[atNode].links[i].targetNode; // Get the next linked node
			if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
			{
				if (nodeweight[newNode] <= lowest_cost)
				{
					lowest_cost = nodeweight[newNode];
					lowest_node = newNode;
				}
			}
		}

		// Add the shortest path to the open list
		if (lowest_node != INVALID)
		{
			SLLpush_back(&openList, lowest_node); // Add it to the open list
			//Com_Printf("%s [%d to %d] lowest_node[%d]\n", __func__, from, to, lowest_node);
		}

		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)
		*/



		//*
		// Find the node with the lowest weight
		float lowest_cost = NAV_INFINITE;
		int lowest_node = INVALID;
		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			if (nodes[atNode].links[i].targetNode == INVALID)
				continue;

			newNode = nodes[atNode].links[i].targetNode; // Get the next linked node

			if (nodes[atNode].links[i].targetNodeType == NODE_POI_LOOKAT) // POI look at nodes are not real links
				continue;

			// Check if the node can be seen by enemy's path
			//if (BOTLIB_NodeCanSeeEnemyPath(ent, atNode)) continue;

			if (path_randomization)
			{
				// Consider skipping this node if it has many links and its weight is high
				if (nodes[newNode].num_links > 3 && nodes[newNode].weight > random())
					continue;

				// [1] Less expensive
				// Randomize paths
				if (newNode != to && random() <= 0.2) // how often we skip checking a linked node
					continue;

				// [2] Expensive
				// Try to occasionally break up paths, expensive operation
				/*
				if (random() < 0.01)
				{
					// Check if node is free, otherwise check next link
					if (Botlib_Nav_NodeInUse(ent, newNode) == false)
						if (random() < 0.1)
							continue;
				}
				*/
			}

			if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
			{
				if (nodeweight[newNode] == NAV_INFINITE)
				{
					lowest_node = INVALID;
					break;
				}

				if (nodeweight[newNode] < lowest_cost)
				{
					lowest_cost = nodeweight[newNode];
					lowest_node = newNode;
					weight = nodeweight[atNode] + nodes[atNode].links[i].cost;
				}
			}
		}
		// Use the shortest path
		if (lowest_node != INVALID)
		{
			// If weight is less than the weight of the new node
			if (weight < nodeweight[lowest_node])
			{
				if (lowest_node != from && lowest_node != to)
				{
					// Check if the node can be seen by enemy's path
					//if (BOTLIB_NodeCanSeeEnemyPath(ent, atNode)) continue;

					if (path_randomization)
					{
						// [1] Less expensive
						// Randomize paths
						if (lowest_node != to && random() <= 0.2) // how often we skip checking a linked node
							continue;
					}
				}

				nodeweight[lowest_node] = weight; // Update the weight
				nodefrom[lowest_node] = atNode; // Update the parent node (open list)
				SLLpush_back(&openList, lowest_node); // Store it
				//Com_Printf("%s lowest_node[%d]\n", __func__, lowest_node);
			}

			if (lowest_node == to) // If node being linked is the goal node
			{
				//Com_Printf("%s [%d] lowest_node found a path\n", __func__, level.framenum);
				break; // We found a path
			}
		}
		else
		{
			for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
			{
				if (nodes[atNode].links[i].targetNode == INVALID)
					continue;
				if (nodes[atNode].links[i].targetNodeType == NODE_POI_LOOKAT) // POI look at nodes are not real links
					continue;

				newNode = nodes[atNode].links[i].targetNode; // Get the next linked node

				// Get the weight of the node we're visiting + the cost to the new node
				weight = nodeweight[atNode] + nodes[atNode].links[i].cost;

				if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
				{
					// If weight is less than the weight of the new node
					if (weight < nodeweight[newNode])
					{
						// Check if the node can be seen by enemy's path
						//if (BOTLIB_NodeCanSeeEnemyPath(ent, atNode)) continue;

						// Consider skipping this node if it has many links and its weight is high
						if (nodes[newNode].num_links > 3 && nodes[newNode].weight > random())
							continue;

						if (path_randomization)
						{
							// [1] Less expensive
							// Randomize paths
							if (newNode != to && random() <= 0.2) // how often we skip checking a linked node
								continue;
						}

						nodeweight[newNode] = weight; // Update the weight
						nodefrom[newNode] = atNode; // Update the parent node (open list)
						SLLpush_back(&openList, newNode); // Store it
					}
				}

				if (newNode == to) // If node being linked is the goal node
				{
					//Com_Printf("%s [%d] normal found a path\n", __func__, level.framenum);
					break; // We found a path
				}

			}
		}
		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)
		//*/



		/*
		for (int i = 0; i < nodes[atNode].num_links; i++) // Look at each link from this node
		{
			if (nodes[atNode].links[i].targetNode != INVALID)
			{
				newNode = nodes[atNode].links[i].targetNode; // Get the next linked node
				// Get the weight of the node we're visiting + the cost to the new node
				weight = nodeweight[atNode] + nodes[atNode].links[i].cost;

				if (nodeused[newNode] == false) // If the node has not been visited (CLOSED LIST)
				{
					// If weight is less than the weight of the new node
					if (weight < nodeweight[newNode])
					{
						nodeweight[newNode] = weight; // Update the weight
						nodefrom[newNode] = atNode; // Update the parent node (open list)
						SLLpush_back(&openList, newNode); // Store it
					}
				}

				if (newNode == to) // If node being linked is the goal node
				{
					break; // We found a path
				}
			}
		}
		
		SLLpop_front(&openList); // Remove atNode from the OPEN List
		nodeused[atNode] = true;  // Mark node as visited (CLOSED LIST)
		*/
	}


	// Free up the memory we allocated
	SLLdelete(&openList);

	// Optimise stored path with this new information
	if (newNode == to)
	{
		// Make the path using the fromnode array pushing node numbers on in reverse order so we can SLLpop_front them back later
		SLLpush_front(&ent->pathList, newNode);

		// This happens when a bot dies and respawns, nodefrom[newNode] spits out INVALID. Check if INVALID to avoid accessing [-1] array
		////if (nodefrom[newNode] != INVALID)
		{
			////path_table[nodefrom[newNode]][newNode] = newNode; // Set the 'newNode' path in node array because this is shortest path
			//Com_Printf("%s path_table[ nodefrom[%d] ] [%d] = %d\n", __func__, newNode, newNode, newNode);
		}

		//Com_Printf("%s path_table[ nodefrom[%d] ] [%d] = %d\n", __func__, newNode, newNode, newNode);
		//int prev_newNode = newNode;

		// We earlier set our start node to INVALID to set up the termination
		// Check if there is a path, and it's not the node we're standing on (safety check)
		while ((newNode = nodefrom[newNode]) != INVALID && (newNode != from))
		{
			//Com_Printf("%s %d = nodefrom[%d]\n", __func__, newNode, prev_newNode);
			//prev_newNode = newNode;

			// The bot has taken this node so increase its weight so other bots will consider skipping this node if its weight is high
			if (nodes[newNode].num_links > 3)
			{
				nodes[newNode].weight += 0.1;
				if (nodes[newNode].weight > 0.5) // Limit max weight
					nodes[newNode].weight = 0; // Reset back to normal weight
				if (nodes[newNode].weight < 0) // Sanity check
					nodes[newNode].weight = 0; // Reset back to normal weight
			}
			
			SLLpush_front(&ent->pathList, newNode); // Push it onto the pathlist
			////path_table[nodefrom[newNode]][to] = newNode; // Set the path in the node array to match this shortest path

			//Com_Printf("%s path_table[ nodefrom[%d][%d] = %d\n", __func__, newNode, to, newNode);
		}

		//Com_Printf("%s EXIT PATH\n\n", __func__);

		// Each time a path is found, make a copy
		// ent->bot.node_list_count = 0;
		// memset(ent->bot.node_list, 0, sizeof(ent->bot.node_list)); // Ensure the node list is cleared
		ent->bot.node_list_count = BOTLIB_SLL_Query_All_Nodes(ent, &ent->pathList, ent->bot.node_list, MAX_NODELIST); // Retrieve the nodes in the list
		ent->bot.node_list_current = 0;
		if (ent->bot.node_list_count) // Set the current and next nodes
		{
			//ent->bot.current_node = ent->bot.node_list[0];
			ent->bot.current_node = from;
			ent->bot.next_node = ent->bot.node_list[0];
		}
		// if(from != ent->bot.node_list[0])
		// 	gi.dprintf("Deviation in provided `from` parameter (%i) and node_list[0] value (%i)!\n", from, ent->bot.node_list[0]);

		ent->bot.node_random_path = path_randomization; // Note down if the bot was taking a random or direct path

		// Huge help in debugging, change this to 1
		if (0)
		{
			if (1)
			{
				gi.dprintf("from: %d, next: %d, to: %d\n", from, ent->bot.next_node, to);
				gi.dprintf("%s s[%d] g[%d] node_list[", __func__, from, to);
				for (int i = 0; i < ent->bot.node_list_count; i++)
				{
					//gi.dprintf(" %d[%.1f] ", ent->bot.node_list[i], nodes[ent->bot.node_list[i]].weight);
					gi.dprintf(" %d ", ent->bot.node_list[i]);
				}
				gi.dprintf(" ]\n");
			}
			else
				gi.dprintf("%s from[%d] to[%d]\n", __func__, from, to);
			//gi.dprintf("%s found a path\n", __func__);
		}

		if (SLLempty(&ent->pathList)){
			//gi.dprintf("SLLempty: %s failed to find a path from %d to %d\n", __func__, from, to);
			return false; // Failure
		}

		return true; // Success
	}

	// If we get to this point, it means our path failed
	// If path_randomization was true, then try again without it
	if (path_randomization) //If using rand pathing, and it fails, try again *ONCE* without it
	{
		//gi.dprintf("Trying again without path randomization\n");
		BOTLIB_DijkstraPath(ent, from, to, false);
	}
	// Else: just fail it completely
	{
		//gi.dprintf("Complete failure: %s failed to find a path from %d to %d\n", __func__, from, to);
		return false; // Failure
	}
}
//rekkie -- Dijkstra pathing -- e






























//=======================
// FindPath
//=======================
//
// Uses OPEN and CLOSED lists to conduct a search
// Many refinements planned
//
qboolean	AntFindPath(edict_t* ent, int from, int to)
{
	int counter = 0; // Link array counter
	//int link_counter = 0; // Link counter
	int	newNode = INVALID; // Stores the node being tested
	int atNode; // Structures for search
	botlib_sll_t	openList; // Locally declared OPEN list
	openList.head = openList.tail = NULL; // MUST do this!!

	// Safety first again - we don't want crashes!
	if (from == INVALID || to == INVALID)
		return false;

	// Put startnode on the OPEN list
	atNode = from;
	nodefrom[atNode] = INVALID;
	SLLpush_back(&openList, from);
	nodeused[from] = true;

	//Com_Printf("%s NEW PATH [%d to %d]\n", __func__, from, to);


	// While there are nodes on the OPEN list AND we are not at destNode
	while (!SLLempty(&openList) && newNode != to)
	{
		// Where we are
		atNode = SLLfront(&openList);

		//Com_Printf("%s atNode [%d]\n", __func__, atNode);

		// Safety check
		if (atNode <= INVALID)
			return false;


		// Get random start point (instead of always starting at 0)
		int start_point = 0; 
		//if (nodes[atNode].num_links) // If we have links
		//	start_point = rand() % (nodes[atNode].num_links); // start at random link
		//if (start_point + 1 > MAXLINKS)
		//	start_point = 0;

		//int start_point = 0;
		counter = start_point;

		// Go to next link, check for unused node
		//while (++counter != start_point)
		while (counter != nodes[atNode].num_links)
		{
			/*
			if (counter + 1 > MAXLINKS) //MAXLINKS
			{
				counter = 0;
				if (counter == start_point)
					break;
			}
			*/

			if (nodes[atNode].links[counter].targetNodeType == NODE_POI_LOOKAT) // POI look at nodes are not real links
				continue;

			// Using an array for FAST access to the path rather than a CLOSED list
			newNode = nodes[atNode].links[counter].targetNode; // Get next linked node

			//Com_Printf("%s counter[%d] newNode [%d]\n", __func__, counter, newNode);
			counter++;

			if (newNode == INVALID)
				continue;
			else
			{
				/*
				// Check for failure rates on links
				// If the fail rate is > 0, then we have a chance of skipping the node
				// The higher the targetFailures, the higher the chance of skipping
				//Com_Printf("%s %s: n[%d] l[%d] f[%d]\n", __func__, ent->client->pers.netname, newNode, counter, nodes[atNode].links[counter].targetFailures);
				if (nodes[atNode].links[counter].targetFailures > 1)
				{
					float percent = (1 / nodes[atNode].links[counter].targetFailures);
					if (random() < percent)
					{
						//Com_Printf("%s %s: Skipping node %d due to %d failures\n", __func__, ent->client->pers.netname, newNode, nodes[atNode].links[counter].targetFailures);
						continue;
					}
				}
				*/

				if (0)
				{
					// [1] Less expensive
					// Randomize paths
					//if (random() < 0.1) // how often we skip a targetNode
					//	continue;

					// [2] Expensive
					// Try to occasionally break up paths, expensive operation
					//if (random() < 0.01)
					{
						// Check if node is free, otherwise check next link
						if (Botlib_Nav_NodeInUse(ent, newNode) == false)
							if (random() < 0.1)
								continue;
					}
				}

				//if (nodes[atNode].links[counter].targetNodeType == NODE_LADDER_UP || nodes[atNode].links[counter].targetNodeType == NODE_LADDER_DOWN)
				//	continue;

				// If newNode NOT on open or closed list
				if (!nodeused[newNode])
				{
					nodeused[newNode] = true; // Mark node as used
					nodefrom[newNode] = atNode; // Add to OPEN list
					SLLpush_back(&openList, newNode); // Store it
				}

				if (newNode == to) // If node being linked is destNode then quit
					break;
			}
		}

		SLLpop_front(&openList); // Remove atNode from the OPEN List
	}



	/*
	// While there are nodes on the OPEN list AND we are not at destNode
	while (!SLLempty(&openList) && newNode != to)
	{
		counter = 0;

		// Where we are
		atNode = SLLfront(&openList);

		////Com_Printf("%s atNode [%d]\n", __func__, atNode);

		// Safety check
		if (atNode <= INVALID)
			return false;

		// Using an array for FAST access to the path rather than a CLOSED list
		newNode = nodes[atNode].links[counter].targetNode;

		// Process this node putting linked nodes on the OPEN list
		while (newNode != INVALID)
		{
			////Com_Printf("%s newNode [%d]\n", __func__, newNode);

			// If newNode NOT on open or closed list
			if (!nodeused[newNode])
			{
				nodeused[newNode] = true; // Mark node as used
				nodefrom[newNode] = atNode; // Add to OPEN list
				SLLpush_back(&openList, newNode); // Store it
			}
			
			if (newNode == to) // If node being linked is destNode then quit
				break;

			// Go to next link, check for unused node
			while (++counter < MAXLINKS)
			{
				newNode = nodes[atNode].links[counter].targetNode; // Get next linked node

				if(newNode == INVALID)
					break;

				// [1] Less expensive
				// Randomize paths
				if (nodes[atNode].links[counter].targetNodeType == NODE_LADDER_UP || nodes[atNode].links[counter].targetNodeType == NODE_LADDER_DOWN)
					break;
				if (random() < 0.25) // how often we skip a targetNode
					continue;
				else
					break;

				// [2] Expensive
				// Try to occasionally break up paths, expensive operation
				//if (random() < 0.88)
				//{
				//	// Check if node is free, otherwise check next link
				//	if (Botlib_Nav_NodeInUse(ent, newNode) == false)
				//		break;
				//}
				//else
				//	break;

			}
			if (counter >= MAXLINKS || newNode == INVALID)
				break;

			//if (++counter >= MAXLINKS) // Increase counter and check we're in bounds
			//	break;
			//newNode = nodes[atNode].links[counter].targetNode; // Get next linked node
		}
		
		SLLpop_front(&openList); // Remove atNode from the OPEN List
	}
	*/

	// Free up the memory we allocated
	SLLdelete(&openList);

	//Com_Printf("%s PATH newNode [%d] to [%d]\n", __func__, newNode, to);

	// Optimise stored path with this new information
	if (newNode == to)
	{
		// Make the path using the fromnode array pushing node numbers on in reverse order
		// so we can SLLpop_front them back later
		SLLpush_front(&ent->pathList, newNode);

		//rekkie -- DEV_1 -- s
		// This happens when a bot dies and respawns, nodefrom[to] spits out INVALID
		// Check if INVALID to avoid accessing [-1] array
		if (nodefrom[to] != INVALID)
		//rekkie -- DEV_1 -- e
			path_table[nodefrom[to]][to] = to; // Set the to path in node array because this is shortest path

		//Com_Printf("%s path_table[ nodefrom[%d][%d] = %d\n", __func__, to, to, to);
		//int prev_newNode = newNode;

		// We earlier set our start node to INVALID to set up the termination
		// Check if there is a path, and it's not the node we're standing on (safety check)
		while ((newNode = nodefrom[newNode]) != INVALID && (newNode != from))
		{
			//Com_Printf("%s %d = nodefrom[%d]\n", __func__, newNode, prev_newNode);
			//prev_newNode = newNode;

			// Push it onto the pathlist
			SLLpush_front(&ent->pathList, newNode);
			// Set the path in the node array to match this shortest path
			path_table[nodefrom[newNode]][to] = newNode;

			//Com_Printf("%s path_table[ nodefrom[%d] ][ %d ] = %d\n", __func__, newNode, to, newNode);
		}

		//Com_Printf("%s EXIT PATH\n\n", __func__);

		// Each time a path is found, make a copy
		ent->bot.node_list_count = BOTLIB_SLL_Query_All_Nodes(ent, &ent->pathList, ent->bot.node_list, MAX_NODELIST); // Retrieve the nodes in the list
		ent->bot.node_list_current = 0;
		if (ent->bot.node_list_count) // Set the current and next nodes
		{
			ent->bot.current_node = ent->bot.node_list[0];
			ent->bot.next_node = ent->bot.node_list[1];
		}

		if (0)
		{
			if (1)
			{
				Com_Printf("%s s[%d] g[%d] node_list[", __func__, from, to);
				for (int i = 0; i < ent->bot.node_list_count; i++)
				{
					Com_Printf(" %d ", ent->bot.node_list[i]);
				}
				Com_Printf(" ]\n");
			}
			else
				Com_Printf("%s from[%d] to[%d]\n", __func__, from, to);
			//Com_Printf("%s found a path\n", __func__);
		}

		return true;
	}
	// else
	return false;
}

//=============================
// LinkExists
//=============================
//
// Check we haven't wandered off path!
//
qboolean	AntLinkExists(int from, int to)
{
	int counter = 0;
	int	testnode;

	if (from == INVALID || to == INVALID)
		return false;

	// Check if the link exists
	while (counter < MAXLINKS)
	{
		testnode = nodes[from].links[counter].targetNode; // Get next linked node
		if (testnode == to)
		{
			return true; // A path exists from -> to 
		}
		else if (testnode == INVALID)
		{
			return false; // No more links and no path found
		}
		counter++;
	}
	
	return false; // Didn't find it!
}




// Draw the bot path
qboolean BOTLIB_DrawPath(edict_t* self)
{
	//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
	int curr_node, next_node;
	float line_width;
	uint32_t color;
	uint32_t blue = MakeColor(0, 0, 255, 255); // Blue
	uint32_t red = MakeColor(255, 0, 0, 255); // Red
	void (*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = NULL;
	void (*DrawArrow)(int number, vec3_t start, vec3_t end, const uint32_t color, float line_width, int time, qboolean occluded) = NULL;
	DrawBox = players[0]->client->pers.draw->DrawBox;
	DrawArrow = players[0]->client->pers.draw->DrawArrow;
	players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
	players[0]->client->pers.draw->arrows_inuse = true; // Flag as being used
	int jumppad_box_num = 0; // Counter to display more than one jumpbox at once
	int start = self->bot.node_list_current - 1;
	if (start < 0)
		start = 0;
	for (int i = start; (i + 1) < self->bot.node_list_count; i++)
	{
		curr_node = self->bot.node_list[i]; // Next node in the list
		next_node = self->bot.node_list[i + 1]; // Next node in the list

		if (curr_node == INVALID || next_node == INVALID)
			break;

		if (players[0]->client->pers.draw->draw_arrow_num + 1 > MAX_DRAW_ARROWS)
			players[0]->client->pers.draw->draw_arrow_num = 0;
		players[0]->client->pers.draw->draw_arrow_num;

		// Draw current link as thicker line
		if (self->bot.current_node == curr_node)
		{
			line_width = 10.0;
			color = MakeColor(255, 255, 0, 255); // Yellow
		}
		else // Draw all other links as thinner lines
		{
			line_width = 1.0;
			color = MakeColor(0, 255, 0, 255); // Green
		}

		// Show jumppad nodes as a blue box
		for (int i = 0; i < nodes[curr_node].num_links; i++)
		{
			if (nodes[curr_node].links[i].targetNode == next_node)
			{
				if (nodes[curr_node].links[i].targetNodeType == NODE_JUMPPAD)
				{
					DrawBox(jumppad_box_num, nodes[curr_node].origin, blue, nodes[curr_node].mins, nodes[curr_node].maxs, 100, false);
					jumppad_box_num++;
				}
				break;
			}
		}

		DrawArrow(players[0]->client->pers.draw->draw_arrow_num, nodes[curr_node].origin, nodes[next_node].origin, color, line_width, 100, false);
		players[0]->client->pers.draw->draw_arrow_num++;
	}

	// Draw goal node
	DrawBox(jumppad_box_num, nodes[self->bot.goal_node].origin, red, nodes[self->bot.goal_node].mins, nodes[self->bot.goal_node].maxs, 100, false);

#endif
	//rekkie -- debug drawing -- e

	return true;
}




//==============================
// SLLpush_front
//==============================
// Add to the front of the list
//
void SLLpush_front(botlib_sll_t* list, int node)
{
	botlib_sll_nodes_t* temp;

	// Store the current head pointer
	temp = list->head;
	// allocate memory for the new data (LEVEL tagged)
	list->head = gi.TagMalloc(sizeof(botlib_sll_nodes_t), TAG_LEVEL);
	// Set up the data and pointer
	list->head->node = node;
	list->head->next = temp;
	// Check if there;'s a next item
	if (!list->head->next)
	{
		// Set the tail pointer = head
		list->tail = list->head;
	}
}

//==============================
// SLLpop_front
//==============================
// Remove the item from the front of the list
//
void SLLpop_front(botlib_sll_t* list)
{
	botlib_sll_nodes_t* temp;

	// Store the head pointer
	temp = list->head;
	// Check if there's a next item
	if (list && list->head)
	{
		if (list->head == list->tail)
		{
			// List is now emptying
			list->tail = list->head = NULL;
		}
		else
		{
			// Move head to point to next item
			list->head = list->head->next;
		}
		// Free the memory (LEVEL tagged)
		gi.TagFree(temp);
	}
	else
	{
		gi.bprintf(PRINT_HIGH, "Attempting to POP an empty list!\n");
	}
}

//rekkie -- DEV_1 -- s
//==============================
// BOTLIB_SLL_Query_Next_Node
//==============================
// Query the SLL for the next node
// Returns the next node, otherwise returns INVALID if next node cannot be found (end of list, or list is NULL)
int BOTLIB_SLL_Query_Next_Node(botlib_sll_t* list)
{
	if (0) //DEBUG: Print out the entire list
	{
		// Check if there's a next item
		if (list && list->head && list->head != list->tail && list->head->next != NULL)
		{
			Com_Printf("%s nodes[ ", __func__);
			Com_Printf("%d ", list->head->node); // Print head node
			botlib_sll_nodes_t* next = list->head->next;
			while (next != NULL)
			{
				if (list->head == list->tail) // We're already at the last item
					break;
				else
				{
					Com_Printf("%d ", next->node); // Print next node
					next = next->next; // Get the next SLL item
				}
			}
			Com_Printf("]\n");
		}
	}

	// Check if there's a next item
	if (list && list->head)
	{
		if (list->head == list->tail || list->head->next == NULL) // We're already at the last item
		{
			return INVALID; // No next item
		}
		else
		{
			botlib_sll_nodes_t* next = list->head->next; // Get the next SLL item
			return next->node; // return the next node
		}
	}

	return INVALID; // Empty list
}
//==============================
// BOTLIB_SLL_Query_All_Nodes
//==============================
// Query the SLL list and return all its nodes
// node_list is the returned list of nodes
// max_nodes is the maximum allowed nodes we can store in node_list
// Returns the node count
int BOTLIB_SLL_Query_All_Nodes(edict_t *ent, botlib_sll_t* list, int* node_list, const int max_nodes)
{
	//int node_count = 0; // Node count

	if (ent->bot.node_list_count + 1 > max_nodes) 
		return ent->bot.node_list_count;

	// Check if there's a next item
	if (list && list->head && list->head != list->tail && list->head->next != NULL)
	{
		//node_list[ent->bot.node_list_count++] = ent->bot.current_node; // Save current node
		node_list[ent->bot.node_list_count++] = list->head->node; // Save head node
		botlib_sll_nodes_t* next = list->head->next;
		while (next != NULL)
		{
			if (ent->bot.node_list_count + 1 > max_nodes) return ent->bot.node_list_count;

			if (list->head == list->tail) // We're already at the last item
				break;
			else
			{
				node_list[ent->bot.node_list_count++] = next->node; // Save next node
				next = next->next; // Get the next SLL item
			}
		}
	}

	return ent->bot.node_list_count; // Return node count
}
//rekkie -- DEV_1 -- e

//==============================
// SLLfront
//==============================
// Get the integer value from the front of the list
// without removing the item (Query the list)
//
int	SLLfront(botlib_sll_t* list)
{
	if (list && !SLLempty(list))
		return(list->head->node);
	else
		return INVALID;
}

//==============================
// SLLpush_front
//==============================
// Add to the back of the list
//
void SLLpush_back(botlib_sll_t* list, int node)
{
	botlib_sll_nodes_t* temp;

	// Allocate memory for the new item (LEVEL tagged)
	temp = (botlib_sll_nodes_t*)gi.TagMalloc(sizeof(botlib_sll_nodes_t), TAG_LEVEL);
	// Store the data
	temp->node = node;
	temp->next = NULL;		// End of the list
	// Store the new item in the list
	// Is the list empty?
	if (!list->head)
	{
		// Yes - add as a new item
		list->head = temp;
		list->tail = temp;
	}
	else
	{
		// No make this the new tail item
		list->tail->next = temp;
		list->tail = temp;
	}
}

//==============================
// SLLempty
//==============================
// See if the list is empty (false if not empty)
//
qboolean SLLempty(botlib_sll_t* list)
{
	// If there is any item in the list then it is NOT empty...
	if (list)
		return (list->head == NULL);
	else	// No list so return empty
		return true;
}

//===============================
// Delete the list
//===============================
// Avoids memory leaks
//
void SLLdelete(botlib_sll_t* list)
{
	botlib_sll_nodes_t* temp;

	while (!SLLempty(list))
	{
		temp = list->head;
		list->head = list->head->next;
		gi.TagFree(temp);
	}
}
