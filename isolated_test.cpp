#include "testqneconnection.h"
#include "application.h"
#include "registertypes.h"
#include "common.h"

#include <QtTest>

int main(int argc, char **argv) {
    registerTypes();
    Comment::setVerbosity(-1);
    Application app(argc, argv);
    
    TestQNEConnection test;
    return QTest::qExec(&test, argc, argv);
}
EOF < /dev/null