// RaldConsoleEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "src/Engine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <Windows.h>

struct RayCastingResult
{
    RayCastingResult(float dist, bool bound) : distanceToWall(dist), isBoundary(bound)
    {
    }
    float distanceToWall;
    bool isBoundary;
};


static void UpdateUser();
static RayCastingResult ComputeRayDistanceToWall(int rayX);
static short ComputeWallShade(RayCastingResult distanceToWall);
static short ComputeFloorShade(int y);
static void RenderCell(int x, int y, int ceilingPos, int floorPos, RayCastingResult distanceToWall);
static void RenderMap();
static void RenderInformation();


constexpr int screenWidth = 120;
constexpr int screenHeight = 40;

constexpr int mapWidth = 16;
constexpr int mapHeight = 16;

constexpr float renderDistance = 16.0f;

static float playerPosX = 8.0f;
static float playerPosY = 8.0f;
static float playerAngle = 0.0f;

static float playerFov = 3.1416f / 4.0f;

// The map of the level.
// '#' is a wall,
// ' ' is walkable space,
// 'P' is the player.
static std::wstring map = L"";

// Screen buffer
static wchar_t screen[screenWidth * screenHeight] = { 0 };

int main()
{
    Engine::Begin();
    // Create a handle to the consoles output buffer.
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                                0,
                                                nullptr,
                                                CONSOLE_TEXTMODE_BUFFER,
                                                nullptr);

    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    map += L"################";
    map += L"#       #      #";
    map += L"#       #      #";
    map += L"#       #      #";
    map += L"#       #      #";
    map += L"#       #      #";
    map += L"#              #";
    map += L"#              #";
    map += L"#     #        #";
    map += L"#              #";
    map += L"#        #     #";
    map += L"#              #";
    map += L"#       ########";
    map += L"#  #           #";
    map += L"#              #";
    map += L"################";

    // Game loop
    while (true)
    {
        UpdateUser();

        for (int x = 0; x < screenWidth; x++)
        {
            RayCastingResult ray = ComputeRayDistanceToWall(x);

            // Calculate distance from the POV to the ceiling and the floor.
            // POV isn't at floor level, that's why screen height is divided by 2.
            auto ceiling = int(float(screenHeight / 2.0f) - screenHeight / ray.distanceToWall);
            int floor = screenHeight - ceiling;

            // Draw the column for that ray
            for (int y = 0; y < screenHeight; y++)
            {
                RenderCell(x, y, ceiling, floor, ray);
            }
        }

        RenderInformation();
        RenderMap();

        // Terminate the screen buffer with a null terminator.
        screen[screenWidth * screenHeight - 1] = '\0';

        // Draw the buffer.
        WriteConsoleOutputCharacter(hConsole,
                                    screen,
                                    screenWidth * screenHeight,
                                    { 0,0 },
                                    &dwBytesWritten);
    }

    Engine::End();

    return 0;
}

void UpdateUser()
{
    if (Engine::SyncFrame() == false)
    {
        return;
    }

    float deltaTime = Engine::DeltaTime();

    float lastPlayerX = playerPosX;
    float lastPlayerY = playerPosY;

    // Controls
    // Handle counter clock-wise rotation.
    if (GetAsyncKeyState(unsigned short('A')) & 0x8000)
    {
        playerAngle -= 1.0f * deltaTime;
    }
    // Handle clock-wise rotation.
    if (GetAsyncKeyState(unsigned short('D')) & 0x8000)
    {
        playerAngle += 1.0f * deltaTime;
    }
    // Handle forward movement.
    if (GetAsyncKeyState(unsigned short('W')) & 0x8000)
    {
        playerPosX += std::sin(playerAngle) * 5.0f * deltaTime;
        playerPosY += std::cos(playerAngle) * 5.0f * deltaTime;
    }
    // Handle backward movement.
    if (GetAsyncKeyState(unsigned short('S')) & 0x8000)
    {
        playerPosX -= std::sin(playerAngle) * 5.0f * deltaTime;
        playerPosY -= std::cos(playerAngle) * 5.0f * deltaTime;
    }

    // Handle collision detection.
    // If the player is in a wall block:
    if (map[uint64_t(playerPosY) * uint64_t(mapWidth) + uint64_t(playerPosX)] == '#')
    {
        // Push it back where it was.
        playerPosX = lastPlayerX;
        playerPosY = lastPlayerY;
    }
}

RayCastingResult ComputeRayDistanceToWall(int rayX)
{
    // For each column, calculate the projected ray angle into world space.
    float rayAngle = (playerAngle - playerFov / 2.0f) +
        ((float)rayX / (float)screenWidth) * playerFov;

    float distanceToWall = 0;
    bool hasHitWall = false;
    bool isBoundary = false;

    // Unit vector for ray in player space.
    float eyeX = std::sin(rayAngle);
    float eyeY = std::cos(rayAngle);

    while (hasHitWall == false && distanceToWall <= renderDistance)
    {
        distanceToWall += 0.1f;

        // Cast a ray to the length we're now checking.
        // The ray is an integer because 
        // the walls' boundaries are integers as well.
        auto testX = (int)(playerPosX + eyeX * distanceToWall);
        auto testY = (int)(playerPosY + eyeY * distanceToWall);

        // If the ray is out of bound (outside of the map's limit):
        if (testX < 0 || testX >= mapWidth ||
            testY < 0 || testY >= mapHeight)
        {
            // Render nothing.
            hasHitWall = true;
            distanceToWall = renderDistance;
        }
        // Ray is inbounds, so test to see if the ray cell we're at is a wall block.
        else if (map.at(uint64_t(testY) * uint64_t(mapWidth) + uint64_t(testX)) == '#')
        {
            // Cell is a wall.
            hasHitWall = true;

            // Distance to corner, angle between the two vectors.
            std::vector<std::pair<float, float>> p;

            for (int tx = 0; tx < 2; tx++)
            {
                for (int ty = 0; ty < 2; ty++)
                {
                    float vy = float(testY) + ty - playerPosY;
                    float vx = float(testX) + tx - playerPosX;

                    float d = std::sqrt(vx * vx + vy * vy);
                    float dot = (eyeX * vx / d) + (eyeY * vy / d);

                    p.emplace_back(std::make_pair(d, dot));
                }
            }

            // Sort pairs from closest to farthest.
            std::sort(p.begin(), p.end(),
                      [](const std::pair<float, float>& left, const std::pair<float, float>& right)
                      {
                          return left.first < right.first;
                      });

            float bound = 0.01f;
            if (std::acos(p.at(0).second) < bound)
            {
                isBoundary = true;
            }
            if (std::acos(p.at(1).second) < bound)
            {
                isBoundary = true;
            }
        }
    }
    return RayCastingResult(distanceToWall, isBoundary);
}

short ComputeWallShade(RayCastingResult ray)
{
    short shade = ' ';

    if (ray.distanceToWall <= renderDistance / 4.0f)
    {
        // Very close to the player. ('█')
        shade = 0x2588;
    }
    else if (ray.distanceToWall < renderDistance / 3.0f)
    {
        // '▓'
        shade = 0x2593;
    }
    else if (ray.distanceToWall < renderDistance / 2.0f)
    {
        // '▒'
        shade = 0x2592;
    }
    else if (ray.distanceToWall < renderDistance)
    {
        // '░'
        shade = 0x2591;
    }
    else
    {
        // Too far away.
        shade = ' ';
    }

    // If we're at a boundary:
    if (ray.isBoundary)
    {
        shade = ' ';
    }

    return shade;
}

short ComputeFloorShade(int y)
{
     // Shade floor based on distance.
    short shade = ' ';

    float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
    if (b < 0.25f)
    {
        shade = '#';
    }
    else if (b < 0.5f)
    {
        shade = 'x';
    }
    else if (b < 0.75f)
    {
        shade = '.';
    }
    else if (b < 0.9f)
    {
        shade = '-';
    }
    else
    {
        shade = ' ';
    }

    return shade;
}

void RenderCell(int x, int y, int ceilingPos, int floorPos, RayCastingResult ray)
{
    // If `y` is inferior to the ceiling:
    if (y < ceilingPos)
    {
        // Draw empty space.
        screen[y * screenWidth + x] = ' ';
    }
    // If we're at a wall:
    else if ((y > ceilingPos) && (y <= floorPos))
    {
        // Draw a wall.
        screen[y * screenWidth + x] = ComputeWallShade(ray);
    }
    else
    {
        // Draw a floor.
        screen[y * screenWidth + x] = ComputeFloorShade(y);
    }
}

void RenderMap()
{
    for (int x = 0; x < mapWidth; x++)
    {
        for (int y = 0; y < mapHeight; y++)
        {
            screen[(y + 1) * screenWidth + x] = map[uint64_t(y) * uint64_t(mapWidth) + uint64_t(x)];
        }
    }

    // Draw the player.
    screen[(int(playerPosY + 1)) * screenWidth + int(playerPosX)] = 'P';
}

void RenderInformation()
{
    float deltaTime = Engine::DeltaTime();

    // Display stats.
    swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ",
               playerPosX, playerPosY, playerAngle, 1 / deltaTime);
}
