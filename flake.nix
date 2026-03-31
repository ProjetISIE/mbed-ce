{
  description = "Nix flake MBED development environment";
  nixConfig = {
    extra-substituters = [ "https://cache.garnix.io" ];
    extra-trusted-public-keys = [ "cache.garnix.io:CTFPyKSLcx5RMJKfLo5EEPUObbA78b0YQ2DTCJXqr9g=" ];
  };
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      localSystems = [
        "x86_64-linux"
        "aarch64-darwin"
      ];
      forSystems = f: nixpkgs.lib.genAttrs localSystems (system: f (import nixpkgs { inherit system; }));
    in
    {
      devShells = forSystems (pkgs: {
        default =
          pkgs.mkShell.override
            {
              stdenv = pkgs.clangStdenv; # Clang instead of GCC
            }
            {
              packages =
                with pkgs;
                [
                  bashInteractive
                  clang-tools # Clang CLIs, including LSP
                  # clang-uml # UML diagram generator
                  cmake-format # CMake formatter
                  cmake-language-server # Cmake LSP
                  # doxygen # Documentation generator
                  # lldb # Clang debug adapter
                  mbed-cli
                  # socat # Serial terminal for manual testing
                  # valgrind # Debugging and profiling
                  # ARM Toolchain
                  gcc-arm-embedded
                  gcovr
                  gdb
                  # Python
                  python3 # Python
                  # ruff # Fast lint and format
                  # ty # Type checker and LSP
                  # uv # Fast packages and environment manager
                  # Web + Misc
                  # vscode-langservers-extracted # HTML/CSS/JS(ON)
                  taplo # TOML LSP
                  yaml-language-server # YAML LSP
                  # Build inputs
                  cmake # Modern build tool
                  cppcheck # C++ Static analysis
                  doctest # Testing framework
                  # llvm # For llvm-cov
                  ninja # Modern build tool
                  pkg-config # Build tool
                ]
                ++ (with pkgs.python3Packages; [
                  appdirs
                  cbor
                  click
                  cmsis-pack-manager
                  colorama
                  cryptography
                  eval-type-backport
                  fasteners
                  humanize
                  intelhex
                  jinja2
                  json5
                  junit-xml
                  lockfile
                  prettytable
                  psutil
                  pydantic
                  pyjson5
                  pyserial
                  pyudev
                  requests
                  tabulate
                  tqdm
                  typing-extensions
                ]);
              # Export compile commands JSON for LSP and other tools
              shellHook = ''
                export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [ pkgs.stdenv.cc.cc.lib ]}:$LD_LIBRARY_PATH"
                export PYTHONPATH="$PWD/mbed-os/tools/python:$PYTHONPATH"
                export PATH="$PWD/.bin:$PATH"
                CC= CXX= cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Debug \
                  -DMBED_TARGET=LPC1768 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
              '';
            };
      });
    };
}
