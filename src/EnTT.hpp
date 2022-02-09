#pragma once

#include "benchmark.hpp"
#include "components.hpp"
#include <entt/entt.hpp>

template <typename... _Unused>
void updatePositionEnTT(const bench::Config& config) {
    using namespace entt;
    using namespace bench;
    BenchMark benchmark;

    registry registry;
    benchmark.run([&registry, &config] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            entity entity = registry.create();
            registry.emplace<Position>(entity);
            registry.emplace<Velocity>(entity);
            registry.emplace<Rotation>(entity);
            unused(registry.emplace<_Unused>(entity)...);
        }
    }, config.create_world.iterations, [&registry]{ registry.clear<>(); });
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    benchmark.run([&registry]{
        registry.view<Position, const Velocity, const Rotation>().each(&updatePositionFunction);
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
