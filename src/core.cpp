#include "core.h"

using namespace Fac;

static int _calculate_ppm(std::optional<int> const processing_time, std::optional<int> const amount) {
    auto const rpm = processing_time.value_or(0) * amount.value_or(0);
    return rpm == 0 ? 0 : (60 / processing_time.value_or(0)) * amount.value_or(0);
}

void Machine::setRecipe(std::optional<Recipe> const &r) {
    if (r->inputs.size() > _input_slots || r->products.size() > _output_slots) {
        throw std::runtime_error("Recipe does not fit in this machine");
    }

    _active_recipe = r;

    // Note: the order of inputs in the recipe determines the slot they are added to
    for (int i = 0; i < r->inputs.size(); i++) {
        auto const input_stack = getInputStack(i);
        input_stack->clear();
        input_stack->lockResource(r->inputs[i].resource);
    }

    // Note: The order of products in the recipe determines the slot they are added to
    for (int i = 0; i < r->products.size(); i++) {
        auto const output_stack = getOutputStack(i);
        output_stack->clear();
        output_stack->lockResource(r->products[i].resource);
    }
}

std::optional<Recipe> Machine::getRecipe() const {
    return _active_recipe;
}

int Machine::getInputRpm() const {
    auto ppm = 0.0;
    if (_active_recipe.has_value()) {
        auto const r = _active_recipe.value();
        for (int i = 0; i < r.inputs.size(); i++) {
            ppm += _calculate_ppm(r.processing_time_s, r.inputs[i].amount);
        }
        ppm = ppm / r.inputs.size();
    }
    return ppm;
}

// TODO this is a copy of getInputRpm
int Machine::getOutputRpm() const {
    auto ppm = 0.0;
    if (_active_recipe.has_value()) {
        auto const r = _active_recipe.value();
        for (int i = 0; i < r.products.size(); i++) {
            ppm += _calculate_ppm(r.processing_time_s, r.products[i].amount);
        }
        ppm = ppm / r.products.size();
    }
    return ppm;
}

void Machine::update(double const dt) {
    if (!_active_recipe.has_value()) {
        return;
    }

    std::ranges::for_each(_input_connections, [](auto &c) {
        auto const input = c.getInputStack(0);
        auto const buffer = c.getOutputStack(0);
        while (!input->isEmpty() && buffer->canAdd(1, input->getResource())) {
            buffer->addOne(input->getResource());
            input->removeOne();
        }
    });

    if (processing) {
        processing_progress += dt;
    }

    const auto [inputs, products, processing_time_s] = _active_recipe.value();

    if (processing_progress >= processing_time_s * 1000) {
        processing = false;
        processing_progress = 0.0;
        for (int i = 0; i < products.size(); i++) {
            if (auto const stack = getOutputStack(i); stack->isFull()) {
                // TODO what to do when output stack is full?
                // Idea: keep in a processing and add to output stack when it becomes available
            } else {
                stack->addAmount(products[i].amount, products[i].resource);
            }

        }
        return;
    }

    if (!processing && canStartProduction()) {
        for (int i = 0; i < inputs.size(); i++) {
            getInputStack(i)->removeAmount(inputs[i].amount);
        }
        processing_progress = 0.0;
        processing = true;
        return;
    }

}

bool Machine::canStartProduction() const {
    if (processing)
        return false;

    if (!_active_recipe.has_value())
        return false;

    auto const r = _active_recipe.value();
    if (r.processing_time_s == 0)
        return false;

    for (int i = 0; i < r.inputs.size(); i++) {
        if (getInputStack(i)->getAmount() < r.inputs[i].amount)
            return false;
    }

    for (int i = 0; i < r.products.size(); i++) {
        if (!getOutputStack(i)->canAdd(r.products[i].amount, r.products[i].resource))
            return false;
    }

    if (getOutputStack(0)->isFull())
        return false;

    return true;
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
        return;
    }

    if (_active && !_in_transit_stack.empty()) {
        _time_to_next_transfer += dt;
        if (_time_to_next_transfer >= 1000 / _items_per_s) {
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
        return;
    }

    // try to unjam
    if (_jammed || (!_active && !_in_transit_stack.empty())) {
        _time_to_next_transfer = 0.0;
        _active = true;
    }
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
        if (_time_to_next_transfer >= 1000 / _items_per_s) {
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
        return;
    }

    // try to unjam
    if (_jammed || (!_active && !_in_transit_stack.empty())) {
        _time_to_next_transfer = 0.0;
        _active = true;
    }
}

void Merger::update(double dt) {
    if (!_active &&  _in_transit_stack.empty()) {

        auto const ip0 = getInputStack(0);
        auto const ip1 = getInputStack(1);

        if (ip0->isEmpty() && ip1->isEmpty()) {
            return;
        }
        if (merge_from_first_input) {
            if (ip0->isEmpty()) {
                merge_from_first_input = false;
                return;
            } else {
                _in_transit_stack.push_back(ip0->getResource());
                ip0->removeOne();
                merge_from_first_input = false;
                _active = true;
                _time_to_next_transfer = 0.0;
                return;
            }
        } else {
            if (ip1->isEmpty()) {
                merge_from_first_input = true;
                return;
            } else {
                _in_transit_stack.push_back(ip1->getResource());
                ip1->removeOne();
                merge_from_first_input = true;
                _active = true;
                _time_to_next_transfer = 0.0;
                return;
            }
        }
        return;
    }

    if (_active && !_in_transit_stack.empty()) {
        _time_to_next_transfer += dt;
        if (_time_to_next_transfer >= 1000 / _items_per_s) {
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
        return;
    }

    // try to unjam
    if (_jammed || (!_active && !_in_transit_stack.empty())) {
        _time_to_next_transfer = 0.0;
        _active = true;
    }
}

void Extractor::update(double const dt) {
    if (extracting) {
        extraction_progress += dt;
    }

    if (!extracting) {
        // can we start extracting?
        if (getOutputStack(0)->getAmount() <= MAX_STACK_SIZE) {
            extracting = true;
        }
        extraction_progress = 0.0;
        return;
    }

    if (extracting && extraction_progress >= 60 * 1000 / _extraction_speed) {
        getOutputStack(0)->addOne(_res_node->getResource());
        extraction_progress = 0.0;
        extracting = false;
        return;
    }
}
