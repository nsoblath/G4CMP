/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

// $Id$
//
// -------------------------------------------------------------------
// This version modified to allow oblique electron propagation
// D. Brandt
// 05 / 23 / 2011
//
// 20140331  Inherit from G4EqMagElectricField to handle holes as well as
//	     electrons.  Do local/global transformations; take valley index
//	     run-time configuration argument.
// 20140404  Drop unnecessary data members, using functions in G4LatticePhysical
// 20140501  Fix sign flip in electron charge calculation.
// 20141217  Avoid floating-point division by using vinv = 1/v.mag()
// 20150528  Add debugging output
// 20190802  Check if field is aligned or anti-aligned with valley, apply
//	     transform to valley axis "closest" to field direction.
// 20210921  Add detailed debugging output, protected with G4CMP_DEBUG flag
// 20210922  Field transformation should be Herring-Vogt, with SqrtInvTensor.
// 20211004  Compute velocity from true momentum, local-to-global as needed.
//		Clarify field transform and force calculation.

#include "G4CMPEqEMField.hh"
#include "G4CMPConfigManager.hh"
#include "G4ElectroMagneticField.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"


G4CMPEqEMField::G4CMPEqEMField(G4ElectroMagneticField *emField,
			       const G4LatticePhysical* lattice)
  : G4EqMagElectricField(emField), theLattice(lattice), 
    verboseLevel(G4CMPConfigManager::GetVerboseLevel()),
    fCharge(0.), fMass(0.), valleyIndex(-1) {;}


// Replace physical lattice if track has changed volumes
// NOTE:  Returns TRUE if lattice was actually changed

G4bool G4CMPEqEMField::ChangeLattice(const G4LatticePhysical* lattice) {
  G4bool newLat = (lattice != theLattice);
  theLattice = lattice;
  return newLat;
}


// Specify local-to-global transformation before field call
// Typically, this will be called from FieldManager::ConfigureForTrack()

void G4CMPEqEMField::SetTransforms(const G4AffineTransform& lToG) {
  fGlobalToLocal = fLocalToGlobal = lToG;
  fGlobalToLocal.Invert();
}


// Specify which valley to use for electrons, or no valley at all

void G4CMPEqEMField::SetValley(size_t ivalley) {
  if (theLattice && ivalley<theLattice->NumberOfValleys()) {
    valleyIndex = ivalley;
  } else {
    valleyIndex = -1;
  }
}


// Configuration function must call through to base class
// NOTE: change of signature with G4 10.0

void G4CMPEqEMField::SetChargeMomentumMass(G4ChargeState particleCharge,
					   G4double MomentumXc,
					   G4double mass) {
  G4EqMagElectricField::SetChargeMomentumMass(particleCharge, MomentumXc, mass);
  fCharge = particleCharge.GetCharge() * eplus;
  fMass = mass/c_squared;
}


// Field evaluation:  Given momentum (y) and field, return velocity, force

void G4CMPEqEMField::EvaluateRhsGivenB(const G4double y[],
				       const G4double field[],
				       G4double dydx[]) const {
  // No lattice behaviour, just use base class
  if (valleyIndex == -1) {
    G4EqMagElectricField::EvaluateRhsGivenB(y, field, dydx);
    return;
  }

#ifdef G4CMP_DEBUG
  if (verboseLevel>2) {
    G4cout << "G4CMPEqEMField"
	   << " @ (" << y[0] << "," << y[1] << "," << y[2] << ") mm" << G4endl
	   << " (q,m) " << fCharge/eplus << " e+ "
	   << fMass*c_squared/electron_mass_c2 << " m_e"
	   << " valley " << valleyIndex << G4endl;
  }
#endif

  /* "Momentum" reported by G4 is the true momentum.  Should it be changed
   * to true velocity using the inverse-mass tensor?
   */
  G4ThreeVector plocal = G4ThreeVector(y[3],y[4],y[5]);
  fGlobalToLocal.ApplyAxisTransform(plocal);

  G4ThreeVector v = theLattice->MapPtoV_el(valleyIndex, plocal);
  fLocalToGlobal.ApplyAxisTransform(v);
  G4double vinv = 1./v.mag();

#ifdef G4CMP_DEBUG
  if (verboseLevel>2) {
    G4cout << " pc (" << y[3] << "," << y[4] << "," << y[5] << ") MeV" << G4endl
	   << " v " << v/(km/s) << " km/s " << G4endl
	   << " TOF (1/v) " << vinv/(ns/mm) << " ns/mm"
	   << " c/v " << vinv*c_light << G4endl;
  }
#endif

  G4ThreeVector Efield(field[3], field[4], field[5]);

#ifdef G4CMP_DEBUG
  if (verboseLevel>2) {
    G4cout << " E-field " << Efield/(volt/cm) << " " << Efield.mag()/(volt/cm)
	   << " V/cm" << G4endl;
  }
#endif

  fGlobalToLocal.ApplyAxisTransform(Efield);
  theLattice->RotateToLattice(Efield);
#ifdef G4CMP_DEBUG
  if (verboseLevel>2) G4cout << " Field (lattice) " << Efield/(eV/m) << G4endl;
#endif

  // Rotate force into and out of valley frame, applying Herring-Vogt transform
  const G4RotationMatrix& nToV = theLattice->GetValley(valleyIndex);
  const G4RotationMatrix& vToN = theLattice->GetValleyInv(valleyIndex);

  Efield.transform(nToV);			// Rotate to valley
#ifdef G4CMP_DEBUG
  if (verboseLevel>2) G4cout << " Field (valley) " << Efield/(eV/m) << G4endl;
#endif

  Efield *= theLattice->GetSqrtInvTensor();	// Herring-Vogt transform
#ifdef G4CMP_DEBUG
  if (verboseLevel>2) G4cout << " Field (H-V) " << Efield/(eV/m) << G4endl;
#endif

  Efield.transform(vToN);			// Back to lattice
#ifdef G4CMP_DEBUG
  if (verboseLevel>2) G4cout << " Field (H-V, lattice) " << Efield/(eV/m) << G4endl;
#endif

  theLattice->RotateToSolid(Efield);		// Back to crystal frame
#ifdef G4CMP_DEBUG
  if (verboseLevel>2) G4cout << " Field (H-V, local) " << Efield/(eV/m) << G4endl;
#endif

  // Restore field to global coordinate frame for G4Transporation
  fLocalToGlobal.ApplyAxisTransform(Efield);
#ifdef G4CMP_DEBUG
  if (verboseLevel>2) G4cout << " Field (H-V, global) " << Efield/(eV/m) << G4endl;
#endif

  // Force = qE/beta
  G4ThreeVector& force = Efield;	// Name change without copying
  force *= fCharge*vinv*c_light;

#ifdef G4CMP_DEBUG
  if (verboseLevel>2) {
    G4cout << " q*Ec/v (global) " << force/(eV/m) << " " << force.mag()/(eV/m)
	   << " eV/m" << G4endl;
  }
#endif

  // Populate output buffer
  dydx[0] = v.x()*vinv;		// Velocity direction
  dydx[1] = v.y()*vinv;
  dydx[2] = v.z()*vinv;
  dydx[3] = force.x();		// Applied force in H-V, global coordinates
  dydx[4] = force.y();
  dydx[5] = force.z();
  dydx[6] = 0.;			// not used
  dydx[7] = vinv;		// Lab Time of flight (ns/mm)
}
