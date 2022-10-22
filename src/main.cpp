#include "components.hpp"

#include "benchmark.hpp"
#include "mustache.hpp"
#include "EnTT.hpp"
#include "open_ecs.hpp"
#include "EntityX.hpp"
#include "OOP.hpp"
#include "flecs.hpp"
#include "mustache/utils/timer.hpp"

#include <map>
#include <fstream>
#include <sstream>

namespace {
    enum Test {
        kUnknown,
        kMustache,
        kEnTT,
        kFlecs,
        kEntityX,
        kOpenEcs,
        kOopWithPools,
        kOop,
    };

    Test strToTest(const std::string& str) noexcept {
        static const std::map<std::string, Test> map {
                {"mustache", Test::kMustache},
                {"default", Test::kMustache},
                {"EnTT", Test::kEnTT},
                {"flecs", Test::kFlecs},
                {"EntityX", Test::kEntityX},
                {"OpenEcs", Test::kOpenEcs},
                {"OopWithPools", Test::kOopWithPools},
                {"Oop", Test::kOop}
        };
        const auto find_res = map.find(str);
        if (find_res != map.end()) {
            return find_res->second;
        }
        return Test::kUnknown;
    }
}
template <typename T>
void readNext(T& out, int& argc, const char**& argv) {
    if (argc < 1) {
        return;
    }
    if constexpr (std::is_same<T, std::string>::value) {
        out = *argv;
    }
    if constexpr (std::is_same<T, Test>::value) {
        out = strToTest(*argv);
    }

    if constexpr (std::is_same<T, uint32_t>::value || std::is_same<T, int32_t>::value) {
        out = static_cast<T>(std::atoi(*argv));
    }

    if constexpr (std::is_same<T, bool>::value) {
        std::string str = *argv;
        std::transform(str.begin(), str.end(), str.begin(), [](auto c){ return std::tolower(c); });
        if (str == "true") {
            out = true;
        } else if (str == "false") {
            out = false;
        } else {
            throw std::runtime_error("Invalid bool value: " + std::string(*argv));
        }
    }
    --argc;
    ++argv;
}
uint64_t bench::entities_updated = 0u;


template<typename... ARGS>
void runTest(std::string test_name, bench::Config& config) {
    Test test = strToTest(test_name);
    switch (test) {

        case kUnknown:
            throw std::runtime_error("Unknown test: [" + test_name + "]");
        case kMustache:
//            indirectAccessMustache(config);
            updatePositionMustache<ARGS...>(config);
            break;
        case kFlecs:
//            indirectAccessFlecs(config);
            updatePositionFlecs<ARGS...>(config);
            break;
        case kEnTT:
            updatePositionEnTT<ARGS...>(config);
            break;
        case kEntityX:
            updatePositionEntityX<ARGS...>(config);
            break;
        case kOpenEcs:
            updatePositionOpenEcs<ARGS...>(config);
            break;
        case kOopWithPools:
            updatePositionOOP<true, ARGS...>(config);
            break;
        case kOop:
            updatePositionOOP<false, ARGS...>(config);
            break;
    }
}

void testCreateMustache(uint32_t count) {
    std::vector<mustache::Entity> out(count);
    mustache::World world;
    auto& entities = world.entities();
    auto& archetype = entities.getArchetype<bench::Position, bench::Rotation, bench::Velocity>();
    bench::BenchMark bench_mark;
    mustache::Timer timer;
    timer.reset();
    for (auto& entity: out) {
//        entity = entities.begin().assign<bench::Position>().assign<bench::Rotation>().assign<bench::Velocity>().end();
//        entity = entities.create<bench::Position, bench::Rotation, bench::Velocity>();
        entity = entities.create(archetype);
    }
    std::cout << timer.elapsed() * 1000 << "ms" << std::endl;
}

void testCreateEnTT(uint32_t count) {
    std::vector<entt::entity> out(count);
    entt::registry registry;
    mustache::Timer timer;
    timer.reset();
    registry.create(out.begin(), out.end());
    for (auto &entity: out) {
        registry.emplace<bench::Position>(entity);
        registry.emplace<bench::Rotation>(entity);
        registry.emplace<bench::Velocity>(entity);
    }
    std::cout << timer.elapsed() * 1000 << "ms" << std::endl;
}

int main(int argc, const char** argv) {
//    for (uint32_t i = 0; i < 10; ++i) {
//        testCreateMustache(2000000);
//        testCreateEnTT(2000000);
//        std::cout << "-----------------------------" << std::endl;
//    }
//    return 0;
    bench::Config config;
    config.parallel_update = false;
    config.create_world.iterations = 1;
    config.update_world.iterations = 100;
    config.create_world.report_type = bench::ReportType::kDefault;
    config.update_world.report_type = bench::ReportType::kDefault;
    --argc;
    ++argv;
    bool append = true;
    std::string test_name = "";
    readNext(test_name, argc, argv);
    readNext(config.entity_count, argc, argv);
    readNext(config.create_extra, argc, argv);
    readNext(config.remove_half, argc, argv);
    readNext(config.parallel_update, argc, argv);
    readNext(config.create_world.iterations, argc, argv);
    readNext(append, argc, argv);
    const auto mode = append ? std::ios_base::out | std::ios_base::app : std::ios_base::out;

    std::stringstream create;
    std::stringstream update;
    config.create_world.output = &create;
    config.update_world.output = &update;

    bench::entities_updated = 0u;

//    using Map = std::map<mustache::EntityId, ComponentToGet>;
//    using Map = std::unordered_map<mustache::EntityId, ComponentToGet, HashId>;
//    using Map = mustache::ArrayWrapper<ComponentToGet, mustache::EntityId, false>;
//    entityGetComponentVsContainer<Map >(config);
//    runTest<bench::UnusedComponent<0, sizeof(glm::mat4)> >(test_name, config);
    runTest<>(test_name, config);

//    const uint64_t expected = config.entity_count * config.update_world.iterations;
//    if (bench::entities_updated != expected) {
//        throw std::runtime_error(test_name + ": invalid updated entities, expected: "
//        + std::to_string(expected) + ", actual: " + std::to_string(bench::entities_updated));
//    }

//    auto& update_file = std::cout;
    constexpr bool write_to_file = false;
    if (write_to_file) {
        std::fstream create_file{"create." + test_name + ".txt", mode};
        std::fstream update_file{"update." + test_name + ".txt", mode};
        create_file << config.entity_count << " " << create.str();
        update_file << config.entity_count << " " << update.str();
    } else {
        std::cout << config.entity_count << " " << create.str() << std::endl;
        std::cout << config.entity_count << " " << update.str();
    }
    return 0;
}
