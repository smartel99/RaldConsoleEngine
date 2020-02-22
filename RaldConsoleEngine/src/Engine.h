#pragma once
#include "src/Entity.h"
#include "src/Player.h"

#include <cassert>
#include <chrono>
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

static bool isRunning = false;


class Engine
{
public:

    static void Begin()
    {
        assert(isRunning == false);
        isRunning = true;

        m_bgThread = std::thread(BackgroundTask);
    }

    static void End()
    {
        assert(isRunning == true);
        assert(m_bgThread.joinable());
        isRunning = false;
        m_bgThread.join();
    }

    inline static float DeltaTime()
    {
        return m_deltaTime;
    }

    inline static bool SyncFrame()
    {
        std::lock_guard<std::mutex> guard(g_m_newFrame);
        bool r = m_newFrame;
        m_newFrame = false;
        return r;
    }

//     inline static 

private:
    // Don't allow instantiation.
    Engine() = delete;

    static bool m_newFrame;
    static std::mutex g_m_newFrame;
    static float m_deltaTime;
    static std::chrono::time_point<std::chrono::system_clock> m_lastTime;
    static std::thread m_bgThread;
    //static std::vector<std::variant<Entity, Player>> m_entityList;
    static std::vector<Entity> m_entityList;

    static void BackgroundTask()
    {
        while (isRunning == true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::lock_guard<std::mutex> guard(g_m_newFrame);
            m_newFrame = true;
            UpdateDeltaTime();
            DoOnUpdate();
        }
    }

    static void UpdateDeltaTime()
    {
        auto tp2 = std::chrono::system_clock::now();

        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - m_lastTime;
        m_lastTime = tp2;
        m_deltaTime = elapsedTime.count();
    }

    static void DoOnUpdate()
    {
        for (auto entity : m_entityList)
        {
            entity.OnUpdate();
//             switch (entity.index())
//             {
//                 case 0:
//                     std::get<0>(entity).OnUpdate();
//                 case 1:
//                     std::get<1>(entity).OnUpdate();
//             }
//             std::visit([](auto& e)
//                        {
//                            e.OnUpdate();
//                        },
//                        entity);
        }
    }
};