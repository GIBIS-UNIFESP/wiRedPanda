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

#include "qneport.h"

#include <QFontMetrics>
#include <QGraphicsScene>
#include <graphicelement.h>

#include <QCursor>
#include <QPen>
#include <iostream>

#include "qneconnection.h"

QNEPort::QNEPort( QGraphicsItem *parent ) : QGraphicsPathItem( parent ) {
  label = new QGraphicsTextItem( this );
  setAcceptHoverEvents( true );
  radius_ = 5;
  margin = 2;

  QPainterPath p;
  p.addRect( -radius_, -radius_, 2 * radius_, 2 * radius_ );
  setPath( p );

  setPen( QPen( Qt::darkRed ) );
  setCurrentBrush( Qt::red );

  setFlag( QGraphicsItem::ItemSendsScenePositionChanges );

  m_portFlags = 0;
  m_value = false;
  m_graphicElement = NULL;
  m_required = true;
}

QNEPort::~QNEPort( ) {
  foreach( QNEConnection * conn, m_connections )
  delete conn;
}

void QNEPort::setNEBlock( QNEBlock *b ) {
  m_block = b;
}

void QNEPort::setName( const QString &n ) {
  name = n;
//  label->setPlainText( n );
  setToolTip(n);
}

void QNEPort::setIsOutput( bool o ) {
  isOutput_ = o;
  if( isInput( ) ) {
    setPen( QPen( Qt::black ) );
    setCurrentBrush( QColor( 0x333333 ) );
  }
  if( scene( ) ) {
    QFontMetrics fm( scene( )->font( ) );
  }
/*  QRect r = fm.boundingRect(name); */
  if( isOutput_ ) {
    label->setPos( -radius_ - margin - label->boundingRect( ).width( ), -label->boundingRect( ).height( ) / 2 );
  }
  else {
    label->setPos( radius_ + margin, -label->boundingRect( ).height( ) / 2 );
  }
}

int QNEPort::radius( ) const {
  return( radius_ );
}

bool QNEPort::isOutput( ) const {
  return( isOutput_ );
}

bool QNEPort::isInput( ) const {
  return( !isOutput_ );
}

QList< QNEConnection* > &QNEPort::connections( ) {
  return( m_connections );
}

void QNEPort::connect( QNEConnection *conn ) {
  graphicElement( )->setChanged( true );
  if( conn && !m_connections.contains( conn ) ) {
    m_connections.append( conn );
  }
  updateConnections( );
}

void QNEPort::disconnect( QNEConnection *conn ) {
  graphicElement( )->setChanged( true );
  m_connections.removeAll( conn );
  updateConnections( );
}

void QNEPort::setPortFlags( int f ) {
  m_portFlags = f;
  if( m_portFlags & TypePort ) {
    QFont font( scene( )->font( ) );
    font.setItalic( true );
    label->setFont( font );
    setPath( QPainterPath( ) );
  }
  else if( m_portFlags & NamePort ) {
    QFont font( scene( )->font( ) );
    font.setBold( true );
    label->setFont( font );
    setPath( QPainterPath( ) );
  }
}

QNEBlock* QNEPort::block( ) const {
  return( m_block );
}

quint64 QNEPort::ptr( ) {
  return( m_ptr );
}

void QNEPort::setPtr( quint64 p ) {
  m_ptr = p;
}

bool QNEPort::isConnected( QNEPort *other ) {
  foreach( QNEConnection * conn, m_connections )
  if( ( conn->port1( ) == other ) || ( conn->port2( ) == other ) ) {
    return( true );
  }
  return( false );
}

void QNEPort::updateConnections( ) {
  foreach( QNEConnection * conn, m_connections ) {
    conn->updatePosFromPorts( );
    conn->updatePath( );
  }
}

QVariant QNEPort::itemChange( GraphicsItemChange change, const QVariant &value ) {
  if( change == ItemScenePositionHasChanged ) {
    updateConnections( );
  }
  return( value );
}

QBrush QNEPort::currentBrush( ) const {
  return( _currentBrush );
}

void QNEPort::setCurrentBrush( const QBrush &currentBrush ) {
  _currentBrush = currentBrush;
  setBrush( currentBrush );
}


bool QNEPort::required( ) const {
  return( m_required );
}

void QNEPort::setRequired( bool required ) {
  m_required = required;
}

char QNEPort::value( ) const {
  if( !required( ) && ( m_connections.size( ) == 0 ) ) {
    return( 0 );
  }
  else {
    return( m_value );
  }
}

void QNEPort::setValue( char value ) {
  m_value = value;
  if( isOutput( ) ) {
    foreach( QNEConnection * conn, connections( ) ) {
      if( value == -1 ) {
        conn->setStatus( QNEConnection::Invalid );
      }
      else if( value == 0 ) {
        conn->setStatus( QNEConnection::Inactive );
      }
      else {
        conn->setStatus( QNEConnection::Active );
      }
      QNEPort *port = conn->otherPort( this );
      if( port ) {
        port->setValue( value );
      }
    }
  }
  update( );
}

GraphicElement* QNEPort::graphicElement( ) const {
  return( m_graphicElement );
}

void QNEPort::setGraphicElement( GraphicElement *graphicElement ) {
  m_graphicElement = graphicElement;
}

void QNEPort::hoverEnter( ) {
  setBrush( QBrush( Qt::green ) );
  update( );
}

void QNEPort::hoverLeave( ) {
  setBrush( currentBrush( ) );
  update( );
}
