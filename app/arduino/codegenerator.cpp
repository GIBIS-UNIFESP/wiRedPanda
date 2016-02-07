#include "codegenerator.h"

#include <qneconnection.h>

CodeGenerator::CodeGenerator( QString fileName, const QVector< GraphicElement* > &elements ) : file( fileName ),
  elements( elements ) {
  if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
    return;
  }
  out.setDevice( &file );
  availblePins = { "A0", "A1", "A2", "A3", "A4", "A5",
                   /*"0", "1",*/ "2", "3", "4", "5", "6",
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
        ( elm->elementType( ) == ElementType::SWITCH ) ||
        ( elm->elementType( ) == ElementType::CLOCK ) ) {
      QString varName = elm->objectName( ).toLower( ).trimmed( ) + QString::number( counter );
      QString label = elm->getLabel( ).toLower( ).trimmed( );
      if( !label.isEmpty( ) ) {
        varName = QString( "%1_%2" ).arg( varName ).arg( label );
      }
      out << QString( "const int %1 = %2;" ).arg( varName ).arg( availblePins.front( ) ) << endl;
      inputMap.append( MappedPin( elm, availblePins.front( ), varName, elm->outputs( ).at( 0 ), 0 ) );
      availblePins.pop_front( );
      varMap[ elm->outputs( ).front( ) ] = varName + QString( "_val" );
/*      varMap[ elm->outputs( ).front( ) ] = QString( "auxVar%1" ).arg( globalCounter++ ); */
      counter++;
    }
  }
  out << endl;
}

QString clearString( QString input ) {
  return( input.toLower( ).trimmed( ).replace( " ", "_" ).replace( QRegExp( "\\W" ), "" ) );
}

void CodeGenerator::declareOutputs( ) {
  int counter = 1;
  out << "/* ========= Outputs ========== */" << endl;
  for( GraphicElement *elm: elements ) {
    if( ( elm->elementType( ) == ElementType::LED ) ||
        ( elm->elementType( ) == ElementType::DISPLAY ) ) {
      QString label = clearString( elm->getLabel( ) );
      for( int i = 0; i < elm->inputs( ).size( ); ++i ) {
        QString varName = clearString( elm->objectName( ) ) + QString::number( counter );
        if( !label.isEmpty( ) ) {
          varName = QString( "%1_%2" ).arg( varName ).arg( label );
        }
        QNEPort *port = elm->inputs( ).at( i );
        if( !port->getName( ).isEmpty( ) ) {
          varName = QString( "%1_%2" ).arg( varName ).arg( clearString( port->getName( ) ) );
        }
        out << QString( "const int %1 = %2;" ).arg( varName ).arg( availblePins.front( ) ) << endl;
        outputMap.append( MappedPin( elm, availblePins.front( ), varName, port, i ) );
        availblePins.pop_front( );
        counter++;
      }
    }
  }
  for( GraphicElement *elm: elements ) {
    QString varName = QString( "aux_%1_%2" ).arg( elm->objectName( ) ).arg( globalCounter++ );
    if( !elm->outputs( ).isEmpty( ) ) {
      QNEPort *port = elm->outputs( )[ 0 ];
      if( elm->elementType( ) == ElementType::VCC ) {
        varMap[ port ] = "HIGH";
      }
      else if( elm->elementType( ) == ElementType::GND ) {
        varMap[ port ] = "LOW";
      }
      else if( varMap[ port ].isEmpty( ) ) {
        varMap[ port ] = varName;
      }
    }
  }
  out << "/* ====== Aux. Variables ====== */" << endl;
  for( GraphicElement *elm: elements ) {
    if( !elm->outputs( ).isEmpty( ) ) {
      QString varName = varMap[ elm->outputs( ).first( ) ];
      out << "int " << varName << ";" << endl;
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
  out << "    // Reading input data." << endl;
  for( MappedPin pin: inputMap ) {
    out << QString( "    int %1_val = digitalRead( %1 );" ).arg( pin.varName ) << endl;
  }
/* Aux variables. */
  for( GraphicElement *elm: elements ) {
    QString varName = varMap[ elm->outputs( ).first( ) ];
    switch( elm->elementType( ) ) {
        case ElementType::AND: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
          inPort = elm->inputs( )[ i ];
          out << " && " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        }
        out << ";" << endl;
        break;
      }
        case ElementType::OR: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
          inPort = elm->inputs( )[ i ];
          out << " || " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        }
        out << ";" << endl;
        break;
      }
        case ElementType::NAND: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = ! ( " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
          inPort = elm->inputs( )[ i ];
          out << " && " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        }
        out << " );" << endl;
        break;
      }
        case ElementType::NOR: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = ! ( " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
          inPort = elm->inputs( )[ i ];
          out << " || " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        }
        out << " );" << endl;
        break;
      }
        case ElementType::XOR: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
          inPort = elm->inputs( )[ i ];
          out << " ^ " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        }
        out << ";" << endl;
        break;
      }
        case ElementType::XNOR: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = ! ( " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
          inPort = elm->inputs( )[ i ];
          out << " ^ " << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ];
        }
        out << " );" << endl;
        break;
      }
        case ElementType::NOT: {
        QNEPort *inPort = elm->inputs( ).front( );
        out << "    " << varName << " = !" << varMap[ inPort->connections( ).front( )->otherPort( inPort ) ] <<
          ";" << endl;
        break;
      }
        default:
        break;
    }
  }
  out << endl;
  out << "    // Writing output data." << endl;
  for( MappedPin pin: outputMap ) {
    QNEPort *otherPort = pin.port->connections( ).first( )->otherPort( pin.port );
    QString varName = varMap.value( otherPort );
    if( varName.isEmpty( ) ) {
      varName = "LOW";
    }
    out << QString( "    digitalWrite( %1, %2 );" ).arg( pin.varName ).arg( varName ) << endl;
  }
  out << "}" << endl;
}
