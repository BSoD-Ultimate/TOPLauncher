# Building instructions

## Requirements

To build this project, make sure these applications are correctly installed on your computer:
- **Visual Studio 2017 or later (or Visual C++ Build Tools)**: When installing Visual Studio, make sure you checked the option 'Desktop development with C++'.
- **CMake**: This project uses CMake to configure and build the project. Version 3.14.0 or later is required.
- **Qt**: The project relies on the Qt library to implement its GUIs. Currently, the version of Qt used by the project is 5.9.8. Qt 5.12 might also be compatible. Binaries released are using the static library version of Qt.

## Building

### Configure

The following configure procedure assumes Visual Studio 2017 is installed on your computer.

- Open the shortcut 'Visual Studio 2017/x86 Native Tools Command Prompt for VS 2019' in the Start Menu. A Command Prompt window which environment variables related to the Visual C++ compiler is configured will be open.
- Firstly, please switch the current working directory to the project's root directory.
- Use CMake to configure the project. Set the variable `Qt5_DIR` as your Qt installation folder. The configure command looks like this:
  ```
  cmake -D "Qt5_DIR=C:\Qt\Qt5.9.8\5.9.8\msvc2015\lib\cmake\Qt5" -D "CMAKE_INSTALL_PREFIX=<the directory you wish to install binaries>"-G "Visual Studio 15 2017" -A "Win32" .
  ```

  - By default, the Qt installer does not provide static library binaries. Only the shared library version is available. You have to build the static library version manually. If you wish to use the static library version of Qt as binaries in the releases do, you could build the static library version of Qt using instructions provided by this blog post: <https://retifrav.github.io/blog/2018/02/17/build-qt-statically/>
  - To use the Qt static library, add the following command in CMake options sections (before the ".") so that the linker could link additional libraries that statically link Qt library requires.
    ```
    -D "TOPLAUNCHER_USE_STATIC_QT_LIB=ON"
    ```

- CMake will automatically generate Visual Studio Solutions, which will be used in building.

### Build & Install

Run `msbuild` for binaries:
```
msbuild TOPLauncher.sln -p:Configuration=RelWithDebInfo;Platform=Win32
```

Install binaries into the directory set in the variable `CMAKE_INSTALL_PREFIX`:
```
msbuild INSTALL.vcxproj -p:Configuration=RelWithDebInfo;Arch=Win32
```

## Adding translations

To add a new language translation for the executable. Do the following steps:

- Open the file `TOPLauncher/CMakeLists.txt`, then find the text `lupdate`. You will find the following code snippets similar to this:
  ```
  # pre-build & post-build commands

  # Qt5 create/update translations
  add_custom_command(
    TARGET TOPLauncher
    PRE_BUILD
    COMMAND "${QT5_BIN_DIR}/lupdate" "${CMAKE_CURRENT_SOURCE_DIR}" "-locations" "none" "-no-ui-lines" "-target-language" "en_US" "-ts" "Resources/translations/en_US.ts"
  )

  # recover "vanished" translations
  add_custom_command(
    TARGET TOPLauncher
    PRE_BUILD
    COMMAND "${QT5_BIN_DIR}/lconvert" "Resources/translations/en_US.ts" "-o" "Resources/translations/en_US.ts"
  )
  ```

- `lupdate` and `lconvert` are tools provided by Qt. When executed, `lupdate` inspects all `ui` files and C++ source files, finding all occurrences of the `QObject::tr` function. After inspecting, the tool generates or updates the XML file passed by the `-ts` argument from the information previously gathered. 
The generated XML file might contain some redundant translation entries. The `lconvert` tool is used to remove these redundant translation entries in the XML file.
The two tools are invoked every time you compile the project before the compilation starts so that every literal string needs to be translated will be covered.
- To add a new language translation file, you have to add a new command in the `add_custom_command` method following the existing commands' format. For instance, to create the Korean translation file, add the following command in the two `add_custom_command` methods:
  ```
  COMMAND "${QT5_BIN_DIR}/lupdate" "${CMAKE_CURRENT_SOURCE_DIR}" "-locations" "none" "-no-ui-lines" "-target-language" "ko_KR" "-ts" "Resources/translations/ko_KR.ts"

  COMMAND "${QT5_BIN_DIR}/lconvert" "Resources/translations/ko_KR.ts" "-o" "Resources/translations/ko_KR.ts"
  
  ```
- The next time you build the project, these tools will generate a translation file for the Korean translation: `Resources/translations/ko_KR.ts`.
- To make the translation file get accessed by the application, you should embed the translation file into the application. To achieve this, add a file entry for the translation file in the `Resources/TOPLauncher.qrc`:
  ```
  <file>translations/ko_KR.ts</file>
  ```
- Finally, do not forget to add the new translation file into the Git Version Control!
- You are now able to create the Korean translation by editing the `ko_KR.ts` file, either directly editing the XML file or opening the file using `linguist.exe` provided by Qt. `linguist.exe` is more recommended.