#ifndef SIMPLEWAVEFORM_H
#define SIMPLEWAVEFORM_H

#include "editor.h"

#include <QChart>
#include <QChartView>
#include <QDialog>
#include <QTextStream>

namespace Ui {
  class SimpleWaveform;
}

class SimpleWaveform : public QDialog {
  Q_OBJECT
  enum class SortingKind { INCREASING, DECREASING, POSITION };

public:
  explicit SimpleWaveform( Editor *editor, QWidget *parent = nullptr );
  ~SimpleWaveform( );

  void showWaveform( );
  static void sortElements( QVector< GraphicElement* > &elements, QVector< GraphicElement* > &inputs,
                            QVector< GraphicElement* > &outputs, SortingKind sorting );

  static bool saveToTxt( QTextStream &outStream, Editor *editor );
private slots:
  void on_radioButton_Position_clicked( );

  void on_radioButton_Increasing_clicked( );

  void on_radioButton_Decreasing_clicked( );

  void on_pushButton_Copy_clicked( );

private:
  Ui::SimpleWaveform *ui;
  QtCharts::QChart chart;
  QtCharts::QChartView *chartView;
  Editor *editor;

  SortingKind sortingKind;
};

#endif /* SIMPLEWAVEFORM_H */
