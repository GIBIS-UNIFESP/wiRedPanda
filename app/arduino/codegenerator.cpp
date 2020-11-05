#include "box.h"
#include "clock.h"
#include "codegenerator.h"
#include "editor.h"
#include "qneconnection.h"

#include <stdexcept>

CodeGenerator::CodeGenerator( QString fileName, const QVector< GraphicElement* > &elements ) : file( fileName ),
  elements( elements ) {
  if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
    return;
  }
  globalCounter = 1;
  out.setDevice( &file );
  availablePins = { "A0", "A1", "A2", "A3", "A4", "A5",
                    /*"0", "1",*/ "2", "3", "4", "5", "6",
                    "7", "8", "9", "10", "11", "12", "13" };

}

static inline QString highLow( int val ) {
  return( val == 1 ? "HIGH" : "LOW" );
}

QString clearString( QString input ) {
  return( input.toLower( ).trimmed( ).replace( " ", "_" ).replace( "-", "_" ).replace( QRegExp( "\\W" ), "" ) );
}


QString CodeGenerator::otherPortName( QNEPort *port ) {
  if( port ) {
    if( port->connections( ).isEmpty( ) ) {
      return( highLow( port->defaultValue( ) ) );
    }
    else {
      QNEPort *other = port->connections( ).front( )->otherPort( port );
      if( other ) {
        return( varMap[ other ] );
      }
      else {
        return( highLow( port->defaultValue( ) ) );
      }
    }
  }
  else {
    return( "LOW" );
  }
}

bool CodeGenerator::generate( ) {
  out << "// ==================================================================== //\n";
  out << "// ======= This code was generated automatically by wiRED PANDA ======= //\n";
  out << "// ==================================================================== //\n";
  out << "\n" << "\n";
  out << "#include <elapsedMillis.h>\n";
  /* Declaring input and output pins; */
  declareInputs( );
  declareOutputs( );
  declareAuxVariables( );
  /* Setup section */
  setup( );

  /* Loop section */
  loop( );
  return( true );
}

void CodeGenerator::declareInputs( ) {
  int counter = 1;
  out << "/* ========= Inputs ========== */\n";
  for( GraphicElement *elm : elements ) {
    if( ( elm->elementType( ) == ElementType::BUTTON ) ||
        ( elm->elementType( ) == ElementType::SWITCH ) ) {
      QString varName = elm->objectName( ) + QString::number( counter );
      QString label = elm->getLabel( );
      if( !label.isEmpty( ) ) {
        varName = QString( "%1_%2" ).arg( varName, label );
      }
      varName = clearString( varName );
      out << QString( "const int %1 = %2;" ).arg( varName, availablePins.front( ) ) << "\n";
      inputMap.append( MappedPin( elm, availablePins.front( ), varName, elm->output( 0 ), 0 ) );
      availablePins.pop_front( );
      varMap[ elm->output( ) ] = varName + QString( "_val" );
      counter++;
    }
  }
  out << "\n";
}

void CodeGenerator::declareOutputs( ) {
  int counter = 1;
  out << "/* ========= Outputs ========== */\n";
  for( GraphicElement *elm : elements ) {
    if( elm->elementGroup( ) == ElementGroup::OUTPUT ) {
      QString label = elm->getLabel( );
      for( int i = 0; i < elm->inputs( ).size( ); ++i ) {
        QString varName = elm->objectName( ) + QString::number( counter );
        if( !label.isEmpty( ) ) {
          varName = QString( "%1_%2" ).arg( varName, label );
        }
        QNEPort *port = elm->input( i );
        if( !port->getName( ).isEmpty( ) ) {
          varName = QString( "%1_%2" ).arg( varName, port->getName( ) );
        }
        varName = clearString( varName );
        out << QString( "const int %1 = %2;" ).arg( varName, availablePins.front( ) ) << "\n";
        outputMap.append( MappedPin( elm, availablePins.front( ), varName, port, i ) );
        availablePins.pop_front( );
      }
    }
    counter++;
  }
  out << "\n";
}


void CodeGenerator::declareAuxVariablesRec( const QVector< GraphicElement* > &elms, bool isBox ) {
  for( GraphicElement *elm : elms ) {
    if( elm->elementType( ) == ElementType::BOX ) {
//      Box *box = qgraphicsitem_cast< Box* >( elm );

      // FIXME: Get code generator to work again
//      if( box ) {
//        out << "// " << box->getLabel( ) << "\n";
//        declareAuxVariablesRec( box->getElements( ), true );
//        out << "// END of " << box->getLabel( ) << "\n";
//        for( int i = 0; i < box->outputSize( ); ++i ) {
//          QNEPort *port = box->outputMap.at( i );
//          varMap[ box->output( i ) ] = otherPortName( port );
//        }
//      }
    }
    else {
      QString varName = QString( "aux_%1_%2" ).arg( clearString( elm->objectName( ) ) ).arg( globalCounter++ );
      auto const outputs = elm->outputs( );
      if( outputs.size( ) == 1 ) {
        QNEPort *port = outputs.first( );
        if( elm->elementType( ) == ElementType::VCC ) {
          varMap[ port ] = "HIGH";
          continue;
        }
        else if( elm->elementType( ) == ElementType::GND ) {
          varMap[ port ] = "LOW";
          continue;
        }
        else if( varMap[ port ].isEmpty( ) ) {
          varMap[ port ] = varName;
        }
      }
      else {
        int portCounter = 0;
        for( QNEPort *port : outputs ) {
          QString portName = varName;
          portName.append( QString( "_%1" ).arg( portCounter++ ) );
          if( !port->getName( ).isEmpty( ) ) {
            portName.append( QString( "_%1" ).arg( clearString( port->getName( ) ) ) );
          }
          varMap[ port ] = portName;
        }
      }
      for( QNEPort *port : outputs ) {
        QString varName2 = varMap[ port ];
        out << "boolean " << varName2 << " = " << highLow( port->defaultValue( ) ) << ";\n";
        switch( elm->elementType( ) ) {
            case ElementType::CLOCK: {
            if( !isBox ) {
              Clock *clk = qgraphicsitem_cast< Clock* >( elm );
              out << "elapsedMillis " << varName2 << "_elapsed = 0;\n";
              out << "int " << varName2 << "_interval = " << 1000 / clk->getFrequency( ) << ";\n";
            }
            break;
          }
            case ElementType::DFLIPFLOP: {
            out << "boolean " << varName2 << "_inclk = LOW;\n";
            out << "boolean " << varName2 << "_last = LOW;\n";
            break;
          }
            case ElementType::TFLIPFLOP:
            case ElementType::SRFLIPFLOP:
            case ElementType::JKFLIPFLOP: {
            out << "boolean " << varName2 << "_inclk = LOW;\n";
            break;
          }

            default:
            break;
        }
      }
    }
  }
}

void CodeGenerator::declareAuxVariables( ) {
  out << "/* ====== Aux. Variables ====== */\n";
  declareAuxVariablesRec( elements );
  out << "\n";
}

void CodeGenerator::setup( ) {
  out << "void setup( ) {\n";
  for( const MappedPin& pin : qAsConst( inputMap ) ) {
    out << "    pinMode( " << pin.varName << ", INPUT );\n";
  }
  for( const MappedPin& pin : qAsConst( outputMap ) ) {
    out << "    pinMode( " << pin.varName << ", OUTPUT );\n";
  }
  out << "}\n" << "\n";
}

void CodeGenerator::assignVariablesRec( const QVector< GraphicElement* > &elms ) {
  for( GraphicElement *elm : elms ) {
    if( elm->elementType( ) == ElementType::BOX ) {
      throw std::runtime_error( QString( "BOX element not supported : %1" ).arg(
                                  elm->objectName( ) ).toStdString( ) );
      // TODO: CodeGenerator::assignVariablesRec for Box Element
//      Box *box = qgraphicsitem_cast< Box* >( elm );
//      out << "    // " << box->getLabel( ) << "\n";
//      for( int i = 0; i < box->inputSize( ); ++i ) {
//        QNEPort *port = box->input( i );
//        QNEPort *otherPort = port->connections( ).first( )->otherPort( port );
//        QString value = highLow( port->defaultValue( ) );
//        if( !varMap[ otherPort ].isEmpty( ) ) {
//          value = varMap[ otherPort ];
//        }
//        out << "    " << varMap[ box->inputMap.at( i ) ] << " = " << value << ";\n";
//      }
//      QVector< GraphicElement* > boxElms = box->getElements( );
//      if( boxElms.isEmpty( ) ) {
//        continue;
//      }
//      boxElms = SimulationController::sortElements( boxElms );
//      assignVariablesRec( boxElms );
//      out << "    // End of " << box->getLabel( ) << "\n";
    }
    else if( elm->inputs( ).isEmpty( ) || elm->outputs( ).isEmpty( ) ) {
      continue;
    }
    else {
      QString firstOut = varMap[ elm->output( 0 ) ];
      switch( elm->elementType( ) ) {
          case ElementType::DFLIPFLOP: {
          QString secondOut = varMap[ elm->output( 1 ) ];
          QString data = otherPortName( elm->input( 0 ) );
          QString clk = otherPortName( elm->input( 1 ) );
          QString inclk = firstOut + "_inclk";
          QString last = firstOut + "_last";
          out << QString( "    //D FlipFlop" ) << "\n";
          out << QString( "    if( %1 && !%2) { " ).arg( clk, inclk ) << "\n";
          out << QString( "        %1 = %2;" ).arg( firstOut, last ) << "\n";
          out << QString( "        %1 = !%2;" ).arg( secondOut, last ) << "\n";
          out << QString( "    }" ) << "\n";
          QString prst = otherPortName( elm->input( 2 ) );
          QString clr = otherPortName( elm->input( 3 ) );
          out << QString( "    if( !%1 || !%2) { " ).arg( prst, clr ) << "\n";
          out << QString( "        %1 = !%2; //Preset" ).arg( firstOut, prst ) << "\n";
          out << QString( "        %1 = !%2; //Clear" ).arg( secondOut, clr ) << "\n";
          out << QString( "    }" ) << "\n";

          /* Updating internal clock. */
          out << "    " << inclk << " = " << clk << ";\n";
          out << "    " << last << " = " << data << ";\n";
          out << QString( "    //End of D FlipFlop" ) << "\n";

          break;
        }
          case ElementType::DLATCH: {
          QString secondOut = varMap[ elm->output( 1 ) ];
          QString data = otherPortName( elm->input( 0 ) );
          QString clk = otherPortName( elm->input( 1 ) );
          out << QString( "    //D Latch" ) << "\n";
          out << QString( "    if( %1 ) { " ).arg( clk ) << "\n";
          out << QString( "        %1 = %2;" ).arg( firstOut, data ) << "\n";
          out << QString( "        %1 = !%2;" ).arg( secondOut, data ) << "\n";
          out << QString( "    }" ) << "\n";
          out << QString( "    //End of D Latch" ) << "\n";
          break;
        }
          case ElementType::JKFLIPFLOP: {
          QString secondOut = varMap[ elm->output( 1 ) ];
          QString j = otherPortName( elm->input( 0 ) );
          QString clk = otherPortName( elm->input( 1 ) );
          QString k = otherPortName( elm->input( 2 ) );
          QString inclk = firstOut + "_inclk";
          out << QString( "    //JK FlipFlop" ) << "\n";
          out << QString( "    if( %1 && !%2 ) { " ).arg( clk, inclk ) << "\n";
          out << QString( "        if( %1 && %2) { " ).arg( j, k ) << "\n";
          out << QString( "            boolean aux = %1;" ).arg( firstOut ) << "\n";
          out << QString( "            %1 = %2;" ).arg( firstOut, secondOut ) << "\n";
          out << QString( "            %1 = aux;" ).arg( secondOut ) << "\n";
          out << QString( "        } else if ( %1 ) {" ).arg( j ) << "\n";
          out << QString( "            %1 = 1;" ).arg( firstOut ) << "\n";
          out << QString( "            %1 = 0;" ).arg( secondOut ) << "\n";
          out << QString( "        } else if ( %1 ) {" ).arg( k ) << "\n";
          out << QString( "            %1 = 0;" ).arg( firstOut ) << "\n";
          out << QString( "            %1 = 1;" ).arg( secondOut ) << "\n";
          out << QString( "        }" ) << "\n";
          out << QString( "    }" ) << "\n";
          QString prst = otherPortName( elm->input( 3 ) );
          QString clr = otherPortName( elm->input( 4 ) );
          out << QString( "    if( !%1 || !%2 ) { " ).arg( prst ).arg( clr ) << "\n";
          out << QString( "        %1 = !%2; //Preset" ).arg( firstOut ).arg( prst ) << "\n";
          out << QString( "        %1 = !%2; //Clear" ).arg( secondOut ).arg( clr ) << "\n";
          out << QString( "    }" ) << "\n";

          /* Updating internal clock. */
          out << "    " << inclk << " = " << clk << ";\n";
          out << QString( "    //End of JK FlipFlop" ) << "\n";
          break;
        }
          case ElementType::SRFLIPFLOP: {
          QString secondOut = varMap[ elm->output( 1 ) ];
          QString s = otherPortName( elm->input( 0 ) );
          QString clk = otherPortName( elm->input( 1 ) );
          QString r = otherPortName( elm->input( 2 ) );
          QString inclk = firstOut + "_inclk";
          out << QString( "    //SR FlipFlop" ) << "\n";
          out << QString( "    if( %1 && !%2 ) { " ).arg( clk, inclk ) << "\n";
          out << QString( "        if( %1 && %2) { " ).arg( s, r ) << "\n";
          out << QString( "            %1 = 1;" ).arg( firstOut ) << "\n";
          out << QString( "            %1 = 1;" ).arg( secondOut ) << "\n";
          out << QString( "        } else if ( %1 != %2) {" ).arg( s, r ) << "\n";
          out << QString( "            %1 = %2;" ).arg( firstOut, s ) << "\n";
          out << QString( "            %1 = %2;" ).arg( secondOut, r ) << "\n";
          out << QString( "        }" ) << "\n";
          out << QString( "    }" ) << "\n";
          QString prst = otherPortName( elm->input( 3 ) );
          QString clr = otherPortName( elm->input( 4 ) );
          out << QString( "    if( !%1 || !%2 ) { " ).arg( prst ).arg( clr ) << "\n";
          out << QString( "        %1 = !%2; //Preset" ).arg( firstOut ).arg( prst ) << "\n";
          out << QString( "        %1 = !%2; //Clear" ).arg( secondOut ).arg( clr ) << "\n";
          out << QString( "    }" ) << "\n";

          /* Updating internal clock. */
          out << "    " << inclk << " = " << clk << ";\n";
          out << QString( "    //End of SR FlipFlop" ) << "\n";
          break;
        }
          case ElementType::TFLIPFLOP: {
          QString secondOut = varMap[ elm->output( 1 ) ];
          QString t = otherPortName( elm->input( 0 ) );
          QString clk = otherPortName( elm->input( 1 ) );
          QString inclk = firstOut + "_inclk";
//          QString last = firstOut + "_last";
          out << QString( "    //T FlipFlop" ) << "\n";
          out << QString( "    if( %1 && !%2) { " ).arg( clk ).arg( inclk ) << "\n";
          out << QString( "        if( %1 ) { " ).arg( t ) << "\n";
          out << QString( "            %1 = !%1;" ).arg( firstOut ) << "\n";
          out << QString( "            %1 = !%2;" ).arg( secondOut ).arg( firstOut ) << "\n";
          out << QString( "        }" ) << "\n";
          out << QString( "    }" ) << "\n";
          QString prst = otherPortName( elm->input( 2 ) );
          QString clr = otherPortName( elm->input( 3 ) );
          out << QString( "    if( !%1 || !%2) { " ).arg( prst ).arg( clr ) << "\n";
          out << QString( "        %1 = !%2; //Preset" ).arg( firstOut ).arg( prst ) << "\n";
          out << QString( "        %1 = !%2; //Clear" ).arg( secondOut ).arg( clr ) << "\n";
          out << QString( "    }" ) << "\n";

          /* Updating internal clock. */
          out << "    " << inclk << " = " << clk << ";\n";
//          out << "    " << last << " = " << data << ";\n";
          out << QString( "    //End of T FlipFlop" ) << "\n";

          break;
        }
          case ElementType::AND:
          case ElementType::OR:
          case ElementType::NAND:
          case ElementType::NOR:
          case ElementType::XOR:
          case ElementType::XNOR:
          case ElementType::NOT:
          case ElementType::NODE:
          assignLogicOperator( elm );
          break;
          default:
          throw std::runtime_error( ERRORMSG( QString( "Element type not supported : %1" ).arg(
                                                elm->objectName( ) ).toStdString( ) ) );
      }
    }
  }
}

void CodeGenerator::assignLogicOperator( GraphicElement *elm ) {
  bool negate = false;
  bool parentheses = true;
  QString logicOperator;
  switch( elm->elementType( ) ) {
      case ElementType::AND: {
      logicOperator = "&&";
      break;
    }
      case ElementType::OR: {
      logicOperator = "||";
      break;
    }
      case ElementType::NAND: {
      logicOperator = "&&";
      negate = true;
      break;
    }
      case ElementType::NOR: {
      logicOperator = "||";
      negate = true;
      break;
    }
      case ElementType::XOR: {
      logicOperator = "^";
      break;
    }
      case ElementType::XNOR: {
      logicOperator = "^";
      negate = true;
      break;
    }
      case ElementType::NOT: {
      negate = true;
      parentheses = false;
      break;
    }
      default:
      break;
  }
  if( elm->outputs( ).size( ) == 1 ) {
    QString varName = varMap[ elm->output( ) ];
    QNEPort *inPort = elm->input( );
    out << "    " << varName << " = ";
    if( negate ) {
      out << "!";
    }
    if( parentheses && negate ) {
      out << "( ";
    }
    if( !inPort->connections( ).isEmpty( ) ) {
      out << otherPortName( inPort );
      for( int i = 1; i < elm->inputs( ).size( ); ++i ) {
        inPort = elm->inputs( )[ i ];
        out << " " << logicOperator << " ";
        out << otherPortName( inPort );
      }
    }
    if( parentheses && negate ) {
      out << " )";
    }
    out << ";\n";
  }
  else {
    /* ... */
  }
}


void CodeGenerator::loop( ) {
  out << "void loop( ) {\n";
  out << "    // Reading input data //.\n";
  for( MappedPin pin : inputMap ) {
    out << QString( "    %1_val = digitalRead( %1 );" ).arg( pin.varName ) << "\n";
  }
  out << "\n";
  out << "    // Updating clocks. //\n";
  for( GraphicElement *elm : elements ) {
    if( elm->elementType( ) == ElementType::CLOCK ) {
      const auto elm_outputs = elm->outputs( );
      QString varName = varMap[ elm_outputs.first( ) ];
      out << QString( "    if( %1_elapsed > %1_interval ){" ).arg( varName ) << "\n";
      out << QString( "        %1_elapsed = 0;" ).arg( varName ) << "\n";
      out << QString( "        %1 = ! %1;" ).arg( varName ) << "\n";
      out << QString( "    }" ) << "\n";
    }
  }
/* Aux variables. */
  out << "\n";
  out << "    // Assigning aux variables. //\n";
  assignVariablesRec( elements );
  out << "\n";
  out << "    // Writing output data. //\n";
  for( MappedPin pin : outputMap ) {
    QString varName = otherPortName( pin.port );
    if( varName.isEmpty( ) ) {
      varName = highLow( pin.port->defaultValue( ) );
    }
    out << QString( "    digitalWrite( %1, %2 );" ).arg( pin.varName, varName ) << "\n";
  }
  out << "}\n";
}
