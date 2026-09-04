// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Quick-native, Scene-free replacement for Tests/Common/TestUtils.h's CircuitBuilder.
 */

#pragma once

#include <memory>
#include <vector>

#include <QString>
#include <QVector>

#include "App/Core/SimulationHost.h"
#include "App/Scene/SceneItemRegistry.h"

class Connection;
class GraphicElement;
class Port;
class Simulation;

/**
 * \class QuickCircuitBuilder
 * \brief Registers a headless circuit (elements + a bound Simulation) for tests, with no Scene/
 * QGraphicsScene behind it -- GraphicElement/Connection are plain, non-QGraphicsItem classes,
 * so nothing but a flat element list and a SimulationHost implementation is actually needed to
 * drive a Simulation.
 *
 * addElement()/add() do NOT take ownership, matching the original CircuitBuilder(Scene*)'s own
 * addElement(GraphicElement*) signature -- ownership there was really QGraphicsScene::addItem()'s
 * business, a separate concern CircuitBuilder happened to delegate to, not something the API
 * itself promised. Real call sites pass stack-allocated elements just as often as heap ones (e.g.
 * Tests/Integration/TestSimulation.cpp's "InputSwitch sw1, sw2; ... builder.add(&sw1, &sw2)"
 * pattern), so an owning addElement() would double-delete/crash on those. Use addOwnedElement()
 * for the other real pattern -- a helper function that heap-allocates a whole circuit via
 * ElementFactory::buildElement() and returns it as one object -- there the builder is the only
 * thing that will ever reference those elements again, so it must own them.
 *
 * Connections created via connect() are intentionally NOT tracked/owned here regardless: a
 * Connection attached to two owned elements' ports is transitively deleted when either owning
 * element is destroyed (~ElementPorts() deletes its ports, whose own destructors drain and
 * delete attached connections symmetrically), so tracking it a second time here would risk a
 * double free. For connections between two caller-owned (non-owned) elements, the caller is
 * likewise responsible for the Connection's lifetime being covered by at least one of its two
 * ports' owning elements.
 *
 * Every element added (owned or not) and every Connection created via connect() gets a fresh,
 * builder-local id via SceneItemRegistry -- the exact id-assignment half of the real
 * Scene::addItem()/a future CanvasItem::addItem()'s pattern (unassigned items get nextId(); pre-
 * assigned ones just advance the counter). A fresh QuickCircuitBuilder's id sequence always
 * restarts at 0, matching a fresh Scene's own fresh SceneItemRegistry -- load-bearing for tests
 * that compare id-derived orderings (e.g. topological sort output) across repeated builds of an
 * identical circuit.
 *
 * Usage:
 *   QuickCircuitBuilder builder;
 *   InputSwitch sw1; And andGate; Led led;
 *   builder.add(&sw1, &andGate, &led);
 *   builder.connect(&sw1, 0, &andGate, 0);
 *   builder.connect(&andGate, 0, &led, 0);
 *   builder.initSimulation()->update();
 */
class QuickCircuitBuilder : public SimulationHost
{
public:
    QuickCircuitBuilder();
    ~QuickCircuitBuilder() override;

    QuickCircuitBuilder(const QuickCircuitBuilder &) = delete;
    QuickCircuitBuilder &operator=(const QuickCircuitBuilder &) = delete;

    /// Registers \a elm (NOT owned -- caller keeps responsibility for its lifetime); returns it
    /// back for convenient chaining at the call site.
    GraphicElement *addElement(GraphicElement *elm);

    /// Add multiple elements (variadic, mirrors CircuitBuilder::add()). Not owned, see addElement().
    template<typename... Args>
    void add(Args *...elements)
    {
        (addElement(elements), ...);
    }

    /// Registers \a elm AND takes ownership of it (deleted in this builder's destructor). For
    /// heap-allocated elements (e.g. ElementFactory::buildElement() results) that only this
    /// builder will reference again -- see the class doc comment.
    GraphicElement *addOwnedElement(GraphicElement *elm);

    /// Add multiple heap-allocated elements at once, each taking ownership (mirrors add()'s own
    /// variadic convenience for the owned case -- see addOwnedElement()).
    template<typename... Args>
    void addOwned(Args *...elements)
    {
        (addOwnedElement(elements), ...);
    }

    /// Untracks \a elm (mirrors Scene::removeItem()'s contract exactly): does NOT delete it --
    /// the caller deletes it separately, same two-step as the real Scene::removeItem()+delete
    /// pattern tests already use. If \a elm was added via addOwnedElement(), ownership is
    /// released here too, so the builder's destructor won't later double-delete it.
    void removeElement(GraphicElement *elm);

    /// Every element added so far (owned or not), in addition order.
    QVector<GraphicElement *> elements() const;

    /// Returns the item (element or connection) registered under \a id, or nullptr.
    /// Mirrors Scene::itemById() -- forwards to this builder's own SceneItemRegistry.
    [[nodiscard]] ItemWithId *itemById(int id) const;

    // --- Connections (same 4 overloads as CircuitBuilder) ---

    Connection *connect(GraphicElement *from, int fromPort, GraphicElement *to, int toPort);
    Connection *connect(GraphicElement *from, const QString &fromLabel, GraphicElement *to, int toPort);
    Connection *connect(GraphicElement *from, int fromPort, GraphicElement *to, const QString &toLabel);
    Connection *connect(GraphicElement *from, const QString &fromLabel, GraphicElement *to, const QString &toLabel);

    /// Lazily constructs (on first call) the Simulation bound to this builder.
    Simulation *simulation();

    /// simulation(), then initialize()s it before returning.
    Simulation *initSimulation();

    // --- SimulationHost ---

    QList<ItemWithId *> simulationItems() const override;
    void setMuted(bool muted) override;

private:
    struct InputPortTraits;
    struct OutputPortTraits;

    template<typename PortTraits>
    int portByLabelImpl(GraphicElement *element, const QString &label);
    template<typename PortTraits>
    QString availablePortsImpl(GraphicElement *element) const;

    int inputPortByLabel(GraphicElement *element, const QString &label);
    int outputPortByLabel(GraphicElement *element, const QString &label);

    /// Assigns a fresh id to \a item (or advances the counter past its pre-assigned one),
    /// mirroring Scene::addItem() exactly.
    void registerId(ItemWithId *item);

    QVector<GraphicElement *> m_elements;
    std::vector<std::unique_ptr<GraphicElement>> m_owned;
    std::unique_ptr<Simulation> m_simulation;
    SceneItemRegistry m_itemRegistry;
};
