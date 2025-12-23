

#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/AmbientLight>
#include <osgThreeJSX/DirectionalLight>
#include <osgThreeJSX/PointLight>
#include <osgThreeJSX/HemisphereLight>
#include <osgThreeJSX/RectAreaLight>
#include <osgThreeJSX/SpotLight>
#include <osgThreeJSX/ProbeLight>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/Materials>
#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/TextureCubeMap>
using namespace osgThreeJSX;

//////////////////////////////////////////////////////////////////////////
class RenderCameraCullCallback : public osg::NodeCallback
{
public:
	RenderCameraCullCallback(RenderState* renderState):_renderState(renderState) {}

	virtual ~RenderCameraCullCallback() { }

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		if (cv)
		{
			_renderState->onCull(cv);
		}
		traverse(node, nv);
	}
private:
	osg::ref_ptr<RenderState> _renderState;
};

//////////////////////////////////////////////////////////////////////////
Capabilities::Capabilities()
{
#ifdef __EMSCRIPTEN__
	glslversion = "300 es";//300 es
#else
	glslversion = "330";//300 es
#endif	

	precision = "highp";
	supportsVertexTextures = true;
}


//////////////////////////////////////////////////////////////////////////
RenderState::RenderState()
{
	setOutputEncoding(TextureEncodingType_LinearEncoding);
	setGammaFactor(2.0f);

	setToneMapping(ToneMappingType_NoToneMapping);
	setToneMappingExposure(1.0f);
	setToneMappingWhitePoint(1.0f);

	setLogarithmicDepthBuffer(false);
	setPhysicallyCorrectLights(false);

	_bgEnv = nullptr;
}

RenderState* RenderState::FromCamera(osg::Camera* camera)
{
	RenderState* renderState = dynamic_cast<RenderState*>(camera->getUserData());
	return renderState;
}

void RenderState::setupCamera(osg::Camera* camera, Light* light)
{
	_camera = camera;
	setShadowLight(light);

	RenderState* rs = RenderState::FromCamera(camera);
	if (rs && rs->_cameraCallback)
	{
		camera->removeCullCallback(rs->_cameraCallback);
	}

	_camera->setUserData(this);
	_cameraCallback = new RenderCameraCullCallback(this);
	camera->addCullCallback(_cameraCallback);
}

void RenderState::setupShadow(osg::Node* root, ShadowMapType mapType)
{
	_shadowMap = new ShadowMap();
	_shadowMap->setup(root, mapType);
}

void RenderState::onCull(osgUtil::CullVisitor* cv)
{
	auto stateset = _camera->getOrCreateStateSet();
	stateset->getOrCreateUniform("osg_ViewMatrix", osg::Uniform::FLOAT_MAT4)->set(_camera->getViewMatrix());
	stateset->getOrCreateUniform("osg_ViewMatrixInverse", osg::Uniform::FLOAT_MAT4)->set(_camera->getInverseViewMatrix());

	if (getShadowLight())
		return;

	if (_shadowMap)
		_shadowMap->onCull(this, cv);

	if (_bgNode)
	{
		osg::Vec3 eye, center, up;
		_camera->getViewMatrixAsLookAt(eye, center, up);

		osg::Matrix mat; mat.makeTranslate(eye);
		_bgNode->setMatrix(mat);

		CullSettingAutoRecover ar(cv, osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
		_bgNode->accept(*cv);
	}

	//lights
	//ambient
	osg::Vec3 ambient; float intensity = 0.0f;
	LightList* list = getLightsOfType(LightType_Ambient);
	for (int i = 0; list && i < list->size(); i++)
	{
		AmbientLight* light = dynamic_cast<AmbientLight*>((*list)[i].get());
		ambient += light->getColor() * light->getIntensity();
		intensity += light->getIntensity();
	}
	stateset->getOrCreateUniform("ambientLightColor", osg::Uniform::FLOAT_VEC3)->set(ambient);
	stateset->getOrCreateUniform("ambientLightIntensity", osg::Uniform::FLOAT)->set(intensity);

	///TODO:ugly, -1 reserve for geometry instance data texture
	int textureUnit = cv->getState()->getMaxTextureUnits() - 2;
	//direction
	updateDirectionLight(cv, textureUnit);

	//point
	updatePointLight(cv, textureUnit);

	//spot
	updateSpotLight(cv, textureUnit);

	//hemisphere
	updateHemisphereLight(cv, textureUnit);

	//RectArea
	updateRectAreaLight(cv, textureUnit);

	//Probe
	updateProbeLight(cv, textureUnit);

	//fog
	if (getFog())
	{
		Fog* fog = dynamic_cast<Fog*>(getFog().get());
		if (fog)
		{
			stateset->getOrCreateUniform("fogColor", osg::Uniform::FLOAT_VEC3)->set(fog->_color);
			stateset->getOrCreateUniform("fogNear", osg::Uniform::FLOAT)->set(fog->_near);
			stateset->getOrCreateUniform("fogFar", osg::Uniform::FLOAT)->set(fog->_far);
		}

		FogExp2* fogExp2 = dynamic_cast<FogExp2*>(getFog().get());
		if (fogExp2)
		{
			stateset->getOrCreateUniform("fogColor", osg::Uniform::FLOAT_VEC3)->set(fogExp2->_color);
			stateset->getOrCreateUniform("fogDensity", osg::Uniform::FLOAT)->set(fogExp2->_density);
		}
	}

	//toneMapping
	stateset->getOrCreateUniform("toneMappingExposure", osg::Uniform::FLOAT)->set(getToneMappingExposure());
	stateset->getOrCreateUniform("toneMappingWhitePoint", osg::Uniform::FLOAT)->set(getToneMappingWhitePoint());

	//
	if (getLogarithmicDepthBuffer())
	{
		double fovy, aspectRatio, zNear, zFar;
		_camera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
		float v = log(2.0);
		v = 2.0 / (log(zFar + 1.0) / log(2.0));
		stateset->getOrCreateUniform("logDepthBufFC", osg::Uniform::FLOAT)->set(v);
	}


}

void RenderState::useTextureUnit(int& textureUnit)
{
	textureUnit--;
}

void RenderState::updateDirectionLight(osgUtil::CullVisitor* cv, int& textureUnit)
{
	auto stateset = _camera->getOrCreateStateSet();
	osg::Matrix viewMat = _camera->getViewMatrix();

	LightList* list = getLightsOfType(LightType_Direction);	
	std::vector<osg::ref_ptr<LightShadow> > shadowLightList;

	for (int i = 0; list && i < list->size(); i++)
	{
		DirectionalLight* light = dynamic_cast<DirectionalLight*>((*list)[i].get());

		char szUniformName[64] = { 0 };
		sprintf(szUniformName, "directionalLights[%d].direction", i);
		if (light->isFlow())
		{
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getDirection());
		}
		else
		{
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(osg::Matrix::transform3x3(osg::Vec3() - light->getDirection(), viewMat));
		}

		sprintf(szUniformName, "directionalLights[%d].color", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getColor() * light->getIntensity());

		if (light->getCastShadow())
		{
			osg::ref_ptr<LightShadow> shadow = light->getShadow();
			shadowLightList.push_back(shadow);
		}
	}

	if (shadowLightList.size() > 0)
	{
		osg::ref_ptr<osg::Uniform> shadowMapUniform = stateset->getOrCreateUniform("directionalShadowMap", osg::Uniform::INT, shadowLightList.size());
		osg::ref_ptr<osg::Uniform> shadowMatrixUniform = stateset->getOrCreateUniform("directionalShadowMatrix", osg::Uniform::FLOAT_MAT4, shadowLightList.size());

		for (size_t i = 0; i < shadowLightList.size(); i++)
		{
			osg::ref_ptr<LightShadow> shadow = shadowLightList[i];

			char szUniformName[64] = { 0 };
			sprintf(szUniformName, "directionalLightShadows[%d].shadowBias", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadow->getBias());
			sprintf(szUniformName, "directionalLightShadows[%d].shadowRadius", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadow->getRadius());
			sprintf(szUniformName, "directionalLightShadows[%d].shadowMapSize", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC2)->set(shadow->getMapSize());

			if (shadow->getMap())
			{
				stateset->setTextureAttribute(textureUnit, shadow->getMap());
				shadowMapUniform->setElement(i, textureUnit);

				shadowMatrixUniform->setElement(i, shadow->getMatrix());
				useTextureUnit(textureUnit);
			}

		}
	}

}

void RenderState::updatePointLight(osgUtil::CullVisitor* cv, int& textureUnit)
{
	auto stateset = _camera->getOrCreateStateSet();
	osg::Matrix viewMat = _camera->getViewMatrix();

	LightList* list = getLightsOfType(LightType_Point);
	
	struct PointLightShadowData
	{
		PointLight* light;
		LightShadow* shadow;
	};
	std::vector<PointLightShadowData> shadowLightList;

	for (int i = 0; list && i < list->size(); i++)
	{
		PointLight* light = dynamic_cast<PointLight*>((*list)[i].get());

		char szUniformName[64] = { 0 };

		osg::Vec3 viewPosition = light->getPosition() * viewMat;

		sprintf(szUniformName, "pointLights[%d].position", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(viewPosition);

		sprintf(szUniformName, "pointLights[%d].color", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getColor() * light->getIntensity());

		sprintf(szUniformName, "pointLights[%d].distance", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(light->getDistance());

		sprintf(szUniformName, "pointLights[%d].decay", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(light->getDecay());

		if (light->getCastShadow())
		{
			osg::ref_ptr<LightShadow> shadow = light->getShadow();
			PointLightShadowData data;
			data.light = light;
			data.shadow = shadow;
			shadowLightList.push_back(data);
		}
	}

	if (shadowLightList.size() > 0)
	{
		osg::ref_ptr<osg::Uniform> shadowMapUniform = stateset->getOrCreateUniform("pointShadowMap", osg::Uniform::INT, shadowLightList.size());
		osg::ref_ptr<osg::Uniform> shadowMatrixUniform = stateset->getOrCreateUniform("pointShadowMatrix", osg::Uniform::FLOAT_MAT4, shadowLightList.size());

		for (size_t i = 0; i < shadowLightList.size(); i++)
		{
			LightShadow* shadow = shadowLightList[i].shadow;

			char szUniformName[64] = { 0 };
			sprintf(szUniformName, "pointLightShadows[%d].shadowBias", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadow->getBias());
			sprintf(szUniformName, "pointLightShadows[%d].shadowRadius", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadow->getRadius());
			sprintf(szUniformName, "pointLightShadows[%d].shadowMapSize", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC2)->set(shadow->getMapSize());
			sprintf(szUniformName, "pointLightShadows[%d].shadowCameraNear", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(0.1f);
			sprintf(szUniformName, "pointLightShadows[%d].shadowCameraFar", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadowLightList[i].light->getDistance());

			if (shadow->getMap())
			{
				stateset->setTextureAttribute(textureUnit, shadow->getMap());
				shadowMapUniform->setElement(i, textureUnit);

				shadowMatrixUniform->setElement(i, shadow->getMatrix());
				useTextureUnit(textureUnit);
			}
		}
	}
}

void RenderState::updateSpotLight(osgUtil::CullVisitor* cv, int& textureUnit)
{
	auto stateset = _camera->getOrCreateStateSet();
	osg::Matrix viewMat = _camera->getViewMatrix();

	LightList* list = getLightsOfType(LightType_Spot);

	std::vector<osg::ref_ptr<LightShadow> > shadowLightList;

	for (int i = 0; list && i < list->size(); i++)
	{
		SpotLight* light = dynamic_cast<SpotLight*>((*list)[i].get());

		char szUniformName[64] = { 0 };

		osg::Vec3 viewPosition = light->getPosition() * viewMat;
		sprintf(szUniformName, "spotLights[%d].position", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(viewPosition);

		sprintf(szUniformName, "spotLights[%d].direction", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(osg::Matrix::transform3x3(osg::Vec3() - light->getDirection(), viewMat));

		sprintf(szUniformName, "spotLights[%d].color", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getColor() * light->getIntensity());

		sprintf(szUniformName, "spotLights[%d].distance", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(light->getDistance());

		sprintf(szUniformName, "spotLights[%d].decay", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(light->getDecay());

		sprintf(szUniformName, "spotLights[%d].coneCos", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(cos(light->getAngle()));

		sprintf(szUniformName, "spotLights[%d].penumbraCos", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(cos(light->getAngle() * (1.0f - light->getPenumbra())));

		if (light->getCastShadow())
		{
			osg::ref_ptr<LightShadow> shadow = light->getShadow();
			shadowLightList.push_back(shadow);
		}
	}

	if (shadowLightList.size() > 0)
	{
		osg::ref_ptr<osg::Uniform> shadowMapUniform = stateset->getOrCreateUniform("spotShadowMap", osg::Uniform::INT, shadowLightList.size());
		osg::ref_ptr<osg::Uniform> shadowMatrixUniform = stateset->getOrCreateUniform("spotShadowMatrix", osg::Uniform::FLOAT_MAT4, shadowLightList.size());

		for (size_t i = 0; i < shadowLightList.size(); i++)
		{
			osg::ref_ptr<LightShadow> shadow = shadowLightList[i];

			char szUniformName[64] = { 0 };
			sprintf(szUniformName, "spotLightShadows[%d].shadowBias", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadow->getBias());
			sprintf(szUniformName, "spotLightShadows[%d].shadowRadius", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT)->set(shadow->getRadius());
			sprintf(szUniformName, "spotLightShadows[%d].shadowMapSize", i);
			stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC2)->set(shadow->getMapSize());

			if (shadow->getMap())
			{
				stateset->setTextureAttribute(textureUnit, shadow->getMap());
				shadowMapUniform->setElement(i, textureUnit);

				shadowMatrixUniform->setElement(i, shadow->getMatrix());
				useTextureUnit(textureUnit);
			}
		}
	}
}

void RenderState::updateHemisphereLight(osgUtil::CullVisitor* cv, int& textureUnit)
{
	auto stateset = _camera->getOrCreateStateSet();
	osg::Matrix viewMat = _camera->getViewMatrix();

	LightList* list = getLightsOfType(LightType_Hemisphere);
	
	for (int i = 0; list && i < list->size(); i++)
	{
		HemisphereLight* light = dynamic_cast<HemisphereLight*>((*list)[i].get());

		char szUniformName[64] = { 0 };

		sprintf(szUniformName, "hemisphereLights[%d].direction", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(osg::Matrix::transform3x3(osg::Vec3()-light->getDirection(), viewMat));

		sprintf(szUniformName, "hemisphereLights[%d].skyColor", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getSkyColor() * light->getIntensity());

		sprintf(szUniformName, "hemisphereLights[%d].groundColor", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getGroundColor() * light->getIntensity());
	}
}

void RenderState::updateRectAreaLight(osgUtil::CullVisitor* cv, int& textureUnit)
{
	auto stateset = _camera->getOrCreateStateSet();
	osg::Matrix viewMat = _camera->getViewMatrix();

	LightList* list = getLightsOfType(LightType_RectArea);

	if (list && list->size())
	{
		stateset->getOrCreateUniform("ltc_1", osg::Uniform::INT)->set(textureUnit);
		stateset->setTextureAttribute(textureUnit, RectAreaLight::getOrCreateLTC1Texture());
		useTextureUnit(textureUnit);

		stateset->getOrCreateUniform("ltc_2", osg::Uniform::INT)->set(textureUnit);
		stateset->setTextureAttribute(textureUnit, RectAreaLight::getOrCreateLTC2Texture());
		useTextureUnit(textureUnit);
	}

	for (int i = 0; list && i < list->size(); i++)
	{
		RectAreaLight* light = dynamic_cast<RectAreaLight*>((*list)[i].get());

		char szUniformName[64] = { 0 };

		osg::Vec3 viewPosition = light->getPosition() * viewMat;

		sprintf(szUniformName, "rectAreaLights[%d].position", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(viewPosition);

		sprintf(szUniformName, "rectAreaLights[%d].color", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(light->getColor() * light->getIntensity());

		osg::Vec3 direction = light->getDirection();
		direction.normalize();		

		osg::Matrix mat;
		mat.makeLookAt(light->getPosition(), light->getPosition() + direction * 10.0, osg::Vec3(0.0f, 0.0f, 1.0f));
		osg::Matrix invertMat = mat.inverse(mat);
		mat = invertMat * viewMat;

		sprintf(szUniformName, "rectAreaLights[%d].halfWidth", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(osg::Matrix::transform3x3(osg::Vec3(light->getWidth() / 2.0, 0, 0), mat));

		sprintf(szUniformName, "rectAreaLights[%d].halfHeight", i);
		stateset->getOrCreateUniform(szUniformName, osg::Uniform::FLOAT_VEC3)->set(osg::Matrix::transform3x3(osg::Vec3(0, light->getHeight() / 2.0, 0), mat));
	}
}


void RenderState::updateProbeLight(osgUtil::CullVisitor* cv, int& textureUnit)
{
	auto stateset = _camera->getOrCreateStateSet();

	LightList* list = getLightsOfType(LightType_Probe);
	if (list && list->size())
	{

		osg::Vec3 probe[PROBE_SH_NUM];

		for (int i = 0; list && i < list->size(); i++)
		{
			ProbeLight* light = dynamic_cast<ProbeLight*>((*list)[i].get());

			for (int j = 0; j < PROBE_SH_NUM; j++)
			{
				probe[j] += light->getCoefficient(j) * light->getIntensity();
			}
			
		}

		osg::Uniform* probeUniform = stateset->getUniform("lightProbe");
		if (!probeUniform)
		{
			probeUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "lightProbe", PROBE_SH_NUM);
			stateset->addUniform(probeUniform);
		}

		for (int j = 0; j < PROBE_SH_NUM; j++)
		{
			probeUniform->setElement(j, probe[j]);
		}
	}
}

void RenderState::addLight(Light* light)
{
	LightMap::iterator iter = _lightMap.find(light->getType());
	if (iter == _lightMap.end())
	{
		//iter = _lightMap.insert(std::make_pair(light->getType(), list));
		iter = _lightMap.insert(std::make_pair(light->getType(), LightList())).first;
	}
	iter->second.push_back(light);

	_lightList.push_back(light);
}

LightList* RenderState::getLightsOfType(LightType type)
{
	LightMap::iterator iter = _lightMap.find(type);
	if (iter != _lightMap.end())
	{
		return &iter->second;
	}
	return NULL;
}

int RenderState::getLightNumOfType(LightType type)
{
	LightList* list = getLightsOfType(type);
	if (list)
	{
		return list->size();
	}
	return 0;
}

int RenderState::getShadowNumOfType(LightType type)
{
	int num = 0;
	LightList* list = getLightsOfType(type);

	if (list)
	{
		for (int i = 0; list && i < list->size(); i++)
		{
			Light* light = (*list)[i].get();
			if (light->getCastShadow())
			{
				num++;
			}
		}
	}
	return num;
}

MaterialDataEnv* RenderState::getBackgroudEnv()
{
	return _bgEnv;
}

void RenderState::setBackground(osg::Texture* bg)
{
	osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable();
	drawable->setShape(new osg::Box(osg::Vec3(0.0, 0.0, 0.0), 1, 1, 1));
	drawable->setUseVertexBufferObjects(true);
	drawable->setCullingActive(false);

	osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
	geode->addChild(drawable);
	geode->setCullingActive(false);


	osg::ref_ptr<osgThreeJSX::Material> material;
	if (dynamic_cast<osg::TextureCubeMap*>(bg))
	{
		osg::ref_ptr<osgThreeJSX::MaterialCube> cubeMapMaterial = new osgThreeJSX::MaterialCube();
		cubeMapMaterial->setSide(MaterialSideType_BackSide);
		cubeMapMaterial->_env.envMap = reinterpret_cast<osg::Texture*>(bg);
		cubeMapMaterial->_env.envMapEncoding = TextureEncodingType_RGBDEncoding;
		cubeMapMaterial->_env.combine = EnvMapCombineType_No;
		material = cubeMapMaterial;
		_bgEnv = &cubeMapMaterial->_env;
	}
	else
	{
		osg::ref_ptr<osgThreeJSX::MaterialEquirect> equirectMapMaterial = new osgThreeJSX::MaterialEquirect();
		equirectMapMaterial->setSide(MaterialSideType_BackSide);
		equirectMapMaterial->_env.envMap = reinterpret_cast<osg::Texture*>(bg);
		equirectMapMaterial->_env.envMapEncoding = TextureEncodingType_RGBDEncoding;
		equirectMapMaterial->_env.envMapMode = EnvMapModeType_EquirectangularReflectionMapping;
		equirectMapMaterial->_env.combine = EnvMapCombineType_No;
		material = equirectMapMaterial;
		_bgEnv = &equirectMapMaterial->_env;
	}	

	geode->setMaterial(material);

	osg::MatrixTransform* transform = new osg::MatrixTransform();
	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask(false);
	transform->getOrCreateStateSet()->setAttributeAndModes(depth, osg::StateAttribute::ON);
	transform->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	transform->addChild(geode);
	transform->setCullingActive(false);

	_bgNode = transform;
}

