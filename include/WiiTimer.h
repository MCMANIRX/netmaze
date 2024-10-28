#pragma once
#define ONE_SEC_FRAMES 60
class WiiTimer {

    public:
        WiiTimer(int dividend=ONE_SEC_FRAMES):
        dividend(dividend){count=0;}

    void setDividend(int div);
    void resetTimer();
    bool checkTimer(int line);
    
    private:
    int dividend;
    int count;

};
