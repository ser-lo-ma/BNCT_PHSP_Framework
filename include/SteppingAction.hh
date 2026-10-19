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

#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4Step;


class SteppingAction: public G4UserSteppingAction
{
public:
  SteppingAction() = default;

  // Prints the primary start-volume tally (see below) for this thread.
  virtual ~SteppingAction() override;

  // To store eligible particles in the IAEAphspWriterstack
  virtual void UserSteppingAction(const G4Step* step) override;

private:
  // Where each primary takes its first step. A phase-space replay places
  // particles exactly on the phantom boundary moving outward; they must start
  // in physWorld, never inside the phantom.
  G4long fPrimInWorld = 0;
  G4long fPrimElsewhere = 0;
};
#endif
