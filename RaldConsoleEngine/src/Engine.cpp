#include "Engine.h"

std::chrono::time_point<std::chrono::system_clock> Engine::m_lastTime;
std::thread Engine::m_bgThread;
float Engine::m_deltaTime;
bool Engine::m_newFrame;
std::mutex Engine::g_m_newFrame;
// std::vector<std::variant<Entity, Player>> Engine::m_entityList;
std::vector<Entity> Engine::m_entityList;
