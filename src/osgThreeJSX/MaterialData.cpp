#include <osgThreeJSX/MaterialData>
#include <osg/TextureCubeMap>
using namespace osgThreeJSX;

//////////////////////////////////////////////////////////////////////////
MaterialDataCommon::MaterialDataCommon():color(osg::Vec3(1.0, 1.0, 1.0)), opacity(1.0)
{
	uvTransform.makeIdentity();
	uv2Transform.makeIdentity();
}

void MaterialDataCommon::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.map = map.valid();
	parameters.mapEncoding = mapEncoding;

	parameters.alphaMap = alphaMap.valid();
}
//
void MaterialDataCommon::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	material->getOrCreateUniform(uniforms, "opacity", opacity);
	material->getOrCreateUniform(uniforms, "diffuse", color);		

	if (map.valid())
	{
		textures.push_back(MaterialTexture("map", map));
	}

	if (alphaMap.valid())
	{
		textures.push_back(MaterialTexture("alphaMap", alphaMap));
	}

	material->getOrCreateUniform(uniforms, "uvTransform", uvTransform);
	material->getOrCreateUniform(uniforms, "uv2Transform", uv2Transform);
}

void MaterialDataSpecular::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.specularMap = specularMap.valid();
}
//
void MaterialDataSpecular::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (specularMap.valid())
	{
		textures.push_back(MaterialTexture("specularMap", specularMap));
	}
	material->getOrCreateUniform(uniforms, "specular", specular);
}

void MaterialDataEnv::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	if (envMap.valid())
	{
		parameters.envMap = envMap.valid();
		parameters.envMapMode = envMapMode;
		parameters.envMapEncoding = envMapEncoding;		
	}
	parameters.combine = combine;
}
//
void MaterialDataEnv::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (envMap.valid())
	{
		textures.push_back(MaterialTexture("envMap", envMap));

		bool isCubeTexture = false;
		isCubeTexture = dynamic_cast<osg::TextureCubeMap*>(envMap.get()) != nullptr;
		material->getOrCreateUniform(uniforms, "flipEnvMap", isCubeTexture ? -1.0f : 1.0f);
	}

	material->getOrCreateUniform(uniforms, "envMapIntensity", envMapIntensity);
	material->getOrCreateUniform(uniforms, "reflectivity", reflectivity);
	material->getOrCreateUniform(uniforms, "refractionRatio", refractionRatio);
	material->getOrCreateUniform(uniforms, "maxMipLevel", 0);
}

void MaterialDataEnv::setMap(const osg::ref_ptr<osg::Texture>& texture)
{
	if (dynamic_cast<osg::TextureCubeMap*>(texture.get()))
	{
		envMap = texture;
		envMapEncoding = TextureEncodingType_RGBDEncoding;
		envMapMode = EnvMapModeType_CubeReflectionMapping;
	}
	else
	{
		envMap = texture;
		envMapEncoding = TextureEncodingType_RGBDEncoding;
		envMapMode = EnvMapModeType_EquirectangularReflectionMapping;
	}
}

void MaterialDataAo::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.aoMap = aoMap.valid();
}
//
void MaterialDataAo::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (aoMap.valid())
	{
		textures.push_back(MaterialTexture("aoMap", aoMap));
		material->getOrCreateUniform(uniforms, "aoMapIntensity", aoMapIntensity);
	}
}

void MaterialDataLight::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.lightMap = lightMap.valid();
	parameters.lightMapEncoding = lightMapEncoding;
}
//
void MaterialDataLight::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (lightMap.valid())
	{
		textures.push_back(MaterialTexture("lightMap", lightMap));
		material->getOrCreateUniform(uniforms, "lightMapIntensity", lightMapIntensity);
	}
}

void MaterialDataMatcap::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.matcap = matcap.valid();
	parameters.matcapEncoding = matcapEncoding;
}
//
void MaterialDataMatcap::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (matcap.valid())
	{
		textures.push_back(MaterialTexture("matcap", matcap));
	}
}

void MaterialDataEmissive::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.emissiveMap = emissiveMap.valid();
	parameters.emissiveMapEncoding = emissiveMapEncoding;
}
//
void MaterialDataEmissive::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	material->getOrCreateUniform(uniforms, "emissive", emissive * emissiveIntensity);

	if (emissiveMap.valid())
	{
		textures.push_back(MaterialTexture("emissiveMap", emissiveMap));
	}
}

void MaterialDataBump::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.bumpMap = bumpMap.valid();
}
//
void MaterialDataBump::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (bumpMap.valid())
	{
		textures.push_back(MaterialTexture("bumpMap", bumpMap));

		auto scale = bumpScale;
		if (material->getSide() == MaterialSideType_BackSide)
			scale = -scale;
		material->getOrCreateUniform(uniforms, "bumpScale", scale);
	}
}

void MaterialDataNormal::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.normalMap = normalMap.valid();
	parameters.objectSpaceNormalMap = normalMapType == NormalMapType_ObjectSpaceNormalMap;
	parameters.tangentSpaceNormalMap = normalMapType == NormalMapType_TangentSpaceNormalMap;
}
//
void MaterialDataNormal::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (normalMap.valid())
	{
		textures.push_back(MaterialTexture("normalMap", normalMap));
		
		auto scale = normalScale;
		if (material->getSide() == MaterialSideType_BackSide)
			scale = osg::Vec2() - scale;
		material->getOrCreateUniform(uniforms, "normalScale", scale);
	}
}

void MaterialDataDisplacement::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.displacementMap = displacementMap.valid();
}
//
void MaterialDataDisplacement::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (displacementMap.valid())
	{
		textures.push_back(MaterialTexture("displacementMap", displacementMap));
		material->getOrCreateUniform(uniforms, "displacementScale", displacementScale);
		material->getOrCreateUniform(uniforms, "displacementBias", displacementBias);
	}
}

void MaterialDataPhysical::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.clearcoatMap = clearcoatMap.valid();
	parameters.clearcoatRoughnessMap = clearcoatRoughnessMap.valid();
	parameters.clearcoatNormalMap = clearcoatNormalMap.valid();

	parameters.sheen = sheenFlag;
}
//
void MaterialDataPhysical::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (clearcoatMap.valid())
	{
		textures.push_back(MaterialTexture("clearcoatMap", clearcoatMap));
	}
	if (clearcoatRoughnessMap.valid())
	{
		textures.push_back(MaterialTexture("clearcoatRoughnessMap", clearcoatRoughnessMap));
	}
	if (clearcoatNormalMap.valid())
	{
		textures.push_back(MaterialTexture("clearcoatNormalMap", clearcoatNormalMap));

		auto scale = clearcoatNormalScale;
		if (material->getSide() == MaterialSideType_BackSide)
			scale = osg::Vec2() - scale;
		material->getOrCreateUniform(uniforms, "clearcoatNormalScale", scale);
	}
	if (sheenFlag)
	{
		material->getOrCreateUniform(uniforms, "sheen", sheen);
	}
	material->getOrCreateUniform(uniforms, "clearcoat", clearcoat);
	material->getOrCreateUniform(uniforms, "clearcoatRoughness", clearcoatRoughness);
	material->getOrCreateUniform(uniforms, "reflectivity", reflectivity);
	material->getOrCreateUniform(uniforms, "transparency", transparency);
}

void MaterialDataRoughness::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.roughnessMap = roughnessMap.valid();
}
//
void MaterialDataRoughness::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (roughnessMap.valid())
	{
		textures.push_back(MaterialTexture("roughnessMap", roughnessMap));
	}
	material->getOrCreateUniform(uniforms, "roughness", roughness);
}

void MaterialDataMetalness::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	parameters.metalnessMap = metalnessMap.valid();
}
//
void MaterialDataMetalness::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	if (metalnessMap.valid())
	{
		textures.push_back(MaterialTexture("metalnessMap", metalnessMap));
	}
	material->getOrCreateUniform(uniforms, "metalness", metalness);
}

void MaterialDataShininess::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
}
//
void MaterialDataShininess::buildUniformAndTexture(Material* material, MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	material->getOrCreateUniform(uniforms, "shininess", shininess);
}
