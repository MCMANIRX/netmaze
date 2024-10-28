 #include "WiiTimer.h"
 void WiiTimer::setDividend(int div){
    dividend = div;
}



void WiiTimer::resetTimer() {
    count = 0;
    dividend = ONE_SEC_FRAMES;
    
}

bool WiiTimer::checkTimer(int line) {

    if(line==0)
        count++;
    if(count==dividend) {
        count=0;
        return true;
    }

    return false;
}