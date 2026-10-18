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

Without them it uses `127.0.0.1:4242`. There's no network yet: pressing Play starts a server inside the client, so the host and port aren't used.

On Windows (PowerShell):

```powershell
.\build-windows\client.exe
```

| Key | Action |
|---|---|
| Arrows / WASD | Move (in the menus: go between the buttons) |
| Enter | Press the highlighted button |
| Space | Fire |
| Esc | In the game: open the menu. In a menu: go back (quits from the title screen) |
| F12 | Screenshot (raylib saves `screenshot000.png`...) |

The menu opens over the game, which goes on under it: the server doesn't pause for one player, so the ship stops but stays in the game. From the menu you can resume, open the settings, leave the game for the title screen or quit.

**Settings** (from the title screen or the menu) only change this client:
- Controls: two keys per movement and for firing. Click a key (or press Enter on it), then press the new one; Backspace removes it, Esc cancels. A key moves from the action that had it. Esc and Enter can't be rebound, they drive the menus.
- Graphics: the FPS counter and the shader features, applied from the next frame.
- Accessibility: high contrast mode. The ships become flat bright shapes, one color per entity type (`contrastColor` in `models.cfg`: the player cyan, the enemies orange and yellow), over a background dimmed to 30%.

The debug views of the Graphics tab (wireframe, outline, normal map, BVH traversal depth) draw over every object when switched on.

### Game assets

- `assets/game/models.cfg` (client): how each entity type looks. Each model is a list of `.obj` parts with their own color, `emission` makes a part glow. The player flies along +x (nose to the right), the enemies face -x. Also the visual animation (spinning, rolling when going up or down) and the color of the model in high contrast mode.
- `assets/game/gameplay.cfg` (server): the speeds, sizes, enemy waves and how often enemies spawn.
- `assets/scenes/space.cfg`: the background layers (tiling textures, each scrolling at its own speed), the lights, the HDRI seen in reflections and the shader features.

Both files name the entity types the same way (`player`, `drone`, `saucer`, `fighter`, `boss`). The list is in `common/include/common/EntityType.hpp`: a new enemy type is added there, then in both files. The boss has its model but isn't spawned yet.

The models (`assets/game/models/*.obj`) are low poly (from about 340 triangles for the drone to 1600 for the boss) and made in Blender by the scripts of `tools/models/`, one per model. Change a model in its script, then write the `.obj` files again with Blender 4.2 or newer:

```bash
blender -b --factory-startup --python-exit-code 1 --python tools/models/export_models.py            # every model
blender -b --factory-startup --python-exit-code 1 --python tools/models/export_models.py -- drone   # only some
blender --python tools/models/showcase.py   # every model in a Blender scene, seen like in the game (numpad 0)
```

Blender's Front view is the game's view: X to the right, Z up, -Y towards the camera. Each part is a closed mesh and the exporter refuses one that is open or inside out (the ray tracer has no back faces). The background textures are made by `python3 tools/generate_space_textures.py`.

### Architecture

The game is split so that the server decides everything and the client only shows it:

```
 keyboard ──► client ── InputState ──► server ── WorldState ──► client ──► scene ──► raytracer
```

- `common/`: what the client and the server exchange (`InputState`, `WorldState`, the entity types, the size of the play area). No raytracer, raylib or ImGui.
- `simulation/`: the game rules (players, enemies, spawning), run by the server. Same restrictions as `common/`, it builds as its own library.
- `src/client/`: the client program. `Session` owns the main loop and a stack of modes: the title screen (`TitleClient`), the game (`GameClient`) and the menu (`MenuClient`, pushed over the game). The mode on top gets the input, the ones under it keep running without it, and they all draw their UI from the bottom up. The game sends the input to an `IServerLink` and `SceneBuilder` turns the world states it gets back into scene instances. `LocalServerLink` runs the simulation on its own thread at the server tick rate, until the network link replaces it.
- `src/` (the rest): the raytracer. `Engine` only renders the scene it is given.

To run it on another Windows machine, copy `client.exe` together with the `rayFeatures/` folder (only the `.dll` files are needed) and the `assets/` folder.

---

##  Creative Extensibility

The true strength of this project lies in its **Plugin Interface**. To add a new feature, you simply create a new shader implementing the interface and compile it as a shared library (`.so` on Linux, `.dll` on Windows). The system handles the rest, integrating it into the pipeline based on its metadata.