/* Copyright (c) 2012, STANISLAW ADASZEWSKI
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * Neither the name of STANISLAW ADASZEWSKI nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL STANISLAW ADASZEWSKI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "qneconnection.h"

#include "qneport.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

QNEConnection::QNEConnection( QGraphicsItem *parent ) : QGraphicsPathItem( parent ) {
  setFlag( QGraphicsItem::ItemIsSelectable );
  setBrush( Qt::NoBrush );
  setStatus( Inactive );
  setZValue( -1 );
  m_port1 = 0;
  m_port2 = 0;
}

QNEConnection::~QNEConnection( ) {
  if( m_port1 ) {
    m_port1->disconnect( this );
  }
  if( m_port2 ) {
    m_port2->disconnect( this );
  }
}

void QNEConnection::setPos1( const QPointF &p ) {
  pos1 = p;
}

void QNEConnection::setPos2( const QPointF &p ) {
  pos2 = p;
}

void QNEConnection::setPort1( QNEPort *p ) {
  m_port1 = p;
  if( p ) {
    m_port1->connect( this );
  }
}

void QNEConnection::setPort2( QNEPort *p ) {
  m_port2 = p;
  if( p ) {
    m_port2->connect( this );
  }
}

void QNEConnection::updatePosFromPorts( ) {
  if( m_port1 ) {
    pos1 = m_port1->scenePos( );
  }
  if( m_port2 ) {
    pos2 = m_port2->scenePos( );
  }
}

void QNEConnection::updatePath( ) {
  QPainterPath p;

  /*
   * QPointF pos1(m_port1->scenePos());
   * QPointF pos2(m_port2->scenePos());
   */

  p.moveTo( pos1 );

  qreal dx = pos2.x( ) - pos1.x( );
  qreal dy = pos2.y( ) - pos1.y( );

  QPointF ctr1( pos1.x( ) + dx * 0.25, pos1.y( ) + dy * 0.1 );
  QPointF ctr2( pos1.x( ) + dx * 0.75, pos1.y( ) + dy * 0.9 );

  p.cubicTo( ctr1, ctr2, pos2 );

/*  p.lineTo(pos2); */
  setPath( p );
}

QNEPort* QNEConnection::port1( ) const {
  return( m_port1 );
}

QNEPort* QNEConnection::port2( ) const {
  return( m_port2 );
}

QNEPort* QNEConnection::otherPort( QNEPort *port ) {
  if( port == m_port1 ) {
    return( m_port2 );
  }
  else {
    return( m_port1 );
  }
}

void QNEConnection::split( QPointF point ) {

/*
 *  QNEPort * port = new QNEPort(this);
 *  port->setPos(point);
 *  port->show();
 *  port2()->connections().removeOne(this);
 *  QNEConnection * conn = new QNEConnection(this);
 *  conn->setPort1(port);
 *  conn->setPort2(m_port2);
 */

/*  setPort2(port); */
}

void QNEConnection::save( QDataStream &ds ) {
  ds << ( quint64 ) m_port1;
  ds << ( quint64 ) m_port2;
}

bool QNEConnection::load( QDataStream &ds, const QMap< quint64, QNEPort* > &portMap ) {
  quint64 ptr1;
  quint64 ptr2;
  ds >> ptr1;
  ds >> ptr2;
  if( portMap.isEmpty( ) ) {
    setPort1( ( QNEPort* ) ptr1 );
    setPort2( ( QNEPort* ) ptr2 );
  }
  else {
    if( !portMap.contains( ptr1 ) || !portMap.contains( ptr2 ) ) {
      return( false );
    }
    setPort1( portMap[ ptr1 ] );
    setPort2( portMap[ ptr2 ] );
  }
  updatePosFromPorts( );
  updatePath( );
  return( true );
}

QNEConnection::Status QNEConnection::status( ) const {
  return( m_status );
}

void QNEConnection::setStatus( const Status &status ) {
  m_status = status;
  switch( status ) {
      case Inactive: {
      setPen( QPen( Qt::black, 3 ) );
      break;
    }
      case Active: {
      setPen( QPen( Qt::green, 3 ) );
      break;
    }
      case Invalid: {
      setPen( QPen( Qt::red, 5 ) );
      break;
    }
  }
}

void QNEConnection::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget* ) {
  painter->setClipRect( option->exposedRect );

  QPainterPath p;

  /*
   * QPointF pos1(m_port1->scenePos());
   * QPointF pos2(m_port2->scenePos());
   */

  p.moveTo( pos1 );
  qreal dx = pos2.x( ) - pos1.x( );
  qreal dy = pos2.y( ) - pos1.y( );

  QPointF ctr1( pos1.x( ) + dx * 0.25, pos1.y( ) + dy * 0.1 );
  QPointF ctr2( pos1.x( ) + dx * 0.75, pos1.y( ) + dy * 0.9 );

  p.cubicTo( ctr1, ctr2, pos2 );
  if( isSelected( ) ) {
    painter->setPen( QPen( Qt::darkGreen, 5 ) );
  }
  else {
    painter->setPen( pen( ) );
  }
  painter->drawPath( p );
  /*  p.lineTo(pos2); */
  setPath( p );
}
