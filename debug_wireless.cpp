#include <QApplication>
#include <QDebug>
#include "app/scene.h"
#include "app/element/node.h"
#include "app/elementfactory.h"
#include "app/nodes/wirelessconnection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Creating scene...";
    auto *scene = new Scene();
    
    qDebug() << "Creating nodes...";
    auto *senderNode = qgraphicsitem_cast<Node*>(ElementFactory::buildElement(ElementType::Node));
    auto *receiverNode = qgraphicsitem_cast<Node*>(ElementFactory::buildElement(ElementType::Node));
    
    scene->addItem(senderNode);
    scene->addItem(receiverNode);
    
    qDebug() << "Setting wireless labels...";
    senderNode->setLabel("test_signal");
    receiverNode->setLabel("test_signal");
    
    qDebug() << "Checking scene items...";
    auto items = scene->items();
    int wirelessCount = 0;
    for (auto *item : items) {
        if (item->type() == WirelessConnection::Type) {
            qDebug() << "Found WirelessConnection:" << item;
            wirelessCount++;
        }
    }
    
    qDebug() << "Total wireless connections found:" << wirelessCount;
    
    delete scene;
    return 0;
}