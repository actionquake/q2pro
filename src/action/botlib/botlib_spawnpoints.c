#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

dc_sp_t* dc_sp;
int dc_sp_count; // Total spawn points
qboolean dc_sp_edit; // If the spawn points have been made visible for editing

// Free the memory
void DC_Free_Spawnpoints(void)
{
	if (dc_sp_count && dc_sp != NULL)
	{
		free(dc_sp);
		dc_sp = NULL; // Nullify dangling pointer
	}
}
void DC_Init_Spawnpoints(void)
{
	dc_sp_edit = false; // Set edit mode to false
	dc_sp_count = 0; // Total spawn points
	dc_sp = NULL;
}
// Add and display a new custom spawn point at the location given
void DC_Remove_Spawnpoint(edict_t* self)
{
	if (dc_sp_count && dc_sp != NULL)
	{
		// Try looking for a dc_spawnpoint nearby and move it to an unused state
		const int search_dist = 256;
		vec3_t v;
		float dist;
		trace_t tr;
		edict_t* spot = NULL;
		while ((spot = G_Find(spot, FOFS(classname), "dc_spawnpoint")) != NULL)
		{
			VectorSubtract(spot->s.origin, self->s.origin, v); // Distance from player
			dist = VectorLength(v);
			if (dist < search_dist) // Found a spawn close enough
			{
				// Ensure we have LoS
				tr = gi.trace(self->s.origin, NULL, NULL, spot->s.origin, self, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0)
				{
					// Search the dc_sp list for this spot and update it
					for (int i = 0; i < dc_sp_count; i++)
					{
						// Test the spot origin against the dc_sp[i].origin
						if (VectorCompare(spot->s.origin, dc_sp[i].origin)) // Same location
						{
							// Edit the existing spawn point
							dc_sp[i].inuse = false; // Change state to unused

							// Mark spot as deleted
							spot->s.renderfx = RF_SHELL_RED | RF_TRANSLUCENT;

							// Free spot
							spot->nextthink = level.framenum + 5 * HZ;
							spot->think = G_FreeEdict;

							return;
						}
					}
				}
			}
		}
	}
}
// Add and display a new custom spawn point at the location given
void DC_Add_Spawnpoint(edict_t* self)
{
	if (dc_sp_count + 1 > DC_SP_LIMIT)
	{
		Com_Printf("%s could not add spawnpoint, limit is %d!\n", __func__, DC_SP_LIMIT);
		return;
	}

	// Ensure we're in edit mode
	if (dc_sp_edit == false)
		BOTLIB_Show_Spawnpoints();

	if (dc_sp_count && dc_sp != NULL)
	{
		// Try looking for an unused spot
		for (int i = 0; i < dc_sp_count; i++)
		{
			if (dc_sp[i].inuse == false) // Recycle deleted spot
			{
				// Edit the existing spawn point
				dc_sp[i].inuse = true;
				VectorCopy(self->s.origin, dc_sp[i].origin);
				VectorCopy(self->s.angles, dc_sp[i].angles);

				// Make a visible version
				edict_t* ent;
				ent = G_Spawn();
				ent->classname = "dc_spawnpoint";
				ent->movetype = MOVETYPE_NONE;
				ent->solid = SOLID_NOT;
				ent->model = "models/objects/dmspot/tris.md2";	// deathmatch spawn point
				ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
				ent->owner = ent;
				ent->nextthink = level.framenum + (60 * 15) * HZ; // Free it after (60 sec * 15) 15 minutes
				ent->think = G_FreeEdict;
				VectorCopy(self->s.origin, ent->s.origin);
				VectorCopy(self->s.angles, ent->s.angles);
				VectorSet(ent->mins, -16, -16, -24);
				VectorSet(ent->maxs, 16, 16, 32);
				ent->s.renderfx = RF_SHELL_GREEN | RF_TRANSLUCENT; // Mark as custom
				gi.linkentity(ent);

				return;
			}
		}

		// Otherwise, try looking for a dc_spawnpoint nearby and move it to the player
		const int search_dist = 256;
		vec3_t v;
		float dist;
		trace_t tr;
		edict_t* spot = NULL;
		while ((spot = G_Find(spot, FOFS(classname), "dc_spawnpoint")) != NULL)
		{
			VectorSubtract(spot->s.origin, self->s.origin, v); // Distance from player
			dist = VectorLength(v);
			if (dist < search_dist) // Found a spawn close enough to move
			{
				// Ensure we have LoS
				tr = gi.trace(self->s.origin, NULL, NULL, spot->s.origin, self, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0)
				{
					// Search the dc_sp list for this spot and update it
					for (int i = 0; i < dc_sp_count; i++)
					{
						// Test the spot origin against the dc_sp[i].origin
						if (VectorCompare(spot->s.origin, dc_sp[i].origin)) // Same location
						{
							// Edit the existing spawn point
							dc_sp[i].inuse = true; // If it was deleted, make it used again
							VectorCopy(self->s.origin, dc_sp[i].origin);
							VectorCopy(self->s.angles, dc_sp[i].angles);

							// Move the spot
							gi.unlinkentity(spot);
							VectorCopy(self->s.origin, spot->s.origin);
							VectorCopy(self->s.angles, spot->s.angles);
							gi.linkentity(spot);

							return;
						}
					}
				}
			}
		}
	}

	// Show a visible spawn point
	edict_t* ent;
	ent = G_Spawn();
	ent->classname = "dc_spawnpoint";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->model = "models/objects/dmspot/tris.md2";	// deathmatch spawn point
	ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
	ent->owner = ent;
	ent->nextthink = level.framenum + (60 * 15) * HZ; // Free it after (60 sec * 15) 15 minutes
	ent->think = G_FreeEdict;
	VectorCopy(self->s.origin, ent->s.origin);
	VectorCopy(self->s.angles, ent->s.angles);
	VectorSet(ent->mins, -16, -16, -24);
	VectorSet(ent->maxs, 16, 16, 32);
	ent->s.renderfx = RF_SHELL_GREEN | RF_TRANSLUCENT; // Mark as custom
	gi.linkentity(ent);

	// Grow memory
	if (dc_sp_count == 0)
	{
		dc_sp = (dc_sp_t*)malloc(sizeof(dc_sp_t));
		if (dc_sp == NULL)
		{
			Com_Printf("%s could not alloc dc_sp. Out of memory!\n", __func__);
			return;
		}
	}
	else
	{
		dc_sp_t* prev = dc_sp;
		dc_sp = (dc_sp_t*)realloc(dc_sp, (dc_sp_count + 1) * sizeof(dc_sp_t));
		if (dc_sp == NULL)
		{
			free(prev);
			prev = NULL;
			Com_Printf("%s could not realloc dc_sp. Out of memory!\n", __func__);
			return;
		}
	}

	// Add it
	dc_sp[dc_sp_count].inuse = true;
	VectorCopy(self->s.origin, dc_sp[dc_sp_count].origin);
	VectorCopy(self->s.angles, dc_sp[dc_sp_count].angles);

	// Increment total SPs
	dc_sp_count++;
}
// Find and add all the map spawn points
void DC_Get_Map_Spawnpoints(void)
{
	edict_t* spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
		DC_Add_Spawnpoint(spot);
}
// Find and display (as visible ents) all spawn points (map and custom)
// Custom spawn points are green
// Deleted spawn points are red
void BOTLIB_Show_Spawnpoints(void)
{
	const int free_time = (60 * 1); // Free it after (60 seconds * 15) = 15 minutes 

	// Only spawn them once per map
	if (dc_sp_edit == false)
	{
		dc_sp_edit = true;
	}
	else
		return;

	// Show map spawn points
	qboolean is_custom_spot;
	edict_t* spot = NULL;
	while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		// Skip adding custom spots that have been labled as "info_player_deathmatch"
		// This occurs when loading custom spots from file, we set them as classname "info_player_deathmatch"
		// so the spawn code can find them. However, because custom spots need editing, we don't want them 'visually'
		// showing up as info_player_deathmatch, but rather as dc_spawnpoint which is handled later in this function.
		is_custom_spot = false;
		if (dc_sp_count && dc_sp != NULL)
		{
			for (int i = 0; i < dc_sp_count; i++)
			{
				if (VectorCompare(spot->s.origin, dc_sp[i].origin)) // Same location
				{
					is_custom_spot = true;
					break;
				}
			}
		}
		if (is_custom_spot) continue; // Skip adding this spot because its custom

		// Show a visible spawn point
		edict_t* ent;
		ent = G_Spawn();
		ent->classname = "dc_spawnpoint_deathmath";
		ent->movetype = MOVETYPE_NONE;
		ent->solid = SOLID_NOT;
		ent->model = "models/objects/dmspot/tris.md2";	// deathmatch spawn point
		ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
		ent->owner = ent;
		ent->nextthink = level.framenum + free_time * HZ;
		ent->think = G_FreeEdict;
		VectorCopy(spot->s.origin, ent->s.origin);
		VectorCopy(spot->s.angles, ent->s.angles);
		VectorSet(ent->mins, -16, -16, -24);
		VectorSet(ent->maxs, 16, 16, 32);
		gi.linkentity(ent);
	}

	while ((spot = G_Find(spot, FOFS(classname), "info_player_team1")) != NULL)
	{
		// Show a visible spawn point
		edict_t* ent;
		ent = G_Spawn();
		ent->classname = "dc_spawnpoint_team1";
		ent->movetype = MOVETYPE_NONE;
		ent->solid = SOLID_NOT;
		ent->model = "models/objects/dmspot/tris.md2";	// deathmatch spawn point
		ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
		ent->owner = ent;
		ent->nextthink = level.framenum + free_time * HZ;
		ent->think = G_FreeEdict;
		VectorCopy(spot->s.origin, ent->s.origin);
		VectorCopy(spot->s.angles, ent->s.angles);
		VectorSet(ent->mins, -16, -16, -24);
		VectorSet(ent->maxs, 16, 16, 32);
		gi.linkentity(ent);
	}

	while ((spot = G_Find(spot, FOFS(classname), "info_player_team2")) != NULL)
	{
		// Show a visible spawn point
		edict_t* ent;
		ent = G_Spawn();
		ent->classname = "dc_spawnpoint_team2";
		ent->movetype = MOVETYPE_NONE;
		ent->solid = SOLID_NOT;
		ent->model = "models/objects/dmspot/tris.md2";	// deathmatch spawn point
		ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
		ent->owner = ent;
		ent->nextthink = level.framenum + free_time * HZ;
		ent->think = G_FreeEdict;
		VectorCopy(spot->s.origin, ent->s.origin);
		VectorCopy(spot->s.angles, ent->s.angles);
		VectorSet(ent->mins, -16, -16, -24);
		VectorSet(ent->maxs, 16, 16, 32);
		gi.linkentity(ent);
	}

	// Show custom user spawn points
	if (dc_sp_count && dc_sp != NULL)
	{
		for (int i = 0; i < dc_sp_count; i++)
		{
			// Show a visible spawn point
			edict_t* ent;
			ent = G_Spawn();
			ent->classname = "dc_spawnpoint";
			ent->movetype = MOVETYPE_NONE;
			ent->solid = SOLID_NOT;
			ent->model = "models/objects/dmspot/tris.md2";	// deathmatch spawn point
			ent->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
			ent->owner = ent;
			ent->nextthink = level.framenum + free_time * HZ;
			ent->think = G_FreeEdict;
			VectorCopy(dc_sp[i].origin, ent->s.origin);
			VectorCopy(dc_sp[i].angles, ent->s.angles);
			VectorSet(ent->mins, -16, -16, -24);
			VectorSet(ent->maxs, 16, 16, 32);
			if (dc_sp[i].inuse == false) ent->s.renderfx = RF_SHELL_RED | RF_TRANSLUCENT; // Mark as not in use
			else ent->s.renderfx = RF_SHELL_GREEN | RF_TRANSLUCENT; // Mark as custom
			gi.linkentity(ent);
		}
	}
}
// Save SPs to file
void DC_Save_Spawnpoints(void)
{
	FILE* fOut;
	char filename[128];
	int fileSize = 0;
	short int i = 0;
	byte version = DC_SP_VERSION;
	cvar_t* game_dir = gi.cvar("game", "action", 0);	// Directory of the gamelib
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

#ifdef _WIN32
	i = sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, botdir->string);
	i += sprintf(filename + i, "\\spawnpoints\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".sp");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/spawnpoints/");
	strcat(filename, level.mapname);
	strcat(filename, ".sp");
#endif

	// Add the map spawn points
	//DC_Get_Map_Spawnpoints();

	if ((fOut = fopen(filename, "wb")) == NULL) // Open file to write binary
	{
		Com_Printf("%s Failed to write spawnpoint data to file %s\n", __func__, filename);
		return;
	}

	Com_Printf("%s Writing spawnpoint data to file...\n", __func__);

	// Write general data
	fileSize += sizeof(byte) * fwrite(&version, sizeof(byte), 1, fOut);			// Version
	fileSize += sizeof(int) * fwrite(&dc_sp_count, sizeof(int), 1, fOut);		// Total spawnpoints

	// Write spawnpoint data
	for (i = 0; i < dc_sp_count; i++)
	{
		fileSize += sizeof(qboolean) * fwrite(&dc_sp[i].inuse, sizeof(qboolean), 1, fOut);	// In use
		fileSize += sizeof(vec3_t) * fwrite(&dc_sp[i].origin, sizeof(vec3_t), 1, fOut);		// Location
		fileSize += sizeof(vec3_t) * fwrite(&dc_sp[i].angles, sizeof(vec3_t), 1, fOut);		// Direction
	}

	fclose(fOut);

	Com_Printf("%s Saved %s [%i bytes] to disk\n", __func__, filename, fileSize);
}
// Load SPs from file
void DC_Load_Spawnpoints(void)
{
	FILE* fIn;
	char filename[128];
	int fileSize = 0;
	short int i = 0;
	byte version = 0;
	cvar_t* game_dir = gi.cvar("game", "action", 0);	// Directory of the gamelib
	cvar_t* botdir = gi.cvar("botdir", "bots", 0);		// Directory of the bot files in the gamelib

#ifdef _WIN32
	i = sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, botdir->string);
	i += sprintf(filename + i, "\\spawnpoints\\");
	i += sprintf(filename + i, level.mapname);
	i += sprintf(filename + i, ".sp");
#else
	strcpy(filename, "./");
	strcat(filename, game_dir->string);
	strcat(filename, "/");
	strcat(filename, botdir->string);
	strcat(filename, "/spawnpoints/");
	strcat(filename, level.mapname);
	strcat(filename, ".sp");
#endif

	if ((fIn = fopen(filename, "rb")) == NULL) // See if AAS file exists
	{
		Com_Printf("%s %s not found. The map has no custom spawnpoints\n", __func__, filename);
		return; // Map has no spawnpoint data
	}
	else
	{
		Com_Printf("%s Reading SP file...\n", __func__);
		fileSize += sizeof(byte) * fread(&version, sizeof(byte), 1, fIn); // AAS Version
		if (version != DC_SP_VERSION)
		{
			Com_Printf("%s %s is from a previous version. File is incompatible.\n", __func__, filename);
			fclose(fIn);
			return;
		}
	}

	fileSize += sizeof(int) * fread(&dc_sp_count, sizeof(int), 1, fIn);			// Total Nodes

	DC_Free_Spawnpoints();  //Soft map change. Free any existing spawnpoint memory used

	// Grow memory
	if (dc_sp_count)
	{
		dc_sp = (dc_sp_t*)malloc(sizeof(dc_sp_t) * dc_sp_count);
		if (dc_sp == NULL)
		{
			Com_Printf("%s could not alloc dc_sp. Out of memory!\n", __func__);
			fclose(fIn);
			return;
		}
	}

	// Read SP data
	for (i = 0; i < dc_sp_count; i++)
	{
		fileSize += sizeof(qboolean) * fread(&dc_sp[i].inuse, sizeof(qboolean), 1, fIn);	// In use
		fileSize += sizeof(vec3_t) * fread(&dc_sp[i].origin, sizeof(vec3_t), 1, fIn);		// Location
		fileSize += sizeof(vec3_t) * fread(&dc_sp[i].angles, sizeof(vec3_t), 1, fIn);		// Direction
	}

	// Show custom user spawn points
	if (dc_sp_count && dc_sp != NULL)
	{
		for (int i = 0; i < dc_sp_count; i++)
		{
			if (dc_sp[i].inuse) // Ignore deleted spawn spots
			{
				// Add the custom spawn points to the map
				edict_t* ent;
				ent = G_Spawn();
				ent->classname = "info_player_deathmatch"; // Make it findable
				ent->movetype = MOVETYPE_NONE;
				ent->solid = SOLID_NOT;
				ent->owner = ent;
				VectorCopy(dc_sp[i].origin, ent->s.origin);
				VectorCopy(dc_sp[i].angles, ent->s.angles);
				VectorSet(ent->mins, -16, -16, -24);
				VectorSet(ent->maxs, 16, 16, 32);
				gi.linkentity(ent);
			}
		}
	}

	fclose(fIn);

	Com_Printf("%s Loaded %s [%i bytes] from disk\n", __func__, filename, fileSize);
}