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
//   General Particle Source (GPS) neutron beam, with the IAEA phase-space
//   reader as the alternative source.
//

#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4Event;
class G4GeneralParticleSource;

class G4IAEAphspReader;
class DetectorConstruction;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:

  // The constructor defines a GeneralParticleSource object, which allows
  // defining beams from /gps/* macro commands.
  // It also needs a pointer to G4IAEAphspReader object in case we need to
  // read particles from an IAEAphsp file
  PrimaryGeneratorAction(const G4int threads);

  //The destructor. It deletes the configured source objects.
  virtual ~PrimaryGeneratorAction() override;
 
  //Generates the primary event from either GPS or IAEA phase-space.
  void GeneratePrimaries(G4Event* anEvent) override;

  void SetIAEAphspReader(const G4String filename);

  inline G4IAEAphspReader* GetIAEAphspReader() const  {return fIAEAphspReader;}


private:

  // Phase space reader
  G4IAEAphspReader* fIAEAphspReader = nullptr;

  G4int             fThreads;
  G4String          fIAEAphspReaderName;

  G4GeneralParticleSource* fParticleSource;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#endif
