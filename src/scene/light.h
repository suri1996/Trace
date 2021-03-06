#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "scene.h"

class Light
	: public SceneElement
{
public:
	virtual vec3f shadowAttenuation(const vec3f& P) const = 0;
	virtual double distanceAttenuation( const vec3f& P ) const = 0;
	virtual vec3f getColor( const vec3f& P ) const = 0;
	virtual vec3f getDirection( const vec3f& P ) const = 0;

protected:
	Light( Scene *scene, const vec3f& col )
		: SceneElement( scene ), color( col ) {}

	vec3f 		color;
};

class DirectionalLight
	: public Light
{
public:
	DirectionalLight( Scene *scene, const vec3f& orien, const vec3f& color )
		: Light( scene, color ), orientation( orien ) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;

protected:
	vec3f 		orientation;
};

class PointLight
	: public Light
{
public:
	PointLight(Scene *scene, const vec3f& pos, const vec3f& color);
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;
	void distanceAttenuationCoeff(const double c, const double l, const double q);

protected:
	vec3f position;
	double c_a_c, l_a_c, q_a_c;
};

class AmbientLight
	: public Light
{
public:
		AmbientLight(Scene *scene, const vec3f& pos, const vec3f& color)
			:Light(scene, color), color(color) {}
		virtual vec3f shadowAttenuation(const vec3f& P) const;
		virtual double distanceAttenuation(const vec3f& P) const;
		virtual vec3f getColor(const vec3f& P) const;
		virtual vec3f getDirection(const vec3f& P) const;

private:
		vec3f color;
};


#endif // __LIGHT_H__
