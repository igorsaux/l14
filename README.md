# L14

A command line utility for messing with Space Station 14 clients and server API.

## Usage

You can just pass commands directly through args, pass files that contain them or in interactive mode:

```sh
l14cmd.exe +fetch_servers https://hub.spacestation14.com/ +quit
```

or:

```sh
l14cmd.exe +exec fetch_servers.txt
```

```
// fetch_servers.txt
fetch_servers https://hub.spacestation14.com/
quit
```

or:

```sh
l14cmd.exe
fetch_servers https://hub.spacestation14.com/
quit
```

### Console variable

There are a bunch of variables that you can change:

```sh
l14cmd.exe @HttpTimeout 2000 @StrictHash false ...
```

The same syntax applies to "in-file" scripts.

### Other

You can pass `/noupdate` as the last argument to skip checking for updates.

## Building

- A compiler that supports C++ 20 is required
- Ninja
- Cmake
- Vcpkg

```sh
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=<VCPKG>/scripts/buildsystems/vcpkg.cmake -B ./cmake-build/
```
