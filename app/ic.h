#ifndef IC_H
#define IC_H

#include "elementfactory.h"
#include "graphicelement.h"
#include "icprototype.h"
#include "scene.h"
#include "simulationcontroller.h"

#include <QFileInfo>
#include <QFileSystemWatcher>

class Editor;

class ICPrototype;

class IC : public GraphicElement
{
    Q_OBJECT

    friend class CodeGenerator;

public:
    IC(QGraphicsItem *parent = nullptr);
    virtual ~IC() override;

    /* GraphicElement interface */
    virtual ElementType elementType() override
    {
        return (ElementType::IC);
    }
    virtual ElementGroup elementGroup() override
    {
        return (ElementGroup::IC);
    }
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    void loadFile(QString fname);
    QString getFile() const;
    bool setFile(QString newFileName);
    ICPrototype *getPrototype();
    QVector<GraphicElement *> getElements() const;
    void setSkin(bool defaultSkin, QString filename) override;

private:
    Editor *editor;
    QString m_file;

    void loadInputs(ICPrototype *prototype);

    void loadOutputs(ICPrototype *prototype);

    /* QGraphicsItem interface */
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif /* IC_H */
