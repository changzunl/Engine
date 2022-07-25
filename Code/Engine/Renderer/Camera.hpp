#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat4x4.hpp"
#include "Engine/Math/Transformation.hpp"
#include "Engine/Math/Vec2.hpp"

#include <vector>

struct Vec3;
struct EulerAngles;

class Camera {

public:
    Camera();
    ~Camera();

    // const getters
    Vec2   GetOrthoBottomLeft() const;
    Vec2   GetOrthoTopRight() const;
    Mat4x4 GetOrthoMatrix() const;
    Mat4x4 GetPerspectiveMatrix() const;
    Mat4x4 GetProjectionMatrix() const;
    Mat4x4 GetRenderMatrix() const;
    Mat4x4 GetViewMatrix() const;
    Mat4x4 GetModelMatrix() const;
    const AABB2& GetViewport() const;
    const Transformation& GetCameraView() const;

    // setters
    void SetViewport(const AABB2& viewport);
    void SetOrthoView(const Vec2& bottomLeft, const Vec2& topRight);
    void SetPerspectiveView(float aspect, float fovDegrees, float near, float far);
    void Translate2D(const Vec2& translation2D);

	void SetViewTransform(const Vec3& translation, const EulerAngles& orientation);
	void SetRenderTransform(const Vec3& iBasis, const Vec3& jBasis, const Vec3& kBasis);

    void PushMatrix();
    void PopMatrix();
    void Translate(const Vec3& translation);
    void Rotate(const EulerAngles& orientation);
    void Scale(const Vec3& scale);
    void Scale(float scale);

private:
    Mat4x4                 m_projMatrix;
	Mat4x4                 m_renderMatrix;
	Mat4x4                 m_viewMatrix;
	Mat4x4                 m_modelMatrix;
	Transformation         m_cameraView;
	std::vector<Mat4x4>    m_modelMatrixStack;
	bool                   m_isOrthoProjection   = true;

    AABB2                  m_viewport            = AABB2::ZERO_TO_ONE;
    AABB2                  m_viewOrtho;
    float                  m_aspect              = 0.0f;
    float                  m_fovDegrees          = 0.0f;
    float                  m_zNear               = 0.0f;
    float                  m_zFar                = 0.0f;
};

