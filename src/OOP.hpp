#pragma once

#include "benchmark.hpp"
#include "components.hpp"
#include <entityx/entityx.h>


template <bool _UseObjectPool, typename... Unused>
void updatePositionOOP(const bench::Config& config) {
    using namespace bench;
    using Type = typename bench::MovableObject<Unused...>;
    using ObjectToStore = typename std::conditional<_UseObjectPool, Type, std::unique_ptr<IUpdatable> >::type;
    std::vector<ObjectToStore> objects;
    BenchMark benchmark;
    std::vector<size_t> to_remove;
    to_remove.reserve(config.entity_count);
    benchmark.run([&objects, &config, &to_remove] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            if constexpr(_UseObjectPool) {
                objects.emplace_back();
            } else {
                objects.emplace_back(new Type{});
                if (config.create_extra) {
                    objects.template emplace_back(new WithoutPosition{});
                    objects.template emplace_back(new WithoutVelocity{});
                    objects.template emplace_back(new WithoutRotation{});
                }
                if (config.remove_half) {
                    to_remove.push_back(objects.size());
                    objects.emplace_back(new Type{});
                }
            }
        }
    }, config.create_world.iterations, [&objects, &to_remove] { objects.clear(); to_remove.clear();});
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();

    if constexpr(!_UseObjectPool) {
        for (auto i : to_remove) {
            objects[i].reset();
        }
    }
    benchmark.run([&objects] {
        for (auto &object : objects) {
            if constexpr(_UseObjectPool) {
                object.update();
            } else {
                if (object) {
                    object->update();
                }
            }
        }
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
}
