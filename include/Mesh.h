#pragma once


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <stdio.h>
#include <vector>
#include <string>
#include <stdlib.h>

#include <gccore.h>
#include <grrlib.h>
using namespace std;

#define CLR_STD 0xffffffff

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    string type;
    string path;  // we store the path of the texture to compare with other textures
};

class Mesh {
public:
    // mesh data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;

    Mesh()
 {
    }

    void setupMesh(vector<Vertex> v, vector<unsigned int> i) {
            this->vertices = v;
            this->indices = i;
        }

    

    void Draw(int color) {

            if (color == NULL)
                color = CLR_STD;



            GX_Begin(GX_TRIANGLES, GX_VTXFMT0, this->indices.size());

            
            for(int i = 0 ; i < this->indices.size(); ++i) {

                auto v = this->vertices[indices[i]];

                GX_Position3f32(v.Position.x,v.Position.y,v.Position.z);
                GX_Normal3f32(v.Normal.x,v.Normal.y,v.Normal.z); 
                GX_Color1u32(color); //for now, no vertex colors
                GX_TexCoord2f32(v.TexCoords.x,1.0f-v.TexCoords.y);

            }
            GX_End();

    }



};



