#include "thememanager.h"

#include <QApplication>
#include <QSettings>

ThemeManager * ThemeManager::globalMngr = nullptr;

Theme ThemeManager::theme( ) const {
  return( m_theme );
}

void ThemeManager::setTheme( const Theme &theme ) {
  if(m_theme != theme ) {
      m_theme = theme;
      QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                          QApplication::organizationName( ), QApplication::applicationName( ) );
      settings.setValue("theme", (int) theme);
      emit themeChanged();
  }
}

void ThemeManager::initialize() {
    emit themeChanged();
}

ThemeManager::ThemeManager( QObject *parent ) : QObject( parent ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  if( settings.contains( "theme" ) ) {
      setTheme((Theme) settings.value("theme").toInt());
  }

}
