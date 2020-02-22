#include "Player.h"
#include "src/Engine.h"

#include <Windows.h>

void Player::OnUpdate()
{
    if (Engine::SyncFrame() == false)
    {
        return;
    }

    float deltaTime = Engine::DeltaTime();

    float lastPlayerX = m_xPos;
    float lastPlayerY = m_yPos;

    // Controls
    // Handle counter clock-wise rotation.
    if (GetAsyncKeyState(unsigned short('A')) & 0x8000)
    {
        m_angle -= 1.0f * deltaTime;
    }
    // Handle clock-wise rotation.
    if (GetAsyncKeyState(unsigned short('D')) & 0x8000)
    {
        m_angle += 1.0f * deltaTime;
    }
    // Handle forward movement.
    if (GetAsyncKeyState(unsigned short('W')) & 0x8000)
    {
        m_xPos += std::sin(m_angle) * 5.0f * deltaTime;
        m_yPos += std::cos(m_angle) * 5.0f * deltaTime;
    }
    // Handle backward movement.
    if (GetAsyncKeyState(unsigned short('S')) & 0x8000)
    {
        m_xPos -= std::sin(m_angle) * 5.0f * deltaTime;
        m_yPos -= std::cos(m_angle) * 5.0f * deltaTime;
    }
}
