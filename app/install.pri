unix{
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }

  BINDIR = $$PREFIX/bin
  DATADIR = $$PREFIX/share

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL
  INSTALLS += target shell desktop mime icon26 icon32 icon48 icon64 icon128 fileIcon26 fileIcon32 fileIcon48 fileIcon64 fileIcon128 postinst

  target.path = $$BINDIR


  shell.path = $$BINDIR
  shell.files = $${PWD}/resources/wpanda.sh

  desktop.path = $$DATADIR/applications
  desktop.files += resources/$${TARGET}.desktop

  mime.path = $$DATADIR/xml/misc
  mime.files += $${PWD}/resources/$${TARGET}-mime.xml

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

  fileIcon64.path = $$DATADIR/icons/hicolor/64x64/apps
  fileIcon64.files += resources/icons/64x64/$${TARGET}-file.png

  fileIcon48.path = $$DATADIR/icons/hicolor/48x48/apps
  fileIcon48.files += resources/icons/48x48/$${TARGET}-file.png

  fileIcon32.path = $$DATADIR/icons/hicolor/32x32/apps
  fileIcon32.files += resources/icons/32x32/$${TARGET}-file.png

  fileIcon26.path = $$DATADIR/icons/hicolor/26x26/apps
  fileIcon26.files += resources/icons/26x26/$${TARGET}-file.png

  postinst.path += $$DATADIR
  postinst.commands += desktop-file-install /usr/local/share/applications/wpanda.desktop ;
  postinst.commands += xdg-mime install --mode system /usr/local/share/xml/misc/wpanda-mime.xml ;
  #postinst.commands += xdg-mime default /usr/local/share/applications/wpanda.desktop application/x-wpanda ;
  postinst.commands += xdg-icon-resource install --context mimetypes --size 128  /usr/local/share/icons/hicolor/128x128/apps/wpanda-file.png application-x-wpanda ;
  postinst.commands += xdg-icon-resource install --context mimetypes --size 64  /usr/local/share/icons/hicolor/64x64/apps/wpanda-file.png application-x-wpanda ;
  postinst.commands += xdg-icon-resource install --context mimetypes --size 48  /usr/local/share/icons/hicolor/48x48/apps/wpanda-file.png application-x-wpanda ;
  postinst.commands += xdg-icon-resource install --context mimetypes --size 32  /usr/local/share/icons/hicolor/32x32/apps/wpanda-file.png application-x-wpanda ;
  postinst.commands += xdg-icon-resource install --context mimetypes --size 26  /usr/local/share/icons/hicolor/26x26/apps/wpanda-file.png application-x-wpanda


}
