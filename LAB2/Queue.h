#pragma once

template <typename T, uint16_t MAX_SIZE>
class Queue {
  
  private: T data[MAX_SIZE];
  private: uint16_t head;
  private: uint16_t tail;
  private: bool full;

  public: void Init() {
    head = 0;
    tail = 0;
    full = false;
  }
  public: bool Push(T item) {
    if(full) {
      return false;
    }

    data[head] = item;

    head = (head + 1) % MAX_SIZE;
    if(head == tail) {
      full = true;
    }

    return true;
  }
  public: bool Pop(T& item) {
    if(tail == head && !full) {
      return false;
    }

    item = data[tail];

    full = false;
    tail = (tail + 1) % MAX_SIZE;

    return true;
  }
};