
#include <osgThreeJSX/Light>
#include <osgThreeJSX/PointLight>
#include <osgThreeJSX/RenderState>
#include <osg/Texture2D>
#include <osg/Geometry>

using namespace osgThreeJSX;

class PointLightShadow : public LightShadow
{
public:
	//
	PointLightShadow()
	{
		_mapSize = osg::Vec2(512, 512);
		_bias = -0.005f;
		_radius = 1.0f;
		_frameExtents = osg::Vec2i(4, 2);
	}
	//
	PointLightShadow(const PointLightShadow& other, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
	{

	}
	//
	virtual ~PointLightShadow()
	{

	}

	std::vector<osg::ref_ptr<osg::Camera>> _cameras;
public:
	//
	virtual void renderCamera(ShadowMap* shadowMap, osg::ref_ptr<Light>& light, osgUtil::CullVisitor* cv)
	{
		//_cameras[i]->setGraphicsContext(cv->getCurrentCamera()->getGraphicsContext());
		CullSettingAutoRecover ar(cv, osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

		for (size_t i = 0; i < _cameras.size(); i++)
		{
			_cameras[i]->accept(*cv);
		}
	}
	//
	virtual void setupCamera(ShadowMap* shadowMap, const osg::ref_ptr<Light>& light, const osg::ref_ptr<osg::Node>& node)
	{
		osg::Vec4 viewports[] = { osg::Vec4(2, 1, 1, 1), osg::Vec4(0, 1, 1, 1), osg::Vec4(3, 1, 1, 1),
			osg::Vec4(1, 1, 1, 1), osg::Vec4(3, 0, 1, 1), osg::Vec4(1, 0, 1, 1) };

		osg::Vec3 cubeDirections[] = { osg::Vec3(1, 0, 0), osg::Vec3(-1, 0, 0), osg::Vec3(0, 0, 1),
			osg::Vec3(0, 0, -1), osg::Vec3(0, 1, 0), osg::Vec3(0, -1, 0) };

		osg::Vec3 cubeUps[] = { osg::Vec3(0, 1, 0), osg::Vec3(0, 1, 0), osg::Vec3(0, 1, 0),
			osg::Vec3(0, 1, 0), osg::Vec3(0, 0, 1), osg::Vec3(0, 0, -1) };

		PointLight* dLight = dynamic_cast<PointLight*>(light.get());

		for (int i = 0; i < sizeof(viewports) / sizeof(viewports[0]); i++)
		{
			osg::Camera* camera = new osg::Camera;
			camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
			camera->setViewport(_mapSize.x() * viewports[i].x(), _mapSize.y() * viewports[i].y(), _mapSize.x(), _mapSize.y());
			camera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
			camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			camera->attach(osg::Camera::COLOR_BUFFER, _map);
			camera->setClearColor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera->addChild(node);

			osg::ref_ptr<RenderState> rs = new RenderState();
			rs->setupCamera(camera, light);

			camera->setProjectionMatrixAsPerspective(90.0, _mapSize.x() / _mapSize.y(), 0.1, dLight->getDistance());

			osg::Vec3 ortho_lightDir = cubeDirections[i];
			camera->setViewMatrixAsLookAt(dLight->getPosition(), dLight->getPosition() + ortho_lightDir * 10.0, cubeUps[i]);

			_cameras.push_back(camera);
		}

		_matrix.makeTranslate(osg::Vec3() - dLight->getPosition());
	}

	META_Object(osg, PointLightShadow);
};



PointLight::PointLight()
{
	this->_intensity = 1.0f;
	this->_distance = 0.0f;
	this->_decay = 1.0f;
	_shadow = new PointLightShadow();
}

PointLight::PointLight(const osg::Vec3& position, const osg::Vec3& color, float intensity, float distance, float decay) :
	_position(position), _color(color), _intensity(intensity), _distance(distance), _decay(decay)
{
	_shadow = new PointLightShadow();
}

PointLight::~PointLight()
{

}
