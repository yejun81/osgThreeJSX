
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



int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	viewer->setUpViewInWindow(150, 150, 1024, 768, 0);

	osg::ref_ptr<osg::Group> root = new osg::Group();

	//osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(new osg::Sphere());
	//shadow caster
	osg::TessellationHints* tessellationHits = new osg::TessellationHints();
	tessellationHits->setTargetNumFaces(2880);

	for (int i = 0; i < 4; i++)
	{
		osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable();
		drawable->setShape(new osg::Cylinder(osg::Vec3(), 0.75, 7));
		drawable->setUseVertexBufferObjects(true);

		osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
		geode->addChild(drawable);

		osg::MatrixTransform* mt = new osg::MatrixTransform();
		mt->addChild(geode);
		root->addChild(mt);

		//osg::ref_ptr<osgThreeJSX::MaterialLambert> material = new osgThreeJSX::MaterialLambert();
		osg::ref_ptr<osgThreeJSX::MaterialPhong> material = new osgThreeJSX::MaterialPhong();
		material->setVertexColors(false);
		material->setCastShadow(true);
		material->setReceiveShadow(false);
		material->_common.color = UnsignedToRGB(0x999999);
		material->_shininess.shininess = 0;
		material->_specular.specular = UnsignedToRGB(0x222222);
		geode->setMaterial(material);
	}

	{
		osg::Matrix mat;
		mat.makeTranslate(osg::Vec3(10, 10, 3.5));
		dynamic_cast<osg::MatrixTransform*>(root->getChild(0))->setMatrix(mat);

		mat.makeTranslate(osg::Vec3(10, -10, 3.5));
		dynamic_cast<osg::MatrixTransform*>(root->getChild(1))->setMatrix(mat);

		mat.makeTranslate(osg::Vec3(-10, -10, 3.5));
		dynamic_cast<osg::MatrixTransform*>(root->getChild(2))->setMatrix(mat);

		mat.makeTranslate(osg::Vec3(-10, 10, 3.5));
		dynamic_cast<osg::MatrixTransform*>(root->getChild(3))->setMatrix(mat);
	}

	{
		osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable();
		drawable->setShape(new osg::Sphere(osg::Vec3(0, 0.0, 3.0), 2.0));
		drawable->setUseVertexBufferObjects(true);

		osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
		geode->addChild(drawable);

		osg::MatrixTransform* mt = new osg::MatrixTransform();
		mt->addChild(geode);
		root->addChild(mt);

		//osg::ref_ptr<osgThreeJSX::MaterialLambert> material = new osgThreeJSX::MaterialLambert();
		osg::ref_ptr<osgThreeJSX::MaterialPhong> material = new osgThreeJSX::MaterialPhong();
		material->setVertexColors(false);
		material->setCastShadow(true);
		material->setReceiveShadow(false);
		material->_common.color = UnsignedToRGB(0x999999);
		material->_shininess.shininess = 0;
		material->_specular.specular = UnsignedToRGB(0x222222);
		geode->setMaterial(material);
	}

	//shadow receiver

	{
		osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable();
		drawable->setShape(new osg::Box(osg::Vec3(), 60.0, 60.0, 2.0));
		drawable->setUseVertexBufferObjects(true);

		osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
		geode->addChild(drawable);
		root->addChild(geode);

		osg::ref_ptr<osgThreeJSX::MaterialPhong> material = new osgThreeJSX::MaterialPhong();
		material->setVertexColors(false);
		material->setReceiveShadow(true);
		material->setCastShadow(false);
		material->_common.color = UnsignedToRGB(0x999999);
		material->_shininess.shininess = 0;
		material->_specular.specular = UnsignedToRGB(0x111111);
		geode->setMaterial(material);
	}

	viewer->setSceneData(root);

	osg::ref_ptr<osgThreeJSX::RenderState> renderState = new osgThreeJSX::RenderState();
	//renderState->setupShadow(root, osgThreeJSX::ShadowMapType_BasicShadowMap);
	renderState->setupShadow(root, osgThreeJSX::ShadowMapType_VSMShadowMap);
	renderState->setOutputEncoding(osgThreeJSX::TextureEncodingType_sRGBEncoding);
	renderState->setToneMapping(osgThreeJSX::ToneMappingType_Uncharted2ToneMapping);
	renderState->setToneMappingExposure(0.75);


	//osgThreeJSX::AmbientLight* ambientLight = new osgThreeJSX::AmbientLight(osg::Vec3(0x22 / 255.0, 0x22 / 255.0, 0x22 / 255.0), 1.0);
	//renderState->addLight(ambientLight);

	osg::Vec3 direction = osg::Vec3() - osg::Vec3(3.0, 17.0, 12.0);
	direction.normalize();
	osgThreeJSX::DirectionalLight* directionLight = new osgThreeJSX::DirectionalLight(direction, osg::Vec3(1.0, 1.0, 1.0), 1.0);
	directionLight->setCastShadow(true);
	directionLight->getShadow()->setBias(-0.0005);
	directionLight->getShadow()->getMapSize() = osg::Vec2(512, 512);
	directionLight->getShadow()->setRadius(4);
	//renderState->addLight(directionLight);


	osg::Vec3 lightPosition = osg::Vec3(8, 5, 10);
	osg::Vec3 lightDirection = osg::Vec3() - lightPosition;
	lightDirection.normalize();
	osgThreeJSX::SpotLight* spotLight = new osgThreeJSX::SpotLight(lightPosition, lightDirection,
		osg::Vec3(1.0, 1.0, 1.0), 1.0, 200, osg::PI / 5.0, 0.3, 1);
	spotLight->setCastShadow(true);
	spotLight->getShadow()->setBias(-0.002);
	spotLight->getShadow()->getMapSize() = osg::Vec2(256, 256);
	spotLight->getShadow()->setRadius(4);
	renderState->addLight(spotLight);

	auto camera = viewer->getCamera();
	renderState->setupCamera(camera);

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

	while (!viewer->done())
	{
		osg::Matrix mat = camera->getViewMatrix();
		if (false)
		{
			viewer->getCameraManipulator()->setByMatrix(mat);
		}
		viewer->frame();
	}

	return 0;
}
