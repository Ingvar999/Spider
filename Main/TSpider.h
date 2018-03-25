#pragma once
#ifndef TSPIDER_H
#define TSPIDER_H

#include "TLeg.h"

class TSpider
{
  public:
    bool balancing = false;
    void Init(int, int, int, int, int, int = 0, int = 0, int = 10);
    void UpdateAllAngles();
    int ChangeHeight(int);
    int GetContacts();
    byte ReadContacts();
    int toContacts();
    int GetRadius() {
      return Radius;
    };
    int SetRadius(int newR);
    int Turn(int angle);
    int FixedTurn(int angle);
    int Balance();
    int CheckBalance();
    void BasicPosition();
    int Move(int direction);
  private:
    TLeg legs[6];
    byte contacts = 0, Radius = 30;
    int positionH = -90, positionV = 0;
    static const byte a = 85;// al = 15;
    void UpdateContacts();
    int MaxHeight();
    inline void TwoLegsUpDown(int i, int j,int a);
    inline void ThreeLegsUpDown(int i, int j, int k, int a);
};
#endif
