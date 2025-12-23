
#include <osgThreeJSX/Light>
#include <osgThreeJSX/DirectionalLight>
#include <osgThreeJSX/RenderState>
#include <osg/Texture2D>
#include <osg/Geometry>


using namespace osgThreeJSX;

class DirectionalLightShadow : public LightShadow
{
public:
	//
	DirectionalLightShadow()
	{

	}
	//
	DirectionalLightShadow(const DirectionalLightShadow& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
	{

	}
	//
	virtual ~DirectionalLightShadow()
	{

	}

public:
	//
	virtual void renderCamera(ShadowMap* shadowMap, osg::ref_ptr<Light>& light, osgUtil::CullVisitor* cv)
	{
		CullSettingAutoRecover ar(cv, osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

		_camera->accept(*cv);
		_matrix = _camera->getViewMatrix() *
			_camera->getProjectionMatrix() *
			osg::Matrix::translate(1.0, 1.0, 1.0) *
			osg::Matrix::scale(0.5f, 0.5f, 0.5f);
	}
	//
	virtual void setupCamera(ShadowMap* shadowMap, const osg::ref_ptr<Light>& light, const osg::ref_ptr<osg::Node>& node)
	{
		_camera = new osg::Camera;
		_camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
		_camera->setViewport(0, 0, _mapSize.x(), _mapSize.y());
		_camera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
		_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		_camera->attach(osg::Camera::COLOR_BUFFER, _map);
		_camera->setClearColor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
		_camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		_camera->addChild(node);

		osg::ref_ptr<RenderState> rs = new RenderState();
		rs->setupCamera(_camera, light);

		DirectionalLight* dLight = dynamic_cast<DirectionalLight*>(light.get());

		osg::BoundingSphere bb = node->getBound();
		osg::Vec3 ortho_lightDir = dLight->getDirection();
		osg::Vec3 position = bb.center() - ortho_lightDir * bb.radius() * 2.0;

		float centerDistance = (position - bb.center()).length();
		float znear = centerDistance - bb.radius();
		float zfar = centerDistance + bb.radius();
		float zNearRatio = 0.001f;
		if (znear < zfar*zNearRatio) znear = zfar * zNearRatio;

		float top = bb.radius();
		float right = top;

		_camera->setProjectionMatrixAsOrtho(-right, right, -top, top, znear, zfar);

		float length = ortho_lightDir.length();
		osg::Vec3 orthogonalVector = ortho_lightDir ^ osg::Vec3(0.0f, 1.0f, 0.0f);
		if (orthogonalVector.normalize() < length*0.5f)
		{
			orthogonalVector = ortho_lightDir ^ osg::Vec3(0.0f, 0.0f, 1.0f);
			orthogonalVector.normalize();
		}

		_camera->setViewMatrixAsLookAt(position, bb.center(), orthogonalVector);

		_matrix = _camera->getViewMatrix() *
			_camera->getProjectionMatrix() *
			osg::Matrix::translate(1.0, 1.0, 1.0) *
			osg::Matrix::scale(0.5f, 0.5f, 0.5f);
	}

	META_Object(osg, DirectionalLightShadow);
};

DirectionalLight::DirectionalLight()
{
	_intensity = 1.0f;
	_shadow = new DirectionalLightShadow();
}

DirectionalLight::DirectionalLight(const osg::Vec3& direction, const osg::Vec3& color, float intensity, bool isflow) :
	_direction(direction), _color(color), _intensity(intensity), _isflow(isflow)
{
	_shadow = new DirectionalLightShadow();
}
