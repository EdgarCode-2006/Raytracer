# Technical Documentation: High-Performance Multi-Threaded Ray Tracer Pipeline

This document outlines the architectural evolution and optimization strategies of our CPU-based ray tracer. By leveraging a custom thread pool, atomic synchronization, and a decoupled "Producer-Consumer" model, we have addressed critical bottlenecks in memory management and thread safety.

---

## 1. Architectural Overview
The engine is divided into three primary modules:
*   **GUI:** Handles user input and scene manipulation.
*   **Core Engine:** Manages the application state, frame timing, and texture uploading to the GPU.
*   **Integrator:** The heavy-lifting ray generation and intersection engine.

---

## 2. Optimized Pixel Buffer Management
### The Challenge: Pointer Contention
In the initial architecture, the **Core** allocated a pixel buffer and passed a raw pointer to the **Integrator**. While a tiling system was used to prevent data races, it created a synchronous bottleneck. Threads were often forced to wait for the next batch of pointers, and the Core had to manage the lifecycle of memory being actively mutated by multiple threads. This led to "memory uncertainty" and potential segmentation faults if the viewport resized during a render.

### The Solution: Inverting Responsibility (Producer-Consumer)
We inverted the ownership of the frame data. The **Integrator** now acts as the owner of the pixel buffer.
*   **Internal Allocation:** The Integrator allocates its own internal frame buffer based on camera dimensions provided by the Core.
*   **Decoupled Cadence:** The Integrator pushes completed frames to a queue. The Core consumes these frames at its own refresh rate to generate textures for display.
*   **Portability:** By making the Integrator the "provider" of data rather than a "modifier" of external memory, the engine becomes platform-agnostic. The Integrator can be ported to any graphics API (Vulkan, OpenGL, DirectX) because it simply outputs a standardized frame object.



---

## 3. Thread-Safe Scene Traversal
### The Challenge: Dynamic Scene Mutation
Initially, the Integrator held a direct reference to the Scene's Bounding Volume Hierarchy (BVH). When we introduced a dynamic GUI, a race condition emerged: if a user moved an object, the BVH was rebuilt or modified while worker threads were mid-traversal. This resulted in threads accessing invalidated memory addresses, leading to immediate crashes.

### The Solution: The Snapshot System
To support real-time interaction without sacrificing performance, we implemented a **Scene Snapshot** system using C++ smart pointers (`std::shared_ptr`).

1.  **Event-Driven Updates:** The GUI communicates changes to the Core via a callback function pointer.
2.  **Atomic Snapshots:** When a change is detected, the Core generates a "Snapshot"—a read-only, lightweight copy of the scene state and its BVH.
3.  **Shared Ownership:** This snapshot is passed to the Integrator. Because it is wrapped in a shared pointer, the snapshot remains valid as long as at least one worker thread is traversing it. 
4.  **Automatic Lifecycle Management:** Once the final thread finishes shooting rays into a specific snapshot and moves to the next one in the queue, the reference count drops to zero, and the memory is automatically reclaimed.



---

## 4. Execution Logic & Pipeline Flow
The final pipeline operates as a streamlined data flow:

1.  **Input:** User modifies an object in the GUI.
2.  **Signal:** GUI triggers a callback; the Core marks the scene as "dirty."
3.  **Capture:** The Core generates a Scene Snapshot and pushes it into the Integrator’s work queue.
4.  **Process:** The Integrator’s **Thread Pool** pulls the snapshot. Each thread uses **Atomic Counters** to claim specific tiles of the frame.
5.  **Output:** Once all tiles are filled, the Integrator yields the completed Frame Buffer to the Core.
6.  **Display:** The Core updates the texture and renders the result to the viewport.

---

## 5. Conclusion: Why This Architecture Works
This design solves the "Big Three" problems of CPU ray tracing: **Latency, Safety, and Scalability.**

*   **Throughput Maximization:** By using a thread pool and tile-based queues, we ensure the CPU stays at 100% utilization without threads idling for memory access.
*   **Memory Safety:** The shift from raw pointers to a Snapshot-based system removes the risk of "use-after-free" errors during scene updates. We have effectively decoupled the *state* of the scene from the *rendering* of the scene.
*   **Developer Agnostic Design:** Because the GUI, Core, and Integrator communicate through strict interfaces and ownership rules, developers can optimize the BVH traversal logic without fearing they will break the GUI’s memory or the Core’s display logic. 

The result is a robust, high-performance engine that allows for fluid, interactive scene editing while maintaining the rigorous execution logic required for path tracing.