#include "Core/SurfaceBehavior/Property/SchlickApproxConductorDielectricFresnel.h"

namespace ph
{

SchlickApproxConductorDielectricFresnel::SchlickApproxConductorDielectricFresnel(
	const real              iorOuter,
	const SpectralStrength& iorInner,
	const SpectralStrength& iorInnerK) : 
	ConductorDielectricFresnel(iorOuter, iorInner, iorInnerK)
{
	const SpectralStrength neg2 = iorInner.sub(SpectralStrength(iorOuter)).pow(2);
	const SpectralStrength pos2 = iorInner.add(SpectralStrength(iorOuter)).pow(2);
	const SpectralStrength nume = neg2.add(iorInnerK.pow(2));
	const SpectralStrength deno = pos2.add(iorInnerK.pow(2));
	m_f0           = nume.div(deno);
	m_f0Complement = m_f0.complement();
}

SchlickApproxConductorDielectricFresnel::SchlickApproxConductorDielectricFresnel(
	const SpectralStrength& f0) : 

	// FIXME: this might cause problems if the class is used polymorphically
	// actual IOR values are not needed by Schlick's approximation during runtime
	ConductorDielectricFresnel(1, SpectralStrength(1), SpectralStrength(1)),

	m_f0(f0), m_f0Complement(f0.complement())
{}

void SchlickApproxConductorDielectricFresnel::calcReflectance(
	const real cosThetaIncident,
	SpectralStrength* const out_reflectance) const
{
	// We treat the incident light be always in the dielectric side (which is
	// reasonable since light should not penetrate conductors easily), so the 
	// sign of cosI does not matter here.
	//
	const real cosI = std::abs(cosThetaIncident);

	out_reflectance->setValues(m_f0Complement.mul(std::pow(1.0_r - cosI, 5)).addLocal(m_f0));
}

}// end namespace ph