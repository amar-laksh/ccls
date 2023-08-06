{
  description = "Standard C++ 20 shell (gcc12) with testing, debugging, and benchmarking support.";

  inputs = {
    # Pointing to the current stable release of nixpkgs. You can
    # customize this to point to an older version or unstable if you
    # like everything shining.
    nixpkgs.url = "github:NixOS/nixpkgs/23.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    # Add the system/architecture you would like to support here. Note that not
    # all packages in the official nixpkgs support all platforms.
    "x86_64-linux"
    "i686-linux"
    "aarch64-linux"
    "x86_64-darwin"
  ]
    (system:
      let
        pkgs = import nixpkgs {
          inherit system;

          # Add overlays here if you need to override the nixpkgs
          # official packages.
          overlays = [
            (
              final: prev: {
                gcc12SShell = pkgs.mkShell.override { stdenv = pkgs.stdenv; };
              }
            )
          ];

          # Uncomment this if you need unfree software (e.g. cuda) for
          # your project.
          #
          # config.allowUnfree = true;
        };
      in
      {
        devShells.default = pkgs.gcc12SShell {
          # Update the name to something that suites your project.
          name = "Ccls dev shell";
          packages = with pkgs;[
            # Development Tools
            fzf
            hyperfine # For benchmarking
            lldb_16 # for debugging

            cmake
            llvmPackages.llvm.dev
            libclang
            llvm
            rapidjson

            # Development time dependencies
            clang-tools_15 # for lsp

            # Build time and Run time dependencies
            ninja
            cmake
            catch2_3 # for testing
            mold # modern lineker
          ];

          # Setting up the environment variables you need during
          # development.
          shellHook =
            ''
              export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"${pkgs.lib.makeLibraryPath [ pkgs.stdenv.cc.cc.lib ]}"
            ''
          ;
        };
      });
}
