
#include <osgThreeJSX/Light>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/Materials>
#include <osg/Texture2D>
#include <osg/Geometry>

static const char* g_shader_vsm_vert = R"(
void main() {

	gl_Position = vec4( position, 1.0 );

}
)";

static const char* g_shader_vsm_frag = R"(
uniform sampler2D shadow_pass;
uniform vec2 resolution;
uniform float radius;

#include <packing>

void main() {

  float mean = 0.0;
  float squared_mean = 0.0;

	// This seems totally useless but it's a crazy work around for a Adreno compiler bug
	float depth = unpackRGBAToDepth( texture2D( shadow_pass, ( gl_FragCoord.xy  ) / resolution ) );

  for ( float i = -1.0; i < 1.0 ; i += SAMPLE_RATE) {

    #ifdef HORIZONAL_PASS

      vec2 distribution = unpackRGBATo2Half( texture2D( shadow_pass, ( gl_FragCoord.xy + vec2( i, 0.0 ) * radius ) / resolution ) );
      mean += distribution.x;
      squared_mean += distribution.y * distribution.y + distribution.x * distribution.x;

    #else

      float depth = unpackRGBAToDepth( texture2D( shadow_pass, ( gl_FragCoord.xy + vec2( 0.0,  i )  * radius ) / resolution ) );
      mean += depth;
      squared_mean += depth * depth;

    #endif

  }

  mean = mean * HALF_SAMPLE_RATE;
  squared_mean = squared_mean * HALF_SAMPLE_RATE;

  float std_dev = sqrt( squared_mean - mean * mean );

  gl_FragColor = pack2HalfToRGBA( vec2( mean, std_dev ) );

}
)";

using namespace osgThreeJSX;

LightShadow::LightShadow()
{
	_mapSize = osg::Vec2(2048, 2048);
	_frameExtents = osg::Vec2i(1, 1);
	_bias = -0.01f;
	_radius = 1.0f;

	_inited = false;
}

LightShadow::LightShadow(const LightShadow& other, const osg::CopyOp& copyop /* = osg::CopyOp::SHALLOW_COPY */)
{

}

LightShadow::~LightShadow()
{

}

void LightShadow::render(ShadowMap* shadowMap, osg::ref_ptr<Light>& light, osg::ref_ptr<osg::Node>& sceneNode, osgUtil::CullVisitor* cv)
{
	if (_inited == false)
	{
		//
		setupTexture(shadowMap, light);
		//
		setupCamera(shadowMap, light, sceneNode);
		//
		setupVSM(shadowMap, light);

		_inited = true;
	}

	renderCamera(shadowMap, light, cv);

	renderVSM(shadowMap, cv);
}

void LightShadow::setupTexture(ShadowMap* shadowMap, const osg::ref_ptr<Light>& light)
{
	{
		osg::Texture2D* texture = new osg::Texture2D();
		texture->setTextureSize(_mapSize.x() * _frameExtents.x(), _mapSize.y() * _frameExtents.y());
		texture->setInternalFormat(GL_RGBA);//GL_RGBA16F_ARB
		texture->setSourceFormat(GL_RGBA);
		texture->setSourceType(GL_UNSIGNED_BYTE);
		texture->setBorderColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));

		if ((shadowMap->getMapType() == ShadowMapType_VSMShadowMap) && (light->getType() != LightType_Point))
		{
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		}
		else
		{
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		}

		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		_map = texture;
	}

	if ((shadowMap->getMapType() == ShadowMapType_VSMShadowMap) && (light->getType() != LightType_Point))
	{
		osg::Texture2D* texture = new osg::Texture2D();
		texture->setTextureSize(_mapSize.x() * _frameExtents.x(), _mapSize.y() * _frameExtents.y());
		texture->setInternalFormat(GL_RGBA);//GL_RGBA16F_ARB
		texture->setSourceFormat(GL_RGBA);
		texture->setSourceType(GL_UNSIGNED_BYTE);
		texture->setBorderColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));

		if ((shadowMap->getMapType() == ShadowMapType_VSMShadowMap) && (light->getType() != LightType_Point))
		{
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		}
		else
		{
			texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
			texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		}

		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		_mapVsm = texture;
	}
}


void LightShadow::setupVSM(ShadowMap* shadowMap, const osg::ref_ptr<Light>& light)
{
	if ((shadowMap->getMapType() == ShadowMapType_VSMShadowMap) && (light->getType() != LightType_Point))
	{
		{
			_cameraVsmVertical = new osg::Camera;
			_cameraVsmVertical->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
			_cameraVsmVertical->setViewport(0, 0, _mapSize.x(), _mapSize.y());
			_cameraVsmVertical->setRenderOrder(osg::Camera::PRE_RENDER, 1);
			_cameraVsmVertical->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			_cameraVsmVertical->attach(osg::Camera::COLOR_BUFFER, _mapVsm);
			_cameraVsmVertical->setClearColor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			_cameraVsmVertical->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			_cameraVsmVertical->setProjectionMatrix(osg::Matrix::ortho2D(0, 1, 0, 1));
			_cameraVsmVertical->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

			osg::Vec3Array *vertices = new osg::Vec3Array;
			vertices->push_back(osg::Vec3(-1.0, -1, 0.5));
			vertices->push_back(osg::Vec3(3, -1, 0.5));
			vertices->push_back(osg::Vec3(-1, 3, 0.5));

			osg::Geometry *geom = new osg::Geometry;
			geom->setName("RenderFboPanel");
			geom->setVertexArray(vertices);
			geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 3));
			geom->setUseVertexBufferObjects(true);			
			
			osg::ref_ptr<osgThreeJSX::MaterialShader> material = new osgThreeJSX::MaterialShader();
			material->setVertexShader(g_shader_vsm_vert);
			material->setFragmentsShader(g_shader_vsm_frag);

			material->addDefine("SAMPLE_RATE", "0.25");
			material->addDefine("HALF_SAMPLE_RATE", "0.125");

			material->setTexture("shadow_pass", _map);
			material->setUniform("radius", _radius);
			material->setUniform("resolution", _mapSize);

			osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
			geode->setMaterial(material);
			geode->addDrawable(geom);
			_cameraVsmVertical->addChild(geode);

			osg::ref_ptr<RenderState> rs = new RenderState();
			rs->setupCamera(_cameraVsmVertical);
		}

		{
			_cameraVsmHorizonal = new osg::Camera;
			_cameraVsmHorizonal->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
			_cameraVsmHorizonal->setViewport(0, 0, _mapSize.x(), _mapSize.y());
			_cameraVsmHorizonal->setRenderOrder(osg::Camera::PRE_RENDER, 1);
			_cameraVsmHorizonal->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			_cameraVsmHorizonal->attach(osg::Camera::COLOR_BUFFER, _map);
			_cameraVsmHorizonal->setClearColor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			_cameraVsmHorizonal->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			_cameraVsmHorizonal->setProjectionMatrix(osg::Matrix::ortho2D(0, 1, 0, 1));
			_cameraVsmHorizonal->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

			osg::Vec3Array *vertices = new osg::Vec3Array;
			vertices->push_back(osg::Vec3(-1.0, -1, 0.5));
			vertices->push_back(osg::Vec3(3, -1, 0.5));
			vertices->push_back(osg::Vec3(-1, 3, 0.5));

			osg::Geometry *geom = new osg::Geometry;
			geom->setName("RenderFboPanel");
			geom->setVertexArray(vertices);
			geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 3));
			geom->setUseVertexBufferObjects(true);

			osg::ref_ptr<osgThreeJSX::MaterialShader> material = new osgThreeJSX::MaterialShader();
			material->setVertexShader(g_shader_vsm_vert);
			material->setFragmentsShader(g_shader_vsm_frag);

			material->addDefine("SAMPLE_RATE", "0.25");
			material->addDefine("HALF_SAMPLE_RATE", "0.125");
			material->addDefine("HORIZONAL_PASS", "1");

			material->setTexture("shadow_pass", _mapVsm);
			material->setUniform("radius", _radius);
			material->setUniform("resolution", _mapSize);

			osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
			geode->setMaterial(material);
			geode->addDrawable(geom);
			_cameraVsmHorizonal->addChild(geode);

			osg::ref_ptr<RenderState> rs = new RenderState();
			rs->setupCamera(_cameraVsmHorizonal);
		}

	}
}

void LightShadow::renderVSM(ShadowMap* shadowMap, osgUtil::CullVisitor* cv)
{
	if (_cameraVsmVertical)
		_cameraVsmVertical->accept(*cv);

	if (_cameraVsmHorizonal)
		_cameraVsmHorizonal->accept(*cv);
}
