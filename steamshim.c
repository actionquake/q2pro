#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#ifdef _WIN32
#include <process.h>
#endif
#include "extern/steam/sdk/public/steam/steam_api.h"

int main() {
    // Initialize the Steamworks SDK
    if (!SteamAPI_Init()) {
        fprintf(stderr, "Failed to initialize Steamworks SDK\n");
        return 1;
    }

    // Get the user's Steam ID and Cloud storage settings
    uint64_t steamId = SteamUser()->GetSteamID().ConvertToUint64();

    // Create a pipe for communication with the child process
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    // Write the Steam ID to the pipe
    write(pipefd[1], &steamId, sizeof(steamId));
    close(pipefd[1]); // Close the write end of the pipe

    // Replace the stdin of the child process with the read end of the pipe
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]); // Close the original file descriptor

    // Launch the child process
    #ifdef _WIN32
        _spawnl(_P_NOWAIT, ".\\q2pro.exe", NULL);
    #else
        execl("./q2pro", "q2pro", NULL);
    #endif

    return 0;
}