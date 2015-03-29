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
    heap.append(dynamic_cast<PriorityElement *>(elm));
  }
  build();
}

PriorityQueue::~PriorityQueue() {

}

int PriorityQueue::parent(int i) {
  return( i/2 );
}

int PriorityQueue::left(int i) {
  return( 2*i );
}

int PriorityQueue::right(int i) {
  return( 2*i+1 );
}

void PriorityQueue::maxHeapify(int l, int r) {
  int i = l;
  int j = right(i);
  PriorityElement * aux = heap[l];
  while (j <= r) {
    if((j < r) && (heap[j]->priority() < heap[j+1]->priority())) {
      j++;
    }
    if(aux->priority() >= heap[j]->priority()) {
      break;
    }
    heap[i] = heap[j];
    i = j;
    j = right(i);
  }
  heap[i] = aux;
}

void PriorityQueue::print() {
  qDebug() << "Priority queue: " << toString();
}

PriorityElement * PriorityQueue::pop() {
  if(heap.isEmpty()){
    return nullptr;
  }
  PriorityElement * elm  = heap.front();
  heap.first() = heap.back();
  heap.pop_back();
  for( int i = 0; i < size();) {
    int l = left(i);
    int r = right(i);
    if(r >= size()){
      break;
    }
    int max = r;
    if(heap[l] >= heap[r]){
      max = l;
    }
    if(heap[i] <= heap[max]) {
      std::swap(heap[i], heap[max]);
      i = max;
    }else{
      break;
    }
  }
  return (elm);
}

int PriorityQueue::size() {
  return heap.size();
}

bool PriorityQueue::isEmpty() {
  return heap.isEmpty();
}

