#include "thememanager.h"

#include <QApplication>
#include <QSettings>


ThemeManager*ThemeManager::globalMngr = nullptr;


Theme ThemeManager::theme( ) const {
  return( m_theme );
}

void ThemeManager::setTheme( const Theme &theme ) {
  attrs.setTheme(theme);
  if( m_theme != theme ) {
    m_theme = theme;
    QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                        QApplication::organizationName( ), QApplication::applicationName( ) );
    settings.setValue( "theme", ( int ) theme );
    emit themeChanged( );
  }
}

void ThemeManager::initialize( ) {
  emit themeChanged( );
}


ThemeAttrs ThemeManager::getAttrs( ) const {
  return( attrs );
}

ThemeManager::ThemeManager( QObject *parent ) : QObject( parent ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  if( settings.contains( "theme" ) ) {
    setTheme( ( Theme ) settings.value( "theme" ).toInt( ) );
  }
  else {
    setTheme( Theme::Panda_Light );
  }
}


ThemeAttrs::ThemeAttrs( ) {
  setTheme( Theme::Panda_Light );
}

void ThemeAttrs::setTheme( Theme thm ) {

  switch( thm ) {
      case Theme::Panda_Light:
      scene_bgBrush = QColor( "#ffffe6" );
      scene_bgDots = QColor( Qt::darkGray );
      selectionBrush = QColor( 175, 0, 0, 80 );
      selectionPen = QColor( 175, 0, 0, 255 );

      graphicElement_labelColor = QColor(Qt::black);

      break;
      case Theme::Panda_Dark:
      scene_bgBrush = QColor( "#404552" );
      scene_bgDots = QColor( Qt::black );
      selectionBrush = QColor( 115, 255, 230, 150);
      selectionPen = QColor( 65, 255, 220, 255 );

      graphicElement_labelColor = QColor("#e0e0e0");

      break;
  }

  qneConnection_selected = selectionPen;

  qneConnection_true = QColor( Qt::green );
  qneConnection_false = QColor( Qt::darkGreen );
  qneConnection_invalid = QColor( Qt::red );

  qnePort_true_pen = QColor( Qt::black );
  qnePort_false_pen = QColor( Qt::black );
  qnePort_invalid_pen = QColor( Qt::black );

  qnePort_true_brush = qneConnection_true;
  qnePort_false_brush = QColor( 0x333333 );
  qnePort_invalid_brush = qneConnection_invalid;

  qnePort_hoverPort = QColor( Qt::yellow );

  qnePort_output_pen = QColor( Qt::darkRed );
  qnePort_output_brush = QColor(  Qt::red );

}
