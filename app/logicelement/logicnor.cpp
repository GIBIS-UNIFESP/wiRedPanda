#include "logicnor.h"

LogicNor::LogicNor(size_t inputSize)
    : LogicElement(inputSize, 1)
{
}

void LogicNor::_updateLogic(const std::vector<bool> &inputs)
{
    bool result = false;
    for (bool in : inputs) {
        result |= in;
    }
    setOutputValue(!result);
}