// interface class for network solvers
#include "2DMap.h"
#include <algorithm>

#define IDLE_THRESH 4


class NetSolver {

    public:
        NetSolver(node * start,vec2d start_coords,std::string ip,u32 color, int anim_dividend)
        :ptr(start)
        ,start_coords(start_coords)
        ,coords(start_coords)
        ,next_coords(start_coords)
        ,div((float)anim_dividend)
        ,ip(ip)
        ,color(color)
        {

            rot = 0;
            w_count = 0;
            idle = 0;
            next_rot = 0;
            done_msg = false;
            done=false;
            moveCount = 0;

        };
    
    int scan();
    void move(char dir);
    vec2d getCoords()const;
    float getRot()const;
    float getNextRot()const;
    float getNextX();
    float getNextY();
    int getWCount()const;
    void setWCount(int ws);
    void move();
    float t_();
    u32 moveCount;

    u32 getColor()const;
    bool doneWithMaze();
    bool getDoneMsg();
    void reset(node *n);
    void idlePenalize();
    int getIdle()const;
    std::string getIP()const;
    void idleClear();



    private:
        node *ptr;
        vec2d coords;
        vec2d start_coords;
        vec2d next_coords;
        std::string ip;
        u32 color;
        bool done;
        bool done_msg;
        float rot;
        float next_rot;
        float div;
        float t;
        
        int w_count;
        int checkMove(node *n)const;
        int idle;
};

int NetSolver::checkMove(node * n) const{

    if(n)
        if(n->data!=0){
            return 1;
        }
        return 0;
}

int NetSolver::scan() {
    if(ptr->data==MAZE_END){
        done = true;
        w_count++;
    }

return (checkMove(ptr->up) << 3) | (checkMove(ptr->down) << 2) | (checkMove(ptr->left) << 1) | checkMove(ptr->right);

}

void NetSolver::move(char dir) {
    moveCount++;
    t=0;
    rot = next_rot;
    coords = next_coords;
    if(dir=='u'){
        ptr = ptr->up;
        next_coords.y--;
        next_rot = 180;
    }
    else if(dir=='d'){
       ptr = ptr->down;
        next_coords.y++;
        next_rot = 0;
    }
    else if(dir=='l'){
        ptr = ptr->left;
        next_coords.x--;
        next_rot = -90;
    }
    else if(dir=='r'){
        ptr = ptr->right;
        next_coords.x++;
        next_rot = 90;
}
}

void NetSolver::reset(node *n) {
    moveCount = 0;
    ptr = n;
    coords = next_coords = start_coords;
    rot = next_rot = 0;
    done_msg =false;
    done=false;
}

void NetSolver::idlePenalize() {
    idle++;
}
void NetSolver::idleClear() {
    idle=0;
}

vec2d NetSolver::getCoords() const{
    return coords;
}

float NetSolver::getNextX(){
    return lerp(coords.x,next_coords.x,t);

}
float NetSolver::getNextY(){
    return lerp(coords.y,next_coords.y,t);

}

float NetSolver::getRot() const{
    return rot;
}

float NetSolver::getNextRot()const {

    float mult = 2;
    
    return slerp(rot*0.0174533,next_rot*0.0174533,(t*mult)>1?1:t*mult)*57.2958;
}

void NetSolver::move() {


     t+=(1/div);
}

bool NetSolver::doneWithMaze(){
    if(done&(!done_msg))
        done_msg = true;
    else done_msg == false;
    
    return done;
}

bool NetSolver::getDoneMsg() {
    return done_msg;
}

int NetSolver::getWCount() const{
    return w_count;
}
void NetSolver::setWCount(int ws) {
    w_count = ws;
}

u32 NetSolver::getColor() const{
    return color;
}

int NetSolver::getIdle() const{
    return idle;
}
std::string NetSolver::getIP() const{
    return ip;
}

float NetSolver::t_() {
   return t;
}