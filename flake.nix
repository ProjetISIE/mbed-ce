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
        "x86_64-linux" # "aarch64-linux"
        "aarch64-darwin"
      ];
      forAllSystems =
        f: nixpkgs.lib.genAttrs localSystems (system: f (import nixpkgs { inherit system; }));
      forAllSystemsWithCross =
        f:
        let
          crossSystem = "aarch64-linux";
        in
        nixpkgs.lib.genAttrs localSystems (
          localSystem:
          f (import nixpkgs { inherit localSystem; }) (
            import nixpkgs {
              inherit localSystem;
              inherit crossSystem;
            }
          )
        );
    in
    {
      # packages = forAllSystems (pkgs: {
      #   default = pkgs.callPackage ./pkg.nix {
      #     inherit self pkgs;
      #     stdenv = pkgs.clangStdenv;
      #   };
      # });
      # crossPackages = forAllSystemsWithCross (
      #   pkgs: crossPkgs: {
      #     default = crossPkgs.callPackage ./pkg.nix {
      #       inherit self;
      #       stdenv = crossPkgs.clangStdenv;
      #       pkgs = crossPkgs;
      #     };
      #   }
      # );
      devShells = forAllSystems (pkgs: {
        default =
          pkgs.mkShell.override
            {
              stdenv = pkgs.clangStdenv; # Clang instead of GCC
            }
            {
              packages = with pkgs; [
                bashInteractive
                clang-tools # Clang CLIs, including LSP
                clang-uml # UML diagram generator
                cmake-format # CMake formatter
                cmake-language-server # Cmake LSP
                doxygen # Documentation generator
                lldb # Clang debug adapter
                mbed-cli
                socat # Serial terminal for manual testing
                valgrind # Debugging and profiling
              ];
              # nativeBuildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.default.nativeBuildInputs;
              # buildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.default.buildInputs;
              # Export compile commands JSON for LSP and other tools
              shellHook = ''
                mkdir --verbose build
                cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -GNinja -S . -B build
              '';
            };
      });
    };
}
