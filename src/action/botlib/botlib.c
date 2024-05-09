#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

// All of these variables are defined in botlib.c, but are used in other files, they are
// declared in botlib.h and are externed in the files that use them.
// It's rather annoying that you can't have these in the header file for some reason

// botlib.h
int DFS_area_nodes[MAX_NAV_AREAS][MAX_NAV_AREAS_NODES]; // Area nodes - [(32 * 1024) * 4 bytes = 132k]
int DFS_area_edges[MAX_NAV_AREAS][MAX_NAV_AREAS_EDGES] = {0}; // Initialize all elements to 0
ctf_status_t bot_ctf_status;
dc_sp_t* dc_sp;
int dc_sp_count; // Total spawn points
qboolean dc_sp_edit; // If the spawn points have been made visible for editing
botlib_noises_t botlib_noises;
bot_connections_t bot_connections;

