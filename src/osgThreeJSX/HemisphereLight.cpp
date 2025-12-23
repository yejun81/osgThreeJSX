
#include <osgThreeJSX/Light>
#include <osgThreeJSX/HemisphereLight>
#include <osgThreeJSX/RenderState>
#include <osg/Texture2D>
#include <osg/Geometry>

using namespace osgThreeJSX;


HemisphereLight::HemisphereLight()
{
	this->_intensity = 1.0f;
	this->_castShadow = false;
}

HemisphereLight::HemisphereLight(const osg::Vec3& direction, const osg::Vec3& skyColor, const osg::Vec3& groundColor, float intensity) :
	_direction(direction), _skyColor(skyColor), _groundColor(groundColor), _intensity(intensity)
{
}

HemisphereLight::~HemisphereLight()
{

}
