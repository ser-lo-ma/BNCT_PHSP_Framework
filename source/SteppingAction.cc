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
//   phase-space scoring at volume boundaries; per-thread check of the volume
//   in which each primary starts.
//


#include "SteppingAction.hh"

#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Threading.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"

#include "IAEAphspRun.hh"
#include "G4IAEAphspWriterStack.hh"


SteppingAction::~SteppingAction()
{
  const G4long n = fPrimInWorld + fPrimElsewhere;
  if (n > 0)
    G4cout << "Primary start volumes (thread " << G4Threading::G4GetThreadId()
           << "): " << fPrimInWorld << " of " << n << " in physWorld, "
           << fPrimElsewhere << " elsewhere" << G4endl;
}


void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  const G4Track* track = aStep->GetTrack();
  if (track->GetParentID() == 0 && track->GetCurrentStepNumber() == 1) {
    const G4VPhysicalVolume* pv = aStep->GetPreStepPoint()->GetPhysicalVolume();
    if (pv && pv->GetName() == "physWorld") {
      ++fPrimInWorld;
    }
    else {
      if (fPrimElsewhere < 5) {
        const G4ThreeVector& x = aStep->GetPreStepPoint()->GetPosition();
        G4cout << "WARNING: primary started in "
               << (pv ? pv->GetName() : G4String("no volume"))
               << " at (" << x.x() / cm << ", " << x.y() / cm << ", "
               << x.z() / cm << ") cm" << G4endl;
      }
      ++fPrimElsewhere;
    }
  }

  const IAEAphspRun* aRun =
    static_cast<const IAEAphspRun*>( G4RunManager::GetRunManager()
				     ->GetCurrentRun() );
  auto phspWriterStack = aRun->GetIAEAphspWriterStack();
  if (phspWriterStack)
    phspWriterStack->StoreParticleIfEligible(aStep);
}
