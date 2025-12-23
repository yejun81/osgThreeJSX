#include <osg/TextureCubeMap>
#include <osg/CullFace>
#include <osg/FrontFace>
#include <osgThreeJSX/Material>
#include <osgThreeJSX/MaterialData>
#include <osgThreeJSX/MaterialNode>
#include <osgThreeJSX/Materials>
#include <osgThreeJSX/Programs>
#include <osgThreeJSX/ShaderLib>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/PointLight>

using namespace osgThreeJSX;

Material::Material()
{
	_vertexAttribList.push_back(MaterailVertexAttrib("position", 0));
	_vertexAttribList.push_back(MaterailVertexAttrib("normal", 1));
	_vertexAttribList.push_back(MaterailVertexAttrib("color", 2));
	_vertexAttribList.push_back(MaterailVertexAttrib("uv", 3));
	_vertexAttribList.push_back(MaterailVertexAttrib("uv2", 4));

	_curVersion = 1;
	_preVersion = 0;

	_startTextureUnit = 0;

	setVertexTangents(false);
	setVertexColors(false);
	setFlatShading(false);
	setSide(MaterialSideType_FrontSide);

	setDithering(false);
	setPremultipliedAlpha(false);
	setAlphaTest(-1.0);
	setFog(false);

	setSkinning(false);
	setMaxBones(0);
	setMorphTargets(false);
	setMorphNormals(false);

	setReceiveShadow(false);
	setCastShadow(false);

	setTransparent(false);
	_blendSrc = osg::BlendFunc::SRC_ALPHA;
	_blendSrcAlpha = osg::BlendFunc::ONE;
	_blendDst = osg::BlendFunc::ONE_MINUS_SRC_ALPHA;
	_blendDstAlpha = osg::BlendFunc::ONE;
	_blendEquation = osg::BlendEquation::FUNC_ADD;
	_blendEquationAlpha = osg::BlendEquation::FUNC_ADD;

	setInstancing(false);
}

Material::Material(const Material& other, const osg::CopyOp& copyop)
{
	_curVersion = 1;
	_preVersion = 0;
}

Material::~Material()
{

}

void Material::removeUniform(const std::string& name)
{
	for (MaterialUniformList::iterator iter = _uniforms.begin(); iter != _uniforms.end(); iter++)
	{
		if ((*iter)->getName() == name)
		{
			_uniforms.erase(iter);
			return;
		}
	}
}

void Material::setTexture(const std::string& name, osg::ref_ptr<osg::Texture>& texture)
{
	for (MaterialTextureList::iterator iter = _textures.begin(); iter != _textures.end(); iter++)
	{
		if (iter->_name == name)
		{
			iter->_texture = texture;
			return;
		}
	}

	_textures.push_back(MaterialTexture(name, texture));
}

void Material::removeTexture(const std::string& name)
{
	for (MaterialTextureList::iterator iter = _textures.begin(); iter != _textures.end(); iter++)
	{
		if (iter->_name == name)
		{
			_textures.erase(iter);
			return;
		}
	}
}

bool Material::getDefine(const std::string& key, std::string& value)
{
	DefineMap::iterator iter = _defines.find(key);
	if (iter != _defines.end())
	{
		value = iter->second;
		return true;
	}
	return false;
}

void Material::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	getOrCreateUniform(uniforms, "receiveShadow", getReceiveShadow());
}

void Material::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	ShaderObject* shaderObject = ShaderLib::instance().getShaderObject(getShaderId());
	if (shaderObject)
	{
		parameters.vertex = shaderObject->vertex;
		parameters.fragment = shaderObject->fragment;

		onBeforeCompile(camera, cv, parameters);
	}

	parameters.isRaw = isRaw();
	parameters.defines = getDefines();

	parameters.dithering = getDithering();
	parameters.premultipliedAlpha = getPremultipliedAlpha();
	parameters.alphaTest = getAlphaTest();

	parameters.vertexTangents = getVertexTangents();
	parameters.vertexColors = getVertexColors();

	parameters.flatShading = getFlatShading();
	parameters.doubleSided = (getSide() == MaterialSideType_DoubleSide);
	parameters.flipSided = (getSide() == MaterialSideType_BackSide);

	parameters.maxBones = getMaxBones();
	parameters.skinning = getSkinning();
	parameters.morphTargets = getMorphTargets();
	parameters.morphNormals = getMorphNormals();

	parameters.instancing = getInstancing();

	parameters.useFog = getFog();
}

void Material::update(osg::Camera* camera, osgUtil::CullVisitor* cv, osg::Node* node)
{
	//if (!_stateset)
	//	_stateset = new osg::StateSet();
	//osg::StateSet* stateset = _stateset;
	osg::StateSet* stateset = NULL;
	if (node->asGroup())
	{
		if (!_stateset.valid())
		{
			_stateset = new osg::StateSet();
		}
		stateset = _stateset.get();
	}
	else
	{
		stateset = node->getOrCreateStateSet();
	}

	bool stateChange = false;

	if (generateProgram())
	{
		ProgramParameters parameters;
		ProgramGenerator::instance().getParameters(this, camera, cv, parameters);

		std::string key = ProgramGenerator::instance().getKey(parameters);
		bool programChange = true;

		if (_program.valid())
		{
			if (_program->getKey() != key)
			{
				programChange = true;
			}
			else
			{
				programChange = false;
			}
		}

		if (programChange)
		{
			_program = ProgramGenerator::instance().getOrCreateProgram(key, parameters);
			for (MaterialVertexAttribList::iterator iter = _vertexAttribList.begin(); iter != _vertexAttribList.end(); iter++)
			{
				_program->getOsgProgram()->addBindAttribLocation(iter->_name, iter->_index);
			}
			stateset->setAttributeAndModes(_program->getOsgProgram(), osg::StateAttribute::ON);

			stateChange = true;
		}
	}

	if (_preVersion != _curVersion)
	{
		stateChange = true;
	}

	if (stateChange)
	{
		if (getSide() != MaterialSideType_DoubleSide)
		{
			osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
			stateset->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

			osg::ref_ptr<osg::FrontFace> ff = new osg::FrontFace();
			ff->setMode(getSide() == MaterialSideType_BackSide ? osg::FrontFace::CLOCKWISE : osg::FrontFace::COUNTER_CLOCKWISE);
			stateset->setAttributeAndModes(ff.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
		}

		if (getTransparent())
		{
			stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
			stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

			osg::ref_ptr<osg::BlendEquation> be = new osg::BlendEquation(_blendEquation, _blendEquationAlpha);
			stateset->setAttributeAndModes(be.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

			osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc(_blendSrc, _blendDst, _blendDst, _blendDstAlpha);
			stateset->setAttributeAndModes(bf.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
		}

		MaterialTextureList textures;
		MaterialUniformList uniforms;

		///insert render state env here, ugly...
		RenderState* rs = RenderState::FromCamera(camera);
		if (rs && rs->getBackgroudEnv())
		{
			rs->getBackgroudEnv()->buildUniformAndTexture(this, uniforms, textures);
		}

		buildUniformAndTexture(uniforms, textures);

		textures.insert(textures.end(), _textures.begin(), _textures.end());
		uniforms.insert(uniforms.end(), _uniforms.begin(), _uniforms.end());

		int startTextureUnit = _startTextureUnit;
		for (MaterialTextureList::iterator iter = textures.begin(); iter != textures.end(); iter++)
		{
			stateset->addUniform(new osg::Uniform(iter->_name.c_str(), startTextureUnit));
			stateset->setTextureAttributeAndModes(startTextureUnit, iter->_texture, osg::StateAttribute::ON);
			startTextureUnit += 1;
		}

		for (MaterialUniformList::iterator iter = uniforms.begin(); iter != uniforms.end(); iter++)
		{
			stateset->addUniform(*iter);
		}

		_preVersion = _curVersion;
	}
}

osg::ref_ptr<Material> Material::getOrCreateDepthMaterial(Light* light)
{
	if (!_depthMaterial)
	{
		if (light->getType() == LightType_Point)
		{
			MaterialDistance* material = new MaterialDistance();
			PointLight* pointLight = dynamic_cast<PointLight*>(light);
			if (pointLight)
			{
				material->_nearDistance = 0.1;
				material->_farDistance = pointLight->getDistance();
				material->_referencePosition = pointLight->getPosition();
			}
			_depthMaterial = material;
		}
		else
			_depthMaterial = new MaterialDepth();

		if (_depthMaterial)
		{
			_depthMaterial->setMaxBones(getMaxBones());
			_depthMaterial->setSkinning(getSkinning());
			_depthMaterial->setMorphTargets(getMorphTargets());
			_depthMaterial->setMorphNormals(getMorphNormals());
		}
	}
	return _depthMaterial;
}
