# osgThreeJSX

### 1. Introduction

Implementation of most three.js(v0.117.1) Materials, Lights, Shadows, Animations in OpenSceneGraph

### 2. Main Features

#### 2.1 Materials

*   Basic
*   Lambert
*   Phong
*   Standard
*   Physical
*   RawShader

#### 2.2 Lights

*   Ambient
*   Point
*   Directional
*   Spot
*   Hemisphere
*   RectArea
*   Probe

#### 2.3 Shadows

##### support light type:

*   PointLight
*   DirectionalLight
*   SpotLight

##### support shadow type:

*   Basic
*   PCF
*   PCFSoft
*   VSM

#### 2.4 Animation

*   Morph
*   Skin

#### 2.5 Instance

geometry instance draw and manager

### 3. Dependencies

1.  currently depends on OpenSceneGraph 3.7.0, maybe work on previous versions，need to confirm

### 4. How to use

*   Material

<!---->

    	osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable();
    	drawable->setShape(new osg::Box(osg::Vec3(0.0, 0.0, 0.0), 5, 5, 2));
    	drawable->setUseVertexBufferObjects(true);

    	osg::ref_ptr<osgThreeJSX::MaterialBaseNode<osg::Geode>> geode = new osgThreeJSX::MaterialBaseNode<osg::Geode>();
    	geode->addChild(drawable);
    	
    	osg::ref_ptr<osgThreeJSX::MaterialStandard> material = new osgThreeJSX::MaterialStandard();
    	material->_vertexColors = false;
    	material->_common.color = osg::Vec3(0.625, 0, 0);
    	material->_roughness.roughness = 0;
    	material->_metalness.metalness = 0;
    	geode->setMaterial(material);

*   Lights & Shadows

<!---->

    	osg::ref_ptr<osgThreeJSX::RenderState> renderState = new osgThreeJSX::RenderState();
    	// renderState->setupShadow(root, osgThreeJSX::ShadowMapType_BasicShadowMap);
    	renderState->setupShadow(root, osgThreeJSX::ShadowMapType_VSMShadowMap);
    	renderState->_outputEncoding = osgThreeJSX::TextureEncodingType_sRGBEncoding;
    	renderState->_toneMapping = osgThreeJSX::ToneMappingType_Uncharted2ToneMapping;
    	renderState->_toneMappingExposure = 0.75;

    	osgThreeJSX::AmbientLight* ambientLight = new osgThreeJSX::AmbientLight(osg::Vec3(1.0, 1.0, 1.0), 1.0);
    	renderState->addLight(ambientLight);

    	osg::Vec3 directionalDirection = osg::Vec3() - osg::Vec3(3.0, 17.0, 12.0);
    	directionalDirection.normalize();
    	osgThreeJSX::DirectionalLight *directionLight = new osgThreeJSX::DirectionalLight(directionalDirection, osg::Vec3(1.0, 1.0, 1.0), 1.0);
    	directionLight->setCastShadow(true);
    	directionLight->getShadow()->setBias(-0.0005);
    	directionLight->getShadow()->getMapSize() = osg::Vec2(512, 512);
    	directionLight->getShadow()->setRadius(4);
    	renderState->addLight(directionLight);

    	osg::Vec3 spotLightPosition = osg::Vec3(8, 5, 10);
    	osg::Vec3 spotLightDirection = osg::Vec3() - spotLightPosition;
    	spotLightDirection.normalize();
    	osgThreeJSX::SpotLight *spotLight = new osgThreeJSX::SpotLight(spotLightPosition, spotLightDirection,
    																   osg::Vec3(1.0, 1.0, 1.0), 1.0, 200, osg::PI / 5.0, 0.3, 1);
    	spotLight->setCastShadow(true);
    	spotLight->getShadow()->setBias(-0.002);
    	spotLight->getShadow()->getMapSize() = osg::Vec2(256, 256);
    	spotLight->getShadow()->setRadius(4);
    	renderState->addLight(spotLight);

    	auto camera = viewer->getCamera();
    	renderState->setupCamera(camera);

*   Instanced geometry

<!---->

    	osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable();
    	drawable->setShape(new osg::Sphere(osg::Vec3(0, 0, 0), 10.0));

    	osgThreeJSX::MaterialBaseNode<osgThreeJSX::InstanceGeometry>* instanceGeometry = new osgThreeJSX::MaterialBaseNode<osgThreeJSX::InstanceGeometry>;
    	instanceGeometry->setGeometry(drawable);

    	osg::Matrix mat;
    	mat.makeTranslate(osg::Vec3(1.0, 2.0, 3.0));
    	instanceGeometry->addInstance(mat);

    	osg::ref_ptr<osgThreeJSX::MaterialLambert> material = new osgThreeJSX::MaterialLambert();
    	material->setVertexColors(false);
    	material->setInstancing(true);
    	instanceGeometry->setMaterial(material);

### 5. Build

<!---->

    git clone
    cd osgThreeJSX
    mkdir build
    cd build
    cmake .. -DOSG_DIR=/path/to/OSG
    cmake --build .

### 6. Examples

- cmake with BUILD_OSGTHREEJSX_EXAMPLES ON(default)
- include the examples folder path in OSG_FILE_PATH system variables

This project's examples include 3D model assets from third-party sources. These assets may be protected by their respective copyright holders. Please review carefully before use.This project provides no warranties regarding the legality, accuracy, or suitability of external assets. The project maintainers are not liable for any issues arising from the use of these resources.