#include "simplewaveform.h"
#include "ui_simplewaveform.h"

#include <QChartView>
#include <QDebug>
#include <QLineSeries>
#include <cmath>
#include <input.h>

#include <QBuffer>
#include <QClipboard>
#include <QMessageBox>
#include <QMimeData>
#include <QPointF>
#include <QSettings>
/* #include <QSvgGenerator> */
#include <QValueAxis>
#include <bitset>

using namespace QtCharts;


class SCStop {
  SimulationController *sc;
  bool restart = false;
public:
  SCStop( SimulationController *sc ) : sc( sc ) {
    if( sc->isRunning( ) ) {
      restart = true;
      sc->stop( );
    }
  }
  ~SCStop( ) {
    if( restart ) {
      sc->start( );
    }
  }
};

SimpleWaveform::SimpleWaveform( Editor *editor, QWidget *parent ) :
  QDialog( parent ),
  ui( new Ui::SimpleWaveform ),
  editor( editor ) {
  ui->setupUi( this );
  resize( 800, 500 );
  chart.legend( )->setAlignment( Qt::AlignLeft );

/*  chart.legend( )->hide( ); */
/*  chart->addSeries(series); */

/*  chart.setTitle( "Simple Waveform View." ); */

  chartView = new QChartView( &chart, this );
  chartView->setRenderHint( QPainter::Antialiasing );
  ui->gridLayout->addWidget( chartView );
  setWindowTitle( "Simple WaveForm - WaveDolphin Beta" );
  setWindowFlags( Qt::Window );
  setModal( true );
  sortingKind = SortingKind::INCREASING;
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "SimpleWaveform" );
  restoreGeometry( settings.value( "geometry" ).toByteArray( ) );
  settings.endGroup( );
}

SimpleWaveform::~SimpleWaveform( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "SimpleWaveform" );
  settings.setValue( "geometry", saveGeometry( ) );
  settings.endGroup( );
  delete ui;
}

void SimpleWaveform::sortElements( QVector< GraphicElement* > elements,
                                   QVector< GraphicElement* > &inputs,
                                   QVector< GraphicElement* > &outputs,
                                   SortingKind sorting ) {
  elements = SimulationController::sortElements( elements );
  for( GraphicElement *elm : elements ) {
    if( elm && ( elm->type( ) == GraphicElement::Type ) ) {
      switch( elm->elementType( ) ) {
          case ElementType::BUTTON:
          case ElementType::SWITCH:
          case ElementType::CLOCK: {
          inputs.append( elm );
          break;
        }
          case ElementType::DISPLAY:
          case ElementType::LED: {
          outputs.append( elm );
          break;
        }
          default:
          break;
      }
    }
  }
  std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
    return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
  } );
  std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
    return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
  } );

  std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
    return( elm1->pos( ).rx( ) < elm2->pos( ).rx( ) );
  } );
  std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
    return( elm1->pos( ).rx( ) < elm2->pos( ).rx( ) );
  } );
  if( sorting == SortingKind::INCREASING ) {
    std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) <= 0 );
    } );
    std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) <= 0 );
    } );
  }
  else if( sorting == SortingKind::DECREASING ) {
    std::stable_sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) >= 0 );
    } );
    std::stable_sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) >= 0 );
    } );
  }
}

bool SimpleWaveform::saveToTxt( QTextStream &outStream, Editor *editor ) {
  QVector< GraphicElement* > elements = editor->getScene( )->getElements( );
  QVector< GraphicElement* > inputs;
  QVector< GraphicElement* > outputs;

  sortElements( elements, inputs, outputs, SortingKind::INCREASING );
  if( elements.isEmpty( ) || inputs.isEmpty( ) || outputs.isEmpty( ) ) {
    return( false );
  }
  SimulationController *sc = editor->getSimulationController( );
  SCStop scst( sc );


  QVector< char > oldValues( inputs.size( ) );
  for( int in = 0; in < inputs.size( ); ++in ) {
    oldValues[ in ] = inputs[ in ]->output( )->value( );
  }
  int num_iter = pow( 2, inputs.size( ) );
  int outputCount = 0;
  for( GraphicElement *out : outputs ) {
    outputCount += out->inputSize( );
  }
  QVector< QVector< uchar > > results( outputCount, QVector< uchar >( num_iter ) );
  for( int itr = 0; itr < num_iter; ++itr ) {
    std::bitset< std::numeric_limits< unsigned int >::digits > bs( itr );
    for( int in = 0; in < inputs.size( ); ++in ) {
      uchar val = bs[ inputs.size( ) - in - 1 ];
      dynamic_cast< Input* >( inputs[ in ] )->setOn( val );
    }
    for( GraphicElement *elm : elements ) {
      elm->updateLogic( );
    }
    int counter = 0;
    for( int out = 0; out < outputs.size( ); ++out ) {
      int inSz = outputs[ out ]->inputSize( );
      for( int port = inSz - 1; port >= 0; --port ) {
        uchar val = outputs[ out ]->input( port )->value( );
        results[ counter ][ itr ] = val;
        counter++;
      }
    }
  }
  for( int in = 0; in < inputs.size( ); ++in ) {
    QString label = inputs[ in ]->getLabel( );
    if( label.isEmpty( ) ) {
      label = ElementFactory::translatedName( inputs[ in ]->elementType( ) );
    }
    outStream << "\"" << label << "\":";
    for( int itr = 0; itr < num_iter; ++itr ) {
      std::bitset< std::numeric_limits< unsigned int >::digits > bs( itr );
      outStream << ( int ) bs[ inputs.size( ) - in - 1 ];
    }
    outStream << "\n";
  }
  outStream << "\n";
  int counter = 0;
  for( int out = 0; out < outputs.size( ); ++out ) {
    QString label = outputs[ out ]->getLabel( );
    if( label.isEmpty( ) ) {
      label = ElementFactory::translatedName( outputs[ out ]->elementType( ) );
    }
    int inSz = outputs[ out ]->inputSize( );
    for( int port = inSz - 1; port >= 0; --port ) {
      outStream << "\"" << label << "[" << port << "]\":";
      for( int itr = 0; itr < num_iter; ++itr ) {
        outStream << ( int ) results[ counter ][ itr ];
      }
      counter += 1;
      outStream << "\n";
    }
  }
  for( int in = 0; in < inputs.size( ); ++in ) {
    dynamic_cast< Input* >( inputs[ in ] )->setOn( oldValues[ in ] );
  }
  return( true );
}

void SimpleWaveform::showWaveform( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  if( settings.contains( "sortingType" ) ) {
    sortingKind = ( SortingKind ) settings.value( "sortingType" ).toInt( );
  }
  settings.endGroup( );
  switch( sortingKind ) {
      case SortingKind::DECREASING:
      ui->radioButton_Decreasing->setChecked( true );
      break;
      case SortingKind::INCREASING:
      ui->radioButton_Increasing->setChecked( true );
      break;
      case SortingKind::POSITION:
      ui->radioButton_Position->setChecked( true );
      break;
  }
  int gap = 2;

  chart.removeAllSeries( );

  SimulationController *sc = editor->getSimulationController( );
  SCStop scst( sc );

  QVector< GraphicElement* > elements = editor->getScene( )->getElements( );
  QVector< GraphicElement* > inputs;
  QVector< GraphicElement* > outputs;

  sortElements( elements, inputs, outputs, sortingKind );
  if( elements.isEmpty( ) ) {
    QMessageBox::warning( parentWidget( ), tr( "Error" ), tr( "Could not find any port for the simulation" ) );
    return;
  }
  if( inputs.isEmpty( ) ) {
    QMessageBox::warning( parentWidget( ), tr( "Error" ), tr( "Could not find any input for the simulation." ) );
    return;
  }
  if( outputs.isEmpty( ) ) {
    QMessageBox::warning( parentWidget( ), tr( "Error" ), tr( "Could not find any output for the simulation." ) );
    return;
  }
  if( inputs.size( ) > 8 ) {
    QMessageBox::warning( parentWidget( ), tr( "Error" ), tr( "The simulation is limited to 8 inputs." ) );
    return;
  }
  QVector< QLineSeries* > in_series;
  QVector< QLineSeries* > out_series;

  QVector< char > oldValues( inputs.size( ) );
  for( int in = 0; in < inputs.size( ); ++in ) {
    in_series.append( new QLineSeries( this ) );
    QString label = inputs[ in ]->getLabel( );
    if( label.isEmpty( ) ) {
      label = ElementFactory::translatedName( inputs[ in ]->elementType( ) );
    }
    in_series[ in ]->setName( label );
    oldValues[ in ] = inputs[ in ]->output( )->value( );
  }
  for( int out = 0; out < outputs.size( ); ++out ) {
    QString label = outputs[ out ]->getLabel( );
    if( label.isEmpty( ) ) {
      label = ElementFactory::translatedName( outputs[ out ]->elementType( ) );
    }
    for( int port = 0; port < outputs[ out ]->inputSize( ); ++port ) {
      out_series.append( new QLineSeries( this ) );
      if( outputs[ out ]->inputSize( ) > 1 ) {
        out_series.last( )->setName( QString( "%1_%2" ).arg( label ).arg( port ) );
      }
      else {
        out_series.last( )->setName( label );
      }
    }
  }
  qDebug( ) << in_series.size( ) << " inputs";
  qDebug( ) << out_series.size( ) << " outputs";

  int num_iter = pow( 2, in_series.size( ) );
  qDebug( ) << "Num iter = " << num_iter;
/*  gap += outputs.size( ) % 2; */
  for( int itr = 0; itr < num_iter; ++itr ) {
    std::bitset< std::numeric_limits< unsigned int >::digits > bs( itr );
    qDebug( ) << itr;
    for( int in = 0; in < inputs.size( ); ++in ) {
      float val = bs[ in ];
      dynamic_cast< Input* >( inputs[ in ] )->setOn( val );
      float offset = ( in_series.size( ) - in - 1 + out_series.size( ) ) * 2 + gap + 0.5;
      in_series[ in ]->append( itr, offset + val );
      in_series[ in ]->append( itr + 1, offset + val );
    }
    for( GraphicElement *elm : elements ) {
      elm->updateLogic( );
    }
    int counter = 0;
    for( int out = 0; out < outputs.size( ); ++out ) {
      int inSz = outputs[ out ]->inputSize( );
      for( int port = inSz - 1; port >= 0; --port ) {
        float val = outputs[ out ]->input( port )->value( ) > 0;
        float offset = ( out_series.size( ) - counter - 1 ) * 2 + 0.5;
        out_series[ counter ]->append( itr, offset + val );
        out_series[ counter ]->append( itr + 1, offset + val );
/*        cout << counter << " " << out; */
        counter++;
      }
    }
  }
  for( QLineSeries *in : in_series ) {
    chart.addSeries( in );
  }
  for( QLineSeries *out : out_series ) {
    chart.addSeries( out );
  }
  chart.createDefaultAxes( );

/*  chart.axisY( )->hide( ); */
  QValueAxis *ax = dynamic_cast< QValueAxis* >( chart.axisX( ) );
  ax->setRange( 0, num_iter );
  ax->setTickCount( num_iter + 1 );
  ax->setLabelFormat( QString( "%i" ) );
  QValueAxis *ay = dynamic_cast< QValueAxis* >( chart.axisY( ) );
/*  ay->setShadesBrush( QBrush( Qt::lightGray ) ); */

  ay->setShadesColor( QColor( 0, 0, 0, 8 ) );
  ay->setShadesPen( QPen( QColor( 0, 0, 0, 0 ) ) );
  ay->setShadesVisible( true );

  ay->setGridLineVisible( false );
  ay->setTickCount( ( in_series.size( ) + out_series.size( ) + gap / 2 + 1 ) );
  ay->setRange( 0, in_series.size( ) * 2 + out_series.size( ) * 2 + gap );
  ay->setGridLineColor( Qt::transparent );
  ay->setLabelsVisible( false );
/*  ay->hide( ); */

  exec( );
  for( int in = 0; in < inputs.size( ); ++in ) {
    dynamic_cast< Input* >( inputs[ in ] )->setOn( oldValues[ in ] );

  }
}

void SimpleWaveform::on_radioButton_Position_clicked( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  sortingKind = SortingKind::POSITION;
  settings.setValue( "sortingType", ( int ) sortingKind );
  settings.endGroup( );

  showWaveform( );
}

void SimpleWaveform::on_radioButton_Increasing_clicked( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  sortingKind = SortingKind::INCREASING;
  settings.setValue( "sortingType", ( int ) sortingKind );
  settings.endGroup( );

  showWaveform( );
}

void SimpleWaveform::on_radioButton_Decreasing_clicked( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  sortingKind = SortingKind::DECREASING;
  settings.setValue( "sortingType", ( int ) sortingKind );
  settings.endGroup( );

  showWaveform( );
}

void SimpleWaveform::on_pushButton_Copy_clicked( ) {
  QSize s = chart.size( ).toSize( );
  QPixmap p( s );
  p.fill( Qt::white );
  QPainter painter;
  painter.begin( &p );
  painter.setRenderHint( QPainter::Antialiasing );
  chart.paint( &painter, 0, 0 ); /* This gives 0 items in 1 group */
  chartView->render( &painter ); /* m_view has app->chart() in it, and this one gives right image */
  qDebug( ) << "Copied";
  painter.end( );
  QMimeData *d = new QMimeData( );
  d->setImageData( p );
  QApplication::clipboard( )->setMimeData( d, QClipboard::Clipboard );
}
