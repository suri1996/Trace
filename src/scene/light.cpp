#include <cmath>

#include "light.h"
#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	vec3f direction = getDirection(P);

	vec3f Pruc = P;
	isect Psect;
	vec3f atten = getColor(P);
	ray r = ray(Pruc, direction);
	while (scene->intersect(r, Psect)) {
		if (Psect.getMaterial().kt.iszero()) return vec3f(0, 0, 0);
		Pruc = r.at(Psect.t);
		r = ray(Pruc, direction);
		atten = prod(atten, Psect.getMaterial().kt);
	}
	return atten;
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

PointLight::PointLight(Scene *scene, const vec3f& pos, const vec3f& color)
: Light(scene, color),
 	position(pos),
 	c_a_c(traceUI->getConstantAttenuation()),
 	l_a_c(traceUI->getLinearAttenuation()),
 	q_a_c(traceUI->getQuadraticAttenuation())
{}


double PointLight::distanceAttenuation( const vec3f& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, I assume no attenuation and just return 1.0
	double d1 = (P - position).length_squared();
	double d2 = sqrt(d1);
	double c = c_a_c + l_a_c * d1 + q_a_c * d2;
	return c == 0.0 ? 1.0 : 1.0 / max<double>(c, 1.0);

}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
	const ray& r = ray(P, getDirection(P));
	double dist = (position - P).length();
	vec3f direction = r.getDirection();
	vec3f final = getColor(P);
	vec3f Pruc = r.getPosition();
	isect Psect;
	ray newr(Pruc, direction);
	while (scene->intersect(newr, Psect)) {
		//prevent going beyond this light 
		if ((dist -= Psect.t) < RAY_EPSILON) return final;
		//if not transparent return black 
		if (Psect.getMaterial().kt.iszero()) return vec3f(0, 0, 0);
		//use current intersection point as new light source 
		Pruc = r.at(Psect.t);
		newr = ray(Pruc, direction);
		final = prod(final, Psect.getMaterial().kt);
	}
	return final;
}

void PointLight::distanceAttenuationCoeff(const double c, const double l, const double q) {
	c_a_c = c;
	l_a_c = l;
	q_a_c = q;
}

double AmbientLight::distanceAttenuation(const vec3f& P) const {
	return 1.0;
}

vec3f AmbientLight::getColor(const vec3f& P) const{
	return color;
}

vec3f AmbientLight::getDirection(const vec3f& P) const {
	return vec3f(1, 1, 1);
}

vec3f AmbientLight::shadowAttenuation(const vec3f& P) const {
	return vec3f(1, 1, 1);
}
