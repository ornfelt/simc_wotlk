using System;
using System.IO;

namespace TestNormal
{
    public static class SanityTests
    {
        public static void TestNormal()
        {
            int[] numbarz = new int[200];
            NormalGenerator ng = new NormalGenerator();

            for (int i = 0; i < 100000; ++i)
            {
                double next = ng.normal(0, 1) * 100 + 100;
                if (0 <= next && next < 200)
                    numbarz[(int)Math.Floor(next)]++;
            }

            using (StreamWriter sw = new StreamWriter(@"d:\" + Guid.NewGuid() + ".csv"))
            {
                foreach (int numbar in numbarz)
                {
                    sw.Write(numbar);
                    sw.Write(";");
                }
            }
        }

        // http://en.wikipedia.org/wiki/68-95-99.7_rule

        public static void Test3SigmaRule()
        {
            int total = 100000;
            int outOf1SigmaBoundary = 0;
            int outOf2SigmaBoundary = 0;
            int outOf3SigmaBoundary = 0;

            NormalGenerator ng = new NormalGenerator();
            for (int i = 0; i < total; ++i)
            {
                double next = ng.normal(0, 1);
                if (Math.Abs(next) > 1) outOf1SigmaBoundary++;
                if (Math.Abs(next) > 2) outOf2SigmaBoundary++;
                if (Math.Abs(next) > 3) outOf3SigmaBoundary++;
            }

            Console.WriteLine("Out of 1sigma: " + 1.0 * outOf1SigmaBoundary / total);
            Console.WriteLine("Out of 2sigma: " + 1.0 * outOf2SigmaBoundary / total);
            Console.WriteLine("Out of 3sigma: " + 1.0 * outOf3SigmaBoundary / total);
            Console.WriteLine();
        }
    }
}
