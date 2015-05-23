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

  ui->lineEditElementLabel->setEnabled(element->hasLabel());
  ui->label_labels->setEnabled(element->hasLabel());
  ui->comboBoxColor->setEnabled(element->hasColors());
  ui->label_color->setEnabled(element->hasColors());
  ui->doubleSpinBoxFrequency->setEnabled(element->hasFrequency());
  ui->label_frequency->setEnabled(element->hasFrequency());
  ui->comboBoxInputSz->clear();
  for( int port = element->minInputSz(); port <= element->maxInputSz(); ++port) {
    ui->comboBoxInputSz->addItem(QString::number(port), port);
  }
  ui->comboBoxInputSz->setDisabled(ui->comboBoxInputSz->count() < 2);
  ui->label_inputs->setDisabled(ui->comboBoxInputSz->count() < 2);
  if(element->hasColors()){
    ui->comboBoxColor->setCurrentText(element->color());
  }
}

void ElementEditor::on_comboBoxColor_currentIndexChanged(int) {
//  qDebug() << "Text = " << ui->comboBoxColor->currentText().toLower() << ", Data = " << ui->comboBoxColor->currentData();
  element->setColor(ui->comboBoxColor->currentText());
}

void ElementEditor::on_comboBoxInputSz_currentIndexChanged(int) {
//  if(element->maxInputSz() != element->minInputSz()) {
//    int newInputSz = ui->comboBoxColor->currentData().toInt();
//    if( newInputSz >= element->minInputSz() && newInputSz <= element->maxInputSz()) {

//    }
//  }
}

void ElementEditor::selectionChanged() {
  QList<QGraphicsItem *> items = scene->selectedItems();
  GraphicElement * elm = NULL;
  foreach (QGraphicsItem * item, items) {
    if( item->type() == GraphicElement::Type) {
      if(elm != NULL) {
        setEnabled(false);
        return;
      }else{
        elm = qgraphicsitem_cast<GraphicElement *>(item);
      }
    }
  }
  if(elm != NULL){
    setEnabled(true);
    setCurrentElement(elm);
  }else {
    setEnabled(false);
  }
}
