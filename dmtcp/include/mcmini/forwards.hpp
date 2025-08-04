#pragma once

class coordinator;
class model_to_system_map;

namespace model {
class state;
class mutable_state;
class detached_state;
class state_sequence;
class transition;
}  // namespace model

namespace real_world {
class local_linux_process;
class fork_process_source;
}  // namespace real_world
