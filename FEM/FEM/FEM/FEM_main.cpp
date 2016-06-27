#include "stdafx.h"

#define ELEM_H 1.0

/**
* Main project function. Argument format (can be used in any order):\n
* -f - path to the database file to be opened. Required parameter.\n
* @param argc default main function parameter - defines size of argv array.
* @param argv default main function parameter - array of input parameters.
* @return integer result: 0 - if everything is OK, not 0 if there is a problem.
*/
int main(int argc, char *argv[])
{
	auto timestamp1 = std::chrono::high_resolution_clock::now();

	std::string file;

	DatabaseAgent * DBAh;

	if (argc < 3)
	{
		std::cout << "Not enough parameters. Read documentation!\n\n";
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (std::string(argv[i]).compare("-f") == 0)
		{
			if (i >= argc - 1)
			{
				std::cout << "No -f parameter value. Read documentation!\n\n";
				return 1;
			}


			file = std::string(argv[i + 1]);

			DBAh = new DatabaseAgent(file.c_str());

			_PhysicalProperetiesS pProp;

			pProp.E = 2000;
			pProp.v = 0.3;

			//LinearElastic2DFEM LE2D_fem(DBAh, ELEM_H, pProp);
			//QuadraticElastic2DFEM LE2D_fem(DBAh, ELEM_H, pProp);

			LinearElastic3DFEM LE2D_fem(DBAh, pProp);
			
			//PLAN_LIN
			
			LE2D_fem.SetConstraints("FIXED", _ConstraintsS::UX1X2X3);

			//LE2D_fem.SetConstraints(3, _ConstraintsS::UX1X2);
			/*
			Eigen::Vector2d force;
			int n1 = 21;
			int n2 = 41;
			double frc = 10.0 * n1;
			frc /= n2;
			//21
			//41
			force << frc, 0.0;
			LE2D_fem.SetLoads("APPLY_X_FORCE_NODES", force);
			*/

			//easy
			/*
			LE2D_fem.SetConstraints("1", _ConstraintsS::UX1X2);

			Eigen::Vector2d disp;
			disp << 1.0, 1.0;

			LE2D_fem.SetDisplacement("2", disp);
			*/
			Eigen::Vector3d disp;
			disp << 0.001, 0.0, 0.0;

			LE2D_fem.SetDisplacement("UX_DISP", disp);

			LE2D_fem.Solve();
		}
	}
	
	delete DBAh;

    return 0;
}

