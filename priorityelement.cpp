#include "priorityelement.h"

PriorityElement::PriorityElement() {
}

PriorityElement::~PriorityElement() {

}

int PriorityElement::priority() const {
  return m_priority;
}

void PriorityElement::setPriority(int priority) {
  m_priority = priority;
}
