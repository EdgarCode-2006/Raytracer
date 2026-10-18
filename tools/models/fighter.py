"""Bydo "fighter": a fast insectoid interceptor that flies LEFT (face towards -X).

Blender Front view (numpad 1) is the game view: X right, Z up, -Y towards the camera.
Parts: fighter_body (chitin), fighter_carapace (bone helmet, cheek ridges, mandibles, scythe
blades, mantis claws, dorsal tergite plates, nozzle claws), fighter_glow (emissive red eyes, maw
and thruster).
Every part is modelled as its camera-side half (-Y) and closed by a live Mirror modifier (Y).
"""
import math
import bpy
import bmesh
from mathutils import Vector
import gametools as gt

MODEL = "fighter"
SCALE = 1.1  # models.cfg scale = gameplay size

COLORS = {
    "body": ((150, 65, 60), 0.0),        # chitin
    "carapace": ((220, 190, 145), 0.0),  # bone
    "glow": ((255, 45, 40), 1.0),        # hostile red
}

OFFSET = Vector((0.07, 0.0, -0.04))  # recentres the model on its visual centre (body mass)
X = Vector((1.0, 0.0, 0.0))
HEX = [(math.cos(math.radians(a)), math.sin(math.radians(a))) for a in range(30, 390, 60)]
# helmet: a roof ridge on top, wide shoulders, keel buried in the head (vertices on y = 0 at
# top and bottom, so the mirror plane never splits a face)
ROOF = [(0.0, 1.0), (1.0, 0.15), (0.7, -0.8), (0.0, -1.0), (-0.7, -0.8), (-1.0, 0.15)]
# body: the widest point sits below the equator, so the big camera-side flank faces up towards
# the upper-left light instead of half of it facing down
HIGH8 = [(1.0, -0.3), (0.72, 0.62), (0.0, 1.0), (-0.72, 0.62), (-1.0, -0.3), (-0.62, -0.82),
         (0.0, -1.0), (0.62, -0.82)]
# dorsal tergite plate: proud on the back and down the camera flank, its belly vertex sunk in the
# body (open at the belly: an armour segment, not a machined collar, and no dark rim on the belly)
TERGITE = [(0.0, 1.0), (0.9, 0.45), (1.0, -0.45), (0.0, -0.8), (-1.0, -0.45), (-0.9, 0.45)]
DIAMOND = [(1.0, 0.0), (0.0, 1.0), (-1.0, 0.0), (0.0, -1.0)]
TRI = [(0.0, 1.0), (0.87, -0.5), (-0.87, -0.5)]


class Geo:
    """Vertex/face accumulator (Blender coordinates, relative to the model root)."""

    def __init__(self):
        self.verts = []
        self.faces = []

    def add(self, co):
        self.verts.append(Vector(co) + OFFSET)
        return len(self.verts) - 1

    def loft(self, rings, start_tip=None, end_tip=None):
        """Skins rings of equal size; each end is closed by a cap (ngon) or a tip vertex."""
        ids = [[self.add(v) for v in r] for r in rings]
        n = len(rings[0])
        for a, b in zip(ids, ids[1:]):
            for k in range(n):
                self.faces.append((a[k], a[(k + 1) % n], b[(k + 1) % n], b[k]))
        if start_tip is None:
            self.faces.append(tuple(reversed(ids[0])))
        else:
            t = self.add(start_tip)
            for k in range(n):
                self.faces.append((ids[0][(k + 1) % n], ids[0][k], t))
        if end_tip is None:
            self.faces.append(tuple(ids[-1]))
        else:
            t = self.add(end_tip)
            for k in range(n):
                self.faces.append((ids[-1][k], ids[-1][(k + 1) % n], t))


def ring_x(x, z, rw, rh, profile, y=0.0):
    """Cross-section in the YZ plane (profile y+ = camera side -Y)."""
    return [Vector((x, y - rw * py, z + rh * pz)) for py, pz in profile]


def ring_frame(center, tangent, up, rw, rh, profile):
    t = tangent.normalized()
    side = t.cross(up).normalized()
    u = side.cross(t).normalized()
    return [center + side * (rw * py) + u * (rh * pz) for py, pz in profile]


def tube_x(g, sections, profile, start_tip=None, end_tip=None):
    """sections: (x, z, half-width, half-height)."""
    g.loft([ring_x(x, z, rw, rh, profile) for x, z, rw, rh in sections],
           start_tip and Vector(start_tip), end_tip and Vector(end_tip))


def tube_path(g, path, up, profile, end_tip):
    """path: (center, half-width, half-height); ends on a tip."""
    pts = [Vector(p) for p, _, _ in path]
    rings = []
    for i, (p, rw, rh) in enumerate(path):
        a = pts[max(i - 1, 0)]
        b = pts[min(i + 1, len(pts) - 1)] if i + 1 < len(pts) else Vector(end_tip)
        rings.append(ring_frame(Vector(p), b - a, up, rw, rh, profile))
    g.loft(rings, None, Vector(end_tip))


def blade(g, origin, span_dir, sections, tip, ridge=0.4):
    """A diamond-section blade in the plane (X, span_dir): sections (v, u_lead, u_trail, thickness)
    with chords along X, ending on tip (v, u)."""
    o = Vector(origin)
    s = Vector(span_dir).normalized()
    n = X.cross(s).normalized()
    rings = []
    for v, ule, ute, th in sections:
        base = o + s * v
        le, te = base + X * ule, base + X * ute
        r = le.lerp(te, ridge)
        rings.append([le, r + n * (th / 2), te, r - n * (th / 2)])
    g.loft(rings, None, o + s * tip[0] + X * tip[1])


def lens(g, center, along, up, out, a, b, c, back=None):
    """Octahedron: half-lengths a (along), b (up), c (out of the surface), back (into it)."""
    cen = Vector(center)
    al, upv, ou = Vector(along).normalized(), Vector(up).normalized(), Vector(out).normalized()
    back = c if back is None else back
    p = [cen - al * a, cen + upv * b, cen + al * a, cen - upv * b]
    top, bot = cen + ou * c, cen - ou * back
    ids = [g.add(v) for v in p]
    t, d = g.add(top), g.add(bot)
    for k in range(4):
        g.faces.append((ids[k], ids[(k + 1) % 4], t))
        g.faces.append((ids[(k + 1) % 4], ids[k], d))


def sdir(deg, down=False):
    """Span direction leaving the body on the camera side, deg above (or below) horizontal."""
    r = math.radians(deg)
    return Vector((0.0, -math.cos(r), -math.sin(r) if down else math.sin(r)))


# --- the parts ---

def body_geo():
    g = Geo()
    tube_x(g, [
        (-0.80, -0.06, 0.11, 0.11),   # face, around the maw
        (-0.68, -0.02, 0.22, 0.24),   # cheeks
        (-0.46, 0.02, 0.25, 0.27),    # back of the head
        (-0.36, 0.02, 0.19, 0.22),    # neck
        (-0.16, 0.02, 0.27, 0.33),    # thorax
        (0.10, 0.01, 0.25, 0.29),     # abdomen, tapering to the nozzle
        (0.66, -0.02, 0.19, 0.20),    # rim
        (0.62, -0.02, 0.12, 0.12),    # recessed nozzle
    ], HIGH8)
    return g


def tergite(g, x0, x1, z0, z1, front, back):
    """A dorsal bone plate on the abdomen: (half-width, half-height) tight at the front, standing
    proud at the back, its belly edge sunk into the body."""
    tube_x(g, [(x0, z0) + front, (x1, z1) + back], TERGITE)


def spindle(g, start, rings, end, up, profile):
    """Rings (center, half-width, half-height) between two tips, framed along the path."""
    pts = [Vector(start)] + [Vector(c) for c, _, _ in rings] + [Vector(end)]
    frames = [ring_frame(pts[i + 1], pts[i + 2] - pts[i], up, rw, rh, profile)
              for i, (_, rw, rh) in enumerate(rings)]
    g.loft(frames, Vector(start), Vector(end))


def carapace_geo():
    g = Geo()
    # helmet: hooked beak over the maw, brow over the eye, crest swept back
    tube_x(g, [
        (-0.88, 0.00, 0.10, 0.10),
        (-0.68, 0.14, 0.29, 0.16),
        (-0.46, 0.23, 0.30, 0.15),
        (-0.32, 0.27, 0.16, 0.09),
    ], ROOF, start_tip=(-1.03, 0.0, -0.11), end_tip=(-0.12, 0.0, 0.44))
    # cheek ridge (camera side): a bone blade under the eye, parallel to its slant, between
    # the eye and the maw, so the red eye sits in a bone frame (brow above, ridge below)
    spindle(g, (-0.38, -0.16, -0.03), [((-0.58, -0.22, -0.12), 0.075, 0.075)],
            (-0.78, -0.21, -0.08), Vector((0, -1, 0)), TRI)
    # mandible (camera side), curving forward and up under the maw
    tube_path(g, [
        ((-0.46, -0.16, -0.19), 0.09, 0.09),
        ((-0.70, -0.22, -0.31), 0.085, 0.09),
        ((-0.92, -0.19, -0.30), 0.065, 0.065),
    ], Vector((0, 0, 1)), DIAMOND, end_tip=(-1.07, -0.11, -0.17))
    # upper scythe blade
    blade(g, (-0.20, -0.13, 0.19), sdir(64), [
        (0.00, -0.12, 0.26, 0.10),
        (0.30, 0.04, 0.34, 0.08),
        (0.60, 0.25, 0.46, 0.06),
    ], (0.86, 0.66))
    # lower mantis claw: down from the thorax, then hooking forward
    blade(g, (-0.02, -0.13, -0.21), sdir(58, down=True), [
        (0.00, -0.14, 0.16, 0.09),
        (0.26, -0.02, 0.20, 0.075),
        (0.44, -0.10, 0.12, 0.06),
    ], (0.58, -0.30), ridge=0.5)
    # abdomen segments (wasp stripes): dorsal tergite plates
    tergite(g, 0.16, 0.30, 0.01, 0.005, (0.24, 0.27), (0.27, 0.30))
    tergite(g, 0.38, 0.52, 0.0, -0.005, (0.215, 0.245), (0.25, 0.27))
    # claws around the nozzle: one on top, one low on each side
    tube_path(g, [((0.52, 0.0, 0.12), 0.08, 0.08), ((0.70, 0.0, 0.22), 0.075, 0.075)],
              Vector((0, 0, 1)), TRI, end_tip=(0.92, 0.0, 0.34))
    tube_path(g, [((0.52, -0.09, -0.10), 0.08, 0.08), ((0.70, -0.15, -0.19), 0.075, 0.075)],
              Vector((0, 0, 1)), TRI, end_tip=(0.92, -0.19, -0.31))
    return g


def glow_geo():
    g = Geo()
    # big compound eye on the side of the head, slanted (angry): outer corner up and back
    lens(g, (-0.60, -0.235, 0.01), (1.0, 0.0, 0.4), (-0.4, 0.0, 1.0), (0.0, -1.0, 0.0),
         0.18, 0.08, 0.05, 0.07)
    # maw core between beak and mandibles
    tube_x(g, [(-0.86, -0.12, 0.11, 0.10)], HEX, start_tip=(-0.98, 0.0, -0.13), end_tip=(-0.74, 0.0, -0.10))
    # thruster: a blunt flame bulb in the nozzle
    tube_x(g, [(0.58, -0.02, 0.13, 0.13), (0.74, -0.02, 0.19, 0.19)], HEX, end_tip=(0.86, 0.0, -0.02))
    return g


def make_part(name, geo, root, collection):
    rgb, emission = COLORS[name]
    part = MODEL + "_" + name
    bm = bmesh.new()
    vs = [bm.verts.new(v) for v in geo.verts]
    for f in geo.faces:
        bm.faces.new([vs[i] for i in f])
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    # keep the camera side only: the Mirror modifier rebuilds the far side
    bmesh.ops.bisect_plane(bm, geom=bm.verts[:] + bm.edges[:] + bm.faces[:], dist=1e-6,
                           plane_co=(0, 0, 0), plane_no=(0, 1, 0), clear_outer=True)
    me = bpy.data.meshes.new(part)
    bm.to_mesh(me)
    bm.free()
    for p in me.polygons:
        p.use_smooth = False
    obj = bpy.data.objects.new(part, me)
    collection.objects.link(obj)
    obj.parent = root
    me.materials.append(gt.make_material(part, rgb, emission))
    obj["game_part"] = part
    mod = obj.modifiers.new("Mirror Y", 'MIRROR')
    mod.use_axis = (False, True, False)
    mod.use_mirror_merge = True
    mod.merge_threshold = 1e-4
    return obj


def build(collection, location=(0.0, 0.0, 0.0)):
    root = gt.new_root(MODEL, location, collection)
    make_part("body", body_geo(), root, collection)
    make_part("carapace", carapace_geo(), root, collection)
    make_part("glow", glow_geo(), root, collection)
    return [root]
