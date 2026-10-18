"""Player ship "R-13 Lancet" (Arrowhead spirit): one mesh set, 4 team color themes.

Parts (one flat color each, set per theme in models.cfg):
  player_hull   light neutral body: nose cone, fuselage, engine block, recessed nozzle bell
  player_trim   team color: dorsal fin, anhedral wings, nose band, upper and lower engine cowl
                panels, cheatline
  player_canopy cockpit glass (emission 1)
  player_engine exhaust core + plume (one shell out of the nozzle bell), vent strip on the white
                band between the cowl panels, wing tip strips (emission 1)

The plume tip reaches x = -1.30, past the -1.0 half-extent of the body: it is an effect, not
hull (the gameplay size only counts the body).

Blender: X = nose direction (game +x), Z = up, -Y = towards the game camera.
Every part is modelled as its -Y half and closed by a live Mirror modifier (axis Y, clip + merge):
edit one side, the other follows. Shells that straddle Y = 0 are left open on that plane on purpose.
"""
import bpy
import bmesh
import math
from mathutils import Vector
import gametools as gt

MODEL = "player"
SCALE = 1.3          # the models.cfg scale (= gameplay size)

# The team colors, the same as the player themes of models.cfg. All cool (the enemies are warm),
# told apart by hue AND lightness (Aqua lightest, Rose and Azure darkest) so that they stay apart
# for the common color vision deficiencies. The hull is a light neutral, the trim the team color.
THEMES = [
    {"id": "p1", "name": "Azure", "hull": (212, 226, 245), "trim": (30, 115, 240),
     "canopy": (180, 228, 255), "engine": (100, 180, 255), "contrast": (0, 215, 255)},
    {"id": "p2", "name": "Aqua", "hull": (185, 200, 206), "trim": (135, 255, 240),
     "canopy": (205, 255, 245), "engine": (40, 255, 200), "contrast": (30, 220, 150)},
    {"id": "p3", "name": "Rose", "hull": (245, 225, 238), "trim": (215, 30, 140),
     "canopy": (255, 180, 225), "engine": (255, 10, 155), "contrast": (250, 10, 155)},
    {"id": "p4", "name": "Lilac", "hull": (242, 242, 250), "trim": (200, 175, 255),
     "canopy": (220, 200, 255), "engine": (140, 95, 255), "contrast": (125, 120, 255)},
]
PARTS = [("hull", 0.0), ("trim", 0.0), ("canopy", 1.0), ("engine", 1.0)]


# ---------------------------------------------------------------- geometry helpers

class Builder:
    """Collects the -Y half of a part in one bmesh. Each helper builds one shell and orients it
    outward with its signed volume, measured from the origin, which lies on the mirror plane:
    the missing faces on Y = 0 add nothing, so a half shell gets the sign of the whole."""

    def __init__(self):
        self.bm = bmesh.new()
        self._mark = 0

    def v(self, p):
        return self.bm.verts.new(Vector(p))

    def face(self, vs):
        vs = [x for i, x in enumerate(vs) if x is not vs[i - 1]]  # drop collapsed corners
        if len(vs) >= 3:
            self.bm.faces.new(vs)

    def _orient(self):
        self.bm.faces.ensure_lookup_table()
        faces = list(self.bm.faces)[self._mark:]
        vol = 0.0
        for f in faces:
            p = [v.co for v in f.verts]
            for i in range(1, len(p) - 1):
                vol += p[0].dot(p[i].cross(p[i + 1]))
        if vol < 0:
            bmesh.ops.reverse_faces(self.bm, faces=faces)
        self._mark = len(self.bm.faces)

    def loft(self, sections, cap_start=True, cap_end=True, closed=False, strip_caps=False):
        """Sections ordered along the shell, all with the same number of points (or ONE point:
        a tip). Open sections (closed=False) run from Y = 0 round the -Y side back to Y = 0: the
        shell stays open on the mirror plane. closed=True: full rings (shells off the plane).
        strip_caps: rings made of m outer points then the m inner ones reversed (a panel): the
        end caps become a strip of quads across the panel's thickness instead of one n-gon,
        which avoids long thin triangles."""
        rings = [[self.v(p) for p in s] for s in sections]
        for a, b in zip(rings, rings[1:]):
            n = max(len(a), len(b))
            A = a * n if len(a) == 1 else a
            B = b * n if len(b) == 1 else b
            for i in range(n if closed else n - 1):
                j = (i + 1) % n
                self.face([A[i], B[i], B[j], A[j]])
        for ring, on, flip in ((rings[0], cap_start, False), (rings[-1], cap_end, True)):
            if not on or len(ring) < 3:
                continue
            if strip_caps:
                m = len(ring) // 2
                quads = [[ring[i], ring[i + 1], ring[-2 - i], ring[-1 - i]] for i in range(m - 1)]
            else:
                quads = [ring]
            for q in quads:
                self.face(list(reversed(q)) if flip else q)
        self._orient()
        return rings

    def blade(self, outline, origin, U, V, t, bevel):
        """Closed blade off the mirror plane: `outline` 2D (u, v) convex polygon in the plane
        origin + u U + v V; both flat faces inset by `bevel`, offset +-t/2 along U x V, so every
        edge is a 2-facet chamfer meeting at a sharp mid-plane rim."""
        origin, U, V = Vector(origin), Vector(U).normalized(), Vector(V).normalized()
        N = U.cross(V).normalized()
        ins = inset2d(outline, bevel)
        mid = [self.v(origin + u * U + w * V) for u, w in outline]
        top = [self.v(origin + u * U + w * V + N * t / 2) for u, w in ins]
        bot = [self.v(origin + u * U + w * V - N * t / 2) for u, w in ins]
        self.face(top)
        self.face(list(reversed(bot)))
        n = len(outline)
        for i in range(n):
            j = (i + 1) % n
            self.face([top[j], top[i], mid[i], mid[j]])
            self.face([mid[j], mid[i], bot[i], bot[j]])
        self._orient()

    def half_blade(self, outline_xz, t, bevel):
        """-Y half of a blade lying in the mirror plane (a centred fin): the sharp rim is the
        open boundary on Y = 0, the flat face sits at Y = -t."""
        ins = inset2d(outline_xz, bevel)
        rim = [self.v((x, 0.0, z)) for x, z in outline_xz]
        side = [self.v((x, -t, z)) for x, z in ins]
        self.face(side)
        n = len(outline_xz)
        for i in range(n):
            j = (i + 1) % n
            self.face([side[j], side[i], rim[i], rim[j]])
        self._orient()

    def box(self, corners):
        """8 corners: 4 of one end then the 4 matching ones of the other end (closed shell)."""
        a = [self.v(p) for p in corners[:4]]
        b = [self.v(p) for p in corners[4:]]
        self.face(a)
        self.face(list(reversed(b)))
        for i in range(4):
            j = (i + 1) % 4
            self.face([a[i], b[i], b[j], a[j]])
        self._orient()

    def finish(self, name):
        me = bpy.data.meshes.new(name)
        bmesh.ops.remove_doubles(self.bm, verts=self.bm.verts, dist=1e-6)
        # a warped quad/ngon gets ONE normal in Blender, so with the game's smooth shading on, its
        # triangles would be lit with that averaged normal instead of their own: split the faces
        # bent by more than 1 degree into triangles (same triangle count after export)
        self.bm.normal_update()
        warped = []
        for f in self.bm.faces:
            if len(f.verts) > 3:
                p = [v.co for v in f.verts]
                for i in range(1, len(p) - 1):
                    t = (p[i] - p[0]).cross(p[i + 1] - p[0])
                    if t.length > 1e-12 and f.normal.angle(t) > math.radians(1.0):
                        warped.append(f)
                        break
        if warped:
            bmesh.ops.triangulate(self.bm, faces=warped, quad_method='BEAUTY', ngon_method='BEAUTY')
        self.bm.to_mesh(me)
        self.bm.free()
        for p in me.polygons:
            p.use_smooth = False
        return me


def inset2d(poly, d):
    """Insets a convex 2D polygon by d (any winding)."""
    n = len(poly)
    area = sum(poly[i][0] * poly[(i + 1) % n][1] - poly[(i + 1) % n][0] * poly[i][1] for i in range(n))
    s = 1.0 if area > 0 else -1.0
    lines = []
    for i in range(n):
        (x0, y0), (x1, y1) = poly[i], poly[(i + 1) % n]
        dx, dy = x1 - x0, y1 - y0
        L = math.hypot(dx, dy)
        nx, ny = -dy / L * s, dx / L * s   # inward normal
        lines.append(((x0 + nx * d, y0 + ny * d), (dx, dy)))
    out = []
    for i in range(n):
        (p, r), (q, s2) = lines[i - 1], lines[i]
        den = r[0] * s2[1] - r[1] * s2[0]
        t = ((q[0] - p[0]) * s2[1] - (q[1] - p[1]) * s2[0]) / den
        out.append((p[0] + r[0] * t, p[1] + r[1] * t))
    return out


# ---------------------------------------------------------------- the ship

# fuselage stations: x, ridge z, keel z, half width
HULL = [
    (0.80, 0.030, -0.075, 0.055),
    (0.58, 0.075, -0.125, 0.105),
    (0.32, 0.115, -0.160, 0.145),
    (0.02, 0.150, -0.180, 0.175),
    (-0.32, 0.175, -0.190, 0.200),
    (-0.42, 0.255, -0.265, 0.270),   # engine block flare
    (-0.82, 0.262, -0.272, 0.276),
]
NOSE_TIP = (1.00, 0.0, -0.025)

# nozzle: the engine block tapers into a round bell (hull) whose recess holds the glowing core;
# the core leaves the bell as a flame cone on the same axis (engine part)
NOZZLE_Z = -0.005            # axis height (game y)
BELL_X, BELL_R = -0.95, 0.21     # lip plane, outer radius
BELL_RI, BELL_FLOOR = 0.168, -0.91  # inner radius (lip 0.042 thick), recess floor
# exhaust core + plume rings (x, radius) and tip: root buried in the block in front of the
# floor, 0.014 clear of the recess wall, then a convex flame cone out of the lip
PLUME = [(-0.88, 0.155), (-0.99, 0.150), (-1.09, 0.120)]
PLUME_TIP = -1.30

# wing: anhedral steep enough to show its lit top from every height of the screen (the
# camera sees ships up to 30 deg above / below); outline (chord u along X, span v along WING_V)
WING_ANHEDRAL = math.radians(40)
WING_ROOT = Vector((0.0, -0.13, -0.10))
WING_V = Vector((0.0, -math.cos(WING_ANHEDRAL), -math.sin(WING_ANHEDRAL)))
WING_N = Vector((1.0, 0.0, 0.0)).cross(WING_V)
WING_OUTLINE = [(-0.10, 0.0), (-0.86, 0.0), (-0.97, 0.60), (-0.74, 0.60)]
WING_T = 0.05


def wing_point(u, v, n=0.0):
    return tuple(WING_ROOT + Vector((u, 0.0, 0.0)) + WING_V * v + WING_N * n)


def hull_station(x):
    """Ridge/keel/width interpolated along the fuselage."""
    for a, b in zip(HULL, HULL[1:]):
        if b[0] <= x <= a[0]:
            t = (a[0] - x) / (a[0] - b[0])
            return tuple(a[k] + (b[k] - a[k]) * t for k in range(4))
    raise ValueError(x)


def hull_points(zt, zb, w):
    """Half section (y, z): ridge -> top shoulder -> 45 deg chamfer -> flat side -> lower
    chamfer -> keel."""
    h = zt - zb
    return [
        (0.0, zt),
        (-0.62 * w, zt - 0.06 * h),
        (-w, zt - 0.06 * h - 0.38 * w),
        (-w, zb + 0.30 * h),
        (-0.55 * w, zb + 0.04 * h),
        (0.0, zb),
    ]


def hull_section(x, zt, zb, w):
    return [(x, y, z) for y, z in hull_points(zt, zb, w)]


def hull_point(x, k, grow=0.0):
    """Point k of the hull section at x, pushed `grow` outward (negative: inward). A fractional
    k lies on the facet between points int(k) and int(k) + 1."""
    _, zt, zb, w = hull_station(x)
    pts = hull_points(zt, zb, w)
    if k != int(k):
        i, f = int(k), k - int(k)
        (y0, z0), (y1, z1) = pts[i], pts[i + 1]
        d = Vector((y1 - y0, z1 - z0)).normalized()
        return (x, y0 + (y1 - y0) * f + d.y * grow, z0 + (z1 - z0) * f - d.x * grow)
    k = int(k)
    y, z = pts[k]
    if k in (0, 5):                       # ridge / keel: straight up / down
        n = Vector((0.0, 1.0 if k == 0 else -1.0))
    else:                                 # average of the two facet normals meeting at k
        n = Vector((0.0, 0.0))
        for (y0, z0), (y1, z1) in ((pts[k - 1], pts[k]), (pts[k], pts[k + 1])):
            d = Vector((y1 - y0, z1 - z0)).normalized()
            n += Vector((d.y, -d.x))
        n.normalize()
    return (x, y + n.x * grow, z + n.y * grow)


def bell_ring(x, r):
    """Half of a 10-sided ring around the nozzle axis, ridge to keel like a hull section."""
    pts = []
    for a in (90, 54, 18, -18, -54, -90):
        c, s = math.cos(math.radians(a)), math.sin(math.radians(a))
        pts.append((x, 0.0 if abs(a) == 90 else -r * c, NOZZLE_Z + r * s))
    return pts


def build_hull():
    b = Builder()
    # nose -> fuselage -> engine block -> boat-tail into the bell lip -> lip face -> recess
    b.loft([[NOSE_TIP]] + [hull_section(*s) for s in HULL]
           + [bell_ring(BELL_X, BELL_R), bell_ring(BELL_X, BELL_RI), bell_ring(BELL_FLOOR, BELL_RI)],
           cap_start=False, cap_end=True)
    return b


def sleeve(b, xs, grow, ks=range(6)):
    """A band hugging the hull between the given x stations, `grow` proud of it. With every
    point (ks = 0..5) it wraps over the mirror plane; with a subset it is a closed panel."""
    ks = list(ks)
    if ks == list(range(6)):
        b.loft([[hull_point(x, k, grow) for k in ks] for x in xs])
    else:
        b.loft([[hull_point(x, k, grow) for k in ks] + [hull_point(x, k, -0.03) for k in reversed(ks)]
                for x in xs], closed=True)


COWL_X = (-0.45, -0.80)
COWL_GROW = 0.014
VENT_BAND = (-0.035, 0.075)      # white band on the flat side between the cowl panels (z)
VENT_Z = (-0.010, 0.050)         # glow strip inside it: >= 0.025 of white above and below
VENT_X = (-0.50, -0.78)          # inside the cowl's x range, clear of its end caps


def flat_k(x, z):
    """Fractional section index of height z on the flat side (between points 2 and 3)."""
    _, zt, zb, w = hull_station(x)
    h = zt - zb
    z2, z3 = zt - 0.06 * h - 0.38 * w, zb + 0.30 * h
    return 2.0 + (z2 - z) / (z2 - z3)


def build_trim():
    b = Builder()
    # dorsal fin (centred, half blade), root buried in the spine / engine block
    b.half_blade([(-0.28, 0.13), (-0.72, 0.64), (-0.93, 0.64), (-0.86, 0.19)], t=0.035, bevel=0.03)
    # anhedral swept wing, -Y side (the mirror makes the other one)
    b.blade(WING_OUTLINE, WING_ROOT, (1, 0, 0), WING_V, t=WING_T, bevel=0.03)
    # nose band
    sleeve(b, (0.66, 0.58, 0.53), 0.014)
    # engine cowl: the sides of the engine block in two panels (ridge and keel left white), with
    # a white band between them that frames the glowing vent strip (engine part)
    for ks in (lambda x: (1, 2, flat_k(x, VENT_BAND[1])), lambda x: (flat_k(x, VENT_BAND[0]), 3, 4)):
        b.loft([[hull_point(x, k, COWL_GROW) for k in ks(x)] + [hull_point(x, k, -0.03) for k in reversed(ks(x))]
                for x in COWL_X], closed=True, strip_caps=True)
    # cheatline: low on the flat side, from under the nose band to the intake step, widening
    # backwards. 3 stations; the grow keeps it >= 0.006 proud where it spans the 0.32 crease.
    stripe = []
    for x, k0, k1, g in ((0.545, 2.50, 2.86, 0.010), (0.02, 2.40, 2.93, 0.018), (-0.32, 2.32, 2.95, 0.012)):
        out = [hull_point(x, k0, g), hull_point(x, k1, g)]
        stripe.append(out + [hull_point(x, k1, -0.03), hull_point(x, k0, -0.03)])
    b.loft(stripe, closed=True)
    return b


def build_canopy():
    b = Builder()
    # x, top z, half width
    stations = [(0.34, 0.190, 0.075), (0.14, 0.255, 0.092), (-0.06, 0.245, 0.085), (-0.20, 0.200, 0.055)]
    secs = []
    for x, zt, w in stations:
        zh = hull_station(x)[1]
        secs.append([(x, 0.0, zt), (x, -0.6 * w, zt - 0.35 * (zt - zh)), (x, -w, zh - 0.035), (x, 0.0, zh - 0.06)])
    front = (0.50, 0.0, hull_station(0.50)[1] - 0.03)
    back = (-0.32, 0.0, hull_station(-0.32)[1] - 0.03)
    b.loft([[front]] + secs + [[back]])
    return b


def build_engine():
    b = Builder()
    zc = NOZZLE_Z

    def octo(x, rr):
        return [(x, 0.0, zc + rr), (x, -0.707 * rr, zc + 0.707 * rr), (x, -rr, zc),
                (x, -0.707 * rr, zc - 0.707 * rr), (x, 0.0, zc - rr)]
    # exhaust core + plume: root buried in the block in front of the recess floor, leaves the
    # bell lip and tapers to a point well behind the ship, so the team glow shows in profile on
    # both screen halves
    b.loft([octo(x, r) for x, r in PLUME] + [[(PLUME_TIP, 0.0, zc)]], cap_start=True, cap_end=False)
    # glowing vent strip on the white band between the cowl panels, 0.03 proud of the hull,
    # inner face 0.05 deep in the hull (clear of the cowl panels' inner faces at 0.03)
    g = 0.03
    pts = []
    for x in VENT_X:
        _, zt, zb, w = hull_station(x)
        for z in reversed(VENT_Z):
            pts.append((x, -w - g, z))
        for z in VENT_Z:
            pts.append((x, -w + 0.05, z))
    b.box(pts)
    # wing tip light: a strip capping the tip chord, 0.012 proud of both wing faces
    n = WING_T / 2 + 0.012
    b.box([wing_point(u, v, sn * n) for u in (-0.725, -0.99)
           for v, sn in ((0.565, -1), (0.565, 1), (0.62, 1), (0.62, -1))])
    return b


BUILDERS = {"hull": build_hull, "trim": build_trim, "canopy": build_canopy, "engine": build_engine}


def part_object(part, theme, root, collection, first):
    name = f"{MODEL}_{part}" if first else f"{MODEL}_{part}.{theme['id']}"
    me = BUILDERS[part]().finish(name)
    emission = dict(PARTS)[part]
    mat = gt.make_material(f"{MODEL}_{theme['id']}_{part}", theme[part], emission)
    me.materials.append(mat)
    obj = bpy.data.objects.new(name, me)
    collection.objects.link(obj)
    obj.parent = root
    obj["game_part"] = f"{MODEL}_{part}"
    mod = obj.modifiers.new("Mirror", 'MIRROR')
    mod.use_axis = (False, True, False)
    mod.use_clip = True
    mod.use_mirror_merge = True
    mod.merge_threshold = 0.0005
    return obj


def build(collection, location=(0.0, 0.0, 0.0)):
    """Builds the 4 team ships in `collection`, stacked 2.0 apart down Blender Z from
    `location` (p1 on top). Returns the 4 roots."""
    roots = []
    for i, theme in enumerate(THEMES):
        loc = (location[0], location[1], location[2] - 2.0 * i)
        root = gt.new_root(f"{MODEL}_{theme['id']}", loc, collection)
        root["game_model"] = MODEL
        root["team"] = theme["name"]
        for part, _ in PARTS:
            part_object(part, theme, root, collection, i == 0)
        roots.append(root)
    return roots
