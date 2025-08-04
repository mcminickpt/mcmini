# McMini Architecture Evolution Plan

## Overview

This strategic document serves as a blueprint for the upcoming development of McMini. It describes the proposed enhancements and critical interfaces that warrant our attention. As a dynamic guide, it will be updated periodically to reflect new insights and directions based on our iterative progress and findings.

### The Model

At the heart of McMini lies the _Model_, an abstraction layer that facilitates the interaction between a live computational process and its analytical representation as perceived by the verifier. The `model::program` serves as a pivotal element in this architecture, encapsulating:

- **Trace Sequence (`S`)**: A chronological record of transitions, known as a _trace_, which catalogs the events that have transpired within the process. These transitions are linearizations of the actions the program has executed, detailed enough to reconstruct the program's path to its current state.

- **State Model**: Post-transition snapshots of the program's state are cataloged at each juncture (`t`) within the trace. This model operates as a comprehensive repository, encapsulating the state configurations throughout the verification sequence. It reflects the program's progression through a series of immutable states, each representing the program's state at a specific point in its execution history.

- **Thread-to-Transition Mapping**: This construct delineates the correspondence between the program's discrete execution entities ("threads") and their subsequent actions ("transitions"). It captures the notion of `next(s_N, p)`, which describes the possible transitions (or "next steps") available to threads in the program's current state.

Our objective is to refine the Model to better align with established theoretical frameworks. To this end, we envision transitions as pure functions mapping one state to another, rather than as operations that modify a state in situ. This functional approach is inspired by formal verification definitions, where a transition is characterized by its ability to be enabled and transform one state into another, underlining the importance of distinct state immutability. By distinguishing between transitions that are defined and those that are enabled, McMini ensures a more nuanced and accurate state progression, vital for verification accuracy.

To further enhance the Model's granularity, we introduce the concept of _visible objects_. These are the conduits through which threads communicate, sharing states that evolve as the program executes. Each visible object has a history, a sequence of states that represent its evolution over time, crucial for understanding inter-thread communication and ensuring comprehensive state coverage during verification.

### The Model Checker

The Model Checker operates as the analytical core of McMini, tasked with rigorously validating the Model's integrity and ensuring its adherence to established correctness criteria. Leveraging the robustness of the Dynamic Partial Order Reduction (DPOR) algorithm, the Model Checker systematically explores all possible states of the program modeled by McMini.

The implementation of the Model Checker is encapsulated within the `model_checking::algorithm` class. This abstraction transforms the complex task of verification into a structured and methodical process. It is a functional entity designed to:

- **Explore States**: Beginning from an initial state, often denoted as `s_0`, the algorithm ventures through the vast landscape of potential program states. Each state is represented by an instance of `model::program`, which serves as a historical ledger detailing the transitions and state evolutions of the process under scrutiny.

- **Model-Process Correspondence**: The algorithm maintains a critical invariant; the process under verification, represented by `real_world::process`, must be accurately modeled by the initial state. This strict correspondence is the linchpin for the algorithm's exploration strategy, relying entirely on the Model to dictate the course of state space traversal.

- **Encounter Callbacks**: As the algorithm delves into the program's state space, it invokes specific callbacks in response to certain conditions:
  - **Deadlock Detection**: When encountering a state `s` where no further progress is possible, signifying a deadlock situation.
  - **Data Race Identification**: On discovery of a state `s` indicative of a data race.
  - **Crash Handling**: If a process crash occurs at any point during verification.

These callbacks are crucial for providing real-time feedback on the verification process, allowing for immediate recognition and response to critical issues.

The Model Checker's approach to verification is comprehensive and exhaustive. It does not merely simulate the program's execution; it actively reconstructs it, replicating the various paths a program might take. This exhaustive reconstruction is essential for ensuring that no stone is left unturned in the quest for software reliability and correctness.

### The Real World

The Real World interface is McMini's conduit to live, operational processes. It provides a structured framework for the exploration and analysis of executing code, enabling McMini to interact with actual process states in a controlled manner. This interface is laden with system-specific mechanics designed for efficient process management and state tracking, such as:

- **Inter-process Communication (IPC)**: Utilizes mechanisms like `xpc` to facilitate communication between the McMini architecture and active processes, ensuring synchronized operation.
- **Dynamic Library Loading**: Employs the dynamic loading of `libmcmini.so` to inject McMini's functionality into the process space, allowing for real-time monitoring and manipulation.
- **Process Control Operations**: Leverages standard operations like `exec` and `fork` to manage process execution flow, allowing McMini to simulate different execution paths and conditions.
- **Checkpointing Integration**: Incorporates Distributed MultiThreaded CheckPointing (DMTCP) to create and restore snapshots of process states, providing a robust mechanism for state exploration and rollback capabilities.

With the `real_world::process_spawner`, McMini can instantiate new processes, emulating the birth of threads in an operating system. This functionality is crucial for exploring the multitude of execution paths a program can take, particularly in a multi-threaded environment.

The `real_world::process` acts as a proxy for the actual processes running on the CPU, mirroring their forward-only progression through time. This design choice reflects the reality that processes in the operating system move unidirectionally forward, just as the sequence of instructions in a thread. The `process` structure provides the necessary interfaces for McMini to "execute" threads in a simulated environment, enabling the Model Checker to evaluate the program's behavior in various scenarios.

Furthermore, the `real_world::transition_encoding` offers a representation of McMini's transitions within the real-world processes. This encoding acts as a serialization layer, allowing McMini to translate its model transitions into a form that can be applied to and understood by the actual executing code.

The Real World interface of McMini thus serves as a vital bridge between theoretical verification models and practical execution environments. It ensures that the insights gained from McMini's verification processes are grounded in reality, providing a high level of confidence in the correctness and robustness of the system under verification.

### The Coordinator

The Coordinator, operationally known as the _Tracer_ within McMini, is charged with the crucial task of ensuring synchronization between the Model and the Real World interfaces. It is the mechanism that aligns the abstract model of the program with the actual execution of the process it represents. This synchronization is not a one-time alignment but a continuous, dynamic process that evolves as the program executes.

In essence, the Coordinator/Tracer acts as a dynamic bridge, adjusting the Model to reflect the live state of the process with each step it takes. It operates under the following principles:

- **Stateful Synchronization**: The Coordinator maintains a consistent state between the model of the program (`model::program`) and the actual running process (`real_world::process`). This is vital because the program model represents not just a snapshot but a history of the process's execution, and any change in the process must be mirrored in the model.

- **Execution Mapping**: The Coordinator maps each thread within the process to its subsequent action or "thread routine." As the process unfolds in real-time, only the next immediate action of any thread is definitively known. The Coordinator oversees the execution of these actions, subsequently discovering and integrating the next steps as they become apparent.

- **Dynamic Discovery**: After a thread executes its immediate action in the process, the Coordinator dynamically discovers the thread's next action. This ongoing discovery is critical as it reveals the process's evolution at runtime, enabling the Model to adapt and transition into new states accurately.

- **Continuous Operation**: The Coordinator, through its tracing functionality, continuously operates to keep the model synchronized with the process. It is responsible for dynamically discovering the transitions and ensuring the model remains an accurate representation of the process as it progresses through its execution path.

The role of the Coordinator/Tracer is thus not merely to monitor but to actively engage with both the Model and the process, facilitating a verification environment that is both reflective of the real world and capable of anticipating its future states.

### Naming Conventions

We follow the naming conventions of the CoreFoundation regarding memory management where possible [see here](https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFMemoryMgmt/Concepts/Ownership.html#//apple_ref/doc/uid/20001148-CJBEJBHH)
