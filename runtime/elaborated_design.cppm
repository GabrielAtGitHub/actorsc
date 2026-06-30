module;
#include <memory>
#include <thread>
#include <utility>

export module runtime.elaborated_design;

export import factory.abstract_factory;
export import factory.ioc_container;
export import scheduler.simulation_cycle;
export import scheduler.thread_pool;

export class Runtime {
public:
    explicit Runtime(
        std::unique_ptr<IoCContainer> container,
        std::size_t num_threads = std::thread::hardware_concurrency())
        : container_(std::move(container))
        , pool_(std::make_unique<ThreadPool>(num_threads ? num_threads : 1))
        , factory_(*container_, *pool_)
        , engine_(*pool_)
    {
        design_ = factory_.elaborate();
        engine_.set_sensitivity_map(design_.sensitivity_map);
    }

    SimulationStats run(StimulusFn has_more_stimuli) {
        return engine_.run(design_.actors, design_.signals,
                           std::move(has_more_stimuli));
    }

    const ElaboratedDesign& design() const { return design_; }

private:
    std::unique_ptr<IoCContainer> container_;
    std::unique_ptr<ThreadPool> pool_;
    AbstractFactory factory_;
    SimulationCycleEngine engine_;
    ElaboratedDesign design_;
};
