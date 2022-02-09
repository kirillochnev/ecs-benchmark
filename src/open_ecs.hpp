#pragma once

#include <ecs.h>
#include "benchmark.hpp"
#include "components.hpp"

template <typename... _Unused>
void updatePositionOpenEcs(const bench::Config& config) {
    using namespace ecs;
    using namespace bench;
    BenchMark benchmark;

    EntityManager entity_manager;
    benchmark.run([&entity_manager, &config] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            auto entity = entity_manager.create();
            entity.add<bench::Position>();
            entity.add<bench::Velocity>();
            entity.add<bench::Rotation>();
        }
    }, config.create_world.iterations, [&entity_manager] {
        // WTF? How to delete Entities?
        entity_manager.~EntityManager();
        new (&entity_manager) EntityManager{};
    });
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    benchmark.run([&entity_manager] {
        entity_manager.with([] (bench::Position& pos, bench::Velocity& vel, bench::Rotation& rot) {
            bench::updatePositionFunction(pos, vel, rot);
        });
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
