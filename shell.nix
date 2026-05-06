{ pkgs ? import <nixpkgs> {} }:

let
  raylib_x11 = pkgs.raylib.overrideAttrs (old: {
    cmakeFlags = [
      "-DCUSTOMIZE_BUILD=ON"
      "-DUSE_WAYLAND=OFF"
      "-DBUILD_SHARED_LIBS=ON"
      "-DUSE_EXTERNAL_GLFW=OFF"
    ];
    buildInputs = (old.buildInputs or []) ++ [
      pkgs.xorg.libX11
      pkgs.xorg.libXrandr
      pkgs.xorg.libXinerama
      pkgs.xorg.libXcursor
      pkgs.xorg.libXi
      pkgs.xorg.libXext
    ];
  });
in

pkgs.mkShell {
  name = "raytracer-env";

  buildInputs = with pkgs; [
    cmake
    gnumake
    gcc
    pkg-config
    
    raylib_x11
    libGL
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    xorg.libXext

    libconfig
  ];

  shellHook = ''
    export NIX_ENFORCE_PURITY=0
    unset WAYLAND_DISPLAY
    export GLFW_PLATFORM=x11
    export DISPLAY=''${DISPLAY:-:0}
  '';
}
