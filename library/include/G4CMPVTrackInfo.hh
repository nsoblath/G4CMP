/***********************************************************************\
 * This software is licensed under the terms of the GNU General Public *
 * License version 3 or later. See G4CMP/LICENSE for the full license. *
\***********************************************************************/

/// \file library/include/G4CMPVTrackInfo.hh
/// \brief Definition of the G4CMPVTrackInfo class. Used as a base class
/// for CMP particles to store auxiliary information that a G4Track can't
/// store, but is necessary for physics processes to know.
///
//
// $Id$
//
// 20161111 Initial commit - R. Agnese

#ifndef G4CMPVTrackInfo_hh
#define G4CMPVTrackInfo_hh 1

#include "G4VAuxiliaryTrackInformation.hh"

class G4LatticePhysical;

class G4CMPVTrackInfo: public G4VAuxiliaryTrackInformation {
public:
  G4CMPVTrackInfo() = delete;
  G4CMPVTrackInfo(const G4LatticePhysical* lat);

  size_t ReflectionCount() const                           { return reflCount; }
  void IncrementReflectionCount()                               { ++reflCount; }

  const G4LatticePhysical* Lattice() const                   { return lattice; }
  void SetLattice(const G4LatticePhysical* lat)               { lattice = lat; }

  virtual void Print() const override;

  enum class BoundaryTermination : unsigned {
    kNone = 0,
    kUnknown = 10,
    kOther = 100,
    kNoMatTable = 101,
    kElectrodeAbsorption = 102,
    kMaxReflections = 103,
    kDefaultTransmission = 104,
    kReflectionFailed = 105
  };
  BoundaryTermination GetBoundaryTermination() const                  { return boundaryTerm; }
  void SetBoundaryTermination(BoundaryTermination aBT)           { boundaryTerm = aBT; }
  std::string ToString( BoundaryTermination term );

private:
  size_t reflCount = 0; // Number of times track has been reflected
  const G4LatticePhysical* lattice; // The lattice the track is currently in
  BoundaryTermination boundaryTerm = BoundaryTermination::kNone; // To track what caused a track to die
};

#endif
