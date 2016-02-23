unix{
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = ~/.local
  }
  BINDIR = $$PREFIX/bin
  DATADIR =$$PREFIX/share

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL
  INSTALLS += target desktop icon26 icon32 icon48 icon64 icon128

  target.path =$$BINDIR

  desktop.path = $$DATADIR/applications
  desktop.files += resources/$${TARGET}.desktop
  desktop.extra += desktop-file-install $${PWD}/resources/$${TARGET}.desktop --dir=$${DATADIR}/applications &&
  desktop.extra += xdg-mime install $${PWD}/resources/$${TARGET}-mime.xml &&
  desktop.extra += xdg-mime default $${TARGET}.desktop application/x-wpanda

  icon128.path = $$DATADIR/icons/hicolor/128x128/apps
  icon128.files += resources/icons/128x128/$${TARGET}.png
  icon128.extra += xdg-icon-resource install --context mimetypes --size 128  $${PWD}/resources/icons/128x128/$${TARGET}.png application-x-wpanda

  icon64.path = $$DATADIR/icons/hicolor/64x64/apps
  icon64.files += resources/icons/64x64/$${TARGET}.png
  icon64.extra += xdg-icon-resource install --context mimetypes --size 64  $${PWD}/resources/icons/64x64/$${TARGET}.png application-x-wpanda

  icon48.path = $$DATADIR/icons/hicolor/48x48/apps
  icon48.files += resources/icons/48x48/$${TARGET}.png
  icon48.extra += xdg-icon-resource install --context mimetypes --size 48  $${PWD}/resources/icons/48x48/$${TARGET}.png application-x-wpanda

  icon32.path = $$DATADIR/icons/hicolor/32x32/apps
  icon32.files += resources/icons/32x32/$${TARGET}.png
  icon32.extra += xdg-icon-resource install --context mimetypes --size 32  $${PWD}/resources/icons/32x32/$${TARGET}.png application-x-wpanda

  icon26.path = $$DATADIR/icons/hicolor/26x26/apps
  icon26.files += resources/icons/26x26/$${TARGET}.png
  icon26.extra += xdg-icon-resource install --context mimetypes --size 26  $${PWD}/resources/icons/26x26/$${TARGET}.png application-x-wpanda

}
