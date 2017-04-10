#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.
	// Initial light get set to ke 
	
	vec3f final = ke;
	vec3f normal = i.N;
	vec3f A = r.at(i.t);
	vec3f Aout = A + i.N*RAY_EPSILON;

	vec3f trans = vec3f(1, 1, 1) - kt; // Transparenmcy determined by Transmissive light 

	vec3f ambient = prod(ka, scene->getAmbient()); // Ambient light 

	final += prod(trans, ambient);
	
	for (Scene::cliter i = scene->beginLights(); i != scene->endLights(); i++) {
		
		vec3f attenuation = (*i)->distanceAttenuation(A)*(*i)->shadowAttenuation(Aout);

		vec3f l = ((*i)->getDirection(A)).normalize();
		double theta = maximum(normal.dot(l), 0.0);
		vec3f diffuse = prod(kd * theta, trans);

		vec3f V = ((2 * (normal.dot(l)) * normal) - l).normalize();
		vec3f specular = ks*(pow(maximum(V*(-r.getDirection()), 0), shininess*128.0));
		final += prod(attenuation, diffuse + specular);
	}
	final = final.clamp();
	return final;

}
