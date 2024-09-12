using System;
using TestNormal.jp.takel.PseudoRandom;

namespace TestNormal
{
    public class NormalGenerator
    {
        private MersenneTwister mt = new MersenneTwister((uint)DateTime.Now.Ticks);

        public double normal(double mean, double std_dev)
        {
            return mean + (normal01() * std_dev);
        }

        public double normal01()
        {
            double u = 2 * mt.NextDouble() - 1;
            double v = 2 * mt.NextDouble() - 1;
            double r = u * u + v * v;
            if (r == 0 || r > 1) return normal01();

            double c = Math.Sqrt(-2 * Math.Log(r) / r);
            return u * c;
        }
    }
}
