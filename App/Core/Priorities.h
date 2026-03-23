// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graph algorithms for topological priority assignment and cycle detection.
 */

#pragma once

#include <QHash>
#include <QSet>
#include <QStack>
#include <QVector>

/**
 * \brief Iterative depth-first priority calculation for directed graphs.
 *
 * Assigns each node a priority equal to its longest path to a sink plus one.
 * Detects feedback loops internally to break cycles during traversal.
 *
 * \param elements       All nodes to process.
 * \param successors     Adjacency list (node -> its successors).
 * \param outPriorities  Output map filled with computed priorities.
 */
template<typename T>
void calculatePriorities(
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

            if (outPriorities.contains(current)) {
                stack.pop();
                inStack.remove(current);
                continue;
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
                        maxSuccessorPriority = qMax(maxSuccessorPriority, outPriorities.value(successor));
                    }
                }
            }

            // Assign priority when all successors are computed, OR when a
            // feedback loop is detected.  The early assignment on feedback is
            // intentional: it gives feedback-loop nodes a *lower* priority
            // (based only on already-computed successors) so the simulation
            // processes them *after* their non-cyclic inputs.  Because
            // LogicElement::updateLogic() reads live predecessor outputs,
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

/**
 * \brief Finds all nodes that participate in feedback loops (cycles).
 *
 * Uses Tarjan's iterative SCC algorithm.  Every node belonging to a
 * strongly connected component of size > 1 (or with a self-loop) is
 * returned in the output set.
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
    QSet<T *> feedbackNodes;

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
                    lowlink[node] = qMin(lowlink[node], nodeIndex[succ]);
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
                        for (auto *n : std::as_const(scc)) {
                            feedbackNodes.insert(n);
                        }
                    } else if ((it != successors.constEnd()) && it->contains(node)) {
                        feedbackNodes.insert(node); // self-loop
                    }
                }

                callStack.pop();

                if (!callStack.isEmpty()) {
                    T *parent = callStack.top().node;
                    lowlink[parent] = qMin(lowlink[parent], lowlink[node]);
                }
            }
        }
    }

    return feedbackNodes;
}

