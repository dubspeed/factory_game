#ifndef STORAGE_H
#define STORAGE_H

#include "core.h"

namespace Fac {
    class Storage : public GameWorldEntity, public InputStackProvider, public OutputStackProvider {
        friend void to_json(json &j, const Storage &r);

        friend void from_json(const json &j, Storage &r);

    public:
        explicit Storage(): InputStackProvider(1), OutputStackProvider(1) {
        }

        void setMaxItemStacks(int max_item_stacks) {
            _max_item_stacks = max_item_stacks;
            _content_stacks.resize(max_item_stacks);
        }

        // api to check if an amount of items can fit in the storage
        bool canStore(int amount, Resource const &r) const {
            return std::ranges::any_of(_content_stacks, [&](Stack const &s) {
                return s.getResource() == r && s.canAdd(amount, r);
            });
        }

        // get the total amount of items of a requested resource
        int getAmount(Resource const &r) const {
            return std::accumulate(_content_stacks.begin(), _content_stacks.end(), 0,
                                   [&](int const acc, Stack const &s) {
                                       if (s.isEmpty()) {
                                           return acc;
                                       }
                                       return s.getResource() == r ? acc + s.getAmount() : acc;
                                   });
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
