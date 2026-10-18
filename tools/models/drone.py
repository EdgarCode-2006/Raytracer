"""Bydo drone: a spinning biomechanical eye-mine (R-Type style shmup enemy).

Blender front view (numpad 1) = game view. X right, Z up, -Y towards the game camera.
The game spins the drone 120 deg/s around game z = Blender Y, counter-clockwise on screen,
so everything is built with 3-fold radial symmetry around Blender Y and the three claws trail
the spin like the arms of a pinwheel (convex edge and thorn leading, tips dragged behind), which
is what makes the direction of the spin readable.

Shading: every non-emissive part ends with a live Triangulate modifier (quad method FIXED). The
quads are ordered here so that the fixed diagonal always folds outwards (a ridge, never a valley),
so the creases are chosen by this script, and every triangle is its own flat face: the game's
optional smooth shading renders exactly like the default flat shading.
"""
import math

import bpy
import gametools as gt
from mathutils import Vector

MODEL = "drone"
SCALE = 0.9          # the models.cfg scale (= gameplay size)

COLORS = {
    "body": (180, 85, 45),     # rust (= boss armor)
    "claws": (220, 190, 145),  # bone (= boss bone, fighter spines)
    "eye": (255, 200, 30),     # amber glow (= boss core)
    "iris": (150, 30, 35),     # sinew, NOT emissive (stays a slit in high-contrast mode)
}

FOLD = 3              # claws
SEG = 12              # eye segments (multiple of FOLD)
CLAW_ANGLE = 90.0     # angle (deg, screen, counter-clockwise from +X) of the first claw
PLATE_EDGE = 16.0     # angle between a claw notch and the edge of the next carapace plate


def polar(r, ang_deg, y):
    a = math.radians(ang_deg)
    return (r * math.cos(a), y, r * math.sin(a))


def uniform_angles(seg, phase=0.0):
    return [(phase + 360.0 * k / seg, 0.0, 0.0) for k in range(seg)]


def carapace_angles():
    """Per claw sector: the notch the claw grows from, the two edges of the plate (close to the
    notch, so the groove walls are steep) and the keel in the middle of the plate.
    (angle, notch weight, keel weight)."""
    out = []
    for i in range(FOLD):
        c = CLAW_ANGLE + 360.0 * i / FOLD
        span = 360.0 / FOLD
        out += [(c, 1.0, 0.0), (c + PLATE_EDGE, 0.0, 0.0), (c + span / 2, 0.0, 1.0),
                (c + span - PLATE_EDGE, 0.0, 0.0)]
    return out


def revolve(profile, angles, cap_front=False, cap_back=False):
    """profile: [(r, y), ...] from the front pole (r = 0) to the back pole (r = 0); with
    cap_front / cap_back the first / last ring is closed by a flat cap instead.
    angles: [(angle, notch_w, keel_w)]. A ring entry (r_plate, y_plate, r_notch, y_notch
    [, dr_keel, dy_keel]) gives the plates, the notches between them and the plates' ridges."""
    verts, faces = [], []
    rings = []
    for entry in profile:
        if entry[0] == 0.0:
            rings.append([len(verts)])
            verts.append((0.0, entry[1], 0.0))
            continue
        rp, yp, rc, yc = entry[:4] if len(entry) >= 4 else (*entry, *entry)
        kr, ky = entry[4:6] if len(entry) == 6 else (0.0, 0.0)
        ring = []
        for ang, w, keel in angles:
            ring.append(len(verts))
            verts.append(polar(rp + (rc - rp) * w + kr * keel, ang, yp + (yc - yp) * w + ky * keel))
        rings.append(ring)
    seg = len(angles)
    for a, b in zip(rings, rings[1:]):
        for k in range(seg):
            k1 = (k + 1) % seg
            if len(a) == 1:
                faces.append((a[0], b[k1], b[k]))
            elif len(b) == 1:
                faces.append((a[k], a[k1], b[0]))
            else:
                faces.append((a[k], a[k1], b[k1], b[k]))
    if cap_front:
        faces.append(tuple(reversed(rings[0])))
    if cap_back:
        faces.append(tuple(rings[-1]))
    return verts, faces


def merge(meshes):
    verts, faces = [], []
    for v, f in meshes:
        base = len(verts)
        verts += v
        faces += [tuple(i + base for i in face) for face in f]
    return verts, faces


def outward(mesh):
    """Winds a closed shell outwards (positive signed volume)."""
    verts, faces = mesh
    vol = 0.0
    for f in faces:
        a = Vector(verts[f[0]])
        for i in range(1, len(f) - 1):
            vol += a.dot(Vector(verts[f[i]]).cross(Vector(verts[f[i + 1]])))
    if vol < 0:
        faces = [tuple(reversed(f)) for f in faces]
    return verts, faces


def ridge_creases(mesh):
    """Orders every quad of an outward-wound shell so that the FIXED triangulation (diagonal
    v0-v2) folds it outwards: a ridge that catches the light, never a valley across a plate."""
    verts, faces = mesh
    out = []
    for f in faces:
        if len(f) == 4:
            a, b, c, d = (Vector(verts[i]) for i in f)
            if (b - a).cross(c - a).dot(d - a) > 1e-12:   # d above plane (a, b, c): a-c is a valley
                f = (f[1], f[2], f[3], f[0])
        out.append(f)
    return verts, out


def set_diagonal(f, u, v):
    """The quad f (winding kept) rotated so that u and v are its 1st and 3rd vertices."""
    for k in range(4):
        g = f[k:] + f[:k]
        if {g[0], g[2]} == {u, v}:
            return g
    raise ValueError(f"{u}-{v} is not a diagonal of {f}")


def triangulate(obj):
    """Live Triangulate modifier, last in the stack: the fixed diagonals chosen by ridge_creases,
    one flat face per triangle (the exporter then splits every vertex, smooth = flat)."""
    mod = obj.modifiers.new("triangulate", 'TRIANGULATE')
    mod.quad_method = 'FIXED'
    mod.ngon_method = 'BEAUTY'
    mod.min_vertices = 4
    return mod


# --- parts -----------------------------------------------------------------------------------

# (r, y) of the eye: a flat-topped dome, its rim buried under the eyelids. The eye is emissive
# (one flat color), so its facets never show: the front is a flat 12-gon the pupil lies on exactly,
# the sloped band (r 0.21 -> 0.40) is where the lids cut the eye opening, and a flat back closes
# it inside the body.
EYE_PROFILE = [
    (0.0, -0.40),
    (0.21, -0.40),
    (0.40, -0.17),
]


def eye_front(r):
    """y of the eye's front surface at cone radius r (on the profile's vertex angles)."""
    for (r0, y0), (r1, y1) in zip(EYE_PROFILE, EYE_PROFILE[1:]):
        if r0 <= r <= r1:
            return y0 + (y1 - y0) * (r - r0) / (r1 - r0)
    return EYE_PROFILE[-1][1]


def eye_surface(x, z):
    """y of the eye's real front surface (its flat facets, not the ideal cone) at Blender (x, z)."""
    rho = math.hypot(x, z)
    if rho < 1e-9:
        return EYE_PROFILE[0][1]
    step = 360.0 / SEG
    phi = math.degrees(math.atan2(z, x))
    centre = CLAW_ANGLE + (math.floor((phi - CLAW_ANGLE) / step) + 0.5) * step
    return eye_front(rho * math.cos(math.radians(phi - centre)) / math.cos(math.radians(step / 2)))


def shell():
    """Rust carapace: three hard eyelid plates between the claws, each with a keel ridge, deep
    V grooves where the claws grow out, closed by a low cone at the back.
    Per ring: (r_plate, y_plate, r_notch, y_notch[, dr_keel, dy_keel]).
    The crest is raised at the notches so a rust lip separates each eye corner from its claw."""
    profile = [
        (0.0, -0.14),                    # hidden behind the eye
        (0.34, -0.34, 0.40, -0.235, -0.01, -0.03),  # eyelid crest: overlaps the eye on the plates
        (0.48, -0.27, 0.47, -0.12, 0.0, -0.05),     # lid outer chamfer
        (0.65, -0.10, 0.50, -0.02, 0.03, -0.06),    # front shoulder: plates bulge, notches at the claws
        (0.64, 0.10, 0.50, 0.10, 0.04, 0.0),        # equator band
        (0.46, 0.32, 0.40, 0.30),        # back shoulder
        (0.0, 0.52),                     # back point
    ]
    return ridge_creases(outward(revolve(profile, carapace_angles())))


def eye():
    """Amber eye: flat front 12-gon, sloped band, flat back (44 triangles)."""
    return outward(revolve(EYE_PROFILE[1:], uniform_angles(SEG, CLAW_ANGLE), cap_front=True, cap_back=True))


IRIS_H = 0.20        # half-length: the whole slit lies on the eye's flat front (r < 0.21)
IRIS_STATIONS = [(-0.12, 0.078), (0.0, 0.124), (0.12, 0.078)]   # (z, half-width): a pointed lens
IRIS_PROUD = 0.013   # ridge above the eye: low, so both front facets catch about the same light
IRIS_SINK = 0.016    # side edges (and the flat back) below the eye: a clean intersection, and the
                     # hidden back stays > 0.015 behind the eye's front (no near-coplanar faces)
IRIS_TIP = 0.010     # tips above the eye: the spindle ends on clean points


def iris():
    """Vertical slit pupil: a lens with a 3-sided section, two front facets meeting on a ridge
    and a flat back buried in the eye, its tips standing proud of the eye."""
    verts = [(0.0, eye_surface(0.0, -IRIS_H) - IRIS_TIP, -IRIS_H),
             (0.0, eye_surface(0.0, IRIS_H) - IRIS_TIP, IRIS_H)]
    for z, hw in IRIS_STATIONS:
        verts += [(-hw, eye_surface(-hw, z) + IRIS_SINK, z),     # left edge
                  (0.0, eye_surface(0.0, z) - IRIS_PROUD, z),    # front ridge
                  (hw, eye_surface(hw, z) + IRIS_SINK, z)]       # right edge
    faces = []
    for s in range(len(IRIS_STATIONS) - 1):
        a, b = 2 + 3 * s, 2 + 3 * (s + 1)
        for k in range(3):
            k1 = (k + 1) % 3
            faces.append((a + k, a + k1, b + k1, b + k))
    first, last = 2, 2 + 3 * (len(IRIS_STATIONS) - 1)
    for k in range(3):
        faces.append((first + k, 0, first + (k + 1) % 3))
        faces.append((last + k, last + (k + 1) % 3, 1))
    verts, faces = outward((verts, faces))
    # front facets: diagonal from the inner station's edge to the outer station's ridge, so the
    # line where the facet leaves the eye bulges outwards (a lens) instead of pinching
    out = []
    for f in faces:
        ridge = [i for i in f if verts[i][0] == 0.0]
        edge = [i for i in f if verts[i][0] != 0.0]
        if len(f) == 4 and len(ridge) == 2:
            inner = min(edge, key=lambda i: abs(verts[i][2]))
            outer = max(ridge, key=lambda i: abs(verts[i][2]))
            f = set_diagonal(f, inner, outer)
        out.append(f)
    return verts, out


CLAW_R0, CLAW_R1 = 0.44, 0.98      # root and tip radius
CLAW_SWEEP = 58.0                  # degrees the tip trails behind the root
CLAW_STATIONS = [
    # t,    width, lead share, half-depth, y
    (0.00, 0.30, 0.45, 0.12, -0.08),
    (0.25, 0.36, 0.50, 0.115, -0.10),
    (0.50, 0.31, 0.55, 0.095, -0.13),
    (0.75, 0.24, 0.60, 0.075, -0.165),
    (0.90, 0.15, 0.60, 0.05, -0.20),
    (0.96, 0.095, 0.60, 0.032, -0.222),  # blunt talon: >= 3 px wide at game scale near the point
]
CLAW_TIP_Y = -0.24
THORN_STATION = 2      # a thorn on the leading (convex) edge of this station


def claw():
    """One sickle claw growing from a groove at the corner of the eye, tip trailing the spin
    (clockwise on screen), with a thorn on its convex edge. Returns (claw, thorn).
    Cross-section: a diamond whose ridge sits towards the trailing (concave) edge, so the long
    bevel of the leading (convex) edge and the short trailing bevel flash in turn as it spins."""

    def center(t):
        r = CLAW_R0 + (CLAW_R1 - CLAW_R0) * t
        ang = CLAW_ANGLE - CLAW_SWEEP * t ** 1.7
        return polar(r, ang, 0.0), ang

    verts, faces = [], []
    thorn = None
    for si, (t, width, lead, depth, y) in enumerate(CLAW_STATIONS):
        (cx, _, cz), ang = center(t)
        # in-plane normal pointing to the leading side (counter-clockwise = +angle)
        (px, _, pz), _ = center(min(1.0, t + 0.01))
        tx, tz = px - cx, pz - cz
        ln = math.hypot(tx, tz)
        tx, tz = tx / ln, tz / ln
        nx, nz = -tz, tx            # rotate tangent +90 deg: leading side
        ridge = (lead - 0.5) * width * 0.6   # ridge moved towards the trailing edge
        lw, tw = lead * width, (1.0 - lead) * width
        verts += [
            (cx + nx * lw, y, cz + nz * lw),                        # leading edge
            (cx - nx * ridge, y - depth, cz - nz * ridge),          # front ridge
            (cx - nx * tw, y, cz - nz * tw),                        # trailing edge
            (cx - nx * ridge, y + depth, cz - nz * ridge),          # back ridge
        ]
        if si == THORN_STATION:
            ex, ez = cx + nx * lw, cz + nz * lw          # leading edge point
            bx, bz = ex - nx * 0.06, ez - nz * 0.06      # root, buried in the claw
            dx, dz = nx + 0.7 * tx, nz + 0.7 * tz        # outwards, raked towards the tip
            dl = math.hypot(dx, dz)
            thorn = ([
                (bx - tx * 0.075, y, bz - tz * 0.075),
                (bx, y - depth * 0.25, bz),
                (bx + tx * 0.075, y, bz + tz * 0.075),
                (bx, y + depth * 0.25, bz),
                (ex + dx / dl * 0.16, y - 0.02, ez + dz / dl * 0.16),
            ], [(k, (k + 1) % 4, 4) for k in range(4)] + [(3, 2, 1, 0)])
    (tx_, _, tz_), _ = center(1.0)
    verts.append((tx_, CLAW_TIP_Y, tz_))
    tip = len(verts) - 1
    ns = len(CLAW_STATIONS)
    for s in range(ns - 1):
        a, b = 4 * s, 4 * (s + 1)
        for k in range(4):
            k1 = (k + 1) % 4
            faces.append((a + k, a + k1, b + k1, b + k))
    last = 4 * (ns - 1)
    for k in range(4):
        faces.append((last + k, last + (k + 1) % 4, tip))
    faces.append((3, 2, 1, 0))
    return ridge_creases(outward((verts, faces))), ridge_creases(outward(thorn))


def claw_unit():
    """What the Array modifier repeats FOLD times: a claw and its thorn."""
    return merge(claw())


# --- contract --------------------------------------------------------------------------------

def build(collection, location=(0.0, 0.0, 0.0)):
    """Builds the drone in `collection` with its root Empty at `location`. Returns [root].
    Never resets the scene: safe to run in a live session (names get .001 suffixes)."""
    root = gt.new_root(MODEL, location, collection)
    mats = {k: gt.make_material(f"{MODEL}_{k}", c, 1.0 if k == "eye" else 0.0) for k, c in COLORS.items()}
    body = gt.mesh_object(f"{MODEL}_body", *shell(), mats["body"], root, collection=collection)
    triangulate(body)
    claws = gt.mesh_object(f"{MODEL}_claws", *claw_unit(), mats["claws"], root, collection=collection)
    # radial symmetry kept live: edit one claw, the Array repeats it around Blender Y
    pivot = bpy.data.objects.new(f"{MODEL}_claws_step", None)
    pivot.empty_display_type = 'ARROWS'
    pivot.empty_display_size = 0.25
    pivot.rotation_euler = (0.0, math.radians(360.0 / FOLD), 0.0)
    collection.objects.link(pivot)
    pivot.parent = root
    array = claws.modifiers.new("radial", 'ARRAY')
    array.count = FOLD
    array.use_relative_offset = False
    array.use_object_offset = True
    array.offset_object = pivot
    triangulate(claws)                      # after the Array
    gt.mesh_object(f"{MODEL}_eye", *eye(), mats["eye"], root, collection=collection)
    iris_obj = gt.mesh_object(f"{MODEL}_iris", *iris(), mats["iris"], root, collection=collection)
    triangulate(iris_obj)
    return [root]
