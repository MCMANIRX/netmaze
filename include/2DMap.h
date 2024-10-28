#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <grrlib.h>
#include "random.h"

#define MAZE_START 4
#define MAZE_END   5

// this type exists to avoid using expensive glm::vec2s
struct vec2d {
    u8 x;
    u8 y;

};
struct node
{
    struct node *left;
    struct node  *up;
    struct node *right;
    struct node *down;
    char data;
    
};

struct list {
    int l;
    int w;
    struct node * map;
};
struct min_inst {
    glm::vec3 pos;
    glm::vec3 rot;
}
;



node * createNode(int data)
{
    node *n = (node*)malloc(sizeof(node));
    n->down = NULL;
    n->right = NULL;
    n->left = NULL;
    n->up = NULL;
    n->data = data;

    return n;
};

node * ConstructList(int H, int W, int i, int j, u8 **data, node * prev) {
   // printf("%d %d\n",i,j);
    if(i>=H || j>= W)
        return NULL;

    node *n = createNode(data[i][j]);

    if(j == 0)
        n->up = prev;
    else{
        n->left = prev;
        if(i>0) {
            prev->up->right->down = n;
            n->up = prev->up->right;
        }
    }
        n->right = ConstructList(H,W,i,j+1,data,n);
        if(j == 0)
            n->down = ConstructList(H,W,i+1,0,data,n);
    return n;
}


void printMap(node *map) {
    if(!map) return;
    node *row = map;

while(row!=NULL) {
    node *cur = row;

    while(cur != NULL) {
        printf("%d\t",cur->data);
        cur = cur->right;
    }
    printf("\n");
    row = row->down;
}


} 

node * coordToNode(node * map, std::vector<int> coords) {

    node * ret = map;

    for(int i = 0; i < coords[0]; ++i)
        ret = ret->right;
    
    for(int i = 0; i < coords[1]; ++i)
        ret = ret->down;    
    
    return ret;

}

void setNodeData(node * n, int data) {

    n->data = data;

}

/*int main() {

    int data[][W] = {{1,2,3},
            {8,5,6},    
            {7,6,9}};
    
    list l;
    l.map= ConstructList(0,0,data,NULL);
    printMap(l.map);
    return 0 ;
}*/



float lerp(float x, float y, float t) {
  return x * (1.f - t) + y * t;
}

inline float slerp(const float& angle1, const float& angle2, const float& alpha)
{
    auto angleDiff = angle2-angle1;
    angleDiff = std::fmod(angleDiff, 2*M_PI);
    angleDiff = std::fmod(angleDiff + 3*M_PI, 2*M_PI)-M_PI;
    return angle1+alpha*angleDiff;
}