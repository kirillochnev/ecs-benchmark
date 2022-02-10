#include "components.hpp"

#include "benchmark.hpp"
#include "mustache.hpp"
#include "EnTT.hpp"
#include "open_ecs.hpp"
#include "EntityX.hpp"
#include "OOP.hpp"

#include <map>
#include <fstream>
#include <sstream>

namespace {
    enum Test {
        kUnknown,
        kMustache,
        kEnTT,
        kEntityX,
        kOpenEcs,
        kOopWithPools,
        kOop,
    };

    Test strToTest(const std::string& str) noexcept {
        static const std::map<std::string, Test> map {
                {"mustache", Test::kMustache},
                {"EnTT", Test::kEnTT},
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

int main(int argc, const char** argv) {
    bench::Config config;
    config.create_world.iterations = 1;
    config.create_world.report_type = bench::ReportType::kShort;
    config.update_world.report_type = bench::ReportType::kShort;
    --argc;
    ++argv;
    bool append = true;
    std::string test_name = "";
    readNext(test_name, argc, argv);
    Test test = strToTest(test_name);
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
    switch (test) {

        case kUnknown:
            throw std::runtime_error("Unknown test: [" + test_name + "]");
        case kMustache:
            updatePositionMustache<>(config);
            break;
        case kEnTT:
            updatePositionEnTT<>(config);
            break;
        case kEntityX:
            updatePositionEntityX<>(config);
            break;
        case kOpenEcs:
            updatePositionOpenEcs<>(config);
            break;
        case kOopWithPools:
            updatePositionOOP<true>(config);
            break;
        case kOop:
            updatePositionOOP<false>(config);
            break;
    }
    const uint64_t expected = config.entity_count * config.update_world.iterations;
    if (bench::entities_updated != expected) {
        throw std::runtime_error(test_name + ": invalid updated entities, expected: "
        + std::to_string(expected) + ", actual: " + std::to_string(bench::entities_updated));
    }

//    auto& update_file = std::cout;
    std::fstream create_file{"create." + test_name + ".txt", mode};
    std::fstream update_file{"update." + test_name + ".txt", mode};
    create_file << config.entity_count << " " << create.str();
    update_file << config.entity_count << " " << update.str();

    return 0;
}
