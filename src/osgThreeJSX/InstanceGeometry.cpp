
#include <osgThreeJSX/InstanceGeometry>
#include <osg/Texture2D>
#include <osgUtil/CullVisitor>
#include <osgUtil/IntersectionVisitor>
#include <osgThreeJSX/RenderState>

using namespace osgThreeJSX;

InstanceGeometry::InstanceGeometry() : _instanceNum(0)
{
	setupInstanceData();

}

InstanceGeometry::InstanceGeometry(const InstanceGeometry& rth, const osg::CopyOp& copyop)
{
	///TODO:
}

InstanceGeometry::~InstanceGeometry()
{

}

void InstanceGeometry::traverse(osg::NodeVisitor& nv)
{
	if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
	{
		if (!_geometry)
			return;

		osgUtil::CullVisitor* cv = nv.asCullVisitor();
		int instanceTextureUnit = cv->getState()->getMaxTextureUnits() - 1;
		osg::StateSet* ss = _geometry->getOrCreateStateSet();
		if (!ss->getTextureAttribute(instanceTextureUnit, osg::StateAttribute::TEXTURE))
		{
			ss->addUniform(new osg::Uniform("instanceImage", instanceTextureUnit));
			ss->setTextureAttributeAndModes(instanceTextureUnit, _instanceTexture, osg::StateAttribute::ON);
		}

		CullSettingAutoRecover ar(cv, osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
		_geometry->setCullingActive(false);
		_geometry->accept(nv);
	}
	else if (nv.getVisitorType() == osg::NodeVisitor::INTERSECTION_VISITOR)
	{
		osgUtil::IntersectionVisitor* iv = dynamic_cast<osgUtil::IntersectionVisitor*>(nv.asIntersectionVisitor());
		if (iv == NULL)
		{
			return;
		}

		//for (int i = 0; i < _instanceNum; i++)
		//{
		//	osg::ref_ptr<osg::RefMatrix> refMatrix = new osg::RefMatrix(_instances[i]);
		//	iv->pushModelMatrix(refMatrix);

		//	_geometry->traverse(nv);

		//	iv->popModelMatrix();
		//}
		_geometry->traverse(nv);
	}
	else
	{
		osg::Node::traverse(nv);
	}
}

osg::BoundingSphere transformBoundingSphere(const osg::BoundingSphere& bs, const osg::Matrix& matrix)
{
	if (!bs.valid()) {
		return bs;
	}

	osg::Vec3 newCenter = osg::Vec3(bs.center()) * matrix;

	osg::Vec3 scale = matrix.getScale();
	float maxScale = osg::maximum(scale.x(), osg::maximum(scale.y(), scale.z()));

	float newRadius = bs.radius() * maxScale;
	return osg::BoundingSphere(newCenter, newRadius);
}

osg::BoundingSphere InstanceGeometry::computeBound() const
{
	osg::BoundingSphere bs;
	if (_geometry)
	{
		for (unsigned int i = 0; i < _instanceNum; i++)
		{
			osg::BoundingSphere bsModel = _geometry->getBound();
			osg::Matrix mat = getInstanceMatrix(i);
			bsModel = transformBoundingSphere(bsModel, mat);
			bs.expandBy(bsModel);
		}
	}
	return bs;
}

osg::Matrix InstanceGeometry::getInstanceMatrix(unsigned int idx) const
{
	osg::Matrixf mat;
	if (idx >= _instanceNum)
		return mat;

	float* imageData = reinterpret_cast<float*>(_instanceImage->data());
	memcpy(mat.ptr(), imageData + idx * 16, sizeof(float) * 16);
	return mat;
}

void InstanceGeometry::setupInstanceData()
{
	_instanceImage = new osg::Image();
	int initSize = 4;
	float* data = new float[initSize * 16];
	_instanceImage->setImage(4, initSize, 1, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT, (unsigned char*)data, osg::Image::USE_NEW_DELETE);
	_instanceImage->setDataVariance(osg::Object::DYNAMIC);
	_instanceImage->setAllocationMode(osg::Image::NO_DELETE);

	osg::Texture2D* tex = new osg::Texture2D();
	tex->setImage(_instanceImage);
	tex->setResizeNonPowerOfTwoHint(false);
	tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
	tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
	_instanceTexture = tex;
}

void InstanceGeometry::addInstance(const osg::Matrix& mat)
{
	unsigned int sizeOfOne = sizeof(float) * 16;

	unsigned int bytes = _instanceImage->getImageSizeInBytes();
	unsigned int capacity = bytes / sizeOfOne;
	if (_instanceNum == capacity)
	{
		unsigned int newCapacity = capacity * 2;
		unsigned char* oldData = _instanceImage->data();
		unsigned char* newData = new unsigned char[newCapacity * sizeOfOne];
		memcpy(newData, oldData, bytes);
		_instanceImage->setImage(4, newCapacity, 1, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT, (unsigned char*)newData, osg::Image::USE_NEW_DELETE);
	}

	osg::Matrixf fMat(mat);
	char* imageData = reinterpret_cast<char*>(_instanceImage->data());
	memcpy(imageData + _instanceNum * sizeOfOne, fMat.ptr(), sizeOfOne);
	_instanceImage->dirty();
	_instanceNum += 1;

	setPrimitiveSetNum();

	dirtyBound();
}

void InstanceGeometry::setGeometry(osg::Geometry* geometry)
{
	_geometry = geometry;

	setPrimitiveSetNum();
}

void InstanceGeometry::setPrimitiveSetNum()
{
	if (!_geometry)
		return;

	for (int i = 0; i < _geometry->getNumPrimitiveSets(); i++)
	{
		_geometry->getPrimitiveSet(i)->setNumInstances(_instanceNum);
	}
}