/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */

/*===========================================================================*\
 *                                                                           *
 *   $Revision: 1258 $                                                         *
 *   $Date: 2015-04-28 15:07:46 +0200 (Di, 28 Apr 2015) $                   *
 *                                                                           *
\*===========================================================================*/


//== INCLUDES =================================================================


//STL
#include <fstream>
#include <limits>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/writer/OBJWriter.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


// register the OBJLoader singleton with MeshLoader
_OBJWriter_  __OBJWriterinstance;
_OBJWriter_& OBJWriter() { return __OBJWriterinstance; }


//=== IMPLEMENTATION ==========================================================


_OBJWriter_::_OBJWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_OBJWriter_::
write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  std::ofstream out;
  openWrite(_filename, _opt, out);

  if (!out)
  {
    omerr() << "[OBJWriter] : cannot open file "
	  << _filename << std::endl;
    return false;
  }

  out.precision(_precision);

  {
#if defined(WIN32)
    std::string::size_type dot = _filename.find_last_of("\\/");
#else
    std::string::size_type dot = _filename.rfind("/");
#endif

    if (dot == std::string::npos){
      path_ = "./";
      objName_ = _filename;
    }else{
      path_ = _filename.substr(0,dot+1);
      objName_ = _filename.substr(dot+1);
    }

    //remove the file extension
    dot = objName_.find_last_of(".");

    if(dot != std::string::npos)
      objName_ = objName_.substr(0,dot);
  }

  bool result = write(out, _be, _opt, _precision);

  out.close();
  return result;
}

//-----------------------------------------------------------------------------

size_t _OBJWriter_::getMaterial(OpenMesh::Vec3f _color) const
{
  for (size_t i=0; i < material_.size(); i++)
    if(material_[i] == _color)
      return i;

  //not found add new material
  material_.push_back( _color );
  return material_.size()-1;
}

//-----------------------------------------------------------------------------

size_t _OBJWriter_::getMaterial(OpenMesh::Vec4f _color) const
{
  for (size_t i=0; i < materialA_.size(); i++)
    if(materialA_[i] == _color)
      return i;

  //not found add new material
  materialA_.push_back( _color );
  return materialA_.size()-1;
}

//-----------------------------------------------------------------------------

bool
_OBJWriter_::
writeMaterial(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  OpenMesh::Vec3f c;
  OpenMesh::Vec4f cA;

  material_.clear();
  materialA_.clear();
  if ( _opt.face_has_color() ) {
    //iterate over faces
    for (size_t i=0, nF=_be.n_faces(); i<nF; ++i)
    {
      //color with alpha
      if ( _opt.color_has_alpha() ){
        cA  = color_cast<OpenMesh::Vec4f> (_be.colorA( FaceHandle(int(i)) ));
        getMaterial(cA);
      }else{
      //and without alpha
        c  = color_cast<OpenMesh::Vec3f> (_be.color( FaceHandle(int(i)) ));
        getMaterial(c);
      }
    }

    //write the materials
    if ( _opt.color_has_alpha() ) {
      for (size_t i=0; i < materialA_.size(); i++){
        _out << "newmtl " << "mat" << i << '\n';
        _out << "Ka 0.5000 0.5000 0.5000" << '\n';
        _out << "Kd " << materialA_[i][0] << materialA_[i][1] << materialA_[i][2] << '\n';
        _out << "Tr " << materialA_[i][3] << '\n';
        _out << "illum 1" << '\n';
      }
    } else {
      for (size_t i=0; i < material_.size(); i++){
        _out << "newmtl " << "mat" << i << '\n';
        _out << "Ka 0.5000 0.5000 0.5000" << '\n';
        _out << "Kd " << material_[i][0] << material_[i][1] << material_[i][2] << '\n';
        _out << "illum 1" << '\n';
      }
    }
  }

  if ( _opt.mesh_has_texfile() ) {
    _out << "newmtl " << "texture" << std::endl;
    _out << "illum 0" << std::endl;
    _out << "map_Kd " << _be.texfile() << std::endl;
  }

  return true;
}

//-----------------------------------------------------------------------------


bool
_OBJWriter_::
write(std::ostream& _out, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  unsigned int idx;
  size_t i, j, k, nV, nF;
  Vec3f v, n;
  Vec2f t;
  std::vector<Vec2f> texcoords;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;
  bool useMaterial = false;
  std::string mtlFileName;
  OpenMesh::Vec3f c;
  OpenMesh::Vec4f cA;
  VertexTexcoordMap vtm;
  std::vector<int> faceTexcoordIdx;
  int nextTexcoordIdx;

  omlog() << "[OBJWriter] : write file\n";

  _out.precision(_precision);

  // check exporter features
  if (!check( _be, _opt))
     return false;


  // check writer features
  if ( _opt.check(Options::Binary)     || // not supported by format
       _opt.check(Options::FaceNormal))
     return false;


  //create material file if needed
  if ( _opt.face_has_color() || _opt.mesh_has_texfile() ) {
    useMaterial = true;

    mtlFileName = objName_ + ".mtl";
    std::string mtlFilePath = path_ + mtlFileName;

    std::ofstream mtlStream;
    openWrite(mtlFilePath, _opt, mtlStream);

    if (!mtlStream)
    {
      omerr() << "[OBJWriter] : cannot write material file " << mtlFilePath << std::endl;
      return false;
    } else {
      writeMaterial(mtlStream, _be, _opt);
      mtlStream.close();
    }
  }

  // header
  _out << "# " << _be.n_vertices() << " vertices, ";
  _out << _be.n_faces() << " faces" << '\n';

  // material file
  if ( useMaterial )
    _out << "mtllib " << mtlFileName << '\n';

  // vertex data (point, normals, texcoords)
  for (i=0, nV=_be.n_vertices(); i<nV; ++i)
  {
    vh = VertexHandle(int(i));
    v  = _be.point(vh);
    n  = _be.normal(vh);
    t  = _be.texcoord(vh);

    _out << "v " << v[0] <<" "<< v[1] <<" "<< v[2] << '\n';

    if (_opt.check(Options::VertexNormal))
      _out << "vn " << n[0] <<" "<< n[1] <<" "<< n[2] << '\n';

    if (_opt.check(Options::VertexTexCoord))
      _out << "vt " << t[0] <<" "<< t[1] << '\n';
  }

  size_t lastMat = std::numeric_limits<std::size_t>::max();

  if ( _opt.face_has_texcoord() ) {
    nextTexcoordIdx = 1;
    vtm.resize(_be.n_vertices());
    _out << "usemtl texture" << '\n';
  }

  // we do not want to write seperators if we only write vertex indices
  bool onlyVertices =    !_opt.vertex_has_texcoord()
                      && !_opt.face_has_texcoord()
                      && !_opt.vertex_has_normal();

  // faces (indices starting at 1 not 0)
  for (i=0, nF=_be.n_faces(); i<nF; ++i)
  {

    if ( _opt.face_has_color() ){
      size_t material = std::numeric_limits<std::size_t>::max();

      //color with alpha
      if ( _opt.color_has_alpha() ){
        cA  = color_cast<OpenMesh::Vec4f> (_be.colorA( FaceHandle(int(i)) ));
        material = getMaterial(cA);
      } else{
      //and without alpha
        c  = color_cast<OpenMesh::Vec3f> (_be.color( FaceHandle(int(i)) ));
        material = getMaterial(c);
      }

      // if we are ina a new material block, specify in the file which material to use
      if(lastMat != material) {
        _out << "usemtl mat" << material << '\n';
        lastMat = material;
      }
    }

    _be.get_vhandles(FaceHandle(int(i)), vhandles);

    if ( _opt.face_has_texcoord() ) {
      faceTexcoordIdx.clear();
      _be.texcoords(FaceHandle(int(i)), texcoords);
      for (j=0; j<3; ++j)
      {
        // list of all previously encountered texcoords for current vertex
        VertexTexcoords& vertexTexcoords = vtm[vhandles[j].idx()];

        // find texcoord in list for this vertex
        bool found = false;
        for (k=0; k<vertexTexcoords.size(); ++k)
        {
          if (vertexTexcoords[k].t_ == texcoords[j])
          {
            found = true;
            faceTexcoordIdx.push_back(vertexTexcoords[k].idx_);
            break;
          }
        }
        if (!found)
        {
          _out << "vt " << texcoords[j][0] << " " << texcoords[j][1] << '\n';
          vertexTexcoords.push_back(VertexTexcoord(nextTexcoordIdx, texcoords[j]));
          faceTexcoordIdx.push_back(nextTexcoordIdx);
          ++nextTexcoordIdx;
        }
      }
    }

    _out << "f";

    for (j=0; j< vhandles.size(); ++j)
    {

      // Write vertex index
      idx = vhandles[j].idx() + 1;
      _out << " " << idx;

      if (!onlyVertices) {
        // write separator
        _out << "/" ;

        // write vertex texture coordinate index
        if ( _opt.vertex_has_texcoord() )
          _out  << idx;

        // write halfedge texture coordinate index
        if ( _opt.face_has_texcoord() )
          _out  << faceTexcoordIdx[j];

        // write vertex normal index
        if ( _opt.vertex_has_normal() ) {
          // write separator
          _out << "/" ;
          _out << idx;
        }
      }
    }

    _out << '\n';
  }

  material_.clear();
  materialA_.clear();

  return true;
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
