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
// Based on the Geant4 advanced example IAEAphsp
// (M.A. Cortes-Giraldo, Universidad de Sevilla).
// Modified by S. Lopez Martinez, University College London, 2026,
// for the BNCT phase-space framework:
//   /action/IAEAphspWriter/scoringVolume and scoringBoundary commands,
//   replacing the constant-z planes.
//

#ifndef ActionInitializationMessenger_h
#define ActionInitializationMessenger_h 1

//....oooOO0OOooo........oooOO00OOooo........oooOO00OOooo........oooOO0OOooo....

#include "globals.hh"
#include "G4UImessenger.hh"

class ActionInitialization;

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcommand;

//....oooOO0OOooo........oooOO00OOooo........oooOO00OOooo........oooOO0OOooo....


class ActionInitializationMessenger: public G4UImessenger
{
public:

  ActionInitializationMessenger(ActionInitialization* );
  virtual ~ActionInitializationMessenger() override;
    
  void SetNewValue(G4UIcommand*, G4String) override;
    
private:

  ActionInitialization*   fAction;
    
  G4UIdirectory*             fActionDir;
  G4UIdirectory*             fIAEAphspReaderDir;
  G4UIdirectory*             fIAEAphspWriterDir;
  G4UIcmdWithAString*        fIAEAphspReaderFileCmd;
  G4UIcmdWithAString*        fIAEAphspWriterFileCmd;
  G4UIcmdWithAString*        fIAEAphspWriterVolumeCmd;
  G4UIcmdWithAString*        fIAEAphspWriterBoundaryCmd;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#endif
