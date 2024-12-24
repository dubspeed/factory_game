#include "core.h"

using namespace Fac;

static int _calculate_rpm(std::optional<int> const processing_time, std::optional<int> const amount) {
    auto const rpm = processing_time.value_or(0) * amount.value_or(0);
    return rpm == 0 ? 0 : (60 / processing_time.value_or(0)) * amount.value_or(0);
}

void Machine::setRecipe(std::optional<Recipe> const &r) {
    // if (r->inputs.size() > fixed_input_slots + 1) {
    //     throw std::runtime_error("This machine only has one input slot");
    // }
    if (r->products.size() > fixed_output_slots + 1) {
        throw std::runtime_error("This machine only has one output slot");
    }
    _active_recipe = r;
    auto const stack_in = getInputStack(1);
    auto const stack_out = getOutputStack(0);

    if (stack_in == nullptr || stack_out == nullptr) {
        return;
    }

    stack_in->clear();
    stack_in->lockResource(r->inputs[fixed_input_slots].resource);
    stack_out->clear();
    stack_out->lockResource(r->products[fixed_output_slots].resource);
}

std::optional<Recipe> Machine::getRecipe() const {
    return _active_recipe;
}

int Machine::getInputRpm() const {
    if (_active_recipe.has_value()) {
        auto const r = _active_recipe.value();
        return _calculate_rpm(r.processing_time_s, r.inputs[fixed_input_slots].amount);
    }
    return 0;
}

int Machine::getOutputRpm() const {
    if (_active_recipe.has_value()) {
        auto const r = _active_recipe.value();
        return _calculate_rpm(r.processing_time_s, r.products[fixed_output_slots].amount);
    }
    return 0;
}

void Machine::update(double const dt) {
    if (!_active_recipe.has_value()) {
        return;
    }

    auto const connected_input = getInputStack(0);
    auto const interal_input_stack = getInputStack(1);
    // move items from connected input to internal input stack
    if (!connected_input->isEmpty() && interal_input_stack->canAdd(1, connected_input->getResource())) {
        interal_input_stack->addOne(connected_input->getResource());
        connected_input->removeOne();
    }

    if (processing) {
        processing_progress += dt;
    }

    const auto [inputs, products, processing_time_s] = _active_recipe.value();

    if (processing_progress >= processing_time_s * 1000) {
        processing = false;
        processing_progress = 0.0;
        if (getOutputStack(0)->getAmount() == MAX_STACK_SIZE) {
            // TODO what to do when output stack is full?
            // Idea: keep in a processing and add to output stack when it becomes available
        } else {
            getOutputStack(0)->addAmount(products[fixed_output_slots].amount,
                                         products[fixed_output_slots].resource);
        }
        return;
    }

    if (!processing && canStartProduction()) {
        getInputStack(1)->removeAmount(inputs[fixed_input_slots].amount);
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

    if (!getInputStack(0) || !getInputStack(1) || !getOutputStack(0))
        return false;

    if (getInputStack(1)->getAmount() < r.inputs[fixed_input_slots].amount)
        return false;

    if (!getOutputStack(0)->canAdd(r.products[fixed_output_slots].amount, r.products[fixed_output_slots].resource))
        return false;

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
