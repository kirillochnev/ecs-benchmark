#pragma once

#include <glm/matrix.hpp>
#include <glm/ext.hpp>

#include <array>

namespace bench {
    struct Position {
        glm::vec3 value {0.0f,0.0f, 0.0f};
    };

    struct Velocity {
        float value { 1.0f };
    };

    struct Rotation {
        glm::quat orient;
    };

    template<uint32_t, size_t _Size = 4>
    struct UnusedComponent {
        std::array<std::byte, _Size> data;
    };

    inline glm::vec3 forward(const glm::quat& q) {
        return glm::vec3{
                -2.0f * (q.x * q.z + q.w * q.y),
                -2.0f * (q.y * q.z - q.w * q.x),
                -1.0f + 2.0f * (q.x * q.x + q.y * q.y),
        };
    }


    inline void updatePositionFunction(Position& position, const Velocity& velocity, const Rotation& orientation) noexcept {
        constexpr float dt = 1.0f / 60.0f;
        position.value += dt * velocity.value * forward(orientation.orient);
    }

    template<typename... _Unused>
    struct MovableObject : private _Unused...{

        void update() noexcept {
            updatePositionFunction(position, velocity, rotation);
        }

        Position position;
        Velocity velocity;
        Rotation rotation;
    };

}