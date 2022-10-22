#pragma once

#include "benchmark.hpp"
#include "components.hpp"

#include <mustache/utils/profiler.hpp>

#include <mustache/ecs/ecs.hpp>

#include <random>

struct ComponentToGet {
    float value;
//    uint32_t foo[4];
};
struct HashId {
    auto operator()(mustache::EntityId id) const noexcept {
        std::hash<uint32_t > hash;
        return hash(id.toInt());
    }
};

template<typename T>
void entityGetComponentVsContainer(const bench::Config& config) {
    bench::BenchMark benchmark;
    mustache::World world;
    auto& entities = world.entities();
    T map;

    std::cout << "------------------- Create ---------------------------" << std::endl;
    benchmark.run([&map, &config] {
        map.resize(config.entity_count);
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            map[mustache::EntityId::make(i)].value = i + 0.125f;
        }
    }, config.create_world.iterations, [&map]{
        map.clear();
    });
    benchmark.show(std::cout);
    benchmark.reset();

    benchmark.run([&entities, &config] {
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            entities.begin().assign<ComponentToGet>(i + 0.125f).end();
        }
    }, config.create_world.iterations, [&entities] {
        entities.clear();
        entities.update();
    });
    benchmark.show(std::cout);
    benchmark.reset();


    std::vector<mustache::Entity> entities_arr(config.entity_count);
    const auto version = mustache::EntityVersion::make(0);
    for (uint32_t i = 0; i < config.entity_count; ++i) {
        const auto id = mustache::EntityId::make(i);
        entities_arr[i].reset(id, version, world.id());
        if (!entities.hasComponent<ComponentToGet>(entities_arr[i])) {
            std::cerr << "Invalid entity: " << i << std::endl;
            std::terminate();
        }
    }

    std::random_shuffle(entities_arr.begin(), entities_arr.end());

    std::cout << "------------------- Get ---------------------------" << std::endl;

    benchmark.run([&map, &entities_arr]{
        for (const auto& e : entities_arr) {
            const auto id = e.id();
            const auto& v = map[id];
            if (id.toInt() != static_cast<uint32_t>(v.value)) {
                std::terminate();
            }
        }
    }, config.update_world.iterations, []{});
    benchmark.show(std::cout);
    benchmark.reset();

    benchmark.run([&entities, &entities_arr]{
        for (const auto& e : entities_arr) {
            const auto v = entities.getComponent<const ComponentToGet, mustache::FunctionSafety::kUnsafe>(e);
            if (e.id().toInt() != static_cast<uint32_t>(v->value)) {
                std::terminate();
            }
        }
    }, config.update_world.iterations, []{});
    benchmark.show(std::cout);
    benchmark.reset();

}

void indirectAccessMustache(const bench::Config& config) {
    MUSTACHE_PROFILER_START();
    MUSTACHE_PROFILER_MAIN_THREAD();
    using namespace mustache;
    using namespace bench;

    BenchMark benchmark;
    World world;

    struct Reference {
        Entity value;
    };
    struct Job : public PerEntityJob<Job> {
        void operator()(const Reference& ref, Counter& counter) {
            auto ptr = manager->getComponent<const Counter, FunctionSafety::kUnsafe>(ref.value);
            counter.value += ptr->value;
        }
        EntityManager* manager;
    } job;
    job.manager = &world.entities();

    std::vector<Entity> entities(config.entity_count);

    for (uint32_t i = 0; i < config.entity_count; ++i) {
        entities[i] = world.entities().begin()
                .assign<Counter>(uint8_t (1u))
                .assign<Reference>()
        .end();
    }
    auto rng = std::default_random_engine {777u};
    std::shuffle(entities.begin(), entities.end(), rng);
    world.entities().forEach([&entities](Reference& ref, JobInvocationIndex i) {
       ref.value = entities[i.entity_index.toInt()];
    });

    benchmark.run([&world, &job, config]{
        job.run(world, config.parallel_update ? JobRunMode::kParallel : JobRunMode::kCurrentThread);
    }, config.update_world.iterations, []{});
    benchmark.show(*config.update_world.output, config.update_world.report_type);
    MUSTACHE_PROFILER_DUMP("indirectAccessMustache.prof");
}

template <typename... _Unused>
void updatePositionMustache(const bench::Config& config) {
    MUSTACHE_PROFILER_START();
    MUSTACHE_PROFILER_MAIN_THREAD();

    using namespace mustache;
    using namespace bench;
    BenchMark benchmark;

    struct MoveObjectJob : public PerEntityJob<MoveObjectJob> {
        void operator()(Position& position, const Velocity& velocity/*, const Rotation& orientation*/) const noexcept {
//            updatePositionFunction(position, velocity, orientation);
            updatePositionFunctionShort(position, velocity);
        }
//        void forEachArray(ComponentArraySize count, Position* pos, const Velocity* velocity) const noexcept {
//            const uint32_t size = count.template toInt();
////            constexpr glm::vec<3, Scalar > forward {1, 1, 1};
//            for (uint32_t i = 0; i < size; ++i) {
////                pos[i].value += forward * velocity[i].value;
//                bench::updatePositionFunctionShort(pos[i], velocity[i]);
//            }
//        }

    } move_job;

    World world;
    std::vector<Entity> to_destroy;
    to_destroy.reserve(config.entity_count);
    benchmark.run([&world, &config, &to_destroy] {
        auto& entities = world.entities();
//        auto& main_arch = entities.getArchetype<Position, Velocity, _Unused...>();
        auto& main_arch = entities.getArchetype<Position, Velocity/*, Rotation*/, _Unused...>();
        auto& extra_arch0 = entities.getArchetype<Velocity, Rotation>();
        auto& extra_arch1 = entities.getArchetype<Position, Rotation>();
        auto& extra_arch2 = entities.getArchetype<Position, Velocity>();
        for (uint32_t i = 0; i < config.entity_count; ++i) {
            (void) entities.create(main_arch);
            if (config.create_extra) {
                (void) entities.create(extra_arch0);
                (void) entities.create(extra_arch1);
                (void) entities.create(extra_arch2);
            }
            if (config.remove_half) {
                to_destroy.push_back(entities.create(main_arch));
            }
        }
    }, config.create_world.iterations, [&world, &to_destroy]{ world.entities().clear(); to_destroy.clear();});
    benchmark.show(config.create_world.getOutStream(), config.create_world.report_type);
    benchmark.reset();
    for (auto e : to_destroy) {
        world.entities().destroyNow(e);
    }
    const auto mode = config.parallel_update ? JobRunMode::kParallel : JobRunMode::kCurrentThread;
    move_job.run(world, mode);
    benchmark.run([&move_job, &world, mode] {
        move_job.run(world, mode);
    }, config.update_world.iterations, []{});
    benchmark.show(config.update_world.getOutStream(), config.update_world.report_type);
    benchmark.reset();
    MUSTACHE_PROFILER_DUMP("updatePositionMustache.prof");
}
