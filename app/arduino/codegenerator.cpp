#include "codegenerator.h"

CodeGenerator::CodeGenerator( QString fileName, const QVector< GraphicElement* > &elements ) : file( fileName ),
  elements( elements ) {
  if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
    return;
  }
  out.setDevice( &file );
  availblePins = { "A0", "A1", "A2", "A3", "A4", "A5",
                   "0", "1", "2", "3", "4", "5", "6",
                   "7", "8", "9", "10", "11", "12", "13" };
}

bool CodeGenerator::generate( ) {
  out << "// ==================================================================== //" << endl;
  out << "// ======= This code was generated automatically by WiRED PANDA ======= //" << endl;
  out << "// ==================================================================== //" << endl;
  out << endl << endl;

  /* Declaring input and output pins; */
  declareInputs( );
  declareOutputs( );

  /* Setup section */
  setup( );

  /* Loop section */
  loop( );
  return( true );
}

void CodeGenerator::declareInputs( ) {
  int counter = 1;
  out << "/* ========= Inputs ========== */" << endl;
  for( GraphicElement *elm: elements ) {
    if( ( elm->elementType( ) == ElementType::BUTTON ) ||
        ( elm->elementType( ) == ElementType::SWITCH ) ) {
      QString varName = elm->objectName( ).toLower( ).trimmed( ) + QString::number( counter );
      out << QString( "const int %1 = %2;" ).arg( varName ).arg( availblePins.back( ) ) << endl;
      inputMap.append( MappedPin( elm, availblePins.back( ), varName ) );
      availblePins.pop_back( );
      counter++;
    }
  }
  out << endl;
}

void CodeGenerator::declareOutputs( ) {
  int counter = 1;
  out << "/* ========= Outputs ========== */" << endl;
  for( GraphicElement *elm: elements ) {
    if( ( elm->elementType( ) == ElementType::LED ) ||
        ( elm->elementType( ) == ElementType::DISPLAY ) ) {
      for( int i = 0; i < elm->inputs( ).size( ); ++i ) {
        QString varName = elm->objectName( ).toLower( ).trimmed( ) + QString::number( counter );
        out << QString( "const int %1 = %2;" ).arg( varName ).arg( availblePins.back( ) ) << endl;
        outputMap.append( MappedPin( elm, availblePins.back( ), varName, i ) );
        availblePins.pop_back( );
        counter++;
      }
    }
  }
  out << endl;
}

void CodeGenerator::setup( ) {
  out << "void setup( ) {" << endl;
  for( MappedPin pin : inputMap ) {
    out << "    pinMode( " << pin.varName << ", INPUT );" << endl;
  }
  for( MappedPin pin : outputMap ) {
    out << "    pinMode( " << pin.varName << ", OUTPUT );" << endl;
  }
  out << "}" << endl << endl;
}

void CodeGenerator::loop( ) {
  out << "void loop( ) {" << endl;
  for( MappedPin pin: inputMap ) {
    out << QString( "    int %1_val = digitalRead( %1 );" ).arg( pin.varName ) << endl;
  }
  out << endl;
  for( MappedPin pin: outputMap ) {
    out << QString( "    int %1_val = LOW;" ).arg( pin.varName ) << endl;
  }
  out << endl;
  for( MappedPin pin: outputMap ) {
    out << QString( "    digitalWrite( %1, %1_val );" ).arg( pin.varName ) << endl;
  }
  out << "}" << endl;
}
