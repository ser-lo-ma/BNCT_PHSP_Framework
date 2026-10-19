//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// Author: M.A. Cortes-Giraldo, Universidad de Sevilla
//
// Modified by S. Lopez Martinez, University College London, 2026,
// for the BNCT phase-space framework:
//   added the /IAEAphspReader/trim* commands; removed the collimator and
//   gantry commands; /IAEAphspReader/rotationOrder now sets the rotation
//   order (it used to set the recycling count).
//
// History changelog prior creation of this example:
// - 17/10/2009: version 1.0
// - 20/11/2009: version 1.1 before publishing:
//   - Changed some names by more suitable ones
// - 02/08/2010: version 1.2-dev:
//   - Added possibility of applying axial symmetries
// - 14/09/2023: version 2.0
//   - Following Geant4 coding guidelines
// - 18/10/2025: version 3.0
//   - Creation of IAEASourceIdRegistry for thread-safe source_id assignation
//

#ifndef G4IAEAphspReaderMessenger_h
#define G4IAEAphspReaderMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"

class G4IAEAphspReader;

class G4UIcmdWith3Vector;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;
class G4UIcommand;
class G4UIdirectory;

class G4IAEAphspReaderMessenger: public G4UImessenger
{
public:
  G4IAEAphspReaderMessenger(G4IAEAphspReader* );
  ~G4IAEAphspReaderMessenger() override;
    
  void SetNewValue(G4UIcommand*, G4String) override;

private:

  G4IAEAphspReader* fIAEAphspReader;
  // Pointer to the IAEA phase-space reader.

  G4UIdirectory* fPhaseSpaceDir;
  // Control of the phase space

  G4UIcmdWithAnInteger* fVerboseCmd;
  // UI command for verbosity level.

  G4UIcmdWithAnInteger* fNofParallelRunsCmd;
  // UI command to define the number of fragments defined in the file.

  G4UIcmdWithAnInteger* fParallelRunCmd;
  // UI command to choose the specific fragment where the particles are 
  // taken from.

  G4UIcmdWithAnInteger* fTimesRecycledCmd;
  // UI command to set the number of times each particle is recycled
  // (not repeated).

  G4UIcmdWith3VectorAndUnit* fPhspGlobalTranslationCmd;
  // UI command to set the three-vector to move the phase-space plane globally.

  G4UIcmdWithAnInteger* fPhspRotationOrderCmd;
  // UI command to set the order in which the rotations are performed.

  G4UIcmdWithADoubleAndUnit* fRotXCmd;
  // UI command to set the rotation angle around X axis.

  G4UIcmdWithADoubleAndUnit* fRotYCmd;
  // UI command to set the rotation angle around Y axis.

  G4UIcmdWithADoubleAndUnit* fRotZCmd;
  // UI command to set the rotation angle around Z axis.

  G4UIcmdWith3VectorAndUnit* fIsocenterPosCmd;
  // UI command to set where the isocenter is.

  G4UIcmdWithABool* fAxialSymmetryXCmd;
  // UI command to turn on/off the rotational symmetry around X.

  G4UIcmdWithABool* fAxialSymmetryYCmd;
  // UI command to turn on/off the rotational symmetry around Y.

  G4UIcmdWithABool* fAxialSymmetryZCmd;
  // UI command to turn on/off the rotational symmetry around Z.

  G4UIcmdWithABool* fTrimEnableCmd;
  G4UIcmdWithABool* fTrimDirectionEnableCmd;
  G4UIcmdWithABool* fTrimTypeEnableCmd;
  G4UIcmdWithABool* fTrimAreaEnableCmd;
  G4UIcmdWithABool* fTrimEnergyEnableCmd;
  G4UIcmdWithABool* fTrimCompactEventsCmd;
  G4UIcmdWith3Vector* fTrimDirectionMinCmd;
  G4UIcmdWith3Vector* fTrimDirectionMaxCmd;
  G4UIcmdWith3VectorAndUnit* fTrimAreaMinCmd;
  G4UIcmdWith3VectorAndUnit* fTrimAreaMaxCmd;
  G4UIcmdWithADoubleAndUnit* fTrimEnergyMinCmd;
  G4UIcmdWithADoubleAndUnit* fTrimEnergyMaxCmd;
  G4UIcmdWithAString* fTrimTypesCmd;
};
#endif

