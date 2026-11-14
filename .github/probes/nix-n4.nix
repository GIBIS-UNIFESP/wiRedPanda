# Forces meta.maintainers of Packaging/Nix/package.nix (N4). The list is
# `with lib.maintainers; [ joao-zip ]`, which fails if nixpkgs has no such maintainer entry.
let
  pkgs = import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz") { };
in
(pkgs.callPackage ../../Packaging/Nix/package.nix { }).meta.maintainers
