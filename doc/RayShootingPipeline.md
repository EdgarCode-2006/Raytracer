# RayTracer Architecture: The Circular Queue Pipeline

## 1. The Recursive Bottleneck
Traditional raytracers are often built on a **recursive stack model**. In this approach, when a ray hits a surface and needs to bounce (reflection or refraction), the function calls itself. 

While mathematically elegant, this creates several industrial-scale problems:
* **Stack Overflow:** In complex scenes requiring 50+ bounces (like a hall of mirrors), the computer's memory stack can exhaust, causing a crash.
* **Implicit State:** Information is hidden inside function calls, making it difficult to debug exactly what a specific "sub-ray" is doing.
* **Monolithic Logic:** It is hard to "unplug" specific features (like disabling only refractions) without rewriting the core loop.

## 2. The Solution: Heap-Based Iteration
Our model moves the logic from the **Stack** (limited and rigid) to the **Heap** (expansive and flexible). We treat rays not as function calls, but as **atomic data packets** inside a circular queue.


### The Workflow:
1.  **Generation:** A ray is created and pushed into a queue.
2.  **Consumption:** The engine pulls a ray from the front of the queue and calculates its intersection with the world.
3.  **Plugin Processing:** Instead of hard-coded logic, the ray passes through a "Pipeline." Various plugins (Reflection, Ambient Occlusion, Refraction) look at the intersection and decide if they need to create **new** rays.
4.  **Feedback Loop:** If a plugin generates a new ray, it is pushed to the back of the queue to be processed in the next iteration.

## 3. Why This Is Cleaner

### Clarity & Decoupling
By using a queue, we separate the **Mathematics** (where the ray hits) from the **Logic** (what happens at the hit). A developer can write a "Glass Material" plugin that simply generates a new ray and pushes it to the queue without ever knowing how the ray-intersection math works.

### Safety in Large Scenes
Because we use a `while` loop to exhaust the queue rather than recursion, we have total control over memory. We can set explicit "Safety Guards":
* **Depth Limits:** Stop rays that have bounced too many times.
* **Energy Thresholds:** If a ray's "energy" (throughput) becomes too low to contribute to the final pixel color, we discard it immediately.

### Atomic Scalability
Every interaction is **Atomic**. Because each ray carries its own data (origin, direction, and color contribution), the process is extremely modular. We can pause the queue, reorder it for performance, or even toggle specific plugins via a GUI in real-time. If you disable the "Reflection Plugin," the engine simply stops pushing new reflection rays into the queue, and the change is reflected across the entire image instantly.

## 4. Conclusion
The shift to a circular queue pipeline transforms the raytracer from a rigid script into a dynamic engine. It provides the memory safety required for massive production scenes while offering a "plug-and-play" environment where teams can collaborate on different material behaviors without interfering with the core shooting logic.