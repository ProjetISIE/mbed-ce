{
  description = "Nix flake C/C++ development environment";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-25.05"; # NixOS Stable
  # inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  # inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0";
  outputs =
    { self, nixpkgs }:
    let
      systems =
        f:
        nixpkgs.lib.genAttrs [
          "x86_64-linux"
          "aarch64-linux"
          # "aarch64-darwin"
        ] (system: f { pkgs = import nixpkgs { inherit system; }; });
    in
    {
      devShells = systems (
        { pkgs }:
        {
          default =
            pkgs.mkShell.override
              {
                stdenv = pkgs.clangStdenv; # Clang instead of GCC
              }
              {
                packages = with pkgs; [
                  clang-tools # Clang CLIs
                  cmake # Automation tool
                  cmake-language-server # LSP
                  cppcheck # Static analysis
                  # clang-uml # Generate UML from c(++)
                  doctest # Testing framework
                  doxygen # Documentation generator
                  # gdb # Debugger
                  # gtest # Testing framework
                  lcov # Code coverage
                  lldb # Debug adapter
                  # pkg-config # Find libraries
                  valgrind # Debugging and profiling
                ];
                env = {
                  CXX = "c++"; # Use Clang++ as the default C++ compiler
                  CC = "clang"; # Use Clang as the default C compiler
                  CXXFLAGS = "-std=c++23 -Wall -Wextra -Wpedantic -Wshadow -Wconversion";
                };
              };
        }
      );
    };
}
