# Manual Installation of Steamworks SDK for steamshim.cpp

To successfully build and run `steamshim.cpp`, you need to manually install the latest Steamworks SDK. Follow the steps below to ensure the SDK is properly installed:

1. **Download the Steamworks SDK**: Visit the [Steamworks SDK](https://partner.steamgames.com/downloads/steamworks_sdk.zip) page and download the latest version of the SDK.

2. **Extract the SDK**: Once the download is complete, extract the contents of the ZIP file to a location on your computer.

3. **Locate the SDK Files**: Inside the extracted folder, you will find various files and directories related to the Steamworks SDK. Make note of the path to this folder.

4. **Configure the Build System**: Open the build configuration file for `steamshim.cpp` and locate the section where the Steamworks SDK path is specified. Update the path to match the location of the SDK files on your computer.

5. **Build and Run**: With the SDK properly installed and the build configuration updated, you can now build and run `steamshim.cpp`. Make sure to follow any additional instructions or dependencies mentioned in the code or project documentation.

By following these steps, you will ensure that `steamshim.cpp` has access to the necessary Steamworks SDK files during the build process. This will enable the code to interact with the Steam platform and utilize its features effectively.


## Building and Running steamshim.cpp

### Mac OS X (Darwin)
```
CFLAGS=-mmacosx-version-min=10.7 clang++ -o aqtion -Wall -O0 -ggdb3 -Dnullptr=0 steamshim.cpp -I steam/sdk/public/steam steam/sdk/redistributable_bin/osx/libsteam_api.dylib
```

### Linux (x86_64)
```
g++ -Wl,-rpath='$ORIGIN' -o aqtion -Wall -O0 -ggdb3 steamshim.cpp steam/sdk/redistributable_bin/linux64/libsteam_api.so -I steam/sdk/public/steam/ -no-pie
```

### Windows (x86_64)
**Requires Visual Studio 2019 or better (v142)**
```
msbuild steamshim.vcxproj /p:IncludePath="steam/sdk/public/steam" /p:configuration=release
```