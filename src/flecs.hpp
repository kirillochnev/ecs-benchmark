#pragma once

#include "components.hpp"
#include "benchmark.hpp"

#include <flecs.h>

#include <random>
#include <memory>
#include <thread>

void indirectAccessFlecs(const bench::Config& config) {
    using namespace bench;

    BenchMark benchmark;
    flecs::world ecs;


    struct Reference {
        flecs::entity value;
//        uint32_t counter = 0u;
    };

    if (config.parallel_update) {
        ecs.set_threads(std::thread::hardware_concurrency());
    }

    ecs.system<Reference, Counter>()
            .multi_threaded(config.parallel_update)
            .each([](const Reference& ref, Counter& counter) {
                auto ptr = ref.value.get<bench::Counter>();
//                if (ptr) {
                counter.value += ptr->value;
//                }
            });


    std::vector<flecs::entity> entities(config.entity_count);

    for (uint32_t i = 0; i < config.entity_count; ++i) {
        entities[i] = ecs.entity()
                .set(Counter{1})
                .add<Reference>();
    }
    const auto cpy = entities;
    auto rng = std::default_random_engine {777u};
    std::shuffle(entities.begin(), entities.end(), rng);

    for (uint32_t i = 0; i < cpy.size(); ++i) {
        cpy[i].get_mut<Reference>()->value = entities[i];
    }

    benchmark.run([&ecs]{
        ecs.progress();
    }, config.update_world.iterations, []{});
    benchmark.show(*config.update_world.output, config.update_world.report_type);
}

template <typename... _Unused>
void updatePositionFlecs(const bench::Config& config) {
    flecs::world ecs;
    if (config.parallel_update) {
        ecs.set_threads(std::thread::hardware_concurrency());
    }
    ecs.system<bench::Position, bench::Velocity/*, bench::Rotation*/>()
            .multi_threaded(config.parallel_update)
//            .each([&ecs](bench::Position& pos, const bench::Velocity& vel/*, const bench::Rotation& rot*/) {
//                updatePositionFunctionShort(pos, vel);
//            });
            .template iter([](flecs::iter& it, bench::Position* pos, bench::Velocity* vel){
                for (auto i : it) {
                    bench::updatePositionFunctionShort(pos[i], vel[i]);
                }
            });

    bench::BenchMark benchmark;

    benchmark.run([&ecs, &config] {
//        auto prefab = ecs.prefab().set(bench::Position{}).set(bench::Velocity{});
//        auto type = ecs.type("Base").add<bench::Position>().add<bench::Velocity>();
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            ecs.template entity().set(bench::Position{}).set(bench::Velocity{});//.set(bench::Rotation{});
        }
    }, 1/*config.create_world.iterations*/, []{});

    benchmark.show(*config.create_world.output, config.create_world.report_type);
    benchmark.reset();
    ecs.progress();
    benchmark.run([&ecs]{
        ecs.progress();
    }, config.update_world.iterations, []{});

    benchmark.show(*config.update_world.output, config.update_world.report_type);
    benchmark.reset();
}
