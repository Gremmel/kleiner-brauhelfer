# Deployment
## Compilation
Open ./source/brauhelfer.pro and check/edit the version
```
VERSION = 1.4.4.0
VERSION_INT = 1040400
```
Open ./source/definitionen.h and make sure the DEBUG line is commented
```
//#define DEBUG true
```
Build application with Qt 5.3 (with Qt Creator) for deployment
```
platform specific
```
Update and compile the language files
```
lupdate source/brauhelfer.pro
lrelease source/brauhelfer.pro
```
## Windows
Run the deploy script
```
.\deployment\windows\deploy.cmd
```
Select the folder containing the compiled application EXE file.
Select the Qt bin folder used to compile the application EXE file.

In the Inno Setup Compiler edit the definitions
```
#define MyAppVersion "1.4.4.0"
#define MyAppOutFileName "kb_setup_qt_v1_4_4_0"
```
## Debian
Run the deploy script
```
chmod +x ./deployment/debian/deploy.sh
./deployment/debian/deploy.sh <path_to_kleiner-brauhelfer_executable> <path_to_qt_bin>
Example:
./deployment/debian/deploy.sh ./build/bin/kleiner-brauhelfer /opt/Qt/5.3/gcc_64/bin/
```
In nano edit the version and architecture and save file
```
Version: 1.4.4.0
Architecture: i386 / amd64
```
## macOS
Run the deploy script
```
chmod +x ./deployment/macOS/deploy.sh
./deployment/macOS/deploy.sh <path_to_kleiner-brauhelfer_bundle> <path_to_qt_bin>
Example:
./deployment/macOS/deploy.sh ./source/bin/kleiner-brauhelfer.app /opt/Qt/5.10.0/clang_64/bin
```
