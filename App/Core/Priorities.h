// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graph algorithms for topological priority assignment and cycle detection.
 */

#pragma once

#include <algorithm>

#include <QHash>
#include <QSet>
#include <QStack>
#include <QVector>

/**
 * \brief Finds the strongly connected components that are cycles, as a PARTITION.
 *
 * Uses Tarjan's iterative SCC algorithm.  Every component of size > 1, and every
 * single node with a self-loop, is returned as its own group.  Acyclic nodes are
 * not returned at all.
 *
 * findFeedbackNodes() is the flattened view of this and is implemented in terms of
 * it.  The grouping matters wherever "which cycle" is the question rather than
 * "is this in a cycle": canonicalising one oscillating region must not disturb an
 * unrelated, settled one.
 *
 * \param elements    All nodes to process.
 * \param successors  Adjacency list (node -> its successors).
 * \return One inner vector per cyclic component.
 */
template<typename T>
QVector<QVector<T *>> findFeedbackComponents(
    const QVector<T *> &elements,
    const QHash<T *, QVector<T *>> &successors)
{
    QVector<QVector<T *>> components;

    int indexCounter = 0;
    QHash<T *, int> nodeIndex;
    QHash<T *, int> lowlink;
    QStack<T *> sccStack;
    QSet<T *> onStack;

    struct Frame {
        T *node;
        int successorIdx;
    };

    for (auto *element : elements) {
        if (nodeIndex.contains(element)) {
            continue;
        }

        QStack<Frame> callStack;
        nodeIndex[element] = indexCounter;
        lowlink[element] = indexCounter;
        ++indexCounter;
        sccStack.push(element);
        onStack.insert(element);
        callStack.push({element, 0});

        while (!callStack.isEmpty()) {
            auto &frame = callStack.top();
            T *node = frame.node;

            const auto it = successors.constFind(node);
            const int succCount = (it != successors.constEnd()) ? static_cast<int>(it->size()) : 0;

            if (frame.successorIdx < succCount) {
                T *succ = (*it)[frame.successorIdx];
                ++frame.successorIdx;

                if (!nodeIndex.contains(succ)) {
                    nodeIndex[succ] = indexCounter;
                    lowlink[succ] = indexCounter;
                    ++indexCounter;
                    sccStack.push(succ);
                    onStack.insert(succ);
                    callStack.push({succ, 0});
                } else if (onStack.contains(succ)) {
                    lowlink[node] = (std::min)(lowlink[node], nodeIndex[succ]);
                }
            } else {
                // All successors processed — check if node is root of an SCC.
                if (lowlink[node] == nodeIndex[node]) {
                    QVector<T *> scc;
                    T *w;
                    do {
                        w = sccStack.pop();
                        onStack.remove(w);
                        scc.append(w);
                    } while (w != node);

                    if (scc.size() > 1) {
                        components.append(scc);
                    } else if ((it != successors.constEnd()) && it->contains(node)) {
                        components.append(QVector<T *>{node}); // self-loop
                    }
                }

                callStack.pop();

                if (!callStack.isEmpty()) {
                    T *parent = callStack.top().node;
                    lowlink[parent] = (std::min)(lowlink[parent], lowlink[node]);
                }
            }
        }
    }

    return components;
}

/**
 * \brief Flattens a cyclic-component partition into one membership set.
 *
 * \param components  Partition as returned by findFeedbackComponents().
 * \return Set of all nodes that are part of at least one cycle.
 */
template<typename T>
QSet<T *> flattenFeedbackComponents(const QVector<QVector<T *>> &components)
{
    QSet<T *> feedbackNodes;
    for (const auto &component : components) {
        for (auto *node : component) {
            feedbackNodes.insert(node);
        }
    }
    return feedbackNodes;
}

/**
 * \brief Finds all nodes that participate in feedback loops (cycles).
 *
 * The flattened view of findFeedbackComponents().  Use this when only membership
 * matters AND the partition is not needed: it runs Tarjan's algorithm.  A caller
 * that already holds the partition should flatten it directly instead.
 *
 * \param elements    All nodes to process.
 * \param successors  Adjacency list (node -> its successors).
 * \return Set of all nodes that are part of at least one cycle.
 */
template<typename T>
QSet<T *> findFeedbackNodes(
    const QVector<T *> &elements,
    const QHash<T *, QVector<T *>> &successors)
{
    return flattenFeedbackComponents(findFeedbackComponents(elements, successors));
}

namespace PrioritiesInternal {

/**
 * \brief Legacy iterative DFS priority calculation, used for cyclic graphs only.
 *
 * Treats any successor that is pending on the explicit stack as a feedback
 * edge and assigns the current node early, from already-computed successors
 * only.  On cycle-free graphs this mis-classifies pending *siblings* as
 * feedback and produces iteration-order-dependent priorities (see
 * calculatePriorities below for the correct DAG path).  On cyclic graphs the
 * resulting order is what the simulation's iterative settling and the
 * gate-built latch circuits (SR/JK/D master-slave ICs) were built around:
 * zero-delay latch races have no graph-derivable "correct" evaluation order,
 * and changing the order flips which stable state a latch settles into from
 * an undetermined start.  Keep this path byte-for-byte stable unless the
 * settling semantics are redesigned.
 */
template<typename T>
void legacyCalculatePriorities(
    const QVector<T *> &elements,
    const QHash<T *, QVector<T *>> &successors,
    QHash<T *, int> &outPriorities)
{
    QStack<T *> stack;
    QSet<T *> inStack;

    for (auto *element : elements) {
        if (outPriorities.contains(element)) {
            continue;
        }

        stack.push(element);
        inStack.insert(element);

        while (!stack.isEmpty()) {
            auto *current = stack.top();

            // Unreachable: a node only leaves `inStack` together with either
            // getting a priority assigned (below, immediately followed by a pop
            // of that same stack entry) or via this very check — and a node is
            // only ever pushed while absent from both `outPriorities` and
            // `inStack`, so `stack.top()` can never already hold a priority.
            // The inner while loop always drains the stack fully before the
            // outer `for` advances to the next top-level element, so no node
            // can be re-discovered here after being resolved elsewhere either.
            if (outPriorities.contains(current)) { // LCOV_EXCL_LINE
                stack.pop(); // LCOV_EXCL_LINE
                inStack.remove(current); // LCOV_EXCL_LINE
                continue; // LCOV_EXCL_LINE
            }

            bool allProcessed = true;
            int maxSuccessorPriority = 0;
            bool hasFeedbackLoop = false;

            const auto it = successors.find(current);
            if (it != successors.end()) {
                for (auto *successor : *it) {
                    if (!outPriorities.contains(successor)) {
                        if (!inStack.contains(successor)) {
                            stack.push(successor);
                            inStack.insert(successor);
                            allProcessed = false;
                        } else {
                            hasFeedbackLoop = true;
                        }
                    } else {
                        maxSuccessorPriority = (std::max)(maxSuccessorPriority, outPriorities.value(successor));
                    }
                }
            }

            // Assign priority when all successors are computed, OR when a
            // feedback loop is detected.  The early assignment on feedback is
            // intentional: it gives feedback-loop nodes a *lower* priority
            // (based only on already-computed successors) so the simulation
            // processes them *after* their non-cyclic inputs.  Because
            // GraphicElement::updateLogic() reads live predecessor outputs,
            // processing feedback nodes late ensures they see fresh values
            // from the current iteration rather than stale ones.
            if (allProcessed || hasFeedbackLoop) {
                outPriorities[current] = maxSuccessorPriority + 1;
                stack.pop();
                inStack.remove(current);
            }
        }
    }
}

} // namespace PrioritiesInternal

/**
 * \brief Priority calculation for directed graphs.
 *
 * Assigns each node a priority equal to its longest path to a sink plus one;
 * higher priority evaluates first (sources before sinks), so every element
 * sees up-to-date predecessor outputs within a single simulation pass.
 *
 * Cycle-free graphs use an iterative DFS with proper post-order assignment:
 * a node's priority is computed only after *all* of its successors have
 * theirs, which makes priorities deterministic and independent of the
 * iteration order of \p elements.  (The previous implementation mistook
 * pending DFS siblings for feedback edges and could order a consumer at or
 * above its producer on plain DAGs — and a pure DAG runs the non-settling
 * fast path, so nothing masked the mis-order; an edge-triggered flip-flop
 * could latch a stale value permanently.)
 *
 * Cyclic graphs keep the legacy ordering (see
 * PrioritiesInternal::legacyCalculatePriorities): their single-pass order is
 * not load-bearing for correctness because the simulation switches to
 * iterative settling over *all* elements whenever feedback is present, and
 * the legacy order is what the gate-built latch circuits settle correctly
 * under.
 *
 * \param elements            All nodes to process.
 * \param successors          Adjacency list (node -> its successors).
 * \param outPriorities       Output map filled with computed priorities.
 * \param feedbackComponents  The cyclic-component partition of the same graph, as returned by
 *                            findFeedbackComponents().  Passed in rather than recomputed so a
 *                            caller that already needs the partition runs Tarjan once instead
 *                            of once per analysis; only its emptiness is read here.
 */
template<typename T>
void calculatePriorities(
    const QVector<T *> &elements,
    const QHash<T *, QVector<T *>> &successors,
    QHash<T *, int> &outPriorities,
    const QVector<QVector<T *>> &feedbackComponents)
{
    if (!feedbackComponents.isEmpty()) {
        PrioritiesInternal::legacyCalculatePriorities(elements, successors, outPriorities);
        return;
    }

    // Cycle-free: two-phase iterative DFS (expand, then assign in post-order).
    // On first visit a node pushes its unfinished successors and stays on the
    // stack; when it surfaces again every successor pushed above it has been
    // assigned (guaranteed acyclic), so its own priority is final.  A node
    // reached from several parents may sit on the stack more than once — the
    // duplicate pops in O(1) via the priority check — keeping the whole pass
    // linear in nodes + edges.
    QSet<T *> expanded;

    for (auto *element : elements) {
        if (outPriorities.contains(element)) {
            continue;
        }

        QStack<T *> stack;
        stack.push(element);

        while (!stack.isEmpty()) {
            auto *current = stack.top();

            if (outPriorities.contains(current)) {
                stack.pop();
                continue;
            }

            const auto it = successors.constFind(current);

            if (!expanded.contains(current)) {
                expanded.insert(current);
                if (it != successors.constEnd()) {
                    for (auto *successor : *it) {
                        if (!outPriorities.contains(successor)) {
                            stack.push(successor);
                        }
                    }
                }
                continue;
            }

            int maxSuccessorPriority = 0;
            if (it != successors.constEnd()) {
                for (auto *successor : *it) {
                    maxSuccessorPriority = (std::max)(maxSuccessorPriority, outPriorities.value(successor));
                }
            }

            outPriorities[current] = maxSuccessorPriority + 1;
            stack.pop();
        }
    }
}

/**
 * \brief Convenience form for callers that do not otherwise need the cyclic-component
 * partition: runs findFeedbackComponents() and forwards to the overload above.
 */
template<typename T>
void calculatePriorities(
    const QVector<T *> &elements,
    const QHash<T *, QVector<T *>> &successors,
    QHash<T *, int> &outPriorities)
{
    calculatePriorities(elements, successors, outPriorities,
                        findFeedbackComponents(elements, successors));
}
