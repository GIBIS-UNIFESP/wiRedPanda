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
  explicit LogicElement( int inputSize, int outputSize );

  virtual ~LogicElement( );

  void connectPredecessor( int index, LogicElement *elm, int port );

  void setOutputValue( int index, bool value );
  void setOutputValue( bool value );
  bool getOutputValue( int index = 0 ) const;
  bool getInputValue( int index = 0 ) const;

  void validate( );

  bool operator<( const LogicElement &other );

  int calculatePriority( );

  bool isValid( ) const;

  void clearPredecessors( );

  void clearSucessors( );

  void updateLogic( );
};

#endif /* LOGICELEMENT_H */
