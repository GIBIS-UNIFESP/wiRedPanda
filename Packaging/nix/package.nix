{
  lib,
  stdenv,
  fetchFromGitHub,
  cmake,
  ninja,
  qt6,
  wrapQtAppsHook,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "wiredpanda";
  version = "5.0.1";

  src = fetchFromGitHub {
    owner = "GIBIS-UNIFESP";
    repo = "wiRedPanda";
    rev = finalAttrs.version;
    hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
  };

  nativeBuildInputs = [
    cmake
    ninja
    qt6.qttools
    wrapQtAppsHook
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtsvg
    qt6.qtmultimedia
    qt6.qtimageformats
  ];

  cmakeFlags = [
    (lib.cmakeFeature "CMAKE_BUILD_TYPE" "Release")
  ];

  # No custom installPhase: the cmake setup hook's default install already runs
  # CMakeLists.txt's own install() rules (binary, desktop file — correctly
  # @PROJECT_VERSION@-templated —, icons, mime type, metainfo, Examples) into $out.

  meta = {
    description = "Logic circuit simulator focused on education";
    homepage = "https://github.com/GIBIS-UNIFESP/wiRedPanda";
    license = lib.licenses.gpl3Plus;
    maintainers = [ ];
    platforms = lib.platforms.linux;
    mainProgram = "wiredpanda";
  };
})
