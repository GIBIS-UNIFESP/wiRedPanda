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
  explicit SimpleWaveform( QWidget *parent = 0 );
  ~SimpleWaveform( );

  void showWaveform( Editor *editor );

private:
  Ui::SimpleWaveform *ui;
  QChart chart;
};

#endif /* SIMPLEWAVEFORM_H */
