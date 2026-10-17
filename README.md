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

Shaders are implemented as dynamic library plugins (`.so` files on Linux, `.dll` on Windows). By implementing a standard interface, new material properties and effects can be added without recompiling the core engine.

* **Automatic Pipeline:** Plugins are assigned names, types, and priorities, allowing the engine to automatically organize them into an efficient execution pipeline.
* **Frontend Integration:** Uses **Raylib** for window management and display, with **ImGui** providing a robust real-time GUI for parameter tweaking.

---

## Usage

### Build

#### Linux

```bash
mkdir build && cd build
cmake ..
make
```

#### Windows

The Windows build uses MinGW-w64 (GCC). raylib and libconfig++ are downloaded and built automatically, and everything is linked statically: `client.exe` and the plugins (`rayFeatures/*.dll`) don't need any extra DLL to run.

Cross-compiling from Linux or WSL (Ubuntu: `sudo apt install mingw-w64`):

```bash
cmake -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake
cmake --build build-windows -j 4
```

Or natively on Windows with [MSYS2](https://www.msys2.org/), from the **UCRT64** shell:

```bash
pacman -S git mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
cmake -B build -G Ninja
cmake --build build -j 4
```

Each compile job needs a fair amount of RAM, so keep `-j` low on machines with little memory.

### Run

Run it from the repository root: the plugins are loaded from `rayFeatures/` and the assets from `assets/`. The game opens on its main menu, in front of the scrolling space background of `assets/scenes/space.cfg` (seen by a fixed camera).

```bash
./build/client
```

The server to connect to can be given at launch (see `./build/client -h`):

```bash
./build/client -t 192.168.1.20 -p 4242
./build/client --host localhost --port 4242
```

Without them it uses `127.0.0.1:4242`.

On Windows (PowerShell):

```powershell
.\build-windows\client.exe
```

| Key | Action |
|---|---|
| Arrows / WASD | Move (menu: navigate the buttons) |
| Enter | Confirm |
| Space | Fire |
| P | Pause |
| Esc | Back (quits from the main menu) |

### Game assets

The ships and the background are placeholders, made by `tools/generate_placeholder_assets.py` (`python3 tools/generate_placeholder_assets.py` from the repository root writes them again). Replace the files or point the configs at new ones:

- `assets/game/entities.cfg`: the player and the enemies. Each model is a list of `.obj` parts with their own color, `emission` makes a part glow. Ships fly along +x (nose to the right). It also sets the speeds, the enemy waves and how often enemies spawn.
- `assets/scenes/space.cfg`: the background layers (tiling textures, each scrolling at its own speed), the lights, the HDRI seen in reflections and the shader features.

To run it on another Windows machine, copy `client.exe` together with the `rayFeatures/` folder (only the `.dll` files are needed) and the `assets/` folder.

---

##  Creative Extensibility

The true strength of this project lies in its **Plugin Interface**. To add a new feature, you simply create a new shader implementing the interface and compile it as a shared library (`.so` on Linux, `.dll` on Windows). The system handles the rest, integrating it into the pipeline based on its metadata.