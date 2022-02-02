#include <mustache/ecs/ecs.hpp>
#include <entt/entt.hpp>
#include <entityx/entityx.h>

#include "components.hpp"

#include <numeric>
#include <iostream>

constexpr uint32_t entity_count = 10000000;
constexpr uint32_t create_iterations = 10;
constexpr uint32_t run_iterations = 100;

class BenchMark {
public:

    template<typename _F, typename _BeforeRun>
    void run(_F&& function, uint32_t iterations = 1, _BeforeRun&& before_run = []{}) {
        dt_array_.reserve(dt_array_.size() + iterations);
        for (uint32_t i = 0; i < iterations; ++i) {
            before_run();
            const auto begin = Clock::now();
            function();
            const auto end = Clock::now();
            dt_array_.push_back(1000.0 * getSeconds(end - begin));
        }
    }

    void printTimes(std::ostream& out) const noexcept {
        for (const auto dt : dt_array_) {
            out << dt << " ";
        }
        out.flush();
    }

    void reset() noexcept {
        dt_array_.clear();
    }

    void show(std::ostream& out) noexcept {
        if(dt_array_.empty()) {
            return;
        }
        if(dt_array_.size() < 2) {
            out<<"Time: " << dt_array_.front() << "ms" << std::endl;
            return;
        }
        const auto sum = std::accumulate(dt_array_.begin(), dt_array_.end(), 0.0);
        std::sort(dt_array_.begin(), dt_array_.end());
        auto med = dt_array_[dt_array_.size() / 2];
        if(dt_array_.size() % 2 == 0) {
            med = (med + dt_array_[dt_array_.size() / 2 - 1]) * 0.5;
        }
        const auto avr = sum / static_cast<double>(dt_array_.size());
        double variance = 0.0;
        for (auto x : dt_array_) {
            variance += (avr - x) * (avr - x) / static_cast<double>(dt_array_.size());
        }
        out << "Call count: " << dt_array_.size() << ", Arv: " << avr << "ms, med: " << med
            << "ms, min:" << dt_array_.front() << "ms, max: "
            << dt_array_.back() << "ms, variance: " << variance << ", sigma: " << sqrt(variance) << std::endl;
    }
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    template<typename T>
    static double getSeconds(const T& pDur) noexcept {
        constexpr double Microseconds2SecondConvertK = 0.000001;
        const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(pDur).count();
        return static_cast<double>(microseconds) * Microseconds2SecondConvertK;
    }

    std::vector<double> dt_array_;

};

template <typename... Unused>
void updatePositionOOP(std::ostream& create, std::ostream& iterate) {
    std::vector<bench::MovableObject<Unused...> > objects;
    BenchMark benchmark;
    benchmark.run([&objects] {
        for (uint32_t i = 0; i < entity_count; ++i) {
            objects.emplace_back();
        }
    }, create_iterations, [&objects]{
        objects.clear();
    });
    benchmark.show(create);
    benchmark.reset();

    benchmark.run([&objects] {
        for (auto &object : objects) {
            object.update();
        }
    }, run_iterations, []{});

    benchmark.show(iterate);
}

template <typename... _Unused>
void updatePositionMustache(std::ostream& create, std::ostream& iterate) {
    struct MoveObjectJob : public mustache::PerEntityJob<MoveObjectJob> {
        void operator()(bench::Position& position, const bench::Velocity& velocity,
                        const bench::Rotation& orientation) const {
            updatePositionFunction(position, velocity, orientation);
        }
    };
    MoveObjectJob move_job;

    BenchMark benchmark;

    mustache::World world { mustache::WorldId::make(0)};
    auto& entities = world.entities();
    auto& arch = entities.getArchetype<bench::Position, bench::Velocity, bench::Rotation, _Unused...>();

    benchmark.run([&arch, &entities] {
        for (uint32_t i = 0; i < entity_count; ++i) {
            (void) entities.create(arch);
        }
    }, create_iterations, [&entities]{
        entities.clear();
    });
    benchmark.show(create);
    benchmark.reset();

    benchmark.run([&move_job, &world]{
        move_job.run(world, mustache::JobRunMode::kCurrentThread);
    }, run_iterations, []{});

    benchmark.show(create);
}

template <typename... ARGS>
void unused(ARGS&&...) {

}

template <typename... _Unused>
void updatePositionEnTT(std::ostream& create, std::ostream& iterate) {
    BenchMark benchmark;
    entt::registry registry;
    benchmark.run([&registry] {
        for (uint32_t i = 0; i < entity_count; ++i) {
            entt::entity entity = registry.create();
            registry.emplace<bench::Position>(entity);
            registry.emplace<bench::Velocity>(entity);
            registry.emplace<bench::Rotation>(entity);
            unused(registry.emplace<_Unused>(entity)...);
        }
    }, create_iterations, [&registry]{
        registry.clear<>();
    });
    benchmark.show(create);
    benchmark.reset();

    benchmark.run([&registry]{
        registry.view<bench::Position, const bench::Velocity, const bench::Rotation>().each(&bench::updatePositionFunction);
    }, run_iterations, []{});

    benchmark.show(create);
}

template <typename... _Unused>
void updatePositionEntityX(std::ostream& create, std::ostream& iterate) {
    BenchMark benchmark;
    entityx::EventManager event_manager;
    entityx::EntityManager entity_manager{event_manager};
    benchmark.run([&entity_manager] {
        for (uint32_t i = 0; i < entity_count; ++i) {
            auto entity = entity_manager.create();
            entity.assign<bench::Position>();
            entity.assign<bench::Velocity>();
            entity.assign<bench::Rotation>();
            unused (entity_manager.template assign<_Unused>(entity.id())...);
        }
    }, create_iterations, [&entity_manager]{
        entity_manager.reset();
    });
    benchmark.show(create);
    benchmark.reset();

    benchmark.run([&entity_manager]{
        entityx::ComponentHandle<bench::Position> position;
        entityx::ComponentHandle<bench::Velocity> velocity;
        entityx::ComponentHandle<bench::Rotation> rotation;

        for (auto entity : entity_manager.entities_with_components(position, velocity, rotation)) {
            bench::updatePositionFunction(*position, *velocity, *rotation);
        }
    }, run_iterations, []{});

    benchmark.show(create);
}

template <typename... _Unused>
void runAll() {
    const std::string names[] {
        mustache::type_name<_Unused>()...,
        std::string{}
    };
    std::string name = "";

    for (uint32_t i = 0; i < sizeof...(_Unused); ++i) {
        if (i > 0) {
            name += ", ";
        }
        name += names[i];
    }

    std::cout << "\n------------- unused components: " << name << std::endl;
    std::cout << "OOP-style:" << std::endl;
    updatePositionOOP<_Unused...>(std::cout, std::cout);

    std::cout << "Mustache:" << std::endl;
    updatePositionMustache<_Unused...>(std::cout, std::cout);

    std::cout << "EnTT:" << std::endl;
    updatePositionEnTT<_Unused...>(std::cout, std::cout);

    std::cout << "EntityX:" << std::endl;
    updatePositionEntityX<_Unused...>(std::cout, std::cout);
}

int main() {
    runAll();
    runAll<bench::UnusedComponent<0, sizeof(glm::vec3)> >();
    runAll<bench::UnusedComponent<0, sizeof(glm::mat4)> >();
}
