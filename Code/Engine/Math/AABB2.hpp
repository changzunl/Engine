#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/FloatRange.hpp"

struct AABB2
{
public:
	static const AABB2 ZERO_TO_ONE;

	Vec2 m_mins;
	Vec2 m_maxs;

public:
	~AABB2() {}                                                        // destructor (do nothing)
	AABB2() {}                                                         // default constructor (do nothing)
	AABB2(const AABB2& copyFrom);                                      // copy constructor (from another AABB2)
	AABB2(AABB2&& moveFrom);                                           // move constructor (from another AABB2)
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(const Vec2& mins, const Vec2& maxs);

	// 
	bool IsPointInside(const Vec2& point) const;
	Vec2 GetCenter() const;
	Vec2 GetDimensions() const;
	Vec2 GetNearestPoint(const Vec2& point) const;
	Vec2 GetPointAtUV(const Vec2& uv) const;
	Vec2 GetUVForPoint(const Vec2& point) const;
	AABB2 GetSubBox(const AABB2& uv) const;
	FloatRange GetRangeX() const;
	FloatRange GetRangeY() const;
	AABB2 GetIntersection(const AABB2& other) const;
	bool HasIntersection(const AABB2& other) const;

	//
	void Translate(const Vec2& translation);
	void SetCenter(const Vec2& newCenter);
	void SetDimensions(const Vec2& newDimensions);
	void StretchToIncludePoint(const Vec2& point);
	void AlignToBox(const AABB2& target, const Vec2& alignment);
	void AlignToBoxHorizontal(const AABB2& target, float alignment);
	void AlignToBoxVertical(const AABB2& target, float alignment);

	//
	AABB2 ChopOffTop(float v, bool rel = true);
	AABB2 ChopOffBottom(float v, bool rel = true);
	AABB2 ChopOffLeft(float u, bool rel = true);
	AABB2 ChopOffRight(float u, bool rel = true);

	bool operator==(const AABB2& other);
	bool operator!=(const AABB2& other);
	void operator=(const AABB2& other);
	void operator=(AABB2&& other) noexcept;

};

