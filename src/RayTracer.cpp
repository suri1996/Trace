// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "ui/TraceUI.h"

extern TraceUI* traceUI;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	indexstack.clear();
	return traceRay( scene, r, vec3f(1.0,1.0,1.0), 0 ).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth )
{
	isect i;

	if (scene->intersect(r, i)) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.
		vec3f shade;
		const Material& m = i.getMaterial();
		shade += m.shade(scene, r, i);
		if (depth >= traceUI->getDepth())
			return shade;
		vec3f cp = r.at(i.t);
		vec3f n;
		vec3f dirR = 2 * (i.N*-r.getDirection()) * i.N - (-r.getDirection());
		ray R = ray(cp, dirR);
		if (!i.getMaterial().kr.iszero()) {
			shade += prod(i.getMaterial().kr, traceRay(scene, R, thresh, depth + 1));
		}
		// Refraction - a map which has order so it can be used like a stack
		if (!i.getMaterial().kt.iszero()) { 
			bool tRefract = false;
			ray oppositeRay(cp, r.getDirection()); //without refraction 
			// marker to simulate a stack 
			bool toAdd = false, toErase = false;

			// For now, the interior is just hardcoded 
			// That is, we judge it according to cap and whether it is box 
			if (i.obj->hasInterior()) {
				// refractive index 
				double indexA, indexB;
				// For ray go out of an object 
				if (i.N*r.getDirection() > RAY_EPSILON) {
					if (indexstack.empty())
						indexA = 1.0;
					else
						indexA = indexstack.rbegin()->second.index;// return the refractive index of last object 			
					indexstack.erase(i.obj->getOrder());
					toAdd = true;
					if (indexstack.empty())
						indexB = 1.0;
					else
						indexB = indexstack.rbegin()->second.index;
					n = -i.N;
				}
				// For ray get in the object 
				else {
					if (indexstack.empty())
						indexA = 1.0;
					else
						indexA = indexstack.rbegin()->second.index;
					indexstack.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
					toErase = true;
					indexB = indexstack.rbegin()->second.index;
					n = i.N;
				}
				double iRatio = indexA / indexB;
				double cos_i = max(min(n*((-r.getDirection()).normalize()), 1.0), -1.0); //SYSNOTE: min(x, 1.0) to prevent cos_i becomes bigger than 1 
				double sin_i = sqrt(1 - cos_i*cos_i);
				double sin_t = sin_i * iRatio;
				if (sin_t > 1.0)
					tRefract = true;
				else {
					tRefract = false;
					double cos_t = sqrt(1 - sin_t*sin_t);
					vec3f Tdir = (iRatio*cos_i - cos_t)*n - iRatio*-r.getDirection();
					oppositeRay = ray(cp, Tdir);
					shade += prod(i.getMaterial().kt, traceRay(scene, oppositeRay, thresh, depth + 1));
				}
			}
			if (toAdd)
				indexstack.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
			if (toErase)
				indexstack.erase(i.obj->getOrder());
		}
		shade = shade.clamp();
		return shade;
	}
	else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( scene,x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}