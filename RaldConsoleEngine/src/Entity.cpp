/**
 * @file   D:\dev\RaldConsoleEngine\RaldConsoleEngine\src\Entity.cpp
 * @author Samuel Martel
 * @date   2020/02/21
 *
 * @brief  Source for the Entity module. */

#include "Entity.h"
#include "src/Engine.h"
std::vector<Entity*> Entity::m_entities;

void Entity::OnUpdate()
{
    m_xPos += std::sin(m_angle) * m_speed * Engine::DeltaTime();
    m_yPos += std::cos(m_angle) * m_speed * Engine::DeltaTime();
}
