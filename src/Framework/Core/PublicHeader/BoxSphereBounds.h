#pragma once

#include "Math.hpp"
#include <algorithm>
#include <limits>

#define CORE_MIN(a,b) ((a) < (b) ? (a) : (b))
#define CORE_MAX(a,b) ((a) > (b) ? (a) : (b))

namespace Core {

// Axis-aligned bounding box (AABB)
struct CORE_API AABB {
	Float3 Min;
	Float3 Max;

	AABB() {
		float inf = std::numeric_limits<float>::infinity();
		Min = Float3(inf, inf, inf);
		Max = Float3(-inf, -inf, -inf);
	}

	AABB(const Float3& InMin, const Float3& InMax) : Min(InMin), Max(InMax) {}

	// Mark as empty
	void SetEmpty() {
		float inf = std::numeric_limits<float>::infinity();
		Min = Float3(inf, inf, inf);
		Max = Float3(-inf, -inf, -inf);
	}

	bool IsEmpty() const {
		return Min.x > Max.x || Min.y > Max.y || Min.z > Max.z;
	}

	// Expand the bounds to include a point
	void ExpandBy(const Float3& Point) {
		if (IsEmpty()) {
			Min = Point; Max = Point; return;
		}
		Min = Float3(CORE_MIN(Min.x, Point.x), CORE_MIN(Min.y, Point.y), CORE_MIN(Min.z, Point.z));
		Max = Float3(CORE_MAX(Max.x, Point.x), CORE_MAX(Max.y, Point.y), CORE_MAX(Max.z, Point.z));
	}

	// Expand to include another AABB
	void Merge(const AABB& Other) {
		if (Other.IsEmpty()) return;
		if (IsEmpty()) { *this = Other; return; }
		Min = Float3(CORE_MIN(Min.x, Other.Min.x), CORE_MIN(Min.y, Other.Min.y), CORE_MIN(Min.z, Other.Min.z));
		Max = Float3(CORE_MAX(Max.x, Other.Max.x), CORE_MAX(Max.y, Other.Max.y), CORE_MAX(Max.z, Other.Max.z));
	}

	// Center and extent
	Float3 GetCenter() const { return Float3((Min.x+Max.x)*0.5f, (Min.y+Max.y)*0.5f, (Min.z+Max.z)*0.5f); }
	Float3 GetExtent() const { return Float3((Max.x-Min.x)*0.5f, (Max.y-Min.y)*0.5f, (Max.z-Min.z)*0.5f); }
};

// Bounding sphere
struct CORE_API BoundingSphere {
	Float3 Center;
	float Radius;

	BoundingSphere() : Center(0.0f,0.0f,0.0f), Radius(0.0f) {}
	BoundingSphere(const Float3& InCenter, float InRadius) : Center(InCenter), Radius(InRadius) {}

	// Build sphere from AABB (sphere that encloses box)
	static BoundingSphere FromAABB(const AABB& Box) {
		Float3 c = Box.GetCenter();
		Float3 e = Box.GetExtent();
		float r = std::sqrt(e.x*e.x + e.y*e.y + e.z*e.z);
		return BoundingSphere(c, r);
	}
};

// Combined box + sphere bounds (common in rendering engines)
struct CORE_API BoxSphereBounds {
	AABB Box;
	BoundingSphere Sphere;

	BoxSphereBounds() : Box(), Sphere() {}

	explicit BoxSphereBounds(const AABB& InBox) : Box(InBox), Sphere(BoundingSphere::FromAABB(InBox)) {}

	void UpdateFromAABB(const AABB& InBox) {
		Box = InBox;
		Sphere = BoundingSphere::FromAABB(InBox);
	}

	// Expand bounds to include a point
	void ExpandBy(const Float3& Point) {
		Box.ExpandBy(Point);
		Sphere = BoundingSphere::FromAABB(Box);
	}

	// Merge with another bounds
	void Merge(const BoxSphereBounds& Other) {
		Box.Merge(Other.Box);
		Sphere = BoundingSphere::FromAABB(Box);
	}
};

} // namespace NSCore

