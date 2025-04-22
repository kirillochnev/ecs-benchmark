#include <mustache/ecs/ecs.hpp>

#ifndef ENTT_ID_TYPE
#include <cstdint>
#define ENTT_ID_TYPE std::uint64_t
#endif
#include <entt/entt.hpp>

#include <iostream>
#include <algorithm>
#include <random>
#include <glm/matrix.hpp>
#include <glm/ext.hpp>

namespace {
        bool apply_shuffle = true;
        struct Position {
            uint8_t value  = std::rand() % 255;
        };

        struct Velocity {
            uint8_t value = std::rand() % 255;
        };

        struct ComponentToCheck {
            static constexpr uint32_t Magic = 0XDEADBEEF;
            uint32_t value = Magic;
        };
        struct Position_v3 {
            glm::vec3 value;
        };

        struct Velocity_v3 {
            glm::vec3 value;
        };

        struct Rotation_quat {
            glm::quat value;
        };

        template<size_t _Tag, size_t _Size = 64>
        struct Dummy {
            std::array<std::byte, _Size> data;
        };


        template<typename _F>
        double benchmark(_F&& function) {
            using clock = std::chrono::high_resolution_clock;
            auto begin = clock ::now();
            uint64_t count = 0;
            const auto bench_time = std::chrono::microseconds(1000000);
            for (;(clock::now() - begin) < bench_time; ++count) {
                function();
            }

            begin = clock ::now();
            for (uint64_t i = 0; i < count; ++i) {
                function();
            }
            const auto end = clock::now();
            const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
            return static_cast<double >(dt) / static_cast<double >(count);
        }

        inline __attribute__((always_inline)) void benchmarkFunction(const Velocity& vel, Position& position) {
            position.value += vel.value;
        }

        template<typename _F>
        double getNanoseconds(_F&& function, uint32_t count) {
            using clock = std::chrono::high_resolution_clock;
            auto begin = clock ::now();
            function();
            const auto end = clock::now();
            const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
            return static_cast<double >(dt) / static_cast<double >(count);
        }

        template<typename... _Components>
        auto createEntity(mustache::EntityManager& ecs) {
            return ecs.create<_Components...>();
        }

        template<typename... _Components>
        auto createEntity(entt::registry& ecs) {
            auto entity = ecs.create();
            (ecs.emplace<_Components>(entity), ...);
            return entity;
        }



        template<typename... _Components, typename _ECS>
        auto createEntities(_ECS& ecs, uint32_t count) {

            assert(count > 0);
            auto first_entity = createEntity<_Components...>(ecs);
            using EntityType = decltype(first_entity);
            std::vector<EntityType > result(count);
            result[0] = first_entity;
            for (uint32_t i = 1; i < count; ++i) {
                result[i] = createEntity<_Components...>(ecs);
            }
            if (apply_shuffle) {
                // Фиксированный seed
                const unsigned seed = 42;
                std::mt19937 gen(seed);

                // Перемешивание массива с фиксированным seed
                std::shuffle(result.begin(), result.end(), gen);
            }
            return result;
        }
        template<typename... _Components>
        void getComponents(mustache::World& ecs, mustache::Entity entity) {
            (ecs.entities().getComponent<_Components, mustache::FunctionSafety::kUnsafe>(entity), ...);
        }
        template<typename... _Components>
        void getComponents(entt::registry& ecs, entt::entity entity) {
            (void)(ecs.get<_Components>(entity), ...);
        }
}

template<typename T>
using ConstComponent = std::conditional_t<
        std::is_trivially_copyable_v<T> && sizeof(T) <= 2 * sizeof(void*),
        const T,
        const T&
>;

template<typename T>
using Component = std::conditional_t<
        std::is_const_v<T>,
        ConstComponent<T>,
        T&
>;

void mustacheBench(uint64_t count) {
    mustache::World world;
    using clock = std::chrono::high_resolution_clock;
    auto begin_create = clock ::now();

    auto arr = createEntities<Position, Velocity, ComponentToCheck>(world.entities(), count);
//    auto arr = createEntities<Position_v3, Velocity_v3, Rotation_quat>(world.entities(), count);
    const auto end_create = clock ::now();
    const auto dt_create = std::chrono::duration_cast<std::chrono::nanoseconds>(end_create - begin_create).count();
    std::cout << "mustache create time per entity: " << static_cast<double >(dt_create) / count << std::endl;

    auto mode = mustache::JobRunMode::kParallel;
    const auto function = [&world] {
        world.entities().forEach([](Velocity vel, Position& position) {
            position.value += vel.value;
        });
//        world.entities().forEach([](Velocity_v3 vel, Position_v3& pos, Rotation_quat& rot) {
//            // Смещение позиции
//            pos.value += vel.value;
//
//            // Простейшее "вращение" — поворот вектора на угол вектора скорости (упрощённо)
//            glm::vec3 axis = glm::normalize(vel.value);
//            float angle = glm::length(vel.value) * 0.01f;
//            glm::quat delta = glm::angleAxis(angle, axis);
//            rot.value = glm::normalize(delta * rot.value); // нормализуем, чтобы избежать ошибок накопления
//        }, mode);
    };
    auto ns = benchmark(function);
    std::cout << "mustache update time per entity mt: " << static_cast<double >(ns) / count << "ns" << std::endl;
//    mode = mustache::JobRunMode::kSingleThread;
//    ns = benchmark(function);
//    std::cout << "mustache update time per entity st: " << static_cast<double >(ns) / count << "ns" << std::endl;

    const auto get_dt = benchmark([&arr, &world]{
        for (auto entity : arr) {
            constexpr auto safety = mustache::FunctionSafety::kSafe;
            if (auto ptr = world.entities().getComponent<const ComponentToCheck, safety>(entity); ptr && ptr->value != ComponentToCheck::Magic) {
                std::exit(1);
            }
//            static_assert(fits_in_min_cache_lines<mustache::EntityLocationInWorld>::recommended_alignment == alignof(mustache::EntityLocationInWorld));
//            getComponents<const Position, const Velocity>(world, entity);
        }
    });
    std::cout << "Get const components time: " << get_dt / count << "ns" << std::endl;

    const auto destroy_dt = getNanoseconds([&arr, &world]{
        for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
            world.entities().destroyNow(*it);
        }
    }, count);
    std::cout << "Destroy time: " << destroy_dt << "ns" << std::endl;
}

void enttBench(uint64_t count) {
    entt::registry registry;
    auto group = registry.group<const Velocity, Position>();
    auto group2 = registry.group<const ComponentToCheck>();
//    auto group = registry.group<const Velocity_v3, Position_v3, Rotation_quat>();
    using clock = std::chrono::high_resolution_clock;
    auto begin_create = clock ::now();
    auto arr = createEntities<Position, Velocity, ComponentToCheck>(registry, count);
//    auto arr = createEntities<Velocity_v3, Position_v3, Rotation_quat>(registry, count);
    const auto end_create = clock ::now();
    const auto dt_create = std::chrono::duration_cast<std::chrono::nanoseconds>(end_create - begin_create).count();
    std::cout << "EnTT create time per entity: " << static_cast<double >(dt_create) / count << std::endl;

    const auto function = [&group] {
        group.each([](const Velocity& vel, Position& position) {
            position.value += vel.value;
        });
    };

//    const auto function = [&registry, &group] {
//        group.each([](Velocity_v3 vel, Position_v3& pos, Rotation_quat& rot) {
//             Смещение позиции
//            pos.value += vel.value;

//             Простейшее "вращение" — поворот вектора на угол вектора скорости (упрощённо)
//            glm::vec3 axis = glm::normalize(vel.value);
//            float angle = glm::length(vel.value) * 0.01f;
//            glm::quat delta = glm::angleAxis(angle, axis);
//            rot.value = glm::normalize(delta * rot.value); // нормализуем, чтобы избежать ошибок накопления
//        });
//    };
    const auto ns = benchmark(function);
    std::cout << "EnTT update time per entity: " << static_cast<double >(ns) / count  << "ns" << std::endl;
    const auto get_dt = benchmark([&arr, &registry]{
        for (auto entity : arr) {
            if (auto ptr = registry.try_get<const ComponentToCheck>(entity); ptr && ptr->value != ComponentToCheck::Magic) {
                std::exit(1);
            }
//            getComponents<const Position, const Velocity>(registry, entity);
        }
    });
    std::cout << "Get const time: " << get_dt / count << "ns" << std::endl;

    const auto destroy_dt = getNanoseconds([&arr, &registry]{
        registry.destroy(arr.begin(), arr.end());
    }, count);
    std::cout << "Destroy time: " << destroy_dt << "ns" << std::endl;
}


template<typename T>
struct ComponentAssignedEvent {
        mustache::Entity entity;
        T* component;
};

template<typename T>
struct ComponentRemovedEvent {
        mustache::Entity entity;
        T* component;
};

void eventTest() {
    struct ComponentWithEvents {
        using This = ComponentWithEvents;
        static void afterAssign(mustache::Entity entity, This& component, mustache::World& world) {
            world.events().post(ComponentAssignedEvent<This>{entity, &component});
        }
        static void beforeRemove(mustache::Entity entity, This& component, mustache::World& world) {
            world.events().post(ComponentRemovedEvent<This>{entity, &component});
        }
    };

    mustache::World world;
    auto assign_subscriber = world.events().subscribe<ComponentAssignedEvent<ComponentWithEvents>>([](const auto& event) {
        std::cout << "Component assigned to entity: " << event.entity.id().toInt() << std::endl;
    });
    auto remove_subscriber = world.events().subscribe<ComponentRemovedEvent<ComponentWithEvents>>([](const auto& event) {
        std::cout << "Component removed to entity: " << event.entity.id().toInt() << std::endl;
    });

    for (uint32_t i = 0; i < 128; ++i) {
        (void)world.entities().create<ComponentWithEvents>();
    }
    world.entities().clear();
}

//const bool mustache::OptionalVersionStorage::is_version_control_enabled{true};



int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Use EcsBenchmark mustache|EnTT [enitity count]" << std::endl;
        return 1;
    }
    const std::string ecs = argv[1];
//    eventTest();
//    if (true) return 0;
    uint64_t count = 1 << 20;
    if (argc > 2) {
        count = std::atoi(argv[2]);
    }
    if (argc > 3) {
        apply_shuffle = argv[3] == std::string ("true");
    }
    if (ecs == "EnTT") {
        enttBench(count);
    }
    if (ecs == "mustache") {
        MUSTACHE_PROFILER_START();
        mustacheBench(count);
        MUSTACHE_PROFILER_DUMP("result.prof");
    }
}
