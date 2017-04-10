#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	// YOUR CODE HERE:
    // Add box intersection code here.
	// it currently ignores all boxes and just returns false.

	i.obj = this;

	double Tnear = -INT_MAX;
	double Tfar = INT_MAX;

	vec3f p = r.getPosition();
	vec3f d = r.getDirection();

	for (int i = 0; i < 3; ++i) {
		if (d[i] == 0.0) {
			if (p[i] > 0.5 || p[i] < -0.5)
				return false; 
		}
		double T1 = (-0.5 - p[i]) / d[i];
		double T2 = (0.5 - p[i]) / d[i];
		if (T1 > T2) {
			double temp = T1;
			T1 = T2;
			T2 = temp;
		}
		if (T1 > Tnear)
			Tnear = T1;
		if (T2 < Tfar)
			Tfar = T2;
		if (Tnear > Tfar || Tfar < 0)
			return false;
	}
	i.t = Tnear;
	i.N = r.at(Tnear).normalize();
	return true;
}
