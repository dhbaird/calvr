#include <menu/MenuManager.h>
#include <menu/MenuSystem.h>
#include <config/ConfigManager.h>
#include <input/TrackingManager.h>
#include <kernel/SceneManager.h>
#include <kernel/ComController.h>
#include <kernel/InteractionManager.h>
#include <kernel/NodeMask.h>
#include <util/Intersection.h>

using namespace cvr;

MenuManager * MenuManager::_myPtr = NULL;

MenuManager::MenuManager()
{
}

MenuManager::~MenuManager()
{
}

MenuManager * MenuManager::instance()
{
    if(!_myPtr)
    {
	_myPtr = new MenuManager();
    }
    return _myPtr;
}

bool MenuManager::init()
{
    MenuSystem * mainMenu = MenuSystem::instance();
    if(!mainMenu->init())
    {
	return false;
    }

    _menuSystemList.push_back(mainMenu);

    //_primaryHand = ConfigManager::getInt("MenuSystem.PrimaryHand",0);
    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
	_handLastMenuSystem.push_back(NULL);
    }
    return true;
}

void MenuManager::update()
{
    if(ComController::instance()->getIsSyncError())
    {
	return;
    }

    // call update on all menus
    //_mainMenu->updateStart();
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	(*it)->updateStart();
    }

    osgUtil::IntersectVisitor iv;
    iv.setTraversalMask(cvr::INTERSECT_MASK);

    // process intersection
    osg::Vec3 pointerStart, pointerEnd;

    std::vector<osg::ref_ptr<osg::LineSegment> > handsegs;
    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
	pointerStart = TrackingManager::instance()->getHandMat(i).getTrans();
	pointerEnd.set(0.0f, 10000.0f, 0.0f);
	pointerEnd = pointerEnd
	    * TrackingManager::instance()->getHandMat(i);
	handsegs.push_back(new osg::LineSegment());
	handsegs.back()->set(pointerStart,pointerEnd);
	iv.addLineSegment(handsegs.back().get());
    }
    
    SceneManager::instance()->getMenuRoot()->accept(iv);

    for(int i = 0; i < TrackingManager::instance()->getNumHands(); i++)
    {
	osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(handsegs[i].get());
	for(size_t j = 0; j < hitList.size(); j++)
	{
	    IsectInfo isect;
	    isect.point = hitList.at(j).getWorldIntersectPoint();
            isect.normal = hitList.at(j).getWorldIntersectNormal();
            isect.geode = hitList.at(j)._geode.get();
	    if(_handLastMenuSystem[i])
	    {
		if(_handLastMenuSystem[i]->processIsect(isect,i))
		{
		    break;
		}
	    }

	    bool found = false;
	    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
	    {
		if((*it) == _handLastMenuSystem[i])
		{
		    continue;
		}
		if((*it)->processIsect(isect,i))
		{
		    found = true;
		    break;
		}
	    }
	    if(found)
	    {
		break;
	    }
	}
    }

    updateEnd();

    /*std::vector<IsectInfo> isecvec;

    if(TrackingManager::instance()->getNumHands())
    {
	pointerStart = TrackingManager::instance()->getHandMat(_primaryHand).getTrans();
	pointerEnd.set(0.0f, 10000.0f, 0.0f);
	pointerEnd = pointerEnd
	    * TrackingManager::instance()->getHandMat(_primaryHand);

	isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
		pointerStart, pointerEnd);

	for(int i = 0; i < isecvec.size(); i++)
	{
	    if(processWithOrder(isecvec[i],false))
	    {
		updateEnd();
		return;
	    }
	}
    }

    // process mouse intersection
    pointerStart = InteractionManager::instance()->getMouseMat().getTrans();
    pointerEnd.set(0.0f, 10000.0f, 0.0f);
    pointerEnd = pointerEnd * InteractionManager::instance()->getMouseMat();

    isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
                                    pointerStart, pointerEnd);

    for(int i = 0; i < isecvec.size(); i++)
    {
	if(processWithOrder(isecvec[i],true))
	{
	    updateEnd();
	    return;
	}
    }

    updateEnd();
    //_mainMenu->updateEnd();*/
}

bool MenuManager::processEvent(InteractionEvent * event)
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it)->processEvent(event))
	{
	    return true;
	}
    }
    return false;
}

void MenuManager::addMenuSystem(MenuSystemBase * ms)
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it) == ms)
	{
	    return;
	}
    }
    _menuSystemList.push_back(ms);
}

void MenuManager::removeMenuSystem(MenuSystemBase * ms)
{
    for(int i = 0; i < _handLastMenuSystem.size(); i++)
    {
	if(_handLastMenuSystem[i] == ms)
	{
	    _handLastMenuSystem[i] = NULL;
	}
    }

    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it) == ms)
	{
	    _menuSystemList.erase(it);
	    return;
	}
    }
}

bool MenuManager::processWithOrder(IsectInfo & isect, bool mouse)
{
    bool used = false;
    MenuSystemBase * item = NULL;

    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	if((*it)->processIsect(isect,mouse))
	{
	    used = true;
	    item = (*it);
	    _menuSystemList.erase(it);
	    break;
	}
    }

    if(used)
    {
	_menuSystemList.push_front(item);
    }

    return used;
}

void MenuManager::updateEnd()
{
    for(std::list<MenuSystemBase*>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	(*it)->updateEnd();
    }
}

void MenuManager::itemDelete(MenuItem * item)
{
    for(std::list<MenuSystemBase *>::iterator it = _menuSystemList.begin(); it != _menuSystemList.end(); it++)
    {
	(*it)->itemDelete(item);
    }
}
