#include "elementeditor.h"
#include "ui_elementeditor.h"

ElementEditor::ElementEditor(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ElementEditor) {
  ui->setupUi(this);
}

ElementEditor::~ElementEditor() {
  delete ui;
}
