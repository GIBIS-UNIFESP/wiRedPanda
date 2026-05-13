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
    qt6.qtmultimedia
    qt6.qtimageformats
  ];

  cmakeFlags = [
    (lib.cmakeFeature "CMAKE_BUILD_TYPE" "Release")
  ];

  installPhase = ''
    runHook preInstall

    install -Dm755 wiredpanda $out/bin/wiredpanda

    install -Dm644 ../App/Resources/Freedesktop/wiredpanda.desktop \
      $out/share/applications/wiredpanda.desktop
    install -Dm644 ../App/Resources/Freedesktop/wiredpanda-mime.xml \
      $out/share/mime/packages/wiredpanda-mime.xml
    install -Dm644 ../App/Resources/Freedesktop/io.github.gibis_unifesp.wiRedPanda.metainfo.xml \
      $out/share/metainfo/io.github.gibis_unifesp.wiRedPanda.metainfo.xml

    install -Dm644 ../App/Resources/Assets/Logos/wpanda.svg \
      $out/share/icons/hicolor/scalable/apps/wpanda.svg
    install -Dm644 ../App/Resources/Assets/Logos/wpanda-256x256.png \
      $out/share/icons/hicolor/256x256/apps/wpanda.png

    mkdir -p $out/share/wiredpanda
    cp -r ../Examples $out/share/wiredpanda/

    runHook postInstall
  '';

  meta = {
    description = "Logic circuit simulator focused on education";
    homepage = "https://github.com/GIBIS-UNIFESP/wiRedPanda";
    license = lib.licenses.gpl3Plus;
    maintainers = [ ];
    platforms = lib.platforms.linux;
    mainProgram = "wiredpanda";
  };
})
