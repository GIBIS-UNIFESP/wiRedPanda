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
  sortingType = SortingType::INCREASING;
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

void SimpleWaveform::showWaveform( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  if( settings.contains( "sortingType" ) ) {
    sortingType = ( SortingType ) settings.value( "sortingType" ).toInt( );
  }
  settings.endGroup( );
  switch( sortingType ) {
      case SortingType::DECREASING:
      ui->radioButton_Decreasing->setChecked( true );
      break;
      case SortingType::INCREASING:
      ui->radioButton_Increasing->setChecked( true );
      break;
      case SortingType::POSITION:
      ui->radioButton_Position->setChecked( true );
      break;
  }
  int gap = 2;

  chart.removeAllSeries( );

  QVector< GraphicElement* > elements = editor->getScene( )->getElements( );
  if( elements.isEmpty( ) ) {
    QMessageBox::warning(parentWidget(), tr("Error"), tr("Could not find any input for the simulation"));
    return;
  }
  SimulationController *sc = editor->getSimulationController( );
  sc->stop( );

  QVector< GraphicElement* > inputs;
  QVector< GraphicElement* > outputs;

  for( GraphicElement *elm : elements ) {
    if( elm && ( elm->type( ) == GraphicElement::Type ) ) {
      switch( elm->elementType( ) ) {
          case ElementType::BUTTON:
          case ElementType::SWITCH: {
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
  if( inputs.isEmpty( ) ) {
    QMessageBox::warning(parentWidget(), tr("Warning"), tr("Could not find any input for the simulation"));
    return;
  }
  if( outputs.isEmpty( ) ) {
    QMessageBox::warning(parentWidget(), tr("Warning"), tr("Could not find any output for the simulation"));
    return;
  }

  int num_iter = pow( 2, inputs.size( ) );
  qDebug( ) << "Num iter = " << num_iter;
  if(num_iter >= 256 ){
    QString msg = tr( "The simulation will have %1 iterations, do you really want to continue?" ).arg(num_iter);
    int ret = QMessageBox::warning( parentWidget(), tr( "Warning" ),msg, QMessageBox::Ok, QMessageBox::Cancel );
    if( ret == QMessageBox::Cancel ) {
      return;
    }
  }


  if( sortingType == SortingType::INCREASING ) {
    std::sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) <= 0 );
    } );
    std::sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) <= 0 );
    } );
  }
  else if( sortingType == SortingType::DECREASING ) {
    std::sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) >= 0 );
    } );
    std::sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( strcasecmp( elm1->getLabel( ).toUtf8( ), elm2->getLabel( ).toUtf8( ) ) >= 0 );
    } );
  }
  else {
    std::sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
    } );
    std::sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
      return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
    } );
  }

  QVector< QLineSeries* > in_series;
  QVector< QLineSeries* > out_series;

  QVector< char > oldValues( inputs.size( ) );
  for( int in = 0; in < inputs.size( ); ++in ) {
    in_series.append( new QLineSeries( this ) );
    QString label = inputs[ in ]->getLabel( );
    if( label.isEmpty( ) ) {
      label = inputs[ in ]->objectName( );
    }
    in_series[ in ]->setName( label );
    oldValues[ in ] = inputs[ in ]->output( )->value( );
  }
  for( int out = 0; out < outputs.size( ); ++out ) {
    QString label = outputs[ out ]->getLabel( );
    if( label.isEmpty( ) ) {
      label = outputs[ out ]->objectName( );
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
/*  gap += outputs.size( ) % 2; */

  for( int itr = 0; itr < num_iter; ++itr ) {
    std::bitset< std::numeric_limits< unsigned int >::digits > bs( itr );
    for( int in = 0; in < inputs.size( ); ++in ) {
      float val = bs[ in_series.size() - in - 1 ];
      dynamic_cast< Input* >( inputs[ in ] )->setOn( val );
      float offset = ( in_series.size() - in - 1 + out_series.size( ) ) * 2 + gap + 0.5;
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
        float offset = ( out_series.size() -  counter - 1) * 2 + 0.5;
        out_series[ counter ]->append( itr, offset + val );
        out_series[ counter ]->append( itr + 1, offset + val );
//        cout << counter << " " << out;
        counter++;
      }
    }
  }

  for(QLineSeries* in : in_series){
    chart.addSeries(in);
  }

  for(QLineSeries* out : out_series){
    chart.addSeries(out);
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
  sc->start( );
}

void SimpleWaveform::on_radioButton_Position_clicked( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  sortingType = SortingType::POSITION;
  settings.setValue( "sortingType", ( int ) sortingType );
  settings.endGroup( );

  showWaveform( );
}

void SimpleWaveform::on_radioButton_Increasing_clicked( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  sortingType = SortingType::INCREASING;
  settings.setValue( "sortingType", ( int ) sortingType );
  settings.endGroup( );

  showWaveform( );
}

void SimpleWaveform::on_radioButton_Decreasing_clicked( ) {
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );
  settings.beginGroup( "waveform" );
  sortingType = SortingType::DECREASING;
  settings.setValue( "sortingType", ( int ) sortingType );
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
