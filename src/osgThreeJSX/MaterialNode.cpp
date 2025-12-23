#include <osgThreeJSX/Material>
#include <osgThreeJSX/MaterialNode>
#include <osgThreeJSX/Programs>
#include <osgThreeJSX/ShaderLib>
#include <osgThreeJSX/RenderState>
#include <osgThreeJSX/PointLight>

using namespace osgThreeJSX;

//////////////////////////////////////////////////////////////////////////
void MaterialNodeCullback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
	if (cv)
	{
		osg::Camera* camera = cv->getCurrentCamera();

		//
		RenderState* renderState = dynamic_cast<RenderState*>(camera->getUserData());
		if (renderState)
		{
			osg::ref_ptr<Material> material = _material;
			if (renderState->getShadowLight() && material.valid())//shadow render state
			{
				if (renderState->getShadowLight()->getShadow()->_mapVsm)//vsm
				{
					if ((!material->getCastShadow()) && (!material->getReceiveShadow()))
						return;
				}
				else
				{
					if (!material->getCastShadow())
						return;
				}

				material = _material->getOrCreateDepthMaterial(renderState->getShadowLight());
			}
			if (material.valid())
			{
				material->update(camera, cv, node);
				if (material->getStateset())
				{
					cv->pushStateSet(material->getStateset());
				}
				traverse(node, nv);
				if (material->getStateset())
				{
					cv->popStateSet();
				}
				return;
			}
		}
	}

	traverse(node, nv);
}
