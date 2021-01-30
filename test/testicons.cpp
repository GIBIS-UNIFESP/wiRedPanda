#include "testicons.h"
#include "elementfactory.h"

void TestIcons::testIcons()
{
    for (int type = static_cast<int>(ElementType::BUTTON); type < static_cast<int>(ElementType::DEMUX); ++type) {
        QVERIFY2(!ElementFactory::getPixmap(static_cast<ElementType>(type)).isNull(),
                 QString(ElementFactory::typeToText(static_cast<ElementType>(type)) + " pixmap not found.").toUtf8());
    }
}