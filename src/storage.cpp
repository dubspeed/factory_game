#include "storage.h"

using namespace Fac;

void Storage::update(double dt) {

    if (!getInputStack(0)->isEmpty()) {
        auto const resource = getInputStack(0)->getResource();

        // find a stack that can take the item
        auto const stack = std::ranges::find_if(_content_stacks, [&](Stack &s) {
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

    // feed any output connections
    if (_content_stacks.empty()) {
        return;
    }

    for (int i = 0; i < _output_stacks.size(); i++) {
        if (auto const output_stack = getOutputStack(i); !output_stack->isFull()) {
            for (auto &contentStack: _content_stacks) {
                if ((contentStack.getResource() == output_stack->getResource() || output_stack->isEmpty())  && contentStack.getAmount() > 0) {
                    output_stack->addOne(contentStack.getResource());
                    contentStack.removeOne();
                    break;
                }
            }
        }
    }
}
