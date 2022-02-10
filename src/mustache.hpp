#pragma once

#include "benchmark.hpp"
#include "components.hpp"
#include <mustache/ecs/ecs.hpp>

template <typename... _Unused>
void updatePositionMustache(const bench::Config& config) {
    using namespace mustache;
    using namespace bench;
    BenchMark benchmark;

    struct MoveObjectJob : public PerEntityJob<MoveObjectJob> {
        void operator()(Position& position, const Velocity& velocity, const Rotation& orientation) const {
            updatePositionFunction(position, velocity, orientation);
        }
    } move_job;

    World world;
    std::vector<Entity> to_destroy;
    to_destroy.reserve(config.entity_count);
    benchmark.run([&world, &config, &to_destroy] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            (void) world.entities().create<Position, Velocity, Rotation, _Unused...>();
            if (config.create_extra) {
                (void) world.entities().create<Velocity, Rotation>();
                (void) world.entities().create<Position, Rotation>();
                (void) world.entities().create<Position, Velocity>();
            }
            if (config.remove_half) {
                to_destroy.push_back(world.entities().create<Position, Velocity, Rotation, _Unused...>());
            }
        }
    }, config.create_world.iterations, [&world, &to_destroy]{ world.entities().clear(); to_destroy.clear();});
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();
    for (auto e : to_destroy) {
        world.entities().destroyNow(e);
    }
    benchmark.run([&move_job, &world, config] {
        move_job.run(world, config.parallel_update ? JobRunMode::kParallel : JobRunMode::kCurrentThread);
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
