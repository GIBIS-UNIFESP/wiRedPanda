#include "priorityqueue.h"
#include "priorityelement.h"
#include "graphicelement.h"
#include <QDebug>
#include <iostream>


void PriorityQueue::build() {
  std::cout << "Before: ";
  print();
  int l = (list.size()/2)-1;
  while (l >= 0) {
    maxHeapify(l,list.size()-1);
    l--;
  }
  std::cout << "After: ";
  print();
}

PriorityQueue::PriorityQueue(QVector<PriorityElement *> elements) {
  list = elements.toList();
  build();
}

PriorityQueue::PriorityQueue(QVector<GraphicElement *> elements) {
  foreach (GraphicElement * elm, elements) {
    list.append(dynamic_cast<PriorityElement *>(elm));
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
  PriorityElement * aux = list[l];
  while (j <= r) {
    if((j < r) && (list[j]->priority() < list[j+1]->priority())) {
      j++;
    }
    if(aux->priority() >= list[j]->priority()) {
      break;
    }
    list[i] = list[j];
    i = j;
    j = right(i);
  }
  list[i] = aux;
}

void PriorityQueue::print() {
  std::cout << "Priority queue : ( ";
  if(!list.isEmpty()) {
    std::cout << list[0]->priority();
    for(int i = 1; i< list.size(); i++) {
      std::cout << ", " << list[i]->priority();
    }
  }
  std::cout << " )" << std::endl;
}

PriorityElement * PriorityQueue::pop() {

}

int PriorityQueue::size() {
  return list.size();
}

bool PriorityQueue::isEmpty() {
  return list.isEmpty();
}

