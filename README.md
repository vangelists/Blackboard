Blackboard
==========

A thread-safe event system that supports both synchronous and asynchronous events, based on the Blackboard design pattern.

This project constituted my bachelor's thesis and is not actively maintained, although, if you happen to find a bug, let me know by filing an issue.

## Building with Ninja

    $ mkdir <build_directory>
    $ cd <build_directory>
    $ cmake <project_source_directory> -G Ninja
    $ cmake --build .

## Generating Xcode project

    $ mkdir <build_directory>
    $ cd <build_directory>
    $ cmake <project_source_directory> -G Xcode
    $ open Blackboard.xcodeproj

## License

Licensed under the [Mozilla Public License 2.0](LICENSE).

## Contact

Vangelis Tsiatsianas - [contact@vangelists.com](mailto:contact@vangelists.com?subject=[GitHub]%20Blackboard)
