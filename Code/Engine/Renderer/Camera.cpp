#include "Engine/Renderer/Camera.hpp"

#include "Engine/Math/EulerAngles.hpp"

Camera::Camera()
{
	m_modelMatrixStack.reserve(16);
}

Camera::~Camera()
{
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_viewOrtho.m_mins;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_viewOrtho.m_maxs;
}

Mat4x4 Camera::GetOrthoMatrix() const
{
	float minX = m_viewOrtho.m_mins.x;
	float maxX = m_viewOrtho.m_maxs.x;
	float minY = m_viewOrtho.m_mins.y;
	float maxY = m_viewOrtho.m_maxs.y;
	return Mat4x4::CreateOrthoProjection(minX, maxX, minY, maxY, 0.0f, 1.0f);
}

Mat4x4 Camera::GetPerspectiveMatrix() const
{
	return Mat4x4::CreatePerspectiveProjection(m_fovDegrees, m_aspect, m_zNear, m_zFar);
}

Mat4x4 Camera::GetProjectionMatrix() const
{
	return m_projMatrix;
}

Mat4x4 Camera::GetRenderMatrix() const
{
	return m_renderMatrix;
}

Mat4x4 Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

Mat4x4 Camera::GetModelMatrix() const
{
	return m_modelMatrix;
}

const AABB2& Camera::GetViewport() const
{
	return m_viewport;
}

const Transformation& Camera::GetCameraView() const
{
	return m_cameraView;
}

void Camera::SetViewport(const AABB2& viewport)
{
	m_viewport = viewport;
}

void Camera::SetOrthoView(const Vec2& bottomLeft, const Vec2& topRight)
{
	m_viewOrtho.m_mins = bottomLeft;
	m_viewOrtho.m_maxs = topRight;

	m_isOrthoProjection = true;
	m_projMatrix = GetOrthoMatrix();
}

void Camera::SetPerspectiveView(float aspect, float fovDegrees, float near, float far)
{
	m_aspect = aspect;
	m_fovDegrees = fovDegrees;
	m_zNear = near;
	m_zFar = far;

	m_isOrthoProjection = false;
	m_projMatrix = GetPerspectiveMatrix();
}

void Camera::Translate2D(const Vec2& translation2D)
{
	m_viewMatrix.AppendTranslation2D(translation2D);
	m_cameraView.m_position += Vec3(translation2D);
}

void Camera::SetViewTransform(const Vec3& translation, const EulerAngles& orientation)
{
	m_viewMatrix.SetIdentity();
	m_viewMatrix.SetTranslation3D(translation);
	m_viewMatrix.Append(orientation.GetMatrix_XFwd_YLeft_ZUp());
	m_viewMatrix = m_viewMatrix.GetOrthonormalInverse();
	m_cameraView.m_position = translation;
	m_cameraView.m_orientation = orientation;
}

void Camera::SetRenderTransform(const Vec3& iBasis, const Vec3& jBasis, const Vec3& kBasis)
{
	m_renderMatrix.SetIJK3D(iBasis, jBasis, kBasis);
}

void Camera::PushMatrix()
{
	m_modelMatrixStack.push_back(m_modelMatrix);
}

void Camera::PopMatrix()
{
	m_modelMatrixStack.pop_back();
}

void Camera::Translate(const Vec3& translation)
{
	m_modelMatrix.AppendTranslation3D(translation);
}

void Camera::Rotate(const EulerAngles& orientation)
{
	m_modelMatrix.Append(orientation.GetMatrix_XFwd_YLeft_ZUp());
}

void Camera::Scale(const Vec3& scale)
{
	m_modelMatrix.AppendScaleNonUniform3D(scale);
}

void Camera::Scale(float scale)
{
	m_modelMatrix.AppendScaleUniform3D(scale);
}

