//== INCLUDES =================================================================

#include <fstream>
#include <limits>

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/writer/VTKWriter.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>

//=== NAMESPACES ==============================================================

namespace OpenMesh {
namespace IO {

//=== INSTANTIATE =============================================================

_VTKWriter_  __VTKWriterinstance;
_VTKWriter_& VTKWriter() { return __VTKWriterinstance; }

//=== IMPLEMENTATION ==========================================================

_VTKWriter_::_VTKWriter_() { IOManager().register_module(this); }

//-----------------------------------------------------------------------------

bool _VTKWriter_::write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
    std::ofstream out;
    openWrite(_filename, _opt, out);

    if (!out) {
        omerr() << "[VTKWriter] : cannot open file " << _filename << std::endl;
        return false;
    }

    return write(out, _be, _opt, _precision);
}

//-----------------------------------------------------------------------------

bool _VTKWriter_::write(std::ostream& _out, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
    Vec3f v, n;
    Vec2f t;
    VertexHandle vh;
    OpenMesh::Vec3f c;
    OpenMesh::Vec4f cA;

    // check exporter features
    if (!check(_be, _opt)) {
        return false;
    }

    // check writer features
    if (!_opt.is_empty()) {
        omlog() << "[VTKWriter] : writer does not support any options\n";
        return false;
    }

    omlog() << "[VTKWriter] : write file\n";
    _out.precision(_precision);

    std::vector<VertexHandle> vhandles;
    size_t polygon_table_size = 0;
    size_t nf = _be.n_faces();
    for (size_t i = 0; i < nf; ++i) {
        polygon_table_size += _be.get_vhandles(FaceHandle(int(i)), vhandles);
    }
    polygon_table_size += nf;

    // header
    _out << "# vtk DataFile Version 3.0\n";
    _out << "Generated by OpenMesh\n";
    _out << "ASCII\n";
    _out << "DATASET POLYDATA\n";

    // points
    _out << "POINTS " << _be.n_vertices() << " float\n";
    size_t nv = _be.n_vertices();
    for (size_t i = 0; i < nv; ++i) {
        Vec3f v = _be.point(VertexHandle(int(i)));
        _out << v[0] << ' ' << v[1] << ' ' << v[2] << std::endl;
    }

    // faces
    _out << "POLYGONS " << nf << ' ' << polygon_table_size << std::endl;
    for (size_t i = 0; i < nf; ++i) {
        _be.get_vhandles(FaceHandle(int(i)), vhandles);

        _out << vhandles.size() << ' ';
        for (size_t j = 0; j < vhandles.size(); ++j) {
            _out << " " << vhandles[j].idx();
        }
        _out << std::endl;
    }

    return true;
}

//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
