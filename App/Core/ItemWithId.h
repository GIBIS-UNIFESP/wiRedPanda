// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Defines ItemWithId, the base class for all uniquely identified circuit items.
 */

#pragma once

#include <QtGlobal>

/**
 * \class ItemWithId
 * \brief Base class providing a unique integer identifier for circuit items.
 *
 * \details Every graphic element, connection, and port inherits from ItemWithId
 * to obtain a stable numeric ID managed by ElementFactory. The ID is used for
 * undo/redo serialization and cross-reference during load/save.
 */
class ItemWithId
{
public:
    /// Constructs a new ItemWithId and registers it with ElementFactory.
    ItemWithId();

    /// Destructor; unregisters the item from ElementFactory.
    virtual ~ItemWithId();

    /// Returns the unique integer identifier of this item.
    int id() const;

    /**
     * \brief Sets the identifier to \a id and updates the factory registry.
     * \param id New identifier value.
     */
    void setId(const int id);

private:
    Q_DISABLE_COPY(ItemWithId)

    int m_id = 0; ///< The unique ID assigned by ElementFactory.
};

