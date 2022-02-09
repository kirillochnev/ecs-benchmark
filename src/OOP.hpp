#pragma once

#include "benchmark.hpp"
#include "components.hpp"
#include <entityx/entityx.h>


template <bool _UseObjectPool, typename... Unused>
void updatePositionOOP(const bench::Config& config) {
    using namespace bench;
    using Type = typename bench::MovableObject<Unused...>;
    using ObjectToStore = typename std::conditional<_UseObjectPool, Type, std::unique_ptr<Type> >::type;
    std::vector<ObjectToStore> objects;
    BenchMark benchmark;
    benchmark.run([&objects, &config] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            if constexpr(_UseObjectPool) {
                objects.emplace_back();
            } else {
                objects.emplace_back(new Type{});
            }
        }
    }, config.create_world.iterations, [&objects] { objects.clear(); });
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    benchmark.run([&objects] {
        for (auto &object : objects) {
            if constexpr(_UseObjectPool) {
                object.update();
            } else {
                object->update();
            }
        }
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
