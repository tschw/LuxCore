/***************************************************************************
 * Copyright 1998-2018 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxCoreRender.                                   *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <cstring>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "luxrays/core/exttrianglemesh.h"
#include "luxrays/utils/ply/rply.h"
#include "luxrays/utils/serializationutils.h"

using namespace std;
using namespace luxrays;

//------------------------------------------------------------------------------
// ExtMesh PLY reader
//------------------------------------------------------------------------------

// rply vertex callback
static int VertexCB(p_ply_argument argument) {
	long userIndex = 0;
	void *userData = NULL;
	ply_get_argument_user_data(argument, &userData, &userIndex);

	Point *p = *static_cast<Point **> (userData);

	long vertIndex;
	ply_get_argument_element(argument, NULL, &vertIndex);

	if (userIndex == 0)
		p[vertIndex].x =
			static_cast<float>(ply_get_argument_value(argument));
	else if (userIndex == 1)
		p[vertIndex].y =
			static_cast<float>(ply_get_argument_value(argument));
	else if (userIndex == 2)
		p[vertIndex].z =
			static_cast<float>(ply_get_argument_value(argument));

	return 1;
}

// rply normal callback
static int NormalCB(p_ply_argument argument) {
	long userIndex = 0;
	void *userData = NULL;

	ply_get_argument_user_data(argument, &userData, &userIndex);

	Normal *n = *static_cast<Normal **> (userData);

	long normIndex;
	ply_get_argument_element(argument, NULL, &normIndex);

	if (userIndex == 0)
		n[normIndex].x =
			static_cast<float>(ply_get_argument_value(argument));
	else if (userIndex == 1)
		n[normIndex].y =
			static_cast<float>(ply_get_argument_value(argument));
	else if (userIndex == 2)
		n[normIndex].z =
			static_cast<float>(ply_get_argument_value(argument));

	return 1;
}

// rply uv callback
static int UVCB(p_ply_argument argument) {
	long userIndex = 0;
	void *userData = NULL;
	ply_get_argument_user_data(argument, &userData, &userIndex);

	UV *uv = *static_cast<UV **> (userData);

	long uvIndex;
	ply_get_argument_element(argument, NULL, &uvIndex);

	if (userIndex == 0)
		uv[uvIndex].u =
			static_cast<float>(ply_get_argument_value(argument));
	else if (userIndex == 1)
		uv[uvIndex].v =
			static_cast<float>(ply_get_argument_value(argument));

	return 1;
}

// rply color callback
static int ColorCB(p_ply_argument argument) {
	long userIndex = 0;
	void *userData = NULL;
	ply_get_argument_user_data(argument, &userData, &userIndex);

	float *c = *static_cast<float **> (userData);

	long colIndex;
	ply_get_argument_element(argument, NULL, &colIndex);

	// Check the type of value used
	p_ply_property property = NULL;
	ply_get_argument_property(argument, &property, NULL, NULL);
	e_ply_type dataType;
	ply_get_property_info(property, NULL, &dataType, NULL, NULL);
	if (dataType == PLY_UCHAR) {
		if (userIndex == 0)
			c[colIndex * 3] =
				static_cast<float>(ply_get_argument_value(argument) / 255.0);
		else if (userIndex == 1)
			c[colIndex * 3 + 1] =
				static_cast<float>(ply_get_argument_value(argument) / 255.0);
		else if (userIndex == 2)
			c[colIndex * 3 + 2] =
				static_cast<float>(ply_get_argument_value(argument) / 255.0);
	} else {
		if (userIndex == 0)
			c[colIndex * 3] =
				static_cast<float>(ply_get_argument_value(argument));
		else if (userIndex == 1)
			c[colIndex * 3 + 1] =
				static_cast<float>(ply_get_argument_value(argument));
		else if (userIndex == 2)
			c[colIndex * 3 + 2] =
				static_cast<float>(ply_get_argument_value(argument));
	}

	return 1;
}

// rply vertex callback
static int AlphaCB(p_ply_argument argument) {
	long userIndex = 0;
	void *userData = NULL;
	ply_get_argument_user_data(argument, &userData, &userIndex);

	float *c = *static_cast<float **> (userData);

	long alphaIndex;
	ply_get_argument_element(argument, NULL, &alphaIndex);

	// Check the type of value used
	p_ply_property property = NULL;
	ply_get_argument_property(argument, &property, NULL, NULL);
	e_ply_type dataType;
	ply_get_property_info(property, NULL, &dataType, NULL, NULL);
	if (dataType == PLY_UCHAR) {
		if (userIndex == 0)
			c[alphaIndex] =
				static_cast<float>(ply_get_argument_value(argument) / 255.0);
	} else {
		if (userIndex == 0)
			c[alphaIndex] =
				static_cast<float>(ply_get_argument_value(argument));		
	}

	return 1;
}

// rply face callback
static int FaceCB(p_ply_argument argument) {
	void *userData = NULL;
	ply_get_argument_user_data(argument, &userData, NULL);

	vector<Triangle> *tris = static_cast<vector<Triangle> *> (userData);

	long length, valueIndex;
	ply_get_argument_property(argument, NULL, &length, &valueIndex);

	if (length == 3) {
		if (valueIndex < 0)
			tris->push_back(Triangle());
		else if (valueIndex < 3)
			tris->back().v[valueIndex] =
					static_cast<u_int> (ply_get_argument_value(argument));
	} else if (length == 4) {
		// I have to split the quad in 2x triangles
		if (valueIndex < 0) {
			tris->push_back(Triangle());
		} else if (valueIndex < 3)
			tris->back().v[valueIndex] =
					static_cast<u_int> (ply_get_argument_value(argument));
		else if (valueIndex == 3) {
			const u_int i0 = tris->back().v[0];
			const u_int i1 = tris->back().v[2];
			const u_int i2 = static_cast<u_int> (ply_get_argument_value(argument));

			tris->push_back(Triangle(i0, i1, i2));
		}
	}

	return 1;
}

ExtTriangleMesh *ExtTriangleMesh::LoadPly(const string &fileName) {
	p_ply plyfile = ply_open(fileName.c_str(), NULL);
	if (!plyfile) {
		stringstream ss;
		ss << "Unable to read PLY mesh file '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	if (!ply_read_header(plyfile)) {
		stringstream ss;
		ss << "Unable to read PLY header from '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	Point *p;
	const long plyNbVerts = ply_set_read_cb(plyfile, "vertex", "x", VertexCB, &p, 0);
	ply_set_read_cb(plyfile, "vertex", "y", VertexCB, &p, 1);
	ply_set_read_cb(plyfile, "vertex", "z", VertexCB, &p, 2);
	if (plyNbVerts <= 0) {
		stringstream ss;
		ss << "No vertices found in '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	vector<Triangle> vi;
	const long plyNbFaces = ply_set_read_cb(plyfile, "face", "vertex_indices", FaceCB, &vi, 0);
	if (plyNbFaces <= 0) {
		stringstream ss;
		ss << "No faces found in '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	// Check if the file includes normal informations
	Normal *n;
	const long plyNbNormals = ply_set_read_cb(plyfile, "vertex", "nx", NormalCB, &n, 0);
	ply_set_read_cb(plyfile, "vertex", "ny", NormalCB, &n, 1);
	ply_set_read_cb(plyfile, "vertex", "nz", NormalCB, &n, 2);
	if ((plyNbNormals > 0) && (plyNbNormals != plyNbVerts)) {
		stringstream ss;
		ss << "Wrong count of normals in '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	// Check if the file includes uv informations
	UV *uv;
	const long plyNbUVs = ply_set_read_cb(plyfile, "vertex", "s", UVCB, &uv, 0);
	ply_set_read_cb(plyfile, "vertex", "t", UVCB, &uv, 1);
	if ((plyNbUVs > 0) && (plyNbUVs != plyNbVerts)) {
		stringstream ss;
		ss << "Wrong count of uvs in '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	// Check if the file includes color informations
	Spectrum *cols;
	const long plyNbColors = ply_set_read_cb(plyfile, "vertex", "red", ColorCB, &cols, 0);
	ply_set_read_cb(plyfile, "vertex", "green", ColorCB, &cols, 1);
	ply_set_read_cb(plyfile, "vertex", "blue", ColorCB, &cols, 2);
	if ((plyNbColors > 0) && (plyNbColors != plyNbVerts)) {
		stringstream ss;
		ss << "Wrong count of colors in '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	// Check if the file includes alpha informations
	float *alphas;
	const long plyNbAlphas = ply_set_read_cb(plyfile, "vertex", "alpha", AlphaCB, &alphas, 0);
	if ((plyNbAlphas > 0) && (plyNbAlphas != plyNbVerts)) {
		stringstream ss;
		ss << "Wrong count of alphas in '" << fileName << "'";
		throw runtime_error(ss.str());
	}

	p = TriangleMesh::AllocVerticesBuffer(plyNbVerts);
	if (plyNbNormals == 0)
		n = NULL;
	else
		n = new Normal[plyNbNormals];
	if (plyNbUVs == 0)
		uv = NULL;
	else
		uv = new UV[plyNbUVs];
	if (plyNbColors == 0)
		cols = NULL;
	else
		cols = new Spectrum[plyNbColors];
	if (plyNbAlphas == 0)
		alphas = NULL;
	else
		alphas = new float[plyNbAlphas];

	if (!ply_read(plyfile)) {
		stringstream ss;
		ss << "Unable to parse PLY file '" << fileName << "'";

		delete[] p;
		delete[] n;
		delete[] uv;
		delete[] cols;
		delete[] alphas;

		throw runtime_error(ss.str());
	}

	ply_close(plyfile);

	// Copy triangle indices vector
	Triangle *tris = TriangleMesh::AllocTrianglesBuffer(vi.size());
	copy(vi.begin(), vi.end(), tris);

	return new ExtTriangleMesh(plyNbVerts, vi.size(), p, tris, n, uv, cols, alphas);
}

//------------------------------------------------------------------------------
// ExtTriangleMesh Load
//------------------------------------------------------------------------------

ExtTriangleMesh *ExtTriangleMesh::Load(const string &fileName) {
	const boost::filesystem::path ext = boost::filesystem::path(fileName).extension();
	if (ext == ".ply")
		return LoadPly(fileName);
	else if (ext == ".bpy")
		return LoadSerialized(fileName);
	else
		throw runtime_error("Unknown file extension while loading a mesh from: " + fileName);	
}

//------------------------------------------------------------------------------
// ExtTriangleMesh Save
//------------------------------------------------------------------------------

void ExtTriangleMesh::Save(const string &fileName) const {
	const boost::filesystem::path ext = boost::filesystem::path(fileName).extension();
	if (ext == ".ply")
		SavePly(fileName);
	else if (ext == ".bpy")
		SaveSerialized(fileName);
	else
		throw runtime_error("Unknown file extension while saving a mesh to: " + fileName);
}

void ExtTriangleMesh::SavePly(const string &fileName) const {
	BOOST_OFSTREAM plyFile(fileName.c_str(), ofstream::out | ofstream::binary | ofstream::trunc);
	if(!plyFile.is_open())
		throw runtime_error("Unable to open: " + fileName);

	// Write the PLY header
	plyFile << "ply\n"
			"format " + string(ply_storage_mode_list[ply_arch_endian()]) + " 1.0\n"
			"comment Created by LuxRays v" LUXRAYS_VERSION_MAJOR "." LUXRAYS_VERSION_MINOR "\n"
			"element vertex " + boost::lexical_cast<string>(vertCount) + "\n"
			"property float x\n"
			"property float y\n"
			"property float z\n";

	if (HasNormals())
		plyFile << "property float nx\n"
				"property float ny\n"
				"property float nz\n";

	if (HasUVs())
		plyFile << "property float s\n"
				"property float t\n";

	if (HasColors())
		plyFile << "property float red\n"
				"property float green\n"
				"property float blue\n";

	if (HasAlphas())
		plyFile << "property float alpha\n";

	plyFile << "element face " + boost::lexical_cast<string>(triCount) + "\n"
				"property list uchar uint vertex_indices\n"
				"end_header\n";

	if (!plyFile.good())
		throw runtime_error("Unable to write PLY header to: " + fileName);

	// Write all vertex data
	for (u_int i = 0; i < vertCount; ++i) {
		plyFile.write((char *)&vertices[i], sizeof(Point));
		if (HasNormals())
			plyFile.write((char *)&normals[i], sizeof(Normal));
		if (HasUVs())
			plyFile.write((char *)&uvs[i], sizeof(UV));
		if (HasColors())
			plyFile.write((char *)&cols[i], sizeof(Spectrum));
		if (HasAlphas())
			plyFile.write((char *)&alphas[i], sizeof(float));
	}
	if (!plyFile.good())
		throw runtime_error("Unable to write PLY vertex data to: " + fileName);

	// Write all face data
	const u_char len = 3;
	for (u_int i = 0; i < triCount; ++i) {
		plyFile.write((char *)&len, 1);
		plyFile.write((char *)&tris[i], sizeof(Triangle));
	}
	if (!plyFile.good())
		throw runtime_error("Unable to write PLY face data to: " + fileName);

	plyFile.close();
}
