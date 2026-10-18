"""Blender helpers shared by the model scripts of tools/models/ (Blender 4.2+, bpy).

Game space: x right (ships fly along +x), y up, z into the screen (the game camera is at -z,
looking along +z). Blender's Front view (numpad 1, looking along +Y) is the game view:
    game (x, y, z) = Blender (X, Z, Y)
so the models are built with X = right on screen, Z = up, -Y = towards the game camera.

The game's OBJ loader ignores materials, uv and vn: one OBJ is one part with one flat color from
assets/game/models.cfg, and it smooths the normals of the faces that share a vertex index.
export_model() splits the vertices wherever Blender's own shading has a hard edge, so the game's
optional smooth shading looks like Blender's viewport, and flat-shaded faces stay faceted.
"""
import os

import bmesh
import bpy
from mathutils import Vector


def srgb_to_linear(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def make_material(name, rgb, emission=0.0):
    """rgb: 0-255 sRGB, the color = { r; g; b; } of models.cfg. Diffuse only like the game, and a
    glowing part shows exactly its color (with the Standard view transform)."""
    mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    lin = (*[srgb_to_linear(c / 255.0) for c in rgb], 1.0)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (0.0, 0.0, 0.0, 1.0) if emission > 0 else lin
    bsdf.inputs["Roughness"].default_value = 1.0
    bsdf.inputs["Specular IOR Level"].default_value = 0.0
    bsdf.inputs["Emission Color"].default_value = lin
    bsdf.inputs["Emission Strength"].default_value = emission
    mat.diffuse_color = lin  # Solid and Workbench color
    return mat


def new_root(model_name, location=(0.0, 0.0, 0.0), collection=None):
    """An Empty at the model's origin: the game rotates and scales the model around it."""
    root = bpy.data.objects.new(model_name, None)
    root.empty_display_type = 'PLAIN_AXES'
    root.empty_display_size = 0.3
    root.location = location
    root["game_model"] = model_name
    (collection or bpy.context.scene.collection).objects.link(root)
    return root


def mesh_object(part_name, verts, faces, material, root=None, smooth=False, collection=None):
    """A part from raw geometry (Blender coordinates). Faces may be quads or ngons."""
    me = bpy.data.meshes.new(part_name)
    me.from_pydata([tuple(v) for v in verts], [], [tuple(f) for f in faces])
    me.validate()
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)  # outward, for closed meshes
    bm.to_mesh(me)
    bm.free()
    for p in me.polygons:
        p.use_smooth = smooth
    obj = bpy.data.objects.new(part_name, me)
    obj.data.materials.append(material)
    (collection or (root.users_collection[0] if root else bpy.context.scene.collection)).objects.link(obj)
    if root is not None:
        obj.parent = root
    obj["game_part"] = part_name
    return obj


def parts_of(root):
    return [o for o in root.children_recursive if o.type == 'MESH' and "game_part" in o]


def game_triangles(obj, root):
    """The triangles of a part with its modifiers, in game space and with the game's outward
    winding. Vertices are shared only where Blender shades smoothly.
    Returns (vertices, triangles, closed, signed volume)."""
    deps = bpy.context.evaluated_depsgraph_get()
    me = bpy.data.meshes.new_from_object(obj.evaluated_get(deps), depsgraph=deps)
    m = root.matrix_world.inverted() @ obj.matrix_world
    if m.determinant() <= 0:
        raise ValueError(f"{obj.name}: negative scale, use a Mirror modifier instead")
    me.transform(m)
    me.calc_loop_triangles()
    corner_normals = me.corner_normals

    edge_faces = {}
    for p in me.polygons:
        for ek in p.edge_keys:
            edge_faces[ek] = edge_faces.get(ek, 0) + 1
    closed = all(c == 2 for c in edge_faces.values())

    index = {}
    verts = []
    tris = []
    for lt in me.loop_triangles:
        ids = []
        for li in lt.loops:
            vi = me.loops[li].vertex_index
            n = corner_normals[li].vector
            key = (vi, round(n.x, 3), round(n.y, 3), round(n.z, 3))
            if key not in index:
                co = me.vertices[vi].co
                index[key] = len(verts)
                verts.append((co.x, co.z, co.y))
            ids.append(index[key])
        # swapping two axes mirrors space: reverse the winding so the normals stay outward
        tris.append((ids[0], ids[2], ids[1]))
    bpy.data.meshes.remove(me)

    volume = 0.0
    for a, b, c in tris:
        volume += Vector(verts[a]).dot(Vector(verts[b]).cross(Vector(verts[c]))) / 6.0
    return verts, tris, closed, volume


def export_model(root, out_dir, source):
    """Writes one OBJ per part of the model, named after its game_part. A part must be closed with
    its normals outward (the game has no back faces and starts its shadow rays from the normal):
    nothing is written unless every part is. Returns {part: triangle count}."""
    meshes = []
    for obj in sorted(parts_of(root), key=lambda o: o["game_part"]):
        verts, tris, closed, volume = game_triangles(obj, root)
        if not closed:
            raise ValueError(f"{obj.name}: not closed, every edge must have two faces")
        if volume <= 0:
            raise ValueError(f"{obj.name}: inside out, recalculate its normals")
        meshes.append((obj["game_part"], verts, tris))

    counts = {}
    for part, verts, tris in meshes:
        with open(os.path.join(out_dir, part + ".obj"), "w", newline="\n") as f:
            f.write(f"# {part}: {len(tris)} triangles, made by {source}\n")
            f.write("# game space: +x right on screen, y up, z into the screen\n")
            for v in verts:
                f.write("v %.5f %.5f %.5f\n" % v)
            for a, b, c in tris:
                f.write("f %d %d %d\n" % (a + 1, b + 1, c + 1))
        counts[part] = len(tris)
    return counts
