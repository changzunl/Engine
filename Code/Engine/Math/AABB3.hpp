#pragma once

#include "Vec3.hpp"

struct AABB3
{
public:
	static const AABB3 ZERO_TO_ONE;

	Vec3 m_mins;
	Vec3 m_maxs;

public:
	~AABB3() {}												// destructor (do nothing)
	AABB3() {}												// default constructor (do nothing)
	AABB3(const AABB3& copyFrom);							// copy constructor (from another Vec3)
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
	explicit AABB3(const Vec3& mins, const Vec3& maxs);

	// 
	bool IsPointInside(const Vec3& point) const;
	Vec3 GetCenter() const;
	Vec3 GetDimensions() const;
	Vec3 GetNearestPoint(const Vec3& point) const;
	Vec3 GetPointAtUV(const Vec3& uv) const;
	Vec3 GetUVForPoint(const Vec3& point) const;
	AABB3 GetShifted(const Vec3& offset) const;

	//
	void Translate(const Vec3& translation);
	void SetCenter(const Vec3& newCenter);
	void SetDimensions(const Vec3& newDimensions);
	void StretchToIncludePoint(const Vec3& point);
	void AlignToBox(const AABB3& target, const Vec3& alignment);
	void AlignToBoxHorizontal(const AABB3& target, float alignment);
	void AlignToBoxVertical(const AABB3& target, float alignment);

};

