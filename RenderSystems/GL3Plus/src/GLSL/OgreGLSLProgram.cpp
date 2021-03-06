/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

  Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#include "OgreGLSLProgram.h"
#include "OgreGLSLShader.h"
#include "OgreGpuProgramManager.h"
#include "OgreGLSLShader.h"
#include "OgreRoot.h"

namespace Ogre {

    GLSLProgram::GLSLProgram(GLSLShader* vertexShader,
                             GLSLShader* hullShader,
                             GLSLShader* domainShader,
                             GLSLShader* geometryShader,
                             GLSLShader* fragmentShader,
                             GLSLShader* computeShader)
        : GLSLProgramCommon(vertexShader)
        , mHullShader(hullShader)
        , mDomainShader(domainShader)
        , mGeometryShader(geometryShader)
        , mFragmentShader(fragmentShader)
        , mComputeShader(computeShader)
    {
    }


    GLSLProgram::~GLSLProgram(void)
    {
        OGRE_CHECK_GL_ERROR(glDeleteProgram(mGLProgramHandle));
    }


    Ogre::String GLSLProgram::getCombinedName()
    {
        String name;
        if (mVertexShader)
        {
            name += "Vertex Shader: ";
            name += mVertexShader->getName();
            name += "\n";
        }
        if (mHullShader)
        {
            name += "Tessellation Control Shader: ";
            name += mHullShader->getName();
            name += "\n";
        }
        if (mDomainShader)
        {
            name += "Tessellation Evaluation Shader: ";
            name += mDomainShader->getName();
            name += "\n";
        }
        if (mGeometryShader)
        {
            name += "Geometry Shader: ";
            name += mGeometryShader->getName();
            name += "\n";
        }
        if (mFragmentShader)
        {
            name += "Fragment Shader: ";
            name += mFragmentShader->getName();
            name += "\n";
        }
        if (mComputeShader)
        {
            name += "Compute Shader: ";
            name += mComputeShader->getName();
            name += "\n";
        }

        return name;
    }

    void GLSLProgram::bindFixedAttributes(GLuint program)
    {
        GLint max_vertex_attribs = 16;
        glGetIntegerv( GL_MAX_VERTEX_ATTRIBS , &max_vertex_attribs);

        size_t numAttribs = sizeof(msCustomAttributes) / sizeof(CustomAttribute);
        for (size_t i = 0; i < numAttribs; ++i)
        {
            const CustomAttribute& a = msCustomAttributes[i];
            if (a.attrib < max_vertex_attribs)
            {

                OGRE_CHECK_GL_ERROR(glBindAttribLocation(program, a.attrib, a.name));
            }
        }
    }

    void GLSLProgram::getMicrocodeFromCache(void)
    {
        GpuProgramManager::Microcode cacheMicrocode =
            GpuProgramManager::getSingleton().getMicrocodeFromCache(getCombinedName());

        cacheMicrocode->seek(0);

        // Turns out we need this param when loading.
        GLenum binaryFormat = 0;
        cacheMicrocode->read(&binaryFormat, sizeof(GLenum));

        // Get size of binary.
        GLint binaryLength = static_cast<GLint>(cacheMicrocode->size() - sizeof(GLenum));

        // Load binary.
        OGRE_CHECK_GL_ERROR(glProgramBinary(mGLProgramHandle,
                                            binaryFormat,
                                            cacheMicrocode->getPtr(),
                                            binaryLength));

        GLint success = 0;
        OGRE_CHECK_GL_ERROR(glGetProgramiv(mGLProgramHandle, GL_LINK_STATUS, &success));
        if (!success)
        {
            // Something must have changed since the program binaries
            // were cached away. Fallback to source shader loading path,
            // and then retrieve and cache new program binaries once again.
            compileAndLink();
        }
    }

} // namespace Ogre
