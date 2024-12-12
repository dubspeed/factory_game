#include "storage.h"

using namespace Fac;

bool Storage::canStore(int amount, Resource const &r) const {
    return std::ranges::any_of(_content_stacks, [&](Stack const &s) {
        return s.getResource() == r && s.canAdd(amount, r);
    });
}

int Storage::getAmount(Resource const &r) const {
    return std::accumulate(_content_stacks.begin(), _content_stacks.end(), 0, [&](int const acc, Stack const &s) {
        if (s.isEmpty()) {
            return acc;
        }
        return s.getResource() == r ? acc + s.getAmount() : acc;
    });
}


void Storage::update(double dt) {
    if (getInputStack(0)->isEmpty()) {
        return;
    }

    auto resource = getInputStack(0)->getResource();

    // find a stack that can take the item
    auto stack = std::find_if(_content_stacks.begin(), _content_stacks.end(), [&](Stack &s) {
        return s.canAdd(1, resource);
    });

    if (stack != _content_stacks.end()) {
        stack->addOne(resource);
        getInputStack(0)->removeOne();
    } else {
        // can we add a new stack for the item?
        if (_content_stacks.size() < _max_item_stacks) {
            auto new_stack = Stack();
            new_stack.addOne(resource);
            _content_stacks.push_back(new_stack);
            getInputStack(0)->removeOne();
        }
    }
}

void Fac::to_json(json &j, const Storage &r) {
    auto id = r.getId();
    auto m = json{
                {"id", id}, {"max_item_stacks", r._max_item_stacks}
    };
    m["input"] = (InputStackProvider) r;
    m["output"] = (OutputStackProvider) r;
    m["content"] = r._content_stacks;
    j["storage"] = m;
}

void Fac::from_json(const json &j, Storage &r) {
    r._id = j.at("storage").at("id").get<int>();
    r._input_connections = j.at("storage").at("input").at("_input_connections").get<std::vector<InputConnection>>();
    r._output_stacks = j.at("storage").at("output").at("_output_stacks").get<std::vector<std::shared_ptr<Stack>>>();
    r._max_item_stacks = j.at("storage").at("max_item_stacks").get<int>();
    r._content_stacks = j.at("storage").at("content").get<std::vector<Stack>>();
}
