#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

botlib_noises_t botlib_noises;

// Returns the XYZ distance
float VectorDistance(vec3_t start, vec3_t end)
{
	vec3_t v = { 0 };
	VectorSubtract(end, start, v);
	return VectorLength(v);
}
// Returns the XY distance
float VectorDistanceXY(vec3_t start, vec3_t end)
{
	vec3_t v = { 0 };
	VectorSubtract(end, start, v);
	v[2] = 0;
	return VectorLength(v);
}

// Converts node type to string
// Types: NODE_MOVE, NODE_CROUCH, NODE_STEP, NODE_JUMP, NODE_JUMPPAD, NODE_STAND_DROP, NODE_CROUCH_DROP, NODE_UNSAFE_DROP, NODE_LADDER_UP, NODE_LADDER_DOWN, NODE_DOOR, NODE_PLATFORM, NODE_TELEPORTER, NODE_ITEM, NODE_WATER, NODE_GRAPPLE, NODE_SPAWNPOINT, NODE_POI, NODE_LEARN
// String: The string to copy the node type to
// Max string size: The maximum size of the string
// Returns: false if node type was not found
qboolean NodeTypeToString(edict_t* self, int type, char *string, const int max_string_size)
{
	//int len;
	switch (type)
	{
	case NODE_MOVE:
		strncpy(string, "MOVE", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_CROUCH:
		strncpy(string, "CROUCH", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_BOXJUMP:
		strncpy(string, "NODE_BOXJUMP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_JUMP:
		strncpy(string, "JUMP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_JUMPPAD:
		strncpy(string, "JUMPPAD", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_LADDER:
		strncpy(string, "LADDER", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_POI:
		strncpy(string, "POI", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_POI_LOOKAT:
		strncpy(string, "NODE_POI_LOOKAT", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_WATER:
		strncpy(string, "WATER", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;



	case NODE_STEP:
		strncpy(string, "STEP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_STAND_DROP:
		strncpy(string, "STAND_DROP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_CROUCH_DROP:
		strncpy(string, "CROUCH_DROP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_UNSAFE_DROP:
		strncpy(string, "UNSAFE_DROP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_LADDER_UP:
		strncpy(string, "LADDER_UP", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_LADDER_DOWN:
		strncpy(string, "LADDER_DOWN", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_DOOR:
		strncpy(string, "DOOR", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_PLATFORM:
		strncpy(string, "PLATFORM", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_TELEPORTER:
		strncpy(string, "TELEPORTER", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_ITEM:
		strncpy(string, "ITEM", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_GRAPPLE:
		strncpy(string, "GRAPPLE", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_SPAWNPOINT:
		strncpy(string, "SPAWNPOINT", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	case NODE_LEARN:
		strncpy(string, "LEARN", max_string_size - 1);
		string[max_string_size - 1] = '\0';
		break;
	default:
		strncpy(string, "UNKNOWN", max_string_size - 1);
		string[max_string_size - 1] = '\0'; // Ensure null-termination for all paths
		return false; // Unknown node type
	}

	return true; // Success
}

///////////////////////////////////////////////////////////////////////
// Bot Movement - Node Type - Utility
// Display all the node type names
// onlyPrintProblemTypes: only output if types are INVALID
///////////////////////////////////////////////////////////////////////
void PrintAllLinkNodeTypes(edict_t *self, qboolean onlyPrintProblemTypes)
{
	qboolean foundProblem = false;
	int curr_node_position = 0; // The position of the current node in the node_list array

	if (self->bot.node_list_count <= 0) // No nodes found
		return;

	for (int i = 0; i < self->bot.node_list_count; i++) // If the list has nodes
	{
		int node = self->bot.node_list[i]; // Next node in the list
		if (node != INVALID && node == self->bot.current_node) // If the node is valid
		{
			curr_node_position = i; // Save the position of the current node
			break;
		}
	}

	if (0) // Debug: print out the node list
	{
		Com_Printf("%s: count[%d] node_list[", __func__, self->bot.node_list_count);
		for (int i = curr_node_position; i < self->bot.node_list_count; i++) // Start at the current node position
		{
			Com_Printf(" %d ", self->bot.node_list[i]);
		}
		Com_Printf("]\n");
	}

	if (0) // Debug: print out the node types
	{
		int curr_node;
		int next_node;
		Com_Printf("%s: targetNodeTypes[", __func__);
		for (int i = curr_node_position; i < self->bot.node_list_count; i++) // Start at the current node position
		{
			if (i + 1 < self->bot.node_list_count) // If there is a next node
			{
				curr_node = self->bot.node_list[i]; // Get current node
				next_node = self->bot.node_list[i + 1]; // Get next node

				// Get the node type this links to
				for (int l = 0; l < nodes[self->bot.next_node].num_links; l++)
				{
					if (nodes[curr_node].links[l].targetNode == next_node) // If this is the link from curr to the next node
					{
						Com_Printf("%d ", nodes[curr_node].links[l].targetNodeType);
						//if (NodeTypeToString(self, nodes[self->bot.next_node].links[l].targetNodeType, after_next_type, sizeof(after_next_type)) == false)
						//	foundProblem = true;
						break;
					}
				}
			}
		}
		Com_Printf("]\n");
	}

	char tmp_string[32] = { '\0' }; // Length of the longest node type name
	char all_node_types[32 * 32] = { '\0' }; // 32 node types, 32 chars each
	int tmp_type;
	int curr_node;
	int next_node;
	//Com_Printf("%s: count[%d] node_list[", __func__, node_count);
	for (int i = curr_node_position; i < self->bot.node_list_count; i++) // Start at the current node position
	{
		if (i + 1 < self->bot.node_list_count) // If there is a next node
		{
			//Com_Printf(" %d ", node_list[i]);
			curr_node = self->bot.node_list[i]; // Get current node
			next_node = self->bot.node_list[i + 1]; // Get next node

			// Get the node type this links to
			for (int l = 0; l < nodes[self->bot.next_node].num_links; l++)
			{
				if (nodes[curr_node].links[l].targetNode == next_node) // If this is the link from curr to the next node
				{
					// Add the node number
					char tmp_num[9]; // Max size of a node number as a char: "[999999]\0" == (8 characters + NULL terminator = length of 9)
					sprintf(tmp_num, "[%d]", self->bot.node_list[i]);
					strcat(all_node_types, tmp_num);

					// Add the node type
					tmp_type = nodes[curr_node].links[l].targetNodeType;
					if (NodeTypeToString(self, tmp_type, tmp_string, sizeof(tmp_string)) == false)
						foundProblem = true;
					strcat(all_node_types, tmp_string);
					if (i + 2 < self->bot.node_list_count)
						strcat(all_node_types, " ");
					break;
				}
			}
		}
	}
	//Com_Printf("]\n");

	Com_Printf("%s: [Node] + Type [%s]\n", __func__, all_node_types);

	/*
	#define MAX_PRINT_ALL_LINK_NODE_TYPES 32
	int node_list[MAX_PRINT_ALL_LINK_NODE_TYPES];
	int node_count = BOTLIB_SLL_Query_All_Nodes(&self->pathList, node_list, MAX_PRINT_ALL_LINK_NODE_TYPES); // Retrieve the nodes in the list, if any

	if (node_count <= 0) // No nodes found
		return;

	
	if (0) // Debug: print out the node list
	{
		Com_Printf("%s: count[%d] node_list[", __func__, node_count);
		for (int i = 0; i < node_count; i++)
		{
			Com_Printf(" %d ", node_list[i]);
		}
		Com_Printf("]\n");
	}

	if (0) // Debug: print out the node types
	{
		int curr_node;
		int next_node;
		Com_Printf("%s: targetNodeTypes[", __func__);
		for (int i = 0; i < node_count; i++)
		{
			if (i + 1 < node_count) // If there is a next node
			{
				curr_node = node_list[i]; // Get current node
				next_node = node_list[i + 1]; // Get next node

				// Get the node type this links to
				for (int l = 0; l < nodes[self->bot.next_node].num_links; l++)
				{
					if (nodes[curr_node].links[l].targetNode == next_node) // If this is the link from curr to the next node
					{
						Com_Printf("%d ", nodes[curr_node].links[l].targetNodeType);
						//if (NodeTypeToString(self, nodes[self->bot.next_node].links[l].targetNodeType, after_next_type, sizeof(after_next_type)) == false)
						//	foundProblem = true;
						break;
					}
				}
			}
		}
		Com_Printf("]\n");
	}


	char tmp_string[32] = { '\0' }; // Length of the longest node type name
	char all_node_types[32 * 32] = { '\0' }; // 32 node types, 32 chars each
	int tmp_type;
	int curr_node;
	int next_node;
	//Com_Printf("%s: count[%d] node_list[", __func__, node_count);
	for (int i = 0; i < node_count; i++)
	{
		if (i + 1 < node_count) // If there is a next node
		{
			//Com_Printf(" %d ", node_list[i]);
			curr_node = node_list[i]; // Get current node
			next_node = node_list[i + 1]; // Get next node

			// Get the node type this links to
			for (int l = 0; l < nodes[self->bot.next_node].num_links; l++)
			{
				if (nodes[curr_node].links[l].targetNode == next_node) // If this is the link from curr to the next node
				{
					// Add the node number
					char tmp_num[6]; // Max size of a node number as a char: 999,999 (6 chars)
					sprintf(tmp_num, "[%d]", node_list[i]);
					strcat(all_node_types, tmp_num);

					// Add the node type
					tmp_type = nodes[curr_node].links[l].targetNodeType;
					if (NodeTypeToString(self, tmp_type, tmp_string, sizeof(tmp_string)) == false)
						foundProblem = true;
					strcat(all_node_types, tmp_string);
					if (i + 2 < node_count)
						strcat(all_node_types, " ");
					break;
				}
			}
		}
	}
	//Com_Printf("]\n");

	Com_Printf("%s: [Node] + Type [%s]\n", __func__, all_node_types);
	*/
		
#if 0
	char curr_type[64];
	char next_type[64];
	if (NodeTypeToString(self, type_from, curr_type, sizeof(next_type)) == false)
		foundProblem = true;
	if (NodeTypeToString(self, type_to, next_type, sizeof(next_type)) == false)
		foundProblem = true;

	// --------------------------------------------
	char after_next_type[64];
	after_next_type[0] = '\0';
	//int node_after_next = BOTLIB_SLL_Query_All_Nodes(&self->pathList); // Find out what node comes after the next node, if any
	if (node_after_next == INVALID)
	{
		strcpy(after_next_type, "INVALID"); // No next node
	}
	else
	{

		qboolean found_link_type = false;
		if (self->bot.next_node != INVALID)
		{
			for (int l = 0; l < nodes[self->bot.next_node].num_links; l++)
			{
				if (nodes[self->bot.next_node].links[l].targetNode == node_after_next)
				{
					Com_Printf("%s: targetNodeType[%d]\n", __func__, nodes[self->bot.next_node].links[l].targetNodeType);
					found_link_type = true;
					if (NodeTypeToString(self, nodes[self->bot.next_node].links[l].targetNodeType, after_next_type, sizeof(after_next_type)) == false)
						foundProblem = true;
					break;
				}
			}
		}
		if (found_link_type == false)
			strcpy(after_next_type, "INVALID"); // No next node


		//Com_Printf("%s: cur[%d] nxt[%d] nxt_nxt[%d]\n", __func__, self->bot.current_node, self->bot.next_node, node_after_next);
		if (NodeTypeToString(self, nodes[node_after_next].type, after_next_type, sizeof(after_next_type)) == false)
			foundProblem = true;
	}
	// --------------------------------------------

	if (onlyPrintProblemTypes) // Only print if errors found
	{
		if (foundProblem == false) // No error, so bail
			return;
	}

	if (self->groundentity)
		Com_Printf("%s: n[%d to %d] cur[%s] nxt[%s] nxt_nxt[%s]  [GROUND %d]\n", __func__, self->bot.current_node, self->bot.next_node, curr_type, next_type, after_next_type, self->last_touched_ground);
	else
		Com_Printf("%s: n[%d to %d] cur[%s] nxt[%s] nxt_nxt[%s]  [AIR %d] [ZVEL %f]\n", __func__, self->bot.current_node, self->bot.next_node, curr_type, next_type, after_next_type, self->last_touched_ground, self->velocity[2]);
#endif
}

///////////////////////////////////////////////////////////////////////
// Changes the bots view angle
//
// move_vector: The move direction
// 
// range: How close the current yaw needs to be to accepted as ideal
//        <= 0: The bot will snap turn instantly
//         > 0: Makes the change in angles a little more gradual, 
//              not so snappy. Subtle, but noticeable.
//        Default: 2
///////////////////////////////////////////////////////////////////////
qboolean BOTLIB_UTIL_ChangeBotAngleYaw(edict_t* ent, vec3_t move_vector, float range)
{
	float   ideal_yaw;
	float   current_yaw;
	float   speed;
	vec3_t  ideal_angle;
	float   yaw_move = 0.f;
	float   move_ratio = 1.f;

	// Normalize the move angle first
	VectorNormalize(move_vector);

	current_yaw = anglemod(ent->s.angles[YAW]);

	vectoangles(move_vector, ideal_angle);

	ideal_yaw = anglemod(ideal_angle[YAW]);

	if (current_yaw == ideal_yaw)
		return true;

	if (range <= 0) // Instant turn
	{
		ent->s.angles[YAW] = ideal_yaw;
		return true;
	}
	else
	{
		// Turn speed based on angle of curr vs ideal
		// If the turn angle is large, the bot will instantly snap
		// This helps prevent circular motion when very close to a node
		yaw_move = ideal_yaw - current_yaw;
		if (ideal_yaw > current_yaw)
		{
			//Com_Printf("%s %s ym:%f\n", __func__, ent->client->pers.netname, yaw_move);
			if (yaw_move >= 180)
				yaw_move = yaw_move - 360;
		}
		else
		{
			//Com_Printf("%s %s ym:%f\n", __func__, ent->client->pers.netname, yaw_move);
			if (yaw_move <= -180)
				yaw_move = yaw_move + 360;
		}
		//Com_Printf("%s %s c:%f  i:%f  ym:%f\n", __func__, ent->client->pers.netname, current_yaw, ideal_yaw, yaw_move);
		if (fabs(yaw_move) >= 45.0) // Snap turn
		{
			//Com_Printf("%s %s snap turn, normal: %f\n", __func__, ent->client->pers.netname, fabs(yaw_move));
			ent->s.angles[YAW] = ideal_yaw;
			return false;
		}
		/*
		float normal = 0;
		if (current_yaw > ideal_yaw)
		{
			normal = (ideal_yaw + 1) / (current_yaw + 1);
		}
		else
		{
			normal = (current_yaw + 1) / (ideal_yaw + 1);
		}
		if (normal < 0.5) // Snap turn when angle is large
		{
			Com_Printf("%s %s snap turn, normal: %f\n", __func__, ent->client->pers.netname, normal);
			ent->s.angles[YAW] = ideal_yaw;
			return true;
		}
		*/

		// Yaw turn speed
		if (current_yaw != ideal_yaw)
		{
			yaw_move = ideal_yaw - current_yaw;
			speed = ent->yaw_speed / (float)BASE_FRAMERATE;
			if (ideal_yaw > current_yaw)
			{
				if (yaw_move >= 180)
					yaw_move = yaw_move - 360;
			}
			else
			{
				if (yaw_move <= -180)
					yaw_move = yaw_move + 360;
			}
			if (yaw_move > 0)
			{
				if (yaw_move > speed)
					yaw_move = speed;
			}
			else
			{
				if (yaw_move < -speed)
					yaw_move = -speed;
			}
		}

		// Raptor007: Interpolate towards desired changes at higher fps.
		if (!FRAMESYNC)
		{
			int frames_until_sync = FRAMEDIV - (level.framenum - 1) % FRAMEDIV;
			move_ratio = 1.f / (float)frames_until_sync;
		}

		//rekkie -- s
		const float turn_speed_factor = 1.25; // Slow down turn speed
		move_ratio /= turn_speed_factor; // Slow turn speed by a factor of
		//rekkie -- e

		ent->s.angles[YAW] = anglemod(current_yaw + yaw_move * move_ratio);

		// Check if current_yaw is +/- from ideal_yaw
		//Com_Printf("%s: current_yaw %f ideal_yaw %f yaw_move %f\n", __func__, current_yaw, ideal_yaw, yaw_move);
		if (yaw_move < range && yaw_move > -(range))
			return true; // Ideal yaw reached
		else
			return false; // Ideal yaw not yet reached

	}
}

// Change in angles can be gradual to snappy
// move_vector : The move direction
// instant : Instant 'snap' turn
// move_speed : How fast to turn, 1.0 is fast and 3.0 is slow
// yaw : Allow changing the yaw (left & right)
// pitch : Allow changing the pitch (up & down)
// Returns true if angle reached, or false if still moving toward angle
qboolean BOTLIB_ChangeBotAngleYawPitch(edict_t* ent, vec3_t move_vector, qboolean instant, float move_speed, qboolean yaw, qboolean pitch)
{
	float   ideal_yaw;
	float   ideal_pitch;
	float   current_yaw;
	float   current_pitch;
	float   speed;
	vec3_t  ideal_angle;
	float   yaw_move = 0.f;
	float   pitch_move = 0.f;
	float   move_ratio = 1.f;

	// Normalize the move angle first
	VectorNormalize(move_vector);
	vectoangles(move_vector, ideal_angle);

	current_yaw = anglemod(ent->s.angles[YAW]);
	current_pitch = anglemod(ent->s.angles[PITCH]);

	ideal_yaw = anglemod(ideal_angle[YAW]);
	ideal_pitch = anglemod(ideal_angle[PITCH]);

	// Raptor007: Compensate for M4 climb.
	if ((ent->client->weaponstate == WEAPON_FIRING) && (ent->client->weapon == FindItem(M4_NAME)))
		ideal_pitch -= ent->client->kick_angles[PITCH];

	// Yaw
	if (instant) // Instant turn
	{
		ent->bot.bi.viewangles[YAW] = ideal_yaw;
	}
	else
	{
		// Turn speed based on angle of curr vs ideal
		// If the turn angle is large, the bot will instantly snap
		// This helps prevent circular motion when very close to a node
		yaw_move = ideal_yaw - current_yaw;
		if (ideal_yaw > current_yaw)
		{
			//Com_Printf("%s %s ym:%f\n", __func__, ent->client->pers.netname, yaw_move);
			if (yaw_move >= 180)
				yaw_move = yaw_move - 360;
		}
		else
		{
			//Com_Printf("%s %s ym:%f\n", __func__, ent->client->pers.netname, yaw_move);
			if (yaw_move <= -180)
				yaw_move = yaw_move + 360;
		}
		//Com_Printf("%s %s c:%f  i:%f  ym:%f\n", __func__, ent->client->pers.netname, current_yaw, ideal_yaw, yaw_move);
		//if (fabs(yaw_move) >= 45.0) // Snap turn
		//{
			//Com_Printf("%s %s snap turn, normal: %f\n", __func__, ent->client->pers.netname, fabs(yaw_move));
			//ent->bot.bi.viewangles[YAW] = ideal_yaw;
			//return false;
		//}

		if (current_yaw != ideal_yaw)
		{
			yaw_move = ideal_yaw - current_yaw;
			speed = ent->yaw_speed / (float)BASE_FRAMERATE;
			//Com_Printf("%s %s speed:%f\n", __func__, ent->client->pers.netname, speed);
			speed *= (360 / fabs(yaw_move)); // Variable speed. Far away = faster turning. Close = slower turnung.
			//Com_Printf("%s %s variable speed:%f\n", __func__, ent->client->pers.netname, speed);
			if (ideal_yaw > current_yaw)
			{
				if (yaw_move >= 180)
					yaw_move = yaw_move - 360;
			}
			else
			{
				if (yaw_move <= -180)
					yaw_move = yaw_move + 360;
			}
			if (yaw_move > 0)
			{
				if (yaw_move > speed)
					yaw_move = speed;
			}
			else
			{
				if (yaw_move < -speed)
					yaw_move = -speed;
			}
		}
	}
	

	// Pitch
	if (instant) // Instant turn
	{
		ent->bot.bi.viewangles[PITCH] = ideal_pitch;
	}

	//if (fabs(pitch_move) >= 45.0) // Snap turn
	//{
		//Com_Printf("%s %s snap turn, normal: %f\n", __func__, ent->client->pers.netname, fabs(pitch_move));
		//ent->bot.bi.viewangles[PITCH] = pitch_move;
		//return false;
	//}

	if (current_pitch != ideal_pitch)
	{
		pitch_move = ideal_pitch - current_pitch;
		speed = ent->yaw_speed / (float)BASE_FRAMERATE;
		speed *= (360 / fabs(pitch_move)); // Variable speed. Far away = faster turning. Close = slower turnung.
		if (ideal_pitch > current_pitch)
		{
			if (pitch_move >= 180)
				pitch_move = pitch_move - 360;
		}
		else
		{
			if (pitch_move <= -180)
				pitch_move = pitch_move + 360;
		}
		if (pitch_move > 0)
		{
			if (pitch_move > speed)
				pitch_move = speed;
		}
		else
		{
			if (pitch_move < -speed)
				pitch_move = -speed;
		}
	}

	if (instant) // Instant turn
		return true;

	/*
	// Raptor007: Interpolate towards desired changes at higher fps.
	if (!FRAMESYNC)
	{
		int frames_until_sync = FRAMEDIV - (level.framenum - 1) % FRAMEDIV;
		move_ratio = 1.f / (float)frames_until_sync;
	}
	*/

	//move_ratio = 1;
	move_ratio = 10.0 / HZ;
	//Com_Printf("%s %s move_ratio[%f]\n", __func__, ent->client->pers.netname, move_ratio);

	//rekkie -- s
	if (move_speed < 0.0)
		move_speed = 0.0;
	if (move_speed > 3.0)
		move_speed = 3.0;
	//float output = (10 - bot_skill->value) * 3 / 9; // [Min:0 Max:10] skill "10" == 0, Skill "0" == 3
	//Com_Printf("%s %s move_speed: %f\n", __func__, ent->client->pers.netname, move_speed);
	float turn_speed_factor = 1 + move_speed; //2.25; // Slow down turn speed
	move_ratio /= turn_speed_factor; // Slow turn speed by a factor of
	//rekkie -- e

	if (yaw)
		ent->bot.bi.viewangles[YAW] = anglemod(current_yaw + yaw_move * move_ratio);
	if (pitch)
		ent->bot.bi.viewangles[PITCH] = anglemod(current_pitch + pitch_move * move_ratio);

	// Check if yaw_move and pitch_move is +/- from ideal_yaw
	//Com_Printf("%s: current_yaw %f ideal_yaw %f yaw_move %f\n", __func__, current_yaw, ideal_yaw, yaw_move);
	if (yaw && pitch && yaw_move < 2 && yaw_move > -2 && pitch_move < 2 && pitch_move > -2)
	{
		if (yaw_move < 2 && yaw_move > -2 && pitch_move < 2 && pitch_move > -2)
			return true; // Ideal reached
		else
			return false;
	}
	if (yaw && !pitch)
	{
		if (yaw_move < 2 && yaw_move > -2)
			return true; // Ideal reached
		else
			return false;
	}
	if (!yaw && pitch)
	{
		if (pitch_move < 2 && pitch_move > -2)
			return true; // Ideal reached
		else
			return false;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
// Checks if bot can move (really just checking the ground)
// Also, this is not a real accurate check, but does a
// pretty good job and looks for lava/slime. 
///////////////////////////////////////////////////////////////////////
// Can move safely at angle
qboolean BOTLIB_CanMoveAngle(edict_t* self, vec3_t angle)
{
	vec3_t forward, right;
	vec3_t offset, start, end;
	trace_t tr;

	// Set up the vectors
	AngleVectors(angle, forward, right, NULL);

	VectorSet(offset, 32, 0, 0);
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	//VectorSet(offset, 32, 0, -32); // RiEvEr reduced drop distance to -110 NODE_MAX_FALL_HEIGHT
	//G_ProjectSource(self->s.origin, offset, forward, right, end);
	VectorCopy(start, end);
	end[2] -= 32;

	tr = gi.trace(start, NULL, NULL, end, self, MASK_PLAYERSOLID | MASK_OPAQUE);
	if (tr.fraction == 1.0)
		return false;

	/*
	if (((tr.fraction == 1.0) && !((lights_camera_action || self->client->uvTime) && CanMoveSafely(self, angle))) // avoid falling after LCA
		|| (tr.contents & MASK_DEADLY)							  // avoid SLIME or LAVA
		|| (tr.ent && (tr.ent->touch == hurt_touch)))			  // avoid MOD_TRIGGER_HURT
	{
		return false; // can't move
	}
	*/

	return true; // yup, can move
}
// Can move safely in direction
qboolean BOTLIB_CanMoveInDirection(edict_t* self, vec3_t direction, float fwd_dist, float down_dist, qboolean rand_fwd_angle)
{
	trace_t tr;
	vec3_t forward, right, start, end, offset, angles;
	qboolean safe_forward = false;
	qboolean safe_downward = false;

	vectoangles(direction, angles); // Direction to angles

	if (rand_fwd_angle) // Randomize the forward angle
	{
		angles[0] = 0;
		angles[1] += 22.5 + (random() * 270); // Get a random angle
		angles[2] = 0;
	}

	// Get forward start & end
	AngleVectors(angles, forward, right, NULL); // Angles to vectors
	VectorSet(offset, 0, 0, self->viewheight);
	G_ProjectSource(self->s.origin, offset, forward, right, start); // Get projection
	VectorMA(start, fwd_dist, forward, end); // Project forward

	// Test forward
	tr = gi.trace(start, tv(-16, -16, -5), tv(-16, -16, 32), end, self, (MASK_PLAYERSOLID | MASK_OPAQUE));
	if (tr.fraction == 1.0 && tr.startsolid == false) // No obstruction
	{
		safe_forward = true; // Way is safe forward
	}

	if (0)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}

	// Test downward
	VectorCopy(end, start); // Make the end the start
	end[2] -= down_dist; // Move the end down
	tr = gi.trace(start, NULL, NULL, end, self, MASK_PLAYERSOLID | MASK_OPAQUE);
	if (tr.fraction < 1.0 && tr.startsolid == false) // Found the ground
	{
		safe_downward = true; // Way is safe downward
	}

	if (0)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}

	// Convert angles back to direction
	AngleVectors(angles, direction, NULL, NULL);

	if (safe_forward && safe_downward)
		return true;
	else
		return false;
}
// Can move safely in direction
qboolean BOTLIB_CanMoveDir(edict_t* self, vec3_t direction)
{
	vec3_t forward, right;
	vec3_t offset, start, end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(direction, angles);
	VectorNormalize(angles);
	vectoangles(angles, angles);

	// Set up the vectors
	AngleVectors(angles, forward, right, NULL);

	VectorSet(offset, 36, 0, 24); // 24
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	VectorSet(offset, 36, 0, -NODE_MAX_FALL_HEIGHT); //-NODE_MAX_FALL_HEIGHT); // -110
	G_ProjectSource(self->s.origin, offset, forward, right, end);

	tr = gi.trace(start, NULL, NULL, end, self, MASK_PLAYERSOLID | MASK_OPAQUE); // Solid, lava, and slime

	if (((tr.fraction == 1.0) && !((lights_camera_action || self->client->uvTime) && BOTCOL_CanMoveSafely(self, angles))) // avoid falling after LCA
		|| (tr.contents & MASK_DEADLY)							  // avoid SLIME or LAVA
		|| (tr.ent && (tr.ent->touch == hurt_touch)))			  // avoid MOD_TRIGGER_HURT
	{
		return false; // can't move
	}

	return true; // yup, can move
}
//rekkie -- Quake3 -- e

///////////////////////////////////////////////////////////////////////
// Make the change in angles a little more gradual, not so snappy
// Subtle, but noticeable.
// 
// Modified from the original id ChangeYaw code...
///////////////////////////////////////////////////////////////////////
qboolean BOTLIB_MOV_ChangeBotAngleYawPitch(edict_t* ent)
{
	float   ideal_yaw;
	float   ideal_pitch;
	float   current_yaw;
	float   current_pitch;
	float   speed;
	vec3_t  ideal_angle;
	float   yaw_move = 0.f;
	float   pitch_move = 0.f;
	float   move_ratio = 1.f;

	// Normalize the move angle first
	VectorNormalize(ent->move_vector);

	current_yaw = anglemod(ent->s.angles[YAW]);
	current_pitch = anglemod(ent->s.angles[PITCH]);

	vectoangles(ent->move_vector, ideal_angle);

	ideal_yaw = anglemod(ideal_angle[YAW]);
	ideal_pitch = anglemod(ideal_angle[PITCH]);

	// Raptor007: Compensate for M4 climb.
	if ((ent->client->weaponstate == WEAPON_FIRING) && (ent->client->weapon == FindItem(M4_NAME)))
		ideal_pitch -= ent->client->kick_angles[PITCH];

	// Yaw
	if (current_yaw != ideal_yaw)
	{
		yaw_move = ideal_yaw - current_yaw;
		speed = ent->yaw_speed / (float)BASE_FRAMERATE;
		if (ideal_yaw > current_yaw)
		{
			if (yaw_move >= 180)
				yaw_move = yaw_move - 360;
		}
		else
		{
			if (yaw_move <= -180)
				yaw_move = yaw_move + 360;
		}
		if (yaw_move > 0)
		{
			if (yaw_move > speed)
				yaw_move = speed;
		}
		else
		{
			if (yaw_move < -speed)
				yaw_move = -speed;
		}
	}

	// Pitch
	if (current_pitch != ideal_pitch)
	{
		pitch_move = ideal_pitch - current_pitch;
		speed = ent->yaw_speed / (float)BASE_FRAMERATE;
		if (ideal_pitch > current_pitch)
		{
			if (pitch_move >= 180)
				pitch_move = pitch_move - 360;
		}
		else
		{
			if (pitch_move <= -180)
				pitch_move = pitch_move + 360;
		}
		if (pitch_move > 0)
		{
			if (pitch_move > speed)
				pitch_move = speed;
		}
		else
		{
			if (pitch_move < -speed)
				pitch_move = -speed;
		}
	}

	// Raptor007: Interpolate towards desired changes at higher fps.
	if (!FRAMESYNC)
	{
		int frames_until_sync = FRAMEDIV - (level.framenum - 1) % FRAMEDIV;
		move_ratio = 1.f / (float)frames_until_sync;
	}

	ent->s.angles[YAW] = anglemod(current_yaw + yaw_move * move_ratio);
	ent->s.angles[PITCH] = anglemod(current_pitch + pitch_move * move_ratio);

	// Check if yaw_move and pitch_move is +/- from ideal_yaw
	//Com_Printf("%s: current_yaw %f ideal_yaw %f yaw_move %f\n", __func__, current_yaw, ideal_yaw, yaw_move);
	if (yaw_move < 2 && yaw_move > -2 && pitch_move < 2 && pitch_move > -2)
		return true; // Ideal yaw reached
	else
		return false;
}

///////////////////////////////////////////////////////////////////////
// Handle special cases of crouch/jump
//
// If the move is resolved here, this function returns true
///////////////////////////////////////////////////////////////////////
qboolean Botlib_Crouch_Or_Jump(edict_t* self, usercmd_t* ucmd)
{
	vec3_t dir, forward, right, start, end, offset;
	vec3_t top;
	trace_t tr;

	// Get current direction
	VectorCopy(self->client->ps.viewangles, dir);
	dir[YAW] = self->s.angles[YAW];
	AngleVectors(dir, forward, right, NULL);

	VectorSet(offset, 0, 0, 0); // changed from 18,0,0
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	offset[0] += 18;
	G_ProjectSource(self->s.origin, offset, forward, right, end);


	start[2] += 18; // so they are not jumping all the time
	end[2] += 18;
	tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);


	if (tr.fraction < 1.0)
	{
		//rekkie -- DEV_1 -- s
		// Handles cases where bots get stuck on each other, for example in teamplay where they can block each other
		// So lets try to make them jump over each other
		if (tr.ent && tr.ent->is_bot && self->groundentity)
		{
			if (random() > 0.5) // try jumping
			{
				if (debug_mode && level.framenum % HZ == 0)
					debug_printf("%s %s: move blocked ----------------------- [[[[[ JUMPING ]]]]]] ---------\n", __func__, self->client->pers.netname);
				self->velocity[2] += 400;
				//ucmd->upmove = SPEED_RUN;
				ucmd->forwardmove = SPEED_RUN;
				return true;
			}
			else // try crouching
			{
				if (debug_mode && level.framenum % HZ == 0)
					debug_printf("%s %s: move blocked ----------------------- [[[[[ CROUCH ]]]]]] ---------\n", __func__, self->client->pers.netname);
				ucmd->upmove = -SPEED_RUN;
				ucmd->forwardmove = SPEED_RUN;
				return true;
			}
		}
		//rekkie -- DEV_1 -- e

		// Check for crouching
		start[2] -= 14;
		end[2] -= 14;
		VectorCopy(self->maxs, top);
		top[2] = 0.0; // crouching height
		tr = gi.trace(start, self->mins, top, end, self, MASK_PLAYERSOLID);
		if (tr.fraction == 1.0)
		{
			if (debug_mode) debug_printf("%s %s attempted to crouch under obstacle\n", __func__, self->client->pers.netname);
			ucmd->forwardmove = SPEED_RUN;
			ucmd->upmove = -SPEED_RUN;
			return true;
		}

		// Check for jump
		start[2] += 32;
		end[2] += 32;
		tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);
		if (tr.fraction == 1.0)
		{
			if (debug_mode) debug_printf("%s %s attempted to jump over obstacle\n", __func__, self->client->pers.netname);
			ucmd->forwardmove = SPEED_RUN;
			ucmd->upmove = SPEED_RUN;
			return true;
		}
	}

	return false; // We did not resolve a move here
}

//rekkie -- Quake3 -- s
// Special cases of crouch/jump when not using nodes
// Also resolves jumping or crouching under other players/bots in our way
// Returns the action taken: ACTION_JUMP, ACTION_CROUCH, ACTION_NONE
int BOTLIB_Crouch_Or_Jump(edict_t* self, usercmd_t* ucmd, vec3_t dir)
{
	vec3_t angle, forward, right, start, end, offset, origin;
	trace_t tr;
	qboolean crouch = false, head = false, jump = false; // Crouch space, head space, jump space

	// Get current direction
	vectoangles(dir, angle);
	AngleVectors(angle, forward, right, NULL);
	VectorCopy(self->s.origin, origin);
	origin[2] -= 24; // From the ground up

	VectorSet(offset, 0, 0, 0);
	G_ProjectSource(origin, offset, forward, right, start);
	offset[0] += 1; // Distance forward dir
	G_ProjectSource(origin, offset, forward, right, end);

	if (0)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}


	// Check if we hit our head
	tr = gi.trace(tv(start[0], start[1], start[2] + 56), tv(-15, -15, 0), tv(15, 15, 1), tv(start[0], start[1], start[2] + 57), self, MASK_PLAYERSOLID);
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		head = true; // Hit our head on something immediately above us
	}

	//vec3_t mins = { self->mins[0], self->mins[1], self->mins[2] - 0.15 };
	//vec3_t maxs = { self->maxs[0], self->maxs[1], self->maxs[2] };


	tr = gi.trace(start, tv(-16, -16, STEPSIZE), tv(16, 16, 56), end, self, MASK_PLAYERSOLID);
	if (tr.fraction < 1.0 || tr.startsolid)
	{

		// Handle cases where bots get stuck on each other, for example in teamplay where they can block each other
		// So lets try to make them jump or crouch
		if (tr.ent && tr.ent->is_bot && self->groundentity)
		{
			if (random() > 0.5) // try jumping
			{
				//if (debug_mode && level.framenum % HZ == 0)
				//	Com_Printf("%s %s: move blocked ----------------------- [[[[[ JUMPING ]]]]]] ---------\n", __func__, self->client->pers.netname);
				return ACTION_JUMP;
			}
			else // try crouching
			{
				//if (debug_mode && level.framenum % HZ == 0)
				//	Com_Printf("%s %s: move blocked ----------------------- [[[[[ CROUCH ]]]]]] ---------\n", __func__, self->client->pers.netname);
				return ACTION_CROUCH;
			}
		}

		// Handle geometry cases
		// Check for crouching
		tr = gi.trace(start, tv(-16, -16, STEPSIZE), tv(16, 16, STEPSIZE), end, self, MASK_PLAYERSOLID); //CROUCHING_MAXS2, 32 --
		if (tr.fraction == 1.0)
		{
			crouch = true; // We can crouch under something
		}

		// Check for jump
		tr = gi.trace(start, tv(-16,-16, 60), tv(16, 16, 61), end, self, MASK_PLAYERSOLID);
		if (tr.fraction == 1.0 && tr.startsolid == false)
		{
			if (ACEMV_CanMove(self, MOVE_FORWARD))
				jump = true; // We can jump over something
		}

		//Com_Printf("%s %s crouch[%d] head[%d] jump[%d]\n", __func__, self->client->pers.netname, crouch, head, jump);

		if ((crouch || head) && jump == false)
		{
			//Com_Printf("%s %s attempted to crouch under obstacle\n", __func__, self->client->pers.netname);
			return ACTION_CROUCH;
		}
		if (jump && (crouch == false && head == false))
		{
			//Com_Printf("%s %s attempted to jump over obstacle\n", __func__, self->client->pers.netname);
			return ACTION_JUMP;
		}
	}

	return ACTION_NONE; // We did not resolve a move here
}

// Checks the bots movement direction against the direction of the next node. 
// 
int BOTLIB_DirectionCheck(edict_t *ent, byte *mov_strafe)
{
	if (ent->bot.next_node == INVALID)
		return 0;

	// Check velocity
	if (VectorEmpty(ent->velocity))
		return 0;

	// Get the bot's direction based from their velocity
	vec3_t walkdir;
	VectorSubtract(ent->s.origin, ent->lastPosition, walkdir);
	VectorNormalize(walkdir);
	vec3_t angle, forward, right, start, end, origin, offset;
	vectoangles(walkdir, angle);
	//VectorCopy(ent->s.angles, angle); // Use the bots's view angles (not their walk direction)
	AngleVectors(angle, forward, right, NULL);

	VectorCopy(forward, ent->bot.bot_walk_dir); // Copy the forward walk direction


	// Calculate the direction from the bot to the node
	vec3_t node_dir;
	VectorSubtract(nodes[ent->bot.next_node].origin, ent->s.origin, node_dir);
	VectorNormalize(node_dir);

	// Calculate the dot product of the forward dir and the node dir
	float dot = DotProduct(forward, node_dir);
	// Calculate the dot to degrees
	//float degrees = acos(dot) * 180 / M_PI;
	//Com_Printf("%s node %d dot %f degrees %f\n", __func__, ent->bot.next_node, dot, degrees);
	//if (dot > 0.995) 
	//	Com_Printf("%s node %d dot %f\n", __func__, ent->bot.next_node, dot);

	float dot_right = DotProduct(right, node_dir);
	VectorNegate(right, right);
	float dot_left = DotProduct(right, node_dir);
	if (dot_right > dot_left) // 
	{
		//Com_Printf("%s [MOVE RIGHT] node %d dot %f dot_right %f dot_left %f\n", __func__, ent->bot.highlighted_node, dot, dot_right, dot_left);
		*mov_strafe = 1;
	}
	else
	{
		//Com_Printf("%s [MOVE LEFT] node %d dot %f  dot_right %f dot_left %f\n", __func__, ent->bot.highlighted_node, dot, dot_right, dot_left);
		*mov_strafe = -1;
	}

	if (0) // Show visual debug
	{
		VectorCopy(ent->s.origin, origin);
		origin[2] += 8; // [Origin 24 units] + [8 units] == 32 units heigh (same as node height)

		VectorSet(offset, 0, 0, 0);
		G_ProjectSource(origin, offset, forward, right, start);
		offset[0] += 1024; // Distance forward dir
		G_ProjectSource(origin, offset, forward, right, end);


		// Bot walk direction
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(ent->s.origin, MULTICAST_PHS);

		/*
		// Bot to node direction
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(nodes[ent->bot.next_node].origin);
		gi.multicast(ent->s.origin, MULTICAST_PHS);
		*/
	}

	return dot;
}
//rekkie -- Quake3 -- e

// Distance types from origin [self, current node, next node]
enum {
	BOT_DIST_XYZ_SELF_CURR = 1, // [XYZ] Self -> Current node
	BOT_DIST_XYZ_SELF_NEXT,		// [XYZ] Self -> Next node
	BOT_DIST_XYZ_CURR_NEXT,		// [XYZ] Current node -> Next node
	
	BOT_DIST_XY_SELF_CURR,		// [XY] Self -> Current node
	BOT_DIST_XY_SELF_NEXT,		// [XY] Self -> Next node
	BOT_DIST_XY_CURR_NEXT		// [XY] Current node -> Next node
};
// Returns the distance between two points, either as XYZ or XY
// In the case of XY, the vec3_t dist is also returned with the Z value set to 0
float BOTLIB_UTIL_Get_Distance(edict_t* self, vec3_t dist, int type)
{
	switch (type)
	{
	// XYZ
	case BOT_DIST_XYZ_SELF_CURR:
		VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, dist);
		return VectorLength(dist);
	case BOT_DIST_XYZ_SELF_NEXT:
		VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		return VectorLength(dist);
	case BOT_DIST_XYZ_CURR_NEXT:
		VectorSubtract(nodes[self->bot.next_node].origin, nodes[self->bot.current_node].origin, dist);
		return VectorLength(dist);

	// XY
	case BOT_DIST_XY_SELF_CURR:
		VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, dist);
		dist[2] = 0; // Remove the Z component
		return VectorLength(dist);
	case BOT_DIST_XY_SELF_NEXT:
		VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		dist[2] = 0; // Remove the Z component
		return VectorLength(dist);
	case BOT_DIST_XY_CURR_NEXT:
		VectorSubtract(nodes[self->bot.next_node].origin, nodes[self->bot.current_node].origin, dist);
		dist[2] = 0; // Remove the Z component
		return VectorLength(dist);
	default:
		return 0;
	}
}

// Determines the closest point (between curr -> next node) the bot is nearest to
//
//                  [bot]
// [curr] -----------[*]---------------------------- [next]
//
//  [*] is the origin nearest to the bot along the path from [curr] to [next]
//
void BOTLIB_UTIL_NEAREST_PATH_POINT(edict_t* self, usercmd_t* ucmd)
{
	if (self->bot.next_node == INVALID) return;

	// Get distance from bot to next node
	vec3_t bot_to_next_vec;
	VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_next_vec);
	float bot_to_next_dist = VectorLength(bot_to_next_vec);

	// Get distance from current node to next node
	vec3_t curr_to_next_vec;
	VectorSubtract(nodes[self->bot.next_node].origin, nodes[self->bot.current_node].origin, curr_to_next_vec);
	float curr_to_next_dist = VectorLength(curr_to_next_vec);

	// Calculate the normalized distance the bot has travelled between current and next node
	float bot_travelled_dist = 0;
	if (bot_to_next_dist < curr_to_next_dist)
		bot_travelled_dist = 1 - (bot_to_next_dist / curr_to_next_dist);
	else
		bot_travelled_dist = 1 - (curr_to_next_dist / bot_to_next_dist);

	//Com_Printf("%s %s [b->n %f] [c->n %f] [%f]\n", __func__, ent->client->pers.netname, bot_to_next_dist, curr_to_next_dist, bot_travelled_dist);

	// Get the origin between the current and next node based on the normalized bot_travelled_dist
	LerpVector(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin, bot_travelled_dist, self->nearest_path_point);
}

// Determines if we're off course while traversing a path (node1 ---> node2)
// Useful for strafing to get back on course
// 
// Returns: '-1' if we're off course to the right (indicating we should strafe left)
// Returns: '0'  if we're on course (or close enough)
// Returns: '1'  if we're off course to the left (indicating we should strafe right)
int BOTLIB_UTIL_PATH_DEVIATION(edict_t* self, usercmd_t* ucmd)
{
	if (self->bot.next_node == INVALID) return 0;
	/*
	// Get distance from bot to next node
	vec3_t bot_to_next_vec;
	VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_next_vec);
	float bot_to_next_dist = VectorLength(bot_to_next_vec);

	// Get distance from current node to next node
	vec3_t curr_to_next_vec;
	VectorSubtract(nodes[self->bot.next_node].origin, nodes[self->bot.current_node].origin, curr_to_next_vec);
	float curr_to_next_dist = VectorLength(curr_to_next_vec);

	// Calculate the normalized distance the bot has travelled between current and next node
	float bot_travelled_dist = 0;
	if (bot_to_next_dist < curr_to_next_dist)
		bot_travelled_dist = 1 - (bot_to_next_dist / curr_to_next_dist);
	else
		bot_travelled_dist = 1 - (curr_to_next_dist / bot_to_next_dist);


	//Com_Printf("%s %s [b->n %f] [c->n %f] [%f]\n", __func__, ent->client->pers.netname, bot_to_next_dist, curr_to_next_dist, bot_travelled_dist);

	// Get the origin between the current and next node based on the normalized bot_travelled_dist
	vec3_t normalized_origin;
	LerpVector(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin, bot_travelled_dist, normalized_origin);

	VectorCopy(normalized_origin, self->nearest_path_point); // Make a copy
	*/

	BOTLIB_UTIL_NEAREST_PATH_POINT(self, ucmd); // Update the nearest path point

	// Show a laser between next node and normalized_origin
	if (1)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(nodes[self->bot.next_node].origin);
		gi.WritePosition(self->nearest_path_point);
		gi.multicast(nodes[self->bot.next_node].origin, MULTICAST_PVS);
	}

	// Calculate the angle between the bot's origin and the normalized origin
	vec3_t normalized_origin_vec;
	VectorSubtract(self->nearest_path_point, self->s.origin, normalized_origin_vec);
	normalized_origin_vec[2] = 0; // Flatten the vector
	float bot_to_path_dist = VectorLength(normalized_origin_vec);
	//Com_Printf("%s %s [d %f]\n", __func__, self->client->pers.netname, bot_to_path_dist);

	// Calculate the angle between the bot's origin and the normalized origin
	float current_yaw = anglemod(self->s.angles[YAW]);
	vec3_t ideal_angle;
	vectoangles(normalized_origin_vec, ideal_angle);
	float ideal_yaw = anglemod(ideal_angle[YAW]);

	// print current_yaw and ideal_yaw
	//Com_Printf("%s %s [c %f] [i %f]\n", __func__, self->client->pers.netname, current_yaw, ideal_yaw);

	// Figure out if current yaw is closer to idea yaw if we turn left
	//const float less_than_90_degrees = 87; // 90 is the optimum angle, but we want to give a little leeway (otherwise the bot's yaw movement will oscillate)
	float left_yaw;
	float right_yaw;

	if (bot_to_path_dist > 16) // Give a little leeway (otherwise the bot's yaw movement will oscillate)
	{
		if (ideal_yaw > current_yaw)
		{
			right_yaw = ideal_yaw - current_yaw;
			left_yaw = 360 - right_yaw;
		}
		else
		{
			left_yaw = current_yaw - ideal_yaw;
			right_yaw = 360 - left_yaw;
		}

		//Com_Printf("%s %s [c %f] [i %f] [ry %f] [ly %f] [dist %f]\n", __func__, ent->client->pers.netname, current_yaw, ideal_yaw, right_yaw, left_yaw, bot_to_path_dist);

		if (right_yaw < left_yaw)
		{
			//Com_Printf("%s %s need to strafe left %f\n", __func__, self->client->pers.netname, left_yaw);
			//if (ACEMV_CanMove(self, MOVE_LEFT))
			{
				//Com_Printf("%s %s --> strafe left %f\n", __func__, self->client->pers.netname, left_yaw);
				//ucmd->sidemove = -SPEED_WALK;
				return -1;  // Strafe left
			}
		}
		else
		{
			//Com_Printf("%s %s need to strafe right %f\n", __func__, self->client->pers.netname, left_yaw);
			//if (ACEMV_CanMove(self, MOVE_RIGHT))
			{
				//Com_Printf("%s %s --> strafe right %f\n", __func__, self->client->pers.netname, right_yaw);
				//ucmd->sidemove = SPEED_WALK;
				return 1; // Strafe right
			}
		}
	}

	return 0; // Strafe neither left or right
}

// Determines if forward vector is clear of obstructions while traversing a path (node1 ---> node2)
// Useful for strafing around obstacles while following a path
// 
// forward_distance: distance to probe forward
// side_distance: distance to probe left or right ( less than 0 = left, greater than 0 = right )
// 
// Returns: 'true' if forward vector is clear of obstructions, 
// Returns: 'false' if forward vector is obstructed
qboolean BOTLIB_UTIL_PATH_FORWARD(edict_t* self, usercmd_t* ucmd, float forward_distance, float side_distance)
{
	// Send out trace on the left or right side parallel to the current direction the bot is facing

	vec3_t groundvec, end;
	vec3_t offset, forward, right, start, down_left_end; // , down_right_end, up_left_end, up_right_end;
	float forward_back, left_or_right, up_or_down, beam_width;

	groundvec[PITCH] = 0;
	groundvec[YAW] = self->client->v_angle[YAW];
	groundvec[ROLL] = 0;

	if (forward_distance == 0)
		forward_distance = 64; // Distance to probe forward

	AngleVectors(groundvec, forward, NULL, NULL);	// Make a forwards pointing vector out of the bot's view angles
	VectorMA(self->s.origin, forward_distance, forward, end);	// Make end equal to 60* the forward pointing vector

	beam_width = 2;	// The actual width is '4' (found in CL_ParseLaser() tent.c) - but slightly reduced it here because it's easier to see when it intersects with a wall
	forward_back = self->maxs[PITCH] + beam_width;	// + forward, - backward

	// +Right or -Left of the bot's current direction
	if (side_distance > 0) // side_distance is greater than zero
		left_or_right = self->maxs[YAW] + side_distance;	// (+maxs) + (-side_distance) ( right is a positive number )
	else // side_distance is less than zero
		left_or_right = self->mins[YAW] + side_distance;	// (-mins) + (-side_distance) ( left is a negative number )

	// +Up or -Down (from the ground) 
	up_or_down = self->mins[ROLL] + STEPSIZE;			// (-mins) + (+STEPSIZE)   -- Player bounding box (-32 + 18) = -14

	VectorSet(offset, forward_back, left_or_right, up_or_down);
	AngleVectors(groundvec, forward, right, NULL);
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	VectorMA(start, forward_distance, forward, down_left_end);	// Make end equal to 200* the forward pointing vector

	if (1)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(down_left_end);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}

	VectorMA(start, (forward_distance - 16), forward, down_left_end);	// Move the end point back -16 units (mins[0])
	trace_t tr = gi.trace(start, tv(-12, -12, 0.1), tv(12, 12, 56), down_left_end, self, MASK_MONSTERSOLID);
	//trace_t tr = gi.trace(start, NULL, NULL, down_left_end, self, MASK_MONSTERSOLID);
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		return true; // Obstructed
	}

	return false; // Clear
}

//rekkie -- Quake3 -- s
// Determines if forward vector is clear of obstructions while traversing a path (node1 ---> node2)
// Useful for strafing around obstacles while following a path
// 
// dir: direction vector
// forward_distance: distance to probe forward
// side_distance: distance to probe left or right ( less than 0 = left, greater than 0 = right )
// 
// Returns: 'true' if forward vector is clear of obstructions, 
// Returns: 'false' if forward vector is obstructed
qboolean BOTLIB_TEST_FORWARD_VEC(edict_t* self, vec3_t dir, float forward_distance, float side_distance)
{
	// Send out trace on the left or right side parallel to the current direction the bot is facing

	vec3_t groundvec;
	vec3_t offset, forward, right, start, down_left_end; // , down_right_end, up_left_end, up_right_end;
	float forward_back, left_or_right, up_or_down, beam_width;

	if (forward_distance == 0)
		forward_distance = 64; // Distance to probe forward

	beam_width = 2;	// The actual width is '4' (found in CL_ParseLaser() tent.c) - but slightly reduced it here because it's easier to see when it intersects with a wall
	forward_back = self->maxs[PITCH] + beam_width;	// + forward, - backward

	// +Right or -Left of the bot's current direction
	if (side_distance > 0) // side_distance is greater than zero
		left_or_right = self->maxs[YAW] + side_distance;	// (+maxs) + (-side_distance) ( right is a positive number )
	else // side_distance is less than zero
		left_or_right = self->mins[YAW] + side_distance;	// (-mins) + (-side_distance) ( left is a negative number )
	// +Up or -Down (from the ground) 
	up_or_down = self->mins[ROLL] + STEPSIZE;			// (-mins) + (+STEPSIZE)   -- Player bounding box (-32 + 18) = -14

	VectorSet(offset, forward_back, left_or_right, up_or_down);
	vectoangles(dir, groundvec);
	AngleVectors(groundvec, forward, right, NULL);
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	VectorMA(start, forward_distance, forward, down_left_end);	// Make end equal to 200* the forward pointing vector

	if (0)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(down_left_end);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}

	VectorMA(start, (forward_distance - 16), forward, down_left_end);	// Move the end point back -16 units (mins[0])
	trace_t tr = gi.trace(start, tv(-4, -4, STEPSIZE+0.01), tv(4, 4, 56), down_left_end, self, MASK_MONSTERSOLID);
	//trace_t tr = gi.trace(start, tv(-12, -12, 0.1), tv(12, 12, 56), down_left_end, self, MASK_MONSTERSOLID);
	//trace_t tr = gi.trace(start, NULL, NULL, down_left_end, self, MASK_MONSTERSOLID);
	if (tr.fraction < 1.0 || tr.startsolid)
	{
		return true; // Obstructed
	}

	return false; // Clear
}
//rekkie -- Quake3 -- e

///////////////////////////////////////////////////////////////////////
// Bot Movement (based on node type)
///////////////////////////////////////////////////////////////////////
void BOTLIB_MOV_Move(edict_t* self, usercmd_t* ucmd)
{
#if 0
	trace_t tr;
	float distance = 0;
	//float distance_xy; // Distance XYZ and Distance XY
	vec3_t dist = { 0 };
	//vec3_t dist_xy = { 0 }; // Distance XYZ and Distance XY
	vec3_t velocity;
	int i = 0, current_node_type = INVALID, next_node_type = INVALID;

	


	// Do not follow path when teammates are still inside us.
	if (OnTransparentList(self))
	{
		//rekkie -- DEV_1 -- s

		// If the bot has just spawned, then we need to wait a bit before we start moving.
		if (self->just_spawned) // set by SpawnPlayers() in a_team.c
		{
			self->just_spawned = false;
			self->just_spawned_go = true; // Bot is ready, when wander_timeout is reached.
			self->bot.state = BOT_MOVE_STATE_MOVE;
			if (random() < 0.3)
				self->just_spawned_timeout = level.framenum + (random() * 7) * HZ;	// Long wait
			else if (random() < 0.7)
				self->just_spawned_timeout = level.framenum + (random() * 3) * HZ;	// Average wait
			else
				self->just_spawned_timeout = level.framenum + random() * HZ;		// Short wait
			return;
		}
		if (self->just_spawned_go && self->just_spawned_timeout > level.framenum) // Wait
		{
			return; // It's not time to move yet, wait!
		}
		else if (self->just_spawned_go)
		{
			// Now we can move!
			self->just_spawned_go = false;
			//ACEAI_PickLongRangeGoal(self); // Lets pick a long range goal
			return;
		}
		if (0) // Don't wander
		{
			//rekkie -- DEV_1 -- e

			self->bot.state = STATE_WANDER;
			self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
			return;

			//rekkie -- DEV_1 -- s
		}
		//rekkie -- DEV_1 -- e
	}

	// Get current and next node back from nav code.
	{
		if (!teamplay->value || (teamplay->value && level.framenum >= self->teamPauseTime))
		{
			self->bot.state = STATE_WANDER;
			self->wander_timeout = level.framenum + 1.0 * HZ;
		}
		else
		{
			// Teamplay mode - just fan out and chill
			if (self->bot.state == STATE_FLEE)
			{
				self->bot.state = STATE_POSITION;
				self->wander_timeout = level.framenum + 3.0 * HZ;
			}
			else
			{
				self->bot.state = STATE_POSITION;
				self->wander_timeout = level.framenum + 1.0 * HZ;
			}
		}
		self->bot.goal_node = INVALID;
		return;
	}

	if (self->bot.current_node == INVALID || self->bot.next_node == INVALID) // rekkie -- safey check because current / next node can be INVALID
	{
		self->bot.state = STATE_WANDER;
		self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
		return;
	}


	
	// Current node type
	current_node_type = nodes[self->bot.current_node].type;

	// Next node type
	// We now get the next node type from the link between current and next node
	// Using the link from the current node to the next node, find the next node type
	int targetNodeLink = INVALID; // Used to keep track of which link to upgrade to a jumppad
	if (nodes[self->bot.next_node].type == NODE_LADDER_UP || nodes[self->bot.next_node].type == NODE_LADDER_DOWN)
	{
		//targetNodeLink = 1;
		next_node_type = nodes[self->bot.next_node].type;
	}
	else
	{
		for (i = 0; i < nodes[self->bot.current_node].num_links; i++)   //for (i = 0; i < MAXLINKS; i++)
		{
			if (nodes[self->bot.current_node].links[i].targetNode == self->bot.next_node)
			{
				targetNodeLink = i;
				next_node_type = nodes[self->bot.current_node].links[i].targetNodeType; // Next node type

				self->prev_to_curr_node_type = self->curr_to_next_node_type; // Previous node type
				self->curr_to_next_node_type = nodes[self->bot.current_node].links[i].targetNodeType; // Next node type
				break;
			}
		}
	}

	//if (next_node_type == NODE_JUMP)
	//	Com_Printf("%s %s [cn:%i] JUMP_NODE [nn:%i]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node);


	//rekkie -- DEV_1 -- s
	//if (current_node_type == NODE_JUMPPAD || next_node_type == NODE_JUMPPAD)
	//	gi.dprintf("%s %s [ct:%i] [nt:%i]\n", __func__, self->client->pers.netname, current_node_type, next_node_type);

	// If the next node is high enough, make it a jump
	/*
	if (next_node_type == NODE_MOVE && self->client->leg_damage == 0)
	{

		// See if there is a gap / hole between the current and next node, if so we need to jump across
		vec3_t pt_25, pt_50, pt_75;
		VectorAdd(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin, pt_25);	// 25%
		VectorAdd(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin, pt_50);  // 50%
		VectorAdd(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin, pt_75);	// 75%
		VectorScale(pt_25, 0.25, pt_25); // Scale 25%
		VectorScale(pt_50, 0.50, pt_50); // Scale 50%
		VectorScale(pt_75, 0.75, pt_75); // Scale 75%
		// Using the points between the current and next node, trace a line down and see if one of them hits air
		trace_t tr_25, tr_50, tr_75;
		vec3_t end_25, end_50, end_75;
		VectorCopy(pt_25, end_25);
		VectorCopy(pt_50, end_50);
		VectorCopy(pt_75, end_75);
		end_25[2] -= NODE_MAX_JUMP_HEIGHT;
		end_50[2] -= NODE_MAX_JUMP_HEIGHT;
		end_75[2] -= NODE_MAX_JUMP_HEIGHT;
		tr_25 = gi.trace(pt_25, vec3_origin, vec3_origin, end_25, self, MASK_SOLID);
		tr_50 = gi.trace(pt_50, vec3_origin, vec3_origin, end_50, self, MASK_SOLID);
		tr_75 = gi.trace(pt_75, vec3_origin, vec3_origin, end_75, self, MASK_SOLID);
		// If one of our trace points hit nothing
		if ( (tr_25.fraction == 1.0 && tr_25.allsolid == false) ||
			(tr_50.fraction == 1.0 && tr_50.allsolid == false) ||
			(tr_75.fraction == 1.0 && tr_75.allsolid == false) )
		{
			next_node_type = NODE_JUMPPAD;
		}

		// Otherwise lets see if the next_node is higher than current_node, if so jump to it
		//else
		{
			// Distance between current and next node.
			//VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
			//distance = VectorLength(dist);

			// Calculate if nodes[self->bot.next_node].origin[2] is higher than self->s.origin[2]
			float higher = 0; float lower = 0;
			if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2])
				higher = (nodes[self->bot.next_node].origin[2] - nodes[self->bot.current_node].origin[2]);
			else if (nodes[self->bot.next_node].origin[2] < nodes[self->bot.current_node].origin[2])
				lower = (nodes[self->bot.current_node].origin[2] - nodes[self->bot.next_node].origin[2]);
			//if (distance >= 128 || higher >= NODE_MAX_JUMP_HEIGHT || lower >= NODE_MAX_JUMP_HEIGHT)
			//if (higher >= NODE_MAX_JUMP_HEIGHT) // || lower >= NODE_MAX_CROUCH_FALL_HEIGHT)
			if (higher) // || lower)
			{
				next_node_type = NODE_JUMPPAD;
			}
		}
	}
	*/

	if (current_node_type == INVALID || next_node_type == INVALID)
	{
		Com_Printf("%s curr[%d]type[%d]   next[%d]type[%d]   Goal[%d]  State[%d]\n", __func__, self->bot.current_node, current_node_type, self->bot.next_node, next_node_type, self->bot.goal_node, self->bot.state);
	}


	// DEBUG: print current and next node types
	//if (1)
	{
		//const qboolean printOnlyErrors = true; // If only printing INVALID nodes
		//PrintNodeType(self, current_node_type, next_node_type, printOnlyErrors);
	}






	// Contents check
	vec3_t temp = { 0,0,0 };
	VectorCopy(self->s.origin, temp);
	temp[2] += 22;
	int contents_head = gi.pointcontents(temp);
	temp[2] = self->s.origin[2] - 8;
	int contents_feet = gi.pointcontents(temp);
	// If in water, force bot to wander...
	if (contents_feet & MASK_WATER)
	{
		self->bot.state = STATE_WANDER;
		return;
		//self->bot.goal_node = INVALID;
		//self->bot.current_node = INVALID;
		//Com_Printf("%s %s is stuck [Wander]: GOAL INVALID\n", __func__, self->client->pers.netname);
	}


	
	//if (next_node_type == INVALID && self->bot.goal_node == INVALID)
	if (next_node_type == INVALID) //self->bot.current_node == self->bot.next_node == self->bot.goal_node)
	{
		//next_node_type = NODE_MOVE;
		
		/*
		if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2) && M_CheckBottom(self, 2))
		{
			if (ACEMV_CanMove(self, MOVE_FORWARD))
			{
				ucmd->forwardmove = SPEED_WALK * sqrtf(xy_dist_self_curr / 64);
				return;
			}
			//else if (ACEMV_CanMove(self, MOVE_BACK))
			//	ucmd->forwardmove = -64;
			//else if (ACEMV_CanMove(self, MOVE_RIGHT))
			//	ucmd->sidemove = 64;
			//else if (ACEMV_CanMove(self, MOVE_LEFT))
			//	ucmd->sidemove = -64;
		}
		*/
	}
	
	
	// 
	//if (current_node_type == NODE_JUMPPAD && self->last_touched_ground > 0)
	//{
		//ucmd->forwardmove = 0;
		//VectorClear(self->velocity);
		//return;
	//}

	distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_CURR_NEXT);
	////VectorSubtract(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin, dist_xy);
	////dist_xy[2] = 0; // Drop height component
	////distance_xy = VectorLength(dist_xy);

	// Strafe jumping
	// Vary it up by doing JUMPPAD 'strafe jumping' in place of MOVE to the target if far enough away
	if (0)
	{
		if (next_node_type == NODE_MOVE && abs(nodes[self->bot.current_node].origin[2] - nodes[self->bot.next_node].origin[2]) < STEPSIZE && distance > 256)
		{
			if (self->strafe_jumps > 30) // Allow jumps to be missed
				self->strafe_jumps = 0; // Reset

			if (self->strafe_jumps < 10) // Mutiple jumps in series (although not contigous)
			{
				next_node_type = NODE_JUMPPAD;
			}

			self->strafe_jumps++; // Count strafe jumps
		}
	}

	//if (random() < 0.01)
	//{
		//self->client->leg_damage = 1; // DEBUG: force leg damage
		//Com_Printf("%s %s leg damage\n", __func__, self->client->pers.netname);
	//}

	// If we have leg damage, prevent jumping
	// Look for a nearby move node to take cover
	if (self->client->leg_damage > 0) // Do we have leg damage?
	{
		next_node_type = NODE_MOVE;

		/*
		if (next_node_type != NODE_MOVE)
		{
			qboolean foundMoveNode = false;

			// Search for a nearby move node
			for (i = 0; i < nodes[self->bot.current_node].num_links; i++)
			{
				if (nodes[self->bot.current_node].links[i].targetNodeType == NODE_MOVE)
				{
					// Check if any enemies can see this node
					qboolean canEnemiesSeeNextNode = false;
					for (int p = 0; p < num_players; p++) // Cycle based on how many players we find
					{
						if (ACEAI_IsEnemy(self, players[p])) // Are they our enemy
						{
							// trace line to see if enemy is visible from the targetNode
							int targetNode = nodes[self->bot.current_node].links[i].targetNode;
							tr = gi.trace(nodes[targetNode].origin, NULL, NULL, players[p]->s.origin, self, MASK_ALL);
							//if (tr.ent)
							//	Com_Printf("%s %s found ent [%s]\n", __func__, self->client->pers.netname, tr.ent->classname);
							if (tr.ent == players[p]) // Enemy can see this node
							{
								//Com_Printf("%s %s found MOVE node [%d] but enemy [%s] can see it\n", __func__, self->client->pers.netname, targetNode, players[p]->client->pers.netname);
								canEnemiesSeeNextNode = true;
								break;
							}
						}
					}

					if (canEnemiesSeeNextNode) // Enemy can see this node, so skip it
						continue;
					else // No enemies can see this node, so use it
					{
						self->bot.next_node = nodes[self->bot.current_node].links[i].targetNode;
						next_node_type = NODE_MOVE;
						foundMoveNode = true;
						Com_Printf("%s %s found MOVE node for cover [%d]\n", __func__, self->client->pers.netname, self->bot.next_node);
						break;
					}
				}
			}

			// Otherwise wander
			if (foundMoveNode == false)
			{
				Com_Printf("%s %s failed to find MOVE node, wandering...\n", __func__, self->client->pers.netname);
				self->bot.state = STATE_WANDER;
				self->wander_timeout = level.time + 5.0;
				return;
			}
		}
		*/
	}
	//return;


	//if ((current_node_type == NODE_LADDER || next_node_type == NODE_LADDER) && OnLadder(self) == false)
	//	next_node_type = NODE_JUMPPAD;
	//rekkie -- DEV_1 -- e

	///////////////////////
	// Grenade Avoidance //
	///////////////////////
	// Try to avoid hand grenades by strafing, backing away, and crouching
	edict_t* target = findradius(NULL, self->s.origin, 512);
	while (target)
	{
		if (target->classname == NULL)
			return;

		if (strcmp(target->classname, "hgrenade") == 0)
		{
			//Com_Printf("%s %s attempted to avoid a grenade \n", __func__, self->client->pers.netname);
			if (debug_mode) debug_printf("%s %s attempted to avoid a grenade \n", __func__, self->client->pers.netname);

			VectorSubtract(target->s.origin, self->s.origin, self->move_vector);
			ACEMV_ChangeBotAngle(self);

			// Try strafing
			if ((self->bot_strafe <= 0) && ACEMV_CanMove(self, MOVE_LEFT))
				ucmd->sidemove = -SPEED_RUN;
			else if (ACEMV_CanMove(self, MOVE_RIGHT))
				ucmd->sidemove = SPEED_RUN;
			else
				ucmd->sidemove = 0;

			self->bot_strafe = ucmd->sidemove;

			// Also try move backwards
			if (ACEMV_CanMove(self, MOVE_BACK))
				ucmd->forwardmove = -SPEED_RUN;

			// And crouch if its very close
			VectorSubtract(target->s.origin, self->s.origin, dist);
			dist[2] = 0;
			if (VectorLength(dist) < 192)
				ucmd->upmove = -SPEED_WALK;

			return;
		}

		target = findradius(target, self->s.origin, 512);
	}

	////////////////////
	// Stay on course //
	////////////////////
	/*
	if (self->bot.next_node != self->bot.current_node && current_node_type == NODE_MOVE && next_node_type == NODE_MOVE)
	{
		// Decide if the bot should strafe to stay on course.
		vec3_t v = { 0,0,0 };
		VectorSubtract(nodes[self->bot.next_node].origin, nodes[self->bot.current_node].origin, v);
		v[2] = 0;
		VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		dist[2] = 0;
		if ((DotProduct(v, dist) > 0) && (VectorLength(v) > 16))
		{
			float left = 0;
			VectorNormalize(v);
			VectorRotate2(v, 90);
			left = DotProduct(v, dist);
			if ((left > 16) && (!self->groundentity || ACEMV_CanMove(self, MOVE_RIGHT)))
				ucmd->sidemove = SPEED_RUN;
			else if ((left < -16) && (!self->groundentity || ACEMV_CanMove(self, MOVE_LEFT)))
				ucmd->sidemove = -SPEED_RUN;
		}
	}
	*/


	
	

	//////////////////////
	// Object Avoidance //
	//////////////////////
	
	// Calculate the players current speed and direction
	float forward_or_back_velocity = 0;
	float left_or_right_velocity = 0;
	if (1)
	{
		vec3_t velocity;
		VectorCopy(self->velocity, velocity);
		velocity[2] = 0;
		float speed = VectorLength(velocity);

		vec3_t angles;
		VectorCopy(self->client->v_angle, angles);
		angles[0] = 0;
		angles[2] = 0;

		vec3_t forward, right;
		AngleVectors(angles, forward, right, NULL);

		forward_or_back_velocity = DotProduct(forward, velocity);
		left_or_right_velocity = DotProduct(right, velocity);
		//Com_Printf("%s: speed %f forward_or_back_velocity %f left_or_right_velocity %f\n", __func__, speed, forward_or_back_velocity, left_or_right_velocity);
	}

	// Check to see if stuck, and if so try to free us. Also handles crouching
	VectorSubtract(self->s.origin, self->lastPosition, dist);
	if (self->groundentity && forward_or_back_velocity > -30 && forward_or_back_velocity < 30)
	{
		if (Botlib_Crouch_Or_Jump(self, ucmd))
		{
			Com_Printf("%s %s attempted to crouch or jump \n", __func__, self->client->pers.netname);
			return;
		}

		// Check if we are obstructed by an oncoming teammate, and if so strafe
		trace_t	tr;
		tr = gi.trace(self->s.origin, tv(-16, -16, -8), tv(16, 16, 8), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
		if ((tr.fraction < 1.0) && tr.ent && (tr.ent != self) && tr.ent->client && (DotProduct(self->velocity, tr.ent->velocity) <= 0))
		{
			if (ACEMV_CanMove(self, MOVE_RIGHT))
				ucmd->sidemove = SPEED_RUN;
			else if (ACEMV_CanMove(self, MOVE_LEFT))
				ucmd->sidemove = -SPEED_RUN;
			ACEMV_ChangeBotAngle(self);
			return;
		}
	}

	/*
	// Check to see if stuck, and if so try to free us. Also handles crouching
	VectorSubtract(self->s.origin, self->lastPosition, dist);
	if ((VectorLength(self->velocity) < 37) || (VectorLength(dist) < FRAMETIME))
	{
		if (self->groundentity)
		{
			//if (random() > 0.5 && 
			if (Botlib_Crouch_Or_Jump(self, ucmd))
				return;

			// Check if we are obstructed by an oncoming teammate, and if so strafe
			trace_t	tr;
			tr = gi.trace(self->s.origin, tv(-16, -16, -8), tv(16, 16, 8), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if ((tr.fraction < 1.0) && tr.ent && (tr.ent != self) && tr.ent->client && (DotProduct(self->velocity, tr.ent->velocity) <= 0))
			{
				if (ACEMV_CanMove(self, MOVE_RIGHT))
					ucmd->sidemove = SPEED_RUN;
				else if (ACEMV_CanMove(self, MOVE_LEFT))
					ucmd->sidemove = -SPEED_RUN;
				ACEMV_ChangeBotAngle(self);
				return;
			}
		}
	}
	*/
	/*
	///////////////////////
	// corner management //
	///////////////////////
	//RiEvEr -- My corner management code
	if (BOTCOL_CheckBump(self, ucmd))
	{
		if (BOTCOL_CanStrafeSafely(self, self->s.angles))
		{
			ucmd->sidemove = self->bot_strafe;
			return true;
		}
	}
	//R
	*/

	/*
	float botspeed = VectorLength(self->velocity);
	if (botspeed == 0 && self->last_touched_ground == 0)
	{
		Com_Printf("%s %s is stuck: STATE_WANDER\n", __func__, self->client->pers.netname);
		self->bot.state = STATE_WANDER;
		self->wander_timeout = level.framenum + 0.1 * HZ;
		return;
	}
	*/
	/*
	// Check if we're standing on top of another player
	// OR touching another player
	if (VectorLength(self->velocity) <= 32) // && self->last_touched_ground > 0)
	{
		//tr = gi.trace(self->s.origin, tv(-16,-16,-24), tv(16,16,32), tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 32), self, MASK_DEADSOLID);
		tr = gi.trace(self->s.origin, tv(-32, -32, -16), tv(32, 32, 24), tv(self->s.origin[0], self->s.origin[1], self->s.origin[2]), self, MASK_DEADSOLID);
		if (tr.ent && tr.ent->client && tr.ent->health > 0)
		{
			Com_Printf("%s %s is stuck on %s: wandering\n", __func__, self->client->pers.netname, tr.ent->client->pers.netname);
			self->bot.state = STATE_WANDER; // Aquire a new node
			return;
		}
		if (tr.ent && tr.ent->classname)
		{
			Com_Printf("%s %s is touching: %s\n", __func__, self->client->pers.netname, tr.ent->classname);
		}
	}
	*/


	// Grab items along the way, if they're not already our goal item to fetch
	if (self->movetarget && self->movetarget->inuse && self->movetarget != self->goalentity)
	{
		// Try to navigate to the item
		/*
		int item_node = ACEND_FindClosestReachableNode(self->goalentity, NODE_DENSITY, NODE_ALL);
		if (item_node != INVALID)
		{ 
			if (self->bot.prev_node != self->bot.goal_node)
				self->bot.prev_node = self->bot.goal_node; // Keep a copy of previous goal
			self->bot.goal_node = item_node; // Make the item node our new goal

			// Try visit node near item
			// Otherwise visit previous goal node
			{
				self->bot.goal_node = self->bot.prev_node; // Restore previous goal
				BOTLIB_CanVisitNode(self, self->bot.goal_node); // Try go back to previous goal
			}
		}
		*/
		
		// When we're near the item we desire, jump to it
		vec3_t item_vec;
		VectorSubtract(self->s.origin, self->movetarget->s.origin, item_vec);
		float dist = VectorLength(item_vec);
		if (dist < 256)
		{
			//if (self->movetarget->inuse == false) self->movetarget = NULL; //ignore freed ents

			if (dist < 32)
			{
				//Com_Printf("%s %s successfully got short range goal: %s\n", __func__, self->client->pers.netname, self->movetarget->classname);
				self->movetarget = NULL;
				//self->bot.goal_node = self->bot.prev_node; // Restore previous goal
				//BOTLIB_CanVisitNode(self, self->bot.goal_node); // Try go back to previous goal
				return;
			}
			else// if (ACEIT_IsReachable(self, self->movetarget->s.origin))
			{
				//Com_Printf("%s %s heading for short range goal: %s\n", __func__, self->client->pers.netname, self->movetarget->classname);

				VectorSubtract(self->movetarget->s.origin, self->s.origin, self->move_vector);
				ACEMV_ChangeBotAngle(self);
				//ucmd->forwardmove = SPEED_RUN;
				if (self->groundentity)
				{
					VectorClear(self->velocity);
					LaunchPlayer(self, self->movetarget->s.origin);
					self->movetarget = NULL;
					//self->bot.state = STATE_WANDER;
					return;
				}
				//return;
			}
		}
	}




	if (next_node_type == NODE_CROUCH)
	{
		// Make sure we're facing the target
		if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2) == false)
			return;

		// Crouch move forward
		ucmd->upmove = -SPEED_RUN;
		ucmd->forwardmove = SPEED_RUN;
		return;
	}

	////////////////////////////////////////////////////////
	// Jumpad Nodes
	///////////////////////////////////////////////////////
	if (next_node_type == NODE_JUMPPAD)
	{

		//if (self->groundentity && self->velocity[2] <= 0)
		//{
		//	VectorClear(self->velocity);
		//	LaunchPlayer(self, nodes[self->bot.next_node].origin);
		//	return;
		//}

		/*
		vec3_t dist;
		VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		float next_node_distance = VectorLength(dist);

		dist[2] = 0; // Drop height component
		float next_node_distance_xy = VectorLength(dist);

		vec3_t curr_node_dist;
		VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, curr_node_dist);
		float curr_node_distance = VectorLength(curr_node_dist);
		*/


		/*
		// If the bot's distance is moving away from the target (failed the jump)
		// Try to find another route, otherwise find a new node and go there instead
		if (self->prev_next_node == self->bot.next_node)
		{
			if (next_node_distance < self->next_node_distance) // Update if getting closer
				self->next_node_distance = next_node_distance;
			else if (next_node_distance > 64 && next_node_distance >= self->next_node_distance + NODE_Z_HALF_HEIGHT) // If getting further away we failed the jump
			{
				self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
				{
					Com_Printf("%s %s failed to jumppad up, trying alternative path to goal\n", __func__, self->client->pers.netname);
					BOTLIB_SetGoal(self, self->bot.goal_node);
				}
				else
				{
					Com_Printf("%s %s failed to jumppad up, finding new goal\n", __func__, self->client->pers.netname);
					self->bot.state = STATE_WANDER; // Aquire a new node
				}
				return;
			}
		}
		if (self->prev_next_node != self->bot.next_node) // Update previous next node and distance
		{
			self->prev_next_node = self->bot.next_node;
			self->next_node_distance = next_node_distance;
		}
		*/
		/*
		// If the bot fell below the target (failed the jump down)
		if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->bot.next_node].origin[2])
		{
			// Upgrade target node by moving its origin higher
			if (targetNodeLink != INVALID)
			{
				//if (debug_mode) 
					Com_Printf("%s %s dropped from a jumppad forward, moving jumppad node higher\n", __func__, self->client->pers.netname);
				int n = nodes[self->bot.current_node].links[targetNodeLink].targetNode;
				nodes[n].origin[2]++;
			}
		}
		*/





		/*
		// Bot fell below current and next target
		//if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->bot.current_node].origin[2])
		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR);
		if (self->last_touched_ground == 0 && distance > 96)
		{
			int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
			if (tmp_node != self->bot.current_node)
			{
				self->bot.prev_node = self->bot.current_node;
				self->bot.current_node = tmp_node;
				//Com_Printf("%s %s [NODE_JUMPPAD][1] prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
			}

			if (self->bot.current_node == INVALID)
			{
				if (debug_mode) 
					Com_Printf("%s %s failed to FindClosestReachableNode for node %d. Wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);
				self->bot.state = STATE_WANDER;
				self->wander_timeout = level.framenum + 0.1 * HZ;
				return;
			}
			{
				if (debug_mode) 
					Com_Printf("%s (1) %s failed to jumppad. Trying alternative path to goal %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
				//BOTLIB_SetGoal(self, self->bot.goal_node);
				return;
			}
		}
		*/





		// See if we need a ladder jump boost
		float speed = VectorLength(self->velocity);
		if (self->bot.current_node == NODE_LADDER_UP && speed < 32 && self->last_touched_ground > 0)
		{
			//if (debug_mode)
			//	Com_Printf("%s %s is stuck [NODE_JUMPPAD]...\n", __func__, self->client->pers.netname);

			// project forward from origin
			// Box test [feet to mid body] -> forward 16 units
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 32, forward, forward);
			tr = gi.trace(self->s.origin, tv(-8, -8, -14), tv(8, 8, 0), forward, self, MASK_DEADSOLID);
			if (tr.fraction < 1 || tr.startsolid)
			{
				ucmd->upmove = 400; // Try jumping
				if (self->s.origin[2] < nodes[self->bot.next_node].origin[2])
					self->velocity[2] = 200;
				//Com_Printf("%s %s is stuck [NODE_JUMPPAD]: jumping\n", __func__, self->client->pers.netname);
			}
			else
			{
				ucmd->upmove = -400; // Try crouching
				//Com_Printf("%s %s is stuck [NODE_JUMPPAD]: crouching\n", __func__, self->client->pers.netname);
			}
		}
		if (speed == 0 && self->last_touched_ground == 0) // We're not moving at all
		{
			//Com_Printf("%s %s is stuck [NODE_JUMPPAD]... speed: 0\n", __func__, self->client->pers.netname);
			ucmd->forwardmove = -50; // Try backing up a tad, then try jumping again
		}





		

		//*
		// Guide the bot in when close to the target
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		///distance = VectorLength(dist);
		vec3_t dist_xy;
		float distance_xy = BOTLIB_UTIL_Get_Distance(self, dist_xy, BOT_DIST_XY_SELF_NEXT);
		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
		/*
		if (0)
		if (distance <= 50 || distance_xy < 32 || VectorLength(self->velocity) < 32)// && next_node_type != NODE_LADDER_UP) //32
		{
			//self->s.angles[PITCH] = 0; // Look directly forward
			
			// When very close to the next node change yaw angle only
			//BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
			BOTLIB_MOV_ChangeBotAngleYawPitch(self);

			//Com_Printf("%s %s jumppad self->last_touched_ground = %d\n", __func__, self->client->pers.netname, self->last_touched_ground);
			
			//tr = gi.trace(self->s.origin, tv(-16, -16, 0), tv(16, 16, 0), nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
			tr = gi.trace(self->s.origin, tv(-8, -8, 0), tv(8, 8, 0), nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
			if (tr.fraction == 1.0)
			{
				float speed = VectorLength(self->velocity);
				//if (debug_mode) 
				//	Com_Printf("%s guiding the bot in from jump [speed %f]\n", __func__, speed);
				
				////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				distance = VectorLength(dist);	// Get the absolute length

				ucmd->forwardmove = 1; // Fake walk, just want the legs to move. Velocity will do the actual moving.

				// Apply it
				//if (!ACEMV_CanMove(self, MOVE_FORWARD))
				//	VectorScale(dist, (SPEED_WALK * sqrtf(distance / 512)), self->velocity); // Slow down
				//else
				//	VectorScale(dist, 128, self->velocity);
				//VectorScale(dist, 224, self->velocity);


				//vec3_t dist_xy;
				//float distance_xy = BOTLIB_UTIL_Get_Distance(self, dist_xy, BOT_DIST_XY_SELF_NEXT);
				//Com_Printf("%s %s distance_xy %f\n", __func__, self->client->pers.netname, distance_xy);
				if (distance_xy > 16)
				{
					if (self->groundentity || ACEMV_CanMove(self, MOVE_FORWARD) == false) // Slow down if on edge with drop
					{
						// touching the ground but on an edge with a drop - slow down
						if (ACEMV_CanMove(self, MOVE_FORWARD) == false)
						{
							//ucmd->forwardmove = -1;
							ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 256);
							VectorScale(dist, 128, self->velocity);
							Com_Printf("%s %s jumppad (1)\n", __func__, self->client->pers.netname);
						}
						else // touching the ground - safe to move at a normal pace
						{
							VectorScale(dist, SPEED_RUN, self->velocity);
							ucmd->forwardmove = SPEED_RUN;
							Com_Printf("%s %s jumppad (2)\n", __func__, self->client->pers.netname);
						}
					}
					else if (distance <= 16) // Ensure we slow right down if needed
					{
						//VectorScale(dist, max((SPEED_WALK * sqrtf(distance / 128)), 32), self->velocity); // Slow down (min speed 32)

						ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 256);
						VectorScale(dist, 128, self->velocity);
						Com_Printf("%s %s jumppad (3)\n", __func__, self->client->pers.netname);
					}
					else
					{
						//ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 256);
						//VectorScale(dist, 224, self->velocity);
						Com_Printf("%s %s jumppad (4)\n", __func__, self->client->pers.netname);
					}

					if (self->s.origin[2] > nodes[self->bot.next_node].origin[2])
						self->velocity[2] -= 32;

					// If we're now touching the ground, just move at a normal pace
					//if (self->last_touched_ground == 0)// && self->s.origin[2] == nodes[self->bot.next_node].origin[2])
					{
						//Com_Printf("%s %s jumppad (4)\n", __func__, self->client->pers.netname);
						//VectorScale(dist, SPEED_RUN, self->velocity);
						//ucmd->forwardmove = SPEED_RUN;
					}
						
				}
				else if (distance_xy <= 16 && self->s.origin[2] > nodes[self->bot.next_node].origin[2])
				{
					Com_Printf("%s %s jumppad (5a)\n", __func__, self->client->pers.netname);
					ucmd->forwardmove = 1;
					self->velocity[0] = 0;
					self->velocity[1] = 0;
					self->velocity[2] -= 32;
					//return;

					if (self->groundentity)
					{
						Com_Printf("%s %s jumppad (5b)\n", __func__, self->client->pers.netname);
						//ucmd->forwardmove = 128 * sqrtf(distance / 256);
						VectorScale(dist, 128, self->velocity);
					}
				}
				//else if (distance_xy <= 16 && self->s.origin[2] < nodes[self->bot.next_node].origin[2])
				//else if (distance_xy <= 16 && self->s.origin[2] < nodes[self->bot.next_node].origin[2] && !self->groundentity)
				else if (distance_xy <= 16 && (self->s.origin[2] + NODE_Z_HALF_HEIGHT) < nodes[self->bot.next_node].origin[2])
				{
					Com_Printf("%s %s jumppad (6)\n", __func__, self->client->pers.netname);

					//float z_height_diff = fabs(nodes[self->bot.next_node].origin[2]) - fabs(self->s.origin[2]);
					//if (z_height_diff < NODE_MAX_JUMP_HEIGHT)
					{
						// Try jumping
						//if (debug_mode)
						//	Com_Printf("%s %s failed to jumppad. Trying to jump next node %d\n", __func__, self->client->pers.netname, self->bot.next_node);
						//ucmd->upmove = SPEED_RUN;
						//ucmd->forwardmove = SPEED_RUN;
						//return;
						//if (Botlib_Crouch_Or_Jump(self, ucmd))
						//	return;
					}
					//return;

					int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
					if (tmp_node != self->bot.current_node)
					{
						self->bot.prev_node = self->bot.current_node;
						self->bot.current_node = tmp_node;
						//Com_Printf("%s %s [NODE_JUMPPAD][2] prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
					}

					if (self->bot.current_node == INVALID)
					{
						if (debug_mode) 
							Com_Printf("%s %s failed to FindClosestReachableNode for node %d. Wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);
						self->bot.state = STATE_WANDER;
						self->wander_timeout = level.framenum + 0.1 * HZ;
						return;
					}
					{
						if (debug_mode) 
							Com_Printf("%s (2) %s failed to jumppad. Trying alternative path to goal %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
						BOTLIB_SetGoal(self, self->bot.goal_node);
						return;
					}
				}
				else
				{
					//if (BOTLIB_MOV_ChangeBotAngleYawPitch(self))
					if (self->groundentity)
					{
						Com_Printf("%s %s jumppad (7)\n", __func__, self->client->pers.netname);
						//ucmd->forwardmove = 128 * sqrtf(distance / 256);
						VectorScale(dist, 128, self->velocity);
					}
				}


				// See if we're stuck, try to unstick
				if (speed < 32 && self->last_touched_ground > 0)
				{
					if (debug_mode) 
						Com_Printf("%s correcting jump, unsticking\n", __func__);

					// project forward from origin
					// Box test [feet to mid body] -> forward 16 units
					vec3_t forward;
					AngleVectors(self->s.angles, forward, NULL, NULL);
					VectorMA(self->s.origin, 32, forward, forward);
					tr = gi.trace(self->s.origin, tv(-8, -8, -14), tv(8, 8, 0), forward, self, MASK_DEADSOLID);
					if (tr.fraction < 1 || tr.startsolid)
					{
						ucmd->upmove = 400; // Try jumping
						if (self->s.origin[2] < nodes[self->bot.next_node].origin[2])
							self->velocity[2] = 200;
					}
					else
						ucmd->upmove = -400; // Try crouching
				}

				return;
			}
		}
		*/
		/*
		else if (next_node_distance <= 128) // || (next_node_distance < 96 && (nodes[self->bot.next_node].type == NODE_LADDER_UP || nodes[self->bot.next_node].type == NODE_LADDER_DOWN)))
		{
			tr = gi.trace(self->s.origin, tv(-16, -16, -6), tv(16, 16, 6), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if (tr.fraction > 0.9)
			{
				VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				next_node_distance = VectorLength(dist);	// Get the absolute length

				// Apply it
				if (next_node_distance > 48)
					VectorScale(dist, SPEED_RUN, self->velocity);
				else if (next_node_distance > 24)
					VectorScale(dist, 350, self->velocity);
				else
					VectorScale(dist, 300, self->velocity);
				
				return;
			}
		}
		*/


		float gravity = self->gravity * sv_gravity->value;

		ACEMV_ChangeBotAngle(self);

		float boost_jump = 1; // Boost jump in rare instances
		
		/*
		if (next_node_distance_xy > 64)
			ucmd->forwardmove = SPEED_RUN; // Move forward
		else if (self->groundentity && next_node_distance_xy < 64)
		{
			// Try alternative route
			BOTLIB_SetGoal(self, self->bot.goal_node);
			return;
		}
		*/
		/*
		else if (nodes[self->bot.next_node].origin[2] > self->s.origin[2] + 96) // It's fairly high above the player (and not a step)
		{
			// Check we've reach our ideal yaw angle
			if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2) == false)
				return;

			// Check we can move backward safely
			if (ACEMV_CanMove(self, MOVE_BACK))
			{
				ucmd->forwardmove = -SPEED_RUN; // Move back a bit
				return;
			}
			else
			{
				boost_jump = 2; // Could not move back, so cheat and boost jump
			}
		}
		*/

		//if (self->velocity[0] || self->velocity[1] || self->velocity[2])
		//{
		//	VectorClear(self->velocity); // Zero current velocity to kill any momentum
		//	return;
		//}
		
		//if (self->groundentity || next_node_distance < NODE_Z_HEIGHT)
		//int curr_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update the node we're near
		// distance between curr_node and self->bot.current_node

		//Com_Printf("%s %s can jumppad? last_touched_ground %d\n", __func__, self->client->pers.netname, self->last_touched_ground);

		// Slightly oversized player standing box ( +2 on sides, +8 bottom, but not top )
		//vec3_t  expanded_player_standing_mins = { -18, -18, -32 }; // default: -16, -16, -24
		//vec3_t  expanded_player_standing_maxs = { 18, 18, 32 }; // default: 16, 16, 32
		// Figure out if player is touching against a wall or floor
		//tr = gi.trace(self->s.origin, expanded_player_standing_mins, expanded_player_standing_maxs, self->s.origin, self, MASK_DEADSOLID);

		//if (self->groundentity || curr_node_distance <= 8 || (current_node_type == NODE_LADDER_UP && curr_node_distance < 24)) // && curr_node == self->bot.current_node))
		//if (self->groundentity || curr_node_distance < 24 || (current_node_type == NODE_LADDER_UP && curr_node_distance <= 24)) // && curr_node == self->bot.current_node))
		//if (self->groundentity || curr_node_distance <= 8 || (current_node_type == NODE_LADDER_UP && curr_node_distance <= 24)) // && curr_node == self->bot.current_node))
		//if (self->last_touched_ground == 0)
		if (self->last_jumppad_node != self->bot.current_node) // && self->velocity[2] > -200) // apply jumppad if we're on a different current_node
		//if (BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR) <= 32)
		//if (self->last_touched_ground < 10)
		//if (self->groundentity)
		//if (self->groundentity || tr.fraction < 1.0 || BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR) <= 48) // touching ground
		//if (self->groundentity || tr.fraction < 1.0) // touching ground
		//if (self->groundentity || BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR) <= 32) // touching ground
		//if (self->groundentity || (self->velocity[2] <= 0 && BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR) <= 32))
		//if ( (self->groundentity || tr.fraction < 1.0) && BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_CURR) <= 32)
		{
			// Skip doing a jumpnode... if the node we jumped from (current_node) changed. 
			// This occurs when a jump is made and fails to reach the next_node; because the current node is updated just before landing (in the wrong spot)
			//if (self->last_jumppad_node == self->bot.prev_node && self->last_touched_ground > 2)
			//{
				//Com_Printf("%s %s jumppad failed to reach target, self->last_touched_ground %d\n", __func__, self->client->pers.netname, self->last_touched_ground);
			//	self->last_jumppad_node = self->bot.current_node; // update node we last conducted a jumppad from
			//	return;
			//}

			// Slightly oversized player standing box ( +2 on sides, +2 bottom, but not top )
			//vec3_t  expanded_player_standing_mins = { -18, -18, -26 }; // default: -16, -16, -24
			//vec3_t  expanded_player_standing_maxs = { 18, 18, 32 }; // default: 16, 16, 32
			// Figure out if player is touching against a wall or floor
			//tr = gi.trace(self->s.origin, expanded_player_standing_mins, expanded_player_standing_maxs, self->s.origin, self, MASK_DEADSOLID);
			//if (tr.fraction == 1.0) // is the player in the air?
			//	return; // bug out

			//Com_Printf("%s %s applied jumppad %f\n", __func__, self->client->pers.netname, level.time);

			VectorClear(self->velocity); // Zero current velocity to kill any momentum
			self->last_jumppad_node = self->bot.current_node; // update node we last conducted a jumppad from

			// Check we've reach our ideal yaw angle and we're not falling
			//if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2) == false) // || self->velocity[2] < 0)
			//	return;

			if (BOTLIB_MOV_ChangeBotAngleYawPitch(self) == false)
			{
				//VectorClear(self->velocity); // Zero velocity
				ucmd->forwardmove = 1;
				//return;
			}
		

			// Check if we're off course for a jump
			vec3_t chk_dist;
			float chk_distance = BOTLIB_UTIL_Get_Distance(self, chk_dist, BOT_DIST_XYZ_SELF_CURR);
			if (chk_distance > 128)
			{
				//if (debug_mode) 
				//	Com_Printf("%s %s correcting jump that is off course\n", __func__, self->client->pers.netname);

				// Change the bot's vector to face the previous node
				VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, self->move_vector);

				// Face previous node
				if (BOTLIB_MOV_ChangeBotAngleYawPitch(self) == false)
				{
					//VectorClear(self->velocity); // Zero velocity
					ucmd->forwardmove = 1;
				}

				// Return the bot's movement vector back to the target
				VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, self->move_vector);
			}




			float z_velocity = self->velocity[2];
			if (z_velocity < 0)
				z_velocity = (z_velocity - (z_velocity * 2)); // Make it positive
			
			// Calculate the jump height to get to the target
			float jump_height = sqrt(2 * (gravity * distance));

			//Com_Printf("%s %s jump_height: %f\n", __func__, self->client->pers.netname, jump_height/16);

			// Calculate the time it will take to get to the target
			float time = distance / (z_velocity + jump_height / 2.0);

			// Calculate the velocity at the end of the jump
			VectorScale(dist, 1.0 / time, velocity);

			velocity[2] = jump_height / 2.0;

			// If the target is above the player, increase the velocity to get to the target
			float z_height;
			if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2])//self->s.origin[2])
			{
				z_height = ((nodes[self->bot.next_node].origin[2] - self->s.origin[2]) * boost_jump);
				z_height += sqrtf(distance) + NODE_Z_HEIGHT;
				velocity[2] += z_height;
			}
			else if (nodes[self->bot.next_node].origin[2] < nodes[self->bot.current_node].origin[2])
			{
				z_height = (self->s.origin[2] - nodes[self->bot.next_node].origin[2]) - NODE_Z_HEIGHT;
				velocity[2] -= z_height;
			}
			
			// If self->velocity[2] is minus, make it positive and add it to the required velocity	
			if (self->velocity[2] < 0 && self->groundentity == NULL)
				velocity[2] += -self->velocity[2];
			

			// Calculate speed from velocity
			//float speed = VectorLength(velocity);
			//gi.dprintf("speed[%f] z_height[%f] jump[%f]\n", speed, z_height, jump_height);

			// Limit max speed
			//if (speed < 750 && z_height < 155 && jump_height <= 965) // current_node_type == NODE_LADDER
			{
				// Set the velocity of the player to the calculated velocity
				VectorCopy(velocity, self->velocity);
				VectorCopy(velocity, self->client->oldvelocity);
				VectorCopy(velocity, self->avelocity);

				return;
			}
		}
		else
		{
			next_node_type = NODE_MOVE;
		}

		/*
		// Guide the bot in when close to the target
		else if (next_node_distance <= 128) // || (next_node_distance < 96 && (nodes[self->bot.next_node].type == NODE_LADDER_UP || nodes[self->bot.next_node].type == NODE_LADDER_DOWN)))
		{
			tr = gi.trace(self->s.origin, tv(-14, -14, -22), tv(14, 14, 2), nodes[self->next_node].origin, self, MASK_PLAYERSOLID);
			if (tr.fraction > 0.9)
			{
				VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				next_node_distance = VectorLength(dist);	// Get the absolute length

				// Apply it
				if (next_node_distance > 48)
					VectorScale(dist, SPEED_RUN, self->velocity);
				else if (next_node_distance > 24)
					VectorScale(dist, 350, self->velocity);
				else
					VectorScale(dist, 300, self->velocity);
			}
		}
		*/



		//return;
	}

	////////////////////////////////////////////////////////
	// Jump Node
	///////////////////////////////////////////////////////
	if (next_node_type == NODE_JUMP)
	{

		// If the bot fell below the target (failed the jump)
		// Try to find another route, otherwise find a new node and go there instead
		if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->bot.next_node].origin[2])
		{
			// Upgrade target node type to jumppad
			if (targetNodeLink != INVALID)
			{
				if (debug_mode)
					Com_Printf("%s %s fell from a jump node, upgrading node to jumppad\n", __func__, self->client->pers.netname);
				nodes[self->bot.current_node].links[targetNodeLink].targetNodeType = NODE_JUMPPAD;

				// Update the node we're near
				self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
				//int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
				//if (tmp_node != self->bot.current_node)
				//{
				//	self->bot.prev_node = self->bot.current_node;
				//	self->bot.current_node = tmp_node;
					//Com_Printf("%s %s [NODE_JUMP][1] prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
				//}

				return;
			}
			else
			{
				if (debug_mode)
					Com_Printf("%s %s failed to jump node to goal %d because targetNodeLink was invalid, wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);
				self->bot.state = STATE_WANDER; // Aquire a new node
				return;
			}
		}

		// See if we're stuck, try to unstick
		float speed = VectorLength(self->velocity);
		if (speed < 32 && self->last_touched_ground == 0)
		{
			//if (debug_mode)
			//	Com_Printf("%s %s is stuck, trying to resolve\n", __func__, self->client->pers.netname);

			// project forward from origin
			// Box test [feet to mid body] -> forward 16 units
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 32, forward, forward);
			tr = gi.trace(self->s.origin, tv(-8, -8, -14), tv(8, 8, 0), forward, self, MASK_DEADSOLID);
			if (tr.fraction < 1 || tr.startsolid)
			{
				ucmd->upmove = 400; // Try jumping
				if (self->s.origin[2] < nodes[self->bot.next_node].origin[2])
					self->velocity[2] = 200;
				//Com_Printf("%s %s is stuck [NODE_JUMP]: jumping\n", __func__, self->client->pers.netname);
			}
			else
			{
				ucmd->upmove = -400; // Try crouching
				//Com_Printf("%s %s is stuck [NODE_JUMP]: crouching\n", __func__, self->client->pers.netname);
			}
		}
		

		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		// Lose the vertical component
		////dist[2] = 0;
		// Get the absolute length
		////distance = VectorLength(dist);

		//if (ACEMV_CanJumpInternal(self, MOVE_FORWARD))
		{
			// Jump only when we are moving the correct direction.
			VectorCopy(self->velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);
			VectorNormalize(dist);
			if (DotProduct(velocity, dist) > 0.8)
				ucmd->upmove = SPEED_RUN;

			//Kill running movement
//			self->move_vector[0]=0;
//			self->move_vector[1]=0;
//			self->move_vector[2]=0;
			// Set up a jump move
			if (distance < 256)
				ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 256);
			else
				ucmd->forwardmove = SPEED_RUN;

			//self->move_vector[2] *= 2;

			ACEMV_ChangeBotAngle(self);

			//rekkie -- DEV_1 -- s
			// Guide the bot in when close to the target
			////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
			////distance = VectorLength(dist);
			distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
			if (distance <= 64)
			{
				tr = gi.trace(self->s.origin, tv(-14, -14, -22), tv(14, 14, 2), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
				if (tr.fraction > 0.7)
				{
					////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					////distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 200, self->velocity); // Apply it
				}
			}
			//rekkie -- DEV_1 -- e

			/*
			// RiEvEr - re-instate the velocity hack
			if (self->jumphack_timeout < level.framenum)
			{
				VectorCopy(self->move_vector,dist);
				if ((440 * distance / 128) < 440)
					VectorScale(dist,(440 * distance / 128),self->velocity);
				else
					VectorScale(dist,440,self->velocity);
				self->jumphack_timeout = level.framenum + 3.0 * HZ;
			}
			*/
		}
		/*
		else
		{
				self->bot.goal_node = INVALID;
		}
		*/
		return;
	}

	////////////////////////////////////////////////////////
	// Ladder Nodes
	///////////////////////////////////////////////////////
	//rekkie -- DEV_1 -- s
	if (current_node_type == NODE_LADDER_UP || current_node_type == NODE_LADDER_DOWN ||
		next_node_type == NODE_LADDER_UP || next_node_type == NODE_LADDER_DOWN)
	{
		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist); // Find the distance vector to the next node
		////distance = VectorLength(dist);

		////distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist_xy); // Find the distance vector to the next node
		////dist_xy[2] = 0; // Lose the vertical component
		////distance_xy = VectorLength(dist_xy); // Get the absolute length

		float speed = VectorLength(self->velocity); // Speed of the player

		// If we are going down the ladder, check for teammates coming up and yield to them.
		trace_t	tr;
		tr = gi.trace(self->s.origin, tv(-16, -16, -8), tv(16, 16, 8), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
		if ((tr.fraction < 1.0) && tr.ent && (tr.ent != self) && tr.ent->client && (tr.ent->velocity[2] >= 0))
		{
			self->velocity[0] = 0;
			self->velocity[1] = 0;
			self->velocity[2] = 200;
			self->bot.state = STATE_WANDER;
			return;
		}

		// If getting off the top of a ladder
		if (current_node_type == NODE_LADDER_UP && next_node_type != NODE_LADDER_DOWN)
		{
			ucmd->forwardmove = SPEED_WALK;

			//Com_Printf("%s %s on top ladder getting off, up vel %f\n", __func__, self->client->pers.netname, self->velocity[2]);

			//Com_Printf("%s %s on top ladder getting off. OnLadder %d\n", __func__, self->client->pers.netname, OnLadder(self));
			if (OnLadder(self))
			{
				ucmd->upmove = SPEED_RUN;
				return;
			}
			else
			{
				////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				////distance = VectorLength(dist);	// Get the absolute length
				VectorScale(dist, 400, self->velocity); // Apply it

				if (self->velocity[2] >= 0 && self->velocity[2] < 200) // do we need a boost?
				{
					float zdiff = 0;
					if (self->s.origin[2] < nodes[self->bot.next_node].origin[2])
						zdiff = abs(nodes[self->bot.next_node].origin[2] - self->s.origin[2]);
					float boost = 64 + zdiff;
					//float boost = 50;
					//Com_Printf("%s %s on top ladder getting off, boosting jump from %f to %f\n", __func__, self->client->pers.netname, self->velocity[2], self->velocity[2] + boost);
					self->velocity[2] += boost; // slight boost jump at top of ladder
				}

				if (distance < 64 && self->s.origin[2] >= nodes[self->bot.next_node].origin[2])
				{
					VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					VectorScale(dist, 50, self->velocity); // Apply it
					//Com_Printf("%s %s on top ladder getting off, guiding velocity\n", __func__, self->client->pers.netname);
				}
			}

			/*
			if ((nodes[self->bot.next_node].origin[2] >= self->s.origin[2])
				&& (nodes[self->bot.next_node].origin[2] >= nodes[self->bot.current_node].origin[2])
				&& (((self->velocity[2] > 0) && !self->groundentity) || OnLadder(self)))
			{
				ucmd->upmove = SPEED_RUN;
				self->velocity[2] = min(200, BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT) * 10); // Jump higher for farther node
			}

			if (1)
			{
				// Guide the bot in when close to the target
				if (self->s.origin[2] <= nodes[self->bot.next_node].origin[2] + 32 && distance < 128)
				{
					////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					////distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 400, self->velocity); // Apply it
				}
			}
			*/

			ACEMV_ChangeBotAngle(self);
			return;
		}


		// If getting off the bottom of a ladder
		if (current_node_type == NODE_LADDER_DOWN && next_node_type != NODE_LADDER_UP)
		{
			// We used to do nothing here, because it was handled by the next node type... but now we just walk forward :)

			BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
			ucmd->forwardmove = SPEED_RUN;
		}


		// Getting onto bottom of the ladder
		if (current_node_type != NODE_LADDER_UP && next_node_type == NODE_LADDER_DOWN && distance < 192 && nodes[self->bot.next_node].origin[2] > self->s.origin[2])
		{
			//Com_Printf("%s %s getting onto bottom of the ladder\n", __func__, self->client->pers.netname);

			// Jump only when we are moving the correct direction.
			// Check we've reach our ideal yaw angle
			//qboolean correct_angle = BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
			//qboolean isOnLadder = OnLadder(self);
			//Com_Printf("%s %s correct_angle %d, isOnLadder %d\n", __func__, self->client->pers.netname, correct_angle, isOnLadder);
			//if (isOnLadder == false)
			if (BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT) > 16)
			{
				//qboolean correct_angle = BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
				//Com_Printf("%s %s correct_angle %d, isOnLadder %d\n", __func__, self->client->pers.netname, correct_angle, isOnLadder);
				BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
				//ucmd->upmove = SPEED_WALK;
				ucmd->forwardmove = 96;

				//Com_Printf("%s %s moving to bottom of the ladder\n", __func__, self->client->pers.netname);
				return;
			}
			else
			{
				//Com_Printf("%s %s is at bottom of the ladder\n", __func__, self->client->pers.netname);
				//current_node_type = NODE_LADDER_DOWN; 
				//next_node_type = NODE_LADDER_UP;
			}

			/*
			if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2) && OnLadder(self) == false)
			//if (OnLadder(self) == false)
			{
				ucmd->upmove = SPEED_RUN;
				ucmd->forwardmove = SPEED_WALK;
			}
			*/


			if (1)
			{
				//Com_Printf("%s %s is at bottom of the ladder\n", __func__, self->client->pers.netname);
				//if (self->s.origin[2] < nodes[self->bot.next_node].origin[2] && distance < 32)
				//	self->velocity[2] = 100;
				//return;

				// Guide the bot in when close to the target
				//if (self->s.origin[2] < nodes[self->bot.next_node].origin[2] + 32 && distance < 128)
				if (self->s.origin[2] < nodes[self->bot.next_node].origin[2] && distance < 128)
				//if (self->s.origin[2] < nodes[self->bot.next_node].origin[2])
				{
					/*
					BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
					////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
					VectorNormalize(dist);
					////distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 32, self->velocity); // Apply it
					*/

					if (OnLadder(self) == false)
					{
						ucmd->forwardmove = 32;
						ucmd->upmove = 32;

						if (NODES_AdvanceToNextNode(self))
							return;
					}
						
					else
					{
						/*
						// Get distance
						VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
						VectorNormalize(dist);
						float next_node_distance = VectorLength(dist);	// Get the absolute length
						// Apply it
						if (next_node_distance > 48)
							VectorScale(dist, SPEED_RUN, self->velocity);
						else if (next_node_distance > 24)
							VectorScale(dist, 350, self->velocity);
						else
							VectorScale(dist, 300, self->velocity);
						*/
					}

					/*
					if (OnLadder(self) == false)
						ucmd->forwardmove = 64;
					else
					{
						// Make bot look directly up from its current pos
						vec3_t above;
						VectorCopy(self->s.origin, above);
						above[2] + 32;
						VectorSubtract(above, self->s.origin, self->move_vector); // point up
						BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2); // look up

						ucmd->upmove = SPEED_RUN;
						//self->s.origin[2] = nodes[self->bot.next_node].origin[2];
					}

					// If touching ladder, advance to next node (ladder down -> ladder up)
					if (OnLadder(self))
					{
						if (NODES_AdvanceToNextNode(self))
							return;
					}
					*/
					
					//Com_Printf("%s %s is at bottom of the ladder, OnLadder:%d\n", __func__, self->client->pers.netname, OnLadder(self));
				}
				return;
			}


			//if (OnLadder(self) == false)
			//return;
		}

		// Getting onto top of the ladder
		if (current_node_type != NODE_LADDER_DOWN && next_node_type == NODE_LADDER_UP)// && self->groundentity)
		{
			// Jump only when we are moving the correct direction.
			VectorCopy(self->velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);
			BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
			VectorNormalize(dist);
			float dot = DotProduct(velocity, dist);
			////VectorNormalize(dist_xy);
			////float dot = DotProduct(velocity, dist_xy);
			if (dot < 0.8 && dot != 0) // Make bot face ladder
			{
				ACEMV_ChangeBotAngle(self);
				//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
			}
			else // Correct direction, jump up
			{
				if (distance > 64)
					ucmd->forwardmove = SPEED_WALK;
				else if (distance > 48)
					ucmd->forwardmove = 50;
				else if (distance > 24)
					ucmd->forwardmove = 25;
			}

			// Guide the bot in when close to the target
			//VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
			//distance = VectorLength(dist);
			if (self->s.origin[2] <= nodes[self->bot.next_node].origin[2] && distance <= 128)
			{
				BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
				////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				////distance = VectorLength(dist);	// Get the absolute length
				VectorScale(dist, 100, self->velocity); // Apply it
			}

			//
			//if (OnLadder(self) == false)
			return;
		}

		// On ladder going up
		if (current_node_type == NODE_LADDER_DOWN && next_node_type == NODE_LADDER_UP)
		{
			//Com_Printf("%s %s on ladder going up\n", __func__, self->client->pers.netname);

			distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
			if (nodes[self->bot.next_node].origin[2] + NODE_Z_HEIGHT >= self->s.origin[2] && distance < 64)
			////if (nodes[self->bot.next_node].origin[2] + NODE_Z_HEIGHT >= self->s.origin[2] && distance_xy < 64)
			{
				VectorCopy(self->velocity, velocity);
				velocity[2] = 0;
				VectorNormalize(velocity);
				VectorNormalize(dist);
				float dot = DotProduct(velocity, dist);
				////VectorNormalize(dist_xy);
				////float dot = DotProduct(velocity, dist_xy);
				if (dot < 0.8 && dot != 0) // Make bot face ladder
				{
					ACEMV_ChangeBotAngle(self);
					//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
				}

				if (!self->groundentity)
				{
					// FIXME: Dirty hack so the bots can actually use ladders.
					//vec3_t origin_up;
					//VectorCopy(self->s.origin, origin_up);
					//origin_up[2] += 8;
					//VectorSubtract(origin_up, self->s.origin, dist_xy);
					
					////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist_xy);
					////VectorNormalize(dist_xy);
					////VectorScale(dist_xy, SPEED_RUN, self->velocity);
					////if (dist_xy[2] > 0)
					////	self->velocity[2] = min(200, self->velocity[2]);
					VectorScale(dist, SPEED_RUN, self->velocity);
					if (dist[2] > 0)
						self->velocity[2] = min(200, self->velocity[2]);
				}

				// Guide the bot in when close to the target
				// Useful for the tall ladder found on murder.bsp - helps to deal with ladders that use playerclip
				//if (self->s.origin[2] <= nodes[self->bot.next_node].origin[2] + 16 && distance < 64)
				if (self->s.origin[2] <= nodes[self->bot.next_node].origin[2] + 16)
				{
					BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
					VectorNormalize(dist);
					////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
					////VectorNormalize(dist);
					////distance = VectorLength(dist);	// Get the absolute length
					VectorScale(dist, 200, self->velocity); // Apply it
				}

				return;
			}
		}

		// On ladder going down
		if (current_node_type == NODE_LADDER_UP && next_node_type == NODE_LADDER_DOWN)
		{
			distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
			if (self->s.origin[2] >= nodes[self->bot.next_node].origin[2] && distance < 64) //&& speed <= SPEED_ROAM)
			////if (self->s.origin[2] >= nodes[self->bot.next_node].origin[2] && distance_xy < 64) //&& speed <= SPEED_ROAM)
			{
				ucmd->forwardmove = SPEED_WALK / 2;
				////if ((distance_xy < 200) && (nodes[self->bot.next_node].origin[2] <= self->s.origin[2] + 16))
				if ((distance < 200) && (nodes[self->bot.next_node].origin[2] <= self->s.origin[2] + 16))
					ucmd->upmove = -SPEED_RUN; //Added by Werewolf to cause crouching

				if (!self->groundentity)
				{
					VectorCopy(self->velocity, velocity);
					velocity[2] = 0;
					VectorNormalize(velocity);
					VectorNormalize(dist);
					float dot = DotProduct(velocity, dist);
					////VectorNormalize(dist_xy);
					////float dot = DotProduct(velocity, dist_xy);
					if (dot < 0.8 && dot != 0) // Make bot face ladder
					{
						//ACEMV_ChangeBotAngle(self);
						//Com_Printf("%s Correcting direction. DotProduct(velocity, dist_xy) = %f\n", __func__, dot);
					}

					// FIXME: Dirty hack so the bots can actually use ladders.
					////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist_xy);
					////VectorNormalize(dist_xy);
					////VectorScale(dist_xy, SPEED_RUN, self->velocity);
					////if (dist_xy[2] < 0)
					////	self->velocity[2] = max(-200, self->velocity[2]);
					BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
					VectorNormalize(dist);
					VectorScale(dist, SPEED_RUN, self->velocity);
					if (dist[2] < 0)
						self->velocity[2] = max(-200, self->velocity[2]);
				}
				return;
			}
		}
	}
	//rekkie -- DEV_1 -- e


	////////////////////////////////////////////////////////
	// Water Nodes
	///////////////////////////////////////////////////////
	if (current_node_type == NODE_WATER || self->waterlevel > 0)
	{
		ACEMV_ChangeBotAngle(self);
		//ucmd->upmove = SPEED_RUN;
		//ucmd->forwardmove = SPEED_RUN;

		// Ensure we have LoS
		tr = gi.trace(self->s.origin, tv(-16, -16, -4), tv(16, 16, 4), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
		if (tr.fraction == 1.0)
		
		// Guide the bot in when close to the target
		//if (next_node_type != NODE_WATER) // && distance <= 128)
		{
			BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
			////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
			////VectorNormalize(dist);
			////distance = VectorLength(dist);	// Get the absolute length
			if (next_node_type != NODE_WATER)
			{
				VectorScale(dist, 400, self->velocity); // Jump out
				ucmd->upmove = SPEED_RUN;
				ucmd->forwardmove = SPEED_RUN;
			}
			else
				VectorScale(dist, 200, self->velocity); // Move water speed
		}
		
		/*
		// We need to be pointed up/down
		ACEMV_ChangeBotAngle(self);

		// If the next node is not in the water, then move up to get out.
		if (next_node_type != NODE_WATER && !(gi.pointcontents(nodes[self->bot.next_node].origin) & MASK_WATER)) // Exit water
			ucmd->upmove = SPEED_RUN;

		ucmd->forwardmove = SPEED_RUN * 3 / 4;
		return;
		*/
	}

	//rekkie -- DEV_1 -- s
	////////////////////////////////////////////////////////
	// NODE_STAND_DROP Nodes
	if (next_node_type == NODE_STAND_DROP || next_node_type == NODE_CROUCH_DROP || next_node_type == NODE_UNSAFE_DROP)
	{
		if (0)
		{
			if (next_node_type == NODE_STAND_DROP) Com_Printf("%s %s next_node_type [NODE_STAND_DROP]... zvel %f\n", __func__, self->client->pers.netname, self->velocity[2]);
			if (next_node_type == NODE_CROUCH_DROP) Com_Printf("%s %s next_node_type [NODE_CROUCH_DROP]... zvel %f\n", __func__, self->client->pers.netname, self->velocity[2]);
			if (next_node_type == NODE_UNSAFE_DROP)	Com_Printf("%s %s next_node_type [NODE_UNSAFE_DROP]... zvel %f\n", __func__, self->client->pers.netname, self->velocity[2]);
		}

		// If target is getting too far away, adjust our angle 
		float distance_xy = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
		if (distance_xy > 32)
			ACEMV_ChangeBotAngle(self);
		
		// Then keep walking
		ucmd->forwardmove = SPEED_WALK;

		//Com_Printf("%s %s vel %f\n", __func__, self->client->pers.netname, VectorLength(self->velocity));

		// If stuck, try clearing the obstacle by moving in a random dir
		if (VectorLength(self->velocity) < 32)
		{
			//Com_Printf("%s %s is stuck [NODE_STAND_DROP]...\n", __func__, self->client->pers.netname);

			tr = gi.trace(self->s.origin, tv(-16, -16, -6), tv(16, 16, 32), nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
			if (tr.fraction < 1.0) // blocked, try find another path to the same goal
			{
				//Com_Printf("%s %s is blocked [NODE_STAND_DROP]...\n", __func__, self->client->pers.netname);

				//self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
				//BOTLIB_CanVisitNode(self, self->bot.goal_node);
				//ACEMV_ChangeBotAngle(self);

				self->s.angles[YAW] += 22.5 + (random() * 270);
				self->s.angles[PITCH] = 0;

				//self->bot.state = STATE_WANDER;
				return;
			}
		}

		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
		VectorNormalize(dist);
		//if (self->velocity[2] < -400 || distance < 64)
		//if (distance < 64)
		//VectorScale(dist, 350, self->velocity); // Apply it
		tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
		if (tr.fraction == 1.0)
		{
			VectorScale(dist, 350, self->velocity); // Apply it
			VectorCopy(self->velocity, self->client->oldvelocity);
			VectorCopy(self->velocity, self->avelocity);
		}

		return;

		/*
		// If the bot fell below the target (failed the jump down)
		// If the bot is trying an unsafe drop past the LCA timer
		// Try to find another route, otherwise find a new node and go there instead
		if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->bot.next_node].origin[2] ||
			(next_node_type == NODE_UNSAFE_DROP && lights_camera_action > 23))
		{
			// Update the node we're near
			int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
			if (tmp_node != self->bot.current_node)
			{
				self->bot.prev_node = self->bot.current_node;
				self->bot.current_node = tmp_node;
				//Com_Printf("%s %s prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
			}

			if (self->bot.current_node == INVALID)
			{
				self->bot.state = STATE_WANDER;
				self->wander_timeout = level.framenum + (random() + 0.5) * HZ;
				return;
			}
			{
				//Com_Printf("%s %s failed to drop down, trying alternative path to goal\n", __func__, self->client->pers.netname);
				BOTLIB_SetGoal(self, self->bot.goal_node);
			}
			else
			{
				//Com_Printf("%s %s failed to drop down, finding new goal\n", __func__, self->client->pers.netname);
				self->bot.state = STATE_WANDER; // Aquire a new node
			}
			return;
		}

		ACEMV_ChangeBotAngle(self);

		// If stuck, randomly crouch or jump
		if (VectorLength(self->velocity) < 3)
		{
			//Com_Printf("%s %s is stuck [NODE_STAND_DROP]...\n", __func__, self->client->pers.netname);
			if (random() > 0.5)
			{
				ucmd->upmove = -400;
				ucmd->forwardmove = SPEED_RUN;
				//if (debug_mode) 
				//Com_Printf("%s %s is stuck [NODE_STAND_DROP]: crouching\n", __func__, self->client->pers.netname);
				//return;
			}
			else
			{
				ucmd->upmove = 400;
				ucmd->forwardmove = SPEED_RUN;
				//if (debug_mode) 
				//Com_Printf("%s %s is stuck [NODE_STAND_DROP]: jumping\n", __func__, self->client->pers.netname);
				//return;
			}
		}



		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XY_SELF_NEXT);
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		////dist[2] = 0;
		////distance = VectorLength(dist);

		if (!self->groundentity)
		{
			// When in air, control speed carefully to avoid overshooting the next node.
			if (distance < 256)
			{
				ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 128); //256
				// Crouch down
				if (next_node_type == NODE_CROUCH_DROP || next_node_type == NODE_UNSAFE_DROP)
					ucmd->upmove = -SPEED_WALK;
			}
			else
				ucmd->forwardmove = SPEED_RUN;
		}
		else if (!ACEMV_CanMove(self, MOVE_FORWARD))
		{
			// If we reached a ledge, slow down.
			if (distance < 512)
			{
				// Jump unless dropping down to the next node.
				//if (nodes[self->bot.next_node].origin[2] > (self->s.origin[2] + 7) && distance < 500)
				//	ucmd->upmove = SPEED_RUN;

				// Crouch down
				if (next_node_type == NODE_CROUCH_DROP || next_node_type == NODE_UNSAFE_DROP)
					ucmd->upmove = -SPEED_WALK;

				ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 512);
			}
			else
				ucmd->forwardmove = SPEED_WALK;
		}
		else // Otherwise move as fast as we can.
			ucmd->forwardmove = SPEED_RUN;


		if(1)
		// Guide the bot in when close to the target
		//if (distance <= 128) //64
		if (self->velocity[2] < 0)
		{
			//if (self->velocity[2] < 32)
			//	ACEMV_ChangeBotAngle(self);

			//if (next_node_type == NODE_STAND_DROP) Com_Printf("%s %s: NODE_STAND_DROP\n", __func__, self->client->pers.netname);
			//if (next_node_type == NODE_CROUCH_DROP) Com_Printf("%s %s: NODE_CROUCH_DROP\n", __func__, self->client->pers.netname);
			//if (next_node_type == NODE_UNSAFE_DROP) Com_Printf("%s %s: NODE_UNSAFE_DROP\n", __func__, self->client->pers.netname);

			distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
			VectorNormalize(dist);
			////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
			////VectorNormalize(dist);
			////distance = VectorLength(dist);	// Get the absolute length

			if (self->groundentity && ACEMV_CanMove(self, MOVE_FORWARD) == false) // Slow down if on edge with drop
				ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 128);

			VectorScale(dist, 350, self->velocity); // Apply it
			
			return;
		}
		*/
	}
	//rekkie -- DEV_1 -- e


	////////////////////////////////////////////////////////
	// Move & Step Nodes
	///////////////////////////////////////////////////////
	if (current_node_type == NODE_MOVE || current_node_type == NODE_STEP || current_node_type == NODE_ITEM)
	{
		//Com_Printf("%s %s move forward\n", __func__, self->client->pers.netname);


		{
			// Get distance from bot to next node
			vec3_t bot_to_prev_vec, bot_to_curr_vec, bot_to_next_vec; // Direction vector to nodes
			float bot_to_prev_dist = 0, bot_to_curr_dist = 0, bot_to_next_dist = 0; // Distance to nodes
			qboolean can_see_prev = false, can_see_curr = false, can_see_next = false; // If bot can see these nodes
			if (self->bot.prev_node != INVALID)
			{
				VectorSubtract(nodes[self->bot.prev_node].origin, self->s.origin, bot_to_prev_vec);
				bot_to_prev_dist = VectorLength(bot_to_prev_vec);
				//trace_t prev_tr = gi.trace(self->s.origin, tv(-16, -16, -24), tv(16, 16, 32), nodes[self->bot.prev_node].origin, self, MASK_PLAYERSOLID);
				trace_t prev_tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.prev_node].origin, self, MASK_PLAYERSOLID);
				if (prev_tr.fraction == 1.0)
					can_see_prev = true;
			}
			VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, bot_to_curr_vec);
			bot_to_curr_dist = VectorLength(bot_to_curr_vec);
			//trace_t curr_tr = gi.trace(self->s.origin, tv(-16, -16, -24), tv(16, 16, 32), nodes[self->bot.current_node].origin, self, MASK_PLAYERSOLID);
			trace_t curr_tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if (curr_tr.fraction == 1.0)
				can_see_curr = true;

			VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_next_vec);
			bot_to_next_dist = VectorLength(bot_to_next_vec);
			//trace_t next_tr = gi.trace(self->s.origin, tv(-16, -16, -24), tv(16, 16, 32), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			trace_t next_tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if (next_tr.fraction == 1.0)
				can_see_next = true;

			if (BOTLIB_UTIL_ChangeBotAngleYaw(self, bot_to_next_vec, 2) == false) // && bot_to_next_dist < 128)
			{
				//Com_Printf("%s %s guiding bot: can_see_next %f\n", __func__, self->client->pers.netname, bot_to_next_dist);
				if (bot_to_next_dist < 192)
				{
					VectorClear(self->velocity);
					bot_to_next_vec[2] = 0;	// Flatten the vector
					VectorScale(bot_to_next_vec, 400, self->velocity);
				}
				else
					ucmd->forwardmove = SPEED_RUN;
			}
			else
				ucmd->forwardmove = SPEED_RUN;

			//qboolean right_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 128, 16); // Check right
			//qboolean left_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 128, -16); // Check left

			// Stay on course by strafing back to the path line (if not already strafing)
			if (1)
			{
				float mov_strafe = BOTLIB_UTIL_PATH_DEVIATION(self, ucmd);
				if (mov_strafe == 0)
				{
					//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
				}
				else if (mov_strafe > 0)
				{
					//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
					//ucmd->sidemove = SPEED_WALK;

					ucmd->forwardmove = SPEED_WALK;

					// Get distance from bot to self->nearest_path_point
					vec3_t bot_to_path_vec; // Direction vector to nodes
					VectorSubtract(self->nearest_path_point, self->s.origin, bot_to_path_vec);
					float bot_to_path_dist = VectorLength(bot_to_path_vec);
					//ucmd->sidemove = SPEED_WALK * sqrtf(bot_to_path_dist / 256);

					bot_to_path_vec[2] = 0;	// Flatten the vector
					VectorNormalize(bot_to_path_vec);
					VectorScale(bot_to_path_vec, 100, self->velocity); // Apply it
				}
				else if (mov_strafe < 0)
				{
					//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
					//ucmd->sidemove = -SPEED_WALK;

					ucmd->forwardmove = SPEED_WALK;

					// Get distance from bot to self->nearest_path_point
					vec3_t bot_to_path_vec; // Direction vector to nodes
					VectorSubtract(self->nearest_path_point, self->s.origin, bot_to_path_vec);
					float bot_to_path_dist = VectorLength(bot_to_path_vec);
					//ucmd->sidemove = -SPEED_WALK * sqrtf(bot_to_path_dist / 256);

					bot_to_path_vec[2] = 0;	// Flatten the vector
					VectorNormalize(bot_to_path_vec);
					VectorScale(bot_to_path_vec, 100, self->velocity); // Apply it
				}
			}

			/*
			// Check if bot can see the nodes
			if (can_see_next)
			{
				//Com_Printf("%s %s guiding bot: can_see_next %f\n", __func__, self->client->pers.netname, bot_to_next_dist);

				//bot_to_next_vec[2] = 0;	// Flatten the vector
				//VectorNormalize(bot_to_next_vec);
				//if (self->groundentity)
				//{
				//	ucmd->forwardmove = SPEED_RUN;
				//	VectorScale(bot_to_next_vec, 400, self->velocity);
				//}
				//else
				ucmd->forwardmove = SPEED_RUN;

				//if (bot_to_next_dist < 64)
				//BOTLIB_UTIL_ChangeBotAngleYaw(self, bot_to_next_vec, 2); // self->move_vector
				if (BOTLIB_UTIL_ChangeBotAngleYaw(self, bot_to_next_vec, 2) == false) // && bot_to_next_dist < 128)
				{
					Com_Printf("%s %s guiding bot: can_see_next %f\n", __func__, self->client->pers.netname, bot_to_next_dist);
					//ucmd->forwardmove = SPEED_RUN;
					bot_to_next_vec[2] = 0;	// Flatten the vector
					VectorScale(bot_to_next_vec, 400, self->velocity);
					//VectorClear(self->velocity);
				}

			}
			else if (can_see_curr)
			{
				Com_Printf("%s %s guiding bot: can_see_curr %f\n", __func__, self->client->pers.netname, bot_to_curr_dist);
				ucmd->forwardmove = SPEED_RUN;
				BOTLIB_UTIL_ChangeBotAngleYaw(self, bot_to_curr_vec, 2); // self->move_vector
			}
			else if (can_see_prev)
			{
				Com_Printf("%s %s guiding bot: can_see_prev %f\n", __func__, self->client->pers.netname, bot_to_prev_dist);
				ucmd->forwardmove = SPEED_RUN;
				BOTLIB_UTIL_ChangeBotAngleYaw(self, bot_to_prev_vec, 2); // self->move_vector
			}
			else
			{
				Com_Printf("%s %s guiding bot: Wander\n", __func__, self->client->pers.netname);
				// Wander or Reroute
			}
			*/

			self->s.angles[PITCH] = 0; // Look directly forward
			return;
		}


		{
			//ucmd->forwardmove = SPEED_RUN/2;
			//self->s.angles[PITCH] = 0; // Look directly forward
			//BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2); // Change the speed when close to node
			//return;
		}











		// Get distance from bot to next node
		vec3_t bot_to_next_vec;
		VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_next_vec);
		float bot_to_next_dist = VectorLength(bot_to_next_vec);
		//Com_Printf("%s: %s bot_to_next_dist %f\n", __func__, self->client->pers.netname, bot_to_next_dist);

		// Guide bot into the next_node when close
		if (bot_to_next_dist < 24)
		{
			tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if (tr.fraction == 1.0)
			{
				bot_to_next_vec[2] = 0;	// Flatten the vector
				VectorNormalize(bot_to_next_vec);
				VectorScale(bot_to_next_vec, 200, self->velocity);
				//Com_Printf("%s %s guiding bot from a move forward\n", __func__, self->client->pers.netname);
				return;
			}
		}


		// Change bot angle to face next node
		if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 11.25) == false)
		{
			VectorClear(self->velocity); // Kill velocity, to give bot time to turn
			return;
		}


		// Strafe if we're off course
		{
			// If not strafing to get back on path
			// Check if path forward is blocked
			qboolean did_strafe_right = false; // If the bot made a strafe move right
			qboolean did_strafe_left = false; // If the bot made a strafe move left
			//tr = gi.trace(self->s.origin, tv(-16, -16, 0), tv(16, 16, 0), nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
			//if (mov_strafe == 0 && tr.fraction < 1.0)
			if (bot_to_next_dist > 64)
			{
				qboolean right_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 128, 16); // Check right
				qboolean left_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 128, -16); // Check left

				qboolean mid_partial_blocked = false;
				qboolean mid_full_blocked = false;

				// Line trace
				tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
				if (tr.fraction < 1.0 || tr.startsolid)
					mid_partial_blocked = true;

				// Box trace -- full width of the bot
				tr = gi.trace(self->s.origin, tv(-16, -16, 0), tv(16, 16, 0), nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
				if (tr.fraction < 1.0 || tr.startsolid)
				{
					mid_full_blocked = true;
				}

				// All [BLOCKED]
				if (mid_partial_blocked && right_blocked && left_blocked)
				{
					BOTLIB_UTIL_NEAREST_PATH_POINT(self, ucmd); // Update the nearest path point

					// Bot -> nearest path point
					tr = gi.trace(self->s.origin, NULL, NULL, self->nearest_path_point, self, MASK_DEADSOLID);
					if (tr.fraction == 1.0 && tr.startsolid == false)
					{
						//Com_Printf("%s %s --> heading for nearest path point\n", __func__, self->client->pers.netname);

						// Move vector between bot and self->nearest_path_point
						VectorSubtract(self->nearest_path_point, self->s.origin, self->move_vector);
						
						self->move_vector[2] = 0;	// Flatten the vector
						VectorNormalize(self->move_vector);
						VectorScale(self->move_vector, 400, self->velocity);
						return;
					}

					// Bot -> current node point
					tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.current_node].origin, self, MASK_DEADSOLID);
					if (tr.fraction == 1.0 && tr.startsolid == false)
					{
						Com_Printf("%s %s --> heading for current node point\n", __func__, self->client->pers.netname);

						// Move vector between bot and nodes[self->bot.current_node].origin
						VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, self->move_vector);

						self->move_vector[2] = 0;	// Flatten the vector
						VectorNormalize(self->move_vector);
						VectorScale(self->move_vector, 400, self->velocity);
						return;
					}

					// Bot -> re-route
					// Update the node we're near
					self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
					//int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
					//if (tmp_node != self->bot.current_node)
					//{
					//	self->bot.prev_node = self->bot.current_node;
					//	self->bot.current_node = tmp_node;
					//	Com_Printf("%s %s [MOVE_NODE][1] prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
					//}
					if (self->bot.current_node == INVALID)
					{
						Com_Printf("%s %s failed to FindClosestReachableNode for goal %d. Wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);
						self->bot.state = STATE_WANDER;
						//self->wander_timeout = level.framenum + 0.1 * HZ;
						return;
					}
					{
						Com_Printf("%s %s failed to move. Trying alternative path to goal %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
						return;
					}

					return;
				}

				// mid [BLOCKED] && right [CLEAR] && left [CLEAR]
				if (mid_partial_blocked && !right_blocked && !left_blocked)
				{
					//Com_Printf("%s: %s partial middle blocked\n", __func__, self->client->pers.netname);

					if (ucmd->sidemove > 0) // Bot was strafing right, so continue
					{
						//Com_Printf("%s %s --> continue strafe right\n", __func__, self->client->pers.netname);
						ucmd->sidemove = SPEED_WALK;
						did_strafe_right = true;
					}
					else if (ucmd->sidemove < 0)  // Bot was strafing left, so continue
					{
						//Com_Printf("%s %s --> continue strafe left\n", __func__, self->client->pers.netname);
						ucmd->sidemove = -SPEED_WALK;
						did_strafe_left = true;
					}
					else // Bot was not strafing, so pick a random direction
					{
						if (rand() % 2 == 0)
						{
							//Com_Printf("%s %s --> RNG strafe right\n", __func__, self->client->pers.netname);
							ucmd->sidemove = SPEED_WALK;
							did_strafe_right = true;
						}
						else
						{
							//Com_Printf("%s %s --> RNG strafe left\n", __func__, self->client->pers.netname);
							ucmd->sidemove = -SPEED_WALK;
							did_strafe_left = true;
						}
					}
				}
				
				// When bot encounters a thin pole, prevent it from getting stuck
				if (mid_full_blocked && forward_or_back_velocity == 0)
				{
					//Com_Printf("%s: %s full middle blocked\n", __func__, self->client->pers.netname);

					if (ucmd->sidemove > 0) // Bot was strafing right, so continue
					{
						//Com_Printf("%s %s --> continue strafe right\n", __func__, self->client->pers.netname);
						ucmd->sidemove = SPEED_WALK;
						did_strafe_right = true;
					}
					else if (ucmd->sidemove < 0)  // Bot was strafing left, so continue
					{
						//Com_Printf("%s %s --> continue strafe left\n", __func__, self->client->pers.netname);
						ucmd->sidemove = -SPEED_WALK;
						did_strafe_left = true;
					}
					else // Bot was not strafing, so pick a random direction
					{
						if (rand() % 2 == 0)
						{
							//Com_Printf("%s %s --> RNG strafe right\n", __func__, self->client->pers.netname);
							ucmd->sidemove = SPEED_WALK;
							did_strafe_right = true;
						}
						else
						{
							//Com_Printf("%s %s --> RNG strafe left\n", __func__, self->client->pers.netname);
							ucmd->sidemove = -SPEED_WALK;
							did_strafe_left = true;
						}
					}
				}
				
				// NARROW strafe search -- RIGHT or LEFT [BLOCKED]
				if (right_blocked)
				{
					//Com_Printf("%s: %s right blocked\n", __func__, self->client->pers.netname);
					if (ACEMV_CanMove(self, MOVE_LEFT))
					{
						//Com_Printf("%s %s --> small strafe left\n", __func__, self->client->pers.netname);
						ucmd->sidemove = -SPEED_WALK;
						did_strafe_left = true;
					}
				}
				else if (left_blocked)
				{
					//Com_Printf("%s: %s left blocked\n", __func__, self->client->pers.netname);
					if (ACEMV_CanMove(self, MOVE_RIGHT))
					{
						//Com_Printf("%s %s --> small strafe right\n", __func__, self->client->pers.netname);
						ucmd->sidemove = SPEED_WALK;
						did_strafe_right = true;
					}
				}

				// WIDE strafe search
				if (!mid_partial_blocked && left_blocked && right_blocked)
				{
					//Com_Printf("%s: %s small left and right blocked\n", __func__, self->client->pers.netname);

					// Try a wider search
					right_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 48, 32); // Check right
					left_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 48, -32); // Check left

					if (right_blocked)
					{
						//Com_Printf("%s: %s right blocked\n", __func__, self->client->pers.netname);
						//if (ACEMV_CanMove(self, MOVE_LEFT))
						{
							//Com_Printf("%s %s --> medium strafe left\n", __func__, self->client->pers.netname);
							ucmd->sidemove = -SPEED_WALK;
							did_strafe_left = true;
						}
					}

					if (left_blocked)
					{
						//Com_Printf("%s: %s left blocked\n", __func__, self->client->pers.netname);
						//if (ACEMV_CanMove(self, MOVE_RIGHT))
						{
							//Com_Printf("%s %s --> medium strafe right\n", __func__, self->client->pers.netname);
							ucmd->sidemove = SPEED_WALK;
							did_strafe_right = true;
						}
					}

					if (left_blocked && right_blocked)
					{
						//Com_Printf("%s: %s medium left and right blocked\n", __func__, self->client->pers.netname);

						// Try the widest search
						right_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 48, 48); // Check right
						left_blocked = BOTLIB_UTIL_PATH_FORWARD(self, ucmd, 48, -48); // Check left

						if (right_blocked)
						{
							//Com_Printf("%s: %s right blocked\n", __func__, self->client->pers.netname);
							//if (ACEMV_CanMove(self, MOVE_LEFT))
							{
								//Com_Printf("%s %s --> large strafe left\n", __func__, self->client->pers.netname);
								ucmd->sidemove = -SPEED_WALK;
								did_strafe_left = true;
							}
						}

						if (left_blocked)
						{
							//Com_Printf("%s: %s left blocked\n", __func__, self->client->pers.netname);
							//if (ACEMV_CanMove(self, MOVE_RIGHT))
							{
								//Com_Printf("%s %s --> large strafe right\n", __func__, self->client->pers.netname);
								ucmd->sidemove = SPEED_WALK;
								did_strafe_right = true;
							}
						}
					}
				}

				// Stay on course by strafing back to the path line (if not already strafing)
				if (did_strafe_right == false && did_strafe_left == false && mid_partial_blocked == false)
				{
					float mov_strafe = BOTLIB_UTIL_PATH_DEVIATION(self, ucmd);
					if (mov_strafe == 0)
					{
						//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
						//ucmd->sidemove = 0;
					}
					else if (mov_strafe > 0)
					{
						//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
						ucmd->sidemove = SPEED_WALK;
					}
					else if (mov_strafe < 0)
					{
						//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
						ucmd->sidemove = -SPEED_WALK;
					}
				}
			}

			ucmd->forwardmove = SPEED_RUN;
			return;
		}








		//distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR);
		// If stuck, try clearing the obstacle by moving in a random dir
		if (VectorLength(self->velocity) <= 8)
		{
			//if (BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_CURR) > 32)
			//Com_Printf("%s %s is stuck [NODE_MOVE]...\n", __func__, self->client->pers.netname);

			tr = gi.trace(self->s.origin, tv(-16, -16, -6), tv(16, 16, 32), nodes[self->bot.next_node].origin, self, MASK_DEADSOLID);
			if (tr.fraction < 1.0) // blocked, try find another path to the same goal
			{
				//Com_Printf("%s %s is blocked [NODE_MOVE]...\n", __func__, self->client->pers.netname);

				// Try strafing
				if (random() > 0.5)
				{
					VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, self->move_vector);
					ACEMV_ChangeBotAngle(self);

					// Try strafing
					if ((self->bot_strafe <= 0) && ACEMV_CanMove(self, MOVE_LEFT))
						ucmd->sidemove = -SPEED_RUN;
					else if (ACEMV_CanMove(self, MOVE_RIGHT))
						ucmd->sidemove = SPEED_RUN;
					else
						ucmd->sidemove = 0;

					self->bot_strafe = ucmd->sidemove;

					if (ACEMV_CanMove(self, MOVE_FORWARD))
						ucmd->forwardmove = SPEED_RUN;
					// Also try move backwards
					else if (ACEMV_CanMove(self, MOVE_BACK))
						ucmd->forwardmove = -SPEED_RUN;

					return;
				}

				/*
				if (random() > 0.25)
				{
					//if (ACEMV_CanMove(self, MOVE_BACK))
					//	ucmd->forwardmove = -SPEED_RUN;
					if (ACEMV_CanMove(self, MOVE_LEFT))
						ucmd->sidemove = -SPEED_RUN;
					else if (ACEMV_CanMove(self, MOVE_RIGHT))
						ucmd->sidemove = SPEED_RUN;

					self->s.angles[YAW] += 22.5 + (random() * 270);
					ucmd->forwardmove = SPEED_RUN;
					ucmd->upmove = SPEED_RUN;
					return;
				}
				*/


				//self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
				//BOTLIB_CanVisitNode(self, self->bot.goal_node);
				//VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, self->move_vector);
				//ACEMV_ChangeBotAngle(self);

				//VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, self->move_vector);
				//BOTLIB_MOV_ChangeBotAngleYawPitch(self);
				/////////////////////////////////// LaunchPlayer(self, nodes[self->bot.current_node].origin); ////////////////////////////
				//VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, dist);
				//VectorNormalize(dist);
				//VectorScale(dist, 400, self->velocity); // Apply it
				//self->velocity[2] = 50;

				/*
				//if (ACEMV_CanMove(self, MOVE_BACK))
				//	ucmd->forwardmove = -SPEED_RUN;
				if (ACEMV_CanMove(self, MOVE_LEFT))
					ucmd->sidemove = -SPEED_RUN;
				else if (ACEMV_CanMove(self, MOVE_RIGHT))
					ucmd->sidemove = SPEED_RUN;

				//self->s.angles[YAW] += 22.5 + (random() * 270);
				self->s.angles[YAW] += 180;
				ucmd->forwardmove = SPEED_RUN;
				ucmd->upmove = SPEED_RUN;
				self->s.angles[PITCH] = 0;
				*/

				// Make the bot look at the current node
				//self->bot.next_node = self->bot.current_node;
				//VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, self->move_vector);
				//BOTLIB_MOV_ChangeBotAngleYawPitch(self);
				//ucmd->forwardmove = SPEED_RUN;
				


				//self->bot.state = STATE_WANDER;
				return;
			}
		}

		// If we cannot see the next node
		tr = gi.trace(self->s.origin, tv(-8, -8, 0), tv(8, 8, 0), nodes[self->bot.current_node].origin, self, MASK_DEADSOLID);
		if (tr.fraction < 1)
		{
			// Update the node we're near
			self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
			//int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
			//if (tmp_node != self->bot.current_node)
			//{
			//	self->bot.prev_node = self->bot.current_node;
			//	self->bot.current_node = tmp_node;
			//	//Com_Printf("%s %s [MOVE_NODE][1] prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
			//}

			if (self->bot.current_node == INVALID)
			{
				if (debug_mode) 
					Com_Printf("%s %s failed to FindClosestReachableNode for goal %d. Wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);
				self->bot.state = STATE_WANDER;
				//self->wander_timeout = level.framenum + 0.1 * HZ;
				return;
			}
			if ((self, self->bot.goal_node)) // Try to find another way to our goal
			{
				if (debug_mode) 
					Com_Printf("%s %s failed to move. Trying alternative path to goal %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
				//BOTLIB_SetGoal(self, self->bot.goal_node);
				return;
			}
		}

		// Run up stairs
		if (current_node_type == NODE_STEP && next_node_type == NODE_STEP)
		{
			ucmd->forwardmove = SPEED_RUN;
			self->s.angles[PITCH] = 0; // Look directly forward
			BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
			//Com_Printf("%s %s Step --> Step\n", __func__, self->client->pers.netname);
			return;
		}

		/*
		// If stuck, crouch or jump
		if (VectorLength(self->velocity) < 3)
		{
			Com_Printf("%s %s is stuck...\n", __func__, self->client->pers.netname);

			trace_t tr_lower_body;
			trace_t tr_upper_body;
			vec3_t fwd;
			AngleVectors(self->s.angles, fwd, NULL, NULL); // project forward from origin
			VectorMA(self->s.origin, 16, fwd, fwd);
			tr_lower_body = gi.trace(self->s.origin, tv(-5, -5, -14), tv(5, 5, 0), fwd, self, MASK_DEADSOLID); // Box test [feet to mid body] -> forward 16 units
			tr_upper_body = gi.trace(self->s.origin, tv(-5, -5, 0), tv(5, 5, 32), fwd, self, MASK_DEADSOLID); // Box test [mid body to head] -> forward 16 units
			// Need to crouch?
			// Lower body is free, upper body is blocked
			if (tr_lower_body.fraction == 1.0 && tr_upper_body.fraction < 1.0)
			{
				BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
				ucmd->upmove = -400;
				ucmd->forwardmove = SPEED_RUN;
				//if (debug_mode) 
				Com_Printf("%s %s is stuck [NODE_MOVE]: crouching\n", __func__, self->client->pers.netname);
				return;
			}
			// Need to jump?
			// Lower body is blocked, upper body is free
			else if (tr_lower_body.fraction < 1.0 && tr_upper_body.fraction == 1.0)
			{
				BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);
				ucmd->upmove = 400;
				ucmd->forwardmove = SPEED_RUN;
				//if (debug_mode) 
				Com_Printf("%s %s is stuck [NODE_MOVE]: jumping\n", __func__, self->client->pers.netname);
				return;
			}
			else
			{
				//self->bot.state = STATE_WANDER;
				//return;
			}
		}
		*/

		
		// If the bot fell below the target (failed the jump down)
		// Try to find another route, otherwise find a new node and go there instead
		if (self->s.origin[2] + NODE_Z_HEIGHT < nodes[self->bot.next_node].origin[2])
		{
			// Upgrade target node type to jumppad
			if (targetNodeLink != INVALID)
			{
				//if (debug_mode) 
				//Com_Printf("%s %s dropped from a move forward, upgrading move to jumppad node\n", __func__, self->client->pers.netname);

				nodes[self->bot.current_node].links[targetNodeLink].targetNodeType = NODE_JUMPPAD;

				// Update the node we're near
				self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
				//int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
				//if (tmp_node != self->bot.current_node)
				//{
				//	self->bot.prev_node = self->bot.current_node;
				//	self->bot.current_node = tmp_node;
				//	//Com_Printf("%s %s [MOVE_NODE][2] prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
				//}

				return;
			}
			else
			{
				{
					//if (debug_mode)
					//Com_Printf("%s %s failed to move. Trying alternative path to goal %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
					BOTLIB_SetGoal(self, self->bot.goal_node);
					return;
				}

				//if (debug_mode) 
				//Com_Printf("%s %s failed to move forward to goal %d, wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);

				//self->bot.state = STATE_WANDER; // Aquire a new node
				//return;
			}
		}

		self->s.angles[PITCH] = 0; // Look directly forward

		//BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2);

		// Check we've reach our ideal yaw angle
		if (BOTLIB_UTIL_ChangeBotAngleYaw(self, self->move_vector, 2) == false)
			return;

		/*
		// See if we're stuck, try to unstick
		if (VectorLength(self->velocity) < 8)
		{
			// project forward from origin
			// Box test [feet to mid body] -> forward 16 units
			vec3_t forward;
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorMA(self->s.origin, 32, forward, forward);
			tr = gi.trace(self->s.origin, tv(-8, -8, -14), tv(8, 8, 0), forward, self, MASK_DEADSOLID);
			if (tr.fraction < 1 || tr.startsolid)
			{
				ucmd->upmove = 400; // Try jumping
			}
			else
				ucmd->upmove = -400; // Try crouching

			// See if the way forward is clear
			tr = gi.trace(self->s.origin, tv(-16, -16, 0), tv(16, 16, 0), forward, self, MASK_DEADSOLID);
			if (tr.fraction < 1)
			{
				// Try strafing left
				tr = gi.trace(self->s.origin, tv(-32, -16, -14), tv(0, 16, 0), forward, self, MASK_DEADSOLID);
				if (tr.fraction == 1)
				{
					ucmd->sidemove = -400;
				}
				else
				{
					// Try strafing right
					tr = gi.trace(self->s.origin, tv(0, -16, -14), tv(32, 16, 0), forward, self, MASK_DEADSOLID);
					if (tr.fraction == 1)
					{
						ucmd->sidemove = 400;
					}
				}
			}
		}
		*/

		// Guide the bot in when close to the target
		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		////distance = VectorLength(dist);
		if (distance <= 32)
		{
			//tr = gi.trace(self->s.origin, tv(-14, -14, -22), tv(14, 14, 2), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			tr = gi.trace(self->s.origin, tv(-14, -14, -6), tv(14, 14, 6), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if (tr.fraction == 1.0)
			{
				////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
				VectorNormalize(dist);
				distance = VectorLength(dist);	// Get the absolute length
				
				ucmd->forwardmove = 1; // Fake walk, just want the legs to move. Velocity will do the actual moving.

				// Apply it
				//if (!ACEMV_CanMove(self, MOVE_FORWARD))
				//	VectorScale(dist, (SPEED_WALK * sqrtf(distance / 512)), self->velocity); // Slow down
				//else
				//	VectorScale(dist, 128, self->velocity);
				//VectorScale(dist, 224, self->velocity);

				if (distance <= 2)
					VectorScale(dist, 128, self->velocity);
				//else if (distance <= 16)
				//	VectorScale(dist, 224, self->velocity);
				else
					VectorScale(dist, 320, self->velocity);

				//Com_Printf("%s %s guiding bot from a move forward\n", __func__, self->client->pers.netname);

				return;
			}
		}

		/*
		if (next_node_type == INVALID)
		{
			if (!ACEMV_CanMove(self, MOVE_FORWARD))
			{
				//ACEAI_PickLongRangeGoal(self);
				return;
			}
		}
		*/

		//ACEMV_ChangeBotAngle(self);

		/*
		distance = BOTLIB_UTIL_Get_Distance(self, dist, BOT_DIST_XYZ_SELF_NEXT);
		////VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, dist);
		////dist[2] = 0;
		////distance = VectorLength(dist);
		if (!self->groundentity)
		{
			Com_Printf("%s %s [MOVE_NODE] air guide...\n", __func__, self->client->pers.netname);

			// When in air, control speed carefully to avoid overshooting the next node.
			if (distance < 256)
				ucmd->forwardmove = SPEED_RUN * sqrtf(distance / 256);
			else
				ucmd->forwardmove = SPEED_RUN;
		}

		//rekkie -- DEV_1 -- s
		else if (!ACEMV_CanMove(self, MOVE_FORWARD))
		{
			Com_Printf("%s %s [MOVE_NODE] move guide...\n", __func__, self->client->pers.netname);

			if (distance < 128)
				ucmd->forwardmove = SPEED_WALK * sqrtf(distance / 128);
			else
				ucmd->forwardmove = SPEED_WALK;
		}
		else
		{
			// Otherwise move as fast as we can.
			ucmd->forwardmove = SPEED_RUN;
		}
		*/

		ucmd->forwardmove = SPEED_RUN;
		//rekkie -- DEV_1 -- e

	}
#endif
}

qboolean BOTLIB_DoParabolaJump(edict_t* self, vec3_t targetpoint)
{
	/*
	vec3_t forward;

	// immediately turn to where we need to go
	float length = VectorDistance(self->s.origin, targetpoint);
	float fwd_speed = length * 1.95f;

	vec3_t dir;
	PredictAim(self, self->enemy, self->s.origin, fwd_speed, false, 0.f, &dir, nullptr);

	self->s.angles[1] = vectoyaw(dir);
	AngleVectors(self->s.angles, forward, nullptr, nullptr);
	self->s.origin[2] += 1;
	self->velocity = forward * fwd_speed;
	self->velocity[2] = 450;
	*/



	float fordward_speed;
	vec3_t forward, dir, vec, target;

	VectorCopy(targetpoint, target);

	// Get height diff from self to target point
	// If > 0, target is higher than self
	// If < 0, target is lower than self
	//float height_diff = targetpoint[2] - (self->s.origin[2] - self->viewheight);  // 56 - 22 = 34
	float height_diff = target[2] - self->s.origin[2];  // 56 - 22 = 34

	//if (height_diff < 0)
	target[2] += 32;
	//else
	//	targetpoint[2] -= 32;

	fordward_speed = VectorDistance(self->s.origin, target); // Get forward speed
	if (VectorEmpty(target)) // If Distance is zero
		return false;

	//fordward_speed = fordward_speed * 1.95f;
	//fordward_speed = fordward_speed * 0.2f;

	VectorSubtract(target, self->s.origin, dir); // Get direction
	//dir[2] += self->viewheight; // Add eye height to direction height
	float distance = VectorLength(dir); // Get distance

	float time = distance / fordward_speed;

	vec[0] = target[0] + time;
	vec[1] = target[1] + time;
	vec[2] = target[2] + time;

	// Adjust the z value by the height difference
	//if (height_diff != 0)
	//	vec[2] += height_diff;

	// If vector is pointing the wrong direction (pointing backwards)
	VectorNormalize(dir);
	vec3_t dir2;
	VectorSubtract(vec, self->s.origin, dir2);
	VectorNormalize(dir2);
	float dot = (dir2[0] * dir[0]) + (dir2[1] * dir[1]) + (dir2[2] * dir[2]);
	if (dot < 0)
	{
		VectorCopy(target, vec);
	}
	else
	{
		// if the shot is going to impact a nearby wall from our prediction, just fire it straight.
		trace_t tr = gi.trace(self->s.origin, NULL, NULL, vec, NULL, MASK_SOLID);
		if (tr.fraction < 0.9f)
		{
			VectorCopy(target, vec);
		}
	}

	//vec[2] += self->viewheight; // Add eye height to direction height

	VectorSubtract(vec, self->s.origin, dir); // Using
	VectorNormalize(dir);

	vec3_t angles = { self->s.angles[0], self->s.angles[1], self->s.angles[2] };
	angles[1] = vectoyaw(dir);
	AngleVectors(angles, forward, NULL, NULL); // Get forward vector
	VectorScale(forward, fordward_speed, self->velocity); // Scale the forward vector by the forward speed to get a velocity
	self->s.origin[2] += 1; // Adjust player slightly off the ground
	
	
	//self->velocity[0] = forward[0] * fordward_speed;
	//self->velocity[1] = forward[1] * fordward_speed;
	//self->velocity[2] = 450;

	if (height_diff > -2)
		self->velocity[2] = 432 + height_diff;
	else
	{
		if (fordward_speed + height_diff > 400)
			self->velocity[2] = 450 + height_diff;
		else
			self->velocity[2] = fordward_speed + height_diff;

		if (self->velocity[2] < 300)
			self->velocity[2] = fordward_speed;

		if (fordward_speed <= 64)
			self->velocity[2] = 450;
		else if (fordward_speed <= 96)
			self->velocity[2] = 550;
		else if (fordward_speed <= 128)
			self->velocity[2] = 650;
		else
			self->velocity[2] = 450;
	}

	self->velocity[2] = 432 + height_diff;
	




	

	//Com_Printf("%s %s speed:%f hdiff:%f  vel[2]:%f\n", __func__, self->client->pers.netname, fordward_speed, height_diff, self->velocity[2]);

	//self->velocity[2] = fwd_speed <= 400 ? (fwd_speed + height_diff) : (400 + height_diff);



	/*
	if (fwd_speed <= 64)
		self->velocity[2] = 300 + height_diff;
	else if (fwd_speed <= 96)
		self->velocity[2] = 350 + height_diff;
	else if (fwd_speed <= 128)
		self->velocity[2] = 400 + height_diff;
	else
		self->velocity[2] = 432 + height_diff;
	*/

	/*
	// peak speeds on flat surface
	velocity[0]; // 800
	velocity[1]; // 870
	velocity[2]; // 260
	speed;		// 937
	*/

	self->bot.jumppad = true; // Successful jump
	self->bot.node_jump_from = self->bot.current_node;
	self->bot.node_jump_to = self->bot.next_node;
	self->bot.jumppad_last_time = level.framenum + 1 * HZ; // Time until jump can be used again
	self->bot.jumppad_land_time = level.framenum + 1 * HZ; // Time until landing measures can take place

	return true;
}

//rekkie -- Quake3 -- s
// predictive calculator
// target is who you want to shoot
// start is where the shot comes from
// bolt_speed is how fast the shot is (or 0 for hitscan)
// eye_height is a boolean to say whether or not to adjust to targets eye_height
// offset is how much time to miss by
// aimdir is the resulting aim direction (pass in nullptr if you don't want it)
// aimpoint is the resulting aimpoint (pass in nullptr if don't want it)
void BOTLIB_PredictJumpPoint(edict_t* self, edict_t* target, vec3_t target_point, float target_viewheight, vec3_t target_velocity, vec3_t start, float bolt_speed, bool eye_height, float offset, vec3_t* aimdir, vec3_t* aimpoint)
{
	vec3_t dir, vec;
	float  dist, time;

	if (target) // Use entity's origin,viewheight,velocity if we can
	{
		VectorCopy(target->s.origin, target_point);
		target_viewheight = target->viewheight;
		VectorCopy(target->velocity, target_velocity);
	}

	VectorSubtract(target_point, start, dir);
	if (eye_height)
		dir[2] += target_viewheight;
	dist = VectorLength(dir);

	// [Paril-KEX] if our current attempt is blocked, try the opposite one
	vec3_t end;
	VectorMA(start, dist, dir, end);
	trace_t tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	//if (tr.ent != target)
	{
		eye_height = !eye_height;
		VectorSubtract(target_point, start, dir);
		if (eye_height)
			dir[2] += target_viewheight;
		dist = VectorLength(dir);
	}

	if (bolt_speed)
		time = dist / bolt_speed;
	else
		time = 0;

	// Calculate the jump height to get to the target
	//float gravity = self->gravity * sv_gravity->value;
	//float jump_height = sqrt(2 * (gravity * dist));
	//time = dist / (jump_height / 2); // Calculate the time it will take to get to the target

	//vec[0] = target_point[0] + (target_velocity[0] * (time - offset));
	//vec[1] = target_point[1] + (target_velocity[1] * (time - offset));
	//vec[2] = target_point[2] + (target_velocity[2] * (time - offset));
	vec[0] = target_point[0] + (target_velocity[0] * (time - offset));
	vec[1] = target_point[1] + (target_velocity[1] * (time - offset));
	vec[2] = target_point[2] + (target_velocity[2] * (time - offset));

	// went backwards...
	VectorNormalize(dir);
	vec3_t dir2;
	VectorSubtract(vec, start, dir2);
	VectorNormalize(dir2);
	float dot = (dir2[0] * dir[0]) + (dir2[1] * dir[1]) + (dir2[2] * dir[2]);
	if (dot < 0)
		VectorCopy(target_point, vec);
	else
	{
		// if the shot is going to impact a nearby wall from our prediction, just fire it straight.
		tr = gi.trace(start, NULL, NULL, vec, NULL, MASK_SOLID);
		if (tr.fraction < 0.9f)
			VectorCopy(target_point, vec);
	}

	if (eye_height)
		vec[2] += target_viewheight;

	if (aimdir)
	{
		VectorSubtract(vec, start, *aimdir);
		VectorNormalize(*aimdir);
	}

	if (aimpoint)
	{
		VectorCopy(vec, *aimpoint);
	}
}

qboolean BOTLIB_Jump_Takeoff(edict_t* self, edict_t* target, vec3_t target_point, float target_viewheight, vec3_t target_velocity)
{
	float length, fwd_speed;
	vec3_t forward, dir;

	if (self->bot.jumppad_last_time > level.framenum) // Can we jump again so soon?
		return false;

	if (self->client->leg_damage) // Can't jump if legs are damaged
		return false;

	//Com_Printf("%s %s -- LETS GO!\n", __func__, self->client->pers.netname);

	// Zero our horizontal velocity
	//target_velocity[0] = 0;
	//target_velocity[1] = 0;
	//target_velocity[2] = 0;

	// immediately turn to where we need to go
	if (target)
	{
		length = VectorDistance(self->s.origin, target->s.origin);
		VectorCopy(target->s.origin, target_point);
	}
	else
	{
		length = VectorDistance(self->s.origin, target_point);
	}
	if (VectorEmpty(target_point))
		return false;

	// Get height diff from self to target point
	// If > 0, target is higher than self
	// If < 0, target is lower than self
	float height_diff = target_point[2] - (self->s.origin[2] - 24);

	/*
	//if (height_diff < 0.1)
	//	height_diff = 0;
	float modifier = 1.5;
	if (height_diff > 0)
		modifier = (height_diff / 60) + 1.5;
	else if (height_diff < 0)
		modifier = 1.7 - (fabs(height_diff) / 200);
	
	//fwd_speed = length * 1.95f;
	//fwd_speed = length;
	//fwd_speed = (length + 32) * modifier;

	fwd_speed = (length * 1.1) + (height_diff);
	*/
	//Com_Printf("%s z[%f] mod[%f] len[%f]\n", __func__, height_diff, modifier, length);

	if (self->is_bot && self->bot.current_node != INVALID && self->current_link != INVALID)
	{
		fwd_speed = length; // * nodes[self->bot.current_node].links[self->current_link].targetNodeJumpPower; // Self adjusting jump power
		//Com_Printf("%s %s fwd_speed[%f]\n", __func__, self->client->pers.netname, fwd_speed);
	}
	else
	{
		fwd_speed = length;
	}


	
	if (target)
	{
		vec3_t target_point, target_velocity;
		BOTLIB_PredictJumpPoint(self, target, target_point, 0, target_velocity, self->s.origin, fwd_speed, false, height_diff, &dir, NULL);
	}
	else
		BOTLIB_PredictJumpPoint(self, NULL, target_point, target_viewheight, target_velocity, self->s.origin, fwd_speed, false, height_diff, &dir, NULL);

	vec3_t angles = { self->s.angles[0], self->s.angles[1], self->s.angles[2] };
	angles[1] = vectoyaw(dir);
	AngleVectors(angles, forward, NULL, NULL);
	self->s.origin[2] += 1;
	VectorScale(forward, fwd_speed, self->velocity);

	//print big jump
	//if (length < 360)
	//	Com_Printf("%s %s: Small Jump [%f]\n", __func__, self->client->pers.netname, length);
	//else
	//	Com_Printf("%s %s: Big Jump [%f]\n", __func__, self->client->pers.netname, length);
	
	/*
	// If the target is above the player, increase the velocity to get to the target
	float z_height;
	if (target && target->s.origin[2] >= self->s.origin[2])
	{
		z_height = (target->s.origin[2] - self->s.origin[2]);
		z_height += sqrtf(length) + NODE_Z_HEIGHT;
		self->velocity[2] += z_height;
	}
	else if (target_point[2] >= self->s.origin[2])
	{
		z_height = (target_point[2] - self->s.origin[2]);
		z_height += sqrtf(length) + NODE_Z_HEIGHT;
		self->velocity[2] += z_height;
	}
	else if (nodes[self->bot.next_node].origin[2] < nodes[self->bot.current_node].origin[2])
	{
		self->velocity[2] = 375; // 450
	}
	*/

	/*
	if (length <= 64)
		self->velocity[2] = 400 + height_diff; // 450
	else
		self->velocity[2] = 432 + height_diff;
	*/

	if (length <= 64)
		self->velocity[2] = 300 + height_diff;
	else if (length <= 96)
		self->velocity[2] = 350 + height_diff;
	else if (length <= 128)
		self->velocity[2] = 400 + height_diff;
	else
		self->velocity[2] = 432 + height_diff;




	/*
		// peak speeds on flat surface
		velocity[0]; // 800
		velocity[1]; // 870
		velocity[2]; // 260
		speed;		// 937
	*/
	/*
	// Get the surface normal of the ground the player is standing on
	trace_t tr = gi.trace(self->s.origin, tv(-16,-16,-24), tv(16,16,32), tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 128), self, MASK_PLAYERSOLID);

	//if (self->velocity[0] > 800) self->velocity[0] = 800;
	//if (self->velocity[1] > 870) self->velocity[1] = 870;
	if (self->velocity[2] > 260) self->velocity[2] = 260;
	if (tr.plane.normal[2] < 1.0)
	{
		float diff = 1.0 - tr.plane.normal[2];
		diff *= 2.0;
		diff += tr.plane.normal[2];
		self->velocity[2] *= diff;
	}
	// calculate velocity speed
	float speed = VectorLength(self->velocity);
	Com_Printf("%s speed[%f] velocity_peak[%f %f %f]\n", __func__, speed, self->velocity[0], self->velocity[1], self->velocity[2]);
	*/





	/*
	// get height diff from self to target
	qboolean above = false;
	if (target && target->s.origin[2] >= self->s.origin[2])
		above = true;
	else if (target_point[2] >= self->s.origin[2])
		above = true;

	float height_diff;
	if (target)
		height_diff = fabs(target->s.origin[2] - self->s.origin[2]);
	else
		height_diff = fabs(target_point[2] - self->s.origin[2]);
	Com_Printf("%s %s: Jump [%f] Height[%f]\n", __func__, self->client->pers.netname, length, height_diff);
	if (length < 256)
	{
		if (height_diff < 32)
			self->velocity[2] = 200;
		else
			self->velocity[2] = 400;
	}
	else if (length < 384)
		self->velocity[2] = 425;
	else
		self->velocity[2] = 450;
	*/

	self->bot.jumppad = true; // Successful jump
	self->bot.node_jump_from = self->bot.current_node;
	self->bot.node_jump_to = self->bot.next_node;
	self->bot.jumppad_last_time = level.framenum + 1 * HZ; // Time until jump can be used again
	self->bot.jumppad_land_time = level.framenum + 1 * HZ; // Time until landing measures can take place

	return true;
}

// Predict enemy position based on their position, velocity, and our velocity
void BOTLIB_PredictEnemyOrigin(edict_t *self, vec3_t out, float multiplier)
{
	// FRAMETIME
	/*
	out[0] = self->enemy->s.origin[0] + ((self->enemy->velocity[0] + self->velocity[0]) * (10.0 / HZ));
	out[1] = self->enemy->s.origin[1] + ((self->enemy->velocity[1] + self->velocity[1]) * (10.0 / HZ));

	if (self->enemy->maxs[2] == CROUCHING_MAXS2) // Predict lower if enemy is crouching
		out[2] = (self->enemy->s.origin[2] - 24) + ((self->enemy->velocity[2] + self->velocity[2]) * (10.0 / HZ));
	else // Predict when standing
		out[2] = self->enemy->s.origin[2] + ((self->enemy->velocity[2] + self->velocity[2]) * (10.0 / HZ));
	*/

	if (rand() % 2 == 0)
		multiplier -= (multiplier * 2);

	//Com_Printf("%s %f\n", __func__, FRAMETIME);
	VectorAdd(self->velocity, tv(self->enemy->velocity[0] * multiplier, self->enemy->velocity[1] * multiplier, self->enemy->velocity[2]), out);
	VectorScale(out, FRAMETIME, out);
	VectorAdd(out, self->enemy->s.origin, out);
	//VectorCopy(self->enemy->s.origin, out);
	if (self->enemy->maxs[2] == CROUCHING_MAXS2)
		out[2] -= 24;

	if (self->client->weapon == FindItemByNum(KNIFE_NUM))
	{
		// Quake 2 uses x,y,z which is stored in a float array called vec3_t[]
		// The Z component is for jumping and falling
		// Throwing knife affected by gravity over time
		//knife->velocity[2] -= sv_gravity->value * FRAMETIME; // velocity is a vec3_t[3] which is a float array
		// My origin
		self->s.origin;	// origin is a vec3_t[3] which is a float array
		// Enemy origin
		self->enemy->s.origin; // origin is a vec3_t[3] which is a float array
		// My pitch aim (looking directly forward is a pitch of 0)
		// #define PITCH 0
		// #define YAW 1
		// #define ROLL 2
		self->s.angles[PITCH];

		// I want to find the aiming pitch requires to reach a target location with a throwing knife that drops with gravity.
		// Quake2 uses x,y,z coordinates stored in a vec3_t[3] which is a float array [0] [1] [2]
		// I have a first-person 3D game. A player origin, a target, gravity (on the Z axis only), and a throwing knife that is affected by gravity. By default the player looks forward at a pitch of 0. Looking directly up is -90 and down is 90.
		
		// Commenting unused?
		//float g = sv_gravity->value; // Gravity
		//float distance = VectorDistanceXY(self->s.origin, self->enemy->s.origin); // XY Distance to enemy
		//float height_diff = self->enemy->s.origin[2] - self->s.origin[2]; // Height difference
		//float knife_speed = 1200;
		
		// Using the distance to my enemy and the gravity of the knife what pitch would I need to set to reach my enemy?

		// Calculate total flight time
		//float t = sqrtf((2 * distance) / sv_gravity->value);

		

	}

}




/////////////////
// Look dir
/////////////////
void BOTLIB_Look(edict_t* self, usercmd_t* ucmd)
{
	vec3_t lookdir = { 0 }; // Direction to look
	trace_t tr;
	//float turn_speed = (MAX_BOTSKILL - self->bot.skill) * 3 / 9; // [Min:0 Max:10] skill "10" == 0, Skill "0" == 3
	float turn_speed = 0.3;
	qboolean reached_look_at = false;
	// Declarations moved outside the labeled block
	const int look_ahead = 5;
	qboolean found_viable_node = false;	

	if (VectorEmpty(self->bot.bi.look_at) == false)
	{
		reached_look_at = BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 0.15, true, true);

		if (reached_look_at == false)
			return;
	}


	
	// Look at ladder
	if ((self->bot.current_node != INVALID && self->bot.next_node != INVALID) && (nodes[self->bot.current_node].type == NODE_LADDER || nodes[self->bot.next_node].type == NODE_LADDER))
	{
		// Check we're looking at ladder
		qboolean looking_at_ladder = false;
		{
			float yaw_rad = 0;
			vec3_t fwd = { 0 }, end = { 0 };

			yaw_rad = DEG2RAD(self->s.angles[YAW]);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);

			VectorMA(self->s.origin, 60, fwd, end);

			vec3_t lmins = { -16, -16, -96 };
			vec3_t lmaxs = { 16, 16, 96 };

			tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);

			looking_at_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			if (looking_at_ladder)
				self->bot.touching_ladder = true;
		}
		// Turn to find ladder
		if (looking_at_ladder == false)
		{
			int additional_rotation = 0;
			for (int i = 0; i < 16; i++)
			{
				vec3_t fwd = { 0 }, end = { 0 };

				self->s.angles[YAW] += 22.5; // (22.5 * 16) = 360

				float yaw_rad = DEG2RAD(self->s.angles[YAW]);

				fwd[0] = cos(yaw_rad);
				fwd[1] = sin(yaw_rad);

				VectorMA(self->s.origin, 60, fwd, end);

				vec3_t lmins = { -16, -16, -96 };
				vec3_t lmaxs = { 16, 16, 96 };

				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);

				if ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER)) // Found ladder
				{
					self->bot.touching_ladder = true;
					VectorSubtract(end, self->s.origin, self->bot.bi.look_at);
					////BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 10.0, true, true);
					if (additional_rotation >= 1)
						break;
					additional_rotation++;
				}
			}
		}

		// Look up or down when on ladder
		if ((self->bot.current_node != INVALID && self->bot.next_node != INVALID) && (nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type == NODE_LADDER))
		{
			if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Going up
			{
				//Com_Printf("%s %s ladder up [%d]\n", __func__, self->client->pers.netname, level.framenum);

				// Level out at top or bottom
				if (VectorDistance(self->s.origin, nodes[self->bot.next_node].origin) <= 64 ||
					VectorDistance(self->s.origin, nodes[self->bot.current_node].origin) <= 64)
					self->bot.bi.viewangles[PITCH] = 0;
				else
					self->bot.bi.viewangles[PITCH] = -45; //-89
			}
			else if (nodes[self->bot.next_node].origin[2] < nodes[self->bot.current_node].origin[2]) // Going down
			{
				//Com_Printf("%s %s ladder down [%d]\n", __func__, self->client->pers.netname, level.framenum);

				// Level out at top or bottom
				if (VectorDistance(self->s.origin, nodes[self->bot.next_node].origin) <= 64 ||
					VectorDistance(self->s.origin, nodes[self->bot.current_node].origin) <= 64)
					self->bot.bi.viewangles[PITCH] = 0;
				else
					self->bot.bi.viewangles[PITCH] = 45; // 89
			}
		}
	}
	// Look at enemy
	else if (self->enemy) // Track target
	{
		qboolean not_infront = false; // If target is in front

		// Update bot to look at an enemy it can see. If the enemy is obstructed behind a wall, 
		// the bot will keep looking in that general direction, but not directly at the enemy's pos
		if (self->bot.see_enemies) // || BOTLIB_Infront(self, self->enemy, 0.3) == false)
		{
			if (BOTLIB_Infront(self, self->enemy, 0.3) == false)
				not_infront = true;

			if (1) // Predicted enemy pos
			{
				// Predict enemy position based on their velocity and distance
				vec3_t predicted_enemy_origin = { 0 };
				BOTLIB_PredictEnemyOrigin(self, predicted_enemy_origin, 2);
				if (0) // Debug draw predicted enemy origin - blue is predicted, yellow is actual
				{
					uint32_t red = MakeColor(255, 255, 255, 255); // red
					uint32_t yellow = MakeColor(255, 255, 0, 255); // Yellow
					void (*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = NULL;
					DrawBox = players[0]->client->pers.draw->DrawBox;
					players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
					DrawBox(self->enemy->s.number, predicted_enemy_origin, yellow, tv(-16, -16, -24), tv(16, 16, 32), 200, false);
					DrawBox(self->enemy->s.number + 64, self->enemy->s.origin, red, tv(-16, -16, -24), tv(16, 16, 32), 100, false);
				}
				VectorSubtract(predicted_enemy_origin, self->s.origin, self->bot.bi.look_at); // Aim at enemy
				/*
				if (self->enemy->maxs[2] == CROUCHING_MAXS2) // If enemy is crouching
					VectorSubtract(tv(predicted_enemy_origin[0], predicted_enemy_origin[1], predicted_enemy_origin[2] - 24), self->s.origin, self->bot.bi.look_at); // Aim lower for crouched enemies
				else
					VectorSubtract(predicted_enemy_origin, self->s.origin, self->bot.bi.look_at); // Aim at standing enemy
				*/
			}
			else // Not predicted
			{
				if (self->enemy->maxs[2] == CROUCHING_MAXS2) // If enemy is crouching
					VectorSubtract(tv(self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] - 24), self->s.origin, self->bot.bi.look_at); // Aim lower for crouched enemies
				else
					VectorSubtract(self->enemy->s.origin, self->s.origin, self->bot.bi.look_at); // Aim at standing enemy
			}
		}
		else // Look where enemy was last
		{
			// Test to see if bot is looking at a wall, if so goto LookAhead, otherwise keep tracking enemy_seen_loc
			vec3_t eyes;
			VectorCopy(self->s.origin, eyes);
			eyes[2] += self->viewheight; // Get our eye level (standing and crouching)
			vec3_t enemy_eyes;
			VectorCopy(self->enemy->s.origin, enemy_eyes);
			enemy_eyes[2] += self->enemy->viewheight; // Get our enemy eye level (standing and crouching)
			trace_t tr = gi.trace(eyes, NULL, NULL, enemy_eyes, self, MASK_SHOT); // Trace to target
			float distance = VectorDistance(self->s.origin, tr.endpos);
			if (distance > 256)
				VectorSubtract(self->bot.enemy_seen_loc, self->s.origin, self->bot.bi.look_at);
			else
				goto LookAhead; // Looking at a wall, so just look at nodes
		}
		/*
		// Adjust turn speed based on skill
		//float turn_speed = (10 - bot_skill->value) * 3 / 9; // [Min:0 Max:10] skill "10" == 0, Skill "0" == 3
		//float turn_speed = (MAX_BOTSKILL - self->bot.skill) * 3 / 9; // [Min:0 Max:10] skill "10" == 0, Skill "0" == 3
		if (self->bot.skill >= MAX_BOTSKILL)
		{
			turn_speed = 0.2;
		}
		//if (bot_skill_threshold->value > 0 && self->bot.skill >= MAX_BOTSKILL)
		{
			//turn_speed = 0.1; // Slightly reduce max turn speed when using bot_skill_threshold
		}

		if (turn_speed > 1.0)
		{
			if (not_infront)
				turn_speed = 1.0;

			if (self->client->weapon == FindItemByNum(HC_NUM))//FindItem(HC_NAME)) // HC bots get almost instant aim
				turn_speed = 0.2;
			else if (self->client->weapon == FindItemByNum(KNIFE_NUM)) // Knife bots get almost instant aim
				turn_speed = 0.2;
			else if (self->client->weapon == FindItemByNum(GRENADE_NUM)) // Grenade bots get almost instant aim
				turn_speed = 0.2;
			else if (rand() % 2 == 0 && self->client->weapon == FindItemByNum(M3_NUM)) // Bots sometimes get aim snap
				turn_speed = 0.5;
			else if (rand() % 2 == 0 && self->client->weapon == FindItemByNum(SNIPER_NUM)) // Bots sometimes get aim snap
				turn_speed = 0.1;
			else if (rand() % 10 == 0) // Bots sometimes get aim snap
				turn_speed = 0.5;

			turn_speed = 1.0;
		}
		*/

		//Com_Printf("%s %s turn_speed[%f]\n", __func__, self->client->pers.netname, turn_speed);

		//if (self->client->weaponstate != WEAPON_READY)
		//	not_infront = rand() % 2;

		/*
		if (not_infront) // Just 'pan' left and right (don't pitch up and down until enemy is in view)
		{
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
		}
		else
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
		*/
	}
	/*
	// When at a POI, get a direction to look at by finding a node NODE_POI_LOOKAT link
	else if (self->bot.current_node != INVALID && self->bot.next_node != INVALID && nodes[self->bot.current_node].type == NODE_POI)
	{
		// See if this node has any POI lookat links
		int num_lookat_nodes = 0;
		int lookat_nodes[MAXLINKS] = { 0 };
		for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)
		{
			if (nodes[self->bot.current_node].links[i].targetNodeType == NODE_POI_LOOKAT)
			{
				lookat_nodes[num_lookat_nodes] = nodes[self->bot.current_node].links[i].targetNode;
				num_lookat_nodes++;
			}
		}
		if (num_lookat_nodes) // One or more POI lookat nodes found
		{
			// When the timer is up pick a new node to look at
			if (self->bot.node_poi_look_time < level.framenum)
			{
				self->bot.node_poi_look_time = level.framenum + 5 * HZ;
				self->bot.node_poi_lookat_node = lookat_nodes[rand() % num_lookat_nodes]; // Pick a random POI lookat node
			}

			// Look at POI lookat node
			VectorSubtract(nodes[self->bot.node_poi_lookat_node].origin, self->s.origin, self->bot.bi.look_at);
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 2.0);
		}
	}
	*/
	else if (0 && self->bot.current_node != INVALID && nodes[self->bot.current_node].num_links)
	{
		//for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)
		{
		}
		//rand() % nodes[self->bot.current_node].num_links + 1;
		Com_Printf("%s found_viable_node [%d]\n", __func__, level.framenum);

		VectorSubtract(nodes[rand() % nodes[self->bot.current_node].num_links + 1].origin, self->s.origin, self->bot.bi.look_at);
		////BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
	}
	// If no enemy, have bot look at PNOISES, if any.
	else if (1)
	{
		int type = 0;
		int player_num = INVALID;
		float nearest = 9999999;
		qboolean found_target = false;
		for (int i = 0; i < num_players; i++)
		{
			if (players[i] != self && OnSameTeam(self, players[i]) == false)
			{
				if (botlib_noises.weapon_time[i] > 0 && BOTLIB_MovingToward(self, botlib_noises.weapon_origin[i], 0.3))
				{
					//Com_Printf("%s %s look at weapon_origin [%d]\n", __func__, self->client->pers.netname, botlib_noises.weapon_time[i]);
					float dist = VectorDistance(botlib_noises.weapon_origin[i], self->s.origin);
					if (dist < nearest && dist > 256)
					{
						type = PNOISE_WEAPON;
						nearest = dist;
						player_num = i;
					}
				}
				if (botlib_noises.self_time[i] > 0 && BOTLIB_MovingToward(self, botlib_noises.self_origin[i], 0.3))
				{
					float dist = VectorDistance(botlib_noises.self_origin[i], self->s.origin);
					if (dist < nearest && dist > 256)
					{
						type = PNOISE_SELF;
						nearest = dist;
						player_num = i;
					}
				}
			}
		}
		if (player_num == INVALID)
		{
			for (int i = 0; i < num_players; i++)
			{
				if (players[i] != self && OnSameTeam(self, players[i]) == false)
				{
					if (botlib_noises.impact_time[i] > 0 && BOTLIB_MovingToward(self, botlib_noises.impact_origin[i], 0.3))
					{
						float dist = VectorDistance(botlib_noises.impact_origin[i], self->s.origin);
						if (dist < nearest && dist > 256)
						{
							type = PNOISE_WEAPON;
							nearest = dist;
							player_num = i;
						}
					}
				}
			}
		}

		if (player_num != INVALID)
		{
			vec3_t noise_origin;

			if (type == PNOISE_WEAPON)
				VectorCopy(botlib_noises.weapon_origin[player_num], noise_origin);
			else if (type == PNOISE_SELF)
				VectorCopy(botlib_noises.self_origin[player_num], noise_origin);
			else if (type == PNOISE_IMPACT)
				VectorCopy(botlib_noises.impact_origin[player_num], noise_origin);


			// Test to see if bot is looking at a wall, if so goto LookAhead, otherwise keep tracking noise
			vec3_t eyes;
			VectorCopy(self->s.origin, eyes);
			eyes[2] += self->viewheight; // Get our eye level (standing and crouching)
			vec3_t noise_origin_eyes;
			VectorCopy(noise_origin, noise_origin_eyes);
			noise_origin_eyes[2] += self->viewheight; // Set noise origin to eye level
			trace_t tr = gi.trace(eyes, NULL, NULL, noise_origin_eyes, self, MASK_SHOT); // Trace to target
			float distance = VectorDistance(self->s.origin, tr.endpos);
			if (distance > 256)
			{
				VectorSubtract(noise_origin_eyes, self->s.origin, self->bot.bi.look_at);
				////BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
			}
			else
				goto LookAhead;
		}
		else
			goto LookAhead;
	}
	// If no enemy, look at nodes in front/behind, or look at map center
	else // if (self->enemy == NULL)
	{
	LookAhead:
		for (int i = self->bot.node_list_current; i < self->bot.node_list_count; i++) // Loop from current node until end of list
		{
			/*
			if ((i + look_ahead) < self->bot.node_list_count && self->bot.node_list[i + look_ahead] != INVALID)
			{
				int look_node = self->bot.node_list[(i + look_ahead)];
				if (nodes[look_node].num_links)
				{
					int rnd_link = rand() % nodes[look_node].num_links + 1;
					int rnd_node = nodes[look_node].links[rnd_link].targetNode;
					if (rnd_node != INVALID)
					{
						VectorSubtract(nodes[rnd_node].origin, self->s.origin, self->bot.bi.look_at);
						BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 0.1);
						found_viable_node = true;
						break;
					}
				}
			}
			*/

			// Look x nodes ahead
			if ((i + look_ahead) < self->bot.node_list_count && self->bot.node_list[i + look_ahead] != INVALID)
			{
				VectorSubtract(nodes[self->bot.node_list[(i + look_ahead)]].origin, self->s.origin, self->bot.bi.look_at);
				////BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
				found_viable_node = true;
				break;
			}
			// Look x nodes behind
			else if ((i - look_ahead) < self->bot.node_list_count && self->bot.node_list[(i - look_ahead)] != INVALID)
			{
				VectorSubtract(nodes[self->bot.node_list[(i - look_ahead)]].origin, self->s.origin, self->bot.bi.look_at);
				////BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
				found_viable_node = true;
				break;
			}
			else
				break;
		}
		//if (found_viable_node)
		//	Com_Printf("%s found_viable_node [%d]\n", __func__, level.framenum);
		//else
		//	Com_Printf("%s no node [%d]\n", __func__, level.framenum);

		if (0 && found_viable_node == false && self->bot.next_node != INVALID && nodes[self->bot.next_node].num_links)
		{
			//for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)
			{
			}
			//rand() % nodes[self->bot.current_node].num_links + 1;
			Com_Printf("%s found_viable_node [%d]\n", __func__, level.framenum);

			VectorSubtract(nodes[rand() % nodes[self->bot.current_node].num_links + 1].origin, self->s.origin, self->bot.bi.look_at);
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
		}

		if (0 && found_viable_node == false)
		{
			// If no enemy, look at random thing
			{
				// If no enemy, look at random item or map center
				// Search through map looking for a random item, or NULL
				//if (self->bot.bi.look_at_ent == NULL || self->bot.bi.look_at_ent_time < level.framenum)
				if (self->bot.bi.look_at_time < level.framenum)
				{
					self->bot.bi.look_at_time = level.framenum + 1 * HZ; // Change target every 5 seconds

					edict_t* map_ents;
					int edict_num = 1 + game.maxclients; // skip worldclass & players
					for (map_ents = g_edicts + edict_num; map_ents < &g_edicts[globals.num_edicts]; map_ents++)
					{
						// Skip ents that are not in used and not an item
						if (map_ents->inuse == false || map_ents->item == NULL)
							continue;

						//trace_t tr = gi.trace(self->s.origin, NULL, NULL, map_ents->s.origin, self, MASK_SOLID);
						//if (tr.fraction == 1.0)
						{
							VectorCopy(map_ents->s.origin, self->bot.bi.look_at);
							break;
						}
					}
				}
				if (VectorEmpty(self->bot.bi.look_at))
					VectorSubtract(vec3_origin, self->s.origin, lookdir); // Look at center of map
				else
					VectorSubtract(self->bot.bi.look_at, self->s.origin, lookdir); // Look at random map item

			}

			// If lookdir is empty, then look at map center
			if (VectorEmpty(self->bot.bi.look_at))
			{
				VectorSubtract(vec3_origin, self->s.origin, lookdir);
			}

			BOTLIB_ChangeBotAngleYawPitch(self, lookdir, false, turn_speed, true, true);
		}
	}
}

void BOTLIB_CrouchFire(edict_t* self)
{
	if (self == NULL || self->client == NULL || self->client->weapon == NULL) {
        return; // Early exit if self, client, or weapon is NULL
    }
	gitem_t* clweapon = self->client->weapon;

	// More skillful bots will crouch when firing
	if (self->bot.skill.aim >= 0) {
		if ((self->bot.bi.actionflags & ACTION_MOVELEFT) == 0 && 
			(self->bot.bi.actionflags & ACTION_MOVERIGHT) == 0)
		{
			// Raptor007: Don't crouch if it blocks the shot.
			float old_z = self->s.origin[2];
			self->s.origin[2] -= 14;
			if (ACEAI_CheckShot(self))
			{
				self->bot.bi.actionflags |= ACTION_CROUCH;
			}
			self->s.origin[2] = old_z;
		}
	}
}

void BOTLIB_Wander(edict_t* self, usercmd_t* ucmd)
{
	//if (teamplay->value && lights_camera_action > 0)
	//	return;

	int next_node = INVALID; // Next node to walk towards
	//vec3_t lookdir = { 0 }; // Direction to look
	vec3_t walkdir; // Direction to walk
	vec3_t lastdir;
	float move_speed = SPEED_RUN; // Movement speed, default to running
	self->bot.bi.speed = 0; // Zero speed
	trace_t tr; // Trace

	// Remove flags
	//self->bot.bi.actionflags = 0;
	//self->enemy = NULL;

	// Prevent stuck suicide if holding position
	if ((self->bot.bi.actionflags & ACTION_HOLDPOS))
		self->suicide_timeout = level.framenum + 10;

	// Check how far we've moved
	qboolean moved = true;
	VectorSubtract(self->s.origin, self->lastPosition, lastdir);
	float move_dist = VectorLength(lastdir);
	if (move_dist < FRAMETIME)
		moved = false; // We've not moved


	// If the bot is near the get_item they're after, and the item is inuse
	// inuse == false if the item was picked up and waiting to respawn
	// inuse == true if the item has spawned in and is ready to be picked up
	if (self->bot.get_item != NULL && self->bot.get_item->inuse)
	{
		float item_dist = VectorDistance(self->bot.get_item->s.origin, self->s.origin);
		//Com_Printf("%s %s is looking for item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);

		// item spawnflags
		//#define ITEM_TRIGGER_SPAWN              0x00000001
		//#define ITEM_NO_TOUCH                   0x00000002
		// 6 bits reserved for editor flags
		// 8 bits used as power cube id bits for coop games
		//#define DROPPED_ITEM                    0x00010000
		//#define DROPPED_PLAYER_ITEM             0x00020000
		//#define ITEM_TARGETS_USED               0x00040000
		// 
		// Edict Flags
		// #define FL_RESPAWN                      0x80000000	// used for item respawning
		// 
		// 
		// TP item not picked up
		// spawnflags = 0
		// flags = 0
		// 
		// TP item picked up and in bot inventory
		// spawnflags = ITEM_TARGETS_USED 0x00040000
		// flags = FL_RESPAWN 0x80000000
		//
		// TP item picked up then dropped by bot
		// spawnflags = DROPPED_ITEM
		// flags = 0
		// 
		// TP item picked up, bot killed, dropping all items to ground
		// spawnflags = DROPPED_PLAYER_ITEM
		// flags = 0
		
		trace_t tr = gi.trace(self->s.origin, NULL, NULL, self->bot.get_item->s.origin, NULL, MASK_PLAYERSOLID);
		if (tr.fraction == 1.0 && item_dist <= 128) // Might want to do a trace line to see if bot can actually see item
		{
			//Com_Printf("%s %s is near item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);
			VectorSubtract(self->bot.get_item->s.origin, self->s.origin, walkdir); // Head to item
			VectorNormalize(walkdir);

			//horizontal direction
			vec3_t hordir;
			hordir[0] = walkdir[0];
			hordir[1] = walkdir[1];
			hordir[2] = 0;
			VectorNormalize(hordir);
			VectorCopy(hordir, self->bot.bi.dir);

			self->bot.bi.speed = move_speed; // Set our suggested speed

			int perform_action = BOTLIB_Crouch_Or_Jump(self, ucmd, walkdir); // Crouch or jump if needed
			if (perform_action == ACTION_NONE)
			{
				//self->bot.bi.actionflags &= ~ACTION_CROUCH; // Remove crouching
				//self->bot.bi.actionflags &= ~ACTION_JUMP; // Remove jumping
			}
			else if (perform_action == ACTION_JUMP)
			{
				self->bot.bi.actionflags |= ACTION_JUMP; // Add jumping
				self->bot.bi.actionflags &= ~ACTION_CROUCH; // Remove crouching
			}
			else if (perform_action == ACTION_CROUCH)
			{
				self->bot.bi.actionflags |= ACTION_CROUCH; // Add crouching
				self->bot.bi.actionflags &= ~ACTION_JUMP; // Remove jumping
			}

			if (item_dist < 128)
				self->bot.bi.speed = SPEED_ROAM; // Slow down when close

			if (self->bot.get_item->solid == SOLID_NOT) // picked up
			{
				//Com_Printf("%s %s grabbed item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);
				self->bot.get_item = NULL;
				return;
			}		
			
			return;

		}
		//else if (item_dist <= 64)
		{
			//Com_Printf("%s %s grabbed item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);
			//self->bot.get_item = NULL;
		}
	}

	/*
	// Get current and next node back from nav code.
	{
		//Com_Printf("%s %s next_node:%d current_node:%d goal_node:%d\n", __func__, self->client->pers.netname, self->bot.next_node, self->bot.current_node, self->bot.goal_node);

		self->bot.bi.speed = 0;
		self->bot.bi.actionflags = 0;

		self->bot.state = STATE_WANDER;
		//self->wander_timeout = level.framenum + 1.0 * HZ;
		self->bot.goal_node = INVALID;
		return;
	}
	*/

	if (self->bot.touching_ladder == false)// || (nodes[self->bot.next_node].type != NODE_LADDER && self->groundentity))
	{
		self->bot.bi.actionflags &= ~ACTION_MOVEDOWN;
		self->bot.bi.actionflags &= ~ACTION_MOVEUP;
	}

#if 0
	/////////////////
	// Look dir
	/////////////////
	qboolean touching_ladder = false;
	//if (nodes[self->bot.current_node].type == NODE_LADDER || nodes[self->bot.next_node].type == NODE_LADDER)
	{
		// Check if touching ladder
		{
			float yaw_rad = 0;
			vec3_t fwd = { 0 }, end = { 0 };
			trace_t tr;

			yaw_rad = DEG2RAD(self->s.angles[YAW]);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);

			//VectorMA(self->s.origin, 1, fwd, end);
			//tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_PLAYERSOLID);

			VectorMA(self->s.origin, 1, fwd, end);
			vec3_t lmins = { -16, -16, -96 };
			vec3_t lmaxs = { 16, 16, 96 };
			tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);

			touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));

			if (touching_ladder == false)
			{
				VectorMA(self->s.origin, 8, fwd, end);
				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);
				touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			}
			if (touching_ladder == false)
			{
				VectorMA(self->s.origin, 16, fwd, end);
				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);
				touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			}
			if (touching_ladder == false)
			{
				VectorMA(self->s.origin, 32, fwd, end);
				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);
				touching_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			}
		}
	}
	//if (touching_ladder)
	//	Com_Printf("%s touching_ladder [%d]\n", __func__, level.framenum);
	if (touching_ladder == false)// || (nodes[self->bot.next_node].type != NODE_LADDER && self->groundentity))
	{
		self->bot.bi.actionflags &= ~ACTION_MOVEDOWN;
		self->bot.bi.actionflags &= ~ACTION_MOVEUP;
		touching_ladder = false;
	}

	// Look at ladder
	if ((self->bot.current_node != INVALID && self->bot.next_node != INVALID) && (nodes[self->bot.current_node].type == NODE_LADDER || nodes[self->bot.next_node].type == NODE_LADDER))
	{
		// Check we're looking at ladder
		qboolean looking_at_ladder = false;
		{
			float yaw_rad = 0;
			vec3_t fwd = { 0 }, end = { 0 };
			trace_t tr;

			yaw_rad = DEG2RAD(self->s.angles[YAW]);
			fwd[0] = cos(yaw_rad);
			fwd[1] = sin(yaw_rad);

			VectorMA(self->s.origin, 60, fwd, end);

			vec3_t lmins = { -16, -16, -96 };
			vec3_t lmaxs = { 16, 16, 96 };

			tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);

			looking_at_ladder = ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER));
			if (looking_at_ladder)
				touching_ladder = true;
		}
		// Turn to find ladder
		if (looking_at_ladder == false)
		{
			int additional_rotation = 0;
			for (int i = 0; i < 16; i++)
			{
				vec3_t fwd = { 0 }, end = { 0 };
				trace_t tr;

				self->s.angles[YAW] += 22.5; // (22.5 * 16) = 360

				float yaw_rad = DEG2RAD(self->s.angles[YAW]);

				fwd[0] = cos(yaw_rad);
				fwd[1] = sin(yaw_rad);

				VectorMA(self->s.origin, 60, fwd, end);

				vec3_t lmins = { -16, -16, -96 };
				vec3_t lmaxs = { 16, 16, 96 };

				tr = gi.trace(self->s.origin, lmins, lmaxs, end, self, MASK_PLAYERSOLID);

				if ((tr.fraction < 1) && (tr.contents & CONTENTS_LADDER)) // Found ladder
				{
					touching_ladder = true;
					VectorSubtract(end, self->s.origin, self->bot.bi.look_at);
					BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 3.0, true, true);
					if (additional_rotation >= 1)
						break;
					additional_rotation++;
				}
			}
		}

		// Look up or down when on ladder
		if ((self->bot.current_node != INVALID && self->bot.next_node != INVALID) && (nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type == NODE_LADDER))
		{
			if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Going up
			{
				//Com_Printf("%s %s ladder up [%d]\n", __func__, self->client->pers.netname, level.framenum);

				// Level out at top or bottom
				if (VectorDistance(self->s.origin, nodes[self->bot.next_node].origin) <= 64 ||
					VectorDistance(self->s.origin, nodes[self->bot.current_node].origin) <= 64)
					self->bot.bi.viewangles[PITCH] = 0;
				else
					self->bot.bi.viewangles[PITCH] = -45; //-89
			}
			else if (nodes[self->bot.next_node].origin[2] < nodes[self->bot.current_node].origin[2]) // Going down
			{
				//Com_Printf("%s %s ladder down [%d]\n", __func__, self->client->pers.netname, level.framenum);

				// Level out at top or bottom
				if (VectorDistance(self->s.origin, nodes[self->bot.next_node].origin) <= 64 ||
					VectorDistance(self->s.origin, nodes[self->bot.current_node].origin) <= 64)
					self->bot.bi.viewangles[PITCH] = 0;
				else
					self->bot.bi.viewangles[PITCH] = 45; // 89
			}
		}
	}
	// Look at enemy
	else if (self->enemy) // Track target
	{
		qboolean not_infront = false; // If target is in front

		// Update bot to look at an enemy it can see. If the enemy is obstructed behind a wall, 
		// the bot will keep looking in that general direction, but not directly at the enemy's pos
		if (self->bot.see_enemies) // || BOTLIB_Infront(self, self->enemy, 0.3) == false)
		{
			if (BOTLIB_Infront(self, self->enemy, 0.3) == false)
				not_infront = true;

			if (1) // Predicted enemy pos
			{
				// Predict enemy position based on their velocity and distance
				vec3_t predicted_enemy_origin = { 0 };
				BOTLIB_PredictEnemyOrigin(self, predicted_enemy_origin, 1);
				if (0) // Debug draw predicted enemy origin - blue is predicted, yellow is actual
				{
					uint32_t blue = MakeColor(0, 0, 255, 255); // Blue
					uint32_t yellow = MakeColor(255, 255, 0, 255); // Yellow
					void (*DrawBox)(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded) = NULL;
					DrawBox = players[0]->client->pers.draw->DrawBox;
					players[0]->client->pers.draw->boxes_inuse = true; // Flag as being used
					DrawBox(self->enemy->s.number, predicted_enemy_origin, blue, tv(-16, -16, -24), tv(16, 16, 32), 100, false);
					DrawBox(self->enemy->s.number + 64, self->enemy->s.origin, yellow, tv(-16, -16, -24), tv(16, 16, 32), 100, false);
				}
				VectorSubtract(predicted_enemy_origin, self->s.origin, self->bot.bi.look_at); // Aim at enemy
				/*
				if (self->enemy->maxs[2] == CROUCHING_MAXS2) // If enemy is crouching
					VectorSubtract(tv(predicted_enemy_origin[0], predicted_enemy_origin[1], predicted_enemy_origin[2] - 24), self->s.origin, self->bot.bi.look_at); // Aim lower for crouched enemies
				else
					VectorSubtract(predicted_enemy_origin, self->s.origin, self->bot.bi.look_at); // Aim at standing enemy
				*/
			}
			else // Not predicted
			{
				if (self->enemy->maxs[2] == CROUCHING_MAXS2) // If enemy is crouching
					VectorSubtract(tv(self->enemy->s.origin[0], self->enemy->s.origin[1], self->enemy->s.origin[2] - 24), self->s.origin, self->bot.bi.look_at); // Aim lower for crouched enemies
				else
					VectorSubtract(self->enemy->s.origin, self->s.origin, self->bot.bi.look_at); // Aim at standing enemy
			}
		}
		else // Look where enemy was last
		{
			// Test to see if bot is looking at a wall, if so goto LookAhead, otherwise keep tracking enemy_seen_loc
			vec3_t eyes;
			VectorCopy(self->s.origin, eyes);
			eyes[2] += self->viewheight; // Get our eye level (standing and crouching)
			vec3_t enemy_eyes;
			VectorCopy(self->enemy->s.origin, enemy_eyes);
			enemy_eyes[2] += self->enemy->viewheight; // Get our enemy eye level (standing and crouching)
			trace_t tr = gi.trace(eyes, NULL, NULL, enemy_eyes, self, MASK_SHOT); // Trace to target
			float distance = VectorDistance(self->s.origin, tr.endpos);
			if (distance > 256)
				VectorSubtract(self->bot.enemy_seen_loc, self->s.origin, self->bot.bi.look_at);
			else
				goto LookAhead; // Looking at a wall, so just look at nodes
		}

		// Adjust turn speed based on skill
		//float turn_speed = (10 - bot_skill->value) * 3 / 9; // [Min:0 Max:10] skill "10" == 0, Skill "0" == 3
		float turn_speed = (MAX_BOTSKILL - self->bot.skill) * 3 / 9; // [Min:0 Max:10] skill "10" == 0, Skill "0" == 3
		if (self->bot.skill >= MAX_BOTSKILL)
		{
			turn_speed = 0.2;
		}
		//if (bot_skill_threshold->value > 0 && self->bot.skill >= MAX_BOTSKILL)
		{
			//turn_speed = 0.1; // Slightly reduce max turn speed when using bot_skill_threshold
		}

		if (turn_speed > 1.0)
		{
			if (not_infront)
				turn_speed = 2.0;

			if (self->client->weapon == FindItemByNum(HC_NUM))//FindItem(HC_NAME)) // HC bots get almost instant aim
				turn_speed = 0.2;
			else if (self->client->weapon == FindItemByNum(KNIFE_NUM)) // Knife bots get almost instant aim
				turn_speed = 0.2;
			else if (self->client->weapon == FindItemByNum(GRENADE_NUM)) // Grenade bots get almost instant aim
				turn_speed = 0.2;
			else if (rand() % 2 == 0 && self->client->weapon == FindItemByNum(M3_NUM)) // Bots sometimes get aim snap
				turn_speed = 0.5;
			else if (rand() % 2 == 0 && self->client->weapon == FindItemByNum(SNIPER_NUM)) // Bots sometimes get aim snap
				turn_speed = 0.5;
			else if (rand() % 10 == 0) // Bots sometimes get aim snap
				turn_speed = 0.5;
		}

		//Com_Printf("%s %s turn_speed[%f]\n", __func__, self->client->pers.netname, turn_speed);

		//if (self->client->weaponstate != WEAPON_READY)
		//	not_infront = rand() % 2;

		if (not_infront) // Just 'pan' left and right (don't pitch up and down until enemy is in view)
		{
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, false);
		}
		else
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, turn_speed, true, true);
	}
	/*
	// When at a POI, get a direction to look at by finding a node NODE_POI_LOOKAT link
	else if (self->bot.current_node != INVALID && self->bot.next_node != INVALID && nodes[self->bot.current_node].type == NODE_POI)
	{
		// See if this node has any POI lookat links
		int num_lookat_nodes = 0;
		int lookat_nodes[MAXLINKS] = { 0 };
		for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)
		{
			if (nodes[self->bot.current_node].links[i].targetNodeType == NODE_POI_LOOKAT)
			{
				lookat_nodes[num_lookat_nodes] = nodes[self->bot.current_node].links[i].targetNode;
				num_lookat_nodes++;
			}
		}
		if (num_lookat_nodes) // One or more POI lookat nodes found
		{
			// When the timer is up pick a new node to look at
			if (self->bot.node_poi_look_time < level.framenum)
			{
				self->bot.node_poi_look_time = level.framenum + 5 * HZ;
				self->bot.node_poi_lookat_node = lookat_nodes[rand() % num_lookat_nodes]; // Pick a random POI lookat node
			}

			// Look at POI lookat node
			VectorSubtract(nodes[self->bot.node_poi_lookat_node].origin, self->s.origin, self->bot.bi.look_at);
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 2.0);
		}
	}
	*/
	else if (0 && self->bot.current_node != INVALID && nodes[self->bot.current_node].num_links)
	{
		//for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)
		{
		}
		//rand() % nodes[self->bot.current_node].num_links + 1;
		Com_Printf("%s found_viable_node [%d]\n", __func__, level.framenum);

		VectorSubtract(nodes[rand() % nodes[self->bot.current_node].num_links + 1].origin, self->s.origin, self->bot.bi.look_at);
		BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 1.0, true, true);
	}
	// If no enemy, have bot look at PNOISES, if any.
	else if (1)
	{
		int type = 0;
		int player_num = INVALID;
		float nearest = 9999999;
		qboolean found_target = false;
		for (int i = 0; i < num_players; i++)
		{
			if (players[i] != self && OnSameTeam(self, players[i]) == false)
			{
				if (botlib_noises.weapon_time[i] > 0 && BOTLIB_MovingToward(self, botlib_noises.weapon_origin[i], 0.3))
				{
					//Com_Printf("%s %s look at weapon_origin [%d]\n", __func__, self->client->pers.netname, botlib_noises.weapon_time[i]);
					float dist = VectorDistance(botlib_noises.weapon_origin[i], self->s.origin);
					if (dist < nearest && dist > 256)
					{
						type = PNOISE_WEAPON;
						nearest = dist;
						player_num = i;
					}
				}
				if (botlib_noises.self_time[i] > 0 && BOTLIB_MovingToward(self, botlib_noises.self_origin[i], 0.3))
				{
					float dist = VectorDistance(botlib_noises.self_origin[i], self->s.origin);
					if (dist < nearest && dist > 256)
					{
						type = PNOISE_SELF;
						nearest = dist;
						player_num = i;
					}
				}
			}
		}
		if (player_num == INVALID)
		{
			for (int i = 0; i < num_players; i++)
			{
				if (players[i] != self && OnSameTeam(self, players[i]) == false)
				{
					if (botlib_noises.impact_time[i] > 0 && BOTLIB_MovingToward(self, botlib_noises.impact_origin[i], 0.3))
					{
						float dist = VectorDistance(botlib_noises.impact_origin[i], self->s.origin);
						if (dist < nearest && dist > 256)
						{
							type = PNOISE_WEAPON;
							nearest = dist;
							player_num = i;
						}
					}
				}
			}
		}

		if (player_num != INVALID)
		{
			vec3_t noise_origin;

			if (type == PNOISE_WEAPON)
				VectorCopy(botlib_noises.weapon_origin[player_num], noise_origin);
			else if (type == PNOISE_SELF)
				VectorCopy(botlib_noises.self_origin[player_num], noise_origin);
			else if (type == PNOISE_IMPACT)
				VectorCopy(botlib_noises.impact_origin[player_num], noise_origin);


			// Test to see if bot is looking at a wall, if so goto LookAhead, otherwise keep tracking noise
			vec3_t eyes;
			VectorCopy(self->s.origin, eyes);
			eyes[2] += self->viewheight; // Get our eye level (standing and crouching)
			vec3_t noise_origin_eyes;
			VectorCopy(noise_origin, noise_origin_eyes);
			noise_origin_eyes[2] += self->viewheight; // Set noise origin to eye level
			trace_t tr = gi.trace(eyes, NULL, NULL, noise_origin_eyes, self, MASK_SHOT); // Trace to target
			float distance = VectorDistance(self->s.origin, tr.endpos);
			if (distance > 256)
			{
				VectorSubtract(noise_origin_eyes, self->s.origin, self->bot.bi.look_at);
				BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 1.0, true, true);
			}
			else
				goto LookAhead;
		}
		else
			goto LookAhead;
	}
	// If no enemy, look at nodes in front/behind, or look at map center
	else // if (self->enemy == NULL)
	{

	LookAhead:

		const int look_ahead = 5;
		qboolean found_viable_node = false;
		for (int i = self->bot.node_list_current; i < self->bot.node_list_count; i++) // Loop from current node until end of list
		{
			/*
			if ((i + look_ahead) < self->bot.node_list_count && self->bot.node_list[i + look_ahead] != INVALID)
			{
				int look_node = self->bot.node_list[(i + look_ahead)];
				if (nodes[look_node].num_links)
				{
					int rnd_link = rand() % nodes[look_node].num_links + 1;
					int rnd_node = nodes[look_node].links[rnd_link].targetNode;
					if (rnd_node != INVALID)
					{
						VectorSubtract(nodes[rnd_node].origin, self->s.origin, self->bot.bi.look_at);
						BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 0.1);
						found_viable_node = true;
						break;
					}
				}
			}
			*/

			// Look x nodes ahead
			if ((i + look_ahead) < self->bot.node_list_count && self->bot.node_list[i + look_ahead] != INVALID)
			{
				VectorSubtract(nodes[self->bot.node_list[(i + look_ahead)]].origin, self->s.origin, self->bot.bi.look_at);
				BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 1.0, true, true);
				found_viable_node = true;
				break;
			}
			// Look x nodes behind
			else if ((i - look_ahead) < self->bot.node_list_count && self->bot.node_list[(i - look_ahead)] != INVALID)
			{
				VectorSubtract(nodes[self->bot.node_list[(i - look_ahead)]].origin, self->s.origin, self->bot.bi.look_at);
				BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 1.0, true, true);
				found_viable_node = true;
				break;
			}
			else
				break;
		}
		//if (found_viable_node)
		//	Com_Printf("%s found_viable_node [%d]\n", __func__, level.framenum);
		//else
		//	Com_Printf("%s no node [%d]\n", __func__, level.framenum);

		if (0 && found_viable_node == false && self->bot.next_node != INVALID && nodes[self->bot.next_node].num_links)
		{
			//for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)
			{
			}
			//rand() % nodes[self->bot.current_node].num_links + 1;
			Com_Printf("%s found_viable_node [%d]\n", __func__, level.framenum);

			VectorSubtract(nodes[rand() % nodes[self->bot.current_node].num_links + 1].origin, self->s.origin, self->bot.bi.look_at);
			BOTLIB_ChangeBotAngleYawPitch(self, self->bot.bi.look_at, false, 1.0, true, true);
		}

		if (0 && found_viable_node == false)
		{
			// If no enemy, look at random thing
			{
				// If no enemy, look at random item or map center
				// Search through map looking for a random item, or NULL
				//if (self->bot.bi.look_at_ent == NULL || self->bot.bi.look_at_ent_time < level.framenum)
				if (self->bot.bi.look_at_time < level.framenum)
				{
					self->bot.bi.look_at_time = level.framenum + 1 * HZ; // Change target every 5 seconds

					edict_t* map_ents;
					int edict_num = 1 + game.maxclients; // skip worldclass & players
					for (map_ents = g_edicts + edict_num; map_ents < &g_edicts[globals.num_edicts]; map_ents++)
					{
						// Skip ents that are not in used and not an item
						if (map_ents->inuse == false || map_ents->item == NULL)
							continue;

						//trace_t tr = gi.trace(self->s.origin, NULL, NULL, map_ents->s.origin, self, MASK_SOLID);
						//if (tr.fraction == 1.0)
						{
							VectorCopy(map_ents->s.origin, self->bot.bi.look_at);
							break;
						}
					}
				}
				if (VectorEmpty(self->bot.bi.look_at))
					VectorSubtract(vec3_origin, self->s.origin, lookdir); // Look at center of map
				else
					VectorSubtract(self->bot.bi.look_at, self->s.origin, lookdir); // Look at random map item

			}

			// If lookdir is empty, then look at map center
			if (VectorEmpty(self->bot.bi.look_at))
			{
				VectorSubtract(vec3_origin, self->s.origin, lookdir);
			}

			BOTLIB_ChangeBotAngleYawPitch(self, lookdir, false, 1.0, true, true);
		}
	}
#endif
	
	// This only applies for teamplay
	
	// Do not follow path when teammates are still inside us.
	if (OnTransparentList(self)) { // Teamplay
		// If the bot has just spawned, then we need to wait a bit before we start moving.
		if (self->just_spawned) // set by SpawnPlayers() in a_team.c
		{
			self->just_spawned = false;
			self->just_spawned_go = true; // Bot is ready, when wander_timeout is reached.

			// If enemy is in sight, don't wait too long
			if (self->enemy) {
				self->just_spawned_timeout = level.framenum + random() * HZ;			// Short wait
			} else if (!bot_personality->value) {
				// Otherwise pick from various wait times before moving out
				int rnd_rng = rand() % 4;
				if (rnd_rng == 0)
					self->just_spawned_timeout = level.framenum + (random() * 10) * HZ;	// Long wait
				else if (rnd_rng == 1)
					self->just_spawned_timeout = level.framenum + (random() * 5) * HZ;	// Medium wait
				else if (rnd_rng == 2)
					self->just_spawned_timeout = level.framenum + (random() * 2) * HZ;  // Short wait
				else
					self->just_spawned_timeout = 0;										// No wait
			} else { // bot_personality is enabled, let's make it more realistic
				int rnd_rng = rand() % 4;
				float skill_factor = (self->bot.skill.movement / 10.0f); // Scale factor based on skill (0 to 1)

				if (rnd_rng == 0)
					self->just_spawned_timeout = level.framenum + (random() * 10 * (1.0f - skill_factor)) * HZ; // Long wait
				else if (rnd_rng == 1)
					self->just_spawned_timeout = level.framenum + (random() * 5 * (1.0f - skill_factor)) * HZ;  // Medium wait
				else if (rnd_rng == 2)
					self->just_spawned_timeout = level.framenum + (random() * 2 * (1.0f - skill_factor)) * HZ;  // Short wait
				else
					self->just_spawned_timeout = 0; // No wait
			}

			self->bot.bi.actionflags |= ACTION_HOLDPOS;
			return;
		}
		// Wait
		if (self->just_spawned_go && self->just_spawned_timeout > level.framenum && self->bot.see_enemies == false)
		{
			self->bot.bi.actionflags |= ACTION_HOLDPOS;
			return; // It's not time to move yet, wait!
		}
		// Go!
		if (self->just_spawned_go || self->bot.see_enemies)
		{
			//BOTLIB_PickLongRangeGoal(self);
			self->just_spawned_go = false; // Now we can move!
		}
	}


	if (self->groundentity && self->bot.next_node == INVALID) // No next node, pick new nav
	{
		//Com_Printf("%s %s next_node is invalid; find a new path\n", __func__, self->client->pers.netname);
		self->bot.bi.speed = 0;
		self->bot.state = BOT_MOVE_STATE_NAV;
		return;
	}
	// On ground and current or next node is invalid
	//if (self->groundentity && (self->bot.current_node == INVALID || self->bot.next_node == INVALID))
	//self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL); // Update current node
	if (self->groundentity && self->bot.current_node == INVALID)
	{
		//self->bot.state = BOT_MOVE_STATE_NAV;
		//self->bot.bi.speed = 0;
		//return;

		//Com_Printf("%s %s on ground and current_node (%i) is invalid; try to wander\n", __func__, self->client->pers.netname, self->bot.current_node);
		self->bot.bi.speed = 0;
		self->bot.stuck_wander_time = 1;
	}
	if (self->groundentity && self->bot.goal_node == INVALID)// && self->bot.node_travel_time >= 15)
	{
		//Com_Printf("%s %s on ground and goal_node (%i) is invalid; find a new path\n", __func__, self->client->pers.netname, self->bot.goal_node);
		self->bot.bi.speed = 0;
		self->bot.state = BOT_MOVE_STATE_NAV;
		return;
	}



	/*
	// If next node is INVALID, assume we're stuck
	//if (self->bot.next_node == INVALID || self->bot.current_node == INVALID || self->bot.goal_node == INVALID)
	if (self->bot.goal_node == INVALID)
	{
		self->bot.stuck_wander_time = 2;
		//Com_Printf("%s %s next_node:%d current_node:%d goal_node:%d\n", __func__, self->client->pers.netname, self->bot.next_node, self->bot.current_node, self->bot.goal_node);
	}
	else */
		next_node = self->bot.next_node;

	/*
	if (self->bot.node_travel_time > 30)
	{
		self->bot.node_travel_time = 0;

		// Wander
		self->bot.state = STATE_WANDER;
		self->wander_timeout = level.framenum + 1.0 * HZ;
		self->bot.goal_node = INVALID;
	}
	*/

	//if (self->bot.goal_node != INVALID && nav_area.total_areas > 0 && self->bot.node_travel_time >= 120)
	{
		//Com_Printf("%s %s ATTEMPTING TO FIX cur[%d] nxt[%d] goal[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
		{
			//self->bot.node_travel_time = 0;
			//Com_Printf("%s %s FIXED node_travel_time cur[%d] nxt[%d] goal[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
		}
	}

	// If travel time took too long, assume we're stuck
	if (self->bot.node_travel_time >= 120) //60 // Bot failure to reach next node
	{
		self->bot.stuck_wander_time = 1;
		//Com_Printf("%s %s node_travel_time was hit! cur[%d] nxt[%d] goal[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
	}

	//self->bot.stuck_wander_time = 0;

	if (self->bot.stuck_wander_time)// && nav_area.total_areas <= 0)
	{
		self->bot.stuck_wander_time--;
		self->bot.node_travel_time = 0;

		//Com_Printf("%s %s stuck_wander cur[%d] nxt[%d] goal[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);

		//Com_Printf("%s %s stuck_wander BOTLIB_PickLongRangeGoal()\n", __func__, self->client->pers.netname);
		//BOTLIB_PickLongRangeGoal(self); // pick a new long range goal
		// Wander
		//if (nav_area.total_areas <= 0)
		{
			//Com_Printf("%s %s stuck_wander\n", __func__, self->client->pers.netname);
			self->bot.state = BOT_MOVE_STATE_NAV;
			self->bot.goal_node = INVALID;
		}
		//return;
		

		VectorCopy(self->bot.stuck_random_dir, walkdir);

		if (self->groundentity)
		{
			self->bot.stuck_last_negate = 0; // Reset if we touched ground
		}

		float fwd_distance = 32;
		tr = gi.trace(self->s.origin, NULL, NULL, tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 128), self, (MASK_PLAYERSOLID | MASK_OPAQUE));
		if (tr.plane.normal[2] < 0.99) // If on a slope that makes the player 'bounce' when moving down the slope
			fwd_distance = 128; // Extend the distance we check for a safe direction to move toward
		//Com_Printf("%s %s tr.plane.normal[%f] \n", __func__, self->client->pers.netname, tr.plane.normal[2]);

		qboolean can_move = BOTLIB_CanMoveInDirection(self, walkdir, fwd_distance, NODE_MAX_CROUCH_FALL_HEIGHT, false);
		if (can_move == false || VectorEmpty(self->bot.stuck_random_dir))
		{
			// Try to aquire a safe random direction to move toward
			qboolean success = false;
			for (int i = 0; i < 8; i++)
			{
				if (BOTLIB_CanMoveInDirection(self, walkdir, fwd_distance, NODE_MAX_CROUCH_FALL_HEIGHT, true) == false)
					continue;
				else
				{
					success = true;
					break;
				}
			}
			if (success)
			{
				VectorCopy(walkdir, self->bot.stuck_random_dir);
				// vect to angles
				vec3_t angles;
				vectoangles(walkdir, angles);
				//Com_Printf("%s %s found random direction [%f %f %f]\n", __func__, self->client->pers.netname, angles[0], angles[1], angles[2]);
			}
			else
			{
				//Com_Printf("%s %s ACTION_HOLDPOS N[%d] L[%d] \n", __func__, self->client->pers.netname, self->bot.stuck_last_negate, level.framenum);

				// Prevent bot from falling off a ledge by reversing its velocity, pulling it away from the ledge
				if (level.framenum > self->bot.stuck_last_negate)
				{
					// Only allow this to happen once every 60 seconds
					// It's reset after 60 seconds, death, or touching ground again
					self->bot.stuck_last_negate = level.framenum + 60 * HZ;

					//Com_Printf("%s %s stuck_last_negate N[%d] L[%d] \n", __func__, self->client->pers.netname, self->bot.stuck_last_negate, level.framenum);

					// Reverse the direction
					VectorNegate(self->velocity, self->velocity);
				}
				//self->bot.bi.actionflags |= ACTION_HOLDPOS; // Stop moving
				//BOTLIB_Crouch_Or_Jump(self, ucmd, walkdir); // Crouch or jump if needed
				//return;
			}
		}
		else
		{
			//Com_Printf("%s %s heading for random dir\n", __func__, self->client->pers.netname);
			//VectorCopy(walkdir, self->bot.stuck_random_dir);
			//VectorCopy(self->bot.stuck_random_dir, walkdir);
		}

		vec3_t hordir;
		VectorNormalize(walkdir);
		hordir[0] = walkdir[0];
		hordir[1] = walkdir[1];
		hordir[2] = 0;
		VectorNormalize(hordir);
		VectorCopy(hordir, self->bot.bi.dir);
		self->bot.bi.speed = move_speed; // Set our suggested speed

		int perform_action = BOTLIB_Crouch_Or_Jump(self, ucmd, walkdir); // Crouch or jump if needed
		if (perform_action == ACTION_NONE)
		{
			//self->bot.bi.actionflags &= ~ACTION_CROUCH; // Remove crouching
			//self->bot.bi.actionflags &= ~ACTION_JUMP; // Remove jumping
		}
		else if (perform_action == ACTION_JUMP)
		{
			self->bot.bi.actionflags |= ACTION_JUMP; // Add jumping
			self->bot.bi.actionflags &= ~ACTION_CROUCH; // Remove crouching
		}
		else if (perform_action == ACTION_CROUCH)
		{
			self->bot.bi.actionflags |= ACTION_CROUCH; // Add crouching
			self->bot.bi.actionflags &= ~ACTION_JUMP; // Remove jumping
		}
		
		return;
	}
	else
	{
		/*
		qboolean fetch_item = false;
		if (self->bot.get_item != NULL)
		{
			float item_dist = VectorDistance(self->bot.get_item->s.origin, self->s.origin);
			Com_Printf("%s %s is looking for item %s [%f] sf[0x%x] f[0x%x]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist, self->bot.get_item->spawnflags, self->bot.get_item->flags);

			// TP item not picked up
			// spawnflags = 0
			// flags = 0
			// 
			// TP item picked up and in bot inventory
			// spawnflags = ITEM_TARGETS_USED
			// flags = FL_RESPAWN
			//
			// TP item picked up then dropped by bot
			// spawnflags = DROPPED_ITEM
			// flags = 0
			// 
			// TP item picked up, bot killed, dropping all items to ground
			// spawnflags = DROPPED_PLAYER_ITEM
			// flags = 0

			if (item_dist <= 128)
			{
				Com_Printf("%s %s is near item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);
				VectorSubtract(self->bot.get_item->s.origin, self->s.origin, walkdir); // Head to next node
				VectorNormalize(walkdir);
				fetch_item = true;

				if (item_dist <= 16)
				{
					Com_Printf("%s %s grabbed item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);
					self->bot.get_item = NULL;
				}
			}
			//else if (item_dist <= 64)
			{
				//Com_Printf("%s %s grabbed item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);
				//self->bot.get_item = NULL;
			}
		}
		*/
		//if (fetch_item == false)
		{
			if (next_node == INVALID)
			{
				//self->bot.stuck_wander_time = 1;
				return;
			}
			VectorSubtract(nodes[next_node].origin, self->s.origin, walkdir); // Head to next node
			VectorNormalize(walkdir);
			//VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, walkdir); // Head to next node
		}
	}

	//qboolean can_move = BOTLIB_CanMoveInDirection(self, walkdir, 32, NODE_MAX_CROUCH_FALL_HEIGHT, false);
	//if (can_move == false)
	//	self->bot.stuck_wander_time = 1;

	if (self->bot.stuck_wander_time)
		return;


	//if (self->bot.state == STATE_WANDER || self->bot.goal_node == INVALID)
	//	Com_Printf("%s %s [%d] INVALID\n", __func__, self->client->pers.netname, level.framenum);

	//qboolean perform = BOTLIB_Crouch_Or_Jump(self, ucmd, walkdir); // Crouch or jump if needed
	/*
	if (perform == false && moved == false)
	{
		//self->bot.bi.actionflags |= (ACTION_JUMP); // Try jumping anyway
		//self->bot.stuck_wander_time = 1;
	}
	*/

	/*
	//horizontal direction
	vec3_t hordir;
	hordir[0] = walkdir[0];
	hordir[1] = walkdir[1];
	hordir[2] = 0;
	VectorNormalize(hordir);
	VectorCopy(hordir, self->bot.bi.dir);
	*/

	// Strafe to get back on track
	if (0 && self->bot.next_node != INVALID)
	{
		// Get distance from bot to next node
		//float dist = VectorDistance(self->s.origin, nodes[self->bot.next_node].origin);
		// Get distanced from current node to next node
		//float dist2 = VectorDistance(nodes[self->bot.current_node].origin, nodes[self->bot.next_node].origin);

		byte mov_strafe = 0;
		float dot = BOTLIB_DirectionCheck(self, &mov_strafe);
		if (dot > 0.7 && dot < 0.99) // .995
		{
			//float dist = VectorDistance(self->s.origin, nodes[self->bot.current_node].origin);
			//if (dist > 64)
			{
				BOTLIB_UTIL_NEAREST_PATH_POINT(self, ucmd); // Update the nearest path point
				VectorSubtract(self->nearest_path_point, self->s.origin, walkdir); // Head to current node
				VectorNormalize(walkdir);
				vec3_t hordir;
				hordir[0] += walkdir[0];
				hordir[1] += walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);
			}



			/*
			//move_speed = SPEED_ROAM; // Slow down if we're moving in the right direction

			//BOTLIB_UTIL_NEAREST_PATH_POINT(self, ucmd); // Update the nearest path point
			float mov_strafe = BOTLIB_UTIL_PATH_DEVIATION(self, ucmd);

			if (mov_strafe > 0)
			{
				//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
				// Get the direction perpendicular to the dir, facing left
				vec3_t right;
				right[0] = walkdir[0];
				right[1] = -walkdir[0];
				right[2] = 0;
				VectorNormalize(right);
				VectorCopy(right, self->bot.bi.dir);
				move_speed = SPEED_ROAM; // Slow down if we're moving in the right direction
			}
			else if (mov_strafe < 0)
			{
				//Com_Printf("%s %s [s %f]\n", __func__, self->client->pers.netname, mov_strafe);
				// Get the direction perpendicular to the dir, facing left
				vec3_t left;
				left[0] = -walkdir[1];
				left[1] = walkdir[1];
				left[2] = 0;
				VectorNormalize(left);
				VectorCopy(left, self->bot.bi.dir);
				move_speed = SPEED_ROAM; // Slow down if we're moving in the right direction
			}
			else
			{
				move_speed = SPEED_RUN; // Slow down if we're moving in the right direction
			}
			*/
		}
	}

	//self->bot.bi.speed = move_speed; // Set our suggested speed

	if (0)
	{
		// Get current direction
		vec3_t angle, forward, right, start, end, origin, offset;
		vectoangles(walkdir, angle);
		AngleVectors(angle, forward, right, NULL);
		VectorCopy(self->s.origin, origin);
		//origin[2] -= 24; // From the ground up
		origin[2] += 8; // [Origin 24 units] + [8 units] == 32 units heigh (same as node height)

		VectorSet(offset, 0, 0, 0); // changed from 18,0,0
		G_ProjectSource(origin, offset, forward, right, start);
		offset[0] += 1024; // Distance forward dir
		G_ProjectSource(origin, offset, forward, right, end);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(self->s.origin, MULTICAST_PHS);
	}

	
	if (1 & self->bot.node_list_count)
	//if (self->bot.node_list_count > 0)
	{
		for (int i = 1; i < self->bot.node_list_count; i++)
		{
			if (self->bot.next_node == self->bot.node_list[i])
			{
				self->bot.current_node = self->bot.node_list[i - 1];
				break;
			}
		}
		if (self->bot.node_list_count)
		{
			//self->bot.current_node = self->bot.node_list[self->bot.node_list_current];
		}
	}

	// Get current and next node types
	int next_node_type = INVALID;
	int current_node_type = INVALID;
	if (self->bot.current_node != INVALID && self->bot.next_node != INVALID)
	{
		current_node_type = nodes[self->bot.current_node].type;

		for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)   //for (i = 0; i < MAXLINKS; i++)
		{
			int target_node = nodes[self->bot.current_node].links[i].targetNode;
			if (target_node == self->bot.next_node)
			{
				next_node_type = nodes[self->bot.current_node].links[i].targetNodeType; // Next node type

				//self->prev_to_curr_node_type = self->curr_to_next_node_type; // Previous node type
				//self->curr_to_next_node_type = nodes[self->bot.current_node].links[i].targetNodeType; // Next node type
				break;
			}

			/*
			// Try search surrounding nodes to see if they contain next_node
			for (int j = 0; j < nodes[target_node].num_links; j++)
			{
				int target_target_node = nodes[target_node].links[j].targetNode;
				for (int k = 0; k < nodes[target_target_node].num_links; k++)
				{
					int target_target_target_node = nodes[target_target_node].links[k].targetNode;
					if (target_target_target_node == self->bot.next_node)
					{
						next_node_type = nodes[target_target_target_node].links[i].targetNodeType; // Next node type
						Com_Printf("%s %s using target_target_target_node\n", __func__, self->client->pers.netname);
						break;
					}
				}
			}
			*/
		}
	}

	//if (current_node_type == INVALID || next_node_type == INVALID)
	if (next_node_type == INVALID)
	{
		//next_node_type = NODE_MOVE;
		//Com_Printf("%s %s invalid types node:curr/next[%d %d]  type:curr/next[%d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, current_node_type, next_node_type);

		if (self->bot.next_node == INVALID)
		{
			if(bot_debug->value)
				Com_Printf("%s %s invalid next_node node:curr/next/goal[%d %d %d]  type:curr/next[%d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node, current_node_type, next_node_type);
			self->bot.state = BOT_MOVE_STATE_NAV;
			return;
		}
		if (self->bot.current_node == self->bot.next_node && self->bot.current_node == self->bot.goal_node)
		{
			if(bot_debug->value)
				Com_Printf("%s %s reached goal node:curr/next/goal[%d %d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
			////if (nav_area.total_areas > 0)
			////	self->bot.state = BOT_MOVE_STATE_NAV_NEXT;
			////else
			self->bot.state = BOT_MOVE_STATE_NAV;
			return;
		}
		else if (self->bot.next_node != INVALID && VectorDistance(nodes[self->bot.next_node].origin, self->s.origin) <= 128)
		{
			/*
			// Try search surrounding nodes to see if they contain next_node
			qboolean resolved = false;
			for (int i = 0; i < nodes[self->bot.current_node].num_links; i++)   //for (i = 0; i < MAXLINKS; i++)
			{
				if (next_node_type != INVALID) break;
				int target_node = nodes[self->bot.current_node].links[i].targetNode;
				if (target_node == INVALID) continue;

				for (int j = 0; j < nodes[target_node].num_links; j++)
				{
					if (next_node_type != INVALID) break;
					int target_target_node = nodes[target_node].links[j].targetNode;
					if (target_target_node == INVALID) continue;

					for (int k = 0; k < nodes[target_target_node].num_links; k++)
					{
						int target_target_target_node = nodes[target_target_node].links[k].targetNode;
						if (target_target_target_node == INVALID) continue;

						if (target_target_target_node == self->bot.next_node)
						{
							if (VectorDistance(nodes[target_target_target_node].origin, self->s.origin) <= 128)
							{
								next_node_type = nodes[target_target_node].links[i].targetNodeType; // Next node type
								Com_Printf("%s %s resolved next node:curr/next/goal[%d %d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
								break;
							}
						}
					}
				}
			}
			*/

			//if (next_node_type != INVALID)
			//	Com_Printf("%s %s resolved next node:curr/next/goal[%d %d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
		}
		if (next_node_type == INVALID)
		{
			//Com_Printf("%s %s invalid types node:curr/next/goal[%d %d %d]  type:curr/next[%d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node, current_node_type, next_node_type);

			// Path failed, so try again
			//BOTLIB_CanVisitNode(self, self->bot.goal_node, false, nodes[self->bot.goal_node].area, true);

			/*
			if (self->bot.goal_node != INVALID)
				BOTLIB_CanGotoNode(self, self->bot.goal_node, false);
			else
				self->bot.state = BOT_MOVE_STATE_NAV;
			*/

			return;
		}

		//VectorCopy(nodes[self->bot.goal_node].origin, self->s.origin);

		//self->bot.state = STATE_WANDER;
		//self->wander_timeout = level.framenum + 1.0 * HZ;
		//self->bot.goal_node = INVALID;
		//return;
	}

	if (current_node_type != INVALID && next_node_type != INVALID)
	{
		// DEBUG: print current and next node types
		if (0)
		{
			const qboolean printOnlyErrors = false; // If only printing INVALID nodes
			PrintAllLinkNodeTypes(self, printOnlyErrors);
		}

		// Set the default movement: this will be overriden by the movement code below
		//if (next_node_type == NODE_MOVE || next_node_type == NODE_CROUCH || next_node_type == NODE_JUMP || next_node_type == NODE_BOXJUMP || next_node_type == NODE_LADDER)
		{
			//*
			//horizontal direction
			vec3_t hordir;
			hordir[0] = walkdir[0];
			hordir[1] = walkdir[1];
			hordir[2] = 0;
			VectorNormalize(hordir);
			VectorCopy(hordir, self->bot.bi.dir);
			//*/
			//VectorCopy(walkdir, self->bot.bi.dir);

			self->bot.bi.speed = move_speed; // Set our suggested speed
		}

		// Water!
		/// Borrowed from P_WorldEffects
		int waterlevel = self->waterlevel;
		int old_waterlevel = self->client->old_waterlevel;
		self->client->old_waterlevel = waterlevel;

		if (current_node_type == NODE_WATER || waterlevel == 3){
			//gi.dprintf("I'm in the water %s and my air is %d\n", self->client->pers.netname, self->air_finished_framenum - level.framenum);
			// Get out before you start drowning!
			if (self->air_finished_framenum < level.framenum + 10) {
				self->bot.bi.actionflags |= ACTION_MOVEUP;
				self->bot.node_travel_time = 0;
			}
		}

		// move, crouch, or jump
		if (next_node_type == NODE_WATER)
		{
			self->bot.bi.actionflags &= ~ACTION_MOVEUP;
			self->bot.bi.actionflags &= ~ACTION_MOVEDOWN;
			self->bot.bi.actionflags &= ~ACTION_CROUCH;

			//VectorClear(self->bot.bi.dir);
			//self->bot.bi.speed = 0;

			VectorNormalize(walkdir);
			VectorCopy(walkdir, self->bot.bi.dir);
			self->bot.bi.speed = move_speed; // Set our suggested speed

			// Check that we're in the water
			vec3_t temp = { 0,0,0 };
			VectorCopy(self->s.origin, temp);
			temp[2] = self->s.origin[2] - 8;
			int contents_feet = gi.pointcontents(temp);

			// Almost out of air, start heading up to the surface
			//if ((contents_feet & MASK_WATER) && self->air_finished_framenum < level.framenum + 5) // Move up to get air
			if (self->air_finished_framenum < level.framenum + 5) // Move up to get air
			{
				self->bot.bi.actionflags |= ACTION_MOVEUP;
				self->bot.node_travel_time = 0; // Ignore node travel time while we get some air
				//Com_Printf("%s %s [%d] water: get air\n", __func__, self->client->pers.netname, level.framenum);
			}
			else if ((contents_feet & MASK_WATER) && fabs(nodes[self->bot.next_node].origin[2] - self->s.origin[2]) <= 33) // Roughly leveled out
			{
				//Com_Printf("%s %s [%d] water: level\n", __func__, self->client->pers.netname, level.framenum);
			}
			else if ((contents_feet & MASK_WATER) && nodes[self->bot.next_node].origin[2] > self->s.origin[2]) // Move up
			{
				//self->bot.bi.actionflags |= ACTION_MOVEUP;
				// darksaint: changed this to MOVEUP and MOVEFORWARD simultanously to get out of water?
				self->bot.bi.actionflags |= (ACTION_MOVEUP | ACTION_MOVEFORWARD);
				//Com_Printf("%s %s [%d] water: move up\n", __func__, self->client->pers.netname, level.framenum);
			}
			else if (nodes[self->bot.next_node].origin[2] < self->s.origin[2]) // Move down
			{
				if (contents_feet & MASK_WATER)
					self->bot.bi.actionflags |= ACTION_MOVEDOWN; // In water moving down
				else
					self->bot.bi.actionflags |= ACTION_CROUCH; // Crouch drop down into water below

				//Com_Printf("%s %s [%d] water: move down\n", __func__, self->client->pers.netname, level.framenum);
			}
			
		}

		// Pull bot in when close to the next node, help guide it in
		if (0)
		{
			if (self->groundentity == false && level.framenum > self->bot.stuck_last_negate)
			{
				vec3_t bot_to_node;
				VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_node);
				bot_to_node[2] = 0;
				float xy_bot_to_next_dist = VectorLength(bot_to_node); // Distance from bot to next node
				if (xy_bot_to_next_dist > 32 && xy_bot_to_next_dist <= 150)
				{
					// Line of sight
					tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
					if (tr.fraction == 1.0)
					{
						self->bot.stuck_last_negate = level.framenum + 1 * HZ;
						vec3_t dir;
						dir[0] = walkdir[0];
						dir[1] = walkdir[1];
						dir[2] = walkdir[2];
						VectorNormalize(dir);
						self->velocity[0] += 50 * dir[0];
						self->velocity[1] += 50 * dir[1];
						self->velocity[2] += 300 * dir[2];
						// Limit velocity
						if (self->velocity[2] < -300)
							self->velocity[2] = -300;
						//Com_Printf("%s %s [%d] directing bot closer to next node\n", __func__, self->client->pers.netname, level.framenum);
					}
				}
			}
		}

		if (next_node_type == NODE_MOVE)
		{
			
			// if (BOTLIB_CanMoveDir(self, walkdir) == false)
			// {
			// 	// We can't move in this direction
			// 	Com_Printf("%s %s can't move safely in direction\n", __func__, self->client->pers.netname);
			// 	self->bot.stuck_wander_time = 15;
			// 	self->bot.bi.actionflags |= ACTION_HOLDPOS; // Stop moving
			// 	return;
			// }
			
			// float fwd_distance = 32;
			// tr = gi.trace(self->s.origin, NULL, NULL, tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 128), self, (MASK_PLAYERSOLID | MASK_OPAQUE));
			// if (tr.plane.normal[2] < 0.99) // If on a slope that makes the player 'bounce' when moving down the slope
			// 	fwd_distance = 128; // Extend the distance we check for a safe direction to move toward

			// // Prevent bot from falling off a ledge by reversing its velocity, pulling it away from the ledge
			// if (BOTLIB_CanMoveInDirection(self, walkdir, fwd_distance, NODE_MAX_CROUCH_FALL_HEIGHT, false) == false)
			// {
			// 	if (level.framenum > self->bot.stuck_last_negate)
			// 	{
			// 		// Only allow this to happen once every 60 seconds
			// 		// It's reset after 60 seconds, death, or touching ground again
			// 		self->bot.stuck_last_negate = level.framenum + 60 * HZ;

			// 		Com_Printf("%s %s stuck_last_negate N[%d] L[%d] \n", __func__, self->client->pers.netname, self->bot.stuck_last_negate, level.framenum);

			// 		// Reverse the direction
			// 		VectorNegate(self->velocity, self->velocity);
			// 		VectorClear(self->bot.bi.dir);
			// 	}
			// }
		}

		//if (next_node_type == NODE_JUMPPAD || next_node_type == NODE_JUMP)
		//	BOTLIB_Crouch_Or_Jump(self, ucmd, dir);

		if (next_node_type == NODE_BOXJUMP)
		{
			if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Going up
			{
				self->bot.bi.actionflags |= ACTION_BOXJUMP;
				self->bot.bi.actionflags &= ~ACTION_JUMP;
				self->bot.bi.actionflags &= ~ACTION_CROUCH;
			}
			else
			{
				// Remove flags
				self->bot.bi.actionflags &= ~ACTION_CROUCH;
				self->bot.bi.actionflags &= ~ACTION_BOXJUMP;
				self->bot.bi.actionflags &= ~ACTION_JUMP;
			}
		}

		// Slow down if we're not on the ground
		if (next_node_type == NODE_MOVE)
		{
			if (self->groundentity == NULL)
			{
				//self->bot.bi.speed = SPEED_CAREFUL; // Set our speed directly
				self->bot.bi.speed = SPEED_ROAM; // Set our speed directly
				//Com_Printf("%s %s SPEED_CAREFUL node_travel_time[%d]\n", __func__, self->client->pers.netname, self->bot.node_travel_time);
			}
			//if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2] + 32) // Going up
			//{
			//	self->bot.bi.actionflags |= ACTION_JUMP;
			//}
			else
			{
				// Remove flags
				//self->bot.bi.actionflags &= ~ACTION_CROUCH;
				//self->bot.bi.actionflags &= ~ACTION_JUMP;

				//if (nodes[self->bot.next_node].normal[2] > 0.7) // If the next node is flat
				//if (random() < 0.1)
				//	self->bot.bi.actionflags |= ACTION_BOXJUMP;
			}
		}
		if (next_node_type == NODE_CROUCH)
		{
			self->bot.bi.actionflags |= ACTION_CROUCH;
			
			// Remove flags
			self->bot.bi.actionflags &= ~ACTION_JUMP;
		}


		if (nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type == NODE_LADDER)
		{
			VectorCopy(walkdir, self->bot.bi.dir);
			self->bot.bi.speed = 100; // Set speed slower for ladders

			if (self->bot.touching_ladder)
			{
				// Remove flags
				self->bot.bi.actionflags &= ~ACTION_ATTACK; // Don't attack when on ladder

				if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Going up
				{
					self->bot.bi.actionflags |= ACTION_MOVEUP;
					self->bot.bi.actionflags &= ~ACTION_MOVEDOWN;
				}
				else if (nodes[self->bot.next_node].origin[2] < nodes[self->bot.current_node].origin[2]) // Going down
				{
					self->bot.bi.actionflags |= ACTION_MOVEDOWN;
					self->bot.bi.actionflags &= ~ACTION_MOVEUP;
				}
			}
			else if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Jump to ladder
			{
				self->bot.bi.actionflags |= ACTION_JUMP;
			}
		}
		/*
		//if (self->bot.prev_node != INVALID && nodes[self->bot.prev_node].type == NODE_LADDER && nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type != NODE_LADDER)
		if (touching_ladder == false && nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type != NODE_LADDER)
		{
			VectorCopy(walkdir, self->bot.bi.dir);
			self->bot.bi.speed = 400; // Set speed slower for ladders

			//if (touching_ladder)
			{
				self->bot.bi.actionflags |= ACTION_JUMP;

				self->bot.bi.actionflags &= ~ACTION_MOVEUP;
				self->bot.bi.actionflags &= ~ACTION_MOVEDOWN;
			}
		}
		*/
		// Make sure the bot gets off the top and bottom of the ladder
		if (self->bot.prev_node != INVALID && nodes[self->bot.prev_node].type == NODE_LADDER && nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type != NODE_LADDER && self->s.origin[2] < nodes[self->bot.current_node].origin[2] + 4 && self->groundentity == NULL) // 
		{
			if (nodes[self->bot.prev_node].origin[2] < nodes[self->bot.current_node].origin[2] && self->s.origin[2] < nodes[self->bot.current_node].origin[2] + 2) // Getting off ladder at the top
			{
				//Com_Printf("%s %s MOVE UP\n", __func__, self->client->pers.netname);
				//self->bot.bi.actionflags |= ACTION_JUMPPAD;
				self->bot.bi.actionflags |= ACTION_MOVEUP;
				//self->bot.bi.actionflags |= ACTION_BOXJUMP;
			}
			else if (nodes[self->bot.prev_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Getting off ladder at the bottom
			{
				//Com_Printf("%s %s MOVE DOWN\n", __func__, self->client->pers.netname);
				//self->bot.bi.actionflags |= ACTION_JUMPPAD;
				//self->bot.bi.actionflags |= ACTION_MOVEDOWN;
				//self->bot.bi.actionflags |= ACTION_BOXJUMP;
			}
		}
		//if (self->bot.prev_node != INVALID)
		{
			//Com_Printf("%s %s types[ %d %d %d ]\n", __func__, self->client->pers.netname, nodes[self->bot.prev_node].type, nodes[self->bot.current_node].type, nodes[self->bot.next_node].type);
		}

		if (next_node_type == NODE_JUMP && self->groundentity)
		{
			//Com_Printf("%s %s ACTION_JUMP\n", __func__, self->client->pers.netname);
			self->bot.bi.actionflags |= ACTION_JUMP;

			// Remove flags
			self->bot.bi.actionflags &= ~ACTION_CROUCH;
		}

		// Handle jumping
		if (next_node_type == NODE_JUMPPAD)
		{
			//Com_Printf("%s %s jump takeoff\n", __func__, self->client->pers.netname);

			// Remove flags
			self->bot.bi.actionflags &= ~ACTION_CROUCH;
			self->bot.bi.actionflags &= ~ACTION_JUMP;

			//self->bot.bi.actionflags = 0;
			//VectorClear(self->bot.bi.dir);

			// Bot only applies direction when it's in a falling state
			if (self->groundentity || self->velocity[2] > 0)
				self->bot.bi.speed = 0;
			else
			{
				//horizontal direction
				vec3_t hordir;
				hordir[0] = walkdir[0];
				hordir[1] = walkdir[1];
				hordir[2] = 0;
				VectorNormalize(hordir);
				VectorCopy(hordir, self->bot.bi.dir);

				self->bot.bi.speed = move_speed; // Set our suggested speed
			}
			self->bot.bi.actionflags |= ACTION_JUMPPAD;
			//BOTLIB_Jump_Takeoff(self, NULL, nodes[self->bot.next_node].origin, self->viewheight, self->velocity);

			//Com_Printf("%s %s NODE_JUMPPAD\n", __func__, self->client->pers.netname);
#if 0
			// Get distance from bot to node
			nodes[self->bot.current_node].origin;
			vec3_t bot_to_node;

			VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_node);
			bot_to_node[2] = 0;
			float xy_bot_to_next_dist = VectorLength(bot_to_node); // Distance from bot to next node

			VectorSubtract(nodes[self->bot.current_node].origin, self->s.origin, bot_to_node);
			bot_to_node[2] = 0;
			float xy_bot_to_curr_dist = VectorLength(bot_to_node); // Distance from bot to current node
			//if (xy_bot_to_curr_dist <= 32) // If close enough to jump pad


			float distance = 16;
			vec3_t bmins = { nodes[self->bot.current_node].absmin[0] + -(distance), nodes[self->bot.current_node].absmin[1] + -(distance), nodes[self->bot.current_node].absmin[2] + -(distance) };
			vec3_t bmaxs = { nodes[self->bot.current_node].absmax[0] + distance, nodes[self->bot.current_node].absmax[1] + distance, nodes[self->bot.current_node].absmax[2] + distance };
			if (BOTLIB_BoxIntersection(self->absmin, self->absmax, bmins, bmaxs))
			{
				Com_Printf("%s %s jump takeoff\n", __func__, self->client->pers.netname);
				self->bot.bi.actionflags |= ACTION_JUMPPAD;
				//BOTLIB_Jump_Takeoff(self, NULL, nodes[self->bot.next_node].origin, self->viewheight, self->velocity);

				// Trace up to see if we're going to hit our head
				tr = gi.trace(self->s.origin, tv(-16, -16, -0), tv(16, 16, 32), tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] + 60), self, MASK_PLAYERSOLID);
				if (tr.fraction == 1.0)
				{
					// Trace down to see if we hit the ground
					tr = gi.trace(self->s.origin, tv(-32, -32, -0), tv(32, 32, 0), tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 60), self, MASK_PLAYERSOLID);
					qboolean is_player = false;
					if (tr.ent && tr.ent->client)
						is_player = true;

					/*
					// If landing on flat ground, only allow jumping when once we touch the ground
					// Otherwise if its a slope, allow jumping when we're close'ish to the ground
					qboolean can_jump = true;
					//if (tr.plane.normal[2] == 1 && self->groundentity == NULL && self->velocity[2] < 0) // flat ground
					if (self->groundentity == NULL && self->velocity[2] < 0) // flat ground
					{
						can_jump = false;
						self->bot.bi.actionflags |= ACTION_HOLDPOS; // Stop moving
						Com_Printf("%s %s ACTION_HOLDPOS\n", __func__, self->client->pers.netname);
					}
					*/

					if ((tr.fraction < 1 || tr.startsolid) && is_player == false) // && can_jump)
					{
						//float height_diff = nodes[self->bot.next_node].origin[2] - self->s.origin[2]; // Height difference between bot and next node
						//if (xy_bot_to_next_dist <= 32 && height_diff <= 60)
						{
							// Bot jumped
							//Com_Printf("%s %s jumped\n", __func__, self->client->pers.netname);
							//self->bot.bi.actionflags |= ACTION_JUMP;
						}
						//else
						{
							// Trace from bot to node to see if we can jump to it
							//tr = gi.trace(self->s.origin, tv(-24, -24, -STEPSIZE), tv(24, 24, 96), nodes[self->bot.current_node].origin, self, MASK_PLAYERSOLID);
							//if (tr.fraction == 1.0)
							{
								//Com_Printf("%s %s jump takeoff\n", __func__, self->client->pers.netname);
								//self->bot.bi.actionflags |= ACTION_JUMPPAD; // Stop moving
							}
						}
					}
				}
			}
#endif
			//if (moved == false) 
		}
	}

	if (ctf->value)
	{
		// Slow down if near flag
		if (bot_ctf_status.flag1_curr_node != INVALID && VectorDistance(nodes[bot_ctf_status.flag1_curr_node].origin, self->s.origin) < 128)
		{
			self->bot.bi.speed = 200;
		}
		if (bot_ctf_status.flag2_curr_node != INVALID && VectorDistance(nodes[bot_ctf_status.flag2_curr_node].origin, self->s.origin) < 128)
		{
			self->bot.bi.speed = 200;
		}
	}

	if (self->bot.see_enemies)
	{
		qboolean dodging = ((rand() % 10) < 7);

		// Don't dodge if < x links on node - low link count might indicate a tight walk or narrow way
		if (nodes[self->bot.current_node].num_links < 4 || nodes[self->bot.next_node].num_links < 4)
			dodging = false;

		// Don't dodge if bot is taking a direct path
		if (self->bot.node_random_path == false)
			dodging = false;

		if (ctf->value) // Reduce dodging in CTF
		{
			//float f1 = BOTLIB_DistanceToFlag(self, FLAG_T1_NUM);
			//float f2 = BOTLIB_DistanceToFlag(self, FLAG_T2_NUM);

			float f1 = 99999999;
			float f2 = 99999999;
			if (bot_ctf_status.flag1_curr_node != INVALID)
				f1 = VectorDistance(nodes[bot_ctf_status.flag1_curr_node].origin, self->s.origin);
			if (bot_ctf_status.flag2_curr_node != INVALID)
				f2 = VectorDistance(nodes[bot_ctf_status.flag2_curr_node].origin, self->s.origin);

			if (f1 < 1500 || f2 < 1500 || BOTLIB_Carrying_Flag(self)) // || self->bot.goal_node == bot_ctf_status.flag1_curr_node || self->bot.goal_node == bot_ctf_status.flag2_curr_node)
			{
				dodging = false;
				//Com_Printf("%s %s dodging is OFF ------ \n", __func__, self->client->pers.netname);
			}

			//dodging = false;
		}

		// Try strafing around enemy
		trace_t tr = gi.trace(self->s.origin, NULL, NULL, tv(self->s.origin[0], self->s.origin[1], self->s.origin[2] - 32), self, MASK_SHOT);
		gitem_t* clweapon = self->client->weapon;
		
		if (dodging && tr.plane.normal[2] > 0.85) // Not too steep
		{
			// Try strafing continually in a general direction, if possible
			static int max_strafe_left = -10;
			static int max_strafe_right = 10;
			if (self->bot_strafe == 0) // Pick new direction
			{
				float strafe_choice = random() < 0.333;
				if (strafe_choice < 0.33) // Left
					self->bot_strafe = -1;
				else if (strafe_choice < 0.66) // Right
					self->bot_strafe = 1;
				else
					self->bot_strafe = 0; // Neither - skip strafing this turn
			}

			if (clweapon == FindItem(HC_NAME))
				self->bot_strafe = 0; // Don't strafe with HC, just go straight for them

			if (self->bot_strafe < 0 && random() > 0.15) // Going left 85% of the time
			{
				if (BOTLIB_CanMove(self, MOVE_LEFT) && self->bot_strafe >= max_strafe_left) // Can go left with a limit
				{
					//Com_Printf("%s %s strafe [LEFT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
					self->bot_strafe--;
					self->bot.bi.actionflags |= ACTION_MOVELEFT;
				}
				else if (BOTLIB_CanMove(self, MOVE_RIGHT)) // Cannot go left anymore, so try going right
				{
					self->bot_strafe = 1; // Go right
					self->bot.bi.actionflags |= ACTION_MOVERIGHT;
					//Com_Printf("%s %s strafe [RIGHT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
				}
				else
					self->bot_strafe = 0; // Could not go either direction, so skip strafing this turn and reset back to random choice
			}
			else if (self->bot_strafe > 0 && random() > 0.15) // Going right 85% of the time
			{
				if (BOTLIB_CanMove(self, MOVE_RIGHT) && self->bot_strafe <= max_strafe_right) // Can go right with a limit
				{
					//Com_Printf("%s %s strafe [RIGHT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
					self->bot_strafe++;
					self->bot.bi.actionflags |= ACTION_MOVERIGHT;
				}
				else if (BOTLIB_CanMove(self, MOVE_LEFT)) // Cannot go right anymore, so try going left
				{
					self->bot_strafe = -1; // Go left
					self->bot.bi.actionflags |= ACTION_MOVELEFT;
					//Com_Printf("%s %s strafe [LEFT] %d\n", __func__, self->client->pers.netname, self->bot_strafe);
				}
				else
					self->bot_strafe = 0; // Could not go either direction, so skip strafing this turn and reset back to random choice
			}
			else
				self->bot_strafe = 0; // Skip strafing this turn

			// Back off if getting too close (unless we have a HC or knife)
			if (self->bot.enemy_dist < 256)
			{
				if (clweapon == FindItem(HC_NAME) && self->client->cannon_rds)
				{
					// Come in close for the kill
					if (ACEMV_CanMove(self, MOVE_FORWARD))
						self->bot.bi.actionflags |= ACTION_MOVEFORWARD;
				}
				else if (clweapon == FindItem(KNIFE_NAME))
				{
					// Come in close for the kill
					if (ACEMV_CanMove(self, MOVE_FORWARD))
						self->bot.bi.actionflags |= ACTION_MOVEFORWARD;
				// Try move backwards
				} else if (BOTLIB_CanMove(self, MOVE_BACK))
				{
					self->bot.bi.actionflags |= ACTION_MOVEBACK;
				}

				if ((clweapon == FindItem(M4_NAME) || 
					clweapon == FindItem(MP5_NAME) || 
					clweapon == FindItem(DUAL_NAME) || 
					clweapon == FindItem(MK23_NAME)) &&
					INV_AMMO(self, LASER_NUM) == false) {
						BOTLIB_CrouchFire(self); // Utilize crouching for better accuracy
				}
			}
			// If distance is far, consider crouching to increase accuracy
			else if (self->bot.enemy_dist > 1024)
			{
				// Check if bot should be crouching based on weapon and if strafing
				if (INV_AMMO(self, LASER_NUM) == false && 
					clweapon != FindItem(SNIPER_NAME) && 
					clweapon != FindItem(HC_NAME) && 
					clweapon != FindItem(M3_NAME) && 
					(self->bot.bi.actionflags & ACTION_MOVELEFT) == 0 && 
					(self->bot.bi.actionflags & ACTION_MOVERIGHT) == 0)
				{
					// Raptor007: Don't crouch if it blocks the shot.
					float old_z = self->s.origin[2];
					self->s.origin[2] -= 14;
					if (ACEAI_CheckShot(self))
					{
						self->bot.bi.actionflags |= ACTION_CROUCH;
						//Com_Printf("%s %s crouch shooting\n", __func__, self->client->pers.netname);
					}
					self->s.origin[2] = old_z;
				}
			}
			else
			{
				// Keep distance with sniper
				if (self->bot.enemy_dist < 1024 && clweapon == FindItemByNum(SNIPER_NUM) && BOTLIB_CanMove(self, MOVE_BACK))
				{
					self->bot.bi.actionflags |= ACTION_MOVEBACK;
				}
				// Keep distance with grenade
				if (self->bot.enemy_dist < 1024 && clweapon == FindItemByNum(GRENADE_NUM) && BOTLIB_CanMove(self, MOVE_BACK))
				{
					self->bot.bi.actionflags |= ACTION_MOVEBACK;
				}
				// Otherwise move toward target
				//else if (ACEMV_CanMove(self, MOVE_FORWARD))
				//	self->bot.bi.actionflags |= ACTION_MOVEFORWARD;
			}

			// If the bot is dodging by strafing, add in some random jumps
			if (self->bot_strafe != 0 && (self->bot.bi.actionflags & ACTION_CROUCH) == 0 && random() < 0.2)
				self->bot.bi.actionflags |= ACTION_BOXJUMP; // Jump while dodging
		}
	}
	
	int perform_action = BOTLIB_Crouch_Or_Jump(self, ucmd, walkdir); // Crouch or jump if needed
	if (perform_action == ACTION_JUMP)
	{
		self->bot.bi.actionflags |= ACTION_JUMP; // Add jumping
		self->bot.bi.actionflags &= ~ACTION_CROUCH; // Remove crouching
		self->bot.bi.actionflags &= ~ACTION_JUMPPAD; // Remove jumppad
	}
	else if (perform_action == ACTION_CROUCH)
	{
		self->bot.bi.actionflags |= ACTION_CROUCH; // Add crouching
		self->bot.bi.actionflags &= ~ACTION_JUMP; // Remove jumping
		self->bot.bi.actionflags &= ~ACTION_JUMPPAD; // Remove jumppad
	}

	// Stay on course by strafing back to the path line (if not already strafing)
	//if (VectorLength(self->velocity) < 37) // If stuck
	if (moved == false)
	{
		// Try strafing
		qboolean right_blocked = BOTLIB_TEST_FORWARD_VEC(self, walkdir, 64, 8); // Check right
		qboolean left_blocked = BOTLIB_TEST_FORWARD_VEC(self, walkdir, 64, -8); // Check left
		if (right_blocked && left_blocked) // Both are blocked
		{
			self->bot.bi.actionflags |= ACTION_JUMP; // Try jumping to clear the obstacle
			//Com_Printf( "%s %s blocked\n", __func__, self->client->pers.netname);


			/*
			// Bot stuck - Test of bot has LoS to next node
			if (self->bot.next_node == INVALID)
			{
				self->bot.stuck = true;
				return;
			}
			trace_t tr = gi.trace(self->s.origin, tv(-4, -4, 0.1), tv(4, 4, 56), nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
			if (tr.startsolid || tr.fraction < 1.0)
			{
				if (VectorLength(self->velocity) < 5)
					self->bot.stuck = true;
			}
			*/
			//self->bot.bi.actionflags |= ACTION_MOVEFORWARD;
			//self->bot.bi.viewangles[YAW] += 22.5 + (random() * 270);
			//self->bot.bi.viewangles[PITCH] = 0;
		}
		else if (right_blocked && BOTLIB_CanMove(self, MOVE_LEFT)) // Strafe left
		{
			// Get the direction perpendicular to the dir, facing left
			vec3_t left;
			left[0] = -walkdir[1];
			left[1] = walkdir[0];
			left[2] = 0;
			VectorNormalize(left);
			VectorCopy(left, self->bot.bi.dir);


			//Com_Printf("%s %s ACTION_MOVELEFT\n", __func__, self->client->pers.netname);
			//self->bot.bi.actionflags |= ACTION_MOVELEFT;
		}
		else if (left_blocked && BOTLIB_CanMove(self, MOVE_RIGHT)) // Strafe right
		{
			// Get the direction perpendicular to the dir, facing right
			vec3_t right;
			right[0] = walkdir[1];
			right[1] = -walkdir[0];
			right[2] = 0;
			VectorNormalize(right);
			VectorCopy(right, self->bot.bi.dir);

			//Com_Printf("%s %s ACTION_MOVERIGHT\n", __func__, self->client->pers.netname);
			//self->bot.bi.actionflags |= ACTION_MOVERIGHT;
		}
		else
		{
			//Com_Printf("%s %s moved == false and not blocked on side\n", __func__, self->client->pers.netname);
			//if (VectorLength(self->velocity) < 5)
			//	self->bot.stuck = true;
		}
	}

	/*
	// Project forward looking for walls
	vec3_t      start, end;
	vec3_t      forward, right;
	vec3_t      offset;
	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7, self->viewheight - 8);
	offset[1] = 0;
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	VectorMA(start, 48, forward, end);
	trace_t tr = gi.trace(start, NULL, NULL, end, self, MASK_PLAYERSOLID);

	// If we hit an obstacle or we've not moved much, turn around
	if (tr.fraction < 1.0 || VectorLength(self->velocity) < 37)
	{
		self->bot.bi.viewangles[YAW] += 22.5 + (random() * 270);
		self->bot.bi.viewangles[PITCH] = 0;
	}
	*/
}
//rekkie -- Quake3 -- e

////////////////////
// Wandering code //
////////////////////
//
// Basic wandering code, simply here as a backup to help the bot find a nearby node
//
void BOTLIB_MOV_Wander(edict_t* self, usercmd_t* ucmd)
{
#if 0
	vec3_t      start, end;
	vec3_t      forward, right;
	vec3_t      offset;

	// Do not move
	if (self->next_move_time > level.framenum)
		return;

	//Com_Printf("%s %s wandering...\n", __func__, self->client->pers.netname);

	// Special check for elevators, stand still until the ride comes to a complete stop.
	if (self->groundentity && (VectorLength(self->groundentity->velocity) >= 8))
	{
		// only move when platform not
		if (self->groundentity->moveinfo.state == STATE_UP || self->groundentity->moveinfo.state == STATE_DOWN)
		{
			self->velocity[0] = 0;
			self->velocity[1] = 0;
			self->velocity[2] = 0;
			self->next_move_time = level.framenum + 0.5 * HZ;
			return;
		}
	}

	// Contents check
	vec3_t temp = { 0,0,0 };
	VectorCopy(self->s.origin, temp);
	temp[2] += 22;
	int contents_head = gi.pointcontents(temp);
	temp[2] = self->s.origin[2] - 8;
	int contents_feet = gi.pointcontents(temp);

	// Just try to keep our head above water.
	if (contents_head & MASK_WATER)
	{
		// If drowning and no node, move up
		if (self->client->next_drown_framenum > 0)
		{
			ucmd->upmove = SPEED_RUN;
			self->s.angles[PITCH] = -45;
		}
		else
			ucmd->upmove = SPEED_WALK;
	}

	// Don't wade in lava, try to get out!
	if (contents_feet & (CONTENTS_LAVA | CONTENTS_SLIME))
		ucmd->upmove = SPEED_RUN;



	// Check speed
	VectorSubtract(self->s.origin, self->lastPosition, temp);
	float moved = VectorLength(temp);

	if (contents_feet & MASK_WATER)
	{
		self->bot.goal_node = INVALID;
		self->bot.current_node = INVALID;
		//if (debug_mode) 
		//Com_Printf("%s %s is stuck [Wander]: GOAL INVALID\n", __func__, self->client->pers.netname);
	}



	// Crouch or jump
	trace_t tr_lower_body;
	trace_t tr_upper_body;
	//vec3_t fwd;
	//AngleVectors(self->s.angles, fwd, NULL, NULL); // project forward from origin
	//VectorMA(self->s.origin, 50, fwd, fwd);
	//tr_lower_body = gi.trace(self->s.origin, tv(-15, -15, -15), tv(15, 15, 0), forward, self, MASK_DEADSOLID); // Box test [feet to mid body] -> forward 16 units
	//tr_upper_body = gi.trace(self->s.origin, tv(-15, -15, 0), tv(15, 15, 32), forward, self, MASK_DEADSOLID); // Box test [mid body to head] -> forward 16 units

	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7, self->viewheight - 8);
	offset[1] = 0;
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	VectorMA(start, 50, forward, end);
	tr_lower_body = gi.trace(start, tv(-15, -15, -15), tv(15, 15, 0), end, self, MASK_DEADSOLID); // Box test [feet to mid body] -> forward 16 units
	tr_upper_body = gi.trace(start, tv(-15, -15, 0), tv(15, 15, 32), end, self, MASK_DEADSOLID); // Box test [mid body to head] -> forward 16 units

	// Need to crouch?
	// Lower body is free, upper body is blocked
	if (tr_lower_body.fraction == 1.0 && tr_upper_body.fraction < 1.0)
	{
		ucmd->upmove = -400;
		ucmd->forwardmove = SPEED_RUN;
		//if (debug_mode) 
		//Com_Printf("%s %s is stuck [Wander]: crouching\n", __func__, self->client->pers.netname);
		return;
	}
	// Need to jump?
	// Lower body is blocked, upper body is free
	if (tr_lower_body.fraction < 1.0 && tr_upper_body.fraction == 1.0)
	{
		if (contents_feet & MASK_WATER)
		{
			//if (debug_mode) 
			//Com_Printf("%s %s is stuck [Wander]: jumping out of water\n", __func__, self->client->pers.netname);
			self->s.angles[PITCH] = -45;
			//VectorScale(end, 50, self->velocity);
			ucmd->forwardmove = 400;
			//ucmd->upmove = 400;
			if (self->velocity[2] < 350)
				self->velocity[2] = 350;
			return;
		}

		ucmd->upmove = 400;
		ucmd->forwardmove = SPEED_RUN;
		//if (debug_mode) 
		//Com_Printf("%s %s is stuck [Wander]: jumping\n", __func__, self->client->pers.netname);

		return;
	}
	// In water facing a ladder
	//if ((contents_feet & MASK_WATER) && tr_lower_body.fraction < 1.0 && tr_upper_body.fraction < 1.0)
	if ((contents_feet & MASK_WATER) && ((tr_lower_body.contents & CONTENTS_LADDER) || (tr_upper_body.contents & CONTENTS_LADDER)))
	{
		//if ((tr_lower_body.contents & CONTENTS_LADDER) || (tr_upper_body.contents & CONTENTS_LADDER))
		{
			//if (debug_mode)
			//Com_Printf("%s %s is stuck [Wander]: MASK_WATER -> CONTENTS_LADDER\n", __func__, self->client->pers.netname);
			ucmd->forwardmove = 400;
			ucmd->upmove = 400;
			self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY * 3, NODE_ALL);
			self->s.angles[PITCH] = -45;
			if (self->velocity[2] < 350)
				self->velocity[2] = 350;
			return;
		}
	}


	
	// Project forward looking for walls
	//vec3_t      start, end;
	//vec3_t      forward, right;
	//vec3_t      offset;
	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7, self->viewheight - 8);
	offset[1] = 0;
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	VectorMA(start, 48, forward, end);
	trace_t tr = gi.trace(start, NULL, NULL, end, self, MASK_PLAYERSOLID);

	// If we hit an obstacle or we've not moved much, turn around
	if (tr.fraction < 1.0 || VectorLength(self->velocity) < 37 || moved < FRAMETIME)
	{
		self->s.angles[YAW] += 22.5 + (random() * 270);
		self->s.angles[PITCH] = 0;
		
		if (contents_feet & MASK_WATER)  // Just keep swimming.
			ucmd->forwardmove = SPEED_RUN;
		else if (!M_CheckBottom(self) && !self->groundentity) // if there is ground continue otherwise wait for next move
			ucmd->forwardmove = 0;
		else if (ACEMV_CanMove(self, MOVE_FORWARD))
			ucmd->forwardmove = SPEED_WALK;

		return;
	}

	// Otherwise try move forward normally
	if (ACEMV_CanMove(self, MOVE_FORWARD) || (contents_feet & MASK_WATER))
	{
		ucmd->forwardmove = SPEED_RUN;
	}

	//if (self->client->leg_damage > 0) // Do we have leg damage?
	//	return;

	// If no goal, try to get a new one
	if (self->bot.goal_node == INVALID)
		ACEAI_PickLongRangeGoal(self);

	// Try to move to our goal if we can
	if (self->bot.goal_node != INVALID)
	{
		// Update the node we're near
		self->bot.current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY*3, NODE_ALL);
		//int tmp_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY*3, NODE_ALL);
		//if (tmp_node != self->bot.current_node)
		//{
		//	self->bot.prev_node = self->bot.current_node;
		//	self->bot.current_node = tmp_node;
		//	//Com_Printf("%s %s prev_node %d -> curr node %d\n", __func__, self->client->pers.netname, self->bot.prev_node, self->bot.current_node);
		//}

		if (self->bot.current_node == INVALID)
		{
			if (debug_mode) 
				Com_Printf("%s %s could not find FindClosestReachableNode to reach goal %d. Wandering...\n", __func__, self->client->pers.netname, self->bot.goal_node);
			self->bot.state = STATE_WANDER;
			self->wander_timeout = level.framenum + 0.1 * HZ;
			return;
		}
		{
			if (debug_mode) 
				Com_Printf("%s %s Wander finding alternative path to goal %d\n", __func__, self->client->pers.netname, self->bot.goal_node);
			//BOTLIB_SetGoal(self, self->bot.goal_node);
			//return;
			
			self->bot.next_node = self->bot.current_node;
			self->bot.state = STATE_POSITION;
		}
		else // We couldn't visit the goal node, so lets pick a new goal
		{
			if (debug_mode)
				Com_Printf("%s %s Wander cannot visit goal %d. Picking new goal\n", __func__, self->client->pers.netname, self->bot.goal_node);
			ACEAI_PickLongRangeGoal(self);
		}
	}
#endif
}
