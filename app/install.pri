unix{
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }

  BINDIR = $$PREFIX/bin
  DATADIR = $$PREFIX/share

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL
  INSTALLS += target desktop icon26 icon32 icon48 icon64 icon128 fileIcon26 fileIcon32 fileIcon48 fileIcon64 fileIcon128

  target.path = $$BINDIR

  desktop.path = $$DATADIR/applications
  desktop.files += resources/$${TARGET}.desktop
  desktop.extra += desktop-file-install $${PWD}/resources/$${TARGET}.desktop --dir=$${DATADIR}/applications &&
  desktop.extra += xdg-mime install --mode system $${PWD}/resources/$${TARGET}-mime.xml &&
  desktop.extra += xdg-mime default $${TARGET}.desktop application/x-wpanda

  icon128.path = $$DATADIR/icons/hicolor/128x128/apps
  icon128.files += resources/icons/128x128/$${TARGET}.png

  icon64.path = $$DATADIR/icons/hicolor/64x64/apps
  icon64.files += resources/icons/64x64/$${TARGET}.png

  icon48.path = $$DATADIR/icons/hicolor/48x48/apps
  icon48.files += resources/icons/48x48/$${TARGET}.png

  icon32.path = $$DATADIR/icons/hicolor/32x32/apps
  icon32.files += resources/icons/32x32/$${TARGET}.png

  icon26.path = $$DATADIR/icons/hicolor/26x26/apps
  icon26.files += resources/icons/26x26/$${TARGET}.png

  fileIcon128.path = $$DATADIR/icons/hicolor/128x128/apps
  fileIcon128.files += resources/icons/128x128/$${TARGET}-file.png
  fileIcon128.extra += xdg-icon-resource install --context mimetypes --size 128   $$DATADIR/icons/hicolor/128x128/apps/$${TARGET}-file.png application-x-wpanda

  fileIcon64.path = $$DATADIR/icons/hicolor/64x64/apps
  fileIcon64.files += resources/icons/64x64/$${TARGET}-file.png
  fileIcon64.extra += xdg-icon-resource install --context mimetypes --size 64  $$DATADIR/icons/hicolor/64x64/apps/$${TARGET}-file.png application-x-wpanda

  fileIcon48.path = $$DATADIR/icons/hicolor/48x48/apps
  fileIcon48.files += resources/icons/48x48/$${TARGET}-file.png
  fileIcon48.extra += xdg-icon-resource install --context mimetypes --size 48  $$DATADIR/icons/hicolor/48x48/apps/$${TARGET}-file.png application-x-wpanda

  fileIcon32.path = $$DATADIR/icons/hicolor/32x32/apps
  fileIcon32.files += resources/icons/32x32/$${TARGET}-file.png
  fileIcon32.extra += xdg-icon-resource install --context mimetypes --size 32  $$DATADIR/icons/hicolor/32x32/apps/$${TARGET}-file.png application-x-wpanda

  fileIcon26.path = $$DATADIR/icons/hicolor/26x26/apps
  fileIcon26.files += resources/icons/26x26/$${TARGET}-file.png
  fileIcon26.extra += xdg-icon-resource install --context mimetypes --size 26  $$DATADIR/icons/hicolor/26x26/apps//$${TARGET}-file.png application-x-wpanda
}
