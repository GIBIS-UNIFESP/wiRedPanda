#include "simplewaveform.h"
#include "ui_simplewaveform.h"

#include <QChartView>
#include <QDebug>
#include <QLineSeries>
#include <cmath>
#include <input.h>

#include <QPointF>
#include <QValueAxis>
#include <bitset>

using namespace QtCharts;


SimpleWaveform::SimpleWaveform( QWidget *parent ) :
  QDialog( parent ),
  ui( new Ui::SimpleWaveform ) {
  ui->setupUi( this );
  resize( 800, 500 );
/*  chart.legend( )->hide( ); */
/*  chart->addSeries(series); */

/*  chart.setTitle( "Simple Waveform View." ); */

  QChartView *chartView = new QChartView( &chart, this );
  chartView->setRenderHint( QPainter::Antialiasing );
  ui->gridLayout->addWidget( chartView );
  setWindowTitle( "Simple WaveForm - WaveDolphin Beta" );
}

SimpleWaveform::~SimpleWaveform( ) {
  delete ui;
}

void SimpleWaveform::showWaveform( Editor *editor ) {

  QVector< GraphicElement* > elements = editor->getScene( )->getElements( );
  SimulationController *sc = editor->getSimulationController( );
  sc->stop( );

  QVector< GraphicElement* > inputs;
  QVector< GraphicElement* > outputs;

  QVector< QLineSeries* > in_series;
  QVector< QLineSeries* > out_series;
  for( GraphicElement *elm : elements ) {
    switch( elm->elementType( ) ) {
        case ElementType::BUTTON:
        case ElementType::SWITCH: {
        inputs.append( elm );
        in_series.append( new QLineSeries( this ) );
        break;
      }
        case ElementType::LED: {
        outputs.append( elm );
        out_series.append( new QLineSeries( this ) );
        break;
      }
        default:
        break;
    }
  }
  std::sort( inputs.begin( ), inputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
    return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
  } );

  QVector< char > oldValues( inputs.size( ) );
  for( int in = 0; in < inputs.size( ); ++in ) {
    in_series[ in ]->setName( inputs[ in ]->getLabel( ) );
    oldValues[ in ] = inputs[ in ]->output( )->value( );
    chart.addSeries( in_series[ in ] );
  }
  std::sort( outputs.begin( ), outputs.end( ), [ ]( GraphicElement *elm1, GraphicElement *elm2 ) {
    return( elm1->pos( ).ry( ) < elm2->pos( ).ry( ) );
  } );
  for( int out = 0; out < outputs.size( ); ++out ) {
    out_series[ out ]->setName( outputs[ out ]->getLabel( ) );
    chart.addSeries( out_series[ out ] );
  }
  int num_iter = pow( 2, inputs.size( ) );
/*  qDebug( ) << "Num iter = " << num_iter; */
  for( int itr = 0; itr < num_iter; ++itr ) {
    std::bitset< std::numeric_limits< unsigned int >::digits > bs( itr );
    for( int in = 0; in < inputs.size( ); ++in ) {
      dynamic_cast< Input* >( inputs[ in ] )->setOn( bs[ in ] );
      int offset = ( inputs.size() - in - 1 ) * 2 + outputs.size( ) * 2 + 2;
      in_series[ in ]->append( itr, offset + bs[ inputs.size() - in - 1 ] );
      in_series[ in ]->append( itr + 1, offset + bs[ inputs.size() - in - 1 ] );
    }
    for( GraphicElement *elm : elements ) {
      elm->updateLogic( );
    }
    for( int out = 0; out < outputs.size( ); ++out ) {
      int val = outputs[ out ]->input( )->value( );
      int offset = ( outputs.size() - out - 1 ) * 2;
      out_series[ out ]->append( itr, offset + val );
      out_series[ out ]->append( itr + 1, offset + val );
    }
  }
  chart.createDefaultAxes( );

/*  chart.axisY( )->hide( ); */
  QValueAxis *ax = dynamic_cast< QValueAxis* >( chart.axisX( ) );
  ax->setRange( 0, num_iter );
  ax->setTickCount( num_iter + 1 );
  QValueAxis *ay = dynamic_cast< QValueAxis* >( chart.axisY( ) );
  ay->setTickCount( inputs.size( ) * 2 + outputs.size( ) * 2 + 3 );
  ay->setRange( 0, inputs.size( ) * 2 + outputs.size( ) * 2 + 2 );
  ay->hide( );
  show( );
  for( int in = 0; in < inputs.size( ); ++in ) {
    dynamic_cast< Input* >( inputs[ in ] )->setOn( oldValues[ in ] );

  }
  sc->start( );
}
