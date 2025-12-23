#include <regex>
#include <string>
#include <sstream>
#include <osg/Program>
#include <osgThreeJSX/Animation>
#include <osgAnimation/BoneMapVisitor>

using namespace osgThreeJSX;

//////////////////////////////////////////////////////////////////////////
MorphTransformMaterial::MorphTransformMaterial():_needInit(true)
{

}

MorphTransformMaterial::MorphTransformMaterial(const MorphTransformMaterial& rth, const osg::CopyOp& copyop)
{

}

void MorphTransformMaterial::operator()(osgAnimation::MorphGeometry& geom)
{
    if (_needInit)
    {
        if (!init(geom))
            return;
    }

	MaterialMorphGeometry* matGeom = dynamic_cast<MaterialMorphGeometry*>(&geom);	
	for (size_t idx = 0; idx < matGeom->getMorphTargetList().size(); idx++)
	{
		_uniTargetInflue->setElement(idx, matGeom->getMorphTarget(idx).getWeight());
	}
}

bool MorphTransformMaterial::init(osgAnimation::MorphGeometry& geom)
{
    MaterialMorphGeometry* matGeom = dynamic_cast<MaterialMorphGeometry*>(&geom);
    if (matGeom == NULL)
        return false;

    Material* material = matGeom->getMaterial().get();
    if (material == NULL)
        return false;

    material->setUniform("morphTargetBaseInfluence", 1.0f);

    _uniTargetInflue = new osg::Uniform(osg::Uniform::FLOAT, "morphTargetInfluences", 8);
    for (size_t idx = 0; idx < _uniTargetInflue->getNumElements(); idx++)
    {
        _uniTargetInflue->setElement(idx, 0.0f);
    }
    material->setUniform(_uniTargetInflue);

    material->dirty();
	_needInit = false;
    return true;
}

//////////////////////////////////////////////////////////////////////////
RigTransformMaterial::RigTransformMaterial():_needInit(true)
{

}

RigTransformMaterial::RigTransformMaterial(const RigTransformMaterial& rth, const osg::CopyOp& copyop)
{

}

bool RigTransformMaterial::init(osgAnimation::RigGeometry& geom)
{
    //if (_matrixPalette->size() != _bonePalette.size())
    //    return false;

    MaterialRigGeometry* matGeom = dynamic_cast<MaterialRigGeometry*>(&geom);
    if (matGeom == NULL)
        return false;

    Material* material = matGeom->getMaterial().get();
    if (material == NULL)
        return false;

    _uniformMatrixPalette = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "boneMatrices", _bonePalette.size());
    material->setUniform(_uniformMatrixPalette);

    _uniformBindMatrixInverse = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "bindMatrixInverse");
    material->setUniform(_uniformBindMatrixInverse);

    osg::Uniform* uniformBindMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "bindMatrix");
    uniformBindMatrix->set(osg::Matrix());
    material->setUniform(uniformBindMatrix);
    //material->setUniform("bindMatrix", osg::Matrix());
    material->dirty();

    _needInit = false;
    return true;
}

void RigTransformMaterial::operator()(osgAnimation::RigGeometry& geom)
{
    if (_needInit)
    {
        if (!init(geom))
            return;
    }

    MaterialRigGeometry* matGeom = dynamic_cast<MaterialRigGeometry*>(&geom);
    if (matGeom == NULL)
        return;

    osg::MatrixTransform* matTransform = dynamic_cast<osg::MatrixTransform*>(geom.getParent(0)->getParent(0));
    _uniformBindMatrixInverse->set(osg::Matrix::inverse(matTransform->getMatrix()));

    for (unsigned int i = 0; i < _bonePalette.size(); ++i)
    {
        const osg::ref_ptr<osgAnimation::Bone>& bone = _bonePalette[i].get();
        const osg::Matrixf& invBindMatrix = (*_matrixPalette)[i];

        const osg::Matrixf& boneMatrix = bone->getMatrixInSkeletonSpace();

        osg::MatrixList mtxList = bone->getWorldMatrices(geom.getSkeleton());
        osg::Matrixf result = invBindMatrix * mtxList[0];

        _uniformMatrixPalette->setElement(i, result);
    }
}

//////////////////////////////////////////////////////////////////////////
AnimationNode::AnimationNode()
{
    _animationMgr = new osgAnimation::BasicAnimationManager();
    this->addUpdateCallback(_animationMgr);
}

AnimationNode::AnimationNode(const AnimationNode& rth, const osg::CopyOp& copyop)
{
    _animationMgr = rth._animationMgr;
}

class FindNamedNodeVisitor : public osg::NodeVisitor
{
public:
	FindNamedNodeVisitor(const std::string& name) : _name(name),
		osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{

	}

	virtual void apply(osg::Node& node)
	{
		if (node.getName() == _name)
			_result = &node;

		traverse(node);
	}

	osg::ref_ptr<osg::Node> getResult() { return _result; }
private:
	std::string _name;
	osg::ref_ptr<osg::Node> _result;
};

void AnimationNode::getMorphGeometryByName(const std::string& name, std::vector<osgAnimation::MorphGeometry*>& geoms)
{
	geoms.clear();

	FindNamedNodeVisitor visitor(name);
	this->accept(visitor);

	auto matNode = visitor.getResult();
	if (matNode)
	{
		osg::MatrixTransform* matrixTransform = dynamic_cast<osg::MatrixTransform*>(matNode.get());
		if (matrixTransform && matrixTransform->getNumChildren() > 0)
		{
			osg::Geode* geode = dynamic_cast<osg::Geode*>(matrixTransform->getChild(0));
			if ((geode) && (geode->getNumChildren() > 0))
			{
				for (int i = 0; i < geode->getNumChildren(); i++)
				{
					geoms.push_back(dynamic_cast<osgAnimation::MorphGeometry*>(geode->getChild(i)));
				}
			}
		}
	}
}