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
    std::vector<entity> to_destroy;
    to_destroy.reserve(config.entity_count);
    benchmark.run([&registry, &config, &to_destroy] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            entity entity = registry.create();
            registry.emplace<Position>(entity);
            registry.emplace<Velocity>(entity);
            registry.emplace<Rotation>(entity);
            unused(registry.emplace<_Unused>(entity)...);

            if (config.remove_half) {
                entity = registry.create();
                registry.emplace<Position>(entity);
                registry.emplace<Velocity>(entity);
                registry.emplace<Rotation>(entity);
                unused(registry.emplace<_Unused>(entity)...);
                to_destroy.push_back(entity);
            }

            if (config.create_extra) {
                entity = registry.create();
                registry.emplace<Velocity>(entity);
                registry.emplace<Rotation>(entity);

                entity = registry.create();
                registry.emplace<Position>(entity);
                registry.emplace<Rotation>(entity);

                entity = registry.create();
                registry.emplace<Position>(entity);
                registry.emplace<Velocity>(entity);
            }
        }
    }, config.create_world.iterations, [&registry, &to_destroy]{ registry.clear<>(); to_destroy.clear();});
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    for (auto e : to_destroy) {
        registry.destroy(e);
    }


    auto group = registry.template view<Position, Velocity, Rotation>();
    benchmark.run([&group]{
        group.each(&updatePositionFunction);
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
