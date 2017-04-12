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

#ifndef QNEPORT_H
#define QNEPORT_H

#include <QGraphicsPathItem>
#include <qpen.h>

class QNEBlock;
class QNEConnection;
class GraphicElement;

class QNEPort : public QGraphicsPathItem {
public:
  enum { Type = QGraphicsItem::UserType + 1 };
  enum { NamePort = 1, TypePort = 2 };

  explicit QNEPort( QGraphicsItem *parent = 0 );

  void setNEBlock( QNEBlock* );
  void setName( const QString &n );
  int radius( ) const;
  const QList< QNEConnection* > &connections( ) const;
  void connect( QNEConnection *conn );
  void disconnect( QNEConnection *conn );
  bool isConnected( QNEPort* );
  void setPortFlags( int );

  virtual bool isOutput( ) const = 0;

  const QString &portName( ) const;
  int portFlags( ) const;

  int type( ) const;

  quint64 ptr( );
  void setPtr( quint64 );


  GraphicElement* graphicElement( ) const;
  void setGraphicElement( GraphicElement *graphicElement );

  void updateConnections( );
  char value( ) const;
  virtual void setValue( char value ) = 0;

  bool required( ) const;
  void setRequired( bool required );

  QBrush currentBrush( ) const;
  void setCurrentBrush( const QBrush &currentBrush );

  int defaultValue( ) const;
  void setDefaultValue( int defaultValue );

  QString getName( ) const;

  virtual bool isValid( ) const = 0;

  void hoverLeave( );
  void hoverEnter( );

protected:
  QVariant itemChange( GraphicsItemChange change, const QVariant &value );
  int m_defaultValue;
  QNEBlock *m_block;
  QString name;
  QGraphicsTextItem *label;
  int radius_;
  int margin;
  QList< QNEConnection* > m_connections;
  int m_portFlags;
  quint64 m_ptr;

  bool m_required;

  /* WPanda */
  GraphicElement *m_graphicElement;
  QBrush _currentBrush;

  /* QGraphicsItem interface */
  char m_value;

  virtual void updateTheme() = 0;
};

class QNEInputPort : public QNEPort {
public:
  explicit QNEInputPort( QGraphicsItem *parent );
  virtual ~QNEInputPort( );
  /* QNEPort interface */
public:
  void setValue( char value );
  bool isOutput( ) const;
  bool isValid( ) const;
  void updateTheme();
};

class QNEOutputPort : public QNEPort {
public:
  explicit QNEOutputPort( QGraphicsItem *parent );
  virtual ~QNEOutputPort( );
  /* QNEPort interface */
public:
  void setValue( char value );
  bool isOutput( ) const;
  bool isValid( ) const;
  void updateTheme();
};


#endif /* QNEPORT_H */
