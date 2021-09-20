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

#ifndef _OgreTerraShadowMapper_H_
#define _OgreTerraShadowMapper_H_

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreShaderParams.h"

#include "Terra/TerrainCell.h"

namespace Ogre
{
    typedef TextureGpu* CompositorChannel;

    class ShadowMapper
    {
        Ogre::TextureGpu    *m_heightMapTex;

        ConstBufferPacked   *m_shadowStarts;
        ConstBufferPacked   *m_shadowPerGroupData;
        CompositorWorkspace *m_shadowWorkspace;
        TextureGpu          *m_shadowMapTex;
        HlmsComputeJob      *m_shadowJob;
        ShaderParams::Param *m_jobParamDelta;
        ShaderParams::Param *m_jobParamXYStep;
        ShaderParams::Param *m_jobParamIsStep;
        ShaderParams::Param *m_jobParamHeightDelta;

        //Ogre stuff
        SceneManager            *m_sceneManager;
        CompositorManager2      *m_compositorManager;

        static inline size_t getStartsPtrCount( int32 *starts, int32 *startsBase );

        /** Gets how many steps are needed in Bresenham's algorithm to reach certain height,
            given its dx / dy ratio where:
                dx = abs( x1 - x0 );
                dy = abs( y1 - y0 );
            and Bresenham is drawn in ranges [x0; x1) and [y0; y1)
        @param y
            Height to reach
        @param fStep
            (dx * 0.5f) / dy;
        @return
            Number of X iterations needed to reach the the pixel at height 'y'
            The returned value is at position (retVal; y)
            which means (retVal-1; y-1) is true unless y = 0;
        */
        static inline int32 getXStepsNeededToReachY( uint32 y, float fStep );

        /** Calculates the value of the error at position x = xIterationsToSkip from
            Bresenham's algorithm.
        @remarks
            We use this function so we can start Bresenham from '0' but resuming as
            if we wouldn't be starting from 0.
        @param xIterationsToSkip
            The X position in which we want the error.
        @param dx
            delta.x
        @param dy
            delta.y
        @return
            The error at position (xIterationsToSkip; y)
        */
        static inline float getErrorAfterXsteps( uint32 xIterationsToSkip, float dx, float dy );

        static void setGaussianFilterParams( HlmsComputeJob *job, uint8 kernelRadius,
                                             float gaussianDeviationFactor=0.5f );

    public:
        ShadowMapper( SceneManager *sceneManager, CompositorManager2 *compositorManager );
        ~ShadowMapper();

        /** Sets the parameter of the gaussian filter we apply to the shadow map.
        @param kernelRadius
            Kernel radius. Must be an even number.
        @param gaussianDeviationFactor
            Expressed in terms of gaussianDeviation = kernelRadius * gaussianDeviationFactor
        */
        void setGaussianFilterParams( uint8 kernelRadius, float gaussianDeviationFactor=0.5f );

        void createShadowMap( IdType id, TextureGpu *heightMapTex );
        void destroyShadowMap(void);
        void updateShadowMap( const Vector3 &lightDir, const Vector2 &xzDimensions, float heightScale );

        void fillUavDataForCompositorChannel( TextureGpu **outChannel,
                                              ResourceLayoutMap &outInitialLayouts,
                                              ResourceAccessMap &outInitialUavAccess ) const;

        Ogre::TextureGpu* getShadowMapTex(void) const           { return m_shadowMapTex; }
    };
}

#endif
