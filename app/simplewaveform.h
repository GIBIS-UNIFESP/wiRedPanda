#ifndef SIMPLEWAVEFORM_H
#define SIMPLEWAVEFORM_H

#include "editor.h"

#include <QChart>
#include <QDialog>

using namespace QtCharts;

namespace Ui {
  class SimpleWaveform;
}

class SimpleWaveform : public QDialog {
  Q_OBJECT

public:
  explicit SimpleWaveform( Editor *editor, QWidget *parent = 0 );
  ~SimpleWaveform( );

  void showWaveform( );

private slots:
  void on_radioButton_Name_clicked( );

  void on_radioButton_Position_clicked( );

private:
  Ui::SimpleWaveform *ui;
  QChart chart;
  Editor *editor;
  enum class SortingType { STRING, POSITION };
  SortingType sortingType;
};

#endif /* SIMPLEWAVEFORM_H */
