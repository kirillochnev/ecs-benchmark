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
    benchmark.run([&world, &config] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            (void) world.entities().create<Position, Velocity, Rotation, _Unused...>();
        }
    }, config.create_world.iterations, [&world]{ world.entities().clear(); });
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    benchmark.run([&move_job, &world, config] {
        move_job.run(world, config.parallel_update ? JobRunMode::kParallel : JobRunMode::kCurrentThread);
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
