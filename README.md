# RayTracer

A high-performance, architecture-focused **CPU Ray Tracer** built with a modular shader system. While modern rendering typically favors the GPU for raw throughput, this project serves as a deep dive into advanced CPU optimizations, creative architectural patterns, and scalable software design.

---

## Why CPU-Based?

In the age of specialized hardware, choosing the CPU for a ray tracer is a deliberate architectural decision. While GPUs excel at "dumb" parallelization, a CPU-based approach allows us to:

* **Explore Design Patterns:** Implement complex object-oriented patterns and plugin architectures that are difficult to replicate in the rigid execution environment of a GPU.
* **Granular Optimization:** Fine-tune memory management, cache locality, and custom instruction sets.
* **Architecture Over Raw Power:** Focus on the "how" and "why" of engine design, creating a modular system where every component—from the BVH to the shader pipeline—is highly flexible.

---

## Performance & Parallelism

The core challenge of CPU rendering is maximizing the efficiency of every clock cycle. We achieved this through a sophisticated concurrency model:

### The Tile-Based Threadpool

To combat the performance bottleneck of single-threaded rendering, we implemented a **custom Threadpool system**.

* **Maximum Throughput:** Instead of simple line-by-line rendering,  work is batched and distributed across all available CPU cores.
* **Worker-Consumer Pattern:** We utilized a robust worker-consumer model across the entire engine. By using **atomic queues** for task distribution, we eliminated much of the overhead associated with thread synchronization, leading to a drastic increase in performance.

### Dynamic Acceleration

* **Dynamic BVH:** Implements a two-level acceleration structure (**TLAS** and **BLAS**) that is rebuilt only when necessary, allowing for dynamic scenes and interactive performance.
* **Adaptive Super Sampling:** A dynamic system that adjusts sampling rates to balance visual fidelity and frame rates in real-time.

---

##  Key Features

###  Modular Shader System

Shaders are implemented as dynamic library plugins (`.so` files). By implementing a standard interface, new material properties and effects can be added without recompiling the core engine.

* **Automatic Pipeline:** Plugins are assigned names, types, and priorities, allowing the engine to automatically organize them into an efficient execution pipeline.
* **Frontend Integration:** Uses **Raylib** for window management and display, with **ImGui** providing a robust real-time GUI for parameter tweaking.

---

## Usage

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Run

The ray tracer supports both raw geometry and pre-defined scene configurations.

```bash
# Load an OBJ Model
./build/raytracer test/dragon.obj
# Load a Scene Configuration
./build/raytracer assets/scenes/multi.cfg
```

---

##  Creative Extensibility

The true strength of this project lies in its **Plugin Interface**. To add a new feature, you simply create a new shader implementing the interface and compile it as a `.so` file. The system handles the rest, integrating it into the pipeline based on its metadata.