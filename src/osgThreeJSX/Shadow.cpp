#include <regex>
#include <string>
#include <sstream>
#include <osg/Program>
#include <osgThreeJSX/Shadow>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/Light>

using namespace osgThreeJSX;


ShadowMap::ShadowMap()
{
	_mapType = ShadowMapType_PCFShadowMap;
	_enable = false;
}

void ShadowMap::setup(const osg::ref_ptr<osg::Node>& scene, ShadowMapType mapType /* = ShadowMapType_BasicShadowMap */)
{
	_sceneNode = scene;
	_mapType = mapType;
	_enable = true;
}

void ShadowMap::onCull(RenderState* renderState, osgUtil::CullVisitor* cv)
{
	if (!isEnable())
		return;

	LightList lights = renderState->getAllLight();
	for (LightList::iterator iter = lights.begin(); iter != lights.end(); iter++)
	{
		osg::ref_ptr<Light>& light = *iter;
		if (light->getCastShadow() == false)
			continue;

		osg::ref_ptr<LightShadow>& shadow = light->getShadow();
		if (shadow.valid() == false)
			continue;

		shadow->render(this, light, _sceneNode, cv);
	}
}