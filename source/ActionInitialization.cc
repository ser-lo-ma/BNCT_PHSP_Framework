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

#include "ActionInitialization.hh"
#include "ActionInitializationMessenger.hh"

#include "globals.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "G4RunManager.hh"
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#endif
#include "G4Threading.hh"

#include "G4IAEAphspReader.hh"
#include "G4IAEAphspWriterStack.hh"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <utility>


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::ActionInitialization()
  : G4VUserActionInitialization()
{
  // Messenger
  fMessenger = new ActionInitializationMessenger(this);

  // IAEAphsp source file name (including path) for primary generator
  fIAEAphspReaderName = "";

  // Name prefix, including path, of IAEAphsp output files (default, nothing).
  fIAEAphspWriterNamePrefix = "";

  // Vector to register scoring volume boundaries.
  fScoringVolumePairs = new std::vector<std::pair<G4String, G4String>>;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization()
{
  if (fScoringVolumePairs) {
    fScoringVolumePairs->clear();
    delete fScoringVolumePairs;
  }
  if (fMessenger)           delete fMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::BuildForMaster() const
{
  // G4cout << "ActionInitialization::BuildForMaster() started" << G4endl;
  SetUserAction(new RunAction());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::Build() const
{
  // G4cout << "ActionInitialization::Build() started" << G4endl;
  G4cout << "IAEAphsp file to read is \"" << fIAEAphspReaderName << "\"."
	 << G4endl;

  // Build() runs after the macro has set /run/numberOfThreads.  Reading the
  // value here is essential: caching the run-manager default in the
  // constructor made a one-worker replay partition the input as if it still
  // had the default 25 workers, then recycle that first partition.
  // Ask the MASTER run manager: Build() runs on each worker thread, where
  // G4RunManager::GetRunManager() is that worker's own run manager and
  // always reports 1 thread, so with N > 1 workers every worker but the
  // first started reading past the end of the file (fixed 2026-10-19).
  G4int numberOfThreads = 1;
#ifdef G4MULTITHREADED
  if (G4Threading::IsMultithreadedApplication()) {
    if (const auto* master = G4MTRunManager::GetMasterRunManager())
      numberOfThreads = master->GetNumberOfThreads();
  }
#endif
  G4cout << "IAEA primary reader configured for " << numberOfThreads
         << " Geant4 worker thread(s)." << G4endl;
  PrimaryGeneratorAction* prim = new PrimaryGeneratorAction(numberOfThreads);
  if ( !fIAEAphspReaderName.empty() )   // never true in sequential mode
    prim->SetIAEAphspReader(fIAEAphspReaderName);
  SetUserAction(prim);

  RunAction* runAct = new RunAction();
  if (fIAEAphspWriterNamePrefix != "") {  // never true in sequential mode
    // Set G4IAEAphspWriterStack object for the local thread
    // and register scoring boundaries to it
    runAct->SetIAEAphspWriterStack(fIAEAphspWriterNamePrefix);

    if (fScoringVolumePairs->size() > 0) {
      for (const auto& scoringPair : (*fScoringVolumePairs)) {
	if (scoringPair.first.empty())
	  runAct->AddScoringVolume(scoringPair.second);
	else
	  runAct->AddScoringVolumeBoundary(scoringPair.first,
					     scoringPair.second);
      }
    }
    else {
      G4ExceptionDescription msg;
      msg << "IAEAphsp output file name provided, but no scoring volumes "
	  << "or boundaries have been registered!" << G4endl;
      G4Exception("ActionInitialization::Build()",
		  "ActionInit001", FatalException, msg);
    }
  }
  SetUserAction(runAct);

  SteppingAction *steppingAction = new SteppingAction();
  SetUserAction(steppingAction);
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::SetIAEAphspReader(const G4String& name)
{
  fIAEAphspReaderName = name;

  if ( !(G4Threading::IsMultithreadedApplication()) ) {
    // In sequential mode, when this command is issued, Build() has been
    // called already. Thus, we must set G4IAEAphspReader object here

    const G4VUserPrimaryGeneratorAction* basePrim =
      G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction();
    if (!basePrim) return; // No PGA yet (very unlikely in sequential)

    // 1) cast while preserving constness
    const auto* myConstPrim =
      dynamic_cast<const PrimaryGeneratorAction*>(basePrim);
    if (!myConstPrim) return;

    // 2) Drop constness to set G4IAEAphspReader object
    auto* myPrim = const_cast<PrimaryGeneratorAction*>(myConstPrim);
    myPrim->SetIAEAphspReader(name);
  }
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::SetIAEAphspWriterPrefix(const G4String& prefix)
{
  // The writer only opens its output files when the run ends, so an unwritable
  // directory used to surface after every event had been simulated (seen
  // 2026-10-05: ../phsp missing). Probe it now, while the macro is parsed.
  {
    const std::string probe = std::string(prefix) + "_writetest.tmp";
    std::ofstream f(probe);
    const bool ok = static_cast<bool>(f);
    f.close();
    std::remove(probe.c_str());
    if (!ok) {
      G4ExceptionDescription msg;
      msg << "Cannot create files with prefix '" << prefix
          << "': does the directory exist and is it writable?";
      G4Exception("ActionInitialization::SetIAEAphspWriterPrefix()",
                  "ActionInit003", FatalException, msg);
    }
  }
  fIAEAphspWriterNamePrefix = prefix;

  if ( !(G4Threading::IsMultithreadedApplication()) ) {
    // In sequential mode, when this command is issued, Build() has been
    // called already. Thus, we must set G4IAEAphspWriterStack object here
    const G4UserRunAction* baseRA =
      G4RunManager::GetRunManager()->GetUserRunAction();
    if (!baseRA) return; // No run action defined

    // 1) cast while preserving constness
    const auto* myConstRA = dynamic_cast<const RunAction*>(baseRA);
    if (!myConstRA) return;

    // 2) Drop constness to modify RunAction object status
    auto* myRA = const_cast<RunAction*>(myConstRA);
    myRA->SetIAEAphspWriterStack(prefix);

    if (fScoringVolumePairs->size() > 0) {
      for (const auto& scoringPair : (*fScoringVolumePairs)) {
	if (scoringPair.first.empty())
	  myRA->AddScoringVolume(scoringPair.second);
	else
	  myRA->AddScoringVolumeBoundary(scoringPair.first,
					 scoringPair.second);
      }
    }
  }
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::AddScoringVolume(const G4String& postVolName)
{
  fScoringVolumePairs->push_back(std::make_pair(G4String(""), postVolName));

  if ( !(G4Threading::IsMultithreadedApplication()) ) {
    // In sequential mode, when this command is issued, Build() has been
    // called already. Thus, we must set G4IAEAphspWriterStack object here
    const G4UserRunAction* baseRA =
      G4RunManager::GetRunManager()->GetUserRunAction();
    if (!baseRA) return; // No run action defined

    // 1) cast while preserving constness
    const auto* myConstRA = dynamic_cast<const RunAction*>(baseRA);
    if (!myConstRA) return;

    // 2) Drop constness to modify RunAction object status
    auto* myRA = const_cast<RunAction*>(myConstRA);
    myRA->AddScoringVolume(postVolName);
  }
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::AddScoringVolumeBoundary(const G4String& preVolName,
							  const G4String& postVolName)
{
  fScoringVolumePairs->push_back(std::make_pair(preVolName, postVolName));

  if ( !(G4Threading::IsMultithreadedApplication()) ) {
    const G4UserRunAction* baseRA =
      G4RunManager::GetRunManager()->GetUserRunAction();
    if (!baseRA) return; // No run action defined

    const auto* myConstRA = dynamic_cast<const RunAction*>(baseRA);
    if (!myConstRA) return;

    auto* myRA = const_cast<RunAction*>(myConstRA);
    myRA->AddScoringVolumeBoundary(preVolName, postVolName);
  }
}
