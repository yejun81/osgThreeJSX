
#include <osgThreeJSX/Light>
#include <osgThreeJSX/SpotLight>
#include <osgThreeJSX/RenderState>
#include <osg/Texture2D>
#include <osg/Geometry>


using namespace osgThreeJSX;

class SpotLightShadow : public LightShadow
{
public:
	//
	SpotLightShadow()
	{
		_bias = 0.0f;
		_radius = 1.0f;
	}
	//
	SpotLightShadow(const SpotLightShadow& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
	{

	}
	//
	virtual ~SpotLightShadow()
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

		SpotLight* dLight = dynamic_cast<SpotLight*>(light.get());
			   	
		float fov = dLight->getAngle() * 2.0 * 180.0 / osg::PI;
		_camera->setProjectionMatrixAsPerspective(fov, _mapSize.x() / _mapSize.y(), 0.1, dLight->getDistance());

		osg::Vec3 ortho_lightDir = dLight->getDirection();
		osg::Vec3 orthogonalVector = ortho_lightDir ^ osg::Vec3(0.0f, 1.0f, 0.0f);
		if (orthogonalVector.normalize() < 0.5f)
		{
			orthogonalVector = ortho_lightDir ^ osg::Vec3(0.0f, 0.0f, 1.0f);
			orthogonalVector.normalize();
		}

		_camera->setViewMatrixAsLookAt(dLight->getPosition(), dLight->getPosition() + ortho_lightDir * 10.0, 
			orthogonalVector);

		_matrix = _camera->getViewMatrix() *
			_camera->getProjectionMatrix() *
			osg::Matrix::translate(1.0, 1.0, 1.0) *
			osg::Matrix::scale(0.5f, 0.5f, 0.5f);
	}

	META_Object(osg, SpotLightShadow);
};


SpotLight::SpotLight()
{
	this->_intensity = 1.0f;
	this->_distance = 0.0f;
	this->_decay = 1.0f;
	_shadow = new SpotLightShadow();
}

SpotLight::SpotLight(const osg::Vec3& position, const osg::Vec3& direction, const osg::Vec3& color, float intensity, float distance, float angle, float penumbra, float decay) :
	_position(position), _direction(direction), _color(color), _intensity(intensity), _distance(distance), _angle(angle), _penumbra(penumbra), _decay(decay)
{
	_shadow = new SpotLightShadow();
}

SpotLight::~SpotLight()
{

}