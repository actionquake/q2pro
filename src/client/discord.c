//======================================================================

//=====================================================================================================
#if USE_DISCORD && USE_CURL && USE_AQTION //rekkie -- discord -- s
//=====================================================================================================

//rekkie -- external ip -- s
#include <client.h>
#include "../extern/discord/c/discord_game_sdk.h"

static size_t CurlWriteCallback(char* buf, size_t size, size_t nmemb, void* up)
{
    if (strlen(buf) <= 16) // Length of an IP address
    {
        buf[strlen(buf) - 1] = '\0'; // Remove the newline
        cl_extern_ip = Cvar_Set("cl_extern_ip", buf);
    }
    return size * nmemb; // Return how many bytes we read
}
static void CL_GetExternalIP(void)
{
    CURL* curl = curl_easy_init(); // Init the curl session
    curl_easy_setopt(curl, CURLOPT_URL, "http://icanhazip.com"); // URL to get IP
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CurlWriteCallback); // Write the data
    CURLcode res = curl_easy_perform(curl); // Perform the request
    if (res != CURLE_OK) // Something went wrong
    {
        // Failover - switching to a redundant URL
        curl_easy_setopt(curl, CURLOPT_URL, "http://checkip.amazonaws.com/"); // URL to get IP
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CurlWriteCallback); // Write the data
        res = curl_easy_perform(curl); // Perform the request
        if (res != CURLE_OK) // Something went wrong, again!
        {
            Com_Printf("%s [CURL] could not resolve the external IP, curl returned: %i\n", __func__, res);
        }
    }
    
    // If all went well
    Com_Printf("External IP: %s\n", cl_extern_ip->string);
    
    curl_easy_cleanup(curl); // Cleanup
}

// Determine if connection is loopback, private ip, or public ip -- https://en.wikipedia.org/wiki/Private_network
static qboolean CL_IsPrivateNetwork(void)
{
    //--------------------------------------------------------------
    // Loopback
    if (cls.serverAddress.type == NA_LOOPBACK) // Loopback
    {
        return true; // 127.x
    }
    else if (cls.serverAddress.type >= NA_BROADCAST) // LAN, IPv4, IPv6
    {
        qboolean ispv = NET_IsLanAddress(&cls.serverAddress);
        if (ispv) // Private IP
        {
            return true; // (10.x  172.x  192.x  etc)
        }
        else // Public IP
        {
            return false;
        }
    }
    else
        return false;
    //--------------------------------------------------------------
}
//rekkie -- external ip -- e


// Using the Discord API
// https://discord.com/developers/docs/game-sdk/sdk-starter-guide
//
// Requires the Discord Game SDK
// https://dl-game-sdk.discordapp.net/2.5.6/discord_game_sdk.zip
//
// Extract the contents of the zip to /extern/discord/

static void DiscordCallback(void* data, enum EDiscordResult result)
{
    //Com_Printf("%s %s\n", __func__, data);
    
    discord.result = result;
    switch (discord.result)
    {
        case DiscordResult_Ok:
        //    Com_Printf("%s DiscordResult_Ok\n", __func__);
            break;
        case DiscordResult_ServiceUnavailable:
            Com_Printf("%s DiscordResult_ServiceUnavailable\n", __func__);
            break;
        case DiscordResult_InvalidVersion:
            Com_Printf("%s DiscordResult_InvalidVersion\n", __func__);
            break;
        case DiscordResult_LockFailed:
            Com_Printf("%s DiscordResult_LockFailed\n", __func__);
            break;
        case DiscordResult_InternalError:
            Com_Printf("%s DiscordResult_InternalError\n", __func__);
            break;
        case DiscordResult_InvalidPayload:
            Com_Printf("%s DiscordResult_InvalidPayload\n", __func__);
            break;
        case DiscordResult_InvalidCommand:
            Com_Printf("%s DiscordResult_InvalidCommand\n", __func__);
            break;
        case DiscordResult_InvalidPermissions:
            Com_Printf("%s DiscordResult_InvalidPermissions\n", __func__);
            break;
        case DiscordResult_NotFetched:
            Com_Printf("%s DiscordResult_NotFetched\n", __func__);
            break;
        case DiscordResult_NotFound:
            Com_Printf("%s DiscordResult_NotFound\n", __func__);
            break;
        case DiscordResult_Conflict:
            Com_Printf("%s DiscordResult_Conflict\n", __func__);
            break;
        case DiscordResult_InvalidSecret:
            Com_Printf("%s DiscordResult_InvalidSecret\n", __func__);
            break;
        case DiscordResult_InvalidJoinSecret:
            Com_Printf("%s DiscordResult_InvalidJoinSecret\n", __func__);
            break;
        case DiscordResult_NoEligibleActivity:
            Com_Printf("%s DiscordResult_NoEligibleActivity\n", __func__);
            break;
        case DiscordResult_InvalidInvite:
            Com_Printf("%s DiscordResult_InvalidInvite\n", __func__);
            break;
        case DiscordResult_NotAuthenticated:
            Com_Printf("%s DiscordResult_NotAuthenticated\n", __func__);
            break;
        case DiscordResult_InvalidAccessToken:
            Com_Printf("%s DiscordResult_InvalidAccessToken\n", __func__);
            break;
        case DiscordResult_ApplicationMismatch:
            Com_Printf("%s DiscordResult_ApplicationMismatch\n", __func__);
            break;
        case DiscordResult_InvalidDataUrl:
            Com_Printf("%s DiscordResult_InvalidDataUrl\n", __func__);
            break;
        case DiscordResult_InvalidBase64:
            Com_Printf("%s DiscordResult_InvalidBase64\n", __func__);
            break;
        case DiscordResult_NotFiltered:
            Com_Printf("%s DiscordResult_NotFiltered\n", __func__);
            break;
        case DiscordResult_LobbyFull:
            Com_Printf("%s DiscordResult_LobbyFull\n", __func__);
            break;
        case DiscordResult_InvalidLobbySecret:
            Com_Printf("%s DiscordResult_InvalidLobbySecret\n", __func__);
            break;
        case DiscordResult_InvalidFilename:
            Com_Printf("%s DiscordResult_InvalidFilename\n", __func__);
            break;
        case DiscordResult_InvalidFileSize:
            Com_Printf("%s DiscordResult_InvalidFileSize\n", __func__);
            break;
        case DiscordResult_InvalidEntitlement:
            Com_Printf("%s DiscordResult_InvalidEntitlement\n", __func__);
            break;
        case DiscordResult_NotInstalled:
            Com_Printf("%s DiscordResult_NotInstalled\n", __func__);
            break;
        case DiscordResult_NotRunning:
            Com_Printf("%s DiscordResult_NotRunning\n", __func__);
            break;
        case DiscordResult_InsufficientBuffer:
            Com_Printf("%s DiscordResult_InsufficientBuffer\n", __func__);
            break;
        case DiscordResult_PurchaseCanceled:
            Com_Printf("%s DiscordResult_PurchaseCanceled\n", __func__);
            break;
        case DiscordResult_InvalidGuild:
            Com_Printf("%s DiscordResult_InvalidGuild\n", __func__);
            break;
        case DiscordResult_InvalidEvent:
            Com_Printf("%s DiscordResult_InvalidEvent\n", __func__);
            break;
        case DiscordResult_InvalidChannel:
            Com_Printf("%s DiscordResult_InvalidChannel\n", __func__);
            break;
        case DiscordResult_InvalidOrigin:
            Com_Printf("%s DiscordResult_InvalidOrigin\n", __func__);
            break;
        case DiscordResult_RateLimited:
            Com_Printf("%s DiscordResult_RateLimited\n", __func__);
            break;
        case DiscordResult_OAuth2Error:
            Com_Printf("%s DiscordResult_OAuth2Error\n", __func__);
            break;
        case DiscordResult_SelectChannelTimeout:
            Com_Printf("%s DiscordResult_SelectChannelTimeout\n", __func__);
            break;
        case DiscordResult_GetGuildTimeout:
            Com_Printf("%s DiscordResult_GetGuildTimeout\n", __func__);
            break;
        case DiscordResult_SelectVoiceForceRequired:
            Com_Printf("%s DiscordResult_SelectVoiceForceRequired\n", __func__);
            break;
        case DiscordResult_CaptureShortcutAlreadyListening:
            Com_Printf("%s DiscordResult_CaptureShortcutAlreadyListening\n", __func__);
            break;
        case DiscordResult_UnauthorizedForAchievement:
            Com_Printf("%s DiscordResult_UnauthorizedForAchievement\n", __func__);
            break;
        case DiscordResult_InvalidGiftCode:
            Com_Printf("%s DiscordResult_InvalidGiftCode\n", __func__);
            break;
        case DiscordResult_PurchaseError:
            Com_Printf("%s DiscordResult_PurchaseError\n", __func__);
            break;
        case DiscordResult_TransactionAborted:
        //    Com_Printf("%s DiscordResult_TransactionAborted\n", __func__);
            break;
        default:
            Com_Printf("%s Unknown error code %i\n", __func__, discord.result);
            break;
    }
}

// Log output - error, warning, information, debug
static void DiscordLogCallback(void* hook_data, enum EDiscordLogLevel level, const char* message)
{
    if (level == DiscordLogLevel_Error)
        Com_Printf("%s ERROR: %s\n", __func__, message);
    else if (level == DiscordLogLevel_Warn)
        Com_Printf("%s WARN: %s\n", __func__, message);
    else if (level == DiscordLogLevel_Info)
        Com_Printf("%s INFO: %s\n", __func__, message);
    else if (level == DiscordLogLevel_Debug)
        Com_Printf("%s DEBUG: %s\n", __func__, message);
}

//  Not used anywhere?
// static void OnOAuth2Token(void* data, enum EDiscordResult result, struct DiscordOAuth2Token* token)
// {
//     if (result == DiscordResult_Ok)
//         Com_Printf("%s access token: %s\n", __func__, token->access_token);
//     else
//     {
//         Com_Printf("%s GetOAuth2Token failed with: ", __func__);
//         DiscordCallback(NULL, result);
//     }
// }

// Filter Discord friends
static bool RelationshipPassFilter(void* data, struct DiscordRelationship* relationship)
{
    return (relationship->type == DiscordRelationshipType_Friend); // Return true if they are a Discord friend
}
// List all Discord friends
static void OnRelationshipsRefresh(void* data)
{
    struct Application* app = (struct Application*)data;
    struct IDiscordRelationshipManager* module = app->relationships;

    module->filter(module, app, RelationshipPassFilter);
    int32_t relation_count = 0;
    DiscordCallback(NULL, module->count(module, &relation_count));
    if (discord.result == DiscordResult_Ok)
    {
        for (int32_t i = 0; i < relation_count; i += 1)
        {
            struct DiscordRelationship relationship;
            DiscordCallback(NULL, module->get_at(module, i, &relationship));
            //if (discord.result == DiscordResult_Ok)
            //    Com_Printf("%lld %s#%s\n", (long long)relationship.user.id, relationship.user.username, relationship.user.discriminator);
        }
    }
}
static void OnRelationshipUpdate(void* data, struct DiscordRelationship* relationship)
{
}

// User event
static void OnUserUpdated(void* data)
{
    struct Application* app = (struct Application*)data;
    struct DiscordUser user;
    discord.app.user->get_current_user(app->user, &user);
    discord.user = user;

    cl_discord_id = Cvar_Set("cl_discord_id", va("%lld", (long long)discord.user.id));
    cl_discord_username = Cvar_Set("cl_discord_username", discord.user.username);
    cl_discord_discriminator = Cvar_Set("cl_discord_discriminator", discord.user.discriminator);
    cl_discord_avatar = Cvar_Set("cl_discord_avatar", discord.user.avatar);
    
    //Com_Printf("%s User profile updated: %lld %s#%s\n", __func__, (long long)discord.user.id, discord.user.username, discord.user.discriminator);
}

// Fires when the user receives a join or spectate invite.
// We let the user decide what they want to do, instead of forcing them to join; this function is currently here just to log the event
static void OnActivityInvite(void* data, enum EDiscordActivityActionType type, struct DiscordUser *user, struct DiscordActivity *activity)
{   
    if (type == DiscordActivityActionType_Join)
    {
        //Com_Printf("%s DiscordActivityActionType_Join %lld %s#%s\n", __func__, (long long)user->id, user->username, user->discriminator);
        
        // If we ever wanted to force the user to join, call this function
        //discord.app.activities->accept_invite(discord.app.activities, user->id, "Accepted", DiscordCallback);
    }
    else // DiscordActivityActionType_Spectate
    {
        //Com_Printf("%s DiscordActivityActionType_Spectate %lld %s#%s\n", __func__, (long long)user->id, user->username, user->discriminator);

        // If we ever wanted to force the user to join, call this function
        //discord.app.activities->accept_invite(discord.app.activities, user->id, "Accepted", DiscordCallback);
    }
}
static void DiscordLobbyCallBack(void* data, enum EDiscordResult result, struct DiscordLobby* lobby)
{
    DiscordCallback("DiscordLobbyCallBack", result); // Check the result
    
    //if (lobby != NULL)
    //    Com_Printf("%s %s ID[%lld] SECRET[%s]\n", __func__, (char*)data, (long long)lobby->id, lobby->secret);
}

// This function runs when a player clicks the join button when invited through Discord
// The secret received is the concatenation of lobby_id + lobby_secret
static void OnActivityJoin(void* event_data, const char* secret) 
{
    //Com_Printf("%s secret[%s]\n", __func__, secret);

    if (discord.lobby.id == 0 && strlen(secret) > 0)
    {
        DiscordLobbyId lobby_id;
        DiscordLobbySecret lobby_secret;
        char activity_secret[128]; // Size here is sizeof DiscordLobbySecret
        Q_strlcpy(activity_secret, secret, sizeof(activity_secret));

        // Copy lobby secret (used if lobby ownership is transferred to us)
        Q_snprintf(discord.activity.secrets.join, sizeof(discord.activity.secrets.join), "%s", activity_secret);

        // Extract lobby_id and lobby_secret from activity_secret (lobby_id:secret)
        //-------------------------------------------------------------------------
        // Get lobby_id
        char* token = strtok(activity_secret, ":");
        if (token != NULL)
            lobby_id = atoll(token);
        else
            lobby_id = 0;
        
        // Get lobby_secret
        token = strtok(NULL, ":");
        if (token != NULL)
            Q_strlcpy(lobby_secret, token, sizeof(lobby_secret));
        else
            strcpy(lobby_secret, "");
        //-------------------------------------------------------------------------

        if (lobby_id)
        {
            // Connect the user to the lobby (secret must be lobby_id:lobby_secret)
            Q_strlcpy(activity_secret, secret, sizeof(activity_secret));
            discord.app.lobbies->connect_lobby_with_activity_secret(discord.app.lobbies, activity_secret, "OnActivityJoin", DiscordLobbyCallBack);

            if (discord.result == DiscordResult_Ok) // Result from DiscordLobbyCallBack()
            {
                //Com_Printf("%s Connected to lobby %lld\n", __func__, (long long)lobby_id);
                discord.lobby.id = lobby_id;
                Q_strlcpy(discord.lobby.secret, lobby_secret, sizeof(discord.lobby.secret));
            }
        }
    }
}
static void OnActivitySpectate(void* event_data, const char* secret) 
{
    //Com_Printf("%s secret[%s]\n", __func__, secret);
}

// Fires when a user asks to join our game.
static void OnActivityJoinRequest(void* event_data, struct DiscordUser* user) 
{
    //Com_Printf("%s %lld %s#%s\n", __func__, (long long)user->id, user->username, user->discriminator);

    // Here we can either automatically accept a join request by calling up send_request_reply()
    // ... or we can do nothing and let the user decide if they want to accept or deny
    if (cl_discord_accept_join_requests->value)
        discord.app.activities->send_request_reply(discord.app.activities, user->id, DiscordActivityJoinRequestReply_Yes, "Accepted", DiscordCallback);
    else
        ; // let user decide
    
}

void CL_DiscordParseServerStatus(serverStatus_t* status, const char* string)
{
    const char* s;
    size_t infolen;

    s = Q_strchrnul(string, '\n'); // parse '\n' terminated infostring

    // Due to off-by-one error in the original version of Info_SetValueForKey,
    // some servers produce infostrings up to 512 characters long. Work this
    // bug around by cutting off the last character(s).
    infolen = s - string;
    if (infolen >= MAX_INFO_STRING)
        infolen = MAX_INFO_STRING - 1;

    // copy infostring off
    memcpy(status->infostring, string, infolen);
    status->infostring[infolen] = 0;

    if (!Info_Validate(status->infostring))
        strcpy(status->infostring, "\\hostname\\badinfo");

    Q_snprintf(discord.server_hostname, sizeof(discord.server_hostname), "%s", Info_ValueForKey(string, "hostname"));

    // parse player list
    discord.curr_players = 0;
    while (discord.curr_players < MAX_STATUS_PLAYERS)
    {
        COM_Parse(&s); // Score
        COM_Parse(&s); // Ping
        COM_Parse(&s); // Name
        if (!s)
            break;
        discord.curr_players++;
    }
}

// Allow players to invite or join a server via Discord
// A lobby is created when a player joins a server, and deleted when they leave the game
static void CL_DiscordGameInvite(void)
{   
    // Create a lobby if none exists
    if (discord.lobby.id == 0)
    {
        CL_CreateDiscordLobby_f();
    }

    // Always update these
    if (discord.lobby.id)
    {
        Q_snprintf(discord.activity.party.id, sizeof(discord.activity.party.id), "%lld\n", (long long)discord.lobby.id);

        // Join -- the join secret must be a concatenation of lobby id and secret ==> lobby_id:lobby_secret
        Q_snprintf(discord.activity.secrets.join, sizeof(discord.activity.secrets.join), "%lld:%s", (long long)discord.lobby.id, discord.lobby.secret); // Secrets.join must be unique
        //Com_Printf("%s %s\n", __func__, discord.activity.secrets.join);

        discord.activity.party.size.current_size = discord.curr_players;
        discord.activity.party.size.max_size = cl.maxclients;
    }
}

static void CL_UpdateActivity(void)
{
    if (cls.serverAddress.type == NA_UNSPECIFIED)
        return;

    if (cls.demo.playback)
    {
        if (discord.lobby.id) // If not connected, remove the lobby (if any)
            CL_DeleteDiscordLobby();

        discord.last_activity_time = 0;
        discord.server_hostname[0] = '\0';
        sprintf(discord.activity.details, "Watching");
        discord.activity.state[0] = '\0';

        // Since the user isn't in a map, just use game logo
        Q_snprintf(discord.activity.assets.large_image, sizeof(discord.activity.assets.large_image), "%s", DISCORD_APP_IMAGE);
        Q_snprintf(discord.activity.assets.large_text, sizeof(discord.activity.assets.large_text), "%s", DISCORD_APP_TEXT);

        // Reset the small logo because we're using the large logo instead
        discord.activity.assets.small_image[0] = '\0';
        discord.activity.assets.small_text[0] = '\0';
    }
    else if (cls.state == ca_active) // Client remote connection
    {
        // Get the hostname and player count (local and remote games)
        if (discord.last_activity_time < cls.realtime)
        {
            // NOTE: Only update every ~15 seconds because this requests an infostring size string
            // from the server that is up to 512 bytes long.
            discord.last_activity_time = cls.realtime + DISCORD_ACTIVITY_UPDATE_MSEC;

            // Get hostname and player count by sending a request to the game server, 
            // the reply is processed by CL_DiscordParseServerStatus().
            // The result is cached to discord.server_hostname
            netadr_t    adr;
            neterr_t    ret;

            adr = cls.netchan.remote_address;
            CL_AddRequest(&adr, REQ_STATUS_DISCORD);

            NET_Config(NET_CLIENT);

            ret = OOB_PRINT(NS_CLIENT, &adr, "status");
            if (ret == NET_ERROR)
                Com_Printf("%s to %s\n", NET_ErrorString(), NET_AdrToString(&adr));

            return; // Wait for server to respond
        }

        // Apply map name and player count
        if (strlen(cl.mapname) > 0)
        {       
            if (cls.state == ca_active) // Player fully in game
                Q_snprintf(discord.activity.state, sizeof(discord.activity.state), "Playing %s [%i/%i]", cl.mapname, discord.curr_players, cl.maxclients);
            else // Connection: handshake, downloading, loading, etc.
                sprintf(discord.activity.state, "Connecting...");
        }

        // If connection is loopback, ignore users 'hostname' setting, which defaults to 'noname'
        if (cls.serverAddress.type == NA_LOOPBACK)
            sprintf(discord.server_hostname, "Local Game");

        // Hostname
        Q_snprintf(discord.activity.details, sizeof(discord.activity.details), "%s", discord.server_hostname);

        // Add the map image (defaults to app icon if not found)
        Q_snprintf(discord.activity.assets.large_image, sizeof(discord.activity.assets.large_image), "%s", cl.mapname); // Set map image
        Q_snprintf(discord.activity.assets.large_text, sizeof(discord.activity.assets.large_text), "%s", cl.mapname);  // Set map name

        // Add the game logo under the map image
        Q_snprintf(discord.activity.assets.small_image, sizeof(discord.activity.assets.small_image), "%s", DISCORD_APP_IMAGE);
        Q_snprintf(discord.activity.assets.small_text, sizeof(discord.activity.assets.small_text), "%s", DISCORD_APP_TEXT);

        CL_DiscordGameInvite(); // Creates a lobby and opens up game invites
    }
    else // Main menu
    {
        if (discord.lobby.id) // If not connected, remove the lobby (if any)
            CL_DeleteDiscordLobby();
        
        discord.last_activity_time = 0;
        discord.server_hostname[0] = '\0';
        sprintf(discord.activity.details, "Main menu");
        discord.activity.state[0] = '\0';

        // Since the user isn't in a map, just use game logo
        Q_snprintf(discord.activity.assets.large_image, sizeof(discord.activity.assets.large_image), "%s", DISCORD_APP_IMAGE);
        Q_snprintf(discord.activity.assets.large_text, sizeof(discord.activity.assets.large_text), "%s", DISCORD_APP_TEXT);

        // Reset the small logo because we're using the large logo instead
        discord.activity.assets.small_image[0] = '\0';
        discord.activity.assets.small_text[0] = '\0';
    }

    discord.activity.type = DiscordActivityType_Playing;
    discord.activity.application_id = DISCORD_APP_ID;
    discord.activity.timestamps.start = 0;
    discord.activity.timestamps.end = 0;
    discord.activity.instance = true;
    discord.app.activities->update_activity(discord.app.activities, &discord.activity, "update_activity", DiscordCallback);
}

static void OnLobbyUpdate(void* event_data, int64_t lobby_id) 
{
    //Com_Printf("%s lobbyID[%lld]\n", __func__, (long long)lobby_id);
}
static void OnLobbyDelete(void* event_data, int64_t lobby_id, uint32_t reason) 
{
    //Com_Printf("%s lobbyID[%lld] reason[%i]\n", __func__, (long long)lobby_id, reason);
}
static void OnLobbyConnect(void* data, enum EDiscordResult result, struct DiscordLobby* lobby)
{
    if (result == DiscordResult_Ok)
    {
        //Com_Printf("%s lobby_id[%lld] lobby_type[%i] owner_id[%lld] secret[%s] capacity[%i] locked[%i]\n", __func__, (long long)lobby->id, lobby->type, (long long)lobby->owner_id, lobby->secret, lobby->capacity, lobby->locked);
        discord.lobby.id = lobby->id;
        discord.lobby.type = lobby->type;
        discord.lobby.owner_id = lobby->owner_id;
        Q_snprintf(discord.lobby.secret, sizeof(discord.lobby.secret), "%s", lobby->secret);
        discord.lobby.capacity = lobby->capacity;
        discord.lobby.locked = lobby->locked;
    }
    else
    {
        Com_Printf("%s OnLobbyConnect failed with: ", __func__);
        DiscordCallback(NULL, result);
    }
}
static void OnLobbyMemberConnect(void* event_data, int64_t lobby_id, int64_t user_id)
{
    char addr[32]; // IP + Port
    char msg[256];

    //Com_Printf("%s lobby_id[%lld] user_id[%lld]\n", __func__, (long long)lobby_id, (long long)user_id);

    // If the host is using a private network IP, ensure the invited client connects to the host's public IP
    if (CL_IsPrivateNetwork()) // Private ip
    {
        
        if (strlen(cl_extern_ip->string)) // We have a valid external IP
        {
            // Copy the external ip + port
            Q_snprintf(addr, sizeof(addr), "%s:%s", cl_extern_ip->string, net_port->string);
        }
        else
            return; // No external IP, or couldn't connect to http://icanhazip.com to get our IP
    }
    else // Public IP - Just use the server IP + Port
    {
        // Copy the remote server ip + port
        Q_snprintf(addr, sizeof(addr), "%s", NET_AdrToString(&cls.serverAddress));
    }
    
    //Com_Printf("%s %s\n", __func__, addr);
    
    // Send new member the server connection details
    // We sent the user_id to identify who the message is for
    // If the game is password protected
    if (strlen(info_password->string) > 0)
    {
        // Send msg type + intended recipient + ip:port + game password
        Q_snprintf(msg, sizeof(msg) - 1, "%i|%lld|%s|%s|", DISCORD_MSG_CONNECT, (long long)user_id, addr, info_password->string);
    }
    else
    {
        // Send msg type + intended recipient + ip:port + empty pass (no password)
        Q_snprintf(msg, sizeof(msg) - 1, "%i|%lld|%s| |", DISCORD_MSG_CONNECT, (long long)user_id, addr);
    }
    
    //Com_Printf("%s [%s]\n", __func__, msg);

    // Broadcast
    discord.app.lobbies->send_lobby_message(discord.app.lobbies, discord.lobby.id, (uint8_t*)msg, strlen(msg), "OnLobbyMemberConnect", DiscordCallback);
}
static void OnLobbyMemberUpdate(void* event_data, int64_t lobby_id, int64_t user_id) 
{
    //Com_Printf("%s lobby_id[%lld] user_id[%lld]\n", __func__, (long long)lobby_id, (long long)user_id);
}
static void OnLobbyMemberDisconnect(void* event_data, int64_t lobby_id, int64_t user_id) 
{
    //Com_Printf("%s lobby_id[%lld] user_id[%lld]\n", __func__, (long long)lobby_id, (long long)user_id);
}
static void OnLobbyMessage(void* event_data, int64_t lobby_id, int64_t user_id, uint8_t* data, uint32_t data_length) 
{
    if (data_length == 0 || data_length > 1024)
        return;

    int64_t intended_user_id = 0;
    
    char msg[1024];
    Q_strlcpy(msg, (char*)data, data_length);
    
    //Com_Printf("%s lobby_id[%lld] user_id[%lld] data[%s] data_length[%i]\n", __func__, (long long)lobby_id, (long long)user_id, msg, data_length);

    int msg_len = 0;
    enum discord_message_type msg_type = 0;
        
    // Get the message type
    char* token = strtok(msg, "|");
    if (token)
    {
        msg_type = atoi(token);
        msg_len += strlen(token) + 1;
    }
    else
    {
        Com_Printf("%s received an invalid message type (malformed data)\n", __func__);
        return;
    }

    // Get the intended user_id
    token = strtok(NULL, "|");
    if (token)
    {
        intended_user_id = atoll(token);
        msg_len += strlen(token) + 1;
    }
    else
    {
        Com_Printf("%s received an invalid user_id (malformed data)\n", __func__);
        return;
    }

    // Special case - we disconnect from lobby only after we've fully transferred ownership
    if (msg_type == DISCORD_MSG_OWNERSHIP && user_id == discord.user.id)
    {
        //Com_Printf("%s We've successfully transfered ownership of the lobby to [%lld]\n", __func__, (long long)intended_user_id);
        discord.app.lobbies->disconnect_lobby(discord.app.lobbies, lobby_id, "disconnect_lobby", DiscordCallback);
    }

    // Check if the msg was intended for someone else
    if (discord.user.id != intended_user_id)
        return;

    // We've just received ownership of the lobby
    if (msg_type == DISCORD_MSG_OWNERSHIP)
    {
        //Com_Printf("%s You've taken ownership of the lobby\n", __func__);
        
        discord.lobby.id = lobby_id;
        Q_snprintf(discord.activity.party.id, sizeof(discord.activity.party.id), "%lld\n", (long long)discord.lobby.id);
        discord.activity.party.size.current_size = discord.curr_players;
        discord.activity.party.size.max_size = cl.maxclients;
        
        return;
    }

    // We recevied a request to connect to a quake2 server
    if (msg_type == DISCORD_MSG_CONNECT && cls.serverAddress.type < NA_IP) // Not connected
    {
        // Extract server addr and password from the message
        //--------------------------------------------------
        char addr[64];
        char pass[64];
        pass[0] = '\n';

        // Copy the ip:port
        token = strtok(NULL, "|");
        if (token)
        {
            Q_snprintf(addr, sizeof(addr), "%s", token);
            msg_len += strlen(token) + 1;
        }
        else
        {
            Com_Printf("%s received invalid server ip:port (malformed data)\n", __func__);
            return;
        }

        // Copy the password
        token = strtok(NULL, "|");
        if (token)
        {
            Q_snprintf(pass, sizeof(pass), "%s", token);
            if (strcmp(pass, " ") != 0) // Has a password, and not " " empty space
            {
                Cvar_Set("password", pass); // Set server password
                //Com_Printf("%s addr[%s] pass[%s]\n", __func__, addr, pass);
            }
            else
            {
                //Com_Printf("%s addr[%s]\n", __func__, addr);
            }
        }
        //---------------------------------------------------------

        //if (pass[0] != '\0') // Set the game password, if any
        //    Cvar_Set("password", pass);

        // Connect player to server
        netadr_t address;
        int protocol;
        protocol = cl_protocol->integer;
        if (!protocol) {
            protocol = PROTOCOL_VERSION_Q2PRO;
        }
        if (!NET_StringToAdr(addr, &address, PORT_SERVER)) {
            Com_Printf("Bad server address\n");
            return;
        }
        Q_strlcpy(cls.servername, addr, sizeof(cls.servername)); // copy early to avoid potential cmd_argv[1] clobbering
        SV_Shutdown("Server was killed.\n", ERR_DISCONNECT); // if running a local server, kill it and reissue
        NET_Config(NET_CLIENT);
        CL_Disconnect(ERR_RECONNECT);
        cls.serverAddress = address;
        cls.serverProtocol = protocol;
        cls.protocolVersion = 0;
        cls.passive = false;
        cls.state = ca_challenging;
        cls.connect_time -= CONNECT_FAST;
        cls.connect_count = 0;
        Con_Popup(true);
        CL_CheckForResend();

        return;
    }
}
static void OnLobbySpeaking(void* event_data, int64_t lobby_id, int64_t user_id, bool speaking) 
{
    //Com_Printf("%s lobby_id[%lld] user_id[%lld] speaking[%i]\n", __func__, (long long)lobby_id, (long long)user_id, speaking);
}
static void OnLobbyNetworkMessage(void* event_data, int64_t lobby_id, int64_t user_id, uint8_t channel_id, uint8_t* data, uint32_t data_length)
{
    //Com_Printf("%s lobby_id[%lld] user_id[%lld] channel_id[%i] data[%s] data_length[%i]\n", __func__, (long long)lobby_id, (long long)user_id, channel_id, data, data_length);
}

// Build a list of lobbies
// Note: must run a query before using lobby functions (discord.app.lobbies->func)
static void CL_QueryLobby(void)
{
    struct IDiscordLobbySearchQuery* query;
    discord.app.lobbies->get_search_query(discord.app.lobbies, &query);
    query->limit(query, 1);
    discord.app.lobbies->search(discord.app.lobbies, query, "lobbies->search", DiscordCallback);
}

void CL_CreateDiscordLobby_f(void)
{
    // Call CL_QueryLobby to build a list of lobbies before creating a new lobby
    CL_QueryLobby();
    if (discord.result == DiscordResult_Ok) // Result from CL_QueryLobby() -> search()
    {
        // Search for lobbies
        int32_t lobby_count;
        discord.app.lobbies->lobby_count(discord.app.lobbies, &lobby_count);
        if (lobby_count == 0) // If no existing lobbies
        {
            DiscordCallback(NULL, discord.app.lobbies->get_lobby_create_transaction(discord.app.lobbies, &discord.transaction));
            if (discord.result == DiscordResult_Ok) // If get_lobby_create_transaction was okay
            {
                // Setup lobby
                DiscordMetadataKey key = "quake2";
                DiscordMetadataValue value = "rocks";
                discord.transaction->set_metadata(discord.transaction, key, value);                 // Metadata
                discord.transaction->set_capacity(discord.transaction, cl.maxclients);              // Capacity
                discord.transaction->set_type(discord.transaction, DiscordLobbyType_Public);        // Lobby type (DiscordLobbyType_Private, DiscordLobbyType_Public)
                discord.transaction->set_locked(discord.transaction, false);                        // Locked

                // Create lobby
                discord.app.lobbies->create_lobby(discord.app.lobbies, discord.transaction, "create_lobby", OnLobbyConnect);
            }
        }
    }
}

void CL_DeleteDiscordLobby(void)
{
    if (discord.init && discord.lobby.id)
    {
        // Call CL_QueryLobby to build a list of lobbies before we delete it
        CL_QueryLobby();
        if (discord.result == DiscordResult_Ok)  // Result from CL_QueryLobby() -> search()
        {
            // Search for lobbies to destroy
            int32_t lobby_count = 0;
            DiscordLobbyId lobby_id;
            discord.app.lobbies->lobby_count(discord.app.lobbies, &lobby_count);
            if (lobby_count == 1)
            {
                // Get lobby id
                discord.app.lobbies->get_lobby_id(discord.app.lobbies, 0, &lobby_id);

                //Com_Printf("%s Ownership owner_id[%lld] user.id[%lld]\n", __func__, (long long)discord.lobby.owner_id, (long long)discord.user.id);

                // Transfer ownership of lobby
                if (discord.lobby.owner_id == discord.user.id) // If we own the lobby
                {
                    // Find a lobby member to transfer ownership to
                    int32_t member_count = 0;
                    DiscordUserId user_id = 0;
                    discord.app.lobbies->member_count(discord.app.lobbies, lobby_id, &member_count);
                    //Com_Printf("%s member_count %i\n", __func__, member_count);
                    for (int i = 0; i < member_count; i++)
                    {                       
                        discord.app.lobbies->get_member_user_id(discord.app.lobbies, lobby_id, i, &user_id);
                        //Com_Printf("%s found user_id %lld\n", __func__, (long long)user_id);
                        if (user_id && user_id != discord.user.id)
                            break;
                        else
                            user_id = 0;
                    }
                    
                    // If we found a member (other than ourself) to transfer ownership to
                    if (user_id)
                    {
                        //Com_Printf("%s Transferring ownership of lobby %lld from %lld to %lld\n", __func__, (long long)lobby_id, (long long)discord.user.id, (long long)user_id);
                        
                        discord.app.lobbies->get_lobby_update_transaction(discord.app.lobbies, lobby_id, &discord.transaction);
                        //if (discord.result == DiscordResult_Ok) // If get_lobby_update_transaction was okay
                        {
                            discord.transaction->set_owner(discord.transaction, user_id); // Transfer ownership
                            discord.app.lobbies->update_lobby(discord.app.lobbies, lobby_id, discord.transaction, "update_lobby", DiscordCallback); // Update lobby
                        
                            // Broadcast the change to the lobby
                            char msg[256];
                            Q_snprintf(msg, sizeof(msg) - 1, "%i|%lld|", DISCORD_MSG_OWNERSHIP, (long long)user_id);
                            discord.app.lobbies->send_lobby_message(discord.app.lobbies, discord.lobby.id, (uint8_t*)msg, strlen(msg), "Transfer ownership", DiscordCallback);
                        }
                    }
                    else // If no one else is in the lobby, delete it
                    {
                        // Destroy lobby
                        discord.app.lobbies->delete_lobby(discord.app.lobbies, lobby_id, "delete_lobby", DiscordCallback);
                        //Com_Printf("%s lobby[%lld] destroyed\n", __func__, (long long)lobby_id);
                    }
                }
 
                // Clear mem
                memset(&discord.transaction, 0, sizeof(discord.transaction));
                memset(&discord.lobby, 0, sizeof(discord.lobby));
            }
        }
    }
}

void CL_InitDiscord(void)
{
    Com_Printf("==== %s ====\n", __func__);

    // Creation
    memset(&discord.app, 0, sizeof(discord.app));
    memset(&discord.params, 0, sizeof(discord.params));

    // Events
    memset(&discord.users_events, 0, sizeof(discord.users_events));
    memset(&discord.activities_events, 0, sizeof(discord.activities_events));
    memset(&discord.relationships_events, 0, sizeof(discord.relationships_events));

    // Activities + Lobbies
    memset(&discord.activity, 0, sizeof(discord.activity));
    memset(&discord.transaction, 0, sizeof(discord.transaction));
    memset(&discord.lobby, 0, sizeof(discord.lobby));
    
    // On events
    discord.relationships_events.on_refresh = OnRelationshipsRefresh;
    discord.relationships_events.on_relationship_update = OnRelationshipUpdate;
    discord.users_events.on_current_user_update = OnUserUpdated;
    discord.activities_events.on_activity_invite = OnActivityInvite;
    discord.activities_events.on_activity_join = OnActivityJoin;
    discord.activities_events.on_activity_spectate = OnActivitySpectate;
    discord.activities_events.on_activity_join_request = OnActivityJoinRequest;
    discord.lobbies_events.on_lobby_update = OnLobbyUpdate;
    discord.lobbies_events.on_lobby_delete = OnLobbyDelete;
    discord.lobbies_events.on_member_connect = OnLobbyMemberConnect;
    discord.lobbies_events.on_member_update = OnLobbyMemberUpdate;
    discord.lobbies_events.on_member_disconnect = OnLobbyMemberDisconnect;
    discord.lobbies_events.on_lobby_message = OnLobbyMessage;
    discord.lobbies_events.on_speaking = OnLobbySpeaking;
    discord.lobbies_events.on_network_message = OnLobbyNetworkMessage;
    
    // Creation + Params
    DiscordCreateParamsSetDefault(&discord.params);
    discord.params.client_id = DISCORD_APP_ID;
    discord.params.flags = DiscordCreateFlags_NoRequireDiscord; // Does not require Discord to be running, use this on other platforms
    discord.params.event_data = &discord.app;
    discord.params.events = discord.events;
    discord.params.user_events = &discord.users_events;
    discord.params.activity_events = &discord.activities_events;
    discord.params.relationship_events = &discord.relationships_events;
    discord.params.lobby_events = &discord.lobbies_events;
    discord.params.network_events = discord.network_events;
    discord.params.overlay_events = discord.overlay_events;
    discord.params.storage_events = discord.storage_events;
    discord.params.store_events = discord.store_events;
    discord.params.voice_events = discord.voice_events;
    discord.params.achievement_events = discord.achievement_events;
    
    DiscordCallback(NULL, DiscordCreate(DISCORD_VERSION, &discord.params, &discord.app.core));

    if (discord.result == DiscordResult_Ok)
    {
        // Managers
        discord.app.application = discord.app.core->get_application_manager(discord.app.core);
        discord.app.user = discord.app.core->get_user_manager(discord.app.core);
        discord.app.images = discord.app.core->get_image_manager(discord.app.core);
        discord.app.activities = discord.app.core->get_activity_manager(discord.app.core);
        discord.app.relationships = discord.app.core->get_relationship_manager(discord.app.core);
        discord.app.lobbies = discord.app.core->get_lobby_manager(discord.app.core);
        discord.app.network = discord.app.core->get_network_manager(discord.app.core);
        discord.app.overlay = discord.app.core->get_overlay_manager(discord.app.core);
        discord.app.storage = discord.app.core->get_storage_manager(discord.app.core);
        discord.app.store = discord.app.core->get_store_manager(discord.app.core);
        discord.app.voice = discord.app.core->get_voice_manager(discord.app.core);
        discord.app.achievements = discord.app.core->get_achievement_manager(discord.app.core);

        // Steam
        discord.app.activities->register_steam(discord.app.activities, 1978800); // Aqtion Steam application ID

        // Command line
        //discord.app.activities->register_command(discord.app.activities, "action.exe +set game action");
        //discord.app.activities->register_command(discord.app.activities, "steam://run/1978800/connect/127.0.0.1:27910");

        // Logs
        discord.app.core->set_log_hook(discord.app.core, DiscordLogLevel_Debug, NULL /*void* hook_data*/, DiscordLogCallback);

        // Init
        discord.init = true;
        discord.server_hostname[0] = '\0';
        discord.last_discord_runtime = cls.realtime + DISCORD_UPDATE_MSEC;
        discord.last_activity_time = 0;
    }
    else // Could not connect to the discord network, or the discord app isn't running
    {
        // Init
        discord.init = false;
        discord.discord_found = false;
        discord.server_hostname[0] = '\0';
        discord.last_discord_runtime = 0;
        discord.last_activity_time = 0;

        Com_Printf("%s could not be initialized because Discord is not running\n", __func__);
    }
}

void CL_RunDiscord(void) // Run in main loop
{
    // Discord cvar disabled, or not running and connected to the Discord network
    if (cl_discord->value != 1 || discord.discord_found == false)
    {
        // If Discord was initialized previously, shut it down
        if (discord.init)
            CL_ShutdownDiscord();
        
        return;
    }
        
    // Ensure Discord is initialized
    if (discord.init == false)
    {
        CL_InitDiscord();
        return; // Give it time to fully init
    }

    // Run discord integration 
    if (cl_discord->value)
    {
        // Run discord callbacks. Must be run first, as per https://discord.com/developers/docs/game-sdk/networking#flush-vs-runcallbacks
        discord.app.core->run_callbacks(discord.app.core);
        
        if (discord.last_discord_runtime < cls.realtime)
        {
            // Update timer
            discord.last_discord_runtime = cls.realtime + DISCORD_UPDATE_MSEC;
            
            // Run discord callbacks. Must be run first, as per https://discord.com/developers/docs/game-sdk/networking#flush-vs-runcallbacks
            //discord.app.core->run_callbacks(discord.app.core);
            
            // Run activity
            CL_UpdateActivity();

            // Flushes the network. Must be run last, and after all networking messages.
            discord.app.network->flush(discord.app.network);
        }
    }
    else // Shutdown discord integration
    {
        CL_ShutdownDiscord();
    }
}

static void CL_ClearDiscordAcivity(void)
{
    discord.app.activities->clear_activity(discord.app.activities, "clear_activity", DiscordCallback);
    memset(&discord.activity, 0, sizeof(discord.activity));
}

void CL_ShutdownDiscord(void)
{
    Com_Printf("==== %s ====\n", __func__);
    CL_ClearDiscordAcivity();
    CL_DeleteDiscordLobby();

    discord.app.core->run_callbacks(discord.app.core);
    discord.app.core->destroy(discord.app.core);
    discord.discord_found = true;
    discord.init = false;
}
//=====================================================================================================
#endif //rekkie -- discord -- e
//=====================================================================================================
