#include "elementeditor.h"
#include "ui_elementeditor.h"

#include <QDebug>

ElementEditor::ElementEditor(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ElementEditor) {
  ui->setupUi(this);
  setEnabled(false);
}

ElementEditor::~ElementEditor() {
  delete ui;
}

void ElementEditor::setScene(QGraphicsScene * s) {
  scene = s;
  connect(s,&QGraphicsScene::selectionChanged,this,&ElementEditor::selectionChanged);
}

void ElementEditor::setCurrentElement(GraphicElement * elm) {
  element = elm;
  if(elm != NULL) {
    setEnabled(false);
    //Label
    ui->lineEditElementLabel->setEnabled(element->hasLabel());
    ui->label_labels->setEnabled(element->hasLabel());
    ui->lineEditElementLabel->setText(element->getLabel());
    //Color
    ui->label_color->setEnabled(element->hasColors());
    ui->comboBoxColor->setEnabled(element->hasColors());
    //Frequency
    ui->doubleSpinBoxFrequency->setEnabled(element->hasFrequency());
    ui->label_frequency->setEnabled(element->hasFrequency());
    ui->doubleSpinBoxFrequency->setValue(element->frequency());
    //Input size
    QString inputSz = QString::number(element->inputSize());
    ui->comboBoxInputSz->clear();
    for( int port = element->minInputSz(); port <= element->maxInputSz(); ++port) {
      ui->comboBoxInputSz->addItem(QString::number(port), port);
    }
    ui->label_inputs->setDisabled(ui->comboBoxInputSz->count() < 2);
    ui->comboBoxInputSz->setDisabled(ui->comboBoxInputSz->count() < 2);
    ui->comboBoxInputSz->setCurrentText(inputSz);
    //Color
    if(element->hasColors()) {
      ui->comboBoxColor->setCurrentText(element->color());
    }
    setEnabled(true);
  } else {
    setEnabled(false);
    ui->lineEditElementLabel->setText("");
  }
}

void ElementEditor::selectionChanged() {
  QList<QGraphicsItem *> items = scene->selectedItems();
  GraphicElement * elm = NULL;
  for (QGraphicsItem * item: items) {
    if( item->type() == GraphicElement::Type) {
      if(elm != NULL) {
        setCurrentElement(NULL);
        return;
      } else {
        elm = qgraphicsitem_cast<GraphicElement *>(item);
      }
    }
  }
  setCurrentElement(elm);
}

void ElementEditor::apply() {
  if(element == NULL || isEnabled() == false) {
    return;
  }
  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  element->save(dataStream);
  if(element->minInputSz() != element->maxInputSz()) {
    element->setInputSize(ui->comboBoxInputSz->currentData().toInt());
  }
  if(element->hasColors()) {
    element->setColor(ui->comboBoxColor->currentText());
  }
  if(element->hasLabel()) {
    element->setLabel(ui->lineEditElementLabel->text());
  }
  if(element->hasFrequency()) {
    element->setFrequency(ui->doubleSpinBoxFrequency->value());
  }
  emit elementUpdated(element, itemData);
}

void ElementEditor::on_lineEditElementLabel_editingFinished() {
  apply();
}

void ElementEditor::on_comboBoxInputSz_currentIndexChanged(int index) {
  Q_UNUSED(index);
  apply();
}

void ElementEditor::on_doubleSpinBoxFrequency_editingFinished() {
  apply();
}

void ElementEditor::on_comboBoxColor_currentIndexChanged(int index) {
  Q_UNUSED(index);
  apply();
}
