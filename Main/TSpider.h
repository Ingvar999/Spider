#pragma once
#ifndef _TSpider_h_
#define _TSpider_h_

#include "TLeg.h"
#include "TGyro.h"
#include "TESP8266.h"

class TSpider
{
  public:
    TGyro gyro;
    TESP8266 esp;
    
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
    long ReadVcc();
    void CheckVcc();
    String GetInfoInHtml(long res);
  private:
    static const byte a = 85;
    static const byte minRadius = 20;
    static const byte lifting = 30;
    static const byte stepLength = 20;
    static const byte motionDelaying = 10;
    static const float maxSkew = 5;
    static const unsigned balancingInterval = 500;

    TLeg legs[6];
    byte contacts = 0, Radius = 30;
    int positionH = 0, positionV = 0;
    unsigned long int lastBalancingTime = 0;

    void UpdateContacts();
    int MaxHeight();
    int MinHeight();

    inline void TwoLegsUpDown(int i, int j, int dir);
    inline void ThreeLegsUpDown(int i, int j, int k, int dir);
};
#endif
