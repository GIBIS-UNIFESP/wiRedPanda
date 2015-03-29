#include "priorityqueue.h"
#include "priorityelement.h"
#include "graphicelement.h"
#include <QDebug>
#include <iostream>


void PriorityQueue::build() {
  int l = (heap.size()/2)-1;
  while (l >= 0) {
    maxHeapify(l,heap.size()-1);
    l--;
  }
}

void PriorityQueue::clear() {
  heap.clear();
}

QString PriorityQueue::toString() {
  QString str = "( ";
  if(!heap.isEmpty()) {
    str += QString::number(heap[0]->priority());
    for(int i = 1; i< heap.size(); i++) {
      str += ", " + QString::number(heap[i]->priority());
    }
  }
  str += " )";
  return str;
}

PriorityQueue::PriorityQueue(QVector<PriorityElement *> elements) {
  heap = elements.toList();
  build();
}

PriorityQueue::PriorityQueue(QVector<GraphicElement *> elements) {
  foreach (GraphicElement * elm, elements) {
    PriorityElement * p_elm = dynamic_cast<PriorityElement *>(elm);
    if(p_elm)
      heap.append(p_elm);
  }
  build();
}

PriorityQueue::~PriorityQueue() {

}

int PriorityQueue::parent(int i) {
  return( i/2 );
}

int PriorityQueue::left(int i) {
  return( 2*i + 1 );
}

int PriorityQueue::right(int i) {
  return( 2*i + 2 );
}

void PriorityQueue::maxHeapify(int l, int r) {
  int i = l;
  int j = left(i);
  PriorityElement * aux = heap[i];
  while (j <= r) {
    if((j < r) && (heap[j]->priority() < heap[j+1]->priority())) {
      j++;
    }
    if(aux->priority() >= heap[j]->priority()) {
      break;
    }
    heap[i] = heap[j];
    i = j;
    j = left(i);
  }
  heap[i] = aux;
}

void PriorityQueue::print() {
  qDebug() << "Priority queue: " << toString();
}

PriorityElement * PriorityQueue::pop() {
  if(heap.isEmpty()) {
    return nullptr;
  }
  PriorityElement * elm  = heap.front();
  heap.front() = heap.back();
  heap.pop_back();
  build();
  return (elm);
}

int PriorityQueue::size() {
  return heap.size();
}

bool PriorityQueue::isEmpty() {
  return heap.isEmpty();
}

bool PriorityQueue::isValid() {
  for(int i = 0; i < (size()/2); i++) {
    if(heap[i]->priority() < heap[left(i)]->priority()) {
      return false;
    } else if( (right(i) < size()) && (heap[i]->priority() < heap[right(i)]->priority())) {
      return false;
    }
  }
  return true;
}
