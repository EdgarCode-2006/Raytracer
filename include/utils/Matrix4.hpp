#ifndef MATRIX4_HPP_
#define MATRIX4_HPP_

namespace RayTracer {

    class Matrix4 {
        public:
            Matrix4() noexcept : m{
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1},
            } {}

            double m[4][4] = {};
    };

}


#endif