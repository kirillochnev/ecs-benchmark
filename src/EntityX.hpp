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
    std::vector<Entity> to_destroy;
    to_destroy.reserve(config.entity_count);
    benchmark.run([&entity_manager, &config, &to_destroy] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            auto entity = entity_manager.create();
            entity.assign<Position>();
            entity.assign<Velocity>();
            entity.assign<Rotation>();
            unused (entity_manager.template assign<_Unused>(entity.id())...);

            if (config.create_extra) {
                entity = entity_manager.create();
                entity.assign<Velocity>();
                entity.assign<Rotation>();


                entity = entity_manager.create();
                entity.assign<Position>();
                entity.assign<Rotation>();


                entity = entity_manager.create();
                entity.assign<Position>();
                entity.assign<Velocity>();
            }
            if (config.remove_half) {
                entity = entity_manager.create();
                entity.assign<Position>();
                entity.assign<Velocity>();
                entity.assign<Rotation>();
                unused (entity_manager.template assign<_Unused>(entity.id())...);
                to_destroy.push_back(entity);
            }
        }
    }, config.create_world.iterations, [&entity_manager, &to_destroy]{ entity_manager.reset(); to_destroy.clear();});
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    for (auto e : to_destroy) {
        e.destroy();
    }
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
