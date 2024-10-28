#include "2DMap.h"
static inline uint32_t coordsRGBA8(uint32_t x, uint32_t y, uint32_t w);


list constructMaze(GRRLIB_texImg *map,vec2d& start_pos, vec2d& end_pos,std::vector<min_inst>& walls) {

    const u32 w = map->w;
    const u32 h = map->h;

    u8 **bitmap = new u8*[h];
    for (int i = 0; i < h; ++i){
        bitmap[i] = new u8[w];
        std::memset(bitmap[i], 0xff, w * sizeof(u8));  
    }


        for(int i = 0; i < h; ++i) 
            for(int j =0; j < w; ++j){
                // de-swizzle texture
                int offset = coordsRGBA8(j,i,w);
                int r,g,b;
                r =((((u8*)(map->data))[offset+1])&0xff);
                g =((((u8*)(map->data))[offset+32])&0xff);
                b =((((u8*)(map->data))[offset+33])&0xff);

                if(r&g&b);
                else if(b)
                    end_pos = vec2d({j,i});
                else if(g)          
                    start_pos = vec2d({j,i});
                else{
                    walls.push_back(min_inst({glm::vec3(j,0,i),glm::vec3(0,((int)(rand()%4))*90.0f,0)}));
                    bitmap[i][j]=0;
                }


            }


    list l;
    l.map = ConstructList(map->h,map->w,0,0,bitmap,NULL);
    
    for (int i = 0; i < h; ++i) {
        delete[] bitmap[i];  // Free each dynamically allocated row
    }

    return l;
    
}

/*

}*/

static inline uint32_t coordsRGBA8(uint32_t x, uint32_t y, uint32_t w)
{
	return ((((y >> 2) * (w >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
}