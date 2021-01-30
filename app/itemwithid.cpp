#include "itemwithid.h"
#include "elementfactory.h"

ItemWithId::ItemWithId()
{
    ElementFactory::addItem(this);
}

int ItemWithId::id() const
{
    return (m_id);
}

void ItemWithId::setId(int id)
{
    m_id = id;
}

ItemWithId::~ItemWithId()
{
    ElementFactory::removeItem(this);
}