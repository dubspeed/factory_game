#include "storage.h"

using namespace Fac;

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
