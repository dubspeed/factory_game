#ifndef CORE_H
#define CORE_H
#include <iostream>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Fac {
    class GameWorldEntity;
    class SimulatedEntity;
    class InputStackProvider;
    class OutputStackProvider;
}

// For custom types like std::optional
namespace nlohmann {
    template<typename T>
    struct adl_serializer<std::optional<T> > {
        static void to_json(json &j, const std::optional<T> &opt) {
            if (opt.has_value()) {
                j = *opt;
            } else {
                j = nullptr;
            }
        }

        static void from_json(const json &j, std::optional<T> &opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };

    template<typename T>
    struct adl_serializer<std::vector<std::shared_ptr<T> > > {
        static void to_json(json &j, const std::vector<std::shared_ptr<T> > &stacks) {
            j = json::array();
            for (const auto &stack: stacks) {
                json sj = *stack;
                j.push_back(sj);
            }
        }

        static void from_json(const json &j, std::vector<std::shared_ptr<T> > &stacks) {
            for (const auto &stack: j) {
                stacks.push_back(std::make_shared<T>(stack.get<T>()));
            }
        }
    };
}

namespace Fac {
    static constexpr int MAX_STACK_SIZE = 100;

    enum class Resource {
        Copper_Ore,
        Iron_Ore,
        Iron_Ingots,
        Iron_Plates,
        Iron_Rods,
        Copper_Ingots,
        Copper_Wire,
        Copper_Cable,
        Iron_Screw,
        Reinforced_Iron_Plate,
    };

    enum class ResourceQuality {
        Normal,
        Pure,
        Impure,
    };

    struct Recipe {
        struct Ingredient {
            Resource resource;
            int amount;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Ingredient, resource, amount)
        };

        std::vector<Ingredient> inputs;
        std::vector<Ingredient> products;
        int processing_time_s;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Recipe, inputs, products, processing_time_s)
    };

    inline Recipe recipe_iron_ingots = {
        .inputs = {{Resource::Iron_Ore, 1}},
        .products = {{Resource::Iron_Ingots, 1}},
        .processing_time_s = 2
    };

    inline Recipe recipe_iron_plates = {
        .inputs = {{Resource::Iron_Ingots, 3}},
        .products = {{Resource::Iron_Plates, 2}},
        .processing_time_s = 6
    };

    inline Recipe recipe_iron_rods = {
        .inputs = {{Resource::Iron_Ingots, 1}},
        .products = {{Resource::Iron_Rods, 1}},
        .processing_time_s = 4
    };

    inline int generate_id() {
        static int id = 0;
        return id++;
    }

    class SerializableEntity {
    public:
        [[nodiscard]] virtual int getId() const = 0;
    };

    class GameWorldEntity : public SerializableEntity {
    public:
        virtual ~GameWorldEntity() = default;
    };

    class SimulatedEntity : public GameWorldEntity {
    public:
        virtual void update(double dt) = 0;
    };


    // inline SingleRecipe srecipe_iron_ingots{Resource::Iron_Ore, 1, Resource::Iron_Ingots, 1, 2};
    // inline SingleRecipe srecipe_iron_plates{Resource::Iron_Ingots, 3, Resource::Iron_Plates, 2, 6};
    // inline SingleRecipe srecipe_iron_rods{Resource::Iron_Ingots, 1, Resource::Iron_Rods, 1, 4};
    // inline SingleRecipe srecipe_copper_ingots{Resource::Copper_Ore, 1, Resource::Copper_Ingots, 1, 2};
    // inline SingleRecipe srecipe_copper_wire{Resource::Copper_Ingots, 1, Resource::Copper_Wire, 2, 4};
    // inline SingleRecipe srecipe_copper_cable{Resource::Copper_Wire, 2, Resource::Copper_Cable, 1, 2};
    // inline SingleRecipe srecipe_iron_screw{Resource::Iron_Rods, 1, Resource::Iron_Screw, 4, 6};


    struct Stack : public SerializableEntity {
        void clear();

        bool isEmpty() const;

        bool isFull() const { return _amount == MAX_STACK_SIZE; }

        bool canAdd(int amount, Resource const &r) const;

        int getAmount() const { return _amount; }
        Resource getResource() const { return resource.value(); }
        void lockResource(Resource const &r) { resource = r; }

        bool removeOne();

        bool removeAmount(int amount);

        bool addOne(Resource const &r);

        bool addAmount(int amount, Resource const &r);

        int getId() const override { return _id; }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Stack, _id, _amount, resource)

    private:
        int _amount = 0;
        int _id = generate_id();
        std::optional<Resource> resource = std::nullopt;
    };


    // Base interface for anything that can provide a stack
    class IStackAccessor {
    public:
        virtual ~IStackAccessor() = default;

        [[nodiscard]] virtual std::shared_ptr<Stack> getStack(int slot) const = 0;
    };

    // Specific interface for output-capable components
    class IOutputProvider : public IStackAccessor {
    public:
        [[nodiscard]] virtual std::shared_ptr<Stack> getOutputStack(int slot) const = 0;
    };

    // Specific interface for input-capable components
    class IInputProvider : public IStackAccessor {
    public:
        [[nodiscard]] virtual std::shared_ptr<Stack> getInputStack(int slot) const = 0;

        // Connect this input to another component's output
        virtual void connectInput(int inputSlot,
                                  std::shared_ptr<GameWorldEntity> sourceEntity,
                                  int sourceOutputSlot) = 0;
    };


    class OutputStackProvider : public IOutputProvider {
    protected:
        std::vector<std::shared_ptr<Stack> > _output_stacks;

    public:
        explicit OutputStackProvider(int nr_of_outputs) {
            _output_stacks.resize(nr_of_outputs);
            for (int i = 0; i < nr_of_outputs; i++) {
                _output_stacks[i] = std::make_shared<Stack>();
            }
        }

        [[nodiscard]] std::shared_ptr<Stack> getStack(int const slot) const override {
            return getOutputStack(slot);
        }

        [[nodiscard]] std::shared_ptr<Stack> getOutputStack(int const slot) const override {
            return _output_stacks.at(slot);
        }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(OutputStackProvider, _output_stacks)
    };

    struct InputConnection {
        std::weak_ptr<IOutputProvider> source;
        int sourceId = 0;
        int sourceOutputSlot = 0;
        std::shared_ptr<Stack> cachedStack;
    };

    void to_json(json &j, const InputConnection &r);

    void from_json(const json &j, InputConnection &r);

    class InputStackProvider : public IInputProvider {
    protected:
        std::vector<InputConnection> _input_connections;

    public:
        explicit InputStackProvider(int nr_of_inputs) {
            _input_connections.resize(nr_of_inputs);
            for (int i = 0; i < nr_of_inputs; i++) {
                _input_connections[i].cachedStack = std::make_shared<Stack>();
            }
        }

        [[nodiscard]] std::shared_ptr<Stack> getStack(int slot) const override {
            return getInputStack(slot);
        }

        // if the connection has a link, it returns the linked OutputStack, otherwise it returns the cached stack
        [[nodiscard]] std::shared_ptr<Stack> getInputStack(int slot) const override {
            const auto &connection = _input_connections.at(slot);
            if (auto source = connection.source.lock()) {
                return source->getOutputStack(connection.sourceOutputSlot);
            }
            return connection.cachedStack;
        }

        void connectInput(int inputSlot,
                          std::shared_ptr<GameWorldEntity> sourceEntity,
                          int sourceOutputSlot) override {
            auto &connection = _input_connections.at(inputSlot);
            if (auto const provider = std::dynamic_pointer_cast<IOutputProvider>(sourceEntity)) {
                connection.source = provider;
            } else {
                throw std::runtime_error("Cannot connect input to non-output provider");
            }
            connection.sourceId = sourceEntity->getId();
            connection.sourceOutputSlot = sourceOutputSlot;
            connection.cachedStack = nullptr;
        }

        void reconnectLinks(std::function<std::shared_ptr<GameWorldEntity>(int)> const &getEntityById) {
            for (int inputSlot = 0; inputSlot < _input_connections.size(); inputSlot++) {
                auto const &connection = _input_connections.at(inputSlot);
                connectInput(inputSlot, getEntityById(connection.sourceId), connection.sourceOutputSlot);
            }
        }

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(InputStackProvider, _input_connections)
    };


    class ResourceNode : public GameWorldEntity {
    public:
        ResourceNode() = default;

        void setResource(Resource const &r, ResourceQuality const quality) {
            _active_resource = r;
            _quality = quality;
        }

        [[nodiscard]] Resource getResource() const { return _active_resource; }
        [[nodiscard]] ResourceQuality getQuality() const { return _quality; }

        int getId() const override { return _id; }

    private:
        int _id = generate_id();
        Resource _active_resource;
        ResourceQuality _quality;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(ResourceNode, _active_resource, _quality)
    };

    // Resource extractions speeds are 30 / 60 / 120 for impure, normal, pure
    class ResourceExtractor : public SimulatedEntity, public OutputStackProvider {
    public:
        double extraction_progress = 0.0;
        bool extracting = false;

        ResourceExtractor(): OutputStackProvider(1) {
        };

        void setResourceNode(std::shared_ptr<ResourceNode> const &depot);

        [[nodiscard]] std::shared_ptr<ResourceNode> getResourceNode() const { return _res_node; };

        void update(double dt) override;

        int getId() const override { return _id; }

    private:
        int _id = generate_id();
        std::shared_ptr<ResourceNode> _res_node;
    };

    /**
    * SingleMachine
    * -------------
    */
    class SingleMachine : public SimulatedEntity, public InputStackProvider, public OutputStackProvider {
        friend void from_json(const json &, SingleMachine &);

    public:
        static constexpr int fixed_input_slots = 0; // TODO allow multiple input slots for bigger machines
        static constexpr int fixed_output_slots = 0; // TODO allow multiple input slots for bigger machines
        double processing_progress = 0.0;
        bool processing = false;

        SingleMachine(): InputStackProvider(1), OutputStackProvider(1) {
        };

        void setRecipe(std::optional<Recipe> const &r);

        [[nodiscard]] std::optional<Recipe> getRecipe() const;

        int getInputRpm() const;

        int getOutputRpm() const;

        void update(double dt) override;

        void _checkAndStartProcessing();

        void _checkAndFinishProduction();

        bool _canStartProduction() const;

        int getId() const override { return _id; }

    private:
        int _id = generate_id();
        std::optional<Recipe> _active_recipe;
    };

    void to_json(json &j, const SingleMachine &r);

    void from_json(const json &j, SingleMachine &r);


    /**
     * ItemMover
     * ---------
     * parent class for Belts, Mergers and Splitters
     */
    class ItemMover : public InputStackProvider, public OutputStackProvider,
                      public std::enable_shared_from_this<ItemMover> {
    public:
        int getItemsPerSecond() const { return _items_per_s; }

        bool getActive() const { return _active; }
        bool getJammed() const { return _jammed; }

        ItemMover(int const input_nodes, int const output_nodes,
                  int const items_per_s): InputStackProvider(input_nodes), OutputStackProvider(output_nodes),
                                          _items_per_s(items_per_s) {
        }

        std::vector<Resource> _in_transit_stack;

    protected:
        double _time_to_next_transfer = 0.0;
        int _items_per_s = 0;
        bool _active = false;
        bool _jammed = false;
    };


    class Belt : public SimulatedEntity, public ItemMover {
        friend void from_json(const json &j, Belt &r);

    public:
        Belt(int const items_per_s = 1): ItemMover(1, 1, items_per_s) {
        }

        void update(double dt) override;

        int getId() const override { return _id; }

    protected:
        int _id = generate_id();
    };

    void to_json(json &j, const Belt &r);

    void from_json(const json &j, Belt &r);

    // TODO maybe the splitter can be a belt with a special case
    // TODO also consider the merger, which is the opposite of the splitter
    // a splitter has one input and two or more outputs
    class Splitter : public SimulatedEntity, public ItemMover {
        friend void from_json(const json &j, Splitter &r);

    public:
        Splitter(): ItemMover(1, 2, 1) {
        }

        void update(double dt) override;

        int getId() const override { return _id; }

    private:
        int _id = generate_id();
        bool split_to_first_output = true;
    };

    void to_json(json &j, const Splitter &r);

    void from_json(const json &j, Splitter &r);

    class Merger : public SimulatedEntity, public ItemMover {
        friend void from_json(const json &j, Merger &r);

    public:
        Merger(): ItemMover(2, 1, 1) {
        }

        void update(double dt) override;

        int getId() const override { return _id; }

    private:
        int _id = generate_id();
        bool merge_from_first_input = true;
    };

    void to_json(json &j, const Merger &r);

    void from_json(const json &j, Merger &r);
}

#endif //CORE_H
