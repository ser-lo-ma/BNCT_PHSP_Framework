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
// Example author: M.A. Cortes-Giraldo, Universidad de Sevilla
//
// Modified by S. Lopez Martinez, University College London, 2026,
// for the BNCT phase-space framework:
//   BNCT application (phantoms, neutron beams, QGSP_BIC_HP with radioactive
//   decay); thread count and seed from the command line; --vis mode.
//
// Reference paper:
//   M.A. Cortes-Giraldo et al., Int J Radiat Biol 88(1-2): 200-208 (2012)
//   (doi: 10.3109/09553002.2011.627977)
//
// The iaea_phsp routines are available at the IAEAphsp project website:
//   https://www-nds.iaea.org/phsp/phsp.htmlx
//

#include "globals.hh"
#include <cstdlib>
#include "Randomize.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4PhysListFactory.hh"
#include "G4OpticalPhysics.hh"
#include "G4HadronicParameters.hh"
#include "G4ScoringManager.hh"


#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv) {

  // Optional visual batch mode executes a macro, then leaves the UI open for
  // inspecting and capturing its accumulated event display.
  // Usage: phantoms_sim --vis <macro> [nThreads] [seed] (MT)
  //        phantoms_sim --vis <macro> [seed] (sequential)
  const G4bool visualBatch = argc >= 2 && G4String(argv[1]) == "--vis";
  const G4int argOffset = visualBatch ? 1 : 0;
  const G4int macroIndex = visualBatch ? 2 : 1;
  if (visualBatch && argc <= macroIndex) {
    G4cerr << "Usage: phantoms_sim --vis <macro> [nThreads] [seed] (MT)\n"
           << "       phantoms_sim --vis <macro> [seed] (sequential)\n";
    return 2;
  }

  // Detect interactive mode and define a UI session. In visual batch mode pass
  // only argv[0], so run arguments are not treated as UI options.
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  } else if (visualBatch) {
    char* uiArgv[] = {argv[0]};
    ui = new G4UIExecutive(1, uiArgv);
  }

  //choose the Random engine
  // MixMax, not Ranecu (2026-10-05): RanecuEngine::setSeed(n) only selects one
  // of 215 tabulated seed pairs (n % 215), so array-job tasks seeded base+task
  // repeat every 215 tasks -- identical events in a multi-chunk production.
  // MixMax (the Geant4 default) uses the full seed.
  G4Random::setTheEngine(new CLHEP::MixMaxRng);

  // Use a deterministic default seed so runs are reproducible unless overridden.
  G4long randomSeed = 123456789L;


#ifdef G4MULTITHREADED
  // Argument order in MT mode:
  //   ./phantoms_sim [macro] [nThreads] [seed]
  if (argc >= 4 + argOffset)
    randomSeed = std::strtol(argv[3 + argOffset], nullptr, 10);   // long: ConvertToInt overflows above 2^31
#else
  // Argument order in sequential mode:
  //   ./phantoms_sim [macro] [seed]
  if (argc >= 3 + argOffset)
    randomSeed = std::strtol(argv[2 + argOffset], nullptr, 10);
#endif

  G4Random::setTheSeed(randomSeed);
  G4cout << "===== Random seed: " << randomSeed << " =====" << G4endl;

  // Construct the default run manager
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager;
  G4int nThreads = 25;
  // Select number of threads from arguments
  if (argc >= 3 + argOffset)
    nThreads = G4UIcommand::ConvertToInt(argv[2 + argOffset]);
  runManager->SetNumberOfThreads(nThreads);
  G4cout << "===== Simulation started with "
         <<  runManager->GetNumberOfThreads() << " threads =====" << G4endl;
#else
  G4RunManager* runManager = new G4RunManager();
#endif

  // Enable UI scoring commands (/score/*), used for scoring meshes from macro files.
  G4ScoringManager* scoringManager = G4ScoringManager::GetScoringManager();
  scoringManager->SetVerboseLevel(1);

  //set mandatory initialization classes
  runManager->SetUserInitialization(new DetectorConstruction());

  // Establish physics list
  G4PhysListFactory factory;
  G4VModularPhysicsList *phys = factory.GetReferencePhysList("QGSP_BIC_HP");
  //phys->RegisterPhysics(new G4OpticalPhysics());
  runManager->SetUserInitialization(phys);

  // VERY IMPORTANT LINE, WITHOUT THIS RADIOACTIVE DECAYS WILL **NOT** WORK IN THE LATEST VERSIONS OF GEANT4
  G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay(1.0e+60 * CLHEP::year);


  // set user action classes
  runManager->SetUserInitialization(new ActionInitialization());

  //initialize visualization
  G4VisManager* visManager = nullptr;

  //get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if (ui)  {
   // Interactive or visual batch mode.
   visManager = new G4VisExecutive;
   visManager->Initialize();
   if (visualBatch) {
     UImanager->ApplyCommand(G4String("/control/execute ") + argv[macroIndex]);
   } else {
     UImanager->ApplyCommand("/control/execute vis.mac");
   }
   ui->SessionStart();
   delete ui;
  }
  else  {
  //batch mode
   G4String command = "/control/execute ";
   G4String fileName = argv[macroIndex];
   UImanager->ApplyCommand(command+fileName);
  }

  //job termination
  if (visManager) delete visManager;
  delete runManager;
}

