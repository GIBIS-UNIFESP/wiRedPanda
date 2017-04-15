#include "elementfactory.h"
#include "testicons.h"

void TestIcons::testIcons( ) {
  for( int type = ( int ) ElementType::BUTTON; type < ( int ) ElementType::DEMUX; ++type ) {
    QVERIFY2( !ElementFactory::getPixmap( ( ElementType ) type ).isNull( ),
              QString( ElementFactory::typeToText( ( ElementType ) type ) + " pixmap not found." ).toUtf8( ) );
  }
}
