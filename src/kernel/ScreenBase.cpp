#include <kernel/ScreenBase.h>
#include <kernel/CVRViewer.h>
#include <kernel/CVRCullVisitor.h>
#include <input/TrackingManager.h>
#include <kernel/NodeMask.h>
#include <config/ConfigManager.h>

#include <osgViewer/Renderer>

#include <iostream>

using namespace cvr;

#ifdef WIN32
#define M_PI 3.141592653589793238462643
#endif

ScreenBase::ScreenBase()
{
    _headMatListInit = false;
}

void ScreenBase::defaultCameraInit(osg::Camera * cam)
{
    cam->setGraphicsContext(_myInfo->myChannel->myWindow->gc);
    cam->setViewport(
            new osg::Viewport(_myInfo->myChannel->left,
                    _myInfo->myChannel->bottom,_myInfo->myChannel->width,
                    _myInfo->myChannel->height));
    GLenum buffer =
            _myInfo->myChannel->myWindow->gc->getTraits()->doubleBuffer ?
                    GL_BACK : GL_FRONT;

    cam->setDrawBuffer(buffer);
    cam->setReadBuffer(buffer);

    cam->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

    cam->setCullMask(CULL_MASK);
    cam->setCullMaskLeft(CULL_MASK_LEFT);
    cam->setCullMaskRight(CULL_MASK_RIGHT);

    std::string cmode = ConfigManager::getEntry("value","CullingMode","CALVR");

    osgViewer::Renderer * renderer =
            dynamic_cast<osgViewer::Renderer*>(cam->getRenderer());
    if(!renderer)
    {
        std::cerr << "Error getting renderer pointer." << std::endl;
    }
    else
    {

        renderer->getSceneView(0)->getDisplaySettings()->setSerializeDrawDispatch(
                false);
        renderer->getSceneView(1)->getDisplaySettings()->setSerializeDrawDispatch(
                false);

        if(cmode == "CALVR")
        {
            renderer->getSceneView(0)->setCullVisitor(new CVRCullVisitor());
            renderer->getSceneView(0)->setCullVisitorLeft(new CVRCullVisitor());
            renderer->getSceneView(0)->setCullVisitorRight(
                    new CVRCullVisitor());
            renderer->getSceneView(1)->setCullVisitor(new CVRCullVisitor());
            renderer->getSceneView(1)->setCullVisitorLeft(new CVRCullVisitor());
            renderer->getSceneView(1)->setCullVisitorRight(
                    new CVRCullVisitor());
        }
    }
}

osg::Matrix & ScreenBase::getCurrentHeadMatrix(int head)
{
    return TrackingManager::instance()->getHeadMat(head);
}

osg::Vec3d ScreenBase::defaultLeftEye(int head)
{
    return osg::Vec3d(-_separation * _eyeSepMult / 2.0,0.0,0.0)
            * getCurrentHeadMatrix(head);
}

osg::Vec3d ScreenBase::defaultRightEye(int head)
{
    return osg::Vec3d(_separation * _eyeSepMult / 2.0,0.0,0.0)
            * getCurrentHeadMatrix(head);
}

void ScreenBase::computeDefaultViewProj(osg::Vec3d eyePos, osg::Matrix & view,
        osg::Matrix & proj)
{
    //translate screen to origin
    osg::Matrix screenTrans;
    screenTrans.makeTranslate(-_myInfo->xyz);

    //rotate screen to xz
    osg::Matrix screenRot;
    screenRot.makeRotate(-_myInfo->h * M_PI / 180.0,osg::Vec3(0,0,1),
            -_myInfo->p * M_PI / 180.0,osg::Vec3(1,0,0),
            -_myInfo->r * M_PI / 180.0,osg::Vec3(0,1,0));

    eyePos = eyePos * screenTrans * screenRot;

    //make frustum
    float top, bottom, left, right;
    float screenDist = -eyePos.y();

    top = _near * (_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    bottom = _near * (-_myInfo->height / 2.0 - eyePos.z()) / screenDist;
    right = _near * (_myInfo->width / 2.0 - eyePos.x()) / screenDist;
    left = _near * (-_myInfo->width / 2.0 - eyePos.x()) / screenDist;

    proj.makeFrustum(left,right,bottom,top,_near,_far);

    // move camera to origin
    osg::Matrix cameraTrans;
    cameraTrans.makeTranslate(-eyePos);

    //make view
    view = screenTrans * screenRot * cameraTrans
            * osg::Matrix::lookAt(osg::Vec3(0,0,0),osg::Vec3(0,1,0),
                    osg::Vec3(0,0,1));
}
