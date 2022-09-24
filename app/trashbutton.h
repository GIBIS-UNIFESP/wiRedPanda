#pragma once

#include <QPushButton>

class TrashButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TrashButton(QWidget *parent = nullptr);

signals:
    void removeICFile(const QString &icFileName);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};
