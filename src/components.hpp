#pragma once

#include <glm/matrix.hpp>
#include <glm/ext.hpp>

#include <array>

namespace bench {
    using Scalar = float;
    struct Position {
        glm::vec<3, Scalar> value {0.0f,0.0f, 0.0f};
    };

    struct Velocity {
        Scalar value { 1.0f };
    };

    struct Rotation {
        glm::qua<Scalar> orient {0.0f, 0.0f, 0.0f, 1.0f};
    };

    template<uint32_t, size_t _Size = 4>
    struct UnusedComponent {
        std::array<std::byte, _Size> data;
    };

    inline glm::vec<3, Scalar> forward(const glm::qua<Scalar>& q) {
        return glm::vec<3, Scalar> {
                -2.0f * (q.x * q.z + q.w * q.y),
                -2.0f * (q.y * q.z - q.w * q.x),
                -1.0f + 2.0f * (q.x * q.x + q.y * q.y),
        };
    }


    inline void updatePositionFunction(Position& position, const Velocity& velocity, const Rotation& orientation) {
        constexpr Scalar dt = 1.0f / 60.0f;
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


    template <typename... ARGS>
    void unused(ARGS&&...) {

    }
}
