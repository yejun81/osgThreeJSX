
#include <regex>
#include <string>
#include <sstream>

#include <osgThreeJSX/ShaderLib>
#include <osgThreeJSX/MaterialNode>
#include <osgThreeJSX/Programs>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/MaterialData>

using namespace osgThreeJSX;

//
template<class Traits, class CharT, class UnaryFunction>
std::string StringReplace(const std::string& str,
	const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
{
	typedef std::string::const_iterator BidirIterator;	
	typename std::match_results<BidirIterator>::difference_type
		lastMatch = 0;
	auto endOfLastMatch = str.cbegin();

	std::basic_string<CharT> s;
	auto callback = [&](const std::match_results<BidirIterator>& match)
	{
		auto curMatch = match.position(0);
		auto diff = curMatch - lastMatch;

		auto startOfThisMatch = endOfLastMatch;
		std::advance(startOfThisMatch, diff);

		s.append(endOfLastMatch, startOfThisMatch);
		s.append(f(match));

		auto lengthOfMatch = match.length(0);

		lastMatch = curMatch + lengthOfMatch;

		endOfLastMatch = startOfThisMatch;
		std::advance(endOfLastMatch, lengthOfMatch);
	};

	std::regex_iterator<BidirIterator> begin(str.cbegin(), str.cend(), re), end;
	std::for_each(begin, end, callback);

	s.append(endOfLastMatch, str.cend());

	return s;
}

//////////////////////////////////////////////////////////////////////////
ProgramParameters::ProgramParameters()
{
	morphTargets = false;
	morphNormals = false;

	skinning = false;
	useVertexTexture = false;

	vertexTangents = false;
	vertexColors = false;
	vertexUvs = false;
	uvsVertexOnly = false;

	flatShading = false;
	doubleSided = false;
	flipSided = false;
	sizeAttenuation = false;

	maxBones = 0;

	map = false;
	matcap = false;
	envMap = false;
	lightMap = false;
	aoMap = false;
	emissiveMap = false;
	bumpMap = false;
	normalMap = false;
	clearcoatMap = false;
	clearcoatRoughnessMap = false;
	clearcoatNormalMap = false;
	displacementMap = false;
	specularMap = false;
	roughnessMap = false;
	metalnessMap = false;
	alphaMap = false;
	gradientMap = false;

	numDirLights = 0;
	numSpotLights = 0;
	numRectAreaLights = 0;
	numPointLights = 0;
	numHemiLights = 0;
	numDirLightShadows = 0;
	numSpotLightShadows = 0;
	numPointLightShadows = 0;

	numClippingPlanes = 0;
	numClipIntersection = 0;

	isOrthographic = false;

	alphaTest = -1.0;
	gammaFactor = 2.2f;

	fog = false;
	fogExp2 = false;

	logarithmicDepthBuffer = false;
	rendererExtensionFragDepth = false;

	shadowMapEnabled = false;
	shadowMapType = ShadowMapType_BasicShadowMap;

	depthPacking = DepthPackingType_No;

	physicallyCorrectLights = false;

	sheen = false;

	instancing = false;
}

//////////////////////////////////////////////////////////////////////////
Program::Program()
{

}
Program::Program(const std::string& cacheKey, const ProgramParameters& parameters) :_cacheKey(cacheKey)
{
	std::string customDefines = generateDefines(parameters);
	std::string precision = generatePrecision(parameters);

	float gammaFactorDefine = parameters.gammaFactor;

	std::string envMapTypeDefine = generateEnvMapTypeDefine(parameters);
	std::string envMapModeDefine = generateEnvMapModeDefine(parameters);
	std::string envMapBlendingDefine = generateEnvMapBlendingDefine(parameters);

	std::string shadowMapTypeDefine = generateShadowMapTypeDefine(parameters);

	std::stringstream prefixVertex, prefixFragment;

	prefixVertex << "#version " << parameters.glslversion << "\n";
	prefixVertex << precision << "\n";
	prefixVertex << customDefines << "\n";
	prefixVertex << "#define attribute in" << "\n";
	prefixVertex << "#define varying out" << "\n";
	prefixVertex << "#define texture2D texture" << "\n";
	prefixVertex << "#define textureCube texture" << "\n";

	if (parameters.supportsVertexTextures) prefixVertex << "#define VERTEX_TEXTURES\n";

	prefixVertex << "#define GAMMA_FACTOR " << gammaFactorDefine << "\n";

	prefixVertex << "#define MAX_BONES " << parameters.maxBones << "\n";

	if ((parameters.useFog) && (parameters.fog)) prefixVertex << "#define USE_FOG\n";
	if ((parameters.useFog) && (parameters.fogExp2)) prefixVertex << "#define FOG_EXP2\n";

	if (parameters.map) prefixVertex << "#define USE_MAP\n";
	if (parameters.envMap) prefixVertex << "#define USE_ENVMAP\n";
	if (parameters.envMap) prefixVertex << "#define " << envMapTypeDefine << "\n";
	if (parameters.envMap) prefixVertex << "#define " << envMapModeDefine << "\n";
	if (parameters.envMap) prefixVertex << "#define " << envMapBlendingDefine << "\n";
	if (parameters.lightMap) prefixVertex << "#define USE_LIGHTMAP\n";
	if (parameters.aoMap) prefixVertex << "#define USE_AOMAP\n";
	if (parameters.emissiveMap) prefixVertex << "#define USE_EMISSIVEMAP\n";
	if (parameters.bumpMap) prefixVertex << "#define USE_BUMPMAP\n";
	if (parameters.normalMap) prefixVertex << "#define USE_NORMALMAP\n";
	if (parameters.normalMap && parameters.objectSpaceNormalMap) prefixVertex << "#define OBJECTSPACE_NORMALMAP\n";
	if (parameters.normalMap && parameters.tangentSpaceNormalMap) prefixVertex << "#define TANGENTSPACE_NORMALMAP\n";

	if (parameters.clearcoatMap) prefixVertex << "#define USE_CLEARCOATMAP\n";
	if (parameters.clearcoatRoughnessMap) prefixVertex << "#define USE_CLEARCOAT_ROUGHNESSMAP\n";
	if (parameters.clearcoatNormalMap) prefixVertex << "#define USE_CLEARCOAT_NORMALMAP\n";
	if (parameters.displacementMap && parameters.supportsVertexTextures) prefixVertex << "#define USE_DISPLACEMENTMAP\n";
	if (parameters.specularMap) prefixVertex << "#define USE_SPECULARMAP\n";
	if (parameters.roughnessMap) prefixVertex << "#define USE_ROUGHNESSMAP\n";
	if (parameters.metalnessMap) prefixVertex << "#define USE_METALNESSMAP\n";
	if (parameters.alphaMap) prefixVertex << "#define USE_ALPHAMAP\n";

	if (parameters.vertexTangents) prefixVertex << "#define USE_TANGENT\n";
	if (parameters.vertexColors) prefixVertex << "#define USE_COLOR\n";
	if (parameters.vertexUvs) prefixVertex << "#define USE_UV\n";
	if (parameters.uvsVertexOnly) prefixVertex << "#define UVS_VERTEX_ONLY\n";

	if (parameters.flatShading) prefixVertex << "#define FLAT_SHADED\n";

	if (parameters.skinning) prefixVertex << "#define USE_SKINNING\n";
	if (parameters.useVertexTexture) prefixVertex << "#define BONE_TEXTURE\n";

	if (parameters.morphTargets) prefixVertex << "#define USE_MORPHTARGETS\n";
	if (parameters.morphNormals && parameters.flatShading == false) prefixVertex << "#define USE_MORPHNORMALS\n";

	if (parameters.doubleSided) prefixVertex << "#define DOUBLE_SIDED\n";
	if (parameters.flipSided) prefixVertex << "#define FLIP_SIDED\n";

	if (parameters.shadowMapEnabled) prefixVertex << "#define USE_SHADOWMAP\n";
	if (parameters.shadowMapEnabled) prefixVertex << "#define " << shadowMapTypeDefine << "\n";

	if (parameters.sizeAttenuation) prefixVertex << "#define USE_SIZEATTENUATION\n";

	if (parameters.logarithmicDepthBuffer) prefixVertex << "#define USE_LOGDEPTHBUF\n";
	if (parameters.logarithmicDepthBuffer && parameters.rendererExtensionFragDepth) prefixVertex << "#define USE_LOGDEPTHBUF_EXT\n";

	if (parameters.instancing)
	{
		prefixVertex << "#define USE_INSTANCING\n";
		prefixVertex << "uniform sampler2D instanceImage;\n";
		prefixVertex << "mat4 getInstanceMatrix(){\n";
		prefixVertex << "mat4 instanceMat = mat4(texelFetch(instanceImage, ivec2(0, gl_InstanceID), 0),\n";
		prefixVertex << "texelFetch(instanceImage, ivec2(1, gl_InstanceID), 0),\n";
		prefixVertex << "texelFetch(instanceImage, ivec2(2, gl_InstanceID), 0),\n";
		prefixVertex << "texelFetch(instanceImage, ivec2(3, gl_InstanceID), 0)); \n";
		prefixVertex << "return instanceMat;} \n";
		prefixVertex << "#define instanceMatrix getInstanceMatrix() \n";
	}
	prefixVertex << "#define modelMatrix (osg_ViewMatrixInverse*osg_ModelViewMatrix)\n";
	prefixVertex << "#define modelViewMatrix (osg_ModelViewMatrix)\n";
	prefixVertex << "#define projectionMatrix (osg_ProjectionMatrix)\n";
	prefixVertex << "#define viewMatrix (osg_ViewMatrix)\n";
	prefixVertex << "#define normalMatrix (osg_NormalMatrix)\n";
	prefixVertex << "#define cameraPosition (osg_ViewMatrixInverse[3].xyz)\n";
	if (parameters.isOrthographic)
	{
		prefixVertex << "#define isOrthographic true\n";
	}
	else
	{
		prefixVertex << "#define isOrthographic false\n";
	}

	prefixVertex << "uniform mat4 osg_ViewMatrixInverse;\n";
	prefixVertex << "uniform mat4 osg_ModelViewMatrix;\n";
	prefixVertex << "uniform mat4 osg_ProjectionMatrix;\n";
	prefixVertex << "uniform mat4 osg_ViewMatrix;\n";
	prefixVertex << "uniform mat3 osg_NormalMatrix;\n";
	prefixVertex << "uniform mat4 osg_ModelViewProjectionMatrix;\n";

	prefixVertex << "attribute vec3 position;\n";
	prefixVertex << "attribute vec3 normal;\n";
	prefixVertex << "attribute vec2 uv;\n";
	prefixVertex << "#ifdef USE_TANGENT\n";
	prefixVertex << "	attribute vec4 tangent;\n";
	prefixVertex << "#endif\n";

	prefixVertex << "#ifdef USE_COLOR\n";
	prefixVertex << "	attribute vec3 color;\n";
	prefixVertex << "#endif\n";

	prefixVertex << "#ifdef USE_MORPHTARGETS\n";
	prefixVertex << "	attribute vec3 morphTarget0;\n";
	prefixVertex << "	attribute vec3 morphTarget1;\n";
	prefixVertex << "	attribute vec3 morphTarget2;\n";
	prefixVertex << "	attribute vec3 morphTarget3;\n";
	prefixVertex << "	#ifdef USE_MORPHNORMALS\n";
	prefixVertex << "		attribute vec3 morphNormal0;\n";
	prefixVertex << "		attribute vec3 morphNormal1;\n";
	prefixVertex << "		attribute vec3 morphNormal2;\n";
	prefixVertex << "		attribute vec3 morphNormal3;\n";
	prefixVertex << "	#else\n";
	prefixVertex << "		attribute vec3 morphTarget4;\n";
	prefixVertex << "		attribute vec3 morphTarget5;\n";
	prefixVertex << "		attribute vec3 morphTarget6;\n";
	prefixVertex << "		attribute vec3 morphTarget7;\n";
	prefixVertex << "	#endif\n";
	prefixVertex << "#endif\n";

	prefixVertex << "#ifdef USE_SKINNING\n";
	prefixVertex << "	attribute vec4 skinIndex;\n";
	prefixVertex << "	attribute vec4 skinWeight;\n";
	prefixVertex << "#endif\n";

	prefixFragment << "#version " << parameters.glslversion << "\n";
	prefixFragment << "#define varying in" << "\n";

	prefixFragment << "out highp vec4 pc_fragColor;" << "\n";
	prefixFragment << "#define gl_FragColor pc_fragColor" << "\n";
	prefixFragment << precision << "\n";
	prefixFragment << customDefines << "\n";

	prefixFragment << "#define texture2D texture" << "\n";
	prefixFragment << "#define textureCube texture" << "\n";

	if (parameters.alphaTest > 0)
		prefixFragment << "#define ALPHATEST " << std::fixed << parameters.alphaTest << "\n";
	//prefixFragment << "#define ALPHATEST 0.0" << "\n";

	prefixFragment << "#define GAMMA_FACTOR " << std::fixed << gammaFactorDefine << "\n";
	//prefixFragment << "#define GAMMA_FACTOR 2.0" << "\n";

	if ((parameters.useFog) && (parameters.fog)) prefixFragment << "#define USE_FOG\n";
	if ((parameters.useFog) && (parameters.fogExp2)) prefixFragment << "#define FOG_EXP2\n";

	if (parameters.map) prefixFragment << "#define USE_MAP\n";
	if (parameters.matcap) prefixFragment << "#define USE_MATCAP\n";
	if (parameters.envMap) prefixFragment << "#define USE_ENVMAP\n";
	if (parameters.envMap) prefixFragment << "#define " << envMapTypeDefine << "\n";
	if (parameters.envMap) prefixFragment << "#define " << envMapModeDefine << "\n";
	if (parameters.envMap) prefixFragment << "#define " << envMapBlendingDefine << "\n";
	if (parameters.lightMap) prefixFragment << "#define USE_LIGHTMAP\n";
	if (parameters.aoMap) prefixFragment << "#define USE_AOMAP\n";
	if (parameters.emissiveMap) prefixFragment << "#define USE_EMISSIVEMAP\n";
	if (parameters.bumpMap) prefixFragment << "#define USE_BUMPMAP\n";
	if (parameters.normalMap) prefixFragment << "#define USE_NORMALMAP\n";
	if (parameters.normalMap && parameters.objectSpaceNormalMap) prefixFragment << "#define OBJECTSPACE_NORMALMAP\n";
	if (parameters.normalMap && parameters.tangentSpaceNormalMap) prefixFragment << "#define TANGENTSPACE_NORMALMAP\n";

	if (parameters.clearcoatMap) prefixFragment << "#define USE_CLEARCOATMAP\n";
	if (parameters.clearcoatRoughnessMap) prefixFragment << "#define USE_CLEARCOAT_ROUGHNESSMAP\n";
	if (parameters.clearcoatNormalMap) prefixFragment << "#define USE_CLEARCOAT_NORMALMAP\n";
	if (parameters.displacementMap && parameters.supportsVertexTextures) prefixFragment << "#define USE_DISPLACEMENTMAP\n";
	if (parameters.specularMap) prefixFragment << "#define USE_SPECULARMAP\n";
	if (parameters.roughnessMap) prefixFragment << "#define USE_ROUGHNESSMAP\n";
	if (parameters.metalnessMap) prefixFragment << "#define USE_METALNESSMAP\n";
	if (parameters.alphaMap) prefixFragment << "#define USE_ALPHAMAP\n";

	if (parameters.sheen) prefixFragment << "#define USE_SHEEN\n";

	if (parameters.vertexTangents) prefixFragment << "#define USE_TANGENT\n";
	if (parameters.vertexColors) prefixFragment << "#define USE_COLOR\n";
	if (parameters.vertexUvs) prefixFragment << "#define USE_UV\n";
	if (parameters.uvsVertexOnly) prefixFragment << "#define UVS_VERTEX_ONLY\n";

	if (parameters.gradientMap) prefixFragment << "#define USE_GRADIENTMAP\n";

	if (parameters.flatShading) prefixFragment << "#define FLAT_SHADED\n";

	if (parameters.doubleSided) prefixFragment << "#define DOUBLE_SIDED\n";
	if (parameters.flipSided) prefixFragment << "#define FLIP_SIDED\n";

	if (parameters.shadowMapEnabled) prefixFragment << "#define USE_SHADOWMAP\n";
	if (parameters.shadowMapEnabled) prefixFragment << "#define " << shadowMapTypeDefine << "\n";

	if (parameters.premultipliedAlpha) prefixFragment << "#define PREMULTIPLIED_ALPHA\n";

	if (parameters.physicallyCorrectLights) prefixFragment << "#define PHYSICALLY_CORRECT_LIGHTS\n";

	if (parameters.logarithmicDepthBuffer) prefixFragment << "#define USE_LOGDEPTHBUF\n";
	if (parameters.logarithmicDepthBuffer && parameters.rendererExtensionFragDepth) prefixFragment << "#define USE_LOGDEPTHBUF_EXT\n";

	prefixFragment << "#define viewMatrix (osg_ViewMatrix)\n";
	prefixFragment << "#define cameraPosition (osg_ViewMatrixInverse[3].xyz)\n";
	if (parameters.isOrthographic)
	{
		prefixFragment << "#define isOrthographic true\n";
	}
	else
	{
		prefixFragment << "#define isOrthographic false\n";
	}
	prefixFragment << "#define normalMatrix (osg_NormalMatrix)\n";

	prefixFragment << "#define TEXTURE_LOD_EXT\n";
	prefixFragment << "#define textureCubeLodEXT textureLod\n";
	prefixFragment << "#define texture2DLodEXT textureLod\n";
	prefixFragment << "#define gl_FragDepthEXT gl_FragDepth\n";

	prefixFragment << "uniform mat4 osg_ViewMatrixInverse;\n";
	prefixFragment << "uniform mat4 osg_ModelViewMatrix;\n";
	prefixFragment << "uniform mat4 osg_ProjectionMatrix;\n";
	prefixFragment << "uniform mat4 osg_ViewMatrix;\n";
	prefixFragment << "uniform mat3 osg_NormalMatrix;\n";

	if (parameters.toneMapping != ToneMappingType_NoToneMapping) prefixFragment << "#define TONE_MAPPING\n";
	if (parameters.toneMapping != ToneMappingType_NoToneMapping) prefixFragment << ShaderChunk::get("tonemapping_pars_fragment") << "\n";
	if (parameters.toneMapping != ToneMappingType_NoToneMapping) prefixFragment << getToneMappingFunction("toneMapping", parameters) << "\n";

	if (parameters.dithering) prefixFragment << "#define DITHERING\n";

	if (parameters.outputEncoding || parameters.mapEncoding || parameters.matcapEncoding ||
		parameters.envMapEncoding || parameters.emissiveMapEncoding || parameters.lightMapEncoding)
	{
		prefixFragment << ShaderChunk::get("encodings_pars_fragment") << "\n";
	}
	if (parameters.mapEncoding) prefixFragment << getTexelDecodingFunction("mapTexelToLinear", parameters.mapEncoding) << "\n";
	if (parameters.matcapEncoding) prefixFragment << getTexelDecodingFunction("matcapTexelToLinear", parameters.matcapEncoding) << "\n";
	if (parameters.envMapEncoding) prefixFragment << getTexelDecodingFunction("envMapTexelToLinear", parameters.envMapEncoding) << "\n";
	if (parameters.emissiveMapEncoding) prefixFragment << getTexelDecodingFunction("emissiveMapTexelToLinear", parameters.emissiveMapEncoding) << "\n";
	if (parameters.lightMapEncoding) prefixFragment << getTexelDecodingFunction("lightMapTexelToLinear", parameters.lightMapEncoding) << "\n";
	if (parameters.outputEncoding) prefixFragment << getTexelEncodingFunction("linearToOutputTexel", parameters.outputEncoding) << "\n";

	if (parameters.depthPacking != DepthPackingType_No) prefixFragment << "#define DEPTH_PACKING " << parameters.depthPacking << "\n";

	std::string vertexGlsl;
	std::string fragmentGlsl;
	if (parameters.isRaw)
	{
		vertexGlsl = parameters.vertex;
		fragmentGlsl = parameters.fragment;
	}
	else
	{
		std::string vertexShader = resolveIncludes(parameters.vertex);
		vertexShader = replaceLightNums(vertexShader, parameters);
		vertexShader = replaceClippingPlaneNums(vertexShader, parameters);
		vertexShader = unrollLoops(vertexShader);

		std::string fragmentShader = resolveIncludes(parameters.fragment);
		fragmentShader = replaceLightNums(fragmentShader, parameters);
		fragmentShader = replaceClippingPlaneNums(fragmentShader, parameters);
		fragmentShader = unrollLoops(fragmentShader);

		vertexGlsl = prefixVertex.str() + vertexShader;
		fragmentGlsl = prefixFragment.str() + fragmentShader;
	}

	_osgProgram = new osg::Program();
	_osgProgram->addShader(new osg::Shader(osg::Shader::VERTEX, vertexGlsl));
	_osgProgram->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentGlsl));
}

ShaderObject* Program::getShaderObject(const std::string& name)
{
	ShaderObject* shaderObject = ShaderLib::instance().getShaderObject(name);
	if (shaderObject)
	{
		ShaderObject* processedShaderObject = new ShaderObject();
		processedShaderObject->vertex = resolveIncludes(shaderObject->vertex);
		processedShaderObject->fragment = resolveIncludes(shaderObject->fragment);
		return processedShaderObject;
	}

	return nullptr;
}

std::string Program::generatePrecision(const ProgramParameters& parameters)
{
	std::stringstream ss;
	ss << "precision " << parameters.precision << " float;\nprecision " << parameters.precision + " int;";

	if (parameters.precision == "highp") {
		ss << "\n#define HIGH_PRECISION";
	}
	else if (parameters.precision == "mediump") {
		ss << "\n#define MEDIUM_PRECISION";
	}
	else if (parameters.precision == "lowp") {
		ss << "\n#define LOW_PRECISION";
	}

	return ss.str();
}

std::string Program::resolveIncludes(const std::string& shaderText)
{
	std::regex includePattern("[ \\t]*#include +<([\\w\\d./]+)>");
	return StringReplace(shaderText, includePattern, [=](const std::smatch& match) {
		return resolveIncludes(ShaderChunk::get(match.str(1)));
	});
}

std::string Program::replaceLightNums(const std::string& shaderText, const ProgramParameters& parameters)
{
	std::regex pattern;
	pattern = std::regex("NUM_DIR_LIGHTS");
	std::string ret = StringReplace(shaderText, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numDirLights;
		return ss.str();
	});

	pattern = std::regex("NUM_SPOT_LIGHTS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numSpotLights;
		return ss.str();
	});

	pattern = std::regex("NUM_RECT_AREA_LIGHTS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numRectAreaLights;
		return ss.str();
	});

	pattern = std::regex("NUM_POINT_LIGHTS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numPointLights;
		return ss.str();
	});

	pattern = std::regex("NUM_HEMI_LIGHTS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numHemiLights;
		return ss.str();
	});

	pattern = std::regex("NUM_DIR_LIGHT_SHADOWS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numDirLightShadows;
		return ss.str();
	});


	pattern = std::regex("NUM_SPOT_LIGHT_SHADOWS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numSpotLightShadows;
		return ss.str();
	});

	pattern = std::regex("NUM_POINT_LIGHT_SHADOWS");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numPointLightShadows;
		return ss.str();
	});

	return ret;
}

std::string Program::replaceClippingPlaneNums(const std::string& shaderText, const ProgramParameters& parameters)
{
	std::regex pattern;
	pattern = std::regex("NUM_CLIPPING_PLANES");
	std::string ret = StringReplace(shaderText, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << parameters.numClippingPlanes;
		return ss.str();
	});

	pattern = std::regex("UNION_CLIPPING_PLANES");
	ret = StringReplace(ret, pattern, [=](const std::smatch& match) {
		std::stringstream ss;
		ss << (parameters.numClippingPlanes - parameters.numClipIntersection);
		return ss.str();
	});

	return ret;
}

std::string Program::unrollLoops(const std::string& shaderText)
{
	std::regex includePattern("#pragma unroll_loop_start[\\s]+?for \\( int i \\= (\\d+)\\; i < (\\d+)\\; i \\+\\+ \\) \\{([\\s\\S]+?)(?=\\})\\}[\\s]+?#pragma unroll_loop_end");

	return StringReplace(shaderText, includePattern, [=](const std::smatch& match) {

		std::string string;
		int start = atoi(match[1].str().c_str());
		int end = atoi(match[2].str().c_str());

		std::regex iReg("\\[ i \\]");
		std::regex markReg("UNROLLED_LOOP_INDEX");
		for (int i = start; i < end; i++)
		{
			std::stringstream strR;
			strR << "[ " << i << " ]";
			std::string tmp = std::regex_replace(match[3].str(), iReg, strR.str());
			std::stringstream strR1;
			strR1 << i;
			tmp = std::regex_replace(tmp, markReg, strR1.str());
			string += tmp;
		}

		return string;
	});
}

std::string Program::generateDefines(const ProgramParameters& parameters)
{
	std::stringstream ss;

	for (DefineMap::const_iterator iter = parameters.defines.begin(); iter != parameters.defines.end(); iter++) {
		ss << "#define " << iter->first.c_str() << " " << iter->second.c_str() << "\n";
	}

	return ss.str();
}

std::string Program::generateEnvMapModeDefine(const ProgramParameters& parameters)
{
	std::string envMapModeDefine("ENVMAP_MODE_REFLECTION");
	if (parameters.envMap)
	{
		switch (parameters.envMapMode)
		{
		case EnvMapModeType_CubeRefractionMapping:
		case EnvMapModeType_EquirectangularRefractionMapping:
			envMapModeDefine = "ENVMAP_MODE_REFRACTION";
			break;
		}
	}

	return envMapModeDefine;
}

std::string Program::generateEnvMapTypeDefine(const ProgramParameters& parameters)
{
	std::string envMapTypeDefine("ENVMAP_TYPE_CUBE");
	if (parameters.envMap)
	{
		switch (parameters.envMapMode)
		{
		case EnvMapModeType_CubeRefractionMapping:
		case EnvMapModeType_CubeReflectionMapping:
			envMapTypeDefine = "ENVMAP_TYPE_CUBE";
			break;
		case EnvMapModeType_CubeUVReflectionMapping:
		case EnvMapModeType_CubeUVRefractionMapping:
			envMapTypeDefine = "ENVMAP_TYPE_CUBE_UV";
			break;
		case EnvMapModeType_EquirectangularReflectionMapping:
		case EnvMapModeType_EquirectangularRefractionMapping:
			envMapTypeDefine = "ENVMAP_TYPE_EQUIREC";
			break;

		case EnvMapModeType_SphericalReflectionMapping:
			envMapTypeDefine = "ENVMAP_TYPE_SPHERE";
			break;
		}
	}

	return envMapTypeDefine;
}

std::string Program::generateEnvMapBlendingDefine(const ProgramParameters& parameters)
{
	std::string envMapBlendingDefine = "ENVMAP_BLENDING_NONE";

	if (parameters.envMap)
	{
		switch (parameters.combine)
		{
		case EnvMapCombineType_No:
			envMapBlendingDefine = "ENVMAP_BLENDING_NONE";
			break;
		case EnvMapCombineType_MultiplyOperation:
			envMapBlendingDefine = "ENVMAP_BLENDING_MULTIPLY";
			break;
		case EnvMapCombineType_MixOperation:
			envMapBlendingDefine = "ENVMAP_BLENDING_MIX";
			break;
		case EnvMapCombineType_AddOperation:
			envMapBlendingDefine = "ENVMAP_BLENDING_ADD";
			break;

		}

	}

	return envMapBlendingDefine;

}

std::string Program::generateShadowMapTypeDefine(const ProgramParameters& parameters)
{
	std::string shadowMapTypeDefine = "SHADOWMAP_TYPE_BASIC";

	switch (parameters.shadowMapType)
	{
	case ShadowMapType_PCFShadowMap:
		shadowMapTypeDefine = "SHADOWMAP_TYPE_PCF";
		break;
	case ShadowMapType_PCFSoftShadowMap:
		shadowMapTypeDefine = "SHADOWMAP_TYPE_PCF_SOFT";
		break;
	case ShadowMapType_VSMShadowMap:
		shadowMapTypeDefine = "SHADOWMAP_TYPE_VSM";
		break;
	}

	return shadowMapTypeDefine;

}

std::string Program::getToneMappingFunction(const std::string& functionName, const ProgramParameters& parameters)
{
	std::string toneMappingName;

	switch (parameters.toneMapping)
	{
	case ToneMappingType_LinearToneMapping:
		toneMappingName = "Linear";
		break;

	case ToneMappingType_ReinhardToneMapping:
		toneMappingName = "Reinhard";
		break;

	case ToneMappingType_Uncharted2ToneMapping:
		toneMappingName = "Uncharted2";
		break;

	case ToneMappingType_CineonToneMapping:
		toneMappingName = "OptimizedCineon";
		break;

	case ToneMappingType_ACESFilmicToneMapping:
		toneMappingName = "ACESFilmic";
		break;

	}

	std::stringstream ss;
	ss << "vec3 " << functionName << "( vec3 color ) { return " + toneMappingName + "ToneMapping( color ); }";
	return ss.str();
}

std::string Program::getTexelDecodingFunction(const std::string& functionName, TextureEncodingType encoding)
{
	TextureEncodingComponent* components = ProgramGenerator::instance().getTextureEncodingComponent(encoding);
	if (components == NULL)
		return "";

	std::stringstream ss;
	ss << "vec4 " << functionName << "( vec4 value ) { return " << components->name << "ToLinear" << components->component << "; }";
	return ss.str();
}

std::string Program::getTexelEncodingFunction(const std::string& functionName, TextureEncodingType encoding)
{
	TextureEncodingComponent* components = ProgramGenerator::instance().getTextureEncodingComponent(encoding);
	if (components == NULL)
		return "";

	std::stringstream ss;
	ss << "vec4 " << functionName << "( vec4 value ) { return LinearTo" << components->name << components->component << "; }";
	return ss.str();
}

//////////////////////////////////////////////////////////////////////////

ProgramGenerator::ProgramGenerator()
{
	_textureEncodingMap[TextureEncodingType_LinearEncoding] = TextureEncodingComponent{ "Linear", "( value )" };
	_textureEncodingMap[TextureEncodingType_sRGBEncoding] = TextureEncodingComponent{ "sRGB", "( value )" };
	_textureEncodingMap[TextureEncodingType_RGBEEncoding] = TextureEncodingComponent{ "RGBE", "( value )" };
	_textureEncodingMap[TextureEncodingType_RGBM7Encoding] = TextureEncodingComponent{ "RGBM", "( value, 7.0 )" };
	_textureEncodingMap[TextureEncodingType_RGBM16Encoding] = TextureEncodingComponent{ "RGBM", "( value, 16.0 )" };
	_textureEncodingMap[TextureEncodingType_RGBDEncoding] = TextureEncodingComponent{ "RGBD", "( value, 256.0 )" };
	_textureEncodingMap[TextureEncodingType_GammaEncoding] = TextureEncodingComponent{ "Gamma", "( value, float( GAMMA_FACTOR ) )" };
	_textureEncodingMap[TextureEncodingType_LogLuvEncoding] = TextureEncodingComponent{ "LogLuv", "( value )" };
}

ProgramGenerator& ProgramGenerator::instance()
{
	static ProgramGenerator instance;
	return instance;
}

void ProgramGenerator::Destory()
{
	_cachePrograms.clear();
}

void ProgramGenerator::getParameters(Material* material, osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{

	double left, right, top, bottom, near, far;
	if (camera->getProjectionMatrixAsOrtho(left, right, bottom, top, near, far))
		parameters.isOrthographic = true;

	//
	RenderState* renderState = RenderState::FromCamera(camera);
	if (renderState)
	{
		parameters.glslversion = renderState->getCapabilities().glslversion;
		parameters.precision = renderState->getCapabilities().precision;
		parameters.supportsVertexTextures = renderState->getCapabilities().supportsVertexTextures;

		parameters.outputEncoding = renderState->getOutputEncoding();
		parameters.gammaFactor = renderState->getGammaFactor();
		parameters.toneMapping = renderState->getToneMapping();

		parameters.numDirLights = renderState->getLightNumOfType(LightType_Direction);
		parameters.numSpotLights = renderState->getLightNumOfType(LightType_Spot);
		parameters.numRectAreaLights = renderState->getLightNumOfType(LightType_RectArea);
		parameters.numPointLights = renderState->getLightNumOfType(LightType_Point);
		parameters.numHemiLights = renderState->getLightNumOfType(LightType_Hemisphere);

		parameters.numDirLightShadows = renderState->getShadowNumOfType(LightType_Direction);
		parameters.numSpotLightShadows = renderState->getShadowNumOfType(LightType_Spot);
		parameters.numPointLightShadows = renderState->getShadowNumOfType(LightType_Point);

		parameters.fog = renderState->getFog().valid();
		if (parameters.fog)
		{
			FogExp2* fogExp = dynamic_cast<FogExp2*>(renderState->getFog().get());
			if (fogExp)
			{
				parameters.fogExp2 = true;
			}
		}

		ShadowMap* shadowMap = renderState->getShadowMap();
		if (shadowMap)
		{
			parameters.shadowMapEnabled = shadowMap->isEnable();
			parameters.shadowMapType = shadowMap->getMapType();
		}

		parameters.physicallyCorrectLights = renderState->getPhysicallyCorrectLights();

		parameters.logarithmicDepthBuffer = renderState->getLogarithmicDepthBuffer();
		parameters.rendererExtensionFragDepth = parameters.logarithmicDepthBuffer;

		parameters.numClippingPlanes = 0;
		parameters.numClipIntersection = 0;

		if (renderState->getBackgroudEnv())
		{
			renderState->getBackgroudEnv()->getProgramParameters(camera, cv, parameters);
		}
	}
	else//error
	{

	}
	//
	material->getProgramParameters(camera, cv, parameters);

	parameters.vertexUvs = parameters.map || parameters.bumpMap || parameters.normalMap || parameters.specularMap
		|| parameters.alphaMap || parameters.emissiveMap || parameters.roughnessMap || parameters.metalnessMap || parameters.clearcoatMap
		|| parameters.clearcoatRoughnessMap || parameters.clearcoatNormalMap || parameters.displacementMap;

	parameters.uvsVertexOnly = (parameters.map || parameters.bumpMap || parameters.normalMap || parameters.specularMap
		|| parameters.alphaMap || parameters.emissiveMap || parameters.roughnessMap || parameters.metalnessMap ||
		parameters.clearcoatNormalMap) && parameters.displacementMap;
}

std::string ProgramGenerator::getKey(const ProgramParameters& parameters)
{
	///TODO: make key uniquely
	std::stringstream ss;

	ss << parameters.shaderId.c_str();

	for (DefineMap::const_iterator iter = parameters.defines.begin(); iter != parameters.defines.end(); iter++)
	{
		ss << iter->first.c_str() << iter->second.c_str();
	}

	ss << parameters.envMap << parameters.envMapMode << parameters.envMapEncoding;

	ss << parameters.alphaTest;

	ss << parameters.toneMapping;

	ss << parameters.map << parameters.emissiveMap << parameters.displacementMap << parameters.specularMap;

	ss << parameters.normalMap << parameters.aoMap << parameters.roughnessMap << parameters.metalnessMap;

	ss << parameters.clearcoatMap << parameters.clearcoatNormalMap << parameters.clearcoatRoughnessMap;

	ss << parameters.skinning << parameters.morphTargets << parameters.morphNormals << parameters.maxBones;

	ss << parameters.instancing;

	ss << parameters.sheen;

	ss << parameters.shadowMapEnabled << parameters.shadowMapType;

	ss << parameters.numDirLightShadows << parameters.numSpotLightShadows << parameters.numPointLightShadows;

	ss << parameters.numClippingPlanes << parameters.numClipIntersection;

	ss << parameters.numDirLights << parameters.numPointLights << parameters.numSpotLights << parameters.numRectAreaLights << parameters.numHemiLights;

	ss << parameters.vertex << parameters.fragment;

	return ss.str();
}

osg::ref_ptr<Program> ProgramGenerator::getOrCreateProgram(const std::string& cacheKey, const ProgramParameters& parameters)
{
	ProgramMap::iterator iter = _cachePrograms.find(cacheKey);
	if (iter != _cachePrograms.end())
	{
		return iter->second;
	}
	osg::ref_ptr<Program> program = new Program(cacheKey, parameters);
	_cachePrograms[cacheKey] = program;
	return program;
}

TextureEncodingComponent* ProgramGenerator::getTextureEncodingComponent(TextureEncodingType type)
{
	TextureEncodingComponentMap::iterator iter = _textureEncodingMap.find(type);
	if (iter != _textureEncodingMap.end())
	{
		return &(iter->second);
	}
	return NULL;
}