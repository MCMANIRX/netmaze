#include "OrbitCamera.h"
#include "Mesh.h"
#include "OBJ_Loader.h"
#include "maze.h"
#include "solvers/Solver.h"
#include "solvers/NetSolver.h"
#include "Logger.h"
#include "network.h"
#include "WiiTimer.h"

#include <gccore.h>
#include <ogc/pad.h>
#include <grrlib.h>
#include <fat.h>

#include "efd_jpg.h"
#include "RGFX_Font_png.h"
#include "blank_png.h"
#include "map0_png.h"

#include <map>
#include <stdint.h>
#include <math.h>

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

list maze;

OrbitCamera *cam;

float mov_spd = 0.125f;
float rot_spd = mov_spd / 16.0f;

float radius = 26.0f;

int vertex_count;

// global list of network clients
std::map<std::string, NetSolver> client_list;

vec2d start_pos;
vec2d end_pos;

float lerp(float x, float y, float t);
void updateCamera(OrbitCamera *cam_);
void OBJLoad(std::vector<objl::Mesh> &lm, std::vector<Mesh> &_meshes);

int flr(int x)
{
    if (abs(x) < 50)
        return 0;
    else
        return x;
}

int main()
{

    PAD_Init();
    GRRLIB_Init();
    GRRLIB_SetBackgroundColour(0x0, 0x0, 0x0, 0xFF);
    GRRLIB_Settings.antialias = true;

    // load assets from FAT

    objl::Loader Loader;

    fatInitDefault();

    vector<Mesh> meshes;
    vertex_count = 0;
    Loader.LoadFile("sd:/netmaze/wall.obj");
    OBJLoad(Loader.LoadedMeshes, meshes);
    Loader.LoadFile("sd:/netmaze/tank.obj");
    OBJLoad(Loader.LoadedMeshes, meshes);
    Loader.LoadFile("sd:/netmaze/flag.obj");
    OBJLoad(Loader.LoadedMeshes, meshes);
    Loader.LoadFile("sd:/netmaze/start.obj");
    OBJLoad(Loader.LoadedMeshes, meshes);

    GRRLIB_texImg *wall_tex = GRRLIB_LoadTextureFromFile("sd:/netmaze/wall.png");
    GRRLIB_texImg *floor_tex = GRRLIB_LoadTextureFromFile("sd:/netmaze/floor.png");
    GRRLIB_texImg *tank_tex = GRRLIB_LoadTextureFromFile("sd:/netmaze/tank_map.png");
    GRRLIB_texImg *flag_tex = GRRLIB_LoadTextureFromFile("sd:/netmaze/flag.png");
    GRRLIB_texImg *start_tex = GRRLIB_LoadTextureFromFile("sd:/netmaze/start.png");
    GRRLIB_texImg *map0 = GRRLIB_LoadTextureFromFile("sd:/netmaze/map0.png");

    // load assets from .dol

    GRRLIB_texImg *momo = GRRLIB_LoadTexture(efd_jpg);
    GRRLIB_texImg *blank = GRRLIB_LoadTexture(blank_png);
    GRRLIB_texImg *GFX_Font = GRRLIB_LoadTexturePNG(RGFX_Font_png);

    GRRLIB_InitTileSet(GFX_Font, 8, 16, 32);

    // setup global Logger singleton
    Logger &logger = Logger::getInstance();
    logger.setFont(GFX_Font);

    // start network thread
    initServer();

    cameraPos.z = radius;
    cam = new OrbitCamera(glm::vec3(0.0f), cameraUp, 2, 0.001f, 3.14159f * 0.5f, 0.0f);

    cam->moveVertical(20);
    cam->moveHorizontal(map0->w - 1);
    cam->rotatePolar(M_PI_4);
    cam->moveForBack(-40);

    /////////////////////////////////////////////////////////////////////////////////////////////
    // Generate maze data structure

    std::vector<min_inst> walls;

    maze = constructMaze(map0, start_pos, end_pos, walls);

    float scale = 2.0f;

    int pos_cnt = 0;

    setNodeData(coordToNode(maze.map, {start_pos.x, start_pos.y}), MAZE_START);
    setNodeData(coordToNode(maze.map, {end_pos.x, end_pos.y}), MAZE_END);



    ///////////////////////////////////////////////////////////////////
    // define floor plane manually

    std::vector<Vertex> floor_pts;

    float tex_scale_x = map0->w;
    float tex_scale_y = map0->w;

    floor_pts.push_back({glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec2(0, 0)});
    floor_pts.push_back({glm::vec3(map0->w * 2, 0, 0), glm::vec3(0, 0, 0), glm::vec2(tex_scale_x, 0)});
    floor_pts.push_back({glm::vec3(0, 0, map0->h * 2), glm::vec3(0, 0, 0), glm::vec2(0, tex_scale_y)});
    floor_pts.push_back({glm::vec3(map0->w * 2, 0, map0->h * 2), glm::vec3(0, 0, 0), glm::vec2(tex_scale_x, tex_scale_y)});

    Mesh floor;
    floor.setupMesh(floor_pts, {0, 1, 2,
                                1, 3, 2});

    //////////////////////////////////////////////////////////////////////////////////
    // begin game logic
    int tick_div = 60;
    Solver local_solver(coordToNode(maze.map, {start_pos.x, start_pos.y}), start_pos, tick_div);
    u32 payload = 0;
    int *weight;
    int *good;

    int cur_line = 0;
    int elapsed = 0;

    WiiTimer timer_60(60);
    WiiTimer tick_timer(tick_div);

    while (1)
    {
        cur_line = VIDEO_GetCurrentLine();

        // global server-side updates for network clients
        if (timer_60.checkTimer(cur_line))
        {
            elapsed++;
            logger.update();

            for (auto &tank : client_list)
            {
                tank.second.idlePenalize(); // currently not in use
                if (tank.second.getIdle() > IDLE_THRESH)
                {
                    logger.logf("Tank %s disconnected.", tank.second.getIP().c_str());
                    client_list.erase(client_list.find(tank.second.getIP()));
                }
            }
        }
        // updates for local client
        if (tick_timer.checkTimer(cur_line))
        {
            if (!local_solver.doneWithMaze())
                local_solver.traverse();
            else
            {
                local_solver.setWCount(local_solver.getWCount() + 1);
                logger.logf("W for Local Tank! Won in %d moves.", local_solver.moveCount);
                local_solver.reset(coordToNode(maze.map, {start_pos.x, start_pos.y}));
            }
            weight = local_solver.DEBUG_getWeights();
            good = local_solver.DEBUG_getGoods();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        PAD_ScanPads();
        int8_t padx = flr(PAD_StickX(0));
        int8_t pady = flr(PAD_StickY(0));
        int8_t spadx = flr(PAD_SubStickX(0));
        int8_t spady = flr(PAD_SubStickY(0));

        int trigger = PAD_ButtonsHeld(0) & PAD_TRIGGER_R ? -1 : PAD_ButtonsHeld(0) & PAD_TRIGGER_L ? 1 : 0;

        // exit game or reset camera
        if (PAD_ButtonsDown(0) & PAD_BUTTON_START)
            break;
        else if (PAD_ButtonsDown(0) & PAD_BUTTON_X)
        {
            cam = new OrbitCamera(glm::vec3(0.0f), cameraUp, cameraPos.z, 1.0f, 3.14159f * 0.5f, 0.0f);

            cam->moveVertical(20);

            cam->moveHorizontal(map0->w - 1);
            cam->rotatePolar(M_PI_4);
            cam->moveForBack(-40);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Begin 3D draw 
        GRRLIB_3dMode(0.001, 1000, 45, 1, 1);

        cam->rotateAzimuth(-spadx / abs(spadx) * rot_spd);
        cam->rotatePolar(spady / abs(spady) * rot_spd);
        cam->moveHorizontal(padx / abs(padx) * mov_spd);
        cam->moveForBack(pady / abs(pady) * mov_spd);
        cam->zoom(trigger * mov_spd * 2);
        updateCamera(cam);

        // GRRLIB_SetLightSpec(0, (guVector){0.0f,0.0f,0.0f}, 130.0f, 0x11, 0x11); //maybe one day...

        GX_SetNumChans(1);
        GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPEC);
        GRRLIB_SetLightAmbient(0x808080FF);
        GRRLIB_SetLightDiff(0, (guVector){map0->w * 2, 20, 4}, 10.0f, 10.0f, 0xffe6d3FF);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //draw walls

        for (int k = 0; k < walls.size(); ++k)
        {

            GRRLIB_ObjectViewBegin();
            GRRLIB_ObjectViewRotate(0, walls[k].rot.y, 0);
            GRRLIB_ObjectViewTrans(walls[k].pos.x * scale, walls[k].pos.y * scale, walls[k].pos.z * scale);
            GRRLIB_ObjectViewEnd();

            GRRLIB_SetTexture(wall_tex, FALSE);
            meshes[0].Draw(CLR_STD);
        }


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //draw network tanks

       // float blinn = 0;  //debug
       // float phong = 0;
        for (auto &tank : client_list)
        {

            tank.second.move();

            GRRLIB_ObjectViewBegin();
            GRRLIB_ObjectViewRotate(0, tank.second.getNextRot(), 0);
           // blinn = tank.second.t_(); //debug
           // phong = tank.second.getCoords().x;
            GRRLIB_ObjectViewTrans(tank.second.getNextX() * scale, -1, tank.second.getNextY() * scale);
            GRRLIB_ObjectViewEnd();

            GRRLIB_SetTexture(tank_tex, FALSE);
            meshes[1].Draw(tank.second.getColor());
        }


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //draw local tank

        GRRLIB_ObjectViewBegin();
        GRRLIB_ObjectViewRotate(0, local_solver.getNextRot(), 0);
        GRRLIB_ObjectViewTrans(local_solver.getNextX() * scale, -1, local_solver.getNextY() * scale);
        GRRLIB_ObjectViewEnd();
        local_solver.move();

        GRRLIB_SetTexture(tank_tex, FALSE);
        meshes[1].Draw(CLR_STD);


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //draw end flag

        GRRLIB_ObjectViewBegin();
        GRRLIB_ObjectViewRotate(0, 90, 0);
        GRRLIB_ObjectViewTrans(end_pos.x * scale, -1, end_pos.y * scale);
        GRRLIB_ObjectViewEnd();

        GRRLIB_SetTexture(flag_tex, FALSE);
        meshes[2].Draw(0xffffffff);


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //draw beginning...thing

        GRRLIB_ObjectViewBegin();
        GRRLIB_ObjectViewTrans(start_pos.x * scale, -0.95, start_pos.y * scale);
        GRRLIB_ObjectViewEnd();

        GRRLIB_SetTexture(start_tex, FALSE);
        meshes[3].Draw(CLR_STD);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //draw floor

        GRRLIB_ObjectViewBegin();
        GRRLIB_ObjectViewTrans(-1.0f, -1.0f, -1.0f);
        GRRLIB_ObjectViewEnd();
        GRRLIB_SetTexture(floor_tex, true);
        floor.Draw(CLR_STD);


        // end 3D draw

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // begin 2D draw

        // most of this is debug text, but this code prints out the time elapsed and the W count per tank (via IP)

        GRRLIB_2dMode();
        int ypos = 10;
        int idx = 0;
        logger.printLog();
        // int ls = local_solver.DEBUG_getLastScan();
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "Vertex Count: %d\nMesh Count: %d %",vertex_count*wall_pos.size(),meshes.size());
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "start value %x",coordToNode(maze.map,{start_pos.x,start_pos.y})->data);
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "x: %d y: %d ",local_solver.getx(),local_solver.gety());
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "%d %d %d %d ",weight[0],weight[1],weight[2],weight[3]);
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "%d %d %d %d ",good[0],good[1],good[2],good[3]);
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "%d %d %d %d ",ls&8,ls&4,ls&2,ls&1);

        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "%d %d",local_solver.getx(),local_solver.gety());

        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "azimuth: %f",cam->getAzimuthAngle());
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "polar: %f",cam->getPolarAngle());
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "z: %f",cam->getPosition().z);
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "x: %f",cam->getPosition().x);
        // GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1, "y: %f",cam->getPosition().y);
        GRRLIB_Printf(32, 32 * ypos++, GFX_Font, CLR_STD, 1, "time: %d", elapsed);
        GRRLIB_Printf(480 - 16, 96 + 64 + 16 * ypos++, GFX_Font, CLR_STD, 1, "W's");
        GRRLIB_Printf(480 - 16, 96 + 64 + 16 * ypos++, GFX_Font, CLR_STD, 1, "-----");
        GRRLIB_Printf(480 - 16, 96 + 64 + 16 * ypos++, GFX_Font, CLR_STD, 1, "Local: %d", local_solver.getWCount());
        //  GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1,"%.4f %.4f",blinn,phong);
        //  GRRLIB_Printf   ( 32, 32*ypos++, GFX_Font, CLR_STD, 1,"cur: %d next %d",local_solver.getx(),local_solver.getNextX());

        for (auto &tank : client_list)
        {
            char *_ip = (char *)tank.second.getIP().c_str();
            char delim[2] = ".";
            char *octet = strtok(_ip, delim);
            for (int i = 0; i < 3; ++i)
                octet = strtok(NULL, delim);
            int num_octet = std::stoi(octet);
            GRRLIB_Printf(480 - 16, 96 + 64 + 16 * ypos++, GFX_Font, CLR_STD, 1, " /%03d: %d", num_octet, tank.second.getWCount());
        }
        GRRLIB_Render();
    }
    GRRLIB_FreeTexture(momo);
    GRRLIB_FreeTexture(GFX_Font);
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    while (1)
        exit(0);
}

void updateCamera(OrbitCamera *cam_)
{

    GRRLIB_Camera3dSettings(
        cam_->getEye().x, cam_->getEye().y, cam_->getEye().z,
        cam_->getUpVector().x, cam_->getUpVector().y, cam_->getUpVector().z,
        cam_->getCenter().x, cam_->getCenter().y, cam_->getCenter().z);
}

void OBJLoad(std::vector<objl::Mesh> &lm, std::vector<Mesh> &_meshes)
{
    for (int i = 0; i < lm.size(); ++i)
    {
        objl::Mesh rawMesh = lm[i];
        vector<Vertex> vertices;
        Vertex v;
        vertex_count += rawMesh.Vertices.size();

        for (int j = 0; j < rawMesh.Vertices.size(); j++)
        {
            v.Position = glm::vec3(rawMesh.Vertices[j].Position.X, rawMesh.Vertices[j].Position.Y, rawMesh.Vertices[j].Position.Z);
            v.Normal = glm::vec3(rawMesh.Vertices[j].Normal.X, rawMesh.Vertices[j].Normal.Y, rawMesh.Vertices[j].Normal.Z);
            v.TexCoords = glm::vec2(rawMesh.Vertices[j].TextureCoordinate.X, rawMesh.Vertices[j].TextureCoordinate.Y);
            vertices.push_back(v);
        }
        Mesh mesh;
        mesh.setupMesh(vertices, rawMesh.Indices);
        _meshes.push_back(mesh);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// network stuff

int initServer()
{
    Logger &logger = Logger::getInstance();

    char localip[16] = {0};
    char gateway[16] = {0};
    char netmask[16] = {0};

    int ret = if_config(localip, netmask, gateway, true, 20);
    if (ret >= 0)
    {
        logger.logf("network configured, ip: %s, gw: %s, mask %s\n", localip, gateway, netmask);

        LWP_CreateThread(&httd_handle, /* thread handle */
                         httpd,        /* code */
                         localip,      /* arg pointer for thread */
                         NULL,         /* stack base */
                         64 * 1024,    /* stack size */
                         50 /* thread priority */);
    }
    else
    {
        logger.logf("network configuration failed!\n");
    }
    return ret;
}

const static char http_200[] = "HTTP/1.1 200 OK\r\n";
const static char http_409[] = "HTTP/1.1 409 CONFLICT\r\n";

const static char indexdata[] = "<html> \
                               <head><title>A test page</title></head> \
                               <body> \
                               This small test page has had %d hits. \
                               </body> \
                               </html>";

const static char http_html_hdr[] = "Content-type: text/html\r\n\r\n";
const static char http_byte_hdr[] = "Content-Type: application/octet-stream\r\nContent-Length: 1\r\n\r\n";

const static char http_get_index[] = "GET / HTTP/1.1\r\n";

auto getClientInMap(std::string ip)
{
    return client_list.find(ip);
}
bool clientInMap(std::string ip)
{
    return getClientInMap(ip) != client_list.end();
}

//---------------------------------------------------------------------------------
void *httpd(void *arg)
{
    //---------------------------------------------------------------------------------
    Logger &logger = Logger::getInstance();

    int sock, csock;
    int ret;
    u32 clientlen;
    struct sockaddr_in client;
    struct sockaddr_in server;
    char temp[1026];
    static int hits = 0;

    clientlen = sizeof(client);

    sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (sock == INVALID_SOCKET)
    {
        logger.logf("Cannot create a socket!");
    }
    else
    {

        memset(&server, 0, sizeof(server));
        memset(&client, 0, sizeof(client));

        server.sin_family = AF_INET;
        server.sin_port = htons(80);
        server.sin_addr.s_addr = INADDR_ANY;
        ret = net_bind(sock, (struct sockaddr *)&server, sizeof(server));

        if (ret)
        {

            logger.logf("Error %d binding socket!", ret);
        }
        else
        {

            if ((ret = net_listen(sock, 5)))
            {

                logger.logf("Error %d listening!", ret);
            }
            else
            {

                while (1)
                {

                    csock = net_accept(sock, (struct sockaddr *)&client, &clientlen);

                    if (csock < 0)
                    {
                        logger.logf("Error connecting socket %d!", csock);
                        while (1)
                            ;
                    }

                    // logger.logf("Connecting port %d from %s\n", client.sin_port, inet_ntoa(client.sin_addr));
                    memset(temp, 0, 1026);
                    ret = net_recv(csock, temp, 1024, 0);

                    std::string ip = inet_ntoa(client.sin_addr); // ntohl(client.sin_addr.s_addr);
                    std::string str(temp);
                    int param_pos = str.find("param=");

                    // logger.clearLog();
                    // logger.logf(inet_ntoa(client.sin_addr));

                    if (str.find("POST") != std::string::npos)
                    {

                        if (str[param_pos + 6] == 'j')
                        {

                            int div = 0;
                            sscanf(&str[param_pos + 7], "%d", &div);

                            if (clientInMap(ip))
                                net_send(csock, http_409, strlen(http_409), 0);

                            else
                            {
                                int color = (((u8)(random() * 0xff)) << 24) | (((u8)(random() * 0xff)) << 16) | (((u8)(random() * 0xff)) << 8) | 0xff;
                                NetSolver *c = new NetSolver(coordToNode(maze.map, {start_pos.x, start_pos.y}), start_pos, ip, color, div);

                                client_list.emplace(ip, *c);
                                logger.logf("Tank %s joined the server.", ip.c_str());
                                net_send(csock, http_200, strlen(http_200), 0);
                            }
                        }

                        else if (str[param_pos + 6] == 'm')
                        {

                            if (clientInMap(ip))
                            {
                                if (!getClientInMap(ip)->second.doneWithMaze())
                                {
                                    getClientInMap(ip)->second.move(str[param_pos + 7]);
                                    net_send(csock, http_200, strlen(http_200), 0);
                                }
                                else
                                {
                                    if (getClientInMap(ip)->second.getDoneMsg())
                                        logger.logf("W for Tank %s! Won in %d moves.", ip.c_str(), getClientInMap(ip)->second.moveCount);
                                    getClientInMap(ip)->second.setWCount(getClientInMap(ip)->second.getWCount());
                                    u8 payload = 'W';
                                    net_send(csock, http_200, strlen(http_200), 0);
                                    net_send(csock, http_byte_hdr, strlen(http_byte_hdr), 0);
                                    net_send(csock, &payload, sizeof(payload), 0);
                                }
                            }
                        }
                        else if (str[param_pos + 6] == 'r')
                        {

                            if (clientInMap(ip))
                            {
                                getClientInMap(ip)->second.reset(coordToNode(maze.map, {start_pos.x, start_pos.y}));
                                net_send(csock, http_200, strlen(http_200), 0);
                            }
                        }
                    }
                    else if (str.find("GET") != std::string::npos)
                    {
                        if (str[param_pos + 6] == 's')
                        {
                            //  logger.logf("scan req");
                            if (clientInMap(ip))
                            {
                                u8 payload = getClientInMap(ip)->second.scan() + '0';
                                // logger.logf("%d",payload);

                                net_send(csock, http_200, strlen(http_200), 0);
                                net_send(csock, http_byte_hdr, strlen(http_byte_hdr), 0);
                                net_send(csock, &payload, sizeof(payload), 0);

                                // net_send(csock, http_html_hdr, strlen(http_html_hdr), 0);
                                // sprintf(temp, indexdata, hits);
                                // net_send(csock, temp, strlen(temp), 0);
                            }
                        }

                        // for(int i =0 ;i<ret;i+=16){
                        //	logger.logf("%.16s\n", temp + i);

                        // if ( !strncmp( temp, http_get_index, strlen(http_get_index) ) ) {
                        //	hits++;

                        //}
                    } //end if
                    getClientInMap(ip)->second.idleClear();
                    net_close(csock);
                } //end while
            }//end else
        }//end else
    }//end else

    return NULL;
}