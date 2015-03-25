#include "object.hpp"

#include <stdexcept>

#include <osg/Group>
#include <osg/PositionAttitudeTransform>

#include "../../model/world/data.hpp"
#include "../../model/world/ref.hpp"
#include "../../model/world/refidcollection.hpp"

#include <components/nifosg/nifloader.hpp>

#include "elements.hpp"

void CSVRender::Object::clear()
{
}

void CSVRender::Object::update()
{
    clear();

    std::string model;
    int error = 0; // 1 referenceable does not exist, 2 referenceable does not specify a mesh

    const CSMWorld::RefIdCollection& referenceables = mData.getReferenceables();

    int index = referenceables.searchId (mReferenceableId);

    if (index==-1)
        error = 1;
    else
    {
        /// \todo check for Deleted state (error 1)

        model = referenceables.getData (index,
            referenceables.findColumnIndex (CSMWorld::Columns::ColumnId_Model)).
            toString().toUtf8().constData();

        if (model.empty())
            error = 2;
    }

    if (error)
    {
        /*
        Ogre::Entity* entity = mBase->getCreator()->createEntity (Ogre::SceneManager::PT_CUBE);
        entity->setMaterialName("BaseWhite"); /// \todo adjust material according to error
        entity->setVisibilityFlags (Element_Reference);

        mBase->attachObject (entity);
        */
    }
    else
    {
        try
        {
            NifOsg::Loader loader;
            loader.resourceManager = mVFS;

            std::string path = "meshes\\" + model;

            Nif::NIFFilePtr file(new Nif::NIFFile(mVFS->get(path), path));

            mBaseNode->removeChildren(0, mBaseNode->getNumChildren());
            mBaseNode->addChild(loader.load(file));
        }
        catch (std::exception& e)
        {
            // TODO: use error marker mesh
            std::cerr << e.what() << std::endl;
        }
    }
}

void CSVRender::Object::adjust()
{
    if (mReferenceId.empty())
        return;

    const CSMWorld::CellRef& reference = getReference();

    // position
    mBaseNode->setPosition(mForceBaseToZero ? osg::Vec3() : osg::Vec3f(reference.mPos.pos[0], reference.mPos.pos[1], reference.mPos.pos[2]));

    // orientation
    osg::Quat xr (-reference.mPos.rot[0], osg::Vec3f(1,0,0));
    osg::Quat yr (-reference.mPos.rot[1], osg::Vec3f(0,1,0));
    osg::Quat zr (-reference.mPos.rot[2], osg::Vec3f(0,0,1));
    mBaseNode->setAttitude(zr*yr*xr);

    mBaseNode->setScale(osg::Vec3(reference.mScale, reference.mScale, reference.mScale));
}

const CSMWorld::CellRef& CSVRender::Object::getReference() const
{
    if (mReferenceId.empty())
        throw std::logic_error ("object does not represent a reference");

    return mData.getReferences().getRecord (mReferenceId).get();
}

CSVRender::Object::Object (const CSMWorld::Data& data, osg::Group* parentNode,
    const std::string& id, bool referenceable, bool forceBaseToZero)
: mVFS(data.getVFS()), mData (data), mBaseNode(0), mParentNode(parentNode), mForceBaseToZero (forceBaseToZero)
{
    mBaseNode = new osg::PositionAttitudeTransform;
    parentNode->addChild(mBaseNode);

    // 0x1 reserved for separating cull and update visitors
    mBaseNode->setNodeMask(Element_Reference<<1);

    if (referenceable)
    {
        mReferenceableId = id;
    }
    else
    {
        mReferenceId = id;
        mReferenceableId = getReference().mRefID;
    }

    update();
    adjust();
}

CSVRender::Object::~Object()
{
    clear();

    mParentNode->removeChild(mBaseNode);
}

bool CSVRender::Object::referenceableDataChanged (const QModelIndex& topLeft,
    const QModelIndex& bottomRight)
{
    const CSMWorld::RefIdCollection& referenceables = mData.getReferenceables();

    int index = referenceables.searchId (mReferenceableId);

    if (index!=-1 && index>=topLeft.row() && index<=bottomRight.row())
    {
        update();
        adjust();
        return true;
    }

    return false;
}

bool CSVRender::Object::referenceableAboutToBeRemoved (const QModelIndex& parent, int start,
    int end)
{
    const CSMWorld::RefIdCollection& referenceables = mData.getReferenceables();

    int index = referenceables.searchId (mReferenceableId);

    if (index!=-1 && index>=start && index<=end)
    {
        // Deletion of referenceable-type objects is handled outside of Object.
        if (!mReferenceId.empty())
        {
            update();
            adjust();
            return true;
        }
    }

    return false;
}

bool CSVRender::Object::referenceDataChanged (const QModelIndex& topLeft,
    const QModelIndex& bottomRight)
{
    if (mReferenceId.empty())
        return false;

    const CSMWorld::RefCollection& references = mData.getReferences();

    int index = references.searchId (mReferenceId);

    if (index!=-1 && index>=topLeft.row() && index<=bottomRight.row())
    {
        int columnIndex =
            references.findColumnIndex (CSMWorld::Columns::ColumnId_ReferenceableId);

        if (columnIndex>=topLeft.column() && columnIndex<=bottomRight.row())
        {
            mReferenceableId =
                references.getData (index, columnIndex).toString().toUtf8().constData();

            update();
        }

        adjust();

        return true;
    }

    return false;
}

std::string CSVRender::Object::getReferenceId() const
{
    return mReferenceId;
}

std::string CSVRender::Object::getReferenceableId() const
{
    return mReferenceableId;
}
