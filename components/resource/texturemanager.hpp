#ifndef OPENMW_COMPONENTS_RESOURCE_TEXTUREMANAGER_H
#define OPENMW_COMPONENTS_RESOURCE_TEXTUREMANAGER_H

#include <string>
#include <map>

#include <osg/ref_ptr>
#include <osg/Image>
#include <osg/Texture2D>

namespace VFS
{
    class Manager;
}

namespace Resource
{

    /// @brief Handles loading/caching of Images and Texture StateAttributes.
    class TextureManager
    {
    public:
        TextureManager(const VFS::Manager* vfs);

        // TODO: texture filtering settings

        /// Create or retrieve a Texture2D using the specified image filename, and wrap parameters.
        osg::ref_ptr<osg::Texture2D> getTexture2D(const std::string& filename, osg::Texture::WrapMode wrapS, osg::Texture::WrapMode wrapT);

        /// Create or retrieve an Image
        //osg::ref_ptr<osg::Image> getImage(const std::string& filename);

        const VFS::Manager* getVFS() { return mVFS; }

    private:
        const VFS::Manager* mVFS;

        typedef std::pair<std::pair<int, int>, std::string> MapKey;

        std::map<std::string, osg::observer_ptr<osg::Image> > mImages;

        std::map<MapKey, osg::ref_ptr<osg::Texture2D> > mTextures;

    };

}

#endif
