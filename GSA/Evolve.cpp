#include "Optim.h"
#include "evolv.h"

#include "grishagin\include\grishagin_function.hpp"
#include <iostream>
#include <math.h>
#include <memory>
using namespace std;
int TwoS()
{

    double r = 2;
    double eps = 0.0001;
    double resAGP;
    double resGood;
    int n_max = 1000;





    for (int num = 1; num < 100; num++)
    {
        std::shared_ptr<IGOProblem<double>> problem;
        vagrish::GrishaginFunction *func = new vagrish::GrishaginFunction();
        func->SetFunctionNumber(num);
        problem = std::shared_ptr<IGOProblem<double>>(func);

        double leftBound[2], rightBound[2];
        problem->GetBounds(leftBound, rightBound);

        global::Pointer res;
        global Glob;
        Glob.r = r;
        Glob.E = eps;
        Glob.Nmax = n_max;
        res = Glob.EV(leftBound, rightBound, func);
        // Parallel_2D solver(leftBound, rightBound, r);

        // solver.Optimize(eps, n_max, 4, num, func);
        resAGP = round(res.z * 100) / 100;
        resGood = round(problem->GetOptimumValue() * 100) / 100;
        cout << "AGP :" << resAGP << " | True : " << resGood;

        if (abs(resAGP - resGood) <= 0.5)
        {
            cout << " || OK" << endl;
        }
        else
            cout << endl;




    }

    return 0;
}

