//
#include "../g_local.h"
#include "acebot.h"
#include "botchat.h"
#include "botscan.h"


// acebot.h
int	dc_total_male_names;		// Total male names
int	dc_total_female_names;		// Total female names
bot_names_t bot_male[MAX_BOT_NAMES];	// Cached copy
bot_names_t bot_female[MAX_BOT_NAMES];	// Cached copy
int num_poi_nodes;
int poi_nodes[MAX_POI_NODES];
int num_vis_nodes;
int node_vis[10][10]; // Cached node visibily. node_vis[X][Y] <-- can X see Y? If Y == INVALID, then false. Otherwise Y == NODE NUM
int node_vis_list[10][10];
nmesh_t nmesh;
edict_t* node_ents[MAX_EDICTS]; // If the node is attached to an entity (such as a NODE_DOOR being attached to a func_door_rotating or func_door entity)
short int** path_table;
node_t *nodes;