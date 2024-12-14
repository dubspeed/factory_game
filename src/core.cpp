#include "core.h"

#include <functional>
using namespace Fac;

static int _calculate_rpm(std::optional<int> const processing_time, std::optional<int> const amount) {
    auto const rpm = processing_time.value_or(0) * amount.value_or(0);
    return rpm == 0 ? 0 : (60 / processing_time.value_or(0)) * amount.value_or(0);
}

void SingleMachine::setRecipe(std::optional<Recipe> const &r) {
    if (r->inputs.size() > fixed_input_slots + 1) {
        throw std::runtime_error("This machine only has one input slot");
    }
    if (r->products.size() > fixed_output_slots + 1) {
        throw std::runtime_error("This machine only has one output slot");
    }
    _active_recipe = r;
    auto const stack_in = getInputStack(0);
    auto const stack_out = getOutputStack(0);

    if (stack_in == nullptr || stack_out == nullptr) {
        return;
    }

    stack_in->clear();
    stack_in->lockResource(r->inputs[fixed_input_slots].resource);
    stack_out->clear();
    stack_out->lockResource(r->inputs[fixed_input_slots].resource);
}

std::optional<Recipe> SingleMachine::getRecipe() const {
    return _active_recipe;
}

int SingleMachine::getInputRpm() const {
    if (_active_recipe.has_value()) {
        auto const r = _active_recipe.value();
        return _calculate_rpm(r.processing_time_s, r.inputs[fixed_input_slots].amount);
    }
    return 0;
}

int SingleMachine::getOutputRpm() const {
    if (_active_recipe.has_value()) {
        auto const r = _active_recipe.value();
        return _calculate_rpm(r.processing_time_s, r.products[fixed_output_slots].amount);
    }
    return 0;
}

void SingleMachine::update(double const dt) {
    if (!_active_recipe.has_value()) {
        return;
    }
    processing_progress += dt;
    _checkAndFinishProduction();
    _checkAndStartProcessing();
}

void SingleMachine::_checkAndStartProcessing() {
    if (!processing && _canStartProduction()) {
        auto const r = _active_recipe.value();
        getInputStack(0)->removeAmount(r.inputs[fixed_input_slots].amount);
        processing_progress = 0.0;
        processing = true;
    }
}

void SingleMachine::_checkAndFinishProduction() {
    if (!processing)
        return;

    auto const r = _active_recipe.value();

    if (processing_progress >= r.processing_time_s * 1000) {
        processing = false;
        if (getOutputStack(0)->getAmount() == MAX_STACK_SIZE) {
            // TODO what to do when output stack is full?
            // Idea: keep in a processing and add to output stack when it becomes available
        } else {
            getOutputStack(0)->addAmount(r.products[fixed_output_slots].amount, r.products[fixed_output_slots].resource);
        }
    }
}

bool SingleMachine::_canStartProduction() const {
    if (processing)
        return false;

    if (!_active_recipe.has_value())
        return false;

    auto const r = _active_recipe.value();
    if (r.processing_time_s == 0)
        return false;

    if (!getInputStack(0) || !getOutputStack(0))
        return false;

    if (getInputStack(0)->getAmount() < r.inputs[fixed_input_slots].amount)
        return false;

    if (getOutputStack(0)->isFull())
        return false;

    return true;
}

void Fac::to_json(json &j, const SingleMachine &r) {
    auto id = r.getId();
    auto m = json{
        {"id", id}, {"recipe", r.getRecipe()}, {"processing", r.processing}, {"progress", r.processing_progress}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    j["machine"] = m;
}

void Fac::from_json(const json &j, SingleMachine &r) {
    r.setRecipe(j.at("machine").at("recipe").get<Recipe>());
    r.processing = j.at("machine").at("processing").get<bool>();
    r.processing_progress = j.at("machine").at("progress").get<double>();
    r._id = j.at("machine").at("id").get<int>();
    r._input_connections = j.at("machine").at("input").at("_input_connections").get<std::vector<InputConnection>>();
    r._output_stacks = j.at("machine").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack>>>();
}

void Fac::to_json(json &j, const InputConnection &r) {
    if (r.source.lock() == nullptr) {
        j = json{{"stack", *r.cachedStack}};
    } else {
        j = json{{"linkedSourceMachineId", r.sourceId}, {"linkedSourceMachineOutputSlot", r.sourceOutputSlot}};
    }
}

void Fac::from_json(const json &j, InputConnection &r) {
    if (j.contains("stack")) {
        r.cachedStack = std::make_shared<Stack>(j.at("stack").get<Stack>());
    } else {
        r.sourceId = j.at("linkedSourceMachineId").get<int>();
        r.sourceOutputSlot = j.at("linkedSourceMachineOutputSlot").get<int>();
    }
}

// TODO Belt, Splitter and Merger share implementation
void Fac::to_json(json &j, const Belt &r) {
    auto id = r.getId();
    auto m = json{
        {"id", id}, {"itemsPerSecond", r.getItemsPerSecond()}, {"active", r.getActive()}, {"jammed", r.getJammed()}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["inTransitStack"] = r._in_transit_stack;
    j["itemMover"] = m;
}

void Fac::from_json(const json &j, Belt &r) {
    r._items_per_s = j.at("itemMover").at("itemsPerSecond").get<int>();
    r._active = j.at("itemMover").at("active").get<bool>();
    r._in_transit_stack = j.at("itemMover").at("inTransitStack").get<std::vector<Resource>>();
    r._jammed = j.at("itemMover").at("jammed").get<bool>();
    r._id = j.at("itemMover").at("id").get<int>();
    r._input_connections = j.at("itemMover").at("input").at("_input_connections").get<std::vector<InputConnection>>();
    r._output_stacks = j.at("itemMover").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack>>>();
}

void Fac::to_json(json &j, const Merger &r) {
    auto id = r.getId();
    auto m = json{
            {"id", id}, {"itemsPerSecond", r.getItemsPerSecond()}, {"active", r.getActive()}, {"jammed", r.getJammed()}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["inTransitStack"] = r._in_transit_stack;
    j["itemMover"] = m;
}

void Fac::from_json(const json &j, Merger &r) {
    r._items_per_s = j.at("itemMover").at("itemsPerSecond").get<int>();
    r._active = j.at("itemMover").at("active").get<bool>();
    r._in_transit_stack = j.at("itemMover").at("inTransitStack").get<std::vector<Resource>>();
    r._jammed = j.at("itemMover").at("jammed").get<bool>();
    r._id = j.at("itemMover").at("id").get<int>();
    r._input_connections = j.at("itemMover").at("input").at("_input_connections").get<std::vector<InputConnection>>();
    r._output_stacks = j.at("itemMover").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack>>>();
}

void Fac::to_json(json &j, const Splitter &r) {
    auto id = r.getId();
    auto m = json{
                {"id", id}, {"itemsPerSecond", r.getItemsPerSecond()}, {"active", r.getActive()}, {"jammed", r.getJammed()}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["inTransitStack"] = r._in_transit_stack;
    j["itemMover"] = m;
}

void Fac::from_json(const json &j, Splitter &r) {
    r._items_per_s = j.at("itemMover").at("itemsPerSecond").get<int>();
    r._active = j.at("itemMover").at("active").get<bool>();
    r._in_transit_stack = j.at("itemMover").at("inTransitStack").get<std::vector<Resource>>();
    r._jammed = j.at("itemMover").at("jammed").get<bool>();
    r._id = j.at("itemMover").at("id").get<int>();
    r._input_connections = j.at("itemMover").at("input").at("_input_connections").get<std::vector<InputConnection>>();
    r._output_stacks = j.at("itemMover").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack>>>();
}



void Belt::update(double dt) {
    // Transfer items from one machine to another
    // if the from machine has items to give and the to machine has space to receive

    // it takes an item from the 'from' machine at the beginning of the transfer
    // and adds it to the 'to' machine at the end of the transfer


    if (!_active && !getInputStack(0)->isEmpty() && _in_transit_stack.empty()) {
        _active = true;
        _in_transit_stack.push_back(getInputStack(0)->getResource());
        _time_to_next_transfer = 0.0;
        getInputStack(0)->removeOne();
    }

    if (_active && !_in_transit_stack.empty()) {
        _time_to_next_transfer += dt;
        if (_time_to_next_transfer >= _items_per_s * 1000) {
            _time_to_next_transfer = 0.0;
            if (getOutputStack(0)->canAdd(1, _in_transit_stack[0])) {
                // TODO if output stack resource is not set, we should set it here
                getOutputStack(0)->addOne(_in_transit_stack[0]);
                _in_transit_stack.pop_back();
                _jammed = false;
            } else {
                _jammed = true;
            }
            _active = false;
        }
    }

    // TODO: check if jammed and try to unjam
}

void Splitter::update(double dt) {
    // the splitter always sees valid stacks, since it does not know
    // if something is connected to a slot. It then sees the default empty stacks


    auto add_to_stack = [this](int const slot_nr) {
        _in_transit_stack.pop_back();
        getOutputStack(slot_nr)->addOne(_in_transit_stack[0]);
        _jammed = false;
        split_to_first_output = !split_to_first_output;
    };

    // TODO this block is a copy from belt!
    if (!_active && !getInputStack(0)->isEmpty() && _in_transit_stack.empty()) {
        _active = true;
        _in_transit_stack.push_back(getInputStack(0)->getResource());
        getInputStack(0)->removeOne();
        _time_to_next_transfer = 0.0;
        return;
    }

    if (_active && !_in_transit_stack.empty()) {
        _time_to_next_transfer += dt;
        if (_time_to_next_transfer >= _items_per_s * 1000) {
            _time_to_next_transfer = 0.0;

            auto can_add_to_first = getOutputStack(0)->canAdd(1, _in_transit_stack[0]);
            auto can_add_to_second = getOutputStack(1)->canAdd(1, _in_transit_stack[0]);

            if (split_to_first_output) {
                if (can_add_to_first) {
                    add_to_stack(0);
                } else if (can_add_to_second) {
                    add_to_stack(1);
                } else {
                    _jammed = true;
                }
            } else {
                if (can_add_to_second) {
                    add_to_stack(1);
                } else if (can_add_to_first) {
                    add_to_stack(0);
                } else {
                    _jammed = true;
                }
            }
            _active = false;
        }
    }

    // problem is that is now has to split everything into it's internal stack
    // -> can I limit it to e.g. 0 or 1 maybe?
}

void Merger::update(double dt) {
    if (!_active && (!getInputStack(0)->isEmpty() || !getInputStack(1)->isEmpty()) && _in_transit_stack.empty()) {
        _active = true;
        merge_from_first_input = merge_from_first_input && !getInputStack(0)->isEmpty();
        if (merge_from_first_input) {
            _in_transit_stack.push_back(getInputStack(0)->getResource());
            getInputStack(0)->removeOne();
        } else {
            _in_transit_stack.push_back(getInputStack(1)->getResource());
            getInputStack(1)->removeOne();
        }
        _time_to_next_transfer = 0.0;
        merge_from_first_input = !merge_from_first_input;
        return;
    }

    if (_active && !_in_transit_stack.empty()) {
        _time_to_next_transfer += dt;
        if (_time_to_next_transfer >= _items_per_s * 1000) {
            _time_to_next_transfer = 0.0;
            if (getOutputStack(0)->canAdd(1, _in_transit_stack[0])) {
                getOutputStack(0)->addOne(_in_transit_stack[0]);
                _in_transit_stack.pop_back();
                _jammed = false;
            } else {
                _jammed = true;
            }
            _active = false;
        }
    }
}

void Stack::clear() {
    _amount = 0;
    resource = std::nullopt;
}

bool Stack::isEmpty() const {
    return _amount == 0;
}

bool Stack::canAdd(int const amount, Resource const &r) const {
    return _amount + amount <= MAX_STACK_SIZE && (resource == r || resource == std::nullopt);
}

bool Stack::removeOne() {
    return removeAmount(1);
}

bool Stack::removeAmount(int const amount) {
    if (_amount >= amount) {
        _amount -= amount;
        if (_amount == 0) {
            clear();
        }
        return true;
    }
    return false;
}

bool Stack::addOne(Resource const &r) {
    return addAmount(1, r);
}

bool Stack::addAmount(int const amount, Resource const &r) {
    if (_amount == 0) {
        resource = r;
    }
    if (_amount + amount <= MAX_STACK_SIZE && resource == r) {
        _amount += amount;
        return true;
    }
    return false;
}



void ResourceExtractor::update(double const dt) {
    if (!extracting) {
        // can we start extracting?
        if (getOutputStack(0)->getAmount() <= MAX_STACK_SIZE) {
            extracting = true;
        }
        extraction_progress = 0.0;
    }
    extraction_progress += dt;

    if (extracting && extraction_progress >= 1000) {
        getOutputStack(0)->addOne(_res_node->getResource());
        extraction_progress = 0.0;
        extracting = false; // done extracting
    }
}
