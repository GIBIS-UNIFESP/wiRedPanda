#include <QApplication>
#include <QDebug>
#include "app/scene.h"
#include "app/element/node.h"
#include "app/elementfactory.h"
#include "app/nodes/qneconnection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Simple Connection Test ===";
    
    auto *scene = new Scene();
    
    // Create two nodes
    auto *node1 = qgraphicsitem_cast<Node*>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node*>(ElementFactory::buildElement(ElementType::Node));
    
    scene->addItem(node1);
    scene->addItem(node2);
    
    qDebug() << "Before connection:";
    qDebug() << "  Node1 input connections:" << node1->inputPort()->connections().size();
    qDebug() << "  Node2 input connections:" << node2->inputPort()->connections().size();
    
    // Create connection from node1 output to node2 input
    auto *conn = new QNEConnection();
    scene->addItem(conn);
    
    conn->setStartPort(node1->outputPort());
    conn->setEndPort(node2->inputPort());
    conn->updatePath();
    
    qDebug() << "After connection:";
    qDebug() << "  Node1 input connections:" << node1->inputPort()->connections().size();
    qDebug() << "  Node2 input connections:" << node2->inputPort()->connections().size();
    qDebug() << "  Node2 hasPhysicalInputConnection:" << node2->hasPhysicalInputConnection();
    
    delete scene;
    return 0;
}