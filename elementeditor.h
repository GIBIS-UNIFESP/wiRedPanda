#ifndef ELEMENTEDITOR_H
#define ELEMENTEDITOR_H

#include "graphicelement.h"
#include <QGraphicsScene>
#include <QWidget>

namespace Ui {
  class ElementEditor;
}

class ElementEditor : public QWidget {
  Q_OBJECT

public:
  explicit ElementEditor(QWidget *parent = 0);
  ~ElementEditor();

  void setScene( QGraphicsScene * s );

private slots:

  void on_comboBoxColor_currentIndexChanged(int index);

  void on_comboBoxInputSz_currentIndexChanged(int index);

  void selectionChanged();

private:
  void setCurrentElement( GraphicElement *element );

  Ui::ElementEditor *ui;
  GraphicElement * element;
  QGraphicsScene * scene;
};

#endif // ELEMENTEDITOR_H
