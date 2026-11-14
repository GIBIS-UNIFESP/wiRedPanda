# Builds Packaging/Nix/package.nix against nixos-unstable (O3, C21). doCheck is the default,
# so the checkPhase (ctest -LE gui) runs.
let
  pkgs = import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz") { };
in
pkgs.callPackage ../../Packaging/Nix/package.nix { }
