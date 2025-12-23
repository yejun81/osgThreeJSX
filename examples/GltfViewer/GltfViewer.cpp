#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osg/Types>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

#include <osgGA/Device>

#include <iostream>
#include <chrono>
#include <osgThreeJSX/AmbientLight>
#include <osgThreeJSX/DirectionalLight>
#include <osgThreeJSX/PointLight>
#include <osgThreeJSX/HemisphereLight>
#include <osgThreeJSX/RectAreaLight>
#include <osgThreeJSX/SpotLight>
#include <osgThreeJSX/ProbeLight>
#include <osgThreeJSX/InstanceGeometry>

#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/Materials>
#include <osgThreeJSX/Animation>
#include <osg/ShapeDrawable>
#include <osg/VertexAttribDivisor>
#include <osg/CullFace>
#include <osg/Depth>
#include <chrono>

inline uint64_t DateNow() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		).count();
}

osg::Vec3 HSLtoRGB(float h, float s, float l) {
	float r, g, b;

	if (s == 0.0f) {
		r = g = b = l; // achromatic
	}
	else {
		auto hue2rgb = [](float p, float q, float t) {
			if (t < 0.0f) t += 1.0f;
			if (t > 1.0f) t -= 1.0f;
			if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
			if (t < 1.0f / 2.0f) return q;
			if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
			return p;
		};

		float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
		float p = 2.0f * l - q;
		r = hue2rgb(p, q, h + 1.0f / 3.0f);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1.0f / 3.0f);
	}

	return osg::Vec3(r, g, b);
}

osg::Vec3 UnsignedToRGB(unsigned int color)
{
	osg::Vec3 ret;
	ret.x() = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
	ret.y() = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
	ret.z() = static_cast<float>(color & 0xFF) / 255.0f;
	return ret;
}

#include <osg/TextureCubeMap>
void cubeTextureAndViewMats(osg::TextureCubeMap* cube_texture, int size)
{
	cube_texture->setTextureSize(size, size);
	cube_texture->setInternalFormat(GL_RGBA16F_ARB);
	cube_texture->setSourceFormat(GL_RGBA);
	cube_texture->setSourceType(GL_FLOAT);
	cube_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
	//cube_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	cube_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	cube_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
}

osg::TextureCubeMap* createCubemapFromImages(const std::vector<osg::ref_ptr<osg::Image>>& images, bool flipVertical /* = false */)
{
	if (images.size() != 6)
		return nullptr;

	osg::TextureCubeMap* texture = new osg::TextureCubeMap;
	cubeTextureAndViewMats(texture, 512);

	for (size_t i = 0; i < images.size(); i++)
	{
		osg::ref_ptr<osg::Image> img = images[i];
		if (img)
		{
			if (flipVertical)
				img->flipVertical();

			texture->setTextureSize(img->s(), img->t());
			texture->setInternalFormat(img->getInternalTextureFormat());
			texture->setSourceFormat(img->getPixelFormat());
			texture->setSourceType(img->getDataType());
			texture->setImage(i, img);
		}
		else
			return nullptr;
	}

	return texture;
}

osg::TextureCubeMap* createCubemapFromImageFiles(const std::vector<std::string>& filenames, bool flipVertical /* = false */)
{
	if (filenames.size() != 6)
		return nullptr;

	std::vector<osg::ref_ptr<osg::Image>> images;
	for (size_t i = 0; i < filenames.size(); i++)
	{
		osg::ref_ptr<osg::Image> img = osgDB::readImageFile(filenames[i]);
		images.push_back(img);
	}

	return createCubemapFromImages(images, flipVertical);
}

osg::Texture2D* createCubemapFromEquirectangular(const std::string& filename, bool flipVertical)
{
	osg::ref_ptr<osgDB::Options> a;
	if (flipVertical) a = new osgDB::Options(std::string("YFLIP"));
	osg::Image* image = osgDB::readImageFile(filename, a.get());
	osg::Texture2D* texture = new osg::Texture2D(image);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setResizeNonPowerOfTwoHint(false);
	return texture;
}


int main(int argc, char** argv)
{
	osgDB::Registry::instance()->addFileExtensionAlias("glb", "gltf");

	osg::ArgumentParser arguments(&argc, argv);

	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	viewer->setUpViewInWindow(150, 150, 1024, 768, 0);

	std::vector<std::string> files;
	files.push_back("/assets/env/skyboxsun25deg/px.jpg");
	files.push_back("/assets/env/skyboxsun25deg/nx.jpg");
	files.push_back("/assets/env/skyboxsun25deg/py.jpg");
	files.push_back("/assets/env/skyboxsun25deg/ny.jpg");
	files.push_back("/assets/env/skyboxsun25deg/pz.jpg");
	files.push_back("/assets/env/skyboxsun25deg/nz.jpg");
	osg::TextureCubeMap* cubeMap = createCubemapFromImageFiles(files, true);

	osg::ref_ptr<osg::Node> gltfNode = osgDB::readNodeFile("/assets/models/1959_porsche_718_rsk.glb");

	osg::ref_ptr<osg::Group> root = new osg::Group();

	root->addChild(gltfNode);
	viewer->setSceneData(root);

	osg::ref_ptr<osgThreeJSX::RenderState> renderState = new osgThreeJSX::RenderState();
	renderState->setOutputEncoding(osgThreeJSX::TextureEncodingType_sRGBEncoding);
	renderState->setToneMapping(osgThreeJSX::ToneMappingType_ACESFilmicToneMapping);
	//renderState->_physicallyCorrectLights = true;
	renderState->setToneMappingExposure(0.8);

	osgThreeJSX::AmbientLight* ambientLight = new osgThreeJSX::AmbientLight(osg::Vec3(1.0,1.0,1.0), 1.0);
	renderState->addLight(ambientLight);

	osg::Vec3 direction = osg::Vec3(1.0, 1.0, 1.0);
	direction.normalize();
	osgThreeJSX::DirectionalLight* directionLight = new osgThreeJSX::DirectionalLight(direction, osg::Vec3(1.0, 1.0, 1.0), 1.0);
	renderState->addLight(directionLight);

	auto camera = viewer->getCamera();
	renderState->setupCamera(camera);

	if (cubeMap)
	{
		renderState->setBackground(cubeMap);
	}

	// set up the camera manipulators.
	{
		osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

		keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
		keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
		keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
		keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
		keyswitchManipulator->addMatrixManipulator('5', "Orbit", new osgGA::OrbitManipulator());
		keyswitchManipulator->addMatrixManipulator('6', "FirstPerson", new osgGA::FirstPersonManipulator());
		keyswitchManipulator->addMatrixManipulator('7', "Spherical", new osgGA::SphericalManipulator());

		std::string pathfile;
		double animationSpeed = 1.0;
		while (arguments.read("--speed", animationSpeed)) {}
		char keyForAnimationPath = '8';
		while (arguments.read("-p", pathfile))
		{
			osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
			if (apm && !apm->getAnimationPath()->empty())
			{
				apm->setTimeScale(animationSpeed);

				unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
				keyswitchManipulator->addMatrixManipulator(keyForAnimationPath, "Path", apm);
				keyswitchManipulator->selectMatrixManipulator(num);
				++keyForAnimationPath;
			}
		}

		viewer->setCameraManipulator(keyswitchManipulator.get());
	}

	osgViewer::Viewer::Windows windows;
	viewer->getWindows(windows);
	for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
	{
		(*itr)->getState()->setUseModelViewAndProjectionUniforms(true);
		(*itr)->getState()->setUseVertexAttributeAliasing(true);
	}

	viewer->run();
	return 0;
}
