#include "sdk/public/steam/steam_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define EXECUTABLE "q2pro.exe"
#else
#define EXECUTABLE "./q2pro"
#endif

// Maximum length of user-supplied parameters is 2048 characters
#define MAX_USER_PARAMS_LENGTH 2048

int main(int argc, char* argv[]) {
    if (SteamAPI_Init()) {
        // Get the SteamID using the Steamworks SDK
        CSteamID steamId = SteamUser()->GetSteamID();
        char steamIdStr[64];
        snprintf(steamIdStr, sizeof steamIdStr, "%llu", steamId.ConvertToUint64());

        // Prepare the command
        char command[2048 + 256]; // Extra space for the executable name and the steamid
        snprintf(command, sizeof command, "%s +set steamid %s", EXECUTABLE, steamIdStr);

        // Append additional user-supplied parameters
        int userParamsLength = 0;
        for (int i = 1; i < argc; ++i) {
            userParamsLength += strlen(argv[i]);
            if (userParamsLength > MAX_USER_PARAMS_LENGTH) {
                fprintf(stderr, "Error: User-supplied parameters are too long\n");
                return 1;
            }
            strncat(command, " ", sizeof command - strlen(command) - 1);
            strncat(command, argv[i], sizeof command - strlen(command) - 1);
        }

        printf("Executing command: %s\n", command);
        system(command);

        SteamAPI_Shutdown();
    } else {
        printf("Failed to initialize Steam API\n");
    }

    return 0;
}