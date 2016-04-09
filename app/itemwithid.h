#ifndef ITEMWITHID_H
#define ITEMWITHID_H


class ItemWithId {
  int m_id;

public:
  ItemWithId( ) {
    m_id = 0;
  }

  int id( ) const {
    return( m_id );
  }
  void setId( int id ) {
    m_id = id;
  }
  virtual ~ItemWithId( ) {
  }
};


#endif /* ITEMWITHID_H */
