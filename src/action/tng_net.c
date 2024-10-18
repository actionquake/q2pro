/*

Support for activities around cURL-based egress network connections
Special thanks to Phatman for a bulk of the core code

*/

#include "g_local.h"
#include <jansson.h>

qboolean curldebug = false;

// You will need one of these for each of the requests ...
// ... if you allow concurrent requests to be sent at the same time
request_list_t *active_requests, *unused_requests;
request_list_t request_nodes[MAX_REQUESTS];
CURLM *stack = NULL;
size_t current_requests = 0;

void init_requests(void)
{
    size_t i;
    for (i = 0; i < MAX_REQUESTS - 1; i++)
        request_nodes[i].next = request_nodes + i + 1;
    request_nodes[MAX_REQUESTS - 1].next = NULL;
    unused_requests = request_nodes;
    active_requests = NULL;
}

request_t* new_request(void)
{
    request_list_t *current = unused_requests;
    if (current == NULL)
        return NULL; // Ran out of request slots
    unused_requests = unused_requests->next;
    current->next = active_requests;
    active_requests = current;
    return &current->request;
}

qboolean recycle_request(request_t* request)
{
    request_list_t *previous = NULL, *current = active_requests;
    if (current == NULL)
        return false; // No active requests exist in the list
    while (&current->request != request)
    {
        if (!current->next)
            return false; // Could not find this request in the list
        previous = current;
        current = current->next;
    }
    if (previous == NULL)
        active_requests = current->next; // Was first in the list
    else
        previous->next = current->next;
    current->next = unused_requests;
    unused_requests = current;
    memset(&current->request, 0, sizeof current->request);
    return true;
}

// This is faster than counting them and checking if it is 0
qboolean has_active_requests(void)
{
    return active_requests != NULL;
}

size_t count_active_requests(void)
{
    request_list_t *current;
    size_t count;
    for (count = 0, current = active_requests; current; current = current->next, count++)
        ; // Phatman: Don't delete this line
    return count;
}

/*
*/
void lc_get_player_stats(char* message)
{
    request_t *request;

    // Don't run this if curl is disabled or the webhook URL is set to "disabled"
    if (!sv_curl_enable->value)
        return;

    // Use webhook.site to test curl, it's very handy!
    //char *url = "https://webhook.site/4de34388-9f3b-47fc-9074-7bdcd3cfa346";
    char* url = AQ2WORLD_STAT_URL;

    // Get a new request object
    request = new_request();
    if (request == NULL) {
        gi.dprintf("tng_net: request_t ran out of request slots\n");
        return;
    }

    request->url = url;
    lc_start_request_function(request);
}


void announce_server_populating(void)
{
    // Don't announce again before sv_last_announce_interval seconds have passed
    if ((int)time(NULL) - (int)sv_last_announce_time->value < (int)sv_last_announce_interval->value)
        return;

    // Do not announce matchmode games
    if (matchmode->value)
        return;

    // Minimum 10 maxclients
    if (maxclients->value < 10)
        return;

    // Do not send if IP is invalid or private
    if (!is_valid_ipv4(server_ip->string)) {
        gi.dprintf("Server IP is invalid, deactivating announcements\n");
        gi.cvar_forceset("sv_discord_announce_enable", "0");
        return;
    }

    int playercount = CountRealPlayers();
    // Do not announce if player count is less than 20% of maxclients
    float threshold = fmax((game.maxclients * 0.2), 3);
    if (playercount < threshold)
        return;

    // All the checks are out of the way, now to form the data and send it

    json_t *srv_announce = json_object();

    json_object_set_new(srv_announce, "hostname", json_string(hostname->string));
    json_object_set_new(srv_announce, "server_ip", json_string(server_ip->string));
    json_object_set_new(srv_announce, "server_port", json_string(server_port->string));
    json_object_set_new(srv_announce, "player_count", json_integer(playercount));
    json_object_set_new(srv_announce, "maxclients", json_integer(game.maxclients));
    json_object_set_new(srv_announce, "mapname", json_string(level.mapname));

    json_t *root = json_object();
    json_object_set_new(root, "srv_announce", srv_announce);
    json_object_set_new(root, "webhook_url", json_string(sv_curl_discord_pickup_url->string));

    char *message = json_dumps(root, 0); // 0 is the flags parameter, you can set it to JSON_INDENT(4) for pretty printing

    lc_server_announce("/srv_announce_filling", message);

    json_decref(root);

    // Update the cvar so we don't announce again for sv_last_announce_interval seconds
    gi.cvar_forceset("sv_last_announce_time", va("%d", (int)time(NULL)));
}

static char* ConstructPlayerAwardStatString(void) {
    gclient_t **ranks = (gclient_t **)malloc(game.maxclients * sizeof(gclient_t *));
    int total = G_CalcRanks(ranks);

    // Calculate the required buffer size
    int buffer_size = 0;
    for (int award = 0; award < AWARD_MAX; award++) {
        for (int i = 0; i < total; i++) {
            if (ranks[i]->resp.awardstats[award] > 0) {
                buffer_size += snprintf(NULL, 0, "%s: %s (%d)\n", 
                    award == ACCURACY ? "Accuracy" :
                    award == IMPRESSIVE ? "Impressive" :
                    award == EXCELLENT ? "Excellent" :
                    award == DOMINATING ? "Domination" :
                    award == UNSTOPPABLE ? "Unstoppable" : "Unknown",
                    ranks[i]->pers.netname, ranks[i]->resp.awardstats[award]);
            }
        }
    }

    // Allocate the buffer
    char *result = (char *)malloc(buffer_size + 1);
    if (!result) {
        free(ranks);
        return NULL; // Handle memory allocation failure
    }

    // Construct the string
    char *ptr = result;
    for (int award = 0; award < AWARD_MAX; award++) {
        for (int i = 0; i < total; i++) {
            if (ranks[i]->resp.awardstats[award] > 0) {
                ptr += sprintf(ptr, "%s: %s (%d)\n", 
                    award == ACCURACY ? "Accuracy" :
                    award == IMPRESSIVE ? "Impressive" :
                    award == EXCELLENT ? "Excellent" :
                    award == DOMINATING ? "Dominating" :
                    award == UNSTOPPABLE ? "Unstoppable" : "Unknown",
                    ranks[i]->pers.netname, ranks[i]->resp.awardstats[award]);
            }
        }
    }

    // Construct the string for additional stats
    double accuracy;
    int shots;
    // Variables to track the highest accuracy
    double highest_accuracy = 0.0;
    gclient_t *highest_accuracy_player = NULL;

    for (int i = 0; i < total; i++) {
        shots = min(ranks[i]->resp.shotsTotal, 9999);
        if (shots)
            accuracy = (double)ranks[i]->resp.hitsTotal * 100.0 / (double)ranks[i]->resp.shotsTotal;
        else
            accuracy = 0;

        if (accuracy > highest_accuracy) {
            highest_accuracy = accuracy;
            highest_accuracy_player = ranks[i];
        }
    }

    // Print the player with the highest accuracy
    if (highest_accuracy_player) {
        ptr += sprintf(ptr, "Highest accuracy: %s (%.2f%%)\n", highest_accuracy_player->pers.netname, highest_accuracy);
    }


    free(ranks);
    return result;
}

static char* DMConstructPlayerScoreString(void) {
    gclient_t **ranks = (gclient_t **)malloc(game.maxclients * sizeof(gclient_t *));
    int total = G_CalcRanks(ranks);

    // Calculate the required buffer size
    int buffer_size = 0;
    for (int i = 0; i < total; i++) {
        buffer_size += snprintf(NULL, 0, "%s: %d\n", ranks[i]->pers.netname, ranks[i]->resp.score);
    }

    // Allocate the buffer
    char *result = (char *)malloc(buffer_size + 1);
    if (!result) {
        return NULL; // Handle memory allocation failure
    }

    // Construct the string
    char *ptr = result;
    for (int i = 0; i < total; i++) {
        ptr += sprintf(ptr, "%s: %d\n", ranks[i]->pers.netname, ranks[i]->resp.score);
    }

    return result;
}

int FilterPlayersByTeam(gclient_t **filtered, int team) {
    int total = 0;
    for (int i = 0; i < game.maxclients; i++) {
        if (game.clients[i].pers.connected == 1 && game.clients[i].resp.team == team) {
            filtered[total] = &game.clients[i];
            total++;
        }
    }
    return total;
}

char* TeamConstructPlayerScoreString(int team) {
    gclient_t **filtered = (gclient_t **)malloc(game.maxclients * sizeof(gclient_t *));
    if (!filtered) {
        return NULL; // Handle memory allocation failure
    }

    int total = FilterPlayersByTeam(filtered, team);

    // Sort the filtered players
    qsort(filtered, total, sizeof(gclient_t *), G_PlayerCmp);

    // Calculate the required buffer size
    int buffer_size = 0;
    for (int i = 0; i < total; i++) {
        buffer_size += snprintf(NULL, 0, "%s: %d\n", filtered[i]->pers.netname, filtered[i]->resp.score);
    }

    // Allocate the buffer
    char *result = (char *)malloc(buffer_size + 1);
    if (!result) {
        free(filtered);
        return NULL; // Handle memory allocation failure
    }

    // Construct the string
    char *ptr = result;
    for (int i = 0; i < total; i++) {
        ptr += sprintf(ptr, "%s: %d\n", filtered[i]->pers.netname, filtered[i]->resp.score);
    }

    free(filtered);
    return result;
}

char* TeamConstructPlayerList(int team) {
    gclient_t **filtered = (gclient_t **)malloc(game.maxclients * sizeof(gclient_t *));
    if (!filtered) {
        return NULL; // Handle memory allocation failure
    }

    int total = FilterPlayersByTeam(filtered, team);

    // Calculate the required buffer size
    int buffer_size = 0;
    for (int i = 0; i < total; i++) {
        buffer_size += snprintf(NULL, 0, "%s\n", filtered[i]->pers.netname);
    }

    // Allocate the buffer
    char *result = (char *)malloc(buffer_size + 1);
    if (!result) {
        free(filtered);
        return NULL; // Handle memory allocation failure
    }

    // Construct the string
    char *ptr = result;
    for (int i = 0; i < total; i++) {
        ptr += sprintf(ptr, "%s\n", filtered[i]->pers.netname);
    }

    free(filtered);
    return result;
}

static char* ConstructGameSettingsString(void) {
    // Calculate the required buffer size
    int buffer_size = 256;

    // Allocate the buffer
    char *result = (char *)malloc(buffer_size);
    if (!result) {
        return NULL; // Handle memory allocation failure
    }

    if (matchmode->value) {
        snprintf(result, buffer_size, "```tgren %s\ntimelimit %s\nroundtimelimit %s\nuse_xerp %s\ndmflags %s\n```",
            tgren->string,
            timelimit->string,
            roundtimelimit->string,
            use_xerp->string,
            dmflags->string
        );
    } else {
        snprintf(result, buffer_size, "```gamemode %s\ntgren %s\nfraglimit %s\ntimelimit %s\nroundtimelimit %s\nuse_xerp %s\ndmflags %s\n```",
            gm->string,
            tgren->string,
            fraglimit->string,
            timelimit->string,
            roundtimelimit->string,
            use_xerp->string,
            dmflags->string
        );
    }

    return result;
}

static char *discord_MatchEndMsg(char* msg)
{
    // Create the root object
    json_t *root = json_object();

    json_object_set_new(root, "content", json_string(""));

    // Create the "embeds" array
    json_t *embeds = json_array();
    json_object_set_new(root, "embeds", embeds);

    // Create the embed object
    json_t *embed = json_object();
    json_array_append_new(embeds, embed);

    // Adjust color based on mode
    json_object_set_new(embed, "description", json_string(msg));
    if (teamplay->value) { // Green
        json_object_set_new(embed, "color", json_integer(65280));
    } else if (matchmode->value) {  // Light blue
        json_object_set_new(embed, "color", json_integer(65535));
    } else { // Red
        json_object_set_new(embed, "color", json_integer(16711680));
    }

    // Add fields to the embed object
    json_object_set_new(embed, "title", json_string(level.mapname));

    // Create the "thumbnail" object
    json_t *thumbnail = json_object();
    json_object_set_new(embed, "thumbnail", thumbnail);

    char mapimgurl[512];
    snprintf(mapimgurl, sizeof(mapimgurl), "https://raw.githubusercontent.com/vrolse/AQ2-pickup-bot/main/thumbnails/%s.jpg", level.mapname);
    json_object_set_new(thumbnail, "url", json_string(mapimgurl));

    // Add "username" field to the root object
    json_object_set_new(root, "username", json_string(TP_MATCH_END_MSG));

    // Create the "author" object (hostname)
    json_t *author = json_object();
    json_object_set_new(author, "name", json_string(hostname->string));
    json_object_set_new(embed, "author", author);

    // Create the "footer" object (server ip and port) if server_ip is valid
    if (is_valid_ipv4(server_ip->string)) {
        json_t *footer = json_object();
        char footerinfo[64];
        snprintf(footerinfo, sizeof(footerinfo), "%s: %s:%s", hostname->string, server_ip->string, net_port->string);
        json_object_set_new(footer, "text", json_string(footerinfo));
        json_object_set_new(embed, "footer", footer);
    }

    // Create the "fields" array
    json_t *fields = json_array();
    json_object_set_new(embed, "fields", fields);

    // Create field objects and add them to the "fields" array
    // Checks teamplay first, then deathmatch
    if (teamplay->value) {
        for (int team = TEAM1; team <= teamCount; team++) {
            char *team_name = TeamName(team);
            int team_score = teams[team].score;
            char field_content[64];
            snprintf(field_content, sizeof(field_content), "%s: %d", team_name, team_score);
            char *team_player_scores = TeamConstructPlayerScoreString(team);

            // Format team player scores
            char formatted_team_player_scores[1024] = ""; // Initialize buffer to empty string
            char line[64]; // Buffer for each formatted line
            char *token;
            int offset = 0;

            // Tokenize the team_player_scores string and format each line
            token = strtok(team_player_scores, "\n");
            while (token != NULL) {
                char player[17]; // Buffer for player name (16 characters + null terminator)
                int score;
                sscanf(token, "%16[^:]: %d", player, &score); // Read player name and score
                snprintf(line, sizeof(line), "%-16s %4d\n", player, score); // Format with fixed width
                offset += snprintf(formatted_team_player_scores + offset, sizeof(formatted_team_player_scores) - offset, "%s", line);
                token = strtok(NULL, "\n");
            }

            // Add triple backticks for Discord formatting
            char discord_formatted_team_player_scores[1280];
            snprintf(discord_formatted_team_player_scores, sizeof(discord_formatted_team_player_scores), "```%s```", formatted_team_player_scores);

            json_t *field = json_object();
            json_object_set_new(field, "name", json_string(field_content));
            json_object_set_new(field, "value", json_string(discord_formatted_team_player_scores));
            json_object_set_new(field, "inline", json_false());
            json_array_append_new(fields, field);

            free(team_player_scores); // Free the allocated memory for team_player_scores
        }
    } else { // Deathmatch
        char *player_scores = DMConstructPlayerScoreString();
        char formatted_player_scores[1024]; // Increase buffer size to accommodate formatted scores
        char line[64]; // Buffer for each formatted line
        char *token;
        int offset = 0;

        // Tokenize the player_scores string and format each line
        token = strtok(player_scores, "\n");
        while (token != NULL) {
            char player[17]; // Buffer for player name (16 characters + null terminator)
            int score;
            // Read player name and score, allowing spaces in player names
            sscanf(token, "%16[^:]: %d", player, &score); 
            // Format with fixed width
            snprintf(line, sizeof(line), "%-16s %4d\n", player, score); 
            // Append formatted line to the result
            offset += snprintf(formatted_player_scores + offset, sizeof(formatted_player_scores) - offset, "%s", line);
            token = strtok(NULL, "\n");
        }

        // Add triple backticks for Discord formatting
        char discord_formatted_scores[1280];
        snprintf(discord_formatted_scores, sizeof(discord_formatted_scores), "```\n%s```", formatted_player_scores);

        json_t *field = json_object();
        json_object_set_new(field, "name", json_string("")); // Empty on purpose
        json_object_set_new(field, "value", json_string(discord_formatted_scores));
        json_object_set_new(field, "inline", json_true()); // Inline true here because it's only one list
        json_array_append_new(fields, field);

        free(player_scores); // Free the allocated memory for player_scores
    }

    //  TODO: Testing out player award stats printing, broken for now

    // char *player_stats = ConstructPlayerAwardStatString();
    // // Format stats
    // char formatted_player_stats[1024] = ""; // Initialize buffer to empty string
    // char line[128]; // Buffer for each formatted line
    // char *token;
    // int offset = 0;

    // // Tokenize the player_stats string and format each line
    // token = strtok(player_stats, "\n");
    // while (token != NULL) {
    //     char award[32]; // Buffer for award name
    //     char player[17]; // Buffer for player name (16 characters + null terminator)
    //     int count;
    //     sscanf(token, "Most %31[^:]: %16[^ ] (%d)", award, player, &count); // Read award, player name, and count
    //     snprintf(line, sizeof(line), "Most %-16s %s (%d)\n", award, player, count); // Format with fixed width
    //     offset += snprintf(formatted_player_stats + offset, sizeof(formatted_player_stats) - offset, "%s", line);
    //     token = strtok(NULL, "\n");
    // }

    // // Add triple backticks for Discord formatting
    // char discord_formatted_scores[1280];
    // snprintf(discord_formatted_scores, sizeof(discord_formatted_scores), "```%s```", formatted_player_stats);

    // json_t *gamesettings = json_object();
    // json_object_set_new(gamesettings, "name", json_string("Awards / Stats"));
    // json_object_set_new(gamesettings, "value", json_string(discord_formatted_scores));
    // json_object_set_new(gamesettings, "inline", json_true());
    // json_array_append_new(fields, gamesettings);

    // Convert the JSON object to a string
    char *json_payload = json_dumps(root, JSON_INDENT(4));

    // Decrement the reference count of the root object to free memory
    json_decref(root);

    return json_payload;
}

static char *discord_PickupReqMsg(char* msg)
{
    // Create the root object
    json_t *root = json_object();

    json_object_set_new(root, "content", json_string(""));

    // Create the "embeds" array
    json_t *embeds = json_array();
    json_object_set_new(root, "embeds", embeds);
    json_object_set_new(root, "avatar_url", json_string("https://raw.githubusercontent.com/darkshade9/aq2world/master/docs/assets/img/common/aq2world_discord_circle.png"));

    // Create the embed object
    json_t *embed = json_object();
    json_array_append_new(embeds, embed);

    // Adjust description and color based on mode
    if (esp->value) {
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(65280)); // Green
    } else if (teamdm->value) { //
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(65535)); // Light blue
    } else if (ctf->value) {
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(16776960)); // Yellow
    } else if (teamplay->value) { 
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(8388736)); // Purple
    } else if (use_tourney->value) { 
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(16744192)); // Orange
    } else { //Deathmatch
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(16711680)); // Red
    }

    // Add fields to the embed object
    json_object_set_new(embed, "title", json_string(level.mapname));

    // Create the "thumbnail" object
    json_t *thumbnail = json_object();
    json_object_set_new(embed, "thumbnail", thumbnail);

    char mapimgurl[512];
    snprintf(mapimgurl, sizeof(mapimgurl), "https://raw.githubusercontent.com/vrolse/AQ2-pickup-bot/main/thumbnails/%s.jpg", level.mapname);
    json_object_set_new(thumbnail, "url", json_string(mapimgurl));

    // Add "username" field to the root object
    json_object_set_new(root, "username", json_string(PICKUP_GAME_REQUEST));

    // Create the "author" object (hostname)
    json_t *author = json_object();
    json_object_set_new(author, "name", json_string(hostname->string));
    json_object_set_new(embed, "author", author);

    // Create the "footer" object (server ip and port) if server_ip is valid
    if (is_valid_ipv4(server_ip->string)) {
        json_t *footer = json_object();
        char footerinfo[64];
        snprintf(footerinfo, sizeof(footerinfo), "%s: %s:%s", hostname->string, server_ip->string, net_port->string);
        json_object_set_new(footer, "text", json_string(footerinfo));
        json_object_set_new(embed, "footer", footer);
    }

    // Create the "fields" array
    json_t *fields = json_array();
    json_object_set_new(embed, "fields", fields);

    // Create field objects and add them to the "fields" array
    // if (teamplay->value) {
    //     for (int team = TEAM1; team <= teamCount; team++) {
    //         char *team_name = TeamName(team);
    //         char field_content[64];
    //         snprintf(field_content, sizeof(field_content), "%s", team_name);
    //         char *team_players = TeamConstructPlayerList(team);

    //         // Add triple backticks for Discord formatting
    //         char discord_formatted_players[1280];
    //         snprintf(discord_formatted_players, sizeof(discord_formatted_players), "```\n%s```", team_players);

    //         json_t *field = json_object();
    //         json_object_set_new(field, "name", json_string(field_content));
    //         json_object_set_new(field, "value", json_string(discord_formatted_players));
    //         json_object_set_new(field, "inline", json_true());
    //         json_array_append_new(fields, field);

    //         free(team_players); // Free the allocated memory for team_players
    //     }
    // } else {
    char *team_name = "Players";
    char field_content[64];
    snprintf(field_content, sizeof(field_content), "%s", team_name);
    char *team_players = TeamConstructPlayerList(0);

    // Add triple backticks for Discord formatting
    char discord_formatted_players[1280];
    snprintf(discord_formatted_players, sizeof(discord_formatted_players), "```\n%s```", team_players);


    json_t *field = json_object();
    json_object_set_new(field, "name", json_string(field_content));
    json_object_set_new(field, "value", json_string(discord_formatted_players));
    json_object_set_new(field, "inline", json_true());
    json_array_append_new(fields, field);

    free(team_players); // Free the allocated memory for team_players

    // Game Settings
    json_t *gamesettings = json_object();
    json_object_set_new(gamesettings, "name", json_string("Game Settings"));
    json_object_set_new(gamesettings, "value", json_string(ConstructGameSettingsString()));
    json_object_set_new(gamesettings, "inline", json_true());
    json_array_append_new(fields, gamesettings);

    // Convert the JSON object to a string
    char *json_payload = json_dumps(root, JSON_INDENT(4));

    // Decrement the reference count of the root object to free memory
    json_decref(root);

    return json_payload;
}

static char *discord_MatchStartMsg(char* msg)
{
    // Create the root object
    json_t *root = json_object();

    json_object_set_new(root, "content", json_string(""));

    // Create the "embeds" array
    json_t *embeds = json_array();
    json_object_set_new(root, "embeds", embeds);

    // Create the embed object
    json_t *embed = json_object();
    json_array_append_new(embeds, embed);

    // Adjust description and color based on mode
    if (teamplay->value) { // Green
        json_object_set_new(embed, "description", json_string(msg));
        json_object_set_new(embed, "color", json_integer(65280));
    }

    // Add fields to the embed object
    json_object_set_new(embed, "title", json_string(level.mapname));

    // Create the "thumbnail" object
    json_t *thumbnail = json_object();
    json_object_set_new(embed, "thumbnail", thumbnail);

    char mapimgurl[512];
    snprintf(mapimgurl, sizeof(mapimgurl), "https://raw.githubusercontent.com/vrolse/AQ2-pickup-bot/main/thumbnails/%s.jpg", level.mapname);
    json_object_set_new(thumbnail, "url", json_string(mapimgurl));

    // Add "username" field to the root object
    json_object_set_new(root, "username", json_string(TP_MATCH_START_MSG));

    // Create the "author" object (hostname)
    json_t *author = json_object();
    json_object_set_new(author, "name", json_string(hostname->string));
    json_object_set_new(embed, "author", author);

    // Create the "footer" object (server ip and port) if server_ip is valid
    if (is_valid_ipv4(server_ip->string)) {
        json_t *footer = json_object();
        char footerinfo[64];
        snprintf(footerinfo, sizeof(footerinfo), "%s: %s:%s", hostname->string, server_ip->string, net_port->string);
        json_object_set_new(footer, "text", json_string(footerinfo));
        json_object_set_new(embed, "footer", footer);
    }

    // Create the "fields" array
    json_t *fields = json_array();
    json_object_set_new(embed, "fields", fields);

    // Create field objects and add them to the "fields" array
    for (int team = TEAM1; team <= teamCount; team++) {
        char *team_name = TeamName(team);
        char field_content[64];
        snprintf(field_content, sizeof(field_content), "%s", team_name);
        char *team_players = TeamConstructPlayerList(team);

        // Add triple backticks for Discord formatting
        char discord_formatted_players[1280];
        snprintf(discord_formatted_players, sizeof(discord_formatted_players), "```\n%s```", team_players);


        json_t *field = json_object();
        json_object_set_new(field, "name", json_string(field_content));
        json_object_set_new(field, "value", json_string(discord_formatted_players));
        json_object_set_new(field, "inline", json_true());
        json_array_append_new(fields, field);

        free(team_players); // Free the allocated memory for team_players
    }

    // Game Settings
    json_t *gamesettings = json_object();
    json_object_set_new(gamesettings, "name", json_string("Game Settings"));
    json_object_set_new(gamesettings, "value", json_string(ConstructGameSettingsString()));
    json_object_set_new(gamesettings, "inline", json_true());
    json_array_append_new(fields, gamesettings);

    // Convert the JSON object to a string
    char *json_payload = json_dumps(root, JSON_INDENT(4));

    // Decrement the reference count of the root object to free memory
    json_decref(root);

    return json_payload;
}

static char *discord_ServerMsg(char* msg, Discord_Notifications msgtype, Awards awardtype)
{
    // Create the root object
    json_t *root = json_object();

    json_object_set_new(root, "content", json_string(""));

    // Create the "embeds" array
    json_t *embeds = json_array();
    json_object_set_new(root, "embeds", embeds);

    // Create the embed object
    json_t *embed = json_object();
    json_array_append_new(embeds, embed);
    json_object_set_new(embed, "description", json_string(msg));
    json_object_set_new(embed, "color", json_integer(16766720));

    // Add "username" field to the root object
    json_object_set_new(root, "username", json_string(hostname->string));

    // Adjust the icon based on msgtype
    switch (awardtype) {
        case EXCELLENT:
            json_object_set_new(root, "avatar_url", json_string("https://raw.githubusercontent.com/darkshade9/aq2world/master/docs/assets/img/common/excellent.png"));
            break;
        case IMPRESSIVE:
            json_object_set_new(root, "avatar_url", json_string("https://raw.githubusercontent.com/darkshade9/aq2world/master/docs/assets/img/common/impressive.png"));
            break;
        default:
            break;
    }
    // Create the "footer" object (server ip and port) if server_ip is valid
    if (is_valid_ipv4(server_ip->string)) {
        json_t *footer = json_object();
        char footerinfo[64];
        snprintf(footerinfo, sizeof(footerinfo), "%s: %s:%s", hostname->string, server_ip->string, net_port->string);
        json_object_set_new(footer, "text", json_string(footerinfo));
        json_object_set_new(embed, "footer", footer);
    }

    // Convert the JSON object to a string
    char *json_payload = json_dumps(root, JSON_INDENT(4));

    // Decrement the reference count of the root object to free memory
    json_decref(root);

    return json_payload;
}

/*
Call this with a string containing the message you want to send to the webhook.
Limited to 2048 chars.
*/
void lc_discord_webhook(char* message, Discord_Notifications msgtype, Awards awardtype)
{
    request_t *request;
    char json_payload[2048];

    // Debug prints
    if (curldebug) {
        gi.dprintf("msgflags value: %d\n", (int)msgflags->value);
        gi.dprintf("msgtype value: %d\n", msgtype);
    }
    // Check if the msgtype is within the allowed message flags
    // Look at the Discord_Notifications enum in g_local.h for the flags
    if (!(msgtype & (int)msgflags->value)) {
        if (curldebug)
            gi.dprintf("Message type (%d) not allowed by msgflags (%d)\n", msgtype, (int)msgflags->value);
        return;
    }

    // Don't run this if curl is disabled or the webhook URL is set to "disabled"
    if (!sv_curl_enable->value || strcmp(sv_curl_discord_info_url->string, "disabled") == 0)
        return;

    // Default url is to the #info-feed channel
    char* url = sv_curl_discord_info_url->string;
    // Change webhook URL based on message type.  If pickup url is disabled, use info url
    if (msgtype == SERVER_MSG || msgtype == PICKUP_REQ_MSG) {
        if (strcmp(sv_curl_discord_pickup_url->string, "disabled") == 0)
            url = sv_curl_discord_info_url->string;
        else
            url = sv_curl_discord_pickup_url->string;
    }

    // Use webhook.site to test curl, it's very handy!
    //char *url = "https://webhook.site/4de34388-9f3b-47fc-9074-7bdcd3cfa346";
    //char* url = sv_curl_discord_info_url->string;

    // Get a new request object
    request = new_request();
    if (request == NULL) {
        gi.dprintf("Ran out of request slots\n");
        return;
    }

    // Remove newline character from the end of message
    char* newline = strchr(message, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }

    switch (msgtype) {
        case CHAT_MSG:
        case DEATH_MSG:
            snprintf(json_payload, sizeof(json_payload), 
                    "{\"content\":\"```%s```\"}", 
                    message);
            break;

        case AWARD_MSG:
        case SERVER_MSG: {
            char *srvmsg = discord_ServerMsg(message, msgtype, awardtype);
            if (srvmsg) {
                snprintf(json_payload, sizeof(json_payload), "%s", srvmsg);
                free(srvmsg);
            }
            break;
        }

        case PICKUP_REQ_MSG: {
            char *pickupreqmsg = discord_PickupReqMsg(message);
            if (pickupreqmsg) {
                snprintf(json_payload, sizeof(json_payload), "%s", pickupreqmsg);
                free(pickupreqmsg);
            }
            break;
        }

        case MATCH_START_MSG: {
            char *matchstartmsg = discord_MatchStartMsg(message);
            if (matchstartmsg) {
                snprintf(json_payload, sizeof(json_payload), "%s", matchstartmsg);
                free(matchstartmsg);
            }
            break;
        }

        case MATCH_END_MSG: {
            char *matchendmsg = discord_MatchEndMsg(message);
            if (matchendmsg) {           
                snprintf(json_payload, sizeof(json_payload), "%s", matchendmsg);
                free(matchendmsg);
            }
            break;
        }

        default:
            if (curldebug)
                gi.dprintf("%s: Unknown message type\n", __func__);
            break;
    }

    if (curldebug)
        gi.dprintf("%s\n", json_payload);

    request->url = url;
    request->payload = strdup(json_payload);

    lc_start_request_function(request);
}

qboolean lc_aqtion_stat_send(const char* stats)
{
    if (!sv_curl_stat_enable->value)
        return false;

    if (!sv_curl_enable->value) {
        gi.dprintf("%s: sv_curl_enable is disabled, disabling stat reporting to API\n", __func__);
        gi.cvar_forceset("sv_curl_stat_enable", "0");
        return false;
    }

    request_t *request;
    char full_url[1024];
    char* url = AQ2WORLD_STAT_URL;
    char path[] = "/api/v1/stats";

    // Get a new request object
    request = new_request();
    if (request == NULL) {
        gi.dprintf("%s: Ran out of request slots\n", __func__);
        return false;
    }
    //gi.dprintf("%s: Sending stats to %s\n", __func__, url);
    // Use webhook.site to test curl, it's very handy!
    //char *url = "https://webhook.site/4de34388-9f3b-47fc-9074-7bdcd3cfa346";
    // Concatenate url and path
    sprintf(full_url, "%s%s", url, path);
    request->url = full_url;
    request->payload = strdup(stats);
    lc_start_request_function(request);

    // Returning true here just means we sent the request to lc_start_request_function, it doesn't mean it worked
    return true;
}

void lc_server_announce(char *path, char *message)
{
    request_t *request;
    char full_url[1024];

    // Don't run this if curl is disabled or the webhook URLs are set to "disabled" or empty, or if no server IP or port is set
    if (!cvar_check(sv_curl_enable) 
        || !cvar_check(server_announce_url)
        || !cvar_check(sv_curl_discord_pickup_url)
        || !cvar_check(server_ip)
        || !cvar_check(server_port))
    {
        return;
    }

    // Get a new request object
    request = new_request();
    if (request == NULL) {
        gi.dprintf("Ran out of request slots\n");
        return;
    }

    // Use webhook.site to test curl, it's very handy!
    //char *url = "https://webhook.site/4de34388-9f3b-47fc-9074-7bdcd3cfa346";
    char* url = server_announce_url->string;
    // Concatenate url and path
    sprintf(full_url, "%s%s", url, path);
    request->url = full_url;
    request->payload = strdup(message);

    gi.dprintf("Sending server announce to %s\n", full_url);
    gi.dprintf("With payload: %s\n", message);

    lc_start_request_function(request);
}

void lc_shutdown_function(void)
{
    if (stack)
    {
        curl_multi_cleanup(stack);
        stack = NULL;
    }
	current_requests = 0;
    curl_global_cleanup();
}

qboolean lc_init_function(void)
{
    lc_shutdown_function();
    init_requests();
    if (curl_global_init(CURL_GLOBAL_ALL))
        return false;
    stack = curl_multi_init();
    if (!stack)
        return false;
    if (curl_multi_setopt(stack, CURLMOPT_MAXCONNECTS, MAX_REQUESTS) != CURLM_OK)
    {
        curl_multi_cleanup(stack);
        curl_global_cleanup();
        return false;
    }
	// ...
	return true;
}

// Your callback function's return type and parameter types _must_ match this:
size_t lc_receive_data_function(char *data, size_t blocks, size_t bytes, void *pvt)
{
	request_t *request;
    if (bytes <= 0){
        return 0;
    }

	request = (request_t*) pvt;
	if (!request)
	{
        gi.dprintf("%s: ERROR! pvt argument was NULL.\n", __func__);
		return 0;
	}
    if (bytes > MAX_DATA_BYTES - request->data_count)
	{
        gi.dprintf("%s: ERROR! Too much data.\n", __func__);
        return 0; // TODO: Ensure this cancels the request
	}
    memcpy(request->data + request->data_count, request->data, bytes);
    request->data_count += bytes;
    return bytes;
}

// Requires that request->url already be set to the target URL
void lc_start_request_function(request_t* request)
{
    struct curl_slist *headers = NULL;

    request->data_count = 0;
    request->handle = curl_easy_init();

    // Set the headers to indicate we're sending JSON data, and a custom one
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "x-aqtion-server: true");
    curl_easy_setopt(request->handle, CURLOPT_HTTPHEADER, headers);

    // Set the JSON payload if it exists (as POST request), else this is a GET request
    if (request->payload)
        curl_easy_setopt(request->handle, CURLOPT_POSTFIELDS, request->payload);

    curl_easy_setopt(request->handle, CURLOPT_WRITEFUNCTION, lc_receive_data_function);
    curl_easy_setopt(request->handle, CURLOPT_URL, request->url);
    curl_easy_setopt(request->handle, CURLOPT_WRITEDATA, request); // Passed as pvt to lc_receive_data_function
    curl_easy_setopt(request->handle, CURLOPT_PRIVATE, request); // Returned by curl_easy_getinfo with the CURLINFO_PRIVATE option
    
    curl_multi_add_handle(stack, request->handle);
    current_requests++;
}

void process_stats(json_t *stats_json)
{
    int i;
    if (!json_is_object(stats_json)) {
        gi.dprintf("error: stats is not a JSON object\n");
        return;
    }

    const char *stat_names[] = {"frags", "deaths", "damage"};
    int num_stats = sizeof(stat_names) / sizeof(stat_names[0]);

    lt_stats_t stats;
    for (i = 0; i < num_stats; i++) {
        json_t *stat_json = json_object_get(stats_json, stat_names[i]);
        if (!stat_json) {
            gi.dprintf("error: %s is missing\n", stat_names[i]);
            return;
        }

        if (!json_is_integer(stat_json)) {
            gi.dprintf("error: %s is not an integer\n", stat_names[i]);
            return;
        }

        int stat_value = json_integer_value(stat_json);
        if (strcmp(stat_names[i], "frags") == 0) {
            stats.frags = stat_value;
        } else if (strcmp(stat_names[i], "deaths") == 0) {
            stats.deaths = stat_value;
        } else if (strcmp(stat_names[i], "damage") == 0) {
            stats.damage = stat_value;
        }
    }
}

void lc_parse_response(char* data)
{
	// json_error_t error;
    // json_t *root = json_loads(data, 0, &error);

    // if(!root)
    // {
    //     gi.dprintf("error: on line %d: %s\n", error.line, error.text);
    //     return;
    // }

    // json_t *stats_json = json_object_get(root, "stats");
    // if(!json_is_object(stats_json))
    // {
    //     // If the root is not "stats", do nothing
    //     json_decref(root);
    //     return;
    // } else {
    //     //process_stats(stats_json);
    //     json_decref(root);
    // }
}

void lc_once_per_gameframe(void)
{
    CURLMsg *messages;
    CURL *handle;
    request_t *request;
    int handles, remaining;

    // Debug request counts
    //gi.dprintf("%s: current_requests = %d\n", __func__, current_requests);
    if (current_requests <= 0)
        return;
    if (curl_multi_perform(stack, &handles) != CURLM_OK)
    {
        gi.dprintf("%s: curl_multi_perform error -- resetting libcurl session\n", __func__);
        lc_init_function();
        return;
    }
    while ((messages = curl_multi_info_read(stack, &remaining)))
    {
        handle = messages->easy_handle;
        if (messages->msg == CURLMSG_DONE)
        {
            curl_easy_getinfo(handle, CURLINFO_PRIVATE, &request);
            if (request->data_count < MAX_DATA_BYTES)
                request->data[request->data_count] = '\0';
            else
                request->data[MAX_DATA_BYTES - 1] = '\0';
			lc_parse_response(request->data);
            free(request->payload); // Frees up the memory allocated by strdup
            recycle_request(request);
            curl_multi_remove_handle(stack, handle);
            curl_easy_cleanup(handle);
            current_requests--;
        }
    }
    if (handles == 0)
		current_requests = 0;
}

qboolean message_timer_check(int delay)
{
    static time_t last_announcement_time = 0;
    time_t current_time = time(NULL);

    if (difftime(current_time, last_announcement_time) >= delay) {
        last_announcement_time = current_time;
        return true;
    }
    return false;
}