# ![logo](http://ascemu.org/images/logo.png)

|                                   Master                                  |                                 Development                                |
| :-----------------------------------------------------------------------: | :------------------------------------------------------------------------: |
| [![AscEmuClangCI][AscEmuClangMasterMacOSBadge]][AscEmuBadgeClangMacOSUrl] | [![AscEmuClangCI][AscEmuClangDevelopMacOSBadge]][AscEmuBadgeClangMacOSUrl] |
| [![AscEmuClangCI][AscEmuClangMasterLinuxBadge]][AscEmuBadgeClangLinuxUrl] | [![AscEmuClangCI][AscEmuClangDevelopLinuxBadge]][AscEmuBadgeClangLinuxUrl] |
|    [![AscEmuGccCI][AscEmuGccMasterLinuxBadge]][AscEmuBadgeGccLinuxUrl]    |    [![AscEmuGccCI][AscEmuGccDevelopLinuxBadge]][AscEmuBadgeGccLinuxUrl]    |
|        [![AppYeyorMaster][AppYeyorMasterBadge]][AppYeyorMasterUrl]        |       [![AppYeyorDevelop][AppYeyorDevelopBadge]][AppYeyorDevelopUrl]       |

## Introduction

AscEmu is derived from ArcEmu and continues the Antrix-Ascent-ArcEmu approach to MMO server framework development.

Our focus is on maintaining a clean and extensible codebase, improving in-game functionality, and supporting multiple World of Warcraft client generations from a single repository.

You can help us by contributing. AscEmu is completely open source and available to everyone.

This project is intended for educational and development purposes. If you are looking for ready-to-run server files or a framework solely for creating custom scripts, this is probably not the right project for you. If you are interested in discussing, developing, reverse engineering protocols, improving emulator architecture, or contributing to an open-source project, feel free to join our community.

|                 Discord                |                    Codefactor                   |                 Openhub                |
| :------------------------------------: | :---------------------------------------------: | :------------------------------------: |
| [![Discord][DiscordBadge]][DiscordUrl] | [![Codefactor][CodefactorBadge]][CodefactorUrl] | [![Openhub][OpenhubBadge]][OpenhubUrl] |

## Multiversion

AscEmu supports multiple World of Warcraft client generations within a single codebase.

Instead of maintaining separate repositories for every expansion, version-specific behavior is isolated where required while shared systems remain part of the common core. This allows improvements to be developed once and reused across supported versions whenever possible.

Our world database follows the same multiversion approach. This makes it possible to maintain and compare data and implementation differences between client generations without duplicating the entire project.

### Supported versions

|     Description    | Classic | TBC | WotLK | Cata | MoP | Forever |
| :----------------: | :-----: | :-: | :---: | :--: | :-: | :-----: |
|   Authentication   |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    ✔️   |
|    World socket    |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    ✔️   |
|   Character list   |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    ✔️   |
| Character creation |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    ✔️   |
|   Log into world   |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    ✔️   |
|   Object updates   |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    🚧   |
|      Movement      |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    🚧   |
|      Gameplay      |    ✔️   |  ✔️ |   ✔️  |  ✔️  |  ✔️ |    ❌    |

> [!NOTE]
> **Forever support is experimental and under active development.**
>
> The current implementation targets **World of Warcraft Forever 1.60.1 build 69893** (`ClientVersion = 12`).
>
> Authentication, the World V2 connection, character handling and entering the world are working. Object updates, movement, visibility, DB2/WDC5 data and gameplay integration are still being validated and should be considered work in progress.
>
> Forever uses the modern Battle.net and World protocols and follows a separate protocol path from the legacy authentication and packet handling used by older AscEmu versions.

### Forever development status

The following parts of the Forever client flow are currently implemented or under active development:

* ✔️ Battle.net TLS connection
* ✔️ Battle.net authentication flow
* ✔️ Realm list
* ✔️ Realm selection and World connection
* ✔️ Modern World V2 socket protocol
* ✔️ World authentication
* ✔️ Character enumeration
* ✔️ Character appearance and customization data
* ✔️ Character creation flow
* ✔️ Entering the world
* ✔️ Initial player self-create object update
* ✔️ Basic Player, Unit and ActivePlayer update-field serialization
* ✔️ Core player values such as health, level and experience
* 🚧 Dynamic object update handling
* 🚧 Update manager integration
* 🚧 Creature and GameObject update support
* 🚧 Visibility and surrounding object creation
* 🚧 Player movement packet handling
* 🚧 Creature spline movement
* 🚧 Random, waypoint and chase movement
* 🚧 Walk/run movement synchronization
* 🚧 Runtime movement synchronization
* 🚧 Complete PlayerData / UnitData / ActivePlayerData coverage
* 🚧 DB2/WDC5 data loading and version-specific structures
* 🚧 Item and inventory support
* 🚧 NPC interaction
* ❌ Complete skill system
* ❌ Complete spell system
* ❌ Complete chat support
* ❌ Full script support
* ❌ Full gameplay support

Legend:

* ✔️ Implemented
* 🚧 Experimental / work in progress
* ❌ Not implemented yet

Forever should currently be considered a **protocol and core-development target**, not a complete playable server version.

The current goal is to establish a clean version-specific foundation for modern clients instead of introducing compatibility hacks into the legacy core. Once the fundamental authentication, world protocol, object update, movement and data systems are stable, gameplay systems can be enabled incrementally.

## Contributing & Issues

> [!IMPORTANT]
> Before opening a new issue or submitting a contribution, please read [CONTRIBUTING.md](CONTRIBUTING.md).

Following the guidelines helps us process requests and review changes more efficiently.

Forever is currently an active development area. Contributions related to modern Battle.net authentication, packet structures, update fields, movement, DB2/WDC5 handling, and version abstraction are welcome.

## Install

Detailed installation guides are available on our wiki.

* [Linux](https://ascemu.github.io/Wiki/docs/installation/linux/)
* [macOS](https://ascemu.github.io/Wiki/docs/installation/macOS/)
* [Windows](https://ascemu.github.io/Wiki/docs/installation/windows/)

## Useful links

* [Web](http://www.ascemu.org)
* [Wiki](https://ascemu.github.io/Wiki/)
* [World DB](https://github.com/AscEmu/OneDB)
* [Forums](https://github.com/AscEmu/AscEmu/discussions)

## Copyright and other stuff

* [License](LICENSE.md)
* [Thanks to all](THANKS.md)
* [Terms of use](TERMS_OF_USE_AGREEMENT.md)

## Special Thanks

* [JetBrains](https://www.jetbrains.com/) - For supporting open-source projects.
* [PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - Static analyzer for C, C++, C#, and Java code.

## What's next?

> [!IMPORTANT]
> **Click on the "⭐ Star" button to help us gain more visibility on GitHub!**

We're always happy to receive feedback and contributions, whether it's a bug report, a feature idea, documentation improvement, or pull request.

Feel free to join our [Discord server](https://discord.com/invite/CBdgrh7).

<!-- Undercover:start:status -->

[AscEmuClangMasterMacOSBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-macos-test-x64.yml/badge.svg?branch=master
[AscEmuClangDevelopMacOSBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-macos-test-x64.yml/badge.svg?branch=develop
[AscEmuClangMasterLinuxBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-linux-test-x64.yml/badge.svg?branch=master
[AscEmuClangDevelopLinuxBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-linux-test-x64.yml/badge.svg?branch=develop
[AscEmuGccMasterLinuxBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/gcc-linux-test-x64.yml/badge.svg?branch=master
[AscEmuGccDevelopLinuxBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/gcc-linux-test-x64.yml/badge.svg?branch=develop
[AppYeyorMasterBadge]: https://ci.appveyor.com/api/projects/status/h70t5a5rd56y8ute/branch/master?svg=true
[AppYeyorDevelopBadge]: https://ci.appveyor.com/api/projects/status/h70t5a5rd56y8ute/branch/develop?svg=true
[AppYeyorMasterUrl]: https://ci.appveyor.com/project/Zyres/ascemu
[AppYeyorDevelopUrl]: https://ci.appveyor.com/project/Zyres/ascemu
[AscEmuBadgeGccLinuxUrl]: https://github.com/AscEmu/AscEmu/actions/workflows/gcc-linux-test-x64.yml
[AscEmuBadgeClangLinuxUrl]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-linux-test-x64.yml
[AscEmuBadgeClangMacOSUrl]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-macos-test-x64.yml

<!-- Undercover:end:status -->

<!-- Undercover:start:community -->

[DiscordBadge]: https://user-images.githubusercontent.com/1216225/168970774-1c2c4b77-64e5-489d-a2ae-0a02e3983479.svg
[CodefactorBadge]: https://www.codefactor.io/repository/github/ascemu/ascemu/badge
[OpenhubBadge]: https://www.openhub.net/p/AscEmu/widgets/project_thin_badge.gif
[DiscordUrl]: https://discord.com/invite/CBdgrh7
[CodefactorUrl]: https://www.codefactor.io/repository/github/ascemu/ascemu
[OpenhubUrl]: https://www.openhub.net/p/AscEmu

<!-- Undercover:end:community -->
