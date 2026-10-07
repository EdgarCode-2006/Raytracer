# Ray Tracer Technical Documentation: Plugin-Based Pipeline Architecture

This document outlines the architectural design and implementation of a high-performance, CPU-based Ray Tracer. The project prioritizes **modular scalability**, **parallel processing**, and **collaborative development** through a unique iterative pipeline.

---

## 1. The Challenges of Traditional Ray Tracing

Implementing a ray tracer in a team environment presents three primary hurdles:

*   **Tight Coupling:** Ray tracing is typically a recursive process where a single function handles intersection, shading, and secondary ray generation. This makes it difficult to divide tasks among developers without constant merge conflicts.
*   **Hardware Bottlenecks:** Without utilizing GPU shader languages, the CPU can easily become a bottleneck. High-performance C++ code is required to bridge the gap.
*   **Scalability:** Standard "Whitted-style" ray tracers are often hard-coded. Expanding a renderer to support path tracing or custom physical effects usually requires deep architectural refactoring.

---

## 2. Core Data Structures

To facilitate a non-recursive, iterative pipeline, we utilize two primary structures. These allow the state of a ray and its interaction with the scene to be passed through various plugin modules.

### The Ray Struct
Represents a single light transport path segment. By storing `throughput` and `depth`, we can process rays iteratively rather than recursively, preventing stack overflow.

```cpp
struct Ray {
    Math::Vector3D origin;
    Math::Vector3D direction;
    Math::Vector3D throughput; // Cumulative color/energy multiplier
    int depth;                 // Current bounce count
};
```

### The SurfaceIntersection Struct
This is the primary data exchange format for our plugin system. It contains the geometric and material context of a hit point.

```cpp
struct SurfaceIntersection {
    Math::Vector3D normal;       // Surface normal at hit point
    Math::Vector3D hitPoint;     // World space coordinates of the hit
    Math::Vector3D rayDir;       // Direction of the incoming ray
    Graphic::Material *mat;      // Material properties of the object
    Graphic::Pixel localColor;   // Color contribution from this interaction
    std::vector<Ray> nextRays;   // Secondary rays (reflections, refractions)
    Math::Vector3D throughput;   // Modified energy for the next path segment
};
```

---

## 3. The Solutions: Architectural Design

### A. Team-Oriented Pipeline (Separation of Concerns)
Our Ray Tracer uses a **Pipeline Pattern**. Instead of one "god function," each ray follows a lifecycle:
1.  **Queue Consumption:** A central loop consumes a queue of rays.
2.  **Intersection:** The BVH identifies the closest hit.
3.  **Feature Processing:** The `SurfaceIntersection` is passed through a list of plugins.
4.  **Ray Generation:** If a plugin (like `SpecularHandler`) adds a ray to `nextRays`, it is pushed back into the global queue.



### B. Hardware Optimization
To maximize CPU throughput, we implemented two critical technologies:
*   **BVH with TLAS/BLAS:** A Bounding Volume Hierarchy using Top-Level and Bottom-Level Acceleration Structures. This enables mesh instancing and allows the engine to handle millions of triangles with minimal overhead.
*   **Threaded Tile Batching:** The screen is divided into 2D tiles. These tiles are dispatched to a **ThreadPool**, ensuring all CPU cores are saturated and rendering occurs concurrently.

### C. Scalability & Plugin System
The system is built around the `IRayFeature` (or `ARayFeature`) interface. This allows developers to write `.so` (Shared Object) files, or `.dll` files on Windows, that can be loaded at runtime.

#### Example Implementation: Specular Reflection
```cpp
namespace RayTracer {
    class feature : public ARayFeature {
        public:
            feature() : ARayFeature("SpecularHandler") {};
            
            // Modifies the intersection data and can spawn reflection rays
            void process(RenderData &data, SurfaceIntersection &interaction) override;
            
            // Ensures correct execution order (e.g., Shadowing before Reflection)
            int getPriority() const noexcept override { return 0; };
    };
}
```

**Priority System:** Since multiple plugins interact with the same `SurfaceIntersection`, each plugin has a `getPriority()` method. The engine sorts the pipeline at runtime, ensuring that logic (like calculating base color) happens before secondary effects (like adding specular highlights).

---

## 4. Comparison: Ray Tracing vs. Path Tracing
By using an iterative queue and the `throughput` vector, our architecture naturally extends from a simple Whitted Ray Tracer to a full **Path Tracer**. 

| Feature | Whitted Ray Tracer | Our Plugin Path Tracer |
| :--- | :--- | :--- |
| **Logic** | Recursive | Iterative (Queue-based) |
| **Reflections** | Single Perfect Mirror | Stochastic (Randomized) via Plugins |
| **Complexity** | Fixed | Highly Scalable |
| **Memory** | Stack-heavy | Heap/Queue Managed |

---

## 5. Conclusion
The combination of a **Threaded Pipeline**, **Iterative Ray Processing**, and a **Prioritized Plugin System** provides a robust framework for 3D rendering. It successfully solves the problem of team concurrency while maintaining the high performance required for CPU-bound rendering.