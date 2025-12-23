#include <osg/TextureCubeMap>
#include <osgThreeJSX/Material>
#include <osgThreeJSX/MaterialData>
#include <osgThreeJSX/Materials>
#include <osgThreeJSX/Programs>
#include <osgThreeJSX/ShaderLib>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/PointLight>

using namespace osgThreeJSX;

MaterialBasic::MaterialBasic()
{

}

MaterialBasic::MaterialBasic(const MaterialBasic& other, const osg::CopyOp& copyop)
{

}

MaterialBasic::~MaterialBasic()
{

}

void MaterialBasic::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
	_common.buildUniformAndTexture(this, uniforms, textures);
	_light.buildUniformAndTexture(this, uniforms, textures);
	_ao.buildUniformAndTexture(this, uniforms, textures);
	_specular.buildUniformAndTexture(this, uniforms, textures);
	_env.buildUniformAndTexture(this, uniforms, textures);
}

void MaterialBasic::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	_common.getProgramParameters(camera, cv, parameters);
	_light.getProgramParameters(camera, cv, parameters);
	_ao.getProgramParameters(camera, cv, parameters);
	_specular.getProgramParameters(camera, cv, parameters);
	_env.getProgramParameters(camera, cv, parameters);
}

MaterialLambert::MaterialLambert()
{

}

MaterialLambert::MaterialLambert(const MaterialLambert& other, const osg::CopyOp& copyop)
{

}

MaterialLambert::~MaterialLambert()
{

}

void MaterialLambert::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
	_common.buildUniformAndTexture(this, uniforms, textures);
	_light.buildUniformAndTexture(this, uniforms, textures);
	_ao.buildUniformAndTexture(this, uniforms, textures);
	_specular.buildUniformAndTexture(this, uniforms, textures);
	_env.buildUniformAndTexture(this, uniforms, textures);
	_emissive.buildUniformAndTexture(this, uniforms, textures);
}

void MaterialLambert::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	_common.getProgramParameters(camera, cv, parameters);
	_light.getProgramParameters(camera, cv, parameters);
	_ao.getProgramParameters(camera, cv, parameters);
	_specular.getProgramParameters(camera, cv, parameters);
	_env.getProgramParameters(camera, cv, parameters);
	_emissive.getProgramParameters(camera, cv, parameters);
}

MaterialPhong::MaterialPhong()
{

}

MaterialPhong::MaterialPhong(const MaterialPhong& other, const osg::CopyOp& copyop)
{

}

MaterialPhong::~MaterialPhong()
{

}

void MaterialPhong::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
	_common.buildUniformAndTexture(this, uniforms, textures);
	_light.buildUniformAndTexture(this, uniforms, textures);
	_ao.buildUniformAndTexture(this, uniforms, textures);
	_specular.buildUniformAndTexture(this, uniforms, textures);
	_env.buildUniformAndTexture(this, uniforms, textures);
	_emissive.buildUniformAndTexture(this, uniforms, textures);
	_bump.buildUniformAndTexture(this, uniforms, textures);
	_normal.buildUniformAndTexture(this, uniforms, textures);
	_displacement.buildUniformAndTexture(this, uniforms, textures);
	_shininess.buildUniformAndTexture(this, uniforms, textures);
}

void MaterialPhong::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	_common.getProgramParameters(camera, cv, parameters);
	_light.getProgramParameters(camera, cv, parameters);
	_ao.getProgramParameters(camera, cv, parameters);
	_specular.getProgramParameters(camera, cv, parameters);
	_env.getProgramParameters(camera, cv, parameters);
	_emissive.getProgramParameters(camera, cv, parameters);
	_bump.getProgramParameters(camera, cv, parameters);
	_normal.getProgramParameters(camera, cv, parameters);
	_shininess.getProgramParameters(camera, cv, parameters);
}

MaterialStandard::MaterialStandard()
{
}

MaterialStandard::MaterialStandard(const MaterialStandard& other, const osg::CopyOp& copyop)
{

}

MaterialStandard::~MaterialStandard()
{

}

void MaterialStandard::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	MaterialBasic::buildUniformAndTexture(uniforms, textures);
	_emissive.buildUniformAndTexture(this, uniforms, textures);
	_bump.buildUniformAndTexture(this, uniforms, textures);
	_normal.buildUniformAndTexture(this, uniforms, textures);
	_displacement.buildUniformAndTexture(this, uniforms, textures);
	_roughness.buildUniformAndTexture(this, uniforms, textures);
	_metalness.buildUniformAndTexture(this, uniforms, textures);

}

void MaterialStandard::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	MaterialBasic::getProgramParameters(camera, cv, parameters);
	_emissive.getProgramParameters(camera, cv, parameters);
	_bump.getProgramParameters(camera, cv, parameters);
	_normal.getProgramParameters(camera, cv, parameters);
	_displacement.getProgramParameters(camera, cv, parameters);
	_roughness.getProgramParameters(camera, cv, parameters);
	_metalness.getProgramParameters(camera, cv, parameters);
}


MaterialPhysical::MaterialPhysical()
{
	addDefine("PHYSICAL", "");
}

MaterialPhysical::MaterialPhysical(const MaterialPhysical& other, const osg::CopyOp& copyop)
{

}

MaterialPhysical::~MaterialPhysical()
{

}

void MaterialPhysical::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	MaterialStandard::buildUniformAndTexture(uniforms, textures);
	_physical.buildUniformAndTexture(this, uniforms, textures);

}

void MaterialPhysical::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	MaterialStandard::getProgramParameters(camera, cv, parameters);
	_physical.getProgramParameters(camera, cv, parameters);
}

MaterialDepth::MaterialDepth()
{
	_depthPacking = DepthPackingType_RGBADepthPacking;
}

MaterialDepth::MaterialDepth(const MaterialDepth& other, const osg::CopyOp& copyop)
{

}

MaterialDepth::~MaterialDepth()
{

}

void MaterialDepth::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
}

void MaterialDepth::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	parameters.depthPacking = _depthPacking;
}

MaterialDistance::MaterialDistance()
{
}

MaterialDistance::MaterialDistance(const MaterialDistance& other, const osg::CopyOp& copyop)
{

}

MaterialDistance::~MaterialDistance()
{

}

void MaterialDistance::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
	getOrCreateUniform(uniforms, "nearDistance", _nearDistance);
	getOrCreateUniform(uniforms, "farDistance", _farDistance);
	getOrCreateUniform(uniforms, "referencePosition", _referencePosition);
}

void MaterialDistance::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
}

MaterialCube::MaterialCube()
{
	_opacity = 1.0;
}

MaterialCube::MaterialCube(const MaterialCube& other, const osg::CopyOp& copyop)
{

}

MaterialCube::~MaterialCube()
{

}

void MaterialCube::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
	_env.buildUniformAndTexture(this, uniforms, textures);
	getOrCreateUniform(uniforms, "opacity", _opacity);
}


void MaterialCube::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	_env.getProgramParameters(camera, cv, parameters);
}

MaterialEquirect::MaterialEquirect()
{
	_opacity = 1.0;
}

MaterialEquirect::MaterialEquirect(const MaterialEquirect& other, const osg::CopyOp& copyop)
{

}

MaterialEquirect::~MaterialEquirect()
{

}

void MaterialEquirect::buildUniformAndTexture(MaterialUniformList& uniforms, MaterialTextureList& textures)
{
	Material::buildUniformAndTexture(uniforms, textures);
	_env.buildUniformAndTexture(this, uniforms, textures);
	getOrCreateUniform(uniforms, "opacity", _opacity);
}


void MaterialEquirect::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	_env.getProgramParameters(camera, cv, parameters);
	parameters.mapEncoding = parameters.envMapEncoding;
}

MaterialShader::MaterialShader()
{
}

MaterialShader::MaterialShader(const MaterialShader& other, const osg::CopyOp& copyop)
{

}

MaterialShader::~MaterialShader()
{

}

void MaterialShader::getProgramParameters(osg::Camera* camera, osgUtil::CullVisitor* cv, ProgramParameters& parameters)
{
	Material::getProgramParameters(camera, cv, parameters);
	parameters.vertex = getVertexShader();
	parameters.fragment = getFragmentShader();
}