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
	int len;
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

qboolean BOTLIB_DoParabolaJump(edict_t* self, vec3_t targetpoint)
{
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
		float g = sv_gravity->value; // Gravity
		float distance = VectorDistanceXY(self->s.origin, self->enemy->s.origin); // XY Distance to enemy
		float height_diff = self->enemy->s.origin[2] - self->s.origin[2]; // Height difference
		float knife_speed = 1200;
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
					VectorSubtract(tv(0, 0, 0), self->s.origin, lookdir); // Look at center of map
				else
					VectorSubtract(self->bot.bi.look_at, self->s.origin, lookdir); // Look at random map item

			}

			// If lookdir is empty, then look at map center
			if (VectorEmpty(self->bot.bi.look_at))
			{
				VectorSubtract(tv(0, 0, 0), self->s.origin, lookdir);
			}

			BOTLIB_ChangeBotAngleYawPitch(self, lookdir, false, turn_speed, true, true);
		}
	}
}

/////////////////
// darksaint: Breaking up BOTLIB_Wander into discrete functionality
// This thing was huge
// Refer to older versions for the original function
/////////////////


void _Wander_PickupNearbyItem(edict_t* self, vec3_t walkdir, float move_speed, usercmd_t* ucmd)
{
	if (self->bot.get_item != NULL && self->bot.get_item->inuse)
	{
		float item_dist = VectorDistance(self->bot.get_item->s.origin, self->s.origin);
		//Com_Printf("%s %s is looking for item %s [%f]\n", __func__, self->client->pers.netname, self->bot.get_item->classname, item_dist);

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
	}
}

void _Wander_Teamplay(edict_t* self)
{
	// If the bot has just spawned, then we need to wait a bit before we start moving.
	if (self->just_spawned) // set by SpawnPlayers() in a_team.c
	{
		self->just_spawned = false;
		self->just_spawned_go = true; // Bot is ready, when wander_timeout is reached.

		// If enemy is in sight, don't wait too long
		if (self->enemy)
			self->just_spawned_timeout = level.framenum + random() * HZ;			// Short wait
		else
		{
			// Otherwise pick from various wait times before moving out
			int rnd_rng = rand() % 4;
			if (rnd_rng == 0)
				self->just_spawned_timeout = level.framenum + (random() * 3) * HZ;	// Long wait
			else if (rnd_rng == 1)
				self->just_spawned_timeout = level.framenum + (random() * 2) * HZ;	// Medium wait
			else if (rnd_rng == 2)
				self->just_spawned_timeout = level.framenum + (random() * HZ);  // Short wait
			else
				self->just_spawned_timeout = 0;										// No wait
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
		BOTLIB_PickLongRangeGoal(self);
		self->just_spawned_go = false; // Now we can move!
	}
}

qboolean _Wandering(edict_t* self, vec3_t walkdir, float move_speed, trace_t tr, usercmd_t* ucmd)
{
	self->bot.stuck_wander_time--;
	self->bot.node_travel_time = 0;
	self->bot.state = BOT_MOVE_STATE_NAV;
	self->bot.goal_node = INVALID;

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
			return true;
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
			return false;
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
	
	return false;
}

void _Wander_Node_Water(edict_t* self, int current_node_type, int next_node_type, vec3_t walkdir, float move_speed)
{
	// Water!
	/// Borrowed from P_WorldEffects
	int waterlevel = self->waterlevel;

	if (current_node_type == NODE_WATER || waterlevel == 3){
		gi.dprintf("I'm in the water %s and my air is %d\n", self->client->pers.netname, self->air_finished_framenum - level.framenum);
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
}

void _Wander_Node_Boxjump(edict_t* self)
{
	if (nodes[self->bot.next_node].origin[2] > nodes[self->bot.current_node].origin[2]) // Going up
	{
		self->bot.bi.actionflags |= ACTION_BOXJUMP;
		self->bot.bi.actionflags &= ~ACTION_JUMP;
		self->bot.bi.actionflags &= ~ACTION_CROUCH;
	} else {
		// Remove flags
		self->bot.bi.actionflags &= ~ACTION_CROUCH;
		self->bot.bi.actionflags &= ~ACTION_BOXJUMP;
		self->bot.bi.actionflags &= ~ACTION_JUMP;
	}
}

void _Wander_Node_Jump(edict_t* self)
{
	self->bot.bi.actionflags |= ACTION_JUMP;
	// Remove flags
	self->bot.bi.actionflags &= ~ACTION_CROUCH;
}

void _Wander_Node_Jumppad(edict_t* self, vec3_t walkdir, float move_speed)
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
}

void _Wander_Node_Move(edict_t* self)
{
	if (self->groundentity == NULL)
		self->bot.bi.speed = SPEED_ROAM; // Set our speed directly
}

void _Wander_Node_Crouch(edict_t* self)
{
	self->bot.bi.actionflags |= ACTION_CROUCH;
	self->bot.bi.actionflags &= ~ACTION_JUMP;
}

void _Wander_Node_Ladder(edict_t* self, vec3_t walkdir)
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

	if (self->bot.prev_node != INVALID && nodes[self->bot.prev_node].type == NODE_LADDER && nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type != NODE_LADDER && self->s.origin[2] < nodes[self->bot.current_node].origin[2] + 4 && self->groundentity == NULL) // 
	{
		if (nodes[self->bot.prev_node].origin[2] < nodes[self->bot.current_node].origin[2] && self->s.origin[2] < nodes[self->bot.current_node].origin[2] + 2) // Getting off ladder at the top
		{
			self->bot.bi.actionflags |= ACTION_MOVEUP;
		}
	}
}

void _Wander_CTF(edict_t* self)
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

void _Wander_SeeEnemies(edict_t* self)
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

		if (self->client->weapon == FindItem(HC_NAME))
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
			if (self->client->weapon == FindItem(HC_NAME) && self->client->cannon_rds)
			{
				// Come in close for the kill
				if (ACEMV_CanMove(self, MOVE_FORWARD))
					self->bot.bi.actionflags |= ACTION_MOVEFORWARD;
			}
			else if (self->client->weapon == FindItem(KNIFE_NAME))
			{
				// Come in close for the kill
				if (ACEMV_CanMove(self, MOVE_FORWARD))
					self->bot.bi.actionflags |= ACTION_MOVEFORWARD;
			}
			// Try move backwards
			else if (BOTLIB_CanMove(self, MOVE_BACK))
			{
				self->bot.bi.actionflags |= ACTION_MOVEBACK;
			}
		}
		// If distance is far, consider crouching to increase accuracy
		else if (self->bot.enemy_dist > 1024)
		{
			// Check if bot should be crouching based on weapon and if strafing
			if (INV_AMMO(self, LASER_NUM) == false && 
				self->client->weapon != FindItem(SNIPER_NAME) && 
				self->client->weapon != FindItem(HC_NAME) && 
				self->client->weapon != FindItem(M3_NAME) && 
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
			if (self->bot.enemy_dist < 1024 && self->client->weapon == FindItemByNum(SNIPER_NUM) && BOTLIB_CanMove(self, MOVE_BACK))
			{
				self->bot.bi.actionflags |= ACTION_MOVEBACK;
			}
			// Keep distance with grenade
			if (self->bot.enemy_dist < 1024 && self->client->weapon == FindItemByNum(GRENADE_NUM) && BOTLIB_CanMove(self, MOVE_BACK))
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

void _Wander_GetUnstuck(edict_t* self, vec3_t walkdir)
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

void BOTLIB_Wander(edict_t* self, usercmd_t* ucmd)
{
	int next_node = INVALID; // Next node to walk towards
	vec3_t walkdir; // Direction to walk
	vec3_t lastdir;
	float move_speed = SPEED_RUN; // Movement speed, default to running
	self->bot.bi.speed = 0; // Zero speed
	trace_t tr; // Trace

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
	_Wander_PickupNearbyItem(self, walkdir, move_speed, ucmd);

	if (self->bot.touching_ladder == false)// || (nodes[self->bot.next_node].type != NODE_LADDER && self->groundentity))
	{
		self->bot.bi.actionflags &= ~ACTION_MOVEDOWN;
		self->bot.bi.actionflags &= ~ACTION_MOVEUP;
	}

	// This only applies for teamplay
	// Do not follow path when teammates are still inside us.
	if (OnTransparentList(self)) // Teamplay
		_Wander_Teamplay(self);


	if (self->groundentity && self->bot.next_node == INVALID) // No next node, pick new nav
	{
		//Com_Printf("%s %s next_node is invalid; find a new path\n", __func__, self->client->pers.netname);
		self->bot.bi.speed = 0;
		self->bot.state = BOT_MOVE_STATE_NAV;
		return;
	}
	// On ground and current or next node is invalid
	if (self->groundentity && self->bot.current_node == INVALID)
	{
		self->bot.bi.speed = 0;
		self->bot.stuck_wander_time = 1;
	}
	if (self->groundentity && self->bot.goal_node == INVALID)// && self->bot.node_travel_time >= 15)
	{
		self->bot.bi.speed = 0;
		self->bot.state = BOT_MOVE_STATE_NAV;
		return;
	}

	next_node = self->bot.next_node;

	// If travel time took too long, assume we're stuck
	if (self->bot.node_travel_time >= 120) //60 // Bot failure to reach next node
	{
		self->bot.stuck_wander_time = 1;
		//Com_Printf("%s %s node_travel_time was hit! cur[%d] nxt[%d] goal[%d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
	}

	//self->bot.stuck_wander_time = 0;

	if (self->bot.stuck_wander_time) {// && nav_area.total_areas <= 0)
		if (!_Wandering(self, walkdir, move_speed, tr, ucmd)) {
			if (next_node == INVALID)
			{
				//self->bot.stuck_wander_time = 1;
				return;
			}
			VectorSubtract(nodes[next_node].origin, self->s.origin, walkdir); // Head to next node
			VectorNormalize(walkdir);
		}
	}

	if (self->bot.stuck_wander_time)
		return;

	// Strafe to get back on track
	if (0 && self->bot.next_node != INVALID)
	{
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
		}
	}

	//self->bot.bi.speed = move_speed; // Set our suggested speed

	// if (0)
	// {
	// 	// Get current direction
	// 	vec3_t angle, forward, right, start, end, origin, offset;
	// 	vectoangles(walkdir, angle);
	// 	AngleVectors(angle, forward, right, NULL);
	// 	VectorCopy(self->s.origin, origin);
	// 	//origin[2] -= 24; // From the ground up
	// 	origin[2] += 8; // [Origin 24 units] + [8 units] == 32 units heigh (same as node height)

	// 	VectorSet(offset, 0, 0, 0); // changed from 18,0,0
	// 	G_ProjectSource(origin, offset, forward, right, start);
	// 	offset[0] += 1024; // Distance forward dir
	// 	G_ProjectSource(origin, offset, forward, right, end);

	// 	gi.WriteByte(svc_temp_entity);
	// 	gi.WriteByte(TE_BFG_LASER);
	// 	gi.WritePosition(start);
	// 	gi.WritePosition(end);
	// 	gi.multicast(self->s.origin, MULTICAST_PHS);
	// }

	
	if (1 & self->bot.node_list_count)
	{
		//for (int i = self->bot.node_list_current; i < self->bot.node_list_count; i++)
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
				break;
			}
		}
	}

	if (next_node_type == INVALID)
	{
		if (self->bot.next_node == INVALID)
		{
			Com_Printf("%s %s invalid next_node node:curr/next/goal[%d %d %d]  type:curr/next[%d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node, current_node_type, next_node_type);
			self->bot.state = BOT_MOVE_STATE_NAV;
			return;
		}
		if (self->bot.current_node == self->bot.next_node && self->bot.current_node == self->bot.goal_node)
		{
			Com_Printf("%s %s reached goal node:curr/next/goal[%d %d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
			////if (nav_area.total_areas > 0)
			////	self->bot.state = BOT_MOVE_STATE_NAV_NEXT;
			////else
				self->bot.state = BOT_MOVE_STATE_NAV;
			return;
		}
		else if (self->bot.next_node != INVALID && VectorDistance(nodes[self->bot.next_node].origin, self->s.origin) <= 128)
		{
			//Com_Printf("Distance to next node: %s %f\n", self->client->pers.netname, VectorDistance(nodes[self->bot.next_node].origin, self->s.origin));
			//Com_Printf("%s %s resolved next node:curr/next/goal[%d %d %d]\n", __func__, self->client->pers.netname, self->bot.current_node, self->bot.next_node, self->bot.goal_node);
		}
		if (next_node_type == INVALID)
		{
			return;
		}
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

		// Decisions based on nodetype

		if (current_node_type == NODE_WATER || next_node_type == NODE_WATER)
			_Wander_Node_Water(self, current_node_type, next_node_type, walkdir, move_speed);
		if (next_node_type == NODE_BOXJUMP)
			_Wander_Node_Boxjump(self);
		if (next_node_type == NODE_MOVE)
			_Wander_Node_Move(self);
		if (next_node_type == NODE_CROUCH)
			_Wander_Node_Crouch(self);
		if (nodes[self->bot.current_node].type == NODE_LADDER && nodes[self->bot.next_node].type == NODE_LADDER)
			_Wander_Node_Ladder(self, walkdir);
		if (next_node_type == NODE_JUMP && self->groundentity)
			_Wander_Node_Jump(self);
		if (next_node_type == NODE_JUMPPAD)
			_Wander_Node_Jumppad(self, walkdir, move_speed);
		if (ctf->value)
			_Wander_CTF(self);
		if (self->bot.see_enemies)
			_Wander_SeeEnemies(self);
	



		// Pull bot in when close to the next node, help guide it in
		// Not sure why this is (0)
		// if (0)
		// {
		// if (self->groundentity == false && level.framenum > self->bot.stuck_last_negate)
		// {
		// 	vec3_t bot_to_node;
		// 	VectorSubtract(nodes[self->bot.next_node].origin, self->s.origin, bot_to_node);
		// 	bot_to_node[2] = 0;
		// 	float xy_bot_to_next_dist = VectorLength(bot_to_node); // Distance from bot to next node
		// 	if (xy_bot_to_next_dist > 16 && xy_bot_to_next_dist <= 150)
		// 	{
		// 		// Line of sight
		// 		tr = gi.trace(self->s.origin, NULL, NULL, nodes[self->bot.next_node].origin, self, MASK_PLAYERSOLID);
		// 		if (tr.fraction == 1.0)
		// 		{
		// 			self->bot.stuck_last_negate = level.framenum + 1 * HZ;
		// 			vec3_t dir;
		// 			dir[0] = walkdir[0];
		// 			dir[1] = walkdir[1];
		// 			dir[2] = walkdir[2];
		// 			VectorNormalize(dir);
		// 			self->velocity[0] += 50 * dir[0];
		// 			self->velocity[1] += 50 * dir[1];
		// 			self->velocity[2] += 300 * dir[2];
		// 			// Limit velocity
		// 			if (self->velocity[2] < -300)
		// 				self->velocity[2] = -300;
		// 			//Com_Printf("%s %s [%d] directing bot closer to next node\n", __func__, self->client->pers.netname, level.framenum);
		// 		}
		// 	}
		// }

		//if (next_node_type == NODE_MOVE)
		//{}
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
		//}
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
		_Wander_GetUnstuck(self, walkdir);
	
	
	}
}