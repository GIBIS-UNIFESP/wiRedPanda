{
  lib,
  stdenv,
  fetchFromGitHub,
  cmake,
  ninja,
  nlohmann_json,
  nlohmann_json_schema_validator,
  qt6,
  nix-update-script,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "wiredpanda";
  version = "5.2.2";

  src = fetchFromGitHub {
    owner = "GIBIS-UNIFESP";
    repo = "wiRedPanda";
    tag = finalAttrs.version;
    hash = "sha256-su02xUb/MRPIuhg0ldFZPZL6ViSOxaBrLb5H0TYSbnI=";
  };

  nativeBuildInputs = [
    cmake
    ninja
    qt6.qttools
    qt6.wrapQtAppsHook
  ];

  buildInputs = [
    nlohmann_json
    nlohmann_json_schema_validator
    qt6.qtbase
    qt6.qtmultimedia
    qt6.qtsvg
  ];

  cmakeFlags = [
    (lib.cmakeBool "WIREDPANDA_USE_SYSTEM_JSON" true)
    (lib.cmakeBool "BUILD_TESTING" finalAttrs.finalPackage.doCheck)
  ];

  doCheck = stdenv.buildPlatform.canExecute stdenv.hostPlatform;

  preCheck = ''
    export HOME="$(mktemp -d)"
    export QT_QPA_PLATFORM=offscreen
  '';

  ctestFlags = [
    "--output-on-failure"
    "-LE"
    "gui"
  ];

  postInstall = lib.optionalString stdenv.hostPlatform.isDarwin ''
    mkdir -p "$out/Applications" "$out/bin"
    mv "$out/wiredpanda.app" "$out/Applications/wiredpanda.app"
    makeQtWrapper "$out/Applications/wiredpanda.app/Contents/MacOS/wiredpanda" "$out/bin/wiredpanda"
  '';

  passthru.updateScript = nix-update-script { };

  meta = {
    description = "Software designed to help students learn about logic circuits and simulate them in an easy and friendly way";
    homepage = "https://gibis-unifesp.github.io/wiRedPanda/";
    downloadPage = "https://github.com/GIBIS-UNIFESP/wiRedPanda/releases";
    changelog = "https://github.com/GIBIS-UNIFESP/wiRedPanda/releases/tag/${finalAttrs.version}";
    license = lib.licenses.gpl3Plus;
    maintainers = with lib.maintainers; [ joao-zip ];
    mainProgram = "wiredpanda";
    platforms = lib.platforms.linux ++ lib.platforms.darwin;
  };
})
