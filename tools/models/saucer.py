"""Bydo saucer: a tall three-tier biomechanical top that spins 60 deg/s around its vertical axis.

Blender: X right on screen, Z up, -Y towards the game camera. The hull shells are lofts of
10-point rings around Z (10 radial segments, flat facets) with 5-fold features, so the spin reads:
  1. top tier    : bronze dome carapace gripped by 5 chitin rib-claws, under a big glowing amber eye
  2. middle tier : thick barrel-shaped chitin armor band, 5 big glowing orange hex ports on
                   alternate facets (the rib-claws hook over the band between the ports)
  3. bottom tier : a chitin pod socket under the belly, holding a big glowing orange bulb
Every part is one or more closed shells; parts meet by piercing each other, never coplanar.
To tweak in Blender: edit the profile tables (CARAPACE, RIB, EYE, BAND, PORT, LENS) and rerun.
"""
import math
import bpy
import gametools as gt

MODEL = "saucer"
SCALE = 1.1          # the models.cfg scale (= gameplay size)

COLORS = {
    "carapace": ((190, 140, 60), 0.0),   # bronze: the dome-shaped top shell
    "band": ((150, 65, 60), 0.0),        # chitin: armored rim band, belly, pod socket, 5 rib-claws
    "ports": ((255, 125, 20), 1.0),      # orange: 5 rim ports + pod lens
    "eye": ((255, 200, 30), 1.0),        # amber: the dome eye
}
LENS_PART = "ports"      # which part the underside glowing bulb belongs to

N = 10                         # radial segments
TAU = 2.0 * math.pi
TH0 = -math.pi / 2.0           # angle facing the game camera (-Y)
SEG = TAU / N
ZOFF = -0.0425                 # recentres the model on its origin (visual centre)


def polar(r, th, z):
    return (r * math.cos(th), r * math.sin(th), z + ZOFF)


def ring(r, z, phase=0.0):
    """N points around Z at radius r and height z, the first one at the camera-facing angle
    (+phase)."""
    return [polar(r, TH0 + phase + i * SEG, z) for i in range(N)]


def loft(rings, cap_first=True, cap_last=True):
    """Closed shell through rings of N points (both ends capped by flat N-gons). Each quad is
    split into 2 triangles, alternating the diagonal so the facets stay mirror-symmetric around
    the rib lines."""
    verts, faces, idx = [], [], []
    for r in rings:
        idx.append(list(range(len(verts), len(verts) + len(r))))
        verts += r
    for a, b in zip(idx, idx[1:]):
        for i in range(N):
            i1 = (i + 1) % N
            if i % 2 == 0:
                faces += [(a[i], a[i1], b[i1]), (a[i], b[i1], b[i])]
            else:
                faces += [(a[i], a[i1], b[i]), (a[i1], b[i1], b[i])]
    if cap_first:
        faces.append(tuple(reversed(idx[0])))
    if cap_last:
        faces.append(tuple(idx[-1]))
    return verts, faces


def cone(rings, apex):
    """Closed shell: rings (first one capped) closed by a point."""
    verts, faces = loft(rings, cap_first=True, cap_last=False)
    last = list(range(len(verts) - N, len(verts)))
    verts.append(polar(0.0, 0.0, apex))
    top = len(verts) - 1
    faces += [(last[i], last[(i + 1) % N], top) for i in range(N)]
    return verts, faces


def merge(*meshes):
    verts, faces = [], []
    for v, f in meshes:
        o = len(verts)
        verts += v
        faces += [tuple(i + o for i in face) for face in f]
    return verts, faces


# --- tier 1: carapace dome, 5 rib-claws, dome eye ---------------------------------------------
# carapace points at TH0 + k*36deg (k odd = the rib lines between the ports)
CARAPACE = [(0.84, 0.080),   # bottom, hidden inside the band
            (0.935, 0.145),  # eave overhanging the band
            (0.87, 0.275),   # steep lower wall (faces the camera, stays lit)
            (0.68, 0.385),   # shoulder
            (0.47, 0.430)]   # collar lip around the eye (flat top under the dome)
# rib-claw stations along the carapace (r, z, outward normal angle from horizontal in degrees,
# half width, height above the surface), then the claw tip hooking over the band's top edge
RIB = [(0.52, 0.419, 78.0, 0.075, 0.080),
       (0.84, 0.292, 60.0, 0.110, 0.090)]
RIB_TIP = (1.00, 0.020)
RIB_SINK = 0.035               # how deep the rib's base edges sit under the carapace surface
EYE = [(0.435, 0.415), (0.35, 0.585)]   # rings (the first one buried under the collar top), then the apex
EYE_APEX = 0.640


def carapace_mesh():
    return loft([ring(r, z) for r, z in CARAPACE])


def rib(th):
    """A triangular-section rib lying on the carapace crease at angle th: two base edges sunk
    RIB_SINK under the surface, a sharp crest standing above it, ending in a hooked claw tip."""
    c, s = math.cos(th), math.sin(th)
    verts = []
    for r, z, a, hw, h in RIB:
        nr, nz = math.cos(math.radians(a)), math.sin(math.radians(a))
        for side, lift in ((-hw, -RIB_SINK), (hw, -RIB_SINK), (0.0, h)):
            rr, zz = r + nr * lift, z + nz * lift
            verts.append((rr * c - side * s, rr * s + side * c, zz + ZOFF))
    verts.append(polar(RIB_TIP[0], th, RIB_TIP[1]))
    faces = [(0, 2, 1)]                                   # end cap at the collar
    for i in range(3):                                    # the 3 sides between the stations
        j = (i + 1) % 3
        faces.append((i, j, 3 + j, 3 + i))
    faces += [(3 + i, 3 + (i + 1) % 3, 6) for i in range(3)]   # claw tip
    return verts, faces


def ribs_mesh():
    return merge(*[rib(TH0 + (2 * k + 1) * SEG) for k in range(N // 2)])


def eye_mesh():
    return cone([ring(r, z) for r, z in EYE], EYE_APEX)


# --- tier 2: armored rim band + ports --------------------------------------------------------
# band points sit half a segment off, so its flat facets are centred on the port angles
BAND = [(0.95, 0.110), (0.985, -0.020), (0.95, -0.150), (0.44, -0.265),
        (0.36, -0.360)]   # the belly ends in the pod socket
PORT = dict(w=0.34, h=0.20, zc=-0.020, proud=0.030, depth=0.20)


def band_mesh():
    return loft([ring(r, z, phase=SEG / 2) for r, z in BAND])


def port(th):
    """Flattened hexagon standing `proud` in front of the band facet, with a pyramid back sunk
    into the band."""
    apothem = max(r for r, z in BAND) * math.cos(SEG / 2)   # the bulge of the barrel band
    rf = apothem + PORT["proud"]
    w, h, zc = PORT["w"], PORT["h"], PORT["zc"]
    u = (math.cos(th), math.sin(th))
    t = (-math.sin(th), math.cos(th))
    hexa = [(-w / 2, zc), (-w * 0.30, zc - h / 2), (w * 0.30, zc - h / 2),
            (w / 2, zc), (w * 0.30, zc + h / 2), (-w * 0.30, zc + h / 2)]
    verts = [(u[0] * rf + t[0] * s, u[1] * rf + t[1] * s, z + ZOFF) for s, z in hexa]
    r0 = rf - PORT["depth"]
    verts.append((u[0] * r0, u[1] * r0, zc + ZOFF))
    faces = [tuple(range(6))] + [((i + 1) % 6, i, 6) for i in range(6)]
    return verts, faces


def ports_mesh():
    return merge(*[port(TH0 + 2 * k * SEG) for k in range(N // 2)])


# --- tier 3: glowing bulb hanging from the pod socket ------------------------------------------
LENS = [(0.28, -0.300), (0.34, -0.420), (0.24, -0.515)]   # first ring buried in the socket
LENS_APEX = -0.555


def lens_mesh():
    return cone([ring(r, z) for r, z in LENS], LENS_APEX)


def build(collection, location=(0.0, 0.0, 0.0)):
    """Builds the saucer in `collection` with its root at `location`. Returns [root]."""
    root = gt.new_root(MODEL, location, collection)
    meshes = {"carapace": [carapace_mesh()], "band": [band_mesh(), ribs_mesh()], "ports": [ports_mesh()], "eye": [eye_mesh()]}
    meshes[LENS_PART].append(lens_mesh())
    for part in ("carapace", "band", "ports", "eye"):
        rgb, em = COLORS[part]
        verts, faces = merge(*meshes[part])
        gt.mesh_object(MODEL + "_" + part, verts, faces, gt.make_material(MODEL + "_" + part, rgb, em), root)
    return [root]
