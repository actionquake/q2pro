#include "client.h"

void CL_Steam_Init(void) {
    // Read the Steam ID from the pipe
    uint64_t steamId = 0;
    ssize_t result = read(STDIN_FILENO, &steamId, sizeof(steamId));

    // Check if read returned an error
    if (result == -1) {
        Com_Printf("Failed to read Steam ID from pipe: %s\n", strerror(errno));
        return;
    }

    // Check if the correct number of bytes were read
    if (result != sizeof(steamId)) {
        Com_Printf("Failed to read the correct number of bytes from the pipe\n");
        return;
    }

    // Now you can use steamId
    Cvar_Set("steamid", va("%lu", steamId));
}