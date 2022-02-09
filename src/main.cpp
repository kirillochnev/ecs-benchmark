#include "components.hpp"

#include "benchmark.hpp"
#include "mustache.hpp"
#include "EnTT.hpp"
#include "open_ecs.hpp"
#include "EntityX.hpp"
#include "OOP.hpp"

#include <map>
#include <fstream>

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

int main(int argc, const char** argv) {
    bench::Config config;
    config.create_world.iterations = 10;
    config.create_world.report_type = bench::ReportType::kShort;
    config.update_world.report_type = bench::ReportType::kShort;
    --argc;
    ++argv;
    bool append = true;
    std::string test_name;
    readNext(test_name, argc, argv);
    Test test = strToTest(test_name);
    readNext(config.entity_count, argc, argv);
    readNext(config.parallel_update, argc, argv);
    readNext(config.create_world.iterations, argc, argv);
    readNext(append, argc, argv);
    const auto mode = append ? std::ios_base::out | std::ios_base::app : std::ios_base::out;
    std::fstream create{"create." + test_name + ".txt", mode};
    std::fstream update{"update." + test_name + ".txt", mode};

    config.create_world.output = &create;
    config.update_world.output = &update;

    create << config.entity_count << " ";
    update << config.entity_count << " ";
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

    return 0;
}
