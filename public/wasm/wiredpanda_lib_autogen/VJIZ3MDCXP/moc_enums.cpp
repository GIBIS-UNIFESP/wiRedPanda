/****************************************************************************
** Meta object code from reading C++ file 'enums.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../app/enums.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'enums.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN5EnumsE_t {};
} // unnamed namespace

template <> constexpr inline auto Enums::qt_create_metaobjectdata<qt_meta_tag_ZN5EnumsE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Enums",
        "Status",
        "Invalid",
        "Inactive",
        "Active",
        "ElementType",
        "And",
        "AudioBox",
        "Buzzer",
        "Clock",
        "DFlipFlop",
        "DLatch",
        "Demux",
        "Display14",
        "Display16",
        "Display7",
        "IC",
        "InputButton",
        "InputGnd",
        "InputRotary",
        "InputSwitch",
        "InputVcc",
        "JKFlipFlop",
        "JKLatch",
        "Led",
        "Line",
        "Mux",
        "Nand",
        "Node",
        "Nor",
        "Not",
        "Or",
        "SRFlipFlop",
        "SRLatch",
        "TFlipFlop",
        "Text",
        "TruthTable",
        "Unknown",
        "Xnor",
        "Xor",
        "ElementGroup",
        "Gate",
        "Input",
        "Memory",
        "Other",
        "Output",
        "StaticInput"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'Status'
        QtMocHelpers::EnumData<Status>(1, 1, QMC::EnumIsScoped).add({
            {    2, Status::Invalid },
            {    3, Status::Inactive },
            {    4, Status::Active },
        }),
        // enum 'ElementType'
        QtMocHelpers::EnumData<ElementType>(5, 5, QMC::EnumIsScoped).add({
            {    6, ElementType::And },
            {    7, ElementType::AudioBox },
            {    8, ElementType::Buzzer },
            {    9, ElementType::Clock },
            {   10, ElementType::DFlipFlop },
            {   11, ElementType::DLatch },
            {   12, ElementType::Demux },
            {   13, ElementType::Display14 },
            {   14, ElementType::Display16 },
            {   15, ElementType::Display7 },
            {   16, ElementType::IC },
            {   17, ElementType::InputButton },
            {   18, ElementType::InputGnd },
            {   19, ElementType::InputRotary },
            {   20, ElementType::InputSwitch },
            {   21, ElementType::InputVcc },
            {   22, ElementType::JKFlipFlop },
            {   23, ElementType::JKLatch },
            {   24, ElementType::Led },
            {   25, ElementType::Line },
            {   26, ElementType::Mux },
            {   27, ElementType::Nand },
            {   28, ElementType::Node },
            {   29, ElementType::Nor },
            {   30, ElementType::Not },
            {   31, ElementType::Or },
            {   32, ElementType::SRFlipFlop },
            {   33, ElementType::SRLatch },
            {   34, ElementType::TFlipFlop },
            {   35, ElementType::Text },
            {   36, ElementType::TruthTable },
            {   37, ElementType::Unknown },
            {   38, ElementType::Xnor },
            {   39, ElementType::Xor },
        }),
        // enum 'ElementGroup'
        QtMocHelpers::EnumData<ElementGroup>(40, 40, QMC::EnumIsScoped).add({
            {   41, ElementGroup::Gate },
            {   16, ElementGroup::IC },
            {   42, ElementGroup::Input },
            {   43, ElementGroup::Memory },
            {   26, ElementGroup::Mux },
            {   44, ElementGroup::Other },
            {   45, ElementGroup::Output },
            {   46, ElementGroup::StaticInput },
            {   37, ElementGroup::Unknown },
        }),
    };
    return QtMocHelpers::metaObjectData<Enums, qt_meta_tag_ZN5EnumsE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Enums::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5EnumsE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN5EnumsE_t>.data,
    nullptr,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN5EnumsE_t>.metaTypes,
    nullptr
} };

QT_WARNING_POP
