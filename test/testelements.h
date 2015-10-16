#ifndef TESTELEMENTS_H
#define TESTELEMENTS_H

#include <QTest>

class TestElements : public QObject {
  Q_OBJECT
public:
  explicit TestElements( QObject *parent = 0 );

signals:

private slots:

  void testAnd( );
};

#endif /* TESTELEMENTS_H */
