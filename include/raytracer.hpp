#include <cmath>

namespace Math {
    class Vector3D {
        public:
            Vector3D()
            {
                this->x = 0;
                this->y = 0;
                this->z = 0;
            }

            Vector3D(double x, double y, double z)
            {
                this->x = x;
                this->y = y;
                this->z = z;
            }

            Vector3D cross(const Vector3D& other) const
            {
                return Vector3D(this->y * other.z - this->z * other.y, this->z * other.x - this->x * other.z, this->x * other.y - this->y * other.x);
            }

            Vector3D normalize() const
            {
                return *this / this->length();
            }

            Vector3D& operator+=(const Vector3D& other)
            {
                this->x += other.x;
                this->y += other.y;
                this->z += other.z;
                return *this;
            }

            Vector3D operator+(const Vector3D& other) const
            {
                Vector3D result = *this;
                return result += other;
            }

            Vector3D& operator-=(const Vector3D& other)
            {
                this->x -= other.x;
                this->y -= other.y;
                this->z -= other.z;
                return *this;
            }
            
            Vector3D operator-(const Vector3D& other) const
            {
                Vector3D result = *this;
                return result -= other;
            }

            Vector3D& operator*=(const Vector3D& other)
            {
                this->x *= other.x;
                this->y *= other.y;
                this->z *= other.z;
                return *this;
            }

            Vector3D operator*(const Vector3D& other) const
            {
                Vector3D result = *this;
                return result *= other;
            }

            Vector3D& operator/=(const Vector3D& other)
            {
                this->x /= other.x;
                this->y /= other.y;
                this->z /= other.z;
                return *this;
            }

            Vector3D operator/(const Vector3D& other) const
            {
                Vector3D result = *this;
                return result /= other;
            }

            Vector3D& operator*=(double t)
            {
                this->x *= t;
                this->y *= t;
                this->z *= t;
                return *this;
            }

            Vector3D operator*(double t) const
            {
                Vector3D result = *this;
                return result *= t;
            }

            Vector3D& operator/=(double t)
            {
                this->x /= t;
                this->y /= t;
                this->z /= t;
                return *this;
            }

            Vector3D operator/(double t) const
            {
                Vector3D result = *this;
                return result /= t;
            }

            double length() const
            {
                return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2)); 
            }

            double dot(const Vector3D& other) const
            {
                return (this->x * other.x) + (this->y * other.y) + (this->z * other.z);
            }

            double x;
            double y;
            double z;
        private:

    };

    class Point3D {
        public:

            Point3D()
            {
                this->x = 0;
                this->y = 0;
                this->z = 0;
            }
            
            Point3D(double x, double y, double z)
            {
                this->x = x;
                this->y = y;
                this->z = z;
            }

            Point3D operator+(const Vector3D& other) const
            {                                                                                                    
                return Point3D(this->x + other.x, this->y + other.y, this->z + other.z);
            }           

            double x;
            double y;
            double z;
    };
}

namespace RayTracer {
    class Ray {
        public:

            Ray() = default;

            Ray(Math::Point3D origin, Math::Vector3D direction)
            {
                this->origin = origin;
                this->direction = direction;
            }

            Math::Point3D origin;
            Math::Vector3D direction;
    };

    class Triangle {
        public:
            Triangle(Math::Vector3D a, Math::Vector3D b, Math::Vector3D c)
            {
                this->a = a;
                this->b = b;
                this->c = c;
            }

            Math::Vector3D a;
            Math::Vector3D b;
            Math::Vector3D c;
    };
}
