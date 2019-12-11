# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Add version number to tools, such as xenon-dm -v, derived from xenon/config.h
- Upgrade 36.331 to r15
- Build: CI testing with Travis now
- Build: Using c++17 and no longer require boost for tests
- Build: make install supported under MacOS/Linux
- Build: Preliminary Windows support for build and test, static library only
- Build: New make rule to build for xcode: make xcode
 
### Changed

- Doc: Updated README
- Doc: Generated XDDL documentation updated
- Tools: Renamed idm to xenon-dm and xv to xenon-xv
- Tools: xenon-dm now looks in the default data dir for xddl files by default

### Fixed

- xddl: fixed bug with switch elements that had an empty last case
- 3GPP/24.008: Fixed Code List decode length bug.

## Older versions

1. [2.2.0] - 2016-9-17
2. [2.1.2] - 2016-8-19
3. [2.1.1] - 2016-8-10
4. [2.1.0] - 2016-8-8
5. [2.0.0] - 2016-7.22
6. [1.0.0] - 2016-7-22

[Unreleased]: https://github.com/intrig/xenon/releases/v2.3.0...HEAD
[2.2.0]: https://github.com/intrig/xenon/releases/tag/v2.2.0
[2.1.2]: https://github.com/intrig/xenon/releases/tag/v2.1.2
[2.1.1]: https://github.com/intrig/xenon/releases/tag/v2.1.1
[2.1.0]: https://github.com/intrig/xenon/releases/tag/v2.1.0
[2.0.0]: https://github.com/intrig/xenon/releases/tag/v2.0.0
[1.0.0]: https://github.com/intrig/xenon/releases/tag/v1.0.0
