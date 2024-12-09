#include "mcmini/misc/cond/cond_var_wakegroup.hpp"
#include <algorithm>


    bool WakeGroup::contains(runner_id_t tid) const {
        return std::find(begin(), end(), tid) != end();
    }

    void WakeGroup::remove_candidate_thread(runner_id_t tid) {
        erase(std::remove(begin(), end(), tid), end());
    }
