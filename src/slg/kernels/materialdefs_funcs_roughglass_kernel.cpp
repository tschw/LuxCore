#include <string>
namespace slg { namespace ocl {
std::string KernelSource_materialdefs_funcs_roughglass = 
"#line 2 \"materialdefs_funcs_roughglass.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// RoughGlass material\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined (PARAM_ENABLE_MAT_ROUGHGLASS)\n"
"\n"
"BSDFEvent RoughGlassMaterial_GetEventTypes() {\n"
"	return GLOSSY | REFLECT | TRANSMIT;\n"
"}\n"
"\n"
"bool RoughGlassMaterial_IsDelta() {\n"
"	return false;\n"
"}\n"
"\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"float3 RoughGlassMaterial_GetPassThroughTransparency(__global const Material *material,\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, const float passThroughEvent\n"
"		TEXTURES_PARAM_DECL) {\n"
"	return BLACK;\n"
"}\n"
"#endif\n"
"\n"
"float3 RoughGlassMaterial_Evaluate(\n"
"		__global HitPoint *hitPoint, const float3 localLightDir, const float3 localEyeDir,\n"
"		BSDFEvent *event, float *directPdfW,\n"
"		const float3 ktVal, const float3 krVal,\n"
"		const float nuVal,\n"
"#if defined(PARAM_ENABLE_MAT_ROUGHGLASS_ANISOTROPIC)\n"
"		const float nvVal,\n"
"#endif\n"
"		const float nc, const float nt\n"
"		) {\n"
"	const float3 kt = Spectrum_Clamp(ktVal);\n"
"	const float3 kr = Spectrum_Clamp(ktVal);\n"
"\n"
"	const bool isKtBlack = Spectrum_IsBlack(kt);\n"
"	const bool isKrBlack = Spectrum_IsBlack(kr);\n"
"	if (isKtBlack && isKrBlack)\n"
"		return BLACK;\n"
"	\n"
"	const float ntc = nt / nc;\n"
"\n"
"	const float u = clamp(nuVal, 0.f, 1.f);\n"
"#if defined(PARAM_ENABLE_MAT_ROUGHGLASS_ANISOTROPIC)\n"
"	const float v = clamp(nvVal, 0.f, 1.f);\n"
"	const float u2 = u * u;\n"
"	const float v2 = v * v;\n"
"	const float anisotropy = (u2 < v2) ? (1.f - u2 / v2) : u2 > 0.f ? (v2 / u2 - 1.f) : 0.f;\n"
"	const float roughness = u * v;\n"
"#else\n"
"	const float anisotropy = 0.f;\n"
"	const float roughness = u * u;\n"
"#endif\n"
"\n"
"	const float threshold = isKrBlack ? 1.f : (isKtBlack ? 0.f : .5f);\n"
"	if (localLightDir.z * localEyeDir.z < 0.f) {\n"
"		// Transmit\n"
"\n"
"		const bool entering = (CosTheta(localLightDir) > 0.f);\n"
"		const float eta = entering ? (nc / nt) : ntc;\n"
"\n"
"		float3 wh = eta * localLightDir + localEyeDir;\n"
"		if (wh.z < 0.f)\n"
"			wh = -wh;\n"
"\n"
"		const float lengthSquared = dot(wh, wh);\n"
"		if (!(lengthSquared > 0.f))\n"
"			return BLACK;\n"
"		wh /= sqrt(lengthSquared);\n"
"		const float cosThetaI = fabs(CosTheta(localEyeDir));\n"
"		const float cosThetaIH = fabs(dot(localEyeDir, wh));\n"
"		const float cosThetaOH = dot(localLightDir, wh);\n"
"\n"
"		const float D = SchlickDistribution_D(roughness, wh, anisotropy);\n"
"		const float G = SchlickDistribution_G(roughness, localLightDir, localEyeDir);\n"
"		const float specPdf = SchlickDistribution_Pdf(roughness, wh, anisotropy);\n"
"		const float3 F = FresnelCauchy_Evaluate(ntc, cosThetaOH);\n"
"\n"
"		if (directPdfW)\n"
"			*directPdfW = threshold * specPdf * (fabs(cosThetaOH) * eta * eta) / lengthSquared;\n"
"\n"
"		//if (reversePdfW)\n"
"		//	*reversePdfW = threshold * specPdf * cosThetaIH / lengthSquared;\n"
"\n"
"		float3 result = (fabs(cosThetaOH) * cosThetaIH * D *\n"
"			G / (cosThetaI * lengthSquared)) *\n"
"			kt * (1.f - F);\n"
"\n"
"        *event = DIFFUSE | TRANSMIT;\n"
"\n"
"		return result;\n"
"	} else {\n"
"		// Reflect\n"
"		const float cosThetaO = fabs(CosTheta(localLightDir));\n"
"		const float cosThetaI = fabs(CosTheta(localEyeDir));\n"
"		if (cosThetaO == 0.f || cosThetaI == 0.f)\n"
"			return BLACK;\n"
"		float3 wh = localLightDir + localEyeDir;\n"
"		if (all(isequal(wh, BLACK)))\n"
"			return BLACK;\n"
"		wh = normalize(wh);\n"
"		if (wh.z < 0.f)\n"
"			wh = -wh;\n"
"\n"
"		float cosThetaH = dot(localEyeDir, wh);\n"
"		const float D = SchlickDistribution_D(roughness, wh, anisotropy);\n"
"		const float G = SchlickDistribution_G(roughness, localLightDir, localEyeDir);\n"
"		const float specPdf = SchlickDistribution_Pdf(roughness, wh, anisotropy);\n"
"		const float3 F = FresnelCauchy_Evaluate(ntc, cosThetaH);\n"
"\n"
"		if (directPdfW)\n"
"			*directPdfW = (1.f - threshold) * specPdf / (4.f * fabs(dot(localLightDir, wh)));\n"
"\n"
"		//if (reversePdfW)\n"
"		//	*reversePdfW = (1.f - threshold) * specPdf / (4.f * fabs(dot(localLightDir, wh));\n"
"\n"
"		float3 result = (D * G / (4.f * cosThetaI)) * kr * F;\n"
"\n"
"        *event = DIFFUSE | REFLECT;\n"
"\n"
"		return result;\n"
"	}\n"
"}\n"
"\n"
"float3 RoughGlassMaterial_Sample(\n"
"		__global HitPoint *hitPoint, const float3 localFixedDir, float3 *localSampledDir,\n"
"		const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float *pdfW, float *absCosSampledDir, BSDFEvent *event,\n"
"		const BSDFEvent requestedEvent,\n"
"		const float3 ktVal, const float3 krVal,\n"
"		const float nuVal,\n"
"#if defined(PARAM_ENABLE_MAT_ROUGHGLASS_ANISOTROPIC)\n"
"		const float nvVal,\n"
"#endif\n"
"		const float nc, const float nt\n"
"		) {\n"
"	if (!(requestedEvent & (GLOSSY | REFLECT | TRANSMIT)) ||\n"
"			(fabs(localFixedDir.z) < DEFAULT_COS_EPSILON_STATIC))\n"
"		return BLACK;\n"
"\n"
"	const float3 kt = Spectrum_Clamp(ktVal);\n"
"	const float3 kr = Spectrum_Clamp(ktVal);\n"
"\n"
"	const bool isKtBlack = Spectrum_IsBlack(kt);\n"
"	const bool isKrBlack = Spectrum_IsBlack(kr);\n"
"	if (isKtBlack && isKrBlack)\n"
"		return BLACK;\n"
"\n"
"	const float u = clamp(nuVal, 0.f, 1.f);\n"
"#if defined(PARAM_ENABLE_MAT_ROUGHGLASS_ANISOTROPIC)\n"
"	const float v = clamp(nvVal, 0.f, 1.f);\n"
"	const float u2 = u * u;\n"
"	const float v2 = v * v;\n"
"	const float anisotropy = (u2 < v2) ? (1.f - u2 / v2) : u2 > 0.f ? (v2 / u2 - 1.f);\n"
"	const float roughness = u * v;\n"
"#else\n"
"	const float anisotropy = 0.f;\n"
"	const float roughness = u * u;\n"
"#endif\n"
"\n"
"	float3 wh;\n"
"	float d, specPdf;\n"
"	SchlickDistribution_SampleH(roughness, anisotropy, u0, u1, &wh, &d, &specPdf);\n"
"	if (wh.z < 0.f)\n"
"		wh = -wh;\n"
"	const float cosThetaOH = dot(localFixedDir, wh);\n"
"\n"
"	const float ntc = nt / nc;\n"
"\n"
"	const float coso = fabs(localFixedDir.z);\n"
"\n"
"	// Decide to transmit or reflect\n"
"	float threshold;\n"
"	if ((requestedEvent & REFLECT) && !isKrBlack) {\n"
"		if ((requestedEvent & TRANSMIT) && !isKtBlack)\n"
"			threshold = .5f;\n"
"		else\n"
"			threshold = 0.f;\n"
"	} else {\n"
"		if ((requestedEvent & TRANSMIT) && !isKtBlack)\n"
"			threshold = 1.f;\n"
"		else\n"
"			return BLACK;\n"
"	}\n"
"\n"
"	float3 result;\n"
"	if (passThroughEvent < threshold) {\n"
"		// Transmit\n"
"\n"
"		const bool entering = (CosTheta(localFixedDir) > 0.f);\n"
"		const float eta = entering ? (nc / nt) : ntc;\n"
"		const float eta2 = eta * eta;\n"
"		const float sinThetaIH2 = eta2 * fmax(0.f, 1.f - cosThetaOH * cosThetaOH);\n"
"		if (sinThetaIH2 >= 1.f)\n"
"			return BLACK;\n"
"		float cosThetaIH = sqrt(1.f - sinThetaIH2);\n"
"		if (entering)\n"
"			cosThetaIH = -cosThetaIH;\n"
"		const float length = eta * cosThetaOH + cosThetaIH;\n"
"		*localSampledDir = length * wh - eta * localFixedDir;\n"
"\n"
"		const float lengthSquared = length * length;\n"
"		*pdfW = specPdf * fabs(cosThetaIH) / lengthSquared;\n"
"		if (*pdfW <= 0.f)\n"
"			return BLACK;\n"
"\n"
"		const float cosi = fabs((*localSampledDir).z);\n"
"		*absCosSampledDir = cosi;\n"
"\n"
"		const float G = SchlickDistribution_G(roughness, localFixedDir, *localSampledDir);\n"
"		float factor = (d / specPdf) * G * fabs(cosThetaOH) / threshold;\n"
"\n"
"		//if (!hitPoint.fromLight) {\n"
"			const float3 F = FresnelCauchy_Evaluate(ntc, cosThetaIH);\n"
"			result = (factor / coso) * kt * (1.f - F);\n"
"		//} else {\n"
"		//	const Spectrum F = FresnelCauchy_Evaluate(ntc, cosThetaOH);\n"
"		//	result = (factor / cosi) * kt * (Spectrum(1.f) - F);\n"
"		//}\n"
"\n"
"		*pdfW *= threshold;\n"
"		*event = GLOSSY | TRANSMIT;\n"
"	} else {\n"
"		// Reflect\n"
"		*pdfW = specPdf / (4.f * fabs(cosThetaOH));\n"
"		if (*pdfW <= 0.f)\n"
"			return BLACK;\n"
"\n"
"		*localSampledDir = 2.f * cosThetaOH * wh - localFixedDir;\n"
"\n"
"		const float cosi = fabs((*localSampledDir).z);\n"
"		*absCosSampledDir = cosi;\n"
"		if ((*absCosSampledDir < DEFAULT_COS_EPSILON_STATIC) || (localFixedDir.z * (*localSampledDir).z < 0.f))\n"
"			return BLACK;\n"
"\n"
"		const float G = SchlickDistribution_G(roughness, localFixedDir, *localSampledDir);\n"
"		float factor = (d / specPdf) * G * fabs(cosThetaOH) / (1.f - threshold);\n"
"\n"
"		const float3 F = FresnelCauchy_Evaluate(ntc, cosThetaOH);\n"
"		//factor /= (!hitPoint.fromLight) ? coso : cosi;\n"
"		factor /= coso;\n"
"		result = factor * F * kr;\n"
"\n"
"		*pdfW *= (1.f - threshold);\n"
"		*event = GLOSSY | REFLECT;\n"
"	}\n"
"\n"
"	return result;\n"
"}\n"
"\n"
"#endif\n"
; } }
