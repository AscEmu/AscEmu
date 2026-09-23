# ![logo](http://ascemu.org/images/logo.png)

# I'm in! What now?

Welcome to AscEmu, and thank you for joining us! 👋 We're glad you've become part of the AE community.

AscEmu is a multiversion emulator project supporting several World of Warcraft client generations from a shared codebase. Alongside the established legacy versions, development is also ongoing for modern client support through **Forever**.

## What you’ll need

### 🖥 Operating System

* Linux (Ubuntu 22.04+, Debian 13+)
* macOS 13+
* Windows 10 / 11

### 🗂️ Database

* MySQL 8.0+

### 📃 Compiler

* GCC 13+
* Clang 17+
* MSVC 2022

### 📦 Build Tools

* CMake 4.1+
* OpenSSL 3.0+
* Git

## 🐛 Opening New Issues

Found a bug? Have an idea? We'd love to hear from you!

* Use our [issue templates](https://github.com/AscEmu/AscEmu/issues/new/choose) to provide the right information.
* Include clear steps to reproduce the issue.
* Mention the affected client version or expansion.
* Describe how the feature or behavior is expected to work on the corresponding official client whenever possible.
* Include relevant build numbers when reporting version-specific issues.
* Screenshots, logs, packet traces, crash reports, and reproduction steps can be a huge help! 📸
* For Forever-related reports, please clearly state that the issue affects the experimental Forever implementation.
* Not sure if your report is valid? Don't hesitate to [open an issue](https://github.com/AscEmu/AscEmu/issues/new/choose) — we'll help clarify it.

Please be patient and provide as much relevant information as possible.

Your reports help us improve AscEmu! ⭐

## 🚀 Contributing

We welcome contributions of all kinds. Please make sure your changes are useful, tested, and aligned with the project's goals.

### What makes a useful contribution?

* Fixing an [open issue](https://github.com/AscEmu/AscEmu/issues)
* Contributing to an existing **Milestone**
* Improving the stability, security, or reliability of the framework
* Adding Blizzlike functionality
* Improving multiversion support
* Improving version-specific protocol handling
* Improving documentation, testing, tooling, or developer experience
* Improving modern client support, including Forever
* Refactoring duplicated version-specific logic into reusable systems where appropriate

### Forever contributions

Forever is an experimental development target for modern World of Warcraft clients.

Useful areas for contributions currently include:

* Battle.net protocol support
* World authentication
* Opcode handling
* Character list and creation
* Modern object update serialization
* PlayerData, UnitData, and ActivePlayerData structures
* Movement packets and movement synchronization
* Visibility and object creation
* Creature and GameObject updates
* UpdateManager integration
* DB2 / WDC5 loading
* Version-specific data structures
* Packet parsing and protocol research
* Skills, spells, chat, inventory, and other gameplay systems

Changes for Forever should avoid introducing modern-client assumptions into legacy code paths unless the functionality is genuinely shared.

Whenever possible, keep version-specific implementations isolated behind the appropriate version abstractions.

### Pull Requests

When opening a pull request:

* Describe what you changed and **why**
* Mention the affected expansion or client version
* Explain how your changes were tested
* Include relevant logs or packet information where useful
* Keep changes focused and easy to review
* Avoid unrelated formatting or cleanup changes inside functional pull requests

For Forever changes, it is especially helpful to document:

* Client build used for testing
* Relevant opcode changes
* Packet structure assumptions
* Update-field or DB2 structure changes
* Whether the implementation is confirmed, partially confirmed, or still experimental

No perfection is required — we'll help you improve your contribution.

We appreciate every contribution! 😃

> [!IMPORTANT]
> **If you want to contribute to the project, you will find many resources that will guide you in our [wiki](https://ascemu.github.io/Wiki/).**

## Project Status

The current implementation status for supported client versions, including Forever, is maintained in the main [README](README.md).

Keeping the status in one place helps prevent outdated or conflicting information across project documentation.

## Resources

* [GitHub's Hello World Guide](https://docs.github.com/en/get-started/using-github/hello-world) (source: docs.github.com)
* [Using Pull Requests](https://docs.github.com/en/pull-requests/reference/pull-requests) (source: docs.github.com)

AscEmu — never ending place to work.
