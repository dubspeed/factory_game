
#ifndef EXAMPLES_H
#define EXAMPLES_H
#include "../factory.h"
struct Examples {
    static void complexFactory(std::shared_ptr<Fac::Factory> const &factory);
    static void simpleFactory(std::shared_ptr<Fac::Factory> const &factory);
    static void starterFactory(std::shared_ptr<Fac::Factory> const &factory);
};

#endif //EXAMPLES_H
