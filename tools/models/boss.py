"""Bydo stage boss: a hunched biomechanical leviathan facing -X (towards the players).

Blender: X = right on screen, Z = up, -Y = towards the game camera (Front view = game view).
Everything is generated from data (lofts along paths, arc-slab plates, lenses), no edit-mode ops.
The shape tables (TORSO_*, PLATES, SPINES, CREASE, SKULL_PROF, EYES, FANGS, ARM, PINCER, RIBS)
are the knobs: edit them and rebuild.
"""
import math
import bpy
import bmesh
from mathutils import Vector as V
import gametools as gt

MODEL = "boss"
SCALE = 4.0

COLORS = {                      # part: (sRGB 0-255, emission)
    "body": ((200, 112, 95), 0.0),    # flesh
    "armor": ((180, 85, 45), 0.0),    # rust carapace
    "bone": ((220, 190, 145), 0.0),   # bone: skull, spines, ribs, pincer
    "core": ((255, 200, 30), 1.0),    # amber weak point (sits on flesh)
    "eyes": ((255, 45, 40), 1.0),     # red eyes (sit on bone), core pupil, maw
}
Y = V((0.0, 1.0, 0.0))
TAU = 2.0 * math.pi


# --------------------------------------------------------------------------- geometry helpers

class Geo:
    """Accumulates closed shells (verts, faces) for one part."""
    def __init__(self):
        self.verts, self.faces = [], []

    def add(self, vf):
        verts, faces = vf
        o = len(self.verts)
        self.verts += [V(p) for p in verts]
        self.faces += [tuple(i + o for i in f) for f in faces]


def loft(rings, cap0=True, cap1=True):
    """Rings of equal size (or a single apex point) -> closed shell."""
    verts, faces, idx = [], [], []
    for r in rings:
        idx.append(list(range(len(verts), len(verts) + len(r))))
        verts += [V(p) for p in r]
    for a, b in zip(idx, idx[1:]):
        if len(a) == 1:
            n = len(b)
            faces += [(a[0], b[(i + 1) % n], b[i]) for i in range(n)]
        elif len(b) == 1:
            n = len(a)
            faces += [(a[i], a[(i + 1) % n], b[0]) for i in range(n)]
        else:
            n = len(a)
            faces += [(a[i], a[(i + 1) % n], b[(i + 1) % n], b[i]) for i in range(n)]
    if cap0 and len(idx[0]) > 1:
        faces.append(tuple(reversed(idx[0])))
    if cap1 and len(idx[-1]) > 1:
        faces.append(tuple(idx[-1]))
    return verts, faces


def frame(t, side=Y):
    """Cross-section frame (t, u, d) of a path: u = t x side, d = u x t.
    With the default side = +Y, u is t turned 90 deg counter-clockwise in the front (game) view:
    UP for a path heading +X (torso), but DOWN for a path heading -X (and d = +Y, away from the
    camera). Pass side = -Y for paths heading -X (the head) so that u is up and d points at the
    camera. Along the curled tail the default keeps u on the outside of the curl."""
    t = V(t).normalized()
    u = t.cross(side)
    if u.length < 1e-6:
        u = t.cross(V((1, 0, 0)))
    u.normalize()
    return t, u, u.cross(t).normalized()


def ngon(n, phase=None, squash=None):
    """Unit cross-section (pu, pd). phase=pi/n puts a flat face towards the camera for n=8."""
    phase = math.pi / n if phase is None else phase
    pts = [(math.cos(phase + TAU * k / n), math.sin(phase + TAU * k / n)) for k in range(n)]
    if squash:  # per-vertex radial factor function of angle
        pts = [(pu * squash(math.atan2(pd, pu)), pd * squash(math.atan2(pd, pu))) for pu, pd in pts]
    return pts


def ring(c, t, prof, h, dep, side=Y):
    _, u, d = frame(t, side)
    c = V(c)
    return [c + u * (h * pu) + d * (dep * pd) for pu, pd in prof]


class Path:
    """Catmull-Rom curve through control points, sampled by arc length (s in 0..1)."""
    def __init__(self, pts, samples=16):
        P = [V(p) for p in pts]
        poly = []
        for i in range(len(P) - 1):
            p0 = P[i - 1] if i > 0 else 2 * P[0] - P[1]
            p1, p2 = P[i], P[i + 1]
            p3 = P[i + 2] if i + 2 < len(P) else 2 * P[-1] - P[-2]
            for k in range(samples):
                t = k / samples
                poly.append(0.5 * (2 * p1 + (p2 - p0) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t
                                   + (3 * p1 - p0 - 3 * p2 + p3) * t * t * t))
        poly.append(P[-1])
        self.poly = poly
        self.cum = [0.0]
        for a, b in zip(poly, poly[1:]):
            self.cum.append(self.cum[-1] + (b - a).length)
        self.length = self.cum[-1]

    def _seg(self, s):
        d = max(0.0, min(1.0, s)) * self.length
        for i in range(len(self.cum) - 1):
            if self.cum[i + 1] >= d:
                f = (d - self.cum[i]) / max(1e-9, self.cum[i + 1] - self.cum[i])
                return i, f
        return len(self.cum) - 2, 1.0

    def point(self, s):
        i, f = self._seg(s)
        return self.poly[i].lerp(self.poly[i + 1], f)

    def tangent(self, s):
        i, _ = self._seg(s)
        return (self.poly[i + 1] - self.poly[i]).normalized()


def keys(table, s):
    """Linear interpolation in [(s, value), ...]."""
    if s <= table[0][0]:
        return table[0][1]
    for (s0, v0), (s1, v1) in zip(table, table[1:]):
        if s <= s1:
            f = (s - s0) / (s1 - s0)
            return v0 + (v1 - v0) * f
    return table[-1][1]


def sweep(path, stations, hk, dk, prof, tip=None, cap0=True, side=Y, tip_ext=0.0):
    """Loft along a Path at stations s, sizes from key tables hk/dk. tip: ends in a point
    (extended by tip_ext along the end tangent)."""
    rings = []
    for s in stations:
        p = prof(s) if callable(prof) else prof
        rings.append(ring(path.point(s), path.tangent(s), p, keys(hk, s), keys(dk, s), side))
    if tip:
        rings.append([path.point(1.0) + path.tangent(1.0) * tip_ext])
    return loft(rings, cap0=cap0, cap1=not tip)


def spike(base, direction, length, w, dep, bend=V((0, 0, 0)), n=4, phase=0.0, segs=2):
    """Tapered horn/spine/blade: from base along direction, bending towards bend."""
    base, direction = V(base), V(direction).normalized()
    pts = [base]
    for k in range(1, segs + 1):
        f = k / segs
        pts.append(base + direction * (length * f) + V(bend) * (length * f * f))
    path = Path(pts, samples=6)
    st = [i / segs for i in range(segs)]
    hk = [(0.0, w), (1.0, w * 0.25)]
    dk = [(0.0, dep), (1.0, dep * 0.25)]
    return sweep(path, st, hk, dk, ngon(n, phase), tip=True)


def plate(path, s0, s1, hk, dk, angs, rf, inner=0.80, k=(1.03, 1.12, 1.08), tooth=0.0, mid=0.5):
    """Curved armor slab (crustacean carapace segment) around a path between stations s0 and s1.
    angs: angles (degrees, 0 = up, -90 = camera side) of the outer arc; rf(a): radial factor of the
    outer surface there (spine keel, camera-band crease). The inner face is one flat chord deep in
    the flesh (never seen, cheap). Three rings (front, ridge, rear) sized from the key tables hk/dk
    times k: the front half faces forward (lit), the rear half back.
    tooth: degrees the rear ring's arc reaches further down on both sides (serrated lower edge)."""
    def prof(kk, ext):
        a = list(angs)
        a[0], a[-1] = a[0] - ext, a[-1] + ext
        outer = [(kk * rf(x) * math.cos(math.radians(x)), kk * rf(x) * math.sin(math.radians(x)))
                 for x in a]
        inn = [(inner * math.cos(math.radians(x)), inner * math.sin(math.radians(x))) for x in (a[-1], a[0])]
        return outer + inn
    rings = []
    for f, kk, ext in [(0.0, k[0], 0.0), (mid, k[1], tooth * 0.4), (1.0, k[2], tooth)]:
        s = s0 + (s1 - s0) * f
        rings.append(ring(path.point(s), path.tangent(s), prof(kk, ext), keys(hk, s), keys(dk, s)))
    return loft(rings)


def lens(c, rx, rz, depth, n=8, front=-1.0, back_depth=None, phase=None):
    """Emissive lens facing the camera: an n-gon in the XZ plane with a pointed front
    (towards front*Y) and back."""
    c = V(c)
    phase = math.pi / n if phase is None else phase
    rim = [c + V((rx * math.cos(phase + TAU * k / n), 0.0, rz * math.sin(phase + TAU * k / n)))
           for k in range(n)]
    tipf = c + V((0, front * depth, 0))
    tipb = c - V((0, front * (back_depth if back_depth is not None else depth), 0))
    return loft([[tipb], rim, [tipf]])


def knuckle(c, axis, r, back, front, n=6, phase=0.0, chamfer=0.68):
    """Flat n-gon joint cap along axis (towards the camera, -Y, tilted): buried at back, flat
    chamfered face at front (distances along axis from c). Its upper chamfer facets catch the
    light, the lower ones fall in shade."""
    c, a = V(c), V(axis).normalized()
    e1 = V((1.0, 0.0, 0.0))
    e1 = (e1 - a * e1.dot(a)).normalized()
    e2 = a.cross(e1)

    def rg(dist, rr):
        return [c + a * dist + e1 * (rr * math.cos(phase + TAU * k / n)) + e2 * (rr * math.sin(phase + TAU * k / n))
                for k in range(n)]
    rim = front + (back - front) * 0.3
    return loft([rg(back, r * 0.8), rg(rim, r), rg(front, r * chamfer)])


def torus(c, axis, R, r, nseg=12, nside=4, phase_side=None):
    c = V(c)
    axis = V(axis).normalized()
    a = axis.orthogonal().normalized()
    b = axis.cross(a)
    ps = math.pi / nside if phase_side is None else phase_side
    verts, faces = [], []
    for i in range(nseg):
        th = TAU * i / nseg
        radial = a * math.cos(th) + b * math.sin(th)
        for j in range(nside):
            ph = ps + TAU * j / nside
            verts.append(c + radial * (R + r * math.cos(ph)) + axis * (r * math.sin(ph)))
    for i in range(nseg):
        for j in range(nside):
            i2, j2 = (i + 1) % nseg, (j + 1) % nside
            faces.append((i * nside + j, i2 * nside + j, i2 * nside + j2, i * nside + j2))
    return verts, faces


def hull(points):
    bm = bmesh.new()
    for p in points:
        bm.verts.new(V(p))
    res = bmesh.ops.convex_hull(bm, input=list(bm.verts))
    hull_verts = {v for g in res["geom"] if isinstance(g, bmesh.types.BMFace) for v in g.verts}
    junk = [v for v in bm.verts if v not in hull_verts]
    if junk:
        bmesh.ops.delete(bm, geom=junk, context='VERTS')
    bmesh.ops.dissolve_limit(bm, angle_limit=math.radians(1.0), verts=list(bm.verts), edges=list(bm.edges))
    bm.verts.index_update()
    verts = [v.co.copy() for v in bm.verts]
    faces = [tuple(v.index for v in f.verts) for f in bm.faces]
    bm.free()
    return verts, faces


def mirror_y(vf):
    verts, faces = vf
    return [V((p.x, -p.y, p.z)) for p in map(V, verts)], [tuple(reversed(f)) for f in faces]


# --------------------------------------------------------------------------- the creature

def torso_path():
    return Path([(-0.40, 0.0, 0.20), (-0.20, 0.0, 0.22), (0.05, 0.0, 0.17), (0.30, 0.0, 0.08),
                 (0.50, 0.0, -0.05), (0.62, 0.0, -0.20), (0.67, 0.0, -0.30)])


TORSO_H = [(0.0, 0.20), (0.15, 0.44), (0.38, 0.50), (0.62, 0.44), (0.80, 0.33), (0.93, 0.20), (1.0, 0.10)]
TORSO_D = [(0.0, 0.30), (0.15, 0.44), (0.38, 0.50), (0.62, 0.46), (0.80, 0.38), (0.93, 0.26), (1.0, 0.14)]
BODY_K = 0.95                    # flesh under the plates is a bit smaller: no edge pokes through
CORE = V((0.06, -0.40, -0.03))   # weak point centre (camera side of the belly)


def scaled(table, k):
    return [(s, v * k) for s, v in table]


def keel(a):
    """Radial factor of the carapace cross-section: a raised ridge along the spine (a in degrees)."""
    c = max(0.0, math.cos(math.radians(a)))
    return 1.0 + 0.16 * c ** 8


# carapace cross-section: outer arc angles (both flanks) and the radial factor by |angle|.
# The bulge at 80 deg is a hard crease about a third of the way up the camera band: above it the
# band faces up-front (full light), below it the strip tucks back to the lower edge and faces
# down-front (mid value). With the dark flesh gaps every segment reads light / mid / dark.
PLATE_ANGS = [-106, -80, -27, 0, 27, 80, 106]
CREASE = [(0, 1.0), (27, 1.0), (80, 1.15), (106, 0.94), (140, 0.94)]


def plate_rf(a):
    return keel(a) * keys(CREASE, abs(a))


# dorsal carapace plates (s0, s1) along the torso path, separated by flesh gaps
PLATES = [(0.02, 0.21), (0.245, 0.43), (0.465, 0.64), (0.675, 0.835), (0.865, 0.97)]
# dorsal spines per plate: (where on the plate 0..1, length, width, up weight, rake, bend)
# one tall hooked crest, a stepped row behind it and small thorns in front of the big ones
SPINES = [
    [(0.60, 0.24, 0.10, 0.60, 0.05, 0.25)],
    [(0.62, 0.44, 0.12, 0.80, 0.10, 0.55), (0.10, 0.15, 0.065, 0.80, 0.20, 0.15)],
    [(0.60, 0.34, 0.10, 0.50, 0.28, 0.35)],
    [(0.62, 0.30, 0.095, 0.30, 0.42, 0.30), (0.12, 0.13, 0.06, 0.50, 0.40, 0.15)],
    [(0.55, 0.19, 0.08, 0.05, 0.50, 0.30)],
]


def build_torso(body, armor, bone):
    tp = torso_path()
    body.add(sweep(tp, [0.0, 0.15, 0.38, 0.62, 0.80, 0.93, 1.0], scaled(TORSO_H, BODY_K),
                   scaled(TORSO_D, BODY_K), ngon(8)))
    # carapace: plates with a spine ridge, a creased camera band and a serrated lower edge;
    # the flesh shows between them
    for (s0, s1), spines in zip(PLATES, SPINES):
        armor.add(plate(tp, s0, s1, TORSO_H, TORSO_D, PLATE_ANGS, plate_rf, k=(1.04, 1.12, 1.09),
                        tooth=14.0, mid=0.55))
        # dorsal spines raked back, rising from the ridge
        for where, length, w, upw, rake, bend in spines:
            s = s0 + (s1 - s0) * where
            c, t = tp.point(s), tp.tangent(s)
            _, u, _ = frame(t)          # the torso runs +X: u is up
            base = c + u * keys(TORSO_H, s) * 1.08
            bone.add(spike(base, u * upw + t * (0.6 + rake), length, w, w * 0.6,
                           bend=t * bend, n=4, phase=0.0))


SKULL_PROF = [(1.25, 0.0), (0.55, -0.85), (0.05, -1.05), (-0.55, -0.9), (-1.0, 0.0),
              (-0.55, 0.9), (0.05, 1.05), (0.55, 0.85)]          # (pu up, pd depth): keel on top
EYES = [(0.24, 0.066, -0.02), (0.43, 0.052, -0.08), (0.61, 0.040, -0.18)]   # (s, radius, height)
# fangs (x, |y|, z, direction z, length): front pair at the jaw tips, rear pair in front of the
# glowing throat (|y| beyond the maw hull) so they cut dark notches into the red
FANGS = [(-0.86, 0.065, 0.49, -1, 0.13), (-0.71, 0.125, 0.57, -1, 0.15),
         (-0.84, 0.065, 0.27, 1, 0.11), (-0.71, 0.125, 0.37, 1, 0.13)]
HEAD = -Y      # the head paths run -X: side -Y keeps u up and d towards the camera (see frame())


def build_neck_head(body, armor, bone, eyes):
    # neck (flesh) arching up from the chest to the skull base
    neck = Path([(-0.28, 0.0, 0.30), (-0.40, 0.0, 0.45), (-0.47, 0.0, 0.60)])
    body.add(sweep(neck, [0.0, 0.5, 1.0], [(0, 0.17), (1, 0.12)], [(0, 0.20), (1, 0.14)], ngon(8),
                   side=HEAD))

    # skull (bone): keel on top, cheekbones; its upper-front facet catches the light
    skull = Path([(-0.38, 0.0, 0.69), (-0.57, 0.0, 0.68), (-0.77, 0.0, 0.61), (-0.93, 0.0, 0.52),
                  (-1.0, 0.0, 0.47)])
    sh = [(0.0, 0.19), (0.3, 0.18), (0.6, 0.13), (0.85, 0.075), (1.0, 0.03)]
    sd = [(0.0, 0.18), (0.3, 0.19), (0.6, 0.14), (0.85, 0.08), (1.0, 0.03)]
    bone.add(sweep(skull, [0.0, 0.3, 0.6, 0.85], sh, sd, SKULL_PROF, tip=True, side=HEAD))

    # brow ridges (bone): angry wedges hooding the eyes, their undersides throw a dark line
    for side in (-1, 1):
        pts = []
        for s, up in [(0.12, 0.62), (0.42, 0.42), (0.74, 0.26)]:
            c, t = skull.point(s), skull.tangent(s)
            _, u, d = frame(t, HEAD)
            h, dep = keys(sh, s), keys(sd, s)
            pts += [c + u * (h * up) + d * (side * dep * 1.24),
                    c + u * (h * (up + 0.30)) + d * (side * dep * 1.02),
                    c + u * (h * (up - 0.05)) + d * (side * dep * 0.70),
                    c + u * (h * (up + 0.35)) + d * (side * dep * 0.60)]
        bone.add(hull(pts))

    # lower jaw (bone), hanging open
    jaw = Path([(-0.46, 0.0, 0.56), (-0.66, 0.0, 0.40), (-0.84, 0.0, 0.25), (-0.92, 0.0, 0.20)])
    bone.add(sweep(jaw, [0.0, 0.35, 0.75], [(0, 0.085), (1, 0.035)], [(0, 0.16), (1, 0.06)],
                   ngon(6), tip=True, side=HEAD))
    # glowing throat (red, framed by the bone jaws and fangs): the boss fires from its maw
    eyes.add(hull([(-0.44, -0.12, 0.66), (-0.44, 0.12, 0.66), (-0.78, -0.09, 0.55), (-0.78, 0.09, 0.55),
                   (-0.74, -0.09, 0.36), (-0.74, 0.09, 0.36), (-0.44, -0.12, 0.46), (-0.44, 0.12, 0.46)]))
    # fangs: two down from the skull, two up from the jaw (each side)
    for x, yy, z, dz, ln in FANGS:
        for sy in (-1, 1):
            bone.add(spike((x, sy * yy, z), (-0.3, 0.0, dz), ln, 0.045, 0.028, n=3, phase=0.0, segs=1))

    # swept-back horns (bone), one each side
    for side in (-1, 1):
        bone.add(spike((-0.40, side * 0.10, 0.80), (0.85, side * 0.16, 0.40), 0.46, 0.08, 0.055,
                       bend=V((0.15, 0.0, -0.12)), n=4, phase=0.0))

    # eyes (red, on bone): three per side along the skull flank, under the brow
    for s, r, up in EYES:
        c, t = skull.point(s), skull.tangent(s)
        _, u, d = frame(t, HEAD)
        dep = keys(sd, s)
        e = lens(c + u * (keys(sh, s) * up) + d * (dep * 0.98), r * 1.2, r * 0.82, 0.04, n=8)
        eyes.add(e)
        eyes.add(mirror_y(e))


CORE_N = 16    # segments of the core orb: it is THE focal point, keep it round
# bone ribs framing the core: (radius, start angle, end angle, width, y, tip extension, stations).
# The orbit rib curls from under the carapace round the right, under the core and up its
# lower-left into the shoulder: with the carapace edge as the eyelid it closes the eye socket.
RIBS = [(0.295, 74, -205, 0.075, -0.475, 0.0, 12),
        (0.405, 52, -58, 0.085, -0.45, 0.02, 5)]


def build_core(body, bone, core, eyes):
    cx, cz = CORE.x, CORE.z
    # glowing orb: back apex inside the body, wide rim, flat front face
    rims = []
    for y, r in [(-0.36, 0.205), (-0.47, 0.195), (-0.54, 0.135)]:
        rims.append([V((cx + r * math.cos(TAU * k / CORE_N), y, cz + r * math.sin(TAU * k / CORE_N)))
                     for k in range(CORE_N)])
    # flat front face as a centred fan (no sliver triangles from a 16-gon cap)
    core.add(loft([[V((cx, -0.15, cz))]] + rims + [[V((cx, -0.54, cz))]], cap0=False, cap1=False))
    # vertical slit pupil (red): the weak point reads as a giant Bydo eye
    eyes.add(lens((cx, -0.558, cz), 0.036, 0.155, 0.03, n=6, back_depth=0.05, phase=0.0))
    # fleshy socket lip
    body.add(torus((cx, -0.43, cz), (0, -1, 0), 0.23, 0.055, nseg=14, nside=4))
    for rr, a0, a1, w, y0, ext, n in RIBS:
        pts = []
        for k in range(n):
            a = math.radians(a0 + (a1 - a0) * k / (n - 1))
            pts.append((cx + rr * math.cos(a), y0 + 0.03 * (k / (n - 1)), cz + rr * math.sin(a)))
        pts[0] = (pts[0][0] + 0.02, -0.30, pts[0][2] + 0.05)   # root under the carapace
        st = [i / (n - 1) for i in range(n - 1)]
        bone.add(sweep(Path(pts, samples=6), st, [(0, w), (1, w * 0.45)],
                       [(0, 0.06), (1, 0.035)], ngon(4, 0.0), tip=True, tip_ext=ext))


# the one huge reaching arm (camera side), fiddler-crab style: shoulder, elbow, wrist
ARM = dict(ys=-0.42, pose=[(-0.33, 0, 0.04), (-0.52, 0, -0.36), (-0.74, 0, -0.29)])
# open-V pincer: (+1 upper / -1 lower blade, length, width, opening, hook towards the other blade)
PINCER = [(+1, 0.36, 0.105, 0.50, 0.55), (-1, 0.31, 0.095, 0.45, 0.60)]
KNUCKLE_TILT = 28.0    # degrees the elbow cap's face turns down from the camera axis


def build_arm(body, armor, bone, ys, pose):
    sh, el, wr = [V(p) for p in pose]
    sh.y = el.y = wr.y = ys
    # shoulder plate (armor) on the flank under the carapace edge
    armor.add(hull([sh + V(o) for o in [(-0.10, -0.09, 0.07), (0.10, -0.09, 0.10),
                                        (0.11, -0.06, -0.07), (-0.08, -0.07, -0.09),
                                        (0.0, -0.13, 0.01), (-0.10, 0.08, 0.07),
                                        (0.10, 0.08, 0.10), (0.11, 0.06, -0.07),
                                        (-0.08, 0.05, -0.09)]]))
    # upper arm (armor)
    up = Path([sh, (sh + el) / 2 + V((-0.03, 0, 0.0)), el])
    armor.add(sweep(up, [0.0, 0.5, 1.0], [(0, 0.09), (1, 0.07)], [(0, 0.09), (1, 0.075)], ngon(6)))
    # elbow knuckle (flesh): a wide flat hexagonal joint cap (1.4x the arm), its face to the
    # camera and tilted down so it sits in half shade instead of glaring pink
    tilt = math.radians(KNUCKLE_TILT)
    body.add(knuckle(el, (0.0, -math.cos(tilt), -math.sin(tilt)), 0.105, -0.03, 0.11))
    # forearm (armor), flaring into the claw
    fo = Path([el, (el + wr) / 2 + V((0, 0, -0.035)), wr])
    armor.add(sweep(fo, [0.0, 0.5, 1.0], [(0, 0.07), (0.6, 0.115), (1, 0.105)],
                    [(0, 0.075), (1, 0.095)], ngon(6)))
    # elbow spur (bone), pointing down and back
    bone.add(spike(el + V((0.03, 0, -0.03)), (0.55, 0, -0.85), 0.20, 0.055, 0.045, n=4))
    # pincer (bone): two blades opening in a V and hooking towards each other. d is the forearm
    # direction (-X-ish); up_ = d turned 90 deg clockwise in the front view = up for this arm.
    d = (wr - el).normalized()
    up_ = V((d.z, 0, -d.x))
    for sgn, ln, w, opening, hook in PINCER:
        dd = (d + up_ * (sgn * opening)).normalized()
        base = wr + up_ * (sgn * 0.045) - d * 0.02      # separate roots inside the wrist
        bone.add(spike(base, dd, ln, w, 0.06, bend=up_ * (-sgn * hook * 0.5), n=4, phase=0.0, segs=3))


def build_tail(body, armor, bone):
    tail = Path([(0.56, 0.0, -0.18), (0.73, 0.0, -0.35), (0.84, 0.0, -0.55), (0.84, 0.0, -0.73),
                 (0.73, 0.0, -0.86), (0.55, 0.0, -0.905), (0.38, 0.0, -0.85), (0.29, 0.0, -0.73)])
    rk = [(0.0, 0.17), (0.5, 0.11), (1.0, 0.06)]
    # flesh cord through the segments (only seen in the gaps between the rings)
    st = [i / 6 for i in range(7)]
    body.add(sweep(tail, st, scaled(rk, 0.66), scaled(rk, 0.66), ngon(6)))
    # shingled armor rings, three with an outward spike (u = outside of the curl)
    n = 8
    for i in range(n):
        s0 = i / n + 0.012
        s1 = (i + 0.82) / n
        r0, r1 = keys(rk, s0), keys(rk, s1)
        p = ngon(6)
        armor.add(loft([ring(tail.point(s0), tail.tangent(s0), p, r0 * 0.9, r0 * 0.9),
                        ring(tail.point(s1), tail.tangent(s1), p, r1 * 1.08, r1 * 1.08)]))
        if i in (1, 3, 5):
            s = (s0 + s1) / 2
            c, t = tail.point(s), tail.tangent(s)
            _, u, _ = frame(t)
            r = keys(rk, s)
            bone.add(spike(c + u * r * 0.9, u + t * 0.8, 0.05 + r * 0.55, r * 0.45, r * 0.32,
                           bend=t * 0.25, n=4, phase=0.0, segs=1))
    # stinger (bone): hooked blade continuing the curl
    end = tail.point(1.0)
    t = tail.tangent(1.0)
    bone.add(spike(end - t * 0.04, t, 0.28, 0.065, 0.05, bend=V((-0.35, 0, 0.1)), n=4, phase=0.0, segs=3))


def build(collection, location=(0.0, 0.0, 0.0)):
    root = gt.new_root(MODEL, location, collection)
    geo = {k: Geo() for k in COLORS}
    build_torso(geo["body"], geo["armor"], geo["bone"])
    build_neck_head(geo["body"], geo["armor"], geo["bone"], geo["eyes"])
    build_core(geo["body"], geo["bone"], geo["core"], geo["eyes"])
    build_arm(geo["body"], geo["armor"], geo["bone"], ARM["ys"], ARM["pose"])
    build_tail(geo["body"], geo["armor"], geo["bone"])
    # fit the side profile in [-1, 1] (x, z) around its centre: models.cfg scale = half-extent
    allv = [v for g in geo.values() for v in g.verts]
    lo = V((min(v.x for v in allv), 0.0, min(v.z for v in allv)))
    hi = V((max(v.x for v in allv), 0.0, max(v.z for v in allv)))
    centre = (lo + hi) * 0.5
    k = 1.0 / max(1.0, (hi.x - lo.x) * 0.5, (hi.z - lo.z) * 0.5)
    for g in geo.values():
        g.verts = [(v - centre) * k for v in g.verts]
    for part, (rgb, em) in COLORS.items():
        g = geo[part]
        mat = gt.make_material(f"{MODEL}_{part}", rgb, em)
        gt.mesh_object(f"{MODEL}_{part}", g.verts, g.faces, mat, root, collection=collection)
    return [root]
