#ifndef __VECTORMATHS_H__
#define __VECTORMATHS_H__

#include "GameUtil.h"
#include<cmath>

namespace VectorMaths
{
	static GLVector3f Normalise(const GLVector3f& original)
	{
		float magnitude = original.length();

		return original / magnitude;
	}

	static GLVector3f Direction(const GLVector3f& from, const GLVector3f& to)
	{
		return Normalise(to - from);
	}

	static float Angle(const GLVector3f& v1, const GLVector3f& v2)
	{
		float dot = Normalise(v1).dot(Normalise(v2));
		return acosf(dot) / DEG2RAD;
	}

	static float SignedAngle(const GLVector3f& v1, const GLVector3f& v2, const GLVector3f& axis)
	{
		// implement later
		float dot = Normalise(v1).dot(Normalise(v2));
		return acosf(dot) / DEG2RAD;
	}
}


#endif