#ifndef LOGICELEMENT_H
#define LOGICELEMENT_H

#include <QSet>
#include <vector>

#include <functional>

/**
 * @brief The LogicElement class was designed to represent logic
 *        elements in the simulation layer of wiredpanda.
 */
class LogicElement {
  /**
   * @brief m_isValid is calculated at compilation time.
   */
  bool m_isValid;
  bool beingVisited;
  int priority;
  std::vector< std::pair< LogicElement*, int > > m_inputs;
  std::vector< bool > m_inputvalues;
  std::vector< bool > m_outputs;
  QSet< LogicElement* > m_sucessors;


protected:
  virtual void _updateLogic( const std::vector< bool > &inputs ) = 0;
public:
  explicit LogicElement( size_t inputSize, size_t outputSize );

  virtual ~LogicElement( );

  void updateLogic( );

  void connectInput( int index, LogicElement *elm, int port );

  void setOutputValue( size_t index, bool value );
  void setOutputValue( bool value );
  bool getOutputValue( size_t index = 0 ) const;

  void validate( );

  bool operator<( const LogicElement &other );

  int calculatePriority( );

  bool isValid( ) const;
};

class LogicNode : public LogicElement {
public:

  explicit LogicNode( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > & );
};

class LogicInput : public LogicElement {
public:

  explicit LogicInput( bool defaultValue = false );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > & );
};

class LogicOutput : public LogicElement {
public:
  explicit LogicOutput( size_t inputSz );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicAnd : public LogicElement {
public:
  explicit LogicAnd( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicOr : public LogicElement {
public:
  explicit LogicOr( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicNand : public LogicElement {
public:
  explicit LogicNand( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicNor : public LogicElement {
public:
  explicit LogicNor( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicXor : public LogicElement {
public:
  explicit LogicXor( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicXnor : public LogicElement {
public:
  explicit LogicXnor( size_t inputSize );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};


class LogicNot : public LogicElement {
public:
  explicit LogicNot( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicJKFlipFlop : public LogicElement {
public:
  explicit LogicJKFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
};

class LogicSRFlipFlop : public LogicElement {
public:
  explicit LogicSRFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
};

class LogicTFlipFlop : public LogicElement {
public:
  explicit LogicTFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
};

class LogicDFlipFlop : public LogicElement {
public:
  explicit LogicDFlipFlop( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
  bool lastValue;
};

class LogicDLatch : public LogicElement {
public:
  explicit LogicDLatch( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

class LogicMux : public LogicElement {
public:
  explicit LogicMux( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};


class LogicDemux : public LogicElement {
public:
  explicit LogicDemux( );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );
};

#endif /* LOGICELEMENT_H */
