
#include <osgThreeJSX/Light>
#include <osgThreeJSX/ProbeLight>
#include <osgThreeJSX/RenderState>
#include <osg/Texture2D>
#include <osg/Geometry>


using namespace osgThreeJSX;

ProbeLight::ProbeLight()
{
	_intensity = 1.0;
}

void GetSphericalHarmonicsBasis(const osg::Vec3d& normal, double* shBasis)
{
	float x = normal.x(), y = normal.y(), z = normal.z();
	// band 0
	shBasis[0] = 0.282095;

	// band 1
	shBasis[1] = 0.488603 * y;
	shBasis[2] = 0.488603 * z;
	shBasis[3] = 0.488603 * x;

	// band 2
	shBasis[4] = 1.092548 * x * y;
	shBasis[5] = 1.092548 * y * z;
	shBasis[6] = 0.315392 * (3 * z * z - 1);
	shBasis[7] = 1.092548 * x * z;
	shBasis[8] = 0.546274 * (x * x - y * y);
}

double SRGBToLinear(double c)
{
	return (c < 0.04045) ? c * 0.0773993808 : pow(c * 0.9478672986 + 0.0521327014, 2.4);
}

void convertColorToLinear(osg::Vec4d& color)
{
	color.r() = SRGBToLinear(color.r());
	color.g() = SRGBToLinear(color.g());
	color.b() = SRGBToLinear(color.b());
}

ProbeLight::ProbeLight(const osg::ref_ptr<osg::TextureCubeMap>& cubeMap, float intensity) :_intensity(intensity)
{
	double shBasis[9] = { 0 };
	double norm = 0;
	double totalWeight = 0;

	for (int i = 0; i < 6; i++)
	{
		const osg::Image* image = cubeMap->getImage(i);
		if (!image)
			continue;

		int pixelIndex = 0;
		double pixelSize = 2.0 / image->s();
		osg::Vec3 coord;
		float lengthSq = 0, weight = 0;

		for (int j = 0; j < image->s(); j++)
			for (int k = 0; k < image->t(); k++)
			{
				osg::Vec4d color = image->getColor(j, k);
				convertColorToLinear(color);

				double col = -1 + (pixelIndex % image->s() + 0.5) * pixelSize;
				double row = 1 - (pixelIndex / image->s() + 0.5) * pixelSize;

				switch (i)
				{
				case 0: coord = osg::Vec3d(-1, row, -col); break;
				case 1: coord = osg::Vec3d(1, row, col); break;
				case 2: coord = osg::Vec3d(-col, 1, -row); break;
				case 3: coord = osg::Vec3d(-col, -1, row); break;
				case 4: coord = osg::Vec3d(-col, row, 1); break;
				case 5: coord = osg::Vec3d(col, row, -1); break;
				default:
					break;
				}

				lengthSq = coord.length2();
				weight = 4.0 / (coord.length() * lengthSq);
				totalWeight += weight;

				osg::Vec3d dir(coord);
				dir.normalize();
				GetSphericalHarmonicsBasis(dir, shBasis);

				// accummuulate
				for (int idx = 0; idx < 9; idx++) {

					_shCoefficients[idx].x() += shBasis[idx] * color.r() * weight;
					_shCoefficients[idx].y() += shBasis[idx] * color.g() * weight;
					_shCoefficients[idx].z() += shBasis[idx] * color.b() * weight;

				}

				pixelIndex++;
			}
	}

	// normalize
	norm = (4 * osg::PI) / totalWeight;

	for (int idx = 0; idx < 9; idx++) {

		_shCoefficients[idx].x() *= norm;
		_shCoefficients[idx].y() *= norm;
		_shCoefficients[idx].z() *= norm;

	}
}
ProbeLight::~ProbeLight()
{

}

osg::Vec3 ProbeLight::getCoefficient(unsigned int idx)
{
	osg::Vec3 ret;
	if (idx < sizeof(_shCoefficients) / sizeof(_shCoefficients[0]))
	{
		ret = _shCoefficients[idx];
	}
	return ret;
}