// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/DragPreview.h"

#include "App/Element/GraphicElement.h"

DragPreview::Scope::Scope(const Picture &picture)
{
    storage() = picture;
}

DragPreview::Scope::~Scope()
{
    storage() = {};
}

const DragPreview::Picture &DragPreview::current()
{
    return storage();
}

QPointF DragPreview::position(const Picture &picture, const QPointF scenePos)
{
    return GraphicElement::snapToGrid(scenePos - picture.grab) + picture.topLeft;
}

DragPreview::Picture &DragPreview::storage()
{
    static Picture picture;
    return picture;
}
