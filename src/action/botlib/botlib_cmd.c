#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

// Bot server commands
qboolean BOTLIB_SV_Cmds(void)
{
	char* cmd;
	cmd = gi.argv(1);

	if (Q_stricmp(cmd, "bots") == 0)
	{
		if (!bot_enable->value) {
			gi.dprintf("%s: bot_enable is 0; Bots are disabled\n", __func__);
			bot_connections.desire_bots = 0;
		}

		int cc = gi.argc();

		if (!bot_enable->value) {
			gi.dprintf("bot_enable is 0; Bots are disabled\n");
			return true;
		}

		gi.cvar_set("bot_maxteam", va("%d", 0)); // Override if bots manually added

		// sv bots <num>
		if (gi.argc() == 3) // Adding a multiple bots
		{
			bot_connections.desire_bots = atoi(gi.argv(2)); // How many bots
			bot_connections.auto_balance_bots = true;

			if (bot_playercount->value > 0 && !(gameSettings & GS_ROUNDBASED))
				gi.dprintf("bot_playercount is set to %i, sv bots has no effect\n", (int)bot_playercount->value);

			return true;
		}

		// sv bots <num> <team>
		else if (gi.argc() == 4) // Adding a multiple bots to a select team
		{
			int count = atoi(gi.argv(2)); // How many bots
			int team = atoi(gi.argv(3)); // What team

			//if (count == 0)
			//	count = -1; // Flag to remove all bots from team

			bot_connections.auto_balance_bots = false;

			// Deathmatch
			if (teamplay->value == 0)
			{
				if (count == 0)
				{
					BOTLIB_RemoveBot("ALL");
					return true;
				}
				else
				{
					bot_connections.desire_bots = count;
					bot_connections.auto_balance_bots = true;
					return true;
				}
			}
			else // Teamplay
			{
				if (team == TEAM1) bot_connections.desire_team1 = count;
				else if (team == TEAM2) bot_connections.desire_team2 = count;
				else if (team == TEAM3) bot_connections.desire_team3 = count;

				return true;
			}
		}

		else
		{
			gi.cprintf(NULL, PRINT_HIGH, "----------------------------------------------\n");
			gi.cprintf(NULL, PRINT_HIGH, "Use: sets the number of bots to join a game\n");
			gi.cprintf(NULL, PRINT_HIGH, "----------------------------------------------\n");
			gi.cprintf(NULL, PRINT_HIGH, "sv bots <count>\n");
			gi.cprintf(NULL, PRINT_HIGH, "sv bots <count> [team]\n");
			if (teamplay->value == 0)
				gi.cprintf(NULL, PRINT_HIGH, "DM bot total [%d]\n", bot_connections.total_bots);
			else if (use_3teams->value)
				gi.cprintf(NULL, PRINT_HIGH, "3TEAM bots on T1[%d] T2[%d] T3[%d] Total[%d]\n", bot_connections.team1_bots, bot_connections.team2_bots, bot_connections.team3_bots, bot_connections.total_bots);
			else if (ctf->value)
				gi.cprintf(NULL, PRINT_HIGH, "CTF bots on T1[%d] T2[%d] Total[%d]\n", bot_connections.team1_bots, bot_connections.team2_bots, bot_connections.total_bots);
			else if (teamplay->value)
				gi.cprintf(NULL, PRINT_HIGH, "TP bots on T1[%d] T2[%d] Total[%d]\n", bot_connections.team1_bots, bot_connections.team2_bots, bot_connections.total_bots);
			
			if (bot_playercount->value > 0 && !(gameSettings & GS_ROUNDBASED)) {
				gi.cprintf(NULL, PRINT_HIGH, "**---------------------------**\n");
				gi.cprintf(NULL, PRINT_HIGH, "bot_playercount is set to %i, sv bots has no effect\n", (int)bot_playercount->value);
			}
			gi.cprintf(NULL, PRINT_HIGH, "-------------------------------\n");
		}

		return true;
	}

	if (Q_stricmp(cmd, "addbot") == 0)
	{
		
		return true;
	}
	else if (Q_stricmp(cmd, "addbots") == 0)
	{
		if (gi.argc() >= 3)
		{
			int count = atoi(gi.argv(2)), i = 0;
			for (i = 0; i < count; i++)
			{
			}
		}
		else
			gi.cprintf(NULL, PRINT_HIGH, "Usage: sv addbots <count> [<team>]\n");

		return true;
	}
	// removebot
	else if (Q_stricmp(cmd, "removebot") == 0)
	{
		ACESP_RemoveBot(gi.argv(2));
		return true;
	}
	else if (Q_stricmp(cmd, "removebots") == 0)
	{
		ACESP_RemoveBot("all");
		return true;
	}

	//rekkie -- python chatbot -- s
#if 0
	// 
	// Allows Quake 2 to communicate with a LLM (Large Language Model)
	// [Input]    Quake 2 ==> textgen.py ==> text-generation-webui (server)
	// [Output]   text-generation-webui (server) ==> textgen.py ==> Quake 2
	// 
	// Idea was taken from: https://www.youtube.com/watch?v=ndLqg4RYcXE  (Full Tutorial of Calling Python Functions(.py) in C/C++(.c or .cpp) using (python.h))
	// 
	// This runs the python file ./action/bots/textgen.py
	// The py file allows communcation with an ai chat server called text-generation-webui (github: https://github.com/oobabooga/text-generation-webui)
	// 
	// text-generation-webui must be run with OpenAI API turned on (port 5000)
	// 
	// From the C end, we require a few includes and libs from Python.
	// NOTE:	When installing python, make sure (Download debug binaries) is ticked, 
	//			otherwise the compiler will complain about "python310_d.lib" missing.
	//			https://github.com/pybind/pybind11/issues/3403
	// 
	// Includes
	// C:\Users\...\AppData\Local\Programs\Python\Python310\include
	// 
	// Linker (libs)
	// C:\Users\...\AppData\Local\Programs\Python\Python310\libs\python310.lib
	// C:\Users\...\AppData\Local\Programs\Python\Python310\libs
	else if (Q_stricmp(cmd, "chat") == 0)
	{
		Py_Initialize(); // Init python

		PyObject* name, * load_module, * func, * callfunc, * args;

		/*
		//https://groups.google.com/g/comp.lang.python/c/14Yl3C8suKI?pli=1
		// https://gist.github.com/Coderx7/509a1aaf154a8d2532a6459a57308a63
		PyObject* mainmod = PyImport_AddModule("__main__");
		Py_INCREF(mainmod);
		PyObject* ns = PyModule_GetDict(mainmod);
		Py_INCREF(ns);
		PyObject* timeModule = PyImport_ImportModuleEx("requests", ns, ns, NULL);
		*/

		//PyObject* module = PyImport_AddModule("requests");
		PyObject* pModule = PyImport_ImportModule("requests"); // The python imports (imports used in textgen.py)

		// Open our custom python file
		const wchar_t* path = L".\\action\\bots\\"; // https://stackoverflow.com/questions/31314767/call-multiple-python-functions-from-different-directories
		PySys_SetPath(path); // https://stackoverflow.com/questions/1796925/how-to-import-a-file-by-its-full-path-using-c-api
		name = PyUnicode_FromString((char*)"textgen");
		load_module = PyImport_Import(name);



		if (load_module != NULL)
		{
			func = PyObject_GetAttrString(load_module, (char*)"func"); // Set our custom function to call
			//args = PyTuple_Pack(1, PyUnicode_FromString((char*)"hello"));
			args = PyTuple_Pack(1, PyUnicode_FromString(gi.argv(2))); // Set the arguments to send

			callfunc = PyObject_CallObject(func, args); // Call our custom python function
			if (callfunc != NULL)
			{
				const char* str = _PyUnicode_AsString(callfunc);
				Com_Printf("%s %s\n", __func__, str);
			}
}

		Py_Finalize();
	}
	else if (Q_stricmp(cmd, "chatfile") == 0)
	{
		char filename[128];
#ifdef _WIN32
		sprintf(filename, ".\\action\\bots\\textchat.txt");
#else
		strcpy(filename, "./action/bots/textchat.txt");
#endif

		// If the file exists and NOT empty, the file is considered 'in use'
		FILE* file = fopen(filename, "r");
		if (file != NULL)
		{
			fseek(file, 0L, SEEK_END);
			long size = ftell(file);
			if (size > 0)
			{
				Com_Printf("File in use: %s\n", filename);
				fclose(file); // Close the file after use
				return;
			}
		}

		// Write to file
		file = fopen(filename, "w"); // Open the file in write mode
		if (file == NULL) {
			Com_Printf("Could not write to file %s\n", filename);
			return;
		}
		fprintf(file, "%s\n%s", gi.argv(2), gi.argv(3)); // Write to the file
		fclose(file); // Close the file after use

		/*
		srand((unsigned int)time(NULL)); // Seed the random number generator

		const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		char key[7]; // Six characters + null terminator
		for (int i = 0; i < 6; ++i) {
			int index = rand() % (sizeof(charset) - 1);
			key[i] = charset[index];
		}
		key[6] = '\0'; // Null-terminate the string

		char playerName[256], chatMessage[256];

		printf("Enter your name: ");
		scanf("%s", playerName);

		printf("Enter your chat message: ");
		getchar(); // Consume newline left by previous input
		fgets(chatMessage, sizeof(chatMessage), stdin); // Read the whole line including spaces

		fprintf(file, "%s\n%s\n%s", key, playerName, chatMessage); // Write to the file

		fclose(file); // Close the file after use
		*/
	}
#endif
	//rekkie -- python chatbot -- e

	return false;
}

// Bot commands
qboolean BOTLIB_Commands(edict_t* ent)
{
	char* cmd = gi.argv(0);
	// if (Q_stricmp(cmd, "dc_save_aas") == 0) // Save AAS
	// {
	// 	ACEND_SaveAAS(true);
	// 	return true;
	// }
	// else if (Q_stricmp(cmd, "an") == 0) // add node
	// {
	// 	ACEND_BSP(ent);
	// 	return true;
	// }

	if (Q_stricmp(cmd, "randomize_team_names") == 0) // Manually randomize team names
	{
		BOTLIB_RandomizeTeamNames(ent);
		return true;
	}
	else if (Q_stricmp(cmd, "randomize_team_skins") == 0) // Manually randomize team skins
	{
		BOTLIB_RandomizeTeamSkins(ent);
		return true;
	}
	//rekkie -- BSP -- e
	/*
	else if (Q_stricmp(cmd, "nav_vis") == 0) // Manually create a navigation vis network
	{
		if (dedicated->value)
			return true;

		BOTLIB_GenerateNodeVis(ent);

		return true;
	}
	*/
	else if (Q_stricmp(cmd, "nav_toggle") == 0) // Toggles display between: node editing, both pathing, and neither
	{
		if (dedicated->value)
			return true;

		//bot_showpath->value = !bot_showpath->value; // Toggle path display
		if (bot_showpath->value == 0 && ent->bot.walknode.enabled == false)
		{
			ent->bot.walknode.enabled = true; // Turn on nav editor
			bot_showpath->value = 0; // Turn on path display

		}
		else if (bot_showpath->value == 0 && ent->bot.walknode.enabled == true)
		{
			ent->bot.walknode.enabled = false; // Turn off nav editor
			bot_showpath->value = 1; // Turn on path display
		}
		else
		{
			ent->bot.walknode.enabled = false; // Turn off nav editor
			bot_showpath->value = 0; // Turn off path display
		}
		return true;
	}
	else if (Q_stricmp(cmd, "nav_edit") == 0) // Turn nav editor on/off
	{
		if (dedicated->value)
			return true;
		ent->bot.walknode.enabled = !ent->bot.walknode.enabled;
		// Turn off limits if we're in edit mode
		gi.cvar_forceset(timelimit->name, "0");
		gi.cvar_forceset(fraglimit->name, "0");
		return true;
	}

	else if (Q_stricmp(cmd, "nav_edges") == 0) // Print out all the area edges
	{
		if (rand() % 2)
			BOTLIB_CanGotoNode(ent, nodes[2505].nodenum, 0);
		else
			BOTLIB_CanGotoNode(ent, nodes[999].nodenum, 0);

		/*
		int goal_node = INVALID;
		if (gi.argc() >= 2)
			goal_node = atoi(gi.argv(1));

		BOTLIB_CanGotoNode(ent, goal_node, false);
		*/
		return true;
	}

	else if (Q_stricmp(cmd, "nav_go") == 0) // Test function
	{
		BOTLIB_GetNextAreaNode(ent);
		if (ent->bot.next_area_node != INVALID)
			VectorCopy(tv(nodes[ent->bot.next_area_node].origin[0], nodes[ent->bot.next_area_node].origin[1], nodes[ent->bot.next_area_node].origin[2] + 32), ent->s.origin); // Teleport to node
		return true;
	}
	else if (Q_stricmp(cmd, "nav_aa") == 0) // Automatically generate nav areas
	{
		if (numnodes)
		{
			BOTLIB_AutoArea(ent);
		}
		return true;
	}
	else if (Q_stricmp(cmd, "nav_area") == 0) // Set the area num
	{
		if (dedicated->value)
			return true;

		if (gi.argc() >= 2)
		{
			if (ent->bot.walknode.enabled)
			{
				ent->bot.walknode.selection_area = atoi(gi.argv(1));

				// Force an area color <red> <green> <blue>
				int red;
				int green;
				int blue;
				if (gi.argc() == 5)
				{
					red = atoi(gi.argv(2));
					green = atoi(gi.argv(3));
					blue = atoi(gi.argv(4));

					// Sanity check -- randomize color if it's malformed
					if (red < 0 || red > 255)
						red = rand() % 255;
					if (green < 0 || green > 255)
						green = rand() % 255;
					if (blue < 0 || blue > 255)
						blue = rand() % 255;
				}
				else // Randomize color if no color picked
				{
					red = rand() % 255;
					green = rand() % 255;
					blue = rand() % 255;
				}

				ent->bot.walknode.selection_area_color = MakeColor(red, green, blue, 255);

				// Reset all area nodes to zero
				if (ent->bot.walknode.selection_area < 0)
				{
					Com_Printf("Nav area resetting all nodes to: 0\n");
					ent->bot.walknode.selection_area = 0;

					for (int i = 0; i < numnodes; i++)
					{
						nodes[i].area = 0;
						nodes[i].area_color = 0;
					}
				}
				else // Try setting area node
				{
					// Check if selected node is disconnected from an existing area (areas must not be disconnected islands)
					int node = 0;
					int target_node = 0;
					qboolean area_is_linked = false;
					qboolean area_is_new = false;
					//if (ent->bot.walknode.selection_area_used)
					{
						// 1) Check if our area num exists
						qboolean area_exists = false;
						for (int n = 0; n < numnodes; n++)
						{
							if (nodes[n].area == ent->bot.walknode.selection_area)
							{
								area_exists = true;
								break;
							}
						}

						// 2) If it doesn't exist, then mark as new area. No need to worry about connecting because we're making a new area.
						if (area_exists == false)
							area_is_new = true;

						// 3) If the area num exists, see if any of our selected nodes are connected to it
						else if (area_exists)
						{
							for (int i = 0; i < ent->bot.walknode.selection_node_count; i++)
							{
								node = ent->bot.walknode.selection_nodes[i];

								// Check if any of the selected nodes are connected to our desired selection_area
								if (nodes[node].area == ent->bot.walknode.selection_area)
								{
									area_is_linked = true;
									break;
								}

								// Check if any of their links connect to our desired selection_area
								for (int l = 0; l < nodes[node].num_links; l++)
								{
									target_node = nodes[node].links[l].targetNode;
									if (nodes[target_node].area == ent->bot.walknode.selection_area)
									{
										area_is_linked = true;
										break;
									}
								}
								if (area_is_linked)
									break;
							}
						}

						// 4) Optionally select surrounding nodes and include them in the set if:
						//		* Nodes are area: 0
						//		* Can connect to the selected nodes without going through another area
						//		* Similar surface normal (and/or height?)
						//if (area_exists && (area_is_linked || area_is_new))
						{
							//if (ent->bot.walknode.selection_node_count)
							//	BOTLIB_GroupConnectedNodeArea(ent, ent->bot.walknode.selection_nodes[0]);
						}
					}

					if (area_is_linked || area_is_new || ent->bot.walknode.selection_area == 0)
					{
						ent->bot.walknode.selection_area_used = true; // Flag that we desire to change the area
						Com_Printf("Nav area set to: %d\n", ent->bot.walknode.selection_area);
					}
					else
					{
						ent->bot.walknode.selection_area_used = false; // Don't change area
						Com_Printf("Found existing area %d but the selected nodes do not connect to it.\nAreas of the same number need to be connected together, they cannot be disconnected.\n", ent->bot.walknode.selection_area);
					}
				}
			}
		}
		else
		{
			gi.cprintf(NULL, PRINT_HIGH, "------------------------------------------------\n");
			gi.cprintf(NULL, PRINT_HIGH, "Use: sets the area number of any selected nodes \n");
			gi.cprintf(NULL, PRINT_HIGH, "------------------------------------------------\n");
			gi.cprintf(NULL, PRINT_HIGH, "nav_area <num>\n");
			gi.cprintf(NULL, PRINT_HIGH, "OPTIONAL: nav_area <num> <red> <green> <blue>\n");
			gi.cprintf(NULL, PRINT_HIGH, "Use '-1' to remove all areas from nodes\n");
			gi.cprintf(NULL, PRINT_HIGH, "------------------------------------------------\n");
		}
		return true;
	}
	else if (Q_stricmp(cmd, "nav_load") == 0) // Load bot nav from file
	{
#ifdef USE_ZLIB
		if (bot_enable->value) {
			BOTLIB_LoadNavCompressed();
#else
			BOTLIB_LoadNav();
#endif
			return true;
		}
	}
	else if (Q_stricmp(cmd, "nav_save") == 0) // Save bot nav to file
	{
#ifdef USE_ZLIB
		BOTLIB_SaveNavCompressed();
#else
		BOTLIB_SaveNav();
#endif
		return true;
	}
	else if (Q_stricmp(cmd, "nav_autogen") == 0) // Auto generate navigation that propagates from spawn points
	{
		BOTLIB_SelfExpandNodesFromSpawnpoints(ent);
		BOTLIB_LinkAllNodesTogether(ent);
		return true;
	}
	//rekkie -- surface data -- s
	else if (Q_stricmp(cmd, "nav") == 0) // add navigation
	{
		BOTLIB_InitNavigation(ent);
		return true;

#if 0
		ent->nav = gi.Nav(); // Grant access to navigation data
		if (ent->nav == NULL)
		{
			gi.dprintf("%s failed to import NAV data\n", __func__);
		}
		else
		{
			Com_Printf("%s NAV faces %d ignored %d\n", __func__, ent->nav->faces_total, ent->nav->ignored_faces_total);
			ACEND_SaveAAS(false);
		}

		return true;
#endif
	}
	//rekkie -- surface data -- e

	/*
	else if (Q_stricmp(cmd, "test") == 0) // Test func --
	{
		char buffer[256];
		sprintf(buffer, "%c Bandaging [ %d%c ] %c", '\x04', ent->health, '\x05', '\x04');
		Com_Printf("%s %s\n", __func__, buffer);
	}
	*/
	else if (Q_stricmp(cmd, "flag") == 0) // Test func -- grab a flag
	{
		if (ctf->value)
		{
			int team = atoi(gi.argv(1));

			if (team == TEAM1)
			{
				gi.unlinkentity(bot_ctf_status.flag1);
				VectorCopy(ent->s.origin, bot_ctf_status.flag1->s.origin);
				gi.linkentity(bot_ctf_status.flag1);
			}
			if (team == TEAM2)
			{
				gi.unlinkentity(bot_ctf_status.flag2);
				VectorCopy(ent->s.origin, bot_ctf_status.flag2->s.origin);
				gi.linkentity(bot_ctf_status.flag2);
			}

			return true;
		}
	}
	// else if (Q_stricmp(cmd, "head") == 0) // Test func -- spawn kickable head
	// {
	// 	ThrowGibbedHead(ent, 50); // Spawn kickable head
	// 	return true;
	// }
	// else if (Q_stricmp(cmd, "ball") == 0) // Test func -- spawn kickable ball
	// {
	// 	QPong_Ball(ent, 50);
	// 	return true;
	// }
	else if (Q_stricmp(cmd, "dc_add_sp") == 0) // Add (or recycle unused) custom spawn point
	{
		DC_Add_Spawnpoint(ent); // This will add a custom spawn point at the player's location
		return true;
	}
	else if (Q_stricmp(cmd, "dc_remove_sp") == 0) // Remove custom spawn point
	{
		DC_Remove_Spawnpoint(ent); // This will remove a custom spawn point at the player's location
		return true;
	}
	else if (Q_stricmp(cmd, "dc_save_sp") == 0) // Save spawn points to file
	{
		DC_Save_Spawnpoints(); // This will save the map and user added spawn points to file
		return true;
	}
	else if (Q_stricmp(cmd, "bot_show_spawns") == 0) // Show spawn points
	{
		BOTLIB_Show_Spawnpoints(); // This will show the map and user added spawn points
		return true;
	}
	else if (Q_stricmp(cmd, "removenode") == 0 && debug_mode)
		ACEND_RemoveNode(ent, atoi(gi.argv(1)));
	else if (Q_stricmp(cmd, "gtnode") == 0 && debug_mode) // Goto node - forces all bots to goto node num
	{
		int node = atoi(gi.argv(1));
		if (node > 0 && node < numnodes)
		{
			for (int i = 0; i <= num_players; i++)
			{
				if (players[i] && players[i]->is_bot && players[i]->health > 0)
				{
					if (BOTLIB_CanGotoNode(players[i], nodes[node].nodenum, false))
					{
						Com_Printf("%s %s visiting node[%i]\n", __func__, players[i]->client->pers.netname, nodes[node].nodenum);
						//players[i]->bot.state = BOT_MOVE_STATE_MOVE;
						//BOTLIB_SetGoal(players[i], nodes[node].nodenum);
					}
				}
			}
		}
		else if (ent->bot.current_node > 0) // Goto player location
		{
			for (int i = 0; i <= num_players; i++)
			{
				if (players[i] && players[i]->is_bot && players[i]->health > 0)
				{
					if (BOTLIB_CanGotoNode(players[i], ent->bot.current_node, false))
					{
						Com_Printf("%s %s visiting node[%i]\n", __func__, players[i]->client->pers.netname, ent->bot.current_node);
						//players[i]->bot.state = BOT_MOVE_STATE_MOVE;
						//BOTLIB_SetGoal(players[i], ent->bot.current_node);
					}
				}
			}
		}

		return true;
	}
	else if (Q_stricmp(cmd, "ttnode") == 0 && debug_mode) // Teleport player to node location
	{
		int node = atoi(gi.argv(1));

		if (node > INVALID && node < numnodes)
		{
			Com_Printf("%s Teleporting to node %i [%f %f %f]\n", __func__, node, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]);
			VectorCopy(nodes[node].origin, ent->s.origin);
		}

		return true;
	}
	else if (Q_stricmp(cmd, "tbtplayer") == 0 && debug_mode) // Teleport random bot to player location
	{
		for (int i = 0; i <= num_players; i++)
		{
			if (players[i] && players[i]->is_bot && players[i]->health > 0)
			{
				Com_Printf("%s Teleporting %s to player %s [%f %f %f]\n", __func__, players[i]->client->pers.netname, ent->client->pers.netname, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
				VectorCopy(ent->s.origin, players[i]->s.origin);
				return true;
			}
		}

		return true;
	}
	else if (Q_stricmp(cmd, "tbtnode") == 0 && debug_mode) // Teleport random bot to node location
	{
		int node = atoi(gi.argv(1));

		if (node > INVALID && node < numnodes)
		{
			for (int i = 0; i <= num_players; i++)
			{
				if (players[i] && players[i]->is_bot && players[i]->health > 0)
				{
					Com_Printf("%s Teleporting %s to node %i [%f %f %f]\n", __func__, players[i]->client->pers.netname, node, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]);
					VectorCopy(nodes[node].origin, players[i]->s.origin);
					return true;
				}
			}
		}

		return true;
	}
	else if (Q_stricmp(cmd, "ttloc") == 0 && debug_mode) // Teleport to location
	{
		float x = atof(gi.argv(1));
		float y = atof(gi.argv(2));
		float z = atof(gi.argv(3));

		Com_Printf("%s Teleporting to location [%f %f %f]\n", __func__, x, y, z);
		ent->s.origin[0] = x;
		ent->s.origin[1] = y;
		ent->s.origin[2] = z;

		return true;
	}
	else if (Q_stricmp(cmd, "bnode") == 0 && debug_mode) // Break all links to node
	{
		int node = atoi(gi.argv(1));

		if (node > 0 && node < numnodes)
			BOTLIB_RemoveAllNodeLinksFrom(node);

		return true;
	}
	else if (Q_stricmp(cmd, "cnode") == 0 && debug_mode) // Convert node
	{
		int node = atoi(gi.argv(1));
		int type = atoi(gi.argv(2));

		if (node > 0 && node < numnodes)
		{
			if (type == NODE_MOVE || type == NODE_JUMPPAD || type == NODE_POI)
			{
				Com_Printf("%s Converting node %i from type %i to %i\n", __func__, node, nodes[node].type, type);
				nodes[node].type = type;

				// Update node's ent details like color
				edict_t* ent;
				for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
				{
					if (ent && ent->node_num == node)
					{
						if (type == NODE_MOVE)
							ent->s.renderfx = RF_SHELL_BLUE;
						else if (type == NODE_JUMPPAD)
							ent->s.renderfx = RF_SHELL_GREEN;
						else if (type == NODE_POI)
							ent->s.renderfx = RF_SHELL_RED;

						break;
					}
				}
			}
		}

		return true;
	}
	else if (Q_stricmp(cmd, "bvnodes") == 0 && debug_mode) // Build vis nodes
	{
		ACEND_BuildVisibilityNodes();
		return true;
	}
	else if (Q_stricmp(cmd, "xynode") == 0 && debug_mode) // Can node X see node Y
	{
		int x = atoi(gi.argv(1));
		int y = atoi(gi.argv(2));

		if (x <= 0 || y <= 0 || x != y)
		{
			Com_Printf("Usage: xynode <x> <y>\n");
			return true;
		}

		if (ACEND_IsNodeVisibleToNodes(x, y))
			Com_Printf("%s Node %i can see node %i\n", __func__, x, y);
		else
			Com_Printf("%s Node %i cannot see node %i\n", __func__, x, y);

		return true;
	}
	else if (Q_stricmp(cmd, "xrngnode") == 0 && debug_mode) // Random node X can see
	{
		int x = atoi(gi.argv(1));

		if (x <= 0)
		{
			Com_Printf("Usage: xrngnode <x>\n");
			return true;
		}

		Com_Printf("%s Node %i can see random visible node %i\n", __func__, x, ACEND_GetRandomVisibleNode(x));

		return true;
	}
	//rekkie -- DEV_1 -- e
	//rekkie -- BSP -- e

	else
		return false;

	return true;
}