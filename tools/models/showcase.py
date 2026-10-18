"""Builds every game model in Blender, at its game scale and laid out like a frame of the game, seen
by a camera that matches the game's (numpad 0) with its light and background.

    blender --python tools/models/showcase.py

or open this file in Blender's Text Editor and Run Script. Everything goes in its own scene,
"R-Type Showcase", which the windows switch to: the other scenes are left alone. Running it again
rebuilds that scene's models.
"""
import importlib
import math
import os
import sys

import bpy
from mathutils import Vector


def tools_dir():
    here = os.path.dirname(os.path.abspath(__file__))
    if os.path.exists(os.path.join(here, "gametools.py")):
        return here
    # run from the Text Editor: __file__ points inside the .blend
    return os.path.dirname(bpy.path.abspath(bpy.context.space_data.text.filepath))


TOOLS = tools_dir()
REPO = os.path.dirname(os.path.dirname(TOOLS))
sys.path.insert(0, TOOLS)
import gametools as gt  # noqa: E402 (needs TOOLS in the path)
importlib.reload(gt)  # picks up edits when run again in the same Blender

SCENE = "R-Type Showcase"
COLLECTION = "R-Type Models"
# game positions (x, y) of each model's looks: the 4 player teams, one of each enemy
LAYOUT = {
    "player": [(-8.5, 4.2), (-6.0, 1.4), (-8.5, -1.4), (-6.0, -4.2)],
    "drone": [(-0.5, 4.2)],
    "saucer": [(0.0, 0.0)],
}
FOV = 60.0  # assets/scenes/space.cfg
WORLD_HALF_HEIGHT = 6.75  # common/include/common/World.hpp


def remove_collection(collection):
    for child in list(collection.children):
        remove_collection(child)
    for obj in list(collection.objects):
        data = obj.data
        bpy.data.objects.remove(obj)
        if data is None or data.users:
            continue
        for kind, store in ((bpy.types.Mesh, bpy.data.meshes), (bpy.types.Curve, bpy.data.curves),
                            (bpy.types.Camera, bpy.data.cameras), (bpy.types.Light, bpy.data.lights)):
            if isinstance(data, kind):
                store.remove(data)
    bpy.data.collections.remove(collection)


def new_collection(name, parent):
    collection = bpy.data.collections.new(name)
    parent.children.link(collection)
    return collection


def label(text, location, collection, size):
    curve = bpy.data.curves.new("label " + text, 'FONT')
    curve.body = text
    curve.size = size
    curve.align_x = 'CENTER'
    curve.materials.append(gt.make_material("showcase_label", (235, 235, 245), 1.0))
    obj = bpy.data.objects.new("label " + text, curve)
    obj.location = location
    obj.rotation_euler = (math.radians(90), 0, 0)  # faces the game camera
    collection.objects.link(obj)


def add_models(top):
    groups = {name: new_collection(name, top) for name in ("Players", "Enemies")}
    labels = new_collection("Labels", top)
    for name, places in LAYOUT.items():
        module = importlib.reload(importlib.import_module(name))
        collection = new_collection(name, groups["Players" if name == "player" else "Enemies"])
        roots = module.build(collection)
        themes = getattr(module, "THEMES", None)
        for i, (root, (x, y)) in enumerate(zip(roots, places)):
            root.location = (x, 0.0, y)
            root.scale = (module.SCALE,) * 3
            text = f"P{i + 1} {themes[i]['name']}" if themes else name
            label(text, (x, 0.0, y - module.SCALE * 1.05 - 0.45), labels, 0.6 if name == "boss" else 0.4)


def add_camera_and_light(top, scene):
    stage = new_collection("Game Camera + Light", top)

    # at game z = -distance looking along +z, so that it sees exactly the 24 x 13.5 play area
    camera = bpy.data.objects.new("Game Camera", bpy.data.cameras.new("Game Camera"))
    camera.location = (0.0, -WORLD_HALF_HEIGHT / math.tan(math.radians(FOV / 2)), 0.0)
    camera.rotation_euler = (math.radians(90), 0, 0)
    camera.data.sensor_fit = 'VERTICAL'
    camera.data.angle = math.radians(FOV)
    camera.data.clip_end = 100.0
    nebula = os.path.join(REPO, "assets", "game", "textures", "space_nebula.png")
    if os.path.exists(nebula):
        background = camera.data.background_images.new()
        background.image = bpy.data.images.load(nebula, check_existing=True)
        background.frame_method = 'CROP'
        camera.data.show_background_images = True
    stage.objects.link(camera)
    scene.camera = camera
    scene.render.resolution_x, scene.render.resolution_y = 960, 540

    # space.cfg: a directional light of 0.85 from the upper left, on the camera's side, game
    # direction (0.5, -0.6, 1) = Blender (0.5, 1, -0.6), and an ambient light of 0.25
    sun = bpy.data.objects.new("Game Light", bpy.data.lights.new("Game Light", 'SUN'))
    sun.data.energy = 0.85 * math.pi
    sun.data.angle = 0.0
    sun.rotation_euler = Vector((0.5, 1.0, -0.6)).normalized().to_track_quat('-Z', 'Y').to_euler()
    stage.objects.link(sun)

    world = bpy.data.worlds.get("R-Type Space") or bpy.data.worlds.new("R-Type Space")
    world.use_nodes = True
    nodes, links = world.node_tree.nodes, world.node_tree.links
    nodes.clear()
    ambient = nodes.new("ShaderNodeBackground")
    ambient.inputs["Color"].default_value = (0.25, 0.25, 0.25, 1.0)
    backdrop = nodes.new("ShaderNodeBackground")
    backdrop.inputs["Color"].default_value = (0.004, 0.004, 0.012, 1.0)
    path = nodes.new("ShaderNodeLightPath")
    mix = nodes.new("ShaderNodeMixShader")
    output = nodes.new("ShaderNodeOutputWorld")
    links.new(path.outputs["Is Camera Ray"], mix.inputs[0])
    links.new(ambient.outputs[0], mix.inputs[1])
    links.new(backdrop.outputs[0], mix.inputs[2])
    links.new(mix.outputs[0], output.inputs[0])
    scene.world = world
    # the game shows the colors of models.cfg as they are
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'None'


def show(scene):
    for window in bpy.context.window_manager.windows:
        window.scene = scene
        for area in window.screen.areas:
            if area.type == 'VIEW_3D':
                space = area.spaces.active
                space.shading.type = 'MATERIAL'
                space.shading.use_scene_lights = True
                space.shading.use_scene_world = True
                space.region_3d.view_perspective = 'CAMERA'


scene = bpy.data.scenes.get(SCENE) or bpy.data.scenes.new(SCENE)
old = bpy.data.collections.get(COLLECTION)
if old:
    remove_collection(old)
top = new_collection(COLLECTION, scene.collection)
add_models(top)
add_camera_and_light(top, scene)
show(scene)
