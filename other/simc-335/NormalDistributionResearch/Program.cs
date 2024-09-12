using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace TestNormal
{
    public class Program
    {
        public static void Main()
        {
//            SanityTests.TestNormal();
//            SanityTests.Test3SigmaRule();

            TestScaling();
            TestScaling2();
        }

        public static void TestScaling2()
        {
            double mean = 5000;
            double factor = 0.002;

            List<double> pimps = new List<double>();
            pimps.Add(20);
            while (pimps[pimps.Count - 1] < 200)
                pimps.Add(pimps[pimps.Count - 1] * 1.5);

            using(StreamWriter sw = new StreamWriter(@"d:\sigmafactors_" 
                + mean + "_" + factor.ToString("0.000", CultureInfo.InvariantCulture) + ".csv"))
            {
                foreach (double pimp in pimps)
                {
                    sw.Write(pimp.ToString("0.0000"));
                    sw.Write(";");
                }

                sw.WriteLine(); 
                
                foreach (double pimp in pimps)
                {
                    double coeff = findCoeffFor95Confidence(factor, mean, mean + pimp);
                    sw.Write(coeff.ToString("0.0000"));
                    sw.Write(";");
                }

                sw.WriteLine();
            }
        }

        public static void TestScaling()
        {
            double meanBaseline = 5000;
            double meanPimped = 5120;

            List<double> factors = new List<double>();
            factors.Add(0.002);
            while (factors[factors.Count - 1] > 0.00015)
                factors.Add(factors[factors.Count - 1] / 1.5);

            using(StreamWriter sw = new StreamWriter(@"d:\sigmafactors_" 
                + meanBaseline + "_" + meanPimped + ".csv"))
            {
                foreach(double factor in factors)
                {
                    sw.Write(factor.ToString("0.0000"));
                    sw.Write(";");
                }

                sw.WriteLine();

                List<double> coeffs = new List<double>();
                foreach (double factor in factors)
                {
                    double coeff = findCoeffFor95Confidence(factor, meanBaseline, meanPimped);
                    coeffs.Add(coeff);

                    sw.Write(coeff.ToString("0.0000"));
                    sw.Write(";");
                }

                sw.WriteLine();

                for(int i = 0; i < factors.Count; ++i)
                {
                    double factor = factors[i];
                    double coeff = coeffs[i];
                    double outside = outsideInterval(
                        factor*meanBaseline, meanBaseline, meanPimped, coeff*factor*meanBaseline);

                    sw.Write(outside.ToString("0.0000"));
                    sw.Write(";");
                }

                sw.WriteLine();
            }
        }

        private static double findCoeffFor95Confidence(
            double sigmaFactor, double baselineMean, double pimpedMean)
        {
            double sigma = sigmaFactor*baselineMean;

            double moreThan5PctOut = 0, lessThan5PctOut = 1.0;
            while (outsideInterval(sigma, baselineMean, pimpedMean, lessThan5PctOut * sigma) > 0.05)
                lessThan5PctOut *= 2.0;

            while(true)
            {
                double avg = (moreThan5PctOut + lessThan5PctOut)/2.0;
                double outside = outsideInterval(sigma, baselineMean, pimpedMean, avg*sigma);
                if (Math.Abs(outside - 0.05) < 0.001) return avg;
                if (outside > 0.05) moreThan5PctOut = avg;
                if (outside < 0.05) lessThan5PctOut = avg;
            }
        }

        private static double outsideInterval(
            double sigma, double baselineMean, double pimpedMean, double ival)
        {
            NormalGenerator ng = new NormalGenerator();

            int total = 100000;
            int outsideIval = 0;

            for (int i = 0; i < total; ++i)
            {
                double baseline = ng.normal(baselineMean, sigma);
                double pimped = ng.normal(pimpedMean, sigma);
                double diff = pimped - baseline;

                if (Math.Abs(diff - (pimpedMean - baselineMean)) > ival)
                {
                    outsideIval++;
                }
            }

            return 1.0 * outsideIval / total;
        }
    }
}
