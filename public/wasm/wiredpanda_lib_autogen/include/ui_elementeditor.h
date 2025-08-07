/********************************************************************************
** Form generated from reading UI file 'elementeditor.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ELEMENTEDITOR_H
#define UI_ELEMENTEDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ElementEditor
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QComboBox *comboBoxAudio;
    QCheckBox *checkBoxLocked;
    QLabel *labelLabels;
    QLabel *labelTrigger;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButtonTruthTable;
    QPushButton *pushButtonChangeSkin;
    QPushButton *pushButtonDefaultSkin;
    QLineEdit *lineEditTrigger;
    QLabel *labelValue;
    QDoubleSpinBox *doubleSpinBoxFrequency;
    QComboBox *comboBoxInputSize;
    QLabel *labelFrequency;
    QLabel *labelColor;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *lineCurrentAudioBox;
    QPushButton *pushButtonAudioBox;
    QLineEdit *lineEditElementLabel;
    QLabel *labelInputs;
    QComboBox *comboBoxColor;
    QLabel *labelAudioBox;
    QLabel *labelPriority;
    QComboBox *comboBoxValue;
    QComboBox *comboBoxOutputSize;
    QSpinBox *spinBoxPriority;
    QLabel *labelOutputs;
    QLabel *labelLocked;
    QLabel *labelAudio;
    QLabel *labelDelay;
    QDoubleSpinBox *doubleSpinBoxDelay;

    void setupUi(QWidget *ElementEditor)
    {
        if (ElementEditor->objectName().isEmpty())
            ElementEditor->setObjectName("ElementEditor");
        ElementEditor->resize(304, 395);
        gridLayout = new QGridLayout(ElementEditor);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        groupBox = new QGroupBox(ElementEditor);
        groupBox->setObjectName("groupBox");
        QFont font;
        font.setBold(true);
        groupBox->setFont(font);
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName("gridLayout_2");
        comboBoxAudio = new QComboBox(groupBox);
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->addItem(QString());
        comboBoxAudio->setObjectName("comboBoxAudio");

        gridLayout_2->addWidget(comboBoxAudio, 8, 1, 1, 1);

        checkBoxLocked = new QCheckBox(groupBox);
        checkBoxLocked->setObjectName("checkBoxLocked");

        gridLayout_2->addWidget(checkBoxLocked, 4, 1, 1, 1);

        labelLabels = new QLabel(groupBox);
        labelLabels->setObjectName("labelLabels");

        gridLayout_2->addWidget(labelLabels, 0, 0, 1, 1);

        labelTrigger = new QLabel(groupBox);
        labelTrigger->setObjectName("labelTrigger");

        gridLayout_2->addWidget(labelTrigger, 10, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setSizeConstraint(QLayout::SetNoConstraint);
        pushButtonTruthTable = new QPushButton(groupBox);
        pushButtonTruthTable->setObjectName("pushButtonTruthTable");
        pushButtonTruthTable->setEnabled(true);

        horizontalLayout->addWidget(pushButtonTruthTable);

        pushButtonChangeSkin = new QPushButton(groupBox);
        pushButtonChangeSkin->setObjectName("pushButtonChangeSkin");

        horizontalLayout->addWidget(pushButtonChangeSkin);

        pushButtonDefaultSkin = new QPushButton(groupBox);
        pushButtonDefaultSkin->setObjectName("pushButtonDefaultSkin");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButtonDefaultSkin->sizePolicy().hasHeightForWidth());
        pushButtonDefaultSkin->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/toolbar/default.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        pushButtonDefaultSkin->setIcon(icon);
        pushButtonDefaultSkin->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(pushButtonDefaultSkin);


        gridLayout_2->addLayout(horizontalLayout, 12, 0, 1, 2);

        lineEditTrigger = new QLineEdit(groupBox);
        lineEditTrigger->setObjectName("lineEditTrigger");

        gridLayout_2->addWidget(lineEditTrigger, 10, 1, 1, 1);

        labelValue = new QLabel(groupBox);
        labelValue->setObjectName("labelValue");

        gridLayout_2->addWidget(labelValue, 3, 0, 1, 1);

        doubleSpinBoxFrequency = new QDoubleSpinBox(groupBox);
        doubleSpinBoxFrequency->setObjectName("doubleSpinBoxFrequency");
        doubleSpinBoxFrequency->setDecimals(1);
        doubleSpinBoxFrequency->setMinimum(0.000000000000000);
        doubleSpinBoxFrequency->setMaximum(50.000000000000000);
        doubleSpinBoxFrequency->setSingleStep(0.100000000000000);

        gridLayout_2->addWidget(doubleSpinBoxFrequency, 5, 1, 1, 1);

        comboBoxInputSize = new QComboBox(groupBox);
        comboBoxInputSize->setObjectName("comboBoxInputSize");

        gridLayout_2->addWidget(comboBoxInputSize, 1, 1, 1, 1);

        labelFrequency = new QLabel(groupBox);
        labelFrequency->setObjectName("labelFrequency");

        gridLayout_2->addWidget(labelFrequency, 5, 0, 1, 1);

        labelColor = new QLabel(groupBox);
        labelColor->setObjectName("labelColor");

        gridLayout_2->addWidget(labelColor, 7, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        lineCurrentAudioBox = new QLineEdit(groupBox);
        lineCurrentAudioBox->setObjectName("lineCurrentAudioBox");
        lineCurrentAudioBox->setReadOnly(true);

        horizontalLayout_2->addWidget(lineCurrentAudioBox);

        pushButtonAudioBox = new QPushButton(groupBox);
        pushButtonAudioBox->setObjectName("pushButtonAudioBox");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButtonAudioBox->sizePolicy().hasHeightForWidth());
        pushButtonAudioBox->setSizePolicy(sizePolicy1);
        pushButtonAudioBox->setMaximumSize(QSize(28, 16777215));

        horizontalLayout_2->addWidget(pushButtonAudioBox);


        gridLayout_2->addLayout(horizontalLayout_2, 9, 1, 1, 1);

        lineEditElementLabel = new QLineEdit(groupBox);
        lineEditElementLabel->setObjectName("lineEditElementLabel");

        gridLayout_2->addWidget(lineEditElementLabel, 0, 1, 1, 1);

        labelInputs = new QLabel(groupBox);
        labelInputs->setObjectName("labelInputs");

        gridLayout_2->addWidget(labelInputs, 1, 0, 1, 1);

        comboBoxColor = new QComboBox(groupBox);
        comboBoxColor->setObjectName("comboBoxColor");

        gridLayout_2->addWidget(comboBoxColor, 7, 1, 1, 1);

        labelAudioBox = new QLabel(groupBox);
        labelAudioBox->setObjectName("labelAudioBox");

        gridLayout_2->addWidget(labelAudioBox, 9, 0, 1, 1);

        labelPriority = new QLabel(groupBox);
        labelPriority->setObjectName("labelPriority");

        gridLayout_2->addWidget(labelPriority, 11, 0, 1, 1);

        comboBoxValue = new QComboBox(groupBox);
        comboBoxValue->setObjectName("comboBoxValue");

        gridLayout_2->addWidget(comboBoxValue, 3, 1, 1, 1);

        comboBoxOutputSize = new QComboBox(groupBox);
        comboBoxOutputSize->setObjectName("comboBoxOutputSize");

        gridLayout_2->addWidget(comboBoxOutputSize, 2, 1, 1, 1);

        spinBoxPriority = new QSpinBox(groupBox);
        spinBoxPriority->setObjectName("spinBoxPriority");
        spinBoxPriority->setMaximum(9);

        gridLayout_2->addWidget(spinBoxPriority, 11, 1, 1, 1);

        labelOutputs = new QLabel(groupBox);
        labelOutputs->setObjectName("labelOutputs");

        gridLayout_2->addWidget(labelOutputs, 2, 0, 1, 1);

        labelLocked = new QLabel(groupBox);
        labelLocked->setObjectName("labelLocked");

        gridLayout_2->addWidget(labelLocked, 4, 0, 1, 1);

        labelAudio = new QLabel(groupBox);
        labelAudio->setObjectName("labelAudio");

        gridLayout_2->addWidget(labelAudio, 8, 0, 1, 1);

        labelDelay = new QLabel(groupBox);
        labelDelay->setObjectName("labelDelay");

        gridLayout_2->addWidget(labelDelay, 6, 0, 1, 1);

        doubleSpinBoxDelay = new QDoubleSpinBox(groupBox);
        doubleSpinBoxDelay->setObjectName("doubleSpinBoxDelay");
        doubleSpinBoxDelay->setDecimals(1);

        gridLayout_2->addWidget(doubleSpinBoxDelay, 6, 1, 1, 1);


        gridLayout->addWidget(groupBox, 0, 0, 1, 1);


        retranslateUi(ElementEditor);

        QMetaObject::connectSlotsByName(ElementEditor);
    } // setupUi

    void retranslateUi(QWidget *ElementEditor)
    {
        ElementEditor->setWindowTitle(QCoreApplication::translate("ElementEditor", "Form", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ElementEditor", "Title", nullptr));
        comboBoxAudio->setItemText(0, QCoreApplication::translate("ElementEditor", "C6", nullptr));
        comboBoxAudio->setItemText(1, QCoreApplication::translate("ElementEditor", "D6", nullptr));
        comboBoxAudio->setItemText(2, QCoreApplication::translate("ElementEditor", "E6", nullptr));
        comboBoxAudio->setItemText(3, QCoreApplication::translate("ElementEditor", "F6", nullptr));
        comboBoxAudio->setItemText(4, QCoreApplication::translate("ElementEditor", "G6", nullptr));
        comboBoxAudio->setItemText(5, QCoreApplication::translate("ElementEditor", "A7", nullptr));
        comboBoxAudio->setItemText(6, QCoreApplication::translate("ElementEditor", "B7", nullptr));
        comboBoxAudio->setItemText(7, QCoreApplication::translate("ElementEditor", "C7", nullptr));

        checkBoxLocked->setText(QString());
        labelLabels->setText(QCoreApplication::translate("ElementEditor", "Label:", nullptr));
        labelTrigger->setText(QCoreApplication::translate("ElementEditor", "Trigger:", nullptr));
        pushButtonTruthTable->setText(QCoreApplication::translate("ElementEditor", "TruthTable", nullptr));
        pushButtonChangeSkin->setText(QCoreApplication::translate("ElementEditor", "Change skin to ...", nullptr));
#if QT_CONFIG(tooltip)
        pushButtonDefaultSkin->setToolTip(QCoreApplication::translate("ElementEditor", "Default", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButtonDefaultSkin->setText(QString());
        labelValue->setText(QCoreApplication::translate("ElementEditor", "Value:", nullptr));
        doubleSpinBoxFrequency->setSpecialValueText(QString());
        doubleSpinBoxFrequency->setSuffix(QCoreApplication::translate("ElementEditor", " Hz", nullptr));
        labelFrequency->setText(QCoreApplication::translate("ElementEditor", "Frequency:", nullptr));
        labelColor->setText(QCoreApplication::translate("ElementEditor", "Color:", nullptr));
        pushButtonAudioBox->setText(QCoreApplication::translate("ElementEditor", "...", nullptr));
        labelInputs->setText(QCoreApplication::translate("ElementEditor", "Inputs:", nullptr));
        comboBoxColor->setCurrentText(QString());
        labelAudioBox->setText(QCoreApplication::translate("ElementEditor", "Audio:", nullptr));
        labelPriority->setText(QCoreApplication::translate("ElementEditor", "Priority:", nullptr));
        labelOutputs->setText(QCoreApplication::translate("ElementEditor", "Outputs:", nullptr));
        labelLocked->setText(QCoreApplication::translate("ElementEditor", "Locked:", nullptr));
        labelAudio->setText(QCoreApplication::translate("ElementEditor", "Audio:", nullptr));
        labelDelay->setText(QCoreApplication::translate("ElementEditor", "Delay:", nullptr));
        doubleSpinBoxDelay->setSuffix(QCoreApplication::translate("ElementEditor", " Hz", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ElementEditor: public Ui_ElementEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ELEMENTEDITOR_H
