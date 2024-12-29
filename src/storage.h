#ifndef STORAGE_H
#define STORAGE_H

#include "core.h"

namespace Fac {
    class Storage final: public GameWorldEntity, public InputStackProvider, public OutputStackProvider {
        friend void to_json(json &j, const Storage &r);

        friend void from_json(const json &j, Storage &r);

    public:
        constexpr static int OUTPUT_STACK_SIZE = 5;
        explicit Storage(): InputStackProvider(1), OutputStackProvider(1) {
            OutputStackProvider::getOutputStack(0)->setMaxStackSize(OUTPUT_STACK_SIZE);

            setMaxItemStacks(1);
        }

        void setMaxItemStacks(int const max_item_stacks) {
            _max_item_stacks = max_item_stacks;
            _content_stacks.resize(max_item_stacks);
        }

        // api to check if an amount of items can fit in the storage
        bool canStore(int const amount, Resource const &r) const {
            return std::ranges::any_of(_content_stacks, [&](Stack const &s) {
                return s.isEmpty() || s.canAdd(amount, r);
            });
        }

        // get the total amount of items of a requested resource
        int getAmount(Resource const &r) const {
            auto content = std::accumulate(_content_stacks.begin(), _content_stacks.end(), 0,
                                   [&](int const acc, Stack const &s) {
                                       if (s.isEmpty()) {
                                           return acc;
                                       }
                                       return s.getResource() == r ? acc + s.getAmount() : acc;
                                   });
            // also include the 5 items in the output stack for amount calculation
            if (getOutputStack(0)->getResource() == r) {
                content += getOutputStack(0)->getAmount();
            }

            return content;
        }

        // TODO get inventory API

        // TODO better API / Tests
        void manualAdd(int const amount, Resource const &r) const {
            if (canStore(amount, r)) {
                getInputStack(0)->addAmount(amount, r);
            }
        }


        void update(double dt) override;
        
        int getId() const override { return _id; }

    private:
        int _id = generate_id();
        std::vector<Stack> _content_stacks = std::vector<Stack>();
        int _max_item_stacks = 0;
    };
} // Fac

#endif //STORAGE_H
