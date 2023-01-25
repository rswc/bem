#include "state.h"

State& getGlobalState() {
    static State state;
    return state;
}