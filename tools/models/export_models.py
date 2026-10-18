"""Writes the game models (assets/game/models/*.obj) from the Blender scripts of tools/models/.

    blender -b --factory-startup --python-exit-code 1 --python tools/models/export_models.py [-- player drone ...]

Every model is built in an empty scene, then each of its parts is written as one OBJ in game
space. Without names after "--", every model is exported. The colors are not in the OBJs: they
are in assets/game/models.cfg.
"""
import importlib
import os
import sys

import bpy

TOOLS = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(TOOLS))
OUT = os.path.join(REPO, "assets", "game", "models")
MODELS = ["player", "drone", "saucer"]

sys.path.insert(0, TOOLS)
import gametools as gt  # noqa: E402 (needs TOOLS in the path)


def main(models):
    for name in models:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        module = importlib.import_module(name)
        roots = module.build(bpy.context.scene.collection)
        bpy.context.view_layer.update()
        # a model with several looks (the player's teams) shares the meshes of its first one
        counts = gt.export_model(roots[0], OUT, f"tools/models/{name}.py")
        parts = ", ".join(f"{part} {n}" for part, n in counts.items())
        print(f"{name}: {sum(counts.values())} triangles ({parts})")


if __name__ == "__main__":
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    unknown = [a for a in args if a not in MODELS]
    if unknown:
        sys.exit(f"Unknown model(s): {', '.join(unknown)}. Models: {', '.join(MODELS)}")
    main(args or MODELS)
