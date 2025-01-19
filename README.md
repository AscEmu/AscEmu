# ![logo](http://ascemu.org/images/logo.png)

Master                                                      | Development
:---------------------------------------------------------: | :--------------------------------------------------------------:
[![AscEmuClangCI][AscEmuClangBadge]][AscEmuBadgeClangUrl]   | [![AscEmuClangCI][AscEmuClangBadge]][AscEmuBadgeClangUrl]      |
[![AscEmuGccCI][AscEmuGccBadge]][AscEmuBadgeGccUrl]         | [![AscEmuGccCI][AscEmuGccBadge]][AscEmuBadgeGccUrl]            |
[![MasterAppYeyor][MasterAppYeyorBadge]][MasterAppYeyorUrl] | [![DevelopAppYeyor][DevelopAppYeyorBadge]][DevelopAppYeyorUrl] |

## Introduction
AscEmu is derived from ArcEmu to keep up the Antrix-Ascent-Arcemu way of Framework.
We focus on optimizing the codebase and improving the ingame functionality.

You can help us by contributing. It is completely open source and can be used by everyone.
This project is for educational purpose. So, if you're looking for serverfiles to run your server or create some "custom" scripts  this is the wrong place for you. If you want to discuss/develop/work on an open source project and on important stuff for the framework feel free to join our community.

Discord                                | Codefactor                                      | Openhub
:------------------------------------: | :---------------------------------------------: | :--------------------------------------:
[![Discord][DiscordBadge]][DiscordUrl] | [![Codefactor][CodefactorBadge]][CodefactorUrl] | [![Openhub][OpenhubBadge]][OpenhubUrl] |

## Multiversion
AscEmu supports several versions as listed below. We achieved to handle different versions of the serverside code in one repo. The biggest advantage of that is that we always develop for all our supported versions without wasting time to merge features across all repos.

Beside our multiversion-core we developed our world database with this attitude. Compared to other projects we are still a small project but with one core and database we are able to work on different versions at the same time. Beside all that we are able to show the changes to the game side by side in our code and database.

Description      | Classic            | TBC                | WotLK              | Cata               | MoP
:--------------: | :----------------: | :----------------: | :----------------: | :----------------: | :------------:
Authentification | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
Worldsocket      | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
Char Enum        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
Log into world   | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :x:

## Contributing
Feel free to contribute any corrections but make sure it is useful and tested before opening PRs.

What are "useful" commits?
 1. Fixing an [open issue](https://github.com/AscEmu/AscEmu/issues).
 2. Related to the milestones.
 3. Making the Framework stable/safer
 4. Adding blizzlike related functions

We strive to make all in-game content to be blizzlike!

Feel free to join our [Discord server](https://discord.com/invite/CBdgrh7).

Click on the "⭐ Star" button to help us gain more visibility on Github!

## Opening new issues
Be patient with us and give us details.
 1. How to reproduce the issue
 2. How should it work
 3. Images will help us a lot

## Install
HowTo install - detailed guides on our wiki.
* [Linux](https://ascemu.github.io/Wiki/docs/installation/linux/)
* [Windows](https://ascemu.github.io/Wiki/docs/installation/windows/)

## Links
* [Web](http://www.ascemu.org)
* [Forums](https://github.com/AscEmu/AscEmu/discussions)
* [Wiki](https://ascemu.github.io/Wiki/)
* [World DB](https://github.com/AscEmu/OneDB)

## Copyright and other stuff
* [License](LICENSE.md)
* [Thanks to all](THANKS.md)
* [Terms of use](TERMS_OF_USE_AGREEMENT.md)

## Special Thanks
[JetBrains](https://www.jetbrains.com/) - For supporting Open Source projects.

## SAST Tools
[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - Static analyzer for C, C++, C#, and Java code.

<!-- Undercover:start:status -->
[AscEmuGccBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/gcc-linux-test-x64.yml/badge.svg
[AscEmuClangBadge]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-linux-test-x64.yml/badge.svg

[MasterAppYeyorBadge]: https://ci.appveyor.com/api/projects/status/h70t5a5rd56y8ute/branch/master?svg=true
[DevelopAppYeyorBadge]: https://ci.appveyor.com/api/projects/status/h70t5a5rd56y8ute/branch/develop?svg=true

[MasterAppYeyorUrl]: https://ci.appveyor.com/project/Zyres/ascemu
[DevelopAppYeyorUrl]: https://ci.appveyor.com/project/Zyres/ascemu

[AscEmuBadgeGccUrl]: https://github.com/AscEmu/AscEmu/actions/workflows/gcc-linux-test-x64.yml
[AscEmuBadgeClangUrl]: https://github.com/AscEmu/AscEmu/actions/workflows/clang-linux-test-x64.yml
<!-- Undercover:end:status -->

<!-- Undercover:start:community -->
[DiscordBadge]: https://user-images.githubusercontent.com/1216225/168970774-1c2c4b77-64e5-489d-a2ae-0a02e3983479.svg
[CodefactorBadge]: https://www.codefactor.io/repository/github/ascemu/ascemu/badge
[OpenhubBadge]: https://www.openhub.net/p/AscEmu/widgets/project_thin_badge.gif

[DiscordUrl]: https://discord.com/invite/CBdgrh7
[CodefactorUrl]: https://www.codefactor.io/repository/github/ascemu/ascemu
[OpenhubUrl]: https://www.openhub.net/p/AscEmu
<!-- Undercover:end:community -->

