#pragma once

//#define GLM_FORCE_SSE2
#define GLM_FORCE_ALIGNED
//
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <array>
#include <iostream>

namespace bench {
    using Scalar = uint16_t;
    struct Position {
        glm::vec<3, Scalar> value {0, 0, 0};
    };

    struct Velocity {
        Scalar value { 1 };
    };

    struct Rotation {
        glm::qua<Scalar> orient {static_cast<Scalar>(0.0), static_cast<Scalar>(0.0),
                                 static_cast<Scalar>(0.0), static_cast<Scalar>(0.0)};
    };
    struct Counter {
        uint8_t value = 0u;
    };

    template<uint32_t, size_t _Size = 4>
    struct UnusedComponent {
        std::array<std::byte, _Size> data;
    };

    inline glm::vec<3, Scalar> forward(const glm::qua<Scalar>& q) {
//        std::cout << q.x << "; " << q.y << "; " << q.z << "; " << q.w << std::endl;
        return glm::vec<3, Scalar> {
                -2.0f * (q.x * q.z + q.w * q.y),
                -2.0f * (q.y * q.z - q.w * q.x),
                -1.0f + 2.0f * (q.x * q.x + q.y * q.y),
        };
    }

    extern uint64_t entities_updated;
    inline void updatePositionFunction(Position& position, const Velocity& velocity, const Rotation& orientation) noexcept {
//        constexpr Scalar dt = 1.0f / 60.0f;
//        position.value += dt * velocity.value * forward(orientation.orient);
//        ++entities_updated;
    }

    inline void updatePositionFunctionShort(Position& position, const Velocity& velocity) noexcept {
//        std::cout << "AAAA" << std::endl;
        static const glm::vec<3, Scalar > forward {1, 1, 1};
        position.value += forward * velocity.value;
        ++entities_updated;

    }

    struct IUpdatable {
        virtual ~IUpdatable() = default;
        virtual void update() noexcept {}
    };
    template<typename... _Unused>
    struct MovableObject : public IUpdatable, private _Unused...{

        void update() noexcept override {
            updatePositionFunctionShort(position, velocity/*, rotation*/);
//            updatePositionFunction(position, velocity, rotation);
        }

        Position position;
        Velocity velocity;
//        Rotation rotation;
    };

    struct WithoutPosition : public IUpdatable {
        void update() noexcept override {}

        Velocity velocity;
        Rotation rotation;
    };

    struct WithoutVelocity : public IUpdatable {
        void update() noexcept override {}

        Position position;
        Rotation rotation;
    };

    struct WithoutRotation : public IUpdatable {
        void update() noexcept override {}

        Position position;
        Velocity velocity;
    };

    template <typename... ARGS>
    void unused(ARGS&&...) {

    }
}
