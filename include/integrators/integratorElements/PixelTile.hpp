#ifndef PIXEL_TILE_HPP_
#define PIXEL_TILE_HPP_

namespace RayTracer {
    class PixelTile {
        public:
            PixelTile(int x, int y, int dimX, int dimY) : 
                _x(x), _y(y), _dimX(dimX), _dimY(dimY) {};
            int getX() const {return _x;};
            int getY() const {return _y;};
            int getYDim() const {return _dimY;};
            int getXDim() const {return _dimX;};

        private:
            int _dimX;
            int _dimY;
            int _x;
            int _y;
    };
};
#endif