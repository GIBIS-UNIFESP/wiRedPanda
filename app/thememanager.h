#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QColor>
#include <QObject>
class Editor;

enum class Theme { Panda_Light, Panda_Dark };

class ThemeManager;

class ThemeAttrs {
public:
  ThemeAttrs( );

  QColor scene_bgBrush;
  QColor scene_bgDots;

  QColor selectionBrush;
  QColor selectionPen;

  QColor qneConnection_true;
  QColor qneConnection_false;
  QColor qneConnection_invalid;
  QColor qneConnection_selected;

  QColor graphicElement_labelColor;

  QColor qnePort_hoverPort;
  QColor qnePort_true_pen;
  QColor qnePort_true_brush;
  QColor qnePort_false_pen;
  QColor qnePort_false_brush;
  QColor qnePort_invalid_pen;
  QColor qnePort_invalid_brush;
  QColor qnePort_output_pen;
  QColor qnePort_output_brush;

  void setTheme( Theme thm );
};

class ThemeManager : public QObject {
  Q_OBJECT

  Editor * editor;
  Theme m_theme;
  ThemeAttrs attrs;

public:
  static ThemeManager *globalMngr;

  explicit ThemeManager( QObject *parent = 0 );

  QString currentTheme( ) const;
  void setCurrentTheme( Theme theme );

  Theme theme( ) const;
  void setTheme( const Theme &theme );

  void initialize( );
  ThemeAttrs getAttrs( ) const;

signals:
  void themeChanged( );

public slots:

};

#endif /* THEMEMANAGER_H */
