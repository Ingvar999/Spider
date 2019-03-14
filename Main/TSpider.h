#pragma once

#include <NewPing.h>

#include "Constants.h"
#include "TLeg.h"
#include "TESP8266.h"
#include "TSubBoard.h"
#include "SimpleQueue.h"
#include "TTask.h"

class TSpider
{
  public:
    TESP8266 esp;
    TSubBoard board;
    NewPing *sonar;

    bool balanceActive = true;
    bool workloadsAlignemtActive = true;
    bool heightControlActive = true;
    bool checkVccActive = true;
    bool lightControlActive = true;
    bool powerOn = true;
    bool onSurface = false;

    inline void PowerOn();
    inline void PowerOff();
    void Init();
    void InitLeg(int, int, int, int, int, int = 0, int = 0, int = 10);
    void StartTimer(unsigned long);
    void UpdateAllAngles();
    void ChangeHeight(int, bool = true);
    int GetRadius() {
      return Radius;
    };
    void SetRadius(int newR);
    void Turn(int angle);
    void FixedTurn(int angle);
    int PositionAlignment();
    void BasicPosition();
    void Move(int direction);
    void CheckVcc();
    void CheckLight();
    int UpdateWorkloads();
    void ReachGround();
    int WorkloadsAlignment();
    int HeightControl();
    void Wander();
    void DoCommands();
    String GetErrorMessage();
    String HandleCurrentRequest();
    void Update_OnSurface_Worklods_Position();
    void ControlServices();
  private:
    static const int powerPin = 40;
    static const int ledPin = 52;
    static const int echoPin = 43;
    static const int trigPin = 45;
    static const int lightDetectionPin = 47;
    static const byte a = 85; 
    static const byte minRadius = 40;
    static const byte minLifting = 30;
    static const byte stepLength = 20;
    static const byte maxTurn = 20;
    static const int stepDelaying = 300;
    static const float maxSkew = 4.5;
    static const int minWorkloadThreshold = 50;
    static const int maxWorkloadThreshold = 500;
    static const float maxWorkloadDisparityRate = 0.35;
    //static const int majorDirection = 150;
    static const int minDistance = 30;
    //static const int minWorkloadOnSurface = 150;
    static const int minVoltage = 6000;
   
    TLeg legs[6];
    SimpleQueue<TTask> tasksQueue;
    byte Radius = minRadius, height = 0;
    int positionH = 0, positionV = 0;
    byte motionDelaying = 5;
    TErrno errno = OK;
    
    bool isLightning = false;

    int MaxHeight();
    int MinHeight();
    int Balance();
    inline void TwoLegsUpDown(int i, int j, int dir);
    inline void ThreeLegsUpDown(int i, int j, int k, int dir);
    String GetInfo();
    String SetProperty();
    bool isValidDistance(){int dist = sonar->ping_cm(); return (dist == 0 || dist > minDistance);}
    void SetErrno(TErrno error);
    void TurnLight(int state);
};

TSpider Spider;
