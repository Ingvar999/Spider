#pragma once
#ifndef _TSpider_h_
#define _TSpider_h_

#include "TLeg.h"
#include "TESP8266.h"
#include "TSubBoard.h"

class TSpider
{
  public:
    TESP8266 esp;
    TSubBoard board;

    bool balanceActive = false;
    bool workloadsAlignemtActive = false;
    bool heightControlActive = true;
    bool checkVccActive = true;
    bool powerOn = true;
    int errno = 0;

    inline void PowerOn();
    inline void PowerOff();
    void Init();
    void InitLeg(int, int, int, int, int, int = 0, int = 0, int = 10);
    void StartTimer(unsigned long);
    void UpdateAllAngles();
    int ChangeHeight(int, bool);
    int GetRadius() {
      return Radius;
    };
    int SetRadius(int newR);
    int Turn(int angle);
    int FixedTurn(int angle);
    int PositionAlignment();
    void BasicPosition();
    int Move(int direction);
    void CheckVcc();
    void UpdateWorkloads();
    int ReachGround();
    int WorkloadsAlignment();
    int GetUp(int);
    int HeightControl();

    String HandleCurrentRequest();
  private:
    static const int powerPin = 40;
    static const byte a = 85; 
    static const byte minRadius = 40;
    static const byte minLifting = 30;
    static const byte stepLength = 20;
    static const byte maxTurn = 20;
    static const byte motionDelaying = 6;
    static const int stepDelaying = 350;
    static const float maxSkew = 4.5;
    static const int minWorkloadThreshold = 50;
    static const float maxWorkloadDisparityRate = 0.35;

    TLeg legs[6];
    byte Radius = minRadius, height = 0;
    int positionH = 0, positionV = 0;

    int MaxHeight();
    int MinHeight();
    int Balance();
    inline void TwoLegsUpDown(int i, int j, int dir);
    inline void ThreeLegsUpDown(int i, int j, int k, int dir);
    int DoCommand();
    String GetInfo();
    int SetProperty();
};

TSpider Spider;
#endif
