#pragma once

namespace be{

class Maths{

    private:
        Maths(){}

    public:
        static float random_float();
        static float random_float(float min, float max);
        static int random_int();
        static int random_int(int min, int max);

        static bool isZero(float f);
        static float sqr(float f);

        static float clamp(float f, float min, float max);
};

}