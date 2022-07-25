#include "Engine/Renderer/DebugRender.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/RgbaF.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <vector>

class DebugRenderer;

extern DevConsole* g_theConsole;

DebugRenderer* g_debugRenderer = nullptr;
Clock* g_debugRendererClock = nullptr;
bool* g_debugRendererShow = nullptr;

struct DebugMessage
{
public:
	DebugMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor);

public:
	std::string m_text;
	Stopwatch   m_lifeTimer;
	bool        m_persist = false;
	RgbaF       m_startColor;
	RgbaF       m_endColor;
};

DebugMessage::DebugMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
	: m_text(text)
	, m_lifeTimer(g_debugRendererClock, duration)
	, m_startColor(startColor)
	, m_endColor(endColor)
{
	if (duration < 0)
	{
		m_persist = true;
	}
}

struct DebugScreenText : public DebugMessage
{
public:
	DebugScreenText(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor, const Vec2& position, const Vec2& alignment, float size);

public:
	Vec2        m_screenPos;
	Vec2        m_alignment;
	float       m_size;
};

DebugScreenText::DebugScreenText(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor, const Vec2& position, const Vec2& alignment, float size)
	: DebugMessage(text, duration, startColor, endColor)
	, m_screenPos(position)
	, m_alignment(alignment)
	, m_size(size)
{
	if (duration < 0)
	{
		m_persist = true;
	}
}

struct DebugProp
{
public:
	DebugProp(const Vec3& position, const EulerAngles& orientation, float duration, const Rgba8& startColor, const Rgba8& endColor);
	DebugProp(const Mat4x4& basis, float duration, const Rgba8& startColor, const Rgba8& endColor);

public:
	Mat4x4          m_basis;
	RgbaF           m_startColor;
	RgbaF           m_endColor;
	Stopwatch       m_lifeTimer;
	bool            m_persist        = false;
	DebugRenderMode m_renderMode     = DebugRenderMode::USEDEPTH;
	bool            m_billboard      = false;
	bool            m_wireframe      = false;
	bool            m_transparent    = true;
	VertexList      m_vertices;
	Texture*        m_texture        = nullptr;
};

DebugProp::DebugProp(const Vec3& position, const EulerAngles& orientation, float duration, const Rgba8& startColor, const Rgba8& endColor)
	: m_startColor(startColor)
	, m_endColor(endColor)
	, m_lifeTimer(g_debugRendererClock, duration)
{
	m_basis.AppendTranslation3D(position);
	m_basis.Append(orientation.GetMatrix_XFwd_YLeft_ZUp());

	if (duration < 0)
	{
		m_persist = true;
	}
}

DebugProp::DebugProp(const Mat4x4& basis, float duration, const Rgba8& startColor, const Rgba8& endColor)
	: m_basis(basis)
	, m_startColor(startColor)
	, m_endColor(endColor)
	, m_lifeTimer(g_debugRendererClock, duration)
{
	if (duration < 0)
	{
		m_persist = true;
	}
}

class DebugRenderer
{
public:
	DebugRenderConfig m_theConfig;

	Clock m_clock;
	bool  m_show = false;
	std::vector<DebugMessage*> m_messages;
	std::vector<DebugScreenText*> m_texts;
	std::vector<DebugProp*> m_props;

	// async members
	std::vector<DebugMessage*> m_asyncMessages;
	std::vector<DebugScreenText*> m_asyncTexts;
	std::vector<DebugProp*> m_asyncProps;

	std::thread::id m_mainThreadId;
	std::mutex m_asyncMutex;
};

void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	g_debugRenderer = new DebugRenderer();
	g_debugRenderer->m_mainThreadId = std::this_thread::get_id();
	g_debugRenderer->m_theConfig = config;
	g_debugRenderer->m_show = !config.m_startHidden;
	g_debugRendererClock = &g_debugRenderer->m_clock;
	g_debugRendererShow = &g_debugRenderer->m_show;
}

void DebugRenderSystemShutdown()
{
	delete g_debugRenderer;
	g_debugRenderer = nullptr;
	g_debugRendererClock = nullptr;
	g_debugRendererShow = nullptr;
}

void DebugRenderSetVisible()
{
	*g_debugRendererShow = true;
}

void DebugRenderSetHidden()
{
	*g_debugRendererShow = false;
}

void DebugRenderClear()
{
	g_debugRenderer->m_messages.clear();
	g_debugRenderer->m_texts.clear();
	g_debugRenderer->m_props.clear();
}

void DebugRenderSetParentClock(Clock& parent)
{
	g_debugRenderer->m_clock.SetParent(parent);
}

void DebugRenderBeginFrame()
{
	std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);

	g_debugRenderer->m_props.insert(g_debugRenderer->m_props.end(), g_debugRenderer->m_asyncProps.begin(), g_debugRenderer->m_asyncProps.end());
	g_debugRenderer->m_asyncProps.clear();

	g_debugRenderer->m_texts.insert(g_debugRenderer->m_texts.end(), g_debugRenderer->m_asyncTexts.begin(), g_debugRenderer->m_asyncTexts.end());
	g_debugRenderer->m_asyncTexts.clear();

	g_debugRenderer->m_messages.insert(g_debugRenderer->m_messages.end(), g_debugRenderer->m_asyncMessages.begin(), g_debugRenderer->m_asyncMessages.end());
	g_debugRenderer->m_asyncMessages.clear();
}

void DebugRenderWorld(const Camera& camera)
{
	if (!g_debugRenderer->m_show)
		return;

	Renderer* renderer = g_debugRenderer->m_theConfig.m_renderer;
	renderer->BeginCamera(camera);

	Mat4x4 modelMatrix;
	Mat4x4 viewMatrix = camera.GetViewMatrix().GetOrthonormalInverse();
	for (auto& pprop : g_debugRenderer->m_props)
	{
		auto& prop = *pprop;
		if (prop.m_billboard)
		{
			viewMatrix.SetTranslation3D(prop.m_basis.GetTranslation3D());
			renderer->SetModelMatrix(viewMatrix);
		}
		else
		{
			modelMatrix.SetIdentity();
			modelMatrix.Append(prop.m_basis);
			renderer->SetModelMatrix(modelMatrix);
		}

		RgbaF colorF = prop.m_lifeTimer.IsStopped() ? prop.m_startColor : RgbaF::LerpColor(prop.m_startColor, prop.m_endColor, prop.m_lifeTimer.GetElapsedFraction());
		Rgba8 color = colorF.GetAsRgba8();
		renderer->SetTintColor(color);
		renderer->BindTexture(prop.m_texture);
		renderer->SetBlendMode(prop.m_transparent ? BlendMode::ALPHA : BlendMode::OPAQUE);
		renderer->SetFillMode(prop.m_wireframe ? FillMode::WIREFRAME : FillMode::SOLID);

		switch (prop.m_renderMode)
		{
		case DebugRenderMode::ALWAYS:
		{
			renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
			renderer->DrawVertexArray(prop.m_vertices);
			break;
		}
		case DebugRenderMode::USEDEPTH:
		{
			renderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
			renderer->DrawVertexArray(prop.m_vertices);
			break;
		}
		case DebugRenderMode::XRAY:
		{
			colorF.r += 0.2f;
			colorF.g += 0.2f;
			colorF.b += 0.2f;
			colorF.a *= 0.5f;
			colorF.Normalize();
			Rgba8 xrayColor = colorF.GetAsRgba8();

			renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
			renderer->SetTintColor(xrayColor);
			renderer->SetBlendMode(BlendMode::ALPHA);
			renderer->DrawVertexArray(prop.m_vertices);

			renderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
			renderer->SetTintColor(color);
			renderer->SetBlendMode(prop.m_transparent ? BlendMode::ALPHA : BlendMode::OPAQUE);
			renderer->DrawVertexArray(prop.m_vertices);
			break;
		}
		}
	}
}

void DebugRenderScreenText();
void DebugRenderMessages(const Camera& camera);

void DebugRenderScreen(const Camera& camera)
{
	if (!g_debugRenderer->m_show)
		return;

	Renderer* renderer = g_debugRenderer->m_theConfig.m_renderer;
	renderer->BeginCamera(camera);

	DebugRenderScreenText();
	DebugRenderMessages(camera);
}

void DebugRenderMessages(const Camera& camera)
{
	constexpr float CARET_WIDTH = 2.5f;
	constexpr float FONT_ASPECT = 0.65f;
	
	const int LINES_PER_SCREEN = g_debugRenderer->m_theConfig.m_messagePerScreen;

	AABB2 bounds = AABB2(camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight());
	Renderer* renderer = g_debugRenderer->m_theConfig.m_renderer;
	BitmapFont* font = g_theConsole->GetFont();

	static std::vector<Vertex_PCU> s_verts(1024 * 1024);
	s_verts.clear();

	float lineHeight = bounds.GetDimensions().y / (float)(LINES_PER_SCREEN + 1);
	float lineSide = lineHeight * 0.05f;
	float textHeight = lineHeight * 0.9f;

	std::vector<DebugMessage> messages;
	for (auto& pmessage : g_debugRenderer->m_messages)
	{
		auto& message = *pmessage;
		if (message.m_persist)
		{
			messages.push_back(message);
		}
	}
	for (auto& pmessage : g_debugRenderer->m_messages)
	{
		auto& message = *pmessage;
		if (!message.m_persist && message.m_lifeTimer.IsStopped())
		{
			messages.push_back(message);
		}
	}
	for (auto& pmessage : g_debugRenderer->m_messages)
	{
		auto& message = *pmessage;
		if (!message.m_persist && !message.m_lifeTimer.IsStopped())
		{
			messages.push_back(message);
		}
	}

	for (int idx = 0; idx < messages.size() && idx < LINES_PER_SCREEN; idx++)
	{
		const auto& line = messages[idx];
		RgbaF colorF = line.m_lifeTimer.IsStopped() ? line.m_startColor : RgbaF::LerpColor(line.m_startColor, line.m_endColor, line.m_lifeTimer.GetElapsedFraction());
		font->AddVertsForText2D(s_verts, Vec2(lineSide, lineSide - lineHeight * (idx + 1)) + Vec2(bounds.m_mins.x, bounds.m_maxs.y), textHeight, line.m_text, colorF.GetAsRgba8(), FONT_ASPECT);
	}

	renderer->BindTexture(&font->GetTexture());
	renderer->DrawVertexArray((int)s_verts.size(), s_verts.data());
	renderer->BindTexture(nullptr);
}

void DebugRenderScreenText()
{
	constexpr float CARET_WIDTH = 2.5f;
	constexpr float FONT_ASPECT = 0.65f;
	constexpr int   LINES_PER_SCREEN = 30;

	AABB2 bounds = AABB2(-1.0f, -1.0f, 1.0f, 1.0f);
	Renderer* renderer = g_debugRenderer->m_theConfig.m_renderer;
	BitmapFont* font = g_theConsole->GetFont();

	static std::vector<Vertex_PCU> s_verts(1024 * 1024);
	s_verts.clear();

	for (auto& ptext : g_debugRenderer->m_texts)
	{
		auto& text = *ptext;
		AABB2 bounds1 = AABB2();
		bounds1.SetCenter(text.m_screenPos);
		bounds1.SetDimensions(Vec2(0.0f, 0.0f));
		RgbaF colorF = text.m_lifeTimer.IsStopped() ? text.m_startColor : RgbaF::LerpColor(text.m_startColor, text.m_endColor, text.m_lifeTimer.GetElapsedFraction());
		font->AddVertsForTextInBox2D(s_verts, bounds1, text.m_size, text.m_text, colorF.GetAsRgba8(), FONT_ASPECT, text.m_alignment, TextDrawMode::OVERRUN);
	}

	renderer->BindTexture(&font->GetTexture());
	renderer->DrawVertexArray((int)s_verts.size(), s_verts.data());
	renderer->BindTexture(nullptr);
}

void DebugRenderEndFrame()
{
	{
		auto ite = g_debugRenderer->m_props.begin();
		while (ite != g_debugRenderer->m_props.end())
		{
			auto& entity = **ite;
			if (entity.m_persist)
			{
				ite++;
				continue;
			}
			else if (entity.m_lifeTimer.IsStopped() || entity.m_lifeTimer.CheckDurationElapsedAndDecrement())
			{
				ite = g_debugRenderer->m_props.erase(ite);
				continue;
			}
			else
			{
				ite++;
				continue;
			}
		}
	}

	{
		auto ite = g_debugRenderer->m_messages.begin();
		while (ite != g_debugRenderer->m_messages.end())
		{
			auto& entity = **ite;
			if (entity.m_persist)
			{
				ite++;
				continue;
			}
			else if (entity.m_lifeTimer.IsStopped() || entity.m_lifeTimer.CheckDurationElapsedAndDecrement())
			{
				ite = g_debugRenderer->m_messages.erase(ite);
				continue;
			}
			else
			{
				ite++;
				continue;
			}
		}
	}

	{
		auto ite = g_debugRenderer->m_texts.begin();
		while (ite != g_debugRenderer->m_texts.end())
		{
			auto& entity = **ite;
			if (entity.m_persist)
			{
				ite++;
				continue;
			}
			else if (entity.m_lifeTimer.IsStopped() || entity.m_lifeTimer.CheckDurationElapsedAndDecrement())
			{
				ite = g_debugRenderer->m_texts.erase(ite);
				continue;
			}
			else
			{
				ite++;
				continue;
			}
		}
	}
}

void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(pos, EulerAngles(), duration, startColor, endColor);

	prop->m_renderMode = mode;
	AddVertsForSphere(prop->m_vertices, Vec3(), radius, Rgba8::WHITE);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Vec3 displacement = end - start;
	float length = displacement.GetLength();
	Vec3 direction = displacement / length;

	DebugProp* prop = new DebugProp(start, DirectionToRotation(direction), duration, startColor, endColor);

	prop->m_renderMode = mode;
// 	AABB3 aabb3(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f);
// 	aabb3.SetDimensions(Vec3(length, radius, radius));
// 
// 	AddVertsForAABB3D(prop.m_vertices, aabb3, Rgba8::WHITE);
	AddVertsForXCylinder(prop->m_vertices, Vec3(), radius, length, Rgba8::WHITE);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Vec3 displacement = top - base;
	float length = displacement.GetLength();
	Vec3 direction = displacement / length;

	DebugProp* prop = new DebugProp(base, DirectionToRotation(direction), duration, startColor, endColor);

	prop->m_renderMode = mode;
	prop->m_wireframe = true;
	AddVertsForXCylinder(prop->m_vertices, Vec3(), radius, length, Rgba8::WHITE);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldWireSphere(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(pos, EulerAngles(), duration, startColor, endColor);

	prop->m_renderMode = mode;
	prop->m_wireframe = true;
	AddVertsForSphere(prop->m_vertices, Vec3(), radius, Rgba8::WHITE);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	Vec3 displacement = end - start;
	float length = displacement.GetLength();
	Vec3 direction = displacement / length;

	DebugProp* prop = new DebugProp(start, DirectionToRotation(direction), duration, startColor, endColor);

	prop->m_renderMode = mode;
	float arrowSize = radius * 5.0f;
	AddVertsForXCylinder(prop->m_vertices, Vec3(), radius, length - arrowSize, baseColor);
	AddVertsForXCone(prop->m_vertices, Vec3(length - arrowSize, 0.0f, 0.0f), radius * 2.0f, arrowSize, baseColor);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldRay(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const RaycastResult3D& result, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(startPos, DirectionToRotation(fwdNormal), duration, startColor, endColor);

	prop->m_renderMode = mode;
	float arrowLength = radius * 5.0f;
	float arrowRadius = radius * 2.5f;

	if (result.DidImpact())
	{
		AddVertsForXCylinder(prop->m_vertices, Vec3(), radius, result.GetImpactDistance(), Rgba8::WHITE);
		AddVertsForXCylinder(prop->m_vertices, Vec3(1.0f, 0.0f, 0.0f) * result.GetImpactDistance(), radius, maxDist - arrowLength - result.GetImpactDistance(), Rgba8::RED);
		AddVertsForXCone(prop->m_vertices, Vec3(maxDist - arrowLength, 0.0f, 0.0f), arrowRadius, arrowLength, Rgba8::RED);

		DebugProp* normProp = new DebugProp(result.GetImpactPosition(), DirectionToRotation(result.GetImpactNormal()), duration, startColor, endColor);

		AddVertsForSphere(normProp->m_vertices, Vec3::ZERO, radius * 2.0f, Rgba8::WHITE);
		AddVertsForXCylinder(normProp->m_vertices, Vec3(), radius, 1.0f - arrowLength, Rgba8(255, 255, 0));
		AddVertsForXCone(normProp->m_vertices, Vec3(1.0f - arrowLength, 0.0f, 0.0f), arrowRadius, arrowLength, Rgba8(255, 255, 0));

		if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
		{
			g_debugRenderer->m_props.push_back(normProp);
			g_debugRenderer->m_props.push_back(prop);
		}
		else
		{
			std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
			g_debugRenderer->m_asyncProps.push_back(normProp);
			g_debugRenderer->m_asyncProps.push_back(prop);
		}
	}
	else
	{
		AddVertsForXCylinder(prop->m_vertices, Vec3(), radius, maxDist - arrowLength, Rgba8::WHITE);
		AddVertsForXCone(prop->m_vertices, Vec3(maxDist - arrowLength, 0.0f, 0.0f), arrowRadius, arrowLength, Rgba8::WHITE);

		if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
		{
			g_debugRenderer->m_props.push_back(prop);
		}
		else
		{
			std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
			g_debugRenderer->m_asyncProps.push_back(prop);
		}
	}
}

void DebugAddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(Vec3(), EulerAngles(), duration, startColor, endColor);

	prop->m_renderMode = mode;
	AddVertsForAABB3D(prop->m_vertices, bounds, Rgba8::WHITE);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldBasis(const Mat4x4& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(basis, duration, startColor, endColor);

	prop->m_renderMode = mode;

	VertexList verts;

	AddVertsForXCylinder(verts, Vec3(), 0.1f, 1.8f, Rgba8(255, 0, 0));
	AddVertsForXCone(verts, Vec3(1.8f, 0.0f, 0.0f), 0.2f, 0.2f, Rgba8(255, 0, 0));
	prop->m_vertices.insert(prop->m_vertices.end(), verts.begin(), verts.end());

	for (auto& vert : verts)
	{
		vert.m_color = Rgba8(0, 255, 0);
	}
	TransformVertexArray(Mat4x4::CreateZRotationDegrees(90.0f), verts);
	prop->m_vertices.insert(prop->m_vertices.end(), verts.begin(), verts.end());

	for (auto& vert : verts)
	{
		vert.m_color = Rgba8(0, 0, 255);
	}
	TransformVertexArray(Mat4x4::CreateXRotationDegrees(90.0f), verts);
	prop->m_vertices.insert(prop->m_vertices.end(), verts.begin(), verts.end());

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldText(const std::string& text, const Mat4x4& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(transform, duration, startColor, endColor);

	prop->m_renderMode = mode;

	BitmapFont* font = g_theConsole->GetFont();
	prop->m_texture = &font->GetTexture();
	static AABB2 box = AABB2(0.0f, 0.0f, 1.0f, 1.0f);
	font->AddVertsForTextInBox2D(prop->m_vertices, box, textHeight, text, Rgba8::WHITE, 0.65f, alignment, TextDrawMode::OVERRUN);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugProp* prop = new DebugProp(origin, EulerAngles(), duration, startColor, endColor);

	prop->m_renderMode = mode;
	prop->m_billboard = true;

	BitmapFont* font = g_theConsole->GetFont();
	prop->m_texture = &font->GetTexture();
	prop->m_transparent = true;
	static AABB2 box = AABB2(-1.0f, -1.0f, 1.0f, 1.0f);
	font->AddVertsForTextInBox2D(prop->m_vertices, box, textHeight, text, Rgba8::WHITE, 0.65f, alignment, TextDrawMode::OVERRUN);
	Mat4x4 renderMatrix;
	renderMatrix.SetIJK3D(Vec3(0, -1, 0), Vec3(0, 0, 1), Vec3(1, 0, 0));
	TransformVertexArray(renderMatrix, prop->m_vertices);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_props.push_back(prop);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncProps.push_back(prop);
	}
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor)
{
	DebugScreenText* scrtext = new DebugScreenText(text, duration, startColor, endColor, position, alignment, size);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_texts.push_back(scrtext);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncTexts.push_back(scrtext);
	}
}

void DebugAddMessage(const std::string& text, float duration, Rgba8 startColor, Rgba8 endColor)
{
	DebugMessage* msg = new DebugMessage(text, duration, startColor, endColor);

	if (std::this_thread::get_id() == g_debugRenderer->m_mainThreadId)
	{
		g_debugRenderer->m_messages.push_back(msg);
	}
	else
	{
		std::lock_guard<std::mutex> guard(g_debugRenderer->m_asyncMutex);
		g_debugRenderer->m_asyncMessages.push_back(msg);
	}
}

