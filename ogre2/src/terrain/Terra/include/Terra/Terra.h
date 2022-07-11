/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2021 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef _OgreTerra_H_
#define _OgreTerra_H_

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreShaderParams.h"

#include "Terra/TerrainCell.h"

namespace Ogre
{
    struct GridPoint
    {
        int32 x;
        int32 z;
    };

    struct GridDirection
    {
        int x;
        int z;
    };

    class ShadowMapper;

    /**
    @brief The Terra class
        Internally Terra operates in Y-up space so input and outputs may
        be converted to/from the correct spaces based on setting, unless
        explicitly stated to be always Y-up by documentation.
    */
    class Terra : public MovableObject
    {
        friend class TerrainCell;

        struct SavedState
        {
            RenderableArray m_renderables;
            size_t m_currentCell;
            Camera const *m_camera;
        };

        std::vector<float>          m_heightMap;
        uint32                      m_width;
        uint32                      m_depth; //PNG's Height
        float                       m_depthWidthRatio;
        float                       m_skirtSize;
        float                       m_invWidth;
        float                       m_invDepth;

        bool m_zUp;

        Vector2     m_xzDimensions;
        Vector2     m_xzInvDimensions;
        Vector2     m_xzRelativeSize; // m_xzDimensions / [m_width, m_height]
        float       m_height;
        Vector3     m_terrainOrigin;
        uint32      m_basePixelDimension;

        /// 0 is currently in use
        /// 1 is SavedState
        std::vector<TerrainCell>   m_terrainCells[2];
        /// 0 & 1 are for tmp use
        std::vector<TerrainCell*>  m_collectedCells[2];
        size_t                     m_currentCell;

        DescriptorSetTexture const  *m_descriptorSet;
        Ogre::TextureGpu*   m_heightMapTex;
        Ogre::TextureGpu*   m_normalMapTex;

        Vector3             m_prevLightDir;
        ShadowMapper        *m_shadowMapper;

        /// When rendering shadows we want to override the data calculated by update
        /// but only temporarily, for later restoring it.
        SavedState m_savedState;

        //Ogre stuff
        CompositorManager2      *m_compositorManager;
        Camera const            *m_camera;

        // GZ CUSTOMIZE BEGIN
        /// See IORM_SOLID_COLOR and IORM_SOLID_THERMAL_COLOR_TEXTURED
        Vector4 mSolidColor[2];
        /// See IORM_SOLID_COLOR and IORM_SOLID_THERMAL_COLOR_TEXTURED
        bool mSolidColorSet[2];
        // GZ CUSTOMIZE END

        /// Converts value from Y-up to whatever the user up vector is (see m_zUp)
        inline Vector3 fromYUp( Vector3 value ) const;
        /// Same as fromYUp, but preserves original sign. Needed when value is a scale
        inline Vector3 fromYUpSignPreserving( Vector3 value ) const;
        /// Converts value from user up vector to Y-up
        inline Vector3 toYUp( Vector3 value ) const;
        /// Same as toYUp, but preserves original sign. Needed when value is a scale
        inline Vector3 toYUpSignPreserving( Vector3 value ) const;

    public:
        uint32 mHlmsTerraIndex;

    protected:
        void createDescriptorSet(void);
        void destroyDescriptorSet(void);
        void destroyHeightmapTexture(void);

        /// Creates the Ogre texture based on the image data.
        /// Called by @see createHeightmap
        void createHeightmapTexture( const Image2 &image, const String &imageName );

        /// Calls createHeightmapTexture, loads image data to our CPU-side buffers
        void createHeightmap( Image2 &image, const String &imageName );

        void createNormalTexture(void);
        void destroyNormalTexture(void);

        ///	Automatically calculates the optimum skirt size (no gaps with
        /// lowest overdraw possible).
        ///	This is done by taking the heighest delta between two adjacent
        /// pixels in a 4x4 block.
        ///	This calculation may not be perfect, as the block search should
        /// get bigger for higher LODs.
        void calculateOptimumSkirtSize(void);

        inline GridPoint worldToGrid( const Vector3 &vPos ) const;
        inline Vector2 gridToWorld( const GridPoint &gPos ) const;

        bool isVisible( const GridPoint &gPos, const GridPoint &gSize ) const;

        void addRenderable( const GridPoint &gridPos, const GridPoint &cellSize, uint32 lodLevel );

        void optimizeCellsAndAdd(void);

    public:
        Terra( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *sceneManager,
               uint8 renderQueueId, CompositorManager2 *compositorManager, Camera *camera, bool zUp );
        ~Terra();

        /// How low should the skirt be. Normally you should let this value untouched and let
        /// calculateOptimumSkirtSize do its thing for best performance/quality ratio.
        ///
        /// However if your height values are unconventional (i.e. artificial, non-natural) and you
        /// need to look the terrain from the "outside" (rather than being inside the terrain),
        /// you may have to tweak this value manually.
        ///
        /// This value should be between min height and max height of the heightmap.
        ///
        /// A value of 0.0 will give you the biggest skirt and fix all skirt-related issues.
        /// Note however, this may have a *tremendous* GPU performance impact.
        void setCustomSkirtMinHeight( const float skirtMinHeight ) { m_skirtSize = skirtMinHeight; }
        float getCustomSkirtMinHeight( void ) const { return m_skirtSize; }

        // GZ CUSTOMIZE BEGIN
        /// \brief See IORM_SOLID_COLOR and IORM_SOLID_THERMAL_COLOR_TEXTURED
        /// Replaces renderable->setCustomRenderable(...) because
        /// a Terrain may have many renderables but the color is the same
        /// for all of them
        /// \param[in] _idx must in range [1; 2]
        /// \param[in] _idx _solidColor color to apply
        void SetSolidColor(size_t _idx, const Vector4 _solidColor);

        /// \brief See IORM_SOLID_COLOR and IORM_SOLID_THERMAL_COLOR_TEXTURED
        /// Retrieves the value set with SetSolidColor. Throws if unset.
        /// \param[in] _idx must in range [1; 2]
        /// \return Color value
        Vector4 SolidColor(size_t _idx) const;

        /// \brief See IORM_SOLID_COLOR and IORM_SOLID_THERMAL_COLOR_TEXTURED
        /// Checks whether a color has been set
        /// \param[in] _idx must in range [1; 2]
        /// \return True if color has been unset
        bool HasSolidColor(size_t _idx) const;

        /// \brief See IORM_SOLID_COLOR and IORM_SOLID_THERMAL_COLOR_TEXTURED
        /// Marks all SetSolidColor as unset so that SolidColor throws
        /// if used again without setting.
        void UnsetSolidColors();
        // GZ CUSTOMIZE END

        /** Must be called every frame so we can check the camera's position
            (passed in the constructor) and update our visible batches (and LODs)
            We also update the shadow map if the light direction changed.
        @param lightDir
            Light direction for computing the shadow map.
        @param lightEpsilon
            Epsilon to consider how different light must be from previous
            call to recompute the shadow map.
            Interesting values are in the range [0; 2], but any value is accepted.
        @par
            Large epsilons will reduce the frequency in which the light is updated,
            improving performance (e.g. only compute the shadow map when needed)
        @par
            Use an epsilon of <= 0 to force recalculation every frame. This is
            useful to prevent heterogeneity between frames (reduce stutter) if
            you intend to update the light slightly every frame.
        */
        void update( const Vector3 &lightDir, float lightEpsilon=1e-6f );

        void load( const String &texName, const Vector3 &center, const Vector3 &dimensions );
        void load( Image2 &image, Vector3 center, Vector3 dimensions,
                   const String &imageName = BLANKSTRING );

        /** Gets the interpolated height at the given location.
            If outside the bounds, it leaves the height untouched.
        @param vPos
            Y-up:
                [in] XZ position, Y for default height.
                [out] Y height, or default Y (from input) if outside terrain bounds.
            Z-up
                [in] XY position, Z for default height.
                [out] Z height, or default Z (from input) if outside terrain bounds.
        @return
            True if Y (or Z for Z-up) component was changed
        */
        bool getHeightAt( Vector3 &vPos ) const;

        /// load must already have been called.
        void setDatablock( HlmsDatablock *datablock );

        //MovableObject overloads
        const String& getMovableType(void) const;

        /// Swaps current state with a saved one. Useful for rendering shadow maps
        void _swapSavedState( void );

        const Camera* getCamera() const                 { return m_camera; }
        void setCamera( const Camera *camera )          { m_camera = camera; }

        bool isZUp( void ) const { return m_zUp; }

        const ShadowMapper* getShadowMapper(void) const { return m_shadowMapper; }

        const Ogre::DescriptorSetTexture* getDescriptorSetTexture(void) const { return m_descriptorSet; }

        Ogre::TextureGpu* getHeightMapTex(void) const   { return m_heightMapTex; }
        Ogre::TextureGpu* getNormalMapTex(void) const   { return m_normalMapTex; }
        TextureGpu* _getShadowMapTex(void) const;

        // These are always in Y-up space
        const Vector2& getXZDimensions(void) const      { return m_xzDimensions; }
        const Vector2& getXZInvDimensions(void) const   { return m_xzInvDimensions; }
        float getHeight(void) const                     { return m_height; }
        const Vector3& getTerrainOriginRaw( void ) const{ return m_terrainOrigin; }

        /// Return value is in client-space (i.e. could be y- or z-up)
        Vector3 getTerrainOrigin( void ) const;

        // Always in Y-up space
        Vector2 getTerrainXZCenter(void) const;
    };
}

#endif
