#include <stdio.h>
#include <stdint.h>
#ifdef _WIN32
#include <process.h>
#include <windows.h>
#include <tchar.h>  // tchar_t
#define WIN32_LEAN_AND_MEAN 1
#else
#include <unistd.h>
#endif
#include "extern/steam/sdk/public/steam/steam_api.h"

int main(int argc, char* argv[]) {

    // Initialize the Steamworks SDK
    if (!SteamAPI_Init()) {
        fprintf(stderr, "Failed to initialize Steamworks SDK\n");
        return 1;
    }

    // Get the user's Steam ID and Cloud storage settings
    uint64_t steamId = SteamUser()->GetSteamID().ConvertToUint64();

#ifdef _WIN32
    // Create a pipe for communication with the child process
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    char buffer[sizeof(uint64_t)] = { 0 };
    memcpy(&buffer, &steamId, sizeof(uint64_t));

    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        printf("Could not create pipe\n");
        return 1;
    }

    DWORD bytesWritten;
    WriteFile(hPipeWrite, buffer, sizeof(uint64_t), &bytesWritten, NULL);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    if (!CreateProcess(NULL, // No module name (use command line).
        "Q2Pro.exe",    // Command line.
        NULL,           // Process handle not inheritable.
        NULL,           // Thread handle not inheritable.
        TRUE,           // Set handle inheritance to FALSE.
        0,              // No creation flags.
        NULL,           // Use parent's environment block.
        NULL,           // Use parent's starting directory.
        &si,            // Pointer to STARTUPINFO structure.
        &pi))           // Pointer to PROCESS_INFORMATION structure.
    {      
        printf("Failed to create q2pro.exe process.\n");
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
#else
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

    // Prepare the arguments for the child process
    char* args[argc + 1];
    char q2pro[] = "./q2pro";
    args[0] = q2pro;

    for (int i = 1; i < argc; ++i)
        args[i] = argv[i];

    args[argc] = NULL; // The argument list must be NULL-terminated

    // Launch the child process
    execv("./q2pro", args);

    return 0;
#endif
}