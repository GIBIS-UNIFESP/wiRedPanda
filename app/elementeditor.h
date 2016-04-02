#ifndef ELEMENTEDITOR_H
#define ELEMENTEDITOR_H

#include "graphicelement.h"
#include <QGraphicsScene>
#include <QWidget>

namespace Ui {
  class ElementEditor;
}

class Editor;

class ElementEditor : public QWidget {
  Q_OBJECT

public:
  explicit ElementEditor(QWidget *parent = 0);
  ~ElementEditor();

  void setScene( QGraphicsScene * s );
  void contextMenu(QPoint screenPos , Editor * editor);
//  void renameAction( const QVector< GraphicElement *> &element );
//  void changeColorAction( const QVector<GraphicElement *> &element );

signals:
  void elementUpdated( const QVector< GraphicElement *> &elements, QByteArray itemData );

private slots:

  void selectionChanged();

  void on_lineEditElementLabel_editingFinished();

  void on_comboBoxInputSz_currentIndexChanged(int index);

  void on_doubleSpinBoxFrequency_editingFinished();

  void on_comboBoxColor_currentIndexChanged(int index);

  void on_trigger_currentIndexChanged(const QString &arg1);

private:
  void setCurrentElements(const QVector<GraphicElement *> & element );
  void apply();

  Ui::ElementEditor *ui;
  QVector< GraphicElement *> elements;
  QGraphicsScene * scene;
  bool hasSomething, hasLabel, hasColors, hasFrequency;
  bool canChangeInputSize, hasTrigger, hasRotation;
  bool hasSameLabel, hasSameColors, hasSameFrequency;
  bool hasSameInputSize, hasSameTrigger;

  QString manyLabels;
  QString manyColors;
  QString manyIS;
  QString manyFreq;
  QString manyTriggers;


};

#endif // ELEMENTEDITOR_H
