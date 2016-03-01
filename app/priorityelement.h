#ifndef PRIORITYELEMENT_H
#define PRIORITYELEMENT_H


class PriorityElement {
public:
  explicit PriorityElement( int priority = -1 );
  virtual ~PriorityElement( );
  int priority( ) const;
  void setPriority( int priority );
  static bool lessThan( const PriorityElement *e1, const PriorityElement *e2 ) {
    return( e1->priority( ) > e2->priority( ) );
  }

private:
  int m_priority;
};

#endif /* PRIORITYELEMENT_H */
