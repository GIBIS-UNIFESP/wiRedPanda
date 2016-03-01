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

signals:
  void elementUpdated( GraphicElement *element, QByteArray itemData );

private slots:

  void selectionChanged();

  void on_lineEditElementLabel_editingFinished();

  void on_comboBoxInputSz_currentIndexChanged(int index);

  void on_doubleSpinBoxFrequency_editingFinished();

  void on_comboBoxColor_currentIndexChanged(int index);

  void on_trigger_currentIndexChanged(const QString &arg1);

private:
  void setCurrentElement( GraphicElement *element );
  void apply();

  Ui::ElementEditor *ui;
  GraphicElement * element;
  QGraphicsScene * scene;
};

#endif // ELEMENTEDITOR_H
