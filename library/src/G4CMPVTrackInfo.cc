/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/src/G4CMPVTrackInfo.cc
/// \brief Implementation of the G4CMPVTrackInfo class. Used as a base class
/// for CMP particles to store auxiliary information that a G4Track can't
/// store, but is necessary for physics processes to know.
///
//
// $Id$
//
// 20161111 Initial commit - R. Agnese

#include "G4CMPVTrackInfo.hh"
#include "G4Exception.hh"

G4CMPVTrackInfo::G4CMPVTrackInfo(const G4LatticePhysical* lat) :
  G4VAuxiliaryTrackInformation(), lattice(lat) {}

void G4CMPVTrackInfo::Print() const {
//TODO
}

std::string G4CMPVTrackInfo::ToString( BoundaryTermination term )
{
  switch (term) {
    case BoundaryTermination::kNone: return "None";
    case BoundaryTermination::kUnknown: return "Unknown";
    case BoundaryTermination::kOther: return "Other";
    case BoundaryTermination::kNoMatTable: return "NoMatTable";
    case BoundaryTermination::kElectrodeAbsorption: return "ElectrodeAbsorption";
    case BoundaryTermination::kMaxReflections: return "MaxReflections";
    case BoundaryTermination::kDefaultTransmission: return "DefaultTransmission";
    case BoundaryTermination::kReflectionFailed: return "ReflectionFailed";
    default: G4Exception("G4CMPVTrackInfo::ToString()", "InvalidEnum", FatalException, "An invalid BoundaryTermination enum was encountered");
  }
}
