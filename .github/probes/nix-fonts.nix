# N10 variant: Packaging/Nix/package.nix with DejaVu fonts and an explicit fontconfig file in the
# check environment. Tests whether the fontconfig failures are the only cause of the checkPhase failures.
let
  pkgs = import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz") { };
  fonts = pkgs.makeFontsConf { fontDirectories = [ pkgs.dejavu_fonts ]; };
in
(pkgs.callPackage ../../Packaging/Nix/package.nix { }).overrideAttrs (old: {
  nativeCheckInputs = (old.nativeCheckInputs or [ ]) ++ [ pkgs.dejavu_fonts ];
  # Only the two classes that still fail after the plugin-path fix (diagnostic round).
  ctestFlags = [ "--output-on-failure" "-R" "^(TestICFixtureLayout|TestICController)$" ];
  # The wrapper sets QT_PLUGIN_PATH for the app, but not for the test binaries. Without the
  # qtsvg image plugin, every test that loads an SVG icon aborts ("Couldn't load pixmap").
  preCheck = old.preCheck + ''
    export FONTCONFIG_FILE=${fonts}
    export QT_PLUGIN_PATH=${pkgs.lib.makeSearchPath "lib/qt-6/plugins" [ pkgs.qt6.qtbase pkgs.qt6.qtsvg pkgs.qt6.qtmultimedia ]}
    echo "DIAG sandbox fc-match sans-serif: $(fc-match sans-serif 2>&1)"
    echo "DIAG sandbox fc-list families: $(fc-list : family 2>/dev/null | sort -u | head -60 | tr '\n' ';')"
    echo "DIAG sandbox /etc/hostname: $(ls -l /etc/hostname 2>&1)"
    echo "DIAG sandbox id: $(id)"
  '';
})
