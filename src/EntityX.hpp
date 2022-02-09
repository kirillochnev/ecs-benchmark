#pragma once

#include "benchmark.hpp"
#include "components.hpp"
#include <entityx/entityx.h>

template <typename... _Unused>
void updatePositionEntityX(const bench::Config& config) {
    using namespace entityx;
    using namespace bench;
    BenchMark benchmark;

    EventManager event_manager;
    EntityManager entity_manager{event_manager};
    benchmark.run([&entity_manager, &config] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            auto entity = entity_manager.create();
            entity.assign<Position>();
            entity.assign<Velocity>();
            entity.assign<Rotation>();
            unused (entity_manager.template assign<_Unused>(entity.id())...);
        }
    }, config.create_world.iterations, [&entity_manager]{ entity_manager.reset();});
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    benchmark.run([&entity_manager]{
        ComponentHandle<Position> position;
        ComponentHandle<Velocity> velocity;
        ComponentHandle<Rotation> rotation;

        for (auto entity : entity_manager.entities_with_components(position, velocity, rotation)) {
            updatePositionFunction(*position, *velocity, *rotation);
        }
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
