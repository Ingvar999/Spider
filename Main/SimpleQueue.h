#pragma once

template<typename T>
class SimpleQueue
{
  public:
    bool isEmpty() {
      return count == 0;
    }

    bool isFull(){
      return count == maxSize;
    }

    void Clear(){
      count = rPos = wPos = 0;
    }
    
    const T& Pop() {
      int itemPos = rPos;
      inc(rPos);
      count--;
      return buffer[itemPos];
    }
    
    void Push(const T& item) {
      buffer[wPos] = item;
      inc(wPos);
      count++;
    }

  private:
    static const int maxSize = 5;

    volatile int count = 0;
    int rPos, wPos;
    T buffer[maxSize];

    inc(int &x) {
      ++x;
      if (x == maxSize){
        x = 0;
      }
    }
};
