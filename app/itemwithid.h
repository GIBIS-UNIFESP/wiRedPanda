#ifndef ITEMWITHID_H
#define ITEMWITHID_H


class ItemWithId {
  int m_id;

public:
  ItemWithId( );
  int id( ) const;
  void setId( int id );
  virtual ~ItemWithId( );
};


#endif /* ITEMWITHID_H */
