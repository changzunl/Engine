#include "Engine/Core/Rgba8.hpp"

#include <string>

struct Rgba8;
struct Vec2;
struct Vec3;
struct AABB3;
struct Mat4x4;
struct RaycastResult3D;
class Clock;
class Camera;
class Renderer;

enum class DebugRenderMode
{
	ALWAYS,
	USEDEPTH,
	XRAY,
};

//------------------------------------------------------------------------
struct DebugRenderConfig
{
	Renderer* m_renderer = nullptr;
	bool m_startHidden = false;
	int m_messagePerScreen = 60;
};

// Setup
void DebugRenderSystemStartup(const DebugRenderConfig& config);
void DebugRenderSystemShutdown();

// Control
void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();
void DebugRenderSetParentClock(Clock& parent);

// Output
void DebugRenderBeginFrame();
void DebugRenderWorld(const Camera& camera);
void DebugRenderScreen(const Camera& camera);
void DebugRenderEndFrame();

// Geometry
void DebugAddUIPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldRay(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const RaycastResult3D& result, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldBasis(const Mat4x4& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldText(const std::string& text, const Mat4x4& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode);
void DebugAddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor);
void DebugAddMessage(const std::string& text, float duration, Rgba8 startColor = Rgba8(), Rgba8 endColor = Rgba8());

