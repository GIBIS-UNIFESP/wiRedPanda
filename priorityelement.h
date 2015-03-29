#ifndef PRIORITYELEMENT_H
#define PRIORITYELEMENT_H


class PriorityElement {
public:
  PriorityElement(int priority = -1);
  ~PriorityElement();
  int priority() const;
  void setPriority(int priority);

private:
  int m_priority;
};

#endif // PRIORITYELEMENT_H
