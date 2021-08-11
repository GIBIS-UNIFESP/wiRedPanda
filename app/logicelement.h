/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICELEMENT_H
#define LOGICELEMENT_H

#include <vector>

#include <QSet>

/**
 * @brief The LogicElement class was designed to represent logic
 *        elements in the simulation layer of wiredpanda.
 */
class LogicElement
{
private:
    /**
     * @brief m_isValid is calculated at compilation time.
     */
    bool m_isValid;
    bool m_beingVisited;
    int m_priority;
    std::vector<std::pair<LogicElement *, int>> m_inputs;
    std::vector<bool> m_inputvalues;
    std::vector<bool> m_outputs;
    QSet<LogicElement *> m_successors;

protected:
    // Main function to update the logic of an element. Computes the outputs, given the inputs
    virtual void _updateLogic(const std::vector<bool> &inputs) = 0;

public:
    explicit LogicElement(size_t inputSize, size_t outputSize);

    virtual ~LogicElement();

    void connectPredecessor(int index, LogicElement *elm, int port);

    void setOutputValue(size_t index, bool value);
    void setOutputValue(bool value);
    bool getOutputValue(size_t index = 0) const;
    bool getInputValue(size_t index = 0) const;
    size_t getInputAmount() { return m_inputs.size(); }
    size_t getOutputAmount() { return m_outputs.size(); }

    void validate();

    bool operator<(const LogicElement &other) const;

    int calculatePriority();

    bool isValid() const;

    void clearPredecessors();

    void clearSucessors();

    // Secure call to _updateLogic() with current inputs.
    void updateLogic();
};

#endif /* LOGICELEMENT_H */
