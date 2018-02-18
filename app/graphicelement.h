#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QKeySequence>

#include "common.h"
#include "itemwithid.h"
#include "nodes/qneport.h"

enum class ElementType {
  UNKNOWN, BUTTON, SWITCH, LED, NOT, AND, OR, NAND, NOR, CLOCK, XOR, XNOR, VCC, GND, DISPLAY,
  DLATCH, JKLATCH, DFLIPFLOP, JKFLIPFLOP, SRFLIPFLOP, TFLIPFLOP, TLATCH, BOX, NODE, MUX, DEMUX,
  BUZZER
};

enum class ElementGroup {
  UNKNOWN, OTHER, BOX, INPUT, GATE, MEMORY, OUTPUT, MUX
};


#define MAXIMUMVALIDINPUTSIZE 256

class GraphicElement;

typedef QVector< GraphicElement* > ElementVector;
typedef QVector< QNEPort* > QNEPortVector;

class GraphicElement : public QGraphicsObject, public ItemWithId {
  Q_OBJECT
public:
  enum { Type = QGraphicsItem::UserType + 3 };

  explicit GraphicElement( int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz,
                           QGraphicsItem *parent = 0 );

private:
  QPixmap *pixmap;
  QString currentPixmapName;
  QColor m_selectionBrush;
  QColor m_selectionPen;

  /* GraphicElement interface. */
public:
  virtual ElementType elementType( ) = 0;

  virtual ElementGroup elementGroup( ) = 0;

  virtual void save( QDataStream &ds );

  virtual void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version );

  virtual void updatePorts( );

  virtual void updateLogic( ) = 0;

  /* QGraphicsItem interface */
public:

  int type( ) const {
    return( Type );
  }

  virtual QRectF boundingRect( ) const;

  virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

  QNEPort* addPort( const QString &name, bool isOutput, int flags = 0, int ptr = 0 );

  void addInputPort( const QString &name = QString( ) );

  void addOutputPort( const QString &name = QString( ) );

  virtual void setPortName( QString name );

  int topPosition( ) const;

  int bottomPosition( ) const;

  int maxInputSz( ) const;

  int maxOutputSz( ) const;

  bool outputsOnTop( ) const;

  QVector< QNEInputPort* > inputs( ) const;
  void setInputs( const QVector< QNEInputPort* > &inputs );

  QVector< QNEOutputPort* > outputs( ) const;

  QNEInputPort* input( int pos = 0 ) const;
  QNEOutputPort* output( int pos = 0 ) const;

  void setOutputs( const QVector< QNEOutputPort* > &outputs );

  int minInputSz( ) const;

  int minOutputSz( ) const;

  int inputSize( );
  void setInputSize( int size );

  int outputSize( );
  void setOutputSize( int size );

  virtual float getFrequency( );
  virtual void setFrequency( float freq );

  void setPixmap( const QString &pixmapName, QRect size = QRect( ) );

  bool rotatable( ) const;

  bool hasLabel( ) const;

  bool hasFrequency( ) const;

  bool hasColors( ) const;

  bool hasTrigger( ) const;

  bool hasAudio( ) const;


  virtual void setColor( QString getColor );
  virtual QString getColor( );

  virtual void setAudio( QString audio );
  virtual QString getAudio( );
/*
 *  bool beingVisited( ) const;
 *  void setBeingVisited( bool beingVisited );
 */

/*
 *  bool visited( ) const;
 *  void setVisited( bool visited );
 */

  bool isValid( );

  void setLabel( QString label );
  QString getLabel( );

  void updateTheme( );
  virtual void updateThemeLocal( );

  void disable( );
  void enable( );
  bool disabled( );

  QPixmap getPixmap( ) const;

  QKeySequence getTrigger( ) const;
  void setTrigger( const QKeySequence &trigger );

  virtual QString genericProperties( );


protected:
  void setRotatable( bool rotatable );
  void setHasLabel( bool hasLabel );
  void setHasFrequency( bool hasFrequency );
  void setHasColors( bool hasColors );
  void setHasTrigger( bool hasTrigger );
  void setMinInputSz( int minInputSz );
  void setMinOutputSz( int minOutputSz );
  void setHasAudio( bool hasAudio );
  void setOutputsOnTop( bool outputsOnTop );
  void setMaxOutputSz( int maxOutputSz );
  void setMaxInputSz( int maxInputSz );
  void setTopPosition( int topPosition );
  void setBottomPosition( int bottomPosition );

/*  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e); */
  QVariant itemChange( GraphicsItemChange change, const QVariant &value );
private:
  QGraphicsTextItem *label;
  int m_topPosition;
  int m_bottomPosition;
  quint64 m_maxInputSz;
  quint64 m_maxOutputSz;
  quint64 m_minInputSz;
  quint64 m_minOutputSz;
  bool m_outputsOnTop;
  bool m_rotatable;
  bool m_hasLabel;
  bool m_hasFrequency;
  bool m_hasColors;
  bool m_hasTrigger;
  bool m_hasAudio;
  bool m_disabled;
  QString m_labelText;
  QKeySequence m_trigger;


  void loadPos( QDataStream &ds );

  void loadAngle( QDataStream &ds );

  void loadLabel( QDataStream &ds, double version );

  void loadMinMax( QDataStream &ds, double version );

  void loadTrigger( QDataStream &ds, double version );

  void loadInputPorts( QDataStream &ds, QMap< quint64, QNEPort* > &portMap );

  void removePortFromMap( QNEPort *deletedPort, QMap< quint64, QNEPort* > &portMap );

  void loadOutputPorts( QDataStream &ds, QMap< quint64, QNEPort* > &portMap );

  void removeSurplusInputs( quint64 inputSz, QMap< quint64, QNEPort* > &portMap );

  void loadInputPort( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, size_t port );

  void loadOutputPort( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, size_t port );

protected:
  QVector< QNEInputPort* > m_inputs;
  QVector< QNEOutputPort* > m_outputs;
};


#endif /* GRAPHICELEMENT_H */
