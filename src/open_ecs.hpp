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
    std::vector<Entity> to_destroy;
    to_destroy.reserve(config.entity_count);
    benchmark.run([&entity_manager, &config, &to_destroy] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            auto entity = entity_manager.create();
            entity.add<bench::Position>();
            entity.add<bench::Velocity>();
//            entity.add<bench::Rotation>();
            if (config.create_extra) {
                entity = entity_manager.create();
                entity.add<bench::Velocity>();
                entity.add<bench::Rotation>();

                entity = entity_manager.create();
                entity.add<bench::Position>();
                entity.add<bench::Rotation>();

                entity = entity_manager.create();
                entity.add<bench::Position>();
                entity.add<bench::Velocity>();
            }
            if (config.remove_half) {
                entity = entity_manager.create();
                entity.add<bench::Position>();
                entity.add<bench::Velocity>();
                entity.add<bench::Rotation>();
                to_destroy.push_back(entity);
            }
        }
    }, config.create_world.iterations, [&entity_manager, &to_destroy] {
        // WTF? How to delete Entities?
        entity_manager.~EntityManager();
        to_destroy.clear();
        new (&entity_manager) EntityManager{};
    });
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    for (auto e : to_destroy) {
        e.destroy();
    }
    benchmark.run([&entity_manager] {
        entity_manager.with([] (bench::Position& pos, bench::Velocity& vel/*, bench::Rotation& rot*/) {
//            bench::updatePositionFunction(pos, vel, rot);
            bench::updatePositionFunctionShort(pos, vel/*, rot*/);
        });
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
