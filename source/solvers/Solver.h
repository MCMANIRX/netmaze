// very basic random maze solver. Did not try to implement any cool algorithms yet, it took a lot just making this game!
#include "2DMap.h"
#include <algorithm>
#define THRESH 5000

class Solver {

    public:
        Solver(node * start,vec2d start_coords,int anim_dividend)
        :ptr(start)
        ,coords(start_coords)
        ,next_coords(coords)
        ,start_coords(start_coords)
        ,div((float)anim_dividend)
        {
            warmup_random(0.10223237);
            for(int i = 0 ; i < 4; ++i){
                good[i] =0;
                weight[i] = 4;
            }
                rot = 0;
                next_rot = 0;
                w_count = 0;
                done=false;
                flag = false;
                moveCount = 0;
        };
    
    void traverse();
    vec2d getCoords();
    vec2d getNextCoords();
    int getx();
    int gety();
    float getNextX();
    float getNextY();
    float getNextRot();
    float getT();
    int *DEBUG_getWeights();
    int *DEBUG_getGoods();
    float getRot();
    int rand_();
    bool doneWithMaze();
    void reset(node *n);
    void move();

    int getWCount();
    void setWCount(int ws);
    int cycle(int dir);
    u32 moveCount;



    private:
        node *ptr;
        vec2d coords;
        vec2d next_coords;
        vec2d start_coords;
        int good[4];
        int weight[4];
        bool done;
        float rot;
        float next_rot;
        bool flag;
        int w_count;
        float div;
        float t;


        int checkMove(node *n, int i);
        int max_();
        int __rand();
};
int Solver::__rand() {
    return (int)(__random()*4.0);
}
int Solver::max_() {
    int maxVal = weight[0], maxIndex = 0;
    bool allEqual = true;

    for (int i = 1; i < 4; ++i) {
        if (weight[i] != maxVal) allEqual = false;
        if (weight[i] > maxVal) {
            maxVal = weight[i];
            maxIndex = i;
        }
    }

    if (allEqual) {
        return __rand();       // Return random index if all are equal
    }

    return maxIndex;  // Return the index of the max value

}


int Solver::checkMove(node * n, int i) {

    if(n)
        if(n->data!=0){
            weight[i]+=__rand()*2;
            return 1;
        }

        if(!flag){
            if(max_()==i)
                flag = true;
        if(i<=1){
            weight[2]+=weight[i]/__rand();
            weight[3]+=weight[i]/__rand();
            weight[0]=weight[i]/4;
            weight[1]=weight[i]/4;

        
        }else {
            weight[0]+=weight[i]/__rand();
            weight[1]+=weight[i]/__rand();
            weight[2]=weight[i]/4;
            weight[3]=weight[i]/4;
        
        }
        }
       // weight[i] = 0;
        return 0;
}

#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

int Solver::cycle (int dir) {
    int r1 = rand()%2;
    int r2 = rand()%2;

    if(dir>2)
        dir=r1+r2;
    else dir = dir+r1+r2;
    return dir;
}

void Solver::move() {


     t+=(1/div);
}



void Solver:: traverse() {
    t=0;
    rot=next_rot;
    coords = next_coords;
    moveCount++;
    static int dir = 0;

    if(ptr->data==MAZE_END){
        done = true;
        return;
    }
    int i = 0;
    flag = false;
    good[0] = checkMove(ptr->up,0);
    good[1] = checkMove(ptr->down,1);
    good[2] = checkMove(ptr->left,2);
    good[3] = checkMove(ptr->right,3);


    while(!good[dir])
        dir=cycle(dir);
    int r3 = rand()%4;
    if(dir<= rand()%24)
        if(good[r3])
            dir =r3;







    if(dir==0){
        ptr = ptr->up;
        next_coords.y--;
        next_rot = 180.0f;
    }
    else if(dir==2){
        ptr = ptr->left;
        next_coords.x--;
        next_rot = -90.0f;
    }
    else if(dir==3){
        ptr = ptr->right;
        next_coords.x++;
        next_rot = 90.0f;
    }
        else if(dir==1){
       ptr = ptr->down;
        next_coords.y++;
        next_rot = 0.0f;
    }

}

int Solver::getx(){
    return coords.x;
}
int Solver::gety(){
    return coords.y;
}

float Solver::getNextX(){
    return lerp(coords.x,next_coords.x,t);

}
float Solver::getNextY(){
    return lerp(coords.y,next_coords.y,t);

}

int *Solver::DEBUG_getWeights() {
    return weight;
}
int *Solver::DEBUG_getGoods() {
    return good;
}

vec2d Solver::getCoords() {
    return coords;
}

vec2d Solver::getNextCoords() {
    return next_coords;
}
float Solver::getRot() {
    return rot;
}
float Solver::getNextRot() {

    float mult = 2;
    
    return slerp(rot*0.0174533,next_rot*0.0174533,(t*mult)>1?1:t*mult)*57.2958;
}
int Solver::rand_(){
    return __rand();
}

void Solver::setWCount(int ws){
    w_count=ws;
}int Solver::getWCount(){
    return w_count;
}void Solver::reset(node *n) {
    ptr = n;
    coords = next_coords = start_coords;
    rot = next_rot = 0;
    done=false;
    moveCount = 0;
}bool Solver::doneWithMaze(){
    return done;
}

