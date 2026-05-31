# RayTracer Studio: UI & User Commands Manual

The application combines a real-time **ImGui Control Panel** (RayTracer Studio Window) with dynamic **Viewport Shortcuts** powered by Raylib for rapid scene interaction and exploration.

---

## Viewport Navigation & Keyboard Shortcuts

When interacting directly with the rendering viewport, the following keyboard and mouse commands control the camera, handle object selection, and trigger system events.

### Camera Movement (WASD-QE)

Standard 3D viewport controls are used to fly through your scene:

* **`W`** : Move Forward
* **`S`** : Move Backward
* **`A`** : Move Left
* **`D`** : Move Right
* **`Q`** : Move Down
* **`E`** : Move Up

### Camera Orientation (Arrow Keys)

Fine-tune where the camera is looking (Yaw and Pitch):

* **`▲ UP` / `▼ DOWN**` : Pitch camera Up / Down
* **`◀ LEFT` / `▶ RIGHT**` : Yaw camera Left / Right

### Mouse Interactivity & Object Selection

* **`Left Mouse Click`** : Performs a hardware raycast through the viewport pixel into the scene's **TLAS (Top-Level Acceleration Structure)**.
* Clicking a mesh instantly **selects** that object instance and highlights it in the UI.
* Clicking into empty space deselects the current object.


### Global Utilities

* **`M`** : Quickly spawns a new instance of the default mesh (`index 0`) at the origin and automatically rebuilds the TLAS.
* **`P`** : Saves the current viewport render out to an image file.

---

## The UI Control Panel (RayTracer Studio)

The graphical interface is divided into 5 contextual tabs allowing real-time modification of the scene graph, materials, pipelines, and environmental rendering parameters.

### 1. Camera Tab

Provides diagnostic readouts of the current viewport camera state and physical lens modifications.

* **Telemetry** : View live telemetry including exact 3D coordinates (`Position X Y Z`) and orientation angles (`Yaw` / `Pitch`).
* **FOV Slider** : Adjust the camera's Field of View dynamically from **10.0° to 150.0°**.

### 2. Objects Tab

Displays a dynamic, scrollable list of all active object instances loaded into the scene graph. Clicking an object in the viewport expands its properties here automatically.

* **Transform Controls (TRS)** :
    * **Position** (`DragFloat3`) : Move the object along X, Y, and Z axes.
    * **Rotation** (`DragFloat3`) : Rotate the object (Euler angles).
    * **Scale** (`DragFloat3`) : Uniform or non-uniform scaling (clamped to a minimum of `0.01` to avoid inversion).


* **Material Editor** :
    * **Color Picker** : A fully interactive Hue Wheel / RGB layout to modify the base color of the mesh material.
    * **Physical Sliders** : Adjust individual optical traits including **Reflectivity** ($0.0 \rightarrow 1.0$), **Refractivity** ($0.0 \rightarrow 1.0$), **IOR** (Index of Refraction, $1.0 \rightarrow 3.0$), and **Emission** intensity ($0.0 \rightarrow 100.0$).
    * **Shader Combo Box** : Swap the underlying shading model for the object on-the-fly (`Emission`, `Diffusion`, `Caustic`, `Utility`, or `Debug`).


* **Texture Mapping** :
    * Input a relative filepath or absolute path to an image texture asset.
    * Click **Load** to bind the texture to the material, or **Remove** to clear out the texture map buffer.



### 3. Lights Tab

Manages ambient and progressive lighting configurations for the integrator.

* **Ambient Light** : Global background illumination intensity slider ($0.0 \rightarrow 1.0$).
* **Light Types Selector** : Switch between managing **Directional** or **Point** light arrays:
* **Directional Lights** : Modify the vector direction (`3D vector tracking`) and scale intensity. You can remove existing lights or click `+ Add Directional` to spawn a new sun-type light source pointing downward by default.
* **Point Lights** : Modify the exact 3D position in space and scale intensity. Use the `+ Add Point Light` button to create a light source localized at height coordinates by default.



### 4. Features Tab

This tab acts as an interactive debugging toggle deck for your rendering pipeline components.

* Dynamically lists every post-processing or ray-generation feature attached to your active `Integrator` pipeline.
* Use the **Checkboxes** next to each feature name to toggle structural effects (like shadows, ambient occlusion, anti-aliasing, etc.) on or off without recompiling the program.

### 5. Scene Tab

Handles input/output configurations and global sky environmental effects.

* **Scene Archiving** : Input a filename path and click **Save** to write out the current scene setup via the `SceneParser`. If an error occurs (invalid path, permission denial), a structural **"Save Error" Modal** will appear to warn you.
* **HDRI Environment Maps** :
* Enter a path to a high-dynamic-range image (`.hdr`/`.exr`).
* Click **Load** to project it as the background sky dome.
* **Ambient Intensity Slider** : Control how much environmental lighting the HDRI contributes to the scene ($0.0 \rightarrow 100.0$).
