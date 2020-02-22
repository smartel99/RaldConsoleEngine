#pragma once


#include <cmath>
#include <vector>

class Entity
{
public:
    Entity()
    {
        m_entities.emplace_back(this);
        m_id = m_entities.end() - 1;
    }
    Entity(float xPos = 0.0f,
           float yPos = 0.0f,
           float angle = 0.0f,
           float speed = 0.0f) :
        m_xPos(xPos), m_yPos(yPos), m_angle(angle), m_speed(speed)
    {
        m_entities.emplace_back(this);
        m_id = m_entities.end() - 1;
    }
    ~Entity()
    {
        m_entities.erase(m_id);
    }

    virtual void Render()
    {
    }

    /**
     * @brief   Called each frame,
     *          updates the position of the entity.
     * @param   None
     * @retval  None
     */
    virtual void OnUpdate();

    virtual void Destroy()
    {
        this->~Entity();
    }

    inline const float XPos() const
    {
        return m_xPos;
    }
    inline void XPos(const float x)
    {
        m_xPos = x;
    }

    inline const float YPos() const
    {
        return m_yPos;
    }
    inline void YPos(const float y)
    {
        m_yPos = y;
    }

    inline const float Angle() const
    {
        return m_angle;
    }
    inline void Angle(const float angle)
    {
        m_angle = angle;
    }

    inline const float Speed() const
    {
        return m_speed;
    }
    inline void Speed(const float speed)
    {
        m_speed = speed;
    }


protected:
    float m_xPos = 0.0f;
    float m_yPos = 0.0f;
    float m_angle = 0.0f;

    float m_speed = 0.0f;
    std::vector<Entity*>::iterator m_id = m_entities.end();

    static std::vector<Entity*> m_entities;
};

