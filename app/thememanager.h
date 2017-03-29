#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
class Editor;

enum class Theme { Panda_Light, Panda_Dark };

class ThemeManager : public QObject {
  Q_OBJECT

  Editor *editor;
  Theme m_theme;

public:
  static ThemeManager *globalMngr;

  explicit ThemeManager( QObject *parent = 0 );

  QString currentTheme( ) const;
  void setCurrentTheme( Theme theme );

  Theme theme( ) const;
  void setTheme( const Theme &theme );

  void initialize();
signals:
  void themeChanged( );

public slots:

};

#endif /* THEMEMANAGER_H */
