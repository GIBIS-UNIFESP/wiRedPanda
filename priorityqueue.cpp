#include "priorityqueue.h"

PriorityQueue::PriorityQueue(QVector<GraphicElement *> elements) {
  foreach (GraphicElement * elm, elements) {
    push(elm);
  }
}

PriorityQueue::~PriorityQueue() {

}

void PriorityQueue::push(GraphicElement * elm) {
  list.append(elm);
  int i = list.count() -1;
  int parent = (i-1)/2;
  while (parent >= 0 && list[i]->priority() < list[parent]->priority()) {
    std::swap(list[parent],list[i]);
    i = parent;
    parent = (i-1)/2;
  }
}

GraphicElement * PriorityQueue::pop() {
  if(list.isEmpty()) {
    return nullptr;
  }
  GraphicElement * elm = list[0];
  int i = 0;
  list[0] = list.back();
  list.removeLast();
  while (i < size()) {
    int left = 2*i + 1;
    int right = left + 1;
    if(right > size() - 1) {
      break;
    }
    int min = left;
    if(list[right] < list[left]) {
      min = right;
    }
    if(list[i] > list[min]) {
      std::swap(list[min],list[i]);
      i = min;
    } else {
      break;
    }
  }
  return elm;
}

int PriorityQueue::size() {
  return list.size();
}

bool PriorityQueue::isEmpty() {
  return list.isEmpty();
}

