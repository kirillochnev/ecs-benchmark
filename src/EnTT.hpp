#pragma once

#define ENTT_ID_TYPE uint64_t
#include "benchmark.hpp"
#include "components.hpp"
#include <entt/entt.hpp>
#include <execution>

template <typename... _Unused>
void updatePositionEnTT(const bench::Config& config) {
    using namespace entt;
    using namespace bench;
    BenchMark benchmark;

    static_assert(std::is_same<ENTT_ID_TYPE, uint64_t>::value);
    registry registry;
    std::vector<entity> to_destroy;
    to_destroy.reserve(config.entity_count);
    benchmark.run([&registry, &config, &to_destroy] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            entity entity = registry.create();
            registry.emplace<Position>(entity);
            registry.emplace<Velocity>(entity);
//            registry.emplace<Rotation>(entity);
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


//    auto group = registry.template group<Position, Velocity, Rotation>();
    auto group = registry.template group<Position, Velocity>();
    benchmark.run([&group, &config] {
        if (config.parallel_update) {
//            std::for_each(std::execution::par_unseq, group.begin(), group.end(), [&group](auto entity) {
//                updatePositionFunction(
//                        group.template get<Position>(entity),
//                    group.template get<Velocity>(entity),
//                  group.template get<Rotation>(entity));
////                decltype(auto) components = group.template get<Position>(entity) ;//[entity];
////                updatePositionFunction(std::get<0>(components), std::get<1>(components), std::get<2>(components));
//            });
        } else {
            group.each(&updatePositionFunctionShort);
        }
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
