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
//   registration of phase-space scoring volumes and boundaries; the reader
//   takes the thread count from the master run manager.
//

#ifndef ActionInitialization_h
#define ActionInitialization_h 1


#include "G4VUserActionInitialization.hh"
#include "globals.hh"

#include "G4IAEAphspWriterStack.hh"
#include <utility>
#include <vector>

class ActionInitializationMessenger;


//------------------------------------------------------------------------------

class ActionInitialization : public G4VUserActionInitialization
{
public:
  ActionInitialization();
  virtual ~ActionInitialization() override;

  virtual void BuildForMaster() const override;
  virtual void Build() const override;

  
  // Set-Get methods

  void SetIAEAphspReader(const G4String& name);
  void SetIAEAphspWriterPrefix(const G4String& name);

  void AddScoringVolume(const G4String& postVolName);
  void AddScoringVolumeBoundary(const G4String& preVolName,
                                const G4String& postVolName);


private:

  // IAEAphsp-related data members
  G4String fIAEAphspReaderName;
  G4String fIAEAphspWriterNamePrefix;
  std::vector<std::pair<G4String, G4String>>* fScoringVolumePairs;

  // Messenger class needed for IAEAphsp commands
  ActionInitializationMessenger* fMessenger;

};

#endif
