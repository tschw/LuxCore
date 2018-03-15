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

#include <limits>
#include <algorithm>
#include <exception>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "slg/film/film.h"

using namespace std;
using namespace luxrays;
using namespace slg;

//------------------------------------------------------------------------------
// Film channels
//------------------------------------------------------------------------------

void Film::FreeChannels() {
	for (u_int i = 0; i < channel_RADIANCE_PER_PIXEL_NORMALIZEDs.size(); ++i)
		delete channel_RADIANCE_PER_PIXEL_NORMALIZEDs[i];
	for (u_int i = 0; i < channel_RADIANCE_PER_SCREEN_NORMALIZEDs.size(); ++i)
		delete channel_RADIANCE_PER_SCREEN_NORMALIZEDs[i];
	delete channel_ALPHA;
	for (u_int i = 0; i < channel_IMAGEPIPELINEs.size(); ++i)
		delete channel_IMAGEPIPELINEs[i];
	delete channel_DEPTH;
	delete channel_POSITION;
	delete channel_GEOMETRY_NORMAL;
	delete channel_SHADING_NORMAL;
	delete channel_MATERIAL_ID;
	delete channel_DIRECT_DIFFUSE;
	delete channel_DIRECT_GLOSSY;
	delete channel_EMISSION;
	delete channel_INDIRECT_DIFFUSE;
	delete channel_INDIRECT_GLOSSY;
	delete channel_INDIRECT_SPECULAR;
	for (u_int i = 0; i < channel_MATERIAL_ID_MASKs.size(); ++i)
		delete channel_MATERIAL_ID_MASKs[i];
	delete channel_DIRECT_SHADOW_MASK;
	delete channel_INDIRECT_SHADOW_MASK;
	delete channel_UV;
	delete channel_RAYCOUNT;
	for (u_int i = 0; i < channel_BY_MATERIAL_IDs.size(); ++i)
		delete channel_BY_MATERIAL_IDs[i];
	delete channel_IRRADIANCE;
	delete channel_OBJECT_ID;
	for (u_int i = 0; i < channel_OBJECT_ID_MASKs.size(); ++i)
		delete channel_OBJECT_ID_MASKs[i];
	for (u_int i = 0; i < channel_BY_OBJECT_IDs.size(); ++i)
		delete channel_BY_OBJECT_IDs[i];
	delete channel_FRAMEBUFFER_MASK;
	delete channel_SAMPLECOUNT;
	delete channel_CONVERGENCE;
}

void Film::AddChannel(const FilmChannelType type, const Properties *prop) {
	if (initialized)
		throw runtime_error("It is only possible to add a channel to a Film before initialization");

	channels.insert(type);
	switch (type) {
		case MATERIAL_ID_MASK: {
			const u_int id = prop->Get(Property("id")(255)).Get<u_int>();
			if (count(maskMaterialIDs.begin(), maskMaterialIDs.end(), id) == 0)
				maskMaterialIDs.push_back(id);
			break;
		}
		case BY_MATERIAL_ID: {
			const u_int id = prop->Get(Property("id")(255)).Get<u_int>();
			if (count(byMaterialIDs.begin(), byMaterialIDs.end(), id) == 0)
				byMaterialIDs.push_back(id);
			break;
		}
		case OBJECT_ID_MASK: {
			const u_int id = prop->Get(Property("id")(255)).Get<u_int>();
			if (count(maskObjectIDs.begin(), maskObjectIDs.end(), id) == 0)
				maskObjectIDs.push_back(id);
			break;
		}
		case BY_OBJECT_ID: {
			const u_int id = prop->Get(Property("id")(255)).Get<u_int>();
			if (count(byObjectIDs.begin(), byObjectIDs.end(), id) == 0)
				byObjectIDs.push_back(id);
			break;
		}
		default:
			break;
	}
}

void Film::RemoveChannel(const FilmChannelType type) {
	if (initialized)
		throw runtime_error("It is only possible to remove a channel from a Film before initialization");

	channels.erase(type);
}

void Film::SetRadianceChannelScale(const u_int index, const RadianceChannelScale &scale) {
	radianceChannelScales.resize(Max<size_t>(radianceChannelScales.size(), index + 1));

	radianceChannelScales[index] = scale;
	radianceChannelScales[index].Init();
}

u_int Film::GetChannelCount(const FilmChannelType type) const {
	switch (type) {
		case RADIANCE_PER_PIXEL_NORMALIZED:
			return channel_RADIANCE_PER_PIXEL_NORMALIZEDs.size();
		case RADIANCE_PER_SCREEN_NORMALIZED:
			return channel_RADIANCE_PER_SCREEN_NORMALIZEDs.size();
		case ALPHA:
			return channel_ALPHA ? 1 : 0;
		case IMAGEPIPELINE:
			return channel_IMAGEPIPELINEs.size();
		case DEPTH:
			return channel_DEPTH ? 1 : 0;
		case POSITION:
			return channel_POSITION ? 1 : 0;
		case GEOMETRY_NORMAL:
			return channel_GEOMETRY_NORMAL ? 1 : 0;
		case SHADING_NORMAL:
			return channel_SHADING_NORMAL ? 1 : 0;
		case MATERIAL_ID:
			return channel_MATERIAL_ID ? 1 : 0;
		case DIRECT_DIFFUSE:
			return channel_DIRECT_DIFFUSE ? 1 : 0;
		case DIRECT_GLOSSY:
			return channel_DIRECT_GLOSSY ? 1 : 0;
		case EMISSION:
			return channel_EMISSION ? 1 : 0;
		case INDIRECT_DIFFUSE:
			return channel_INDIRECT_DIFFUSE ? 1 : 0;
		case INDIRECT_GLOSSY:
			return channel_INDIRECT_GLOSSY ? 1 : 0;
		case INDIRECT_SPECULAR:
			return channel_INDIRECT_SPECULAR ? 1 : 0;
		case MATERIAL_ID_MASK:
			return channel_MATERIAL_ID_MASKs.size();
		case DIRECT_SHADOW_MASK:
			return channel_DIRECT_SHADOW_MASK ? 1 : 0;
		case INDIRECT_SHADOW_MASK:
			return channel_INDIRECT_SHADOW_MASK ? 1 : 0;
		case UV:
			return channel_UV ? 1 : 0;
		case RAYCOUNT:
			return channel_RAYCOUNT ? 1 : 0;
		case BY_MATERIAL_ID:
			return channel_BY_MATERIAL_IDs.size();
		case IRRADIANCE:
			return channel_IRRADIANCE ? 1 : 0;
		case OBJECT_ID:
			return channel_OBJECT_ID ? 1 : 0;
		case OBJECT_ID_MASK:
			return channel_OBJECT_ID_MASKs.size();
		case BY_OBJECT_ID:
			return channel_BY_OBJECT_IDs.size();
		case FRAMEBUFFER_MASK:
			return channel_FRAMEBUFFER_MASK ? 1 : 0;
		case SAMPLECOUNT:
			return channel_SAMPLECOUNT ? 1 : 0;
		case CONVERGENCE:
			return channel_CONVERGENCE ? 1 : 0;
		default:
			throw runtime_error("Unknown FilmChannelType in Film::GetChannelCount(): " + ToString(type));
	}
}

template<> const float *Film::GetChannel<float>(const FilmChannelType type, const u_int index) {
	if (!HasChannel(type))
		throw runtime_error("Film channel not defined in Film::GetChannel<float>(): " + ToString(type));

	if (index > GetChannelCount(type))
		throw runtime_error("Film channel index not defined in Film::GetChannel<float>(): " + ToString(type) + "/" + ToString(index));

	switch (type) {
		case RADIANCE_PER_PIXEL_NORMALIZED:
			return channel_RADIANCE_PER_PIXEL_NORMALIZEDs[index]->GetPixels();
		case RADIANCE_PER_SCREEN_NORMALIZED:
			return channel_RADIANCE_PER_SCREEN_NORMALIZEDs[index]->GetPixels();
		case ALPHA:
			return channel_ALPHA->GetPixels();
		case IMAGEPIPELINE: {
			ExecuteImagePipeline(index);
			return channel_IMAGEPIPELINEs[index]->GetPixels();
		}
		case DEPTH:
			return channel_DEPTH->GetPixels();
		case POSITION:
			return channel_POSITION->GetPixels();
		case GEOMETRY_NORMAL:
			return channel_GEOMETRY_NORMAL->GetPixels();
		case SHADING_NORMAL:
			return channel_SHADING_NORMAL->GetPixels();
		case DIRECT_DIFFUSE:
			return channel_DIRECT_DIFFUSE->GetPixels();
		case DIRECT_GLOSSY:
			return channel_DIRECT_GLOSSY->GetPixels();
		case EMISSION:
			return channel_EMISSION->GetPixels();
		case INDIRECT_DIFFUSE:
			return channel_INDIRECT_DIFFUSE->GetPixels();
		case INDIRECT_GLOSSY:
			return channel_INDIRECT_GLOSSY->GetPixels();
		case INDIRECT_SPECULAR:
			return channel_INDIRECT_SPECULAR->GetPixels();
		case MATERIAL_ID_MASK:
			return channel_MATERIAL_ID_MASKs[index]->GetPixels();
		case DIRECT_SHADOW_MASK:
			return channel_DIRECT_SHADOW_MASK->GetPixels();
		case INDIRECT_SHADOW_MASK:
			return channel_INDIRECT_SHADOW_MASK->GetPixels();
		case UV:
			return channel_UV->GetPixels();
		case RAYCOUNT:
			return channel_RAYCOUNT->GetPixels();
		case BY_MATERIAL_ID:
			return channel_BY_MATERIAL_IDs[index]->GetPixels();
		case IRRADIANCE:
			return channel_IRRADIANCE->GetPixels();
		case OBJECT_ID_MASK:
			return channel_OBJECT_ID_MASKs[index]->GetPixels();
		case BY_OBJECT_ID:
			return channel_BY_OBJECT_IDs[index]->GetPixels();
		case CONVERGENCE:
			return channel_CONVERGENCE->GetPixels();
		default:
			throw runtime_error("Unknown FilmChannelType in Film::GetChannel<float>(): " + ToString(type));
	}
}

template<> const u_int *Film::GetChannel<u_int>(const FilmChannelType type, const u_int index) {
	if (!HasChannel(type))
		throw runtime_error("Film channel not defined in Film::GetChannel<u_int>(): " + ToString(type));

	if (index > GetChannelCount(type))
		throw runtime_error("Film channel index not defined in Film::GetChannel<u_int>(): " + ToString(type) + "/" + ToString(index));

	switch (type) {
		case MATERIAL_ID:
			return channel_MATERIAL_ID->GetPixels();
		case OBJECT_ID:
			return channel_OBJECT_ID->GetPixels();
		case FRAMEBUFFER_MASK:
			return channel_FRAMEBUFFER_MASK->GetPixels();
		case SAMPLECOUNT:
			return channel_SAMPLECOUNT->GetPixels();
		default:
			throw runtime_error("Unknown FilmChannelType in Film::GetChannel<u_int>(): " + ToString(type));
	}
}

Film::FilmChannelType Film::String2FilmChannelType(const std::string &type) {
	if (type == "RADIANCE_PER_PIXEL_NORMALIZED")
		return RADIANCE_PER_PIXEL_NORMALIZED;
	else if (type == "RADIANCE_PER_SCREEN_NORMALIZED")
		return RADIANCE_PER_SCREEN_NORMALIZED;
	else if (type == "ALPHA")
		return ALPHA;
	else if (type == "DEPTH")
		return DEPTH;
	else if (type == "POSITION")
		return POSITION;
	else if (type == "GEOMETRY_NORMAL")
		return GEOMETRY_NORMAL;
	else if (type == "SHADING_NORMAL")
		return SHADING_NORMAL;
	else if (type == "MATERIAL_ID")
		return MATERIAL_ID;
	else if (type == "DIRECT_DIFFUSE")
		return DIRECT_DIFFUSE;
	else if (type == "DIRECT_GLOSSY")
		return DIRECT_GLOSSY;
	else if (type == "EMISSION")
		return EMISSION;
	else if (type == "INDIRECT_DIFFUSE")
		return INDIRECT_DIFFUSE;
	else if (type == "INDIRECT_GLOSSY")
		return INDIRECT_GLOSSY;
	else if (type == "INDIRECT_SPECULAR")
		return INDIRECT_SPECULAR;
	else if (type == "INDIRECT_SPECULAR")
		return INDIRECT_SPECULAR;
	else if (type == "MATERIAL_ID_MASK")
		return MATERIAL_ID_MASK;
	else if (type == "DIRECT_SHADOW_MASK")
		return DIRECT_SHADOW_MASK;
	else if (type == "INDIRECT_SHADOW_MASK")
		return INDIRECT_SHADOW_MASK;
	else if (type == "UV")
		return UV;
	else if (type == "RAYCOUNT")
		return RAYCOUNT;
	else if (type == "BY_MATERIAL_ID")
		return BY_MATERIAL_ID;
	else if (type == "IRRADIANCE")
		return IRRADIANCE;
	else if (type == "OBJECT_ID")
		return OBJECT_ID;
	else if (type == "OBJECT_ID_MASK")
		return OBJECT_ID_MASK;
	else if (type == "BY_OBJECT_ID")
		return BY_OBJECT_ID;
	else if (type == "SAMPLECOUNT")
		return SAMPLECOUNT;
	else if (type == "CONVERGENCE")
		return CONVERGENCE;
	else
		throw runtime_error("Unknown film output type in Film::String2FilmChannelType(): " + type);
}

const std::string Film::FilmChannelType2String(const Film::FilmChannelType type) {
	switch (type) {
		case Film::RADIANCE_PER_PIXEL_NORMALIZED:
			return "RADIANCE_PER_PIXEL_NORMALIZED";
		case Film::RADIANCE_PER_SCREEN_NORMALIZED:
			return "RADIANCE_PER_SCREEN_NORMALIZED";
		case Film::ALPHA:
			return "ALPHA";
		case Film::DEPTH:
			return "DEPTH";
		case Film::POSITION:
			return "POSITION";
		case Film::GEOMETRY_NORMAL:
			return "GEOMETRY_NORMAL";
		case Film::SHADING_NORMAL:
			return "SHADING_NORMAL";
		case Film::MATERIAL_ID:
			return "MATERIAL_ID";
		case Film::DIRECT_DIFFUSE:
			return "DIRECT_DIFFUSE";
		case Film::DIRECT_GLOSSY:
			return "DIRECT_GLOSSY";
		case Film::EMISSION:
			return "EMISSION";
		case Film::INDIRECT_DIFFUSE:
			return "INDIRECT_DIFFUSE";
		case Film::INDIRECT_GLOSSY:
			return "INDIRECT_GLOSSY";
		case Film::INDIRECT_SPECULAR:
			return "INDIRECT_SPECULAR";
		case Film::MATERIAL_ID_MASK:
			return "MATERIAL_ID_MASK";
		case Film::DIRECT_SHADOW_MASK:
			return "DIRECT_SHADOW_MASK";
		case Film::INDIRECT_SHADOW_MASK:
			return "INDIRECT_SHADOW_MASK";
		case Film::UV:
			return "UV";
		case Film::RAYCOUNT:
			return "RAYCOUNT";
		case Film::BY_MATERIAL_ID:
			return "BY_MATERIAL_ID";
		case Film::IRRADIANCE:
			return "IRRADIANCE";
		case Film::OBJECT_ID:
			return "OBJECT_ID";
		case Film::OBJECT_ID_MASK:
			return "OBJECT_ID_MASK";
		case Film::BY_OBJECT_ID:
			return "BY_OBJECT_ID";
		case Film::SAMPLECOUNT:
			return "SAMPLECOUNT";
		case Film::CONVERGENCE:
			return "CONVERGENCE";
		default:
			throw runtime_error("Unknown film output type in Film::FilmChannelType2String(): " + ToString(type));
	}
}