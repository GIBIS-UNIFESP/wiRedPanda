#ifndef ELEMENTEDITOR_H
#define ELEMENTEDITOR_H

#include <QWidget>

namespace Ui {
  class ElementEditor;
}

class ElementEditor : public QWidget {
  Q_OBJECT

public:
  explicit ElementEditor(QWidget *parent = 0);
  ~ElementEditor();

private:
  Ui::ElementEditor *ui;
};

#endif // ELEMENTEDITOR_H
