#pragma once

template<typename T>
class SimpleQueue
{
  public:
    bool isEmpty() {
      return count == 0;
    }

    void Clear(){
      count = rPos = wPos = 0;
    }
    
    T Pop() {
      T item = buffer[rPos];
      rPos = inc(rPos);
      count--;
      return item;
    }
    
    void Push(T item) {
      buffer[wPos] = item;
      wPos = inc(wPos);
      count++;
    }

  private:
    static const int maxSize = 5;

    int count = 0;
    int rPos, wPos;
    T buffer[maxSize];

    int inc(int x) {
      return (++x) % maxSize;
    }
};
