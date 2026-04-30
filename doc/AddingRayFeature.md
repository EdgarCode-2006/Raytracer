# Developer Documentation: Creating Plugins for the Ray Tracer

## 1. What are Ray Tracer Plugins?
In this engine, **Plugins** (referred to as `RayFeatures`) are modular components that define how light interacts with the surfaces in a scene. Instead of a hard-coded rendering loop, the engine uses a **Pipeline** of these plugins.

Each plugin is responsible for a specific graphical effect—such as specular reflection, ambient occlusion, or basic Lambertian lighting. By separating these into plugins, multiple developers can work on different lighting models simultaneously without touching the core engine code.

---

## 2. The Plugin Interface (`IRayFeature`)
To create a new plugin, you must implement the `IRayFeature` interface. This ensures the core engine knows how to process your logic and where to place it in the execution order.

```cpp
class IRayFeature {
    public:
        virtual ~IRayFeature() = default;

        // The core logic of your plugin
        virtual void process(RenderData &data, SurfaceIntersection &interaction) = 0;

        // Determines the execution order (lower values run first)
        virtual int getPriority() const noexcept = 0;

        // Toggle state (enabled/disabled)
        virtual bool isActive() const noexcept = 0;
        virtual void setActivity(const bool &activity) noexcept = 0;

        // Identifier for the UI and debugging
        virtual const std::string getName() const noexcept = 0;
};
```

### Key Method: `process()`
This is where the "magic" happens. The engine provides you with:
*   `RenderData`: Global scene information (lights, textures, HDRI).
*   `SurfaceIntersection`: Local data about the hit (normal, material, position). 

**Your plugin can:**
1.  Modify `interaction.localColor` to add light to the pixel.
2.  Add new `Ray` objects to `interaction.nextRays` to create reflections or refractions.

---

## 3. How Plugins are Executed (The Pipeline)
The engine utilizes an iterative queue system. When a ray hits an object, the engine iterates through all loaded and active plugins in order of their **priority**.



```cpp
// Inside Integrator::consumeRays
for (auto &rayFeature : _pipeline) {
    if (rayFeature->isActive())
        rayFeature->process(data, intersection);
}
```

By setting a `getPriority()`, you ensure that a plugin calculating "Shadows" runs before a plugin calculating "Reflection," ensuring the final color is mathematically sound.

---

## 4. Creating and Compiling a Plugin

### Step 1: Implementation
Create your class inheriting from `ARayFeature` (or `IRayFeature`). Ensure you implement an entry point (usually `getInstance`) so the dynamic loader can instantiate your class from a `.so` file.

### Step 2: Build System (CMake)
Use the `add_ray_feature` macro provided in the project's CMake configuration. This macro handles the complexity of creating a Shared Object (`.so`) and linking it against the core library.

```cmake
# Example: Adding a new Bloom effect plugin
add_ray_feature(BloomEffect_feat src/integrators/features/BloomEffect.cpp)
```

This will automatically:
1.  Compile your source into a shared library.
2.  Link `raytracer_core`.
3.  Output the `.so` file into the `./rayFeatures/` directory.

---

### Important Notes:
*   **Hot-Swapping:** Because plugins are stored in a `std::vector` of unique pointers, you can enable or disable specific features at runtime via `setActivity()`.
*   **Memory Safety:** The engine manages the lifecycle of the plugin. Always use the provided structs for data modification to avoid memory corruption in a multi-threaded environment.