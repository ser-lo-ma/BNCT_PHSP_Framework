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


#include "G4IAEAphspReaderMessenger.hh"
#include "G4IAEAphspReader.hh"

#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"


G4IAEAphspReaderMessenger::G4IAEAphspReaderMessenger(G4IAEAphspReader* reader)
  :fIAEAphspReader(reader)
{
  fPhaseSpaceDir = new G4UIdirectory("/IAEAphspReader/");
  fPhaseSpaceDir
    ->SetGuidance("Commands for the IAEA phase-space file management.");


  fVerboseCmd = new G4UIcmdWithAnInteger("/IAEAphspReader/verbose", this);
  fVerboseCmd->SetGuidance("Set verbose level of G4IAEAphspReader class");
  fVerboseCmd->SetParameterName("value", false);
  fVerboseCmd->SetRange("value >= 0");
  fVerboseCmd->AvailableForStates(G4State_Idle);

  fNofParallelRunsCmd =
    new G4UIcmdWithAnInteger("/IAEAphspReader/numberOfParallelRuns", this);
  fNofParallelRunsCmd
    ->SetGuidance("Select the number of fragments N in which the phase-space");
  fNofParallelRunsCmd
    ->SetGuidance(" file is divided into.");
  fNofParallelRunsCmd->SetParameterName("N", false);
  fNofParallelRunsCmd->SetRange("N > 0");
  fNofParallelRunsCmd->AvailableForStates(G4State_Idle);

  fParallelRunCmd =
    new G4UIcmdWithAnInteger("/IAEAphspReader/parallelRun", this);
  fParallelRunCmd->
    SetGuidance("Use the fragment F (of a total of N) from which particles");
  fParallelRunCmd->SetGuidance(" are extracted. (1 <= F <= N).");
  fParallelRunCmd->SetParameterName("frag", false);
  fParallelRunCmd->SetRange("frag > 0");
  fParallelRunCmd->AvailableForStates(G4State_Idle);

  fTimesRecycledCmd =
    new G4UIcmdWithAnInteger("/IAEAphspReader/recycling", this);
  fTimesRecycledCmd
    ->SetGuidance("Select the number of times that each particle is reused.");
  fTimesRecycledCmd
    ->SetGuidance("(Caution: 1 means that each particle is used twice.)");
  fTimesRecycledCmd->SetParameterName("choice", false);
  fTimesRecycledCmd->SetRange("choice >= 0");
  fTimesRecycledCmd->AvailableForStates(G4State_Idle);

  fPhspGlobalTranslationCmd = 
    new G4UIcmdWith3VectorAndUnit("/IAEAphspReader/translate", this);
  fPhspGlobalTranslationCmd->SetGuidance("Set the translation components.");
  fPhspGlobalTranslationCmd->SetParameterName("x0", "y0", "z0", false);
  fPhspGlobalTranslationCmd->SetDefaultUnit("cm");
  fPhspGlobalTranslationCmd->SetUnitCandidates("mm cm m");
  fPhspGlobalTranslationCmd->AvailableForStates(G4State_Idle);

  fPhspRotationOrderCmd = 
    new G4UIcmdWithAnInteger("/IAEAphspReader/rotationOrder", this);
  fPhspRotationOrderCmd
    ->SetGuidance("Select the order in which the rotations are performed.");
  fPhspRotationOrderCmd
    ->SetGuidance("1 means X axis, 2 means Y axis and 3 means Z axis.");
  fPhspRotationOrderCmd
    ->SetGuidance("The argument must be a 3-digit integer without repetition.");
  fPhspRotationOrderCmd->SetParameterName("choice", false);
  fPhspRotationOrderCmd->AvailableForStates(G4State_Idle);

  fRotXCmd = new G4UIcmdWithADoubleAndUnit("/IAEAphspReader/rotateX", this);
  fRotXCmd->SetGuidance("Set the rotation angle around the global X axis.");
  fRotXCmd->SetParameterName("angle", false);
  fRotXCmd->SetDefaultUnit("deg");
  fRotXCmd->SetUnitCandidates("deg rad");
  fRotXCmd->AvailableForStates(G4State_Idle);

  fRotYCmd = new G4UIcmdWithADoubleAndUnit("/IAEAphspReader/rotateY", this);
  fRotYCmd->SetGuidance("Set the rotation angle around the global Y axis.");
  fRotYCmd->SetParameterName("angle", false);
  fRotYCmd->SetDefaultUnit("deg");
  fRotYCmd->SetUnitCandidates("deg rad");
  fRotYCmd->AvailableForStates(G4State_Idle);

  fRotZCmd = new G4UIcmdWithADoubleAndUnit("/IAEAphspReader/rotateZ", this);
  fRotZCmd->SetGuidance("Set the rotation angle around the global Z axis.");
  fRotZCmd->SetParameterName("angle", false);
  fRotZCmd->SetDefaultUnit("deg");
  fRotZCmd->SetUnitCandidates("deg rad");
  fRotZCmd->AvailableForStates(G4State_Idle);

  fIsocenterPosCmd = 
    new G4UIcmdWith3VectorAndUnit("/IAEAphspReader/isocenterPosition", this);
  fIsocenterPosCmd->SetGuidance("Set the isocenter position.");
  fIsocenterPosCmd->SetParameterName("Xic", "Yic", "Zic", false);
  fIsocenterPosCmd->SetDefaultUnit("cm");
  fIsocenterPosCmd->SetUnitCandidates("mm cm m");
  fIsocenterPosCmd->AvailableForStates(G4State_Idle);

  fAxialSymmetryXCmd =
    new G4UIcmdWithABool("/IAEAphspReader/axialSymmetryX", this);
  fAxialSymmetryXCmd
    ->SetGuidance("Command to take into account rotational symmetry around X");
  fAxialSymmetryXCmd->SetParameterName("choice", true);
  fAxialSymmetryXCmd->SetDefaultValue(true);
  fAxialSymmetryXCmd->AvailableForStates(G4State_Idle);

  fAxialSymmetryYCmd =
    new G4UIcmdWithABool("/IAEAphspReader/axialSymmetryY", this);
  fAxialSymmetryYCmd
    ->SetGuidance("Command to take into account rotational symmetry around Y");
  fAxialSymmetryYCmd->SetParameterName("choice", true);
  fAxialSymmetryYCmd->SetDefaultValue(true);
  fAxialSymmetryYCmd->AvailableForStates(G4State_Idle);

  fAxialSymmetryZCmd =
    new G4UIcmdWithABool("/IAEAphspReader/axialSymmetryZ", this);
  fAxialSymmetryZCmd
    ->SetGuidance("Command to take into account rotational symmetry around Z");
  fAxialSymmetryZCmd->SetParameterName("choice", true);
  fAxialSymmetryZCmd->SetDefaultValue(true);
  fAxialSymmetryZCmd->AvailableForStates(G4State_Idle);

  fTrimEnableCmd = new G4UIcmdWithABool("/IAEAphspReader/trimEnable", this);
  fTrimEnableCmd->SetGuidance("Enable/disable all reader-side trimming.");
  fTrimEnableCmd->SetParameterName("value", false);
  fTrimEnableCmd->SetDefaultValue(false);
  fTrimEnableCmd->AvailableForStates(G4State_Idle);

  fTrimDirectionEnableCmd =
    new G4UIcmdWithABool("/IAEAphspReader/trimDirectionEnable", this);
  fTrimDirectionEnableCmd
    ->SetGuidance("Enable/disable direction filter using trimDirectionMin/Max.");
  fTrimDirectionEnableCmd->SetParameterName("value", false);
  fTrimDirectionEnableCmd->SetDefaultValue(false);
  fTrimDirectionEnableCmd->AvailableForStates(G4State_Idle);

  fTrimTypeEnableCmd =
    new G4UIcmdWithABool("/IAEAphspReader/trimTypeEnable", this);
  fTrimTypeEnableCmd
    ->SetGuidance("Enable/disable particle-type filter using trimTypes.");
  fTrimTypeEnableCmd->SetParameterName("value", false);
  fTrimTypeEnableCmd->SetDefaultValue(false);
  fTrimTypeEnableCmd->AvailableForStates(G4State_Idle);

  fTrimAreaEnableCmd =
    new G4UIcmdWithABool("/IAEAphspReader/trimAreaEnable", this);
  fTrimAreaEnableCmd
    ->SetGuidance("Enable/disable coordinate filter using trimAreaMin/Max.");
  fTrimAreaEnableCmd->SetParameterName("value", false);
  fTrimAreaEnableCmd->SetDefaultValue(false);
  fTrimAreaEnableCmd->AvailableForStates(G4State_Idle);

  fTrimCompactEventsCmd =
    new G4UIcmdWithABool("/IAEAphspReader/trimCompactEvents", this);
  fTrimCompactEventsCmd
    ->SetGuidance("If true, skip empty histories and compact accepted primaries into each Geant4 event.");
  fTrimCompactEventsCmd->SetParameterName("value", false);
  fTrimCompactEventsCmd->SetDefaultValue(false);
  fTrimCompactEventsCmd->AvailableForStates(G4State_Idle);

  fTrimDirectionMinCmd =
    new G4UIcmdWith3Vector("/IAEAphspReader/trimDirectionMin", this);
  fTrimDirectionMinCmd->SetGuidance("Set minimum (u,v,w) accepted by direction filter.");
  fTrimDirectionMinCmd->SetParameterName("uMin", "vMin", "wMin", false);
  fTrimDirectionMinCmd->AvailableForStates(G4State_Idle);

  fTrimDirectionMaxCmd =
    new G4UIcmdWith3Vector("/IAEAphspReader/trimDirectionMax", this);
  fTrimDirectionMaxCmd->SetGuidance("Set maximum (u,v,w) accepted by direction filter.");
  fTrimDirectionMaxCmd->SetParameterName("uMax", "vMax", "wMax", false);
  fTrimDirectionMaxCmd->AvailableForStates(G4State_Idle);

  fTrimAreaMinCmd =
    new G4UIcmdWith3VectorAndUnit("/IAEAphspReader/trimAreaMin", this);
  fTrimAreaMinCmd->SetGuidance("Set minimum accepted original coordinates (x,y,z).");
  fTrimAreaMinCmd->SetParameterName("xMin", "yMin", "zMin", false);
  fTrimAreaMinCmd->SetDefaultUnit("cm");
  fTrimAreaMinCmd->SetUnitCandidates("mm cm m");
  fTrimAreaMinCmd->AvailableForStates(G4State_Idle);

  fTrimAreaMaxCmd =
    new G4UIcmdWith3VectorAndUnit("/IAEAphspReader/trimAreaMax", this);
  fTrimAreaMaxCmd->SetGuidance("Set maximum accepted original coordinates (x,y,z).");
  fTrimAreaMaxCmd->SetParameterName("xMax", "yMax", "zMax", false);
  fTrimAreaMaxCmd->SetDefaultUnit("cm");
  fTrimAreaMaxCmd->SetUnitCandidates("mm cm m");
  fTrimAreaMaxCmd->AvailableForStates(G4State_Idle);

  fTrimEnergyEnableCmd =
    new G4UIcmdWithABool("/IAEAphspReader/trimEnergyEnable", this);
  fTrimEnergyEnableCmd
    ->SetGuidance("Enable/disable kinetic-energy filter using trimEnergyMin/Max.");
  fTrimEnergyEnableCmd->SetParameterName("value", false);
  fTrimEnergyEnableCmd->SetDefaultValue(false);
  fTrimEnergyEnableCmd->AvailableForStates(G4State_Idle);

  fTrimEnergyMinCmd =
    new G4UIcmdWithADoubleAndUnit("/IAEAphspReader/trimEnergyMin", this);
  fTrimEnergyMinCmd->SetGuidance("Set minimum accepted kinetic energy (inclusive).");
  fTrimEnergyMinCmd->SetParameterName("eMin", false);
  fTrimEnergyMinCmd->SetRange("eMin >= 0.");
  fTrimEnergyMinCmd->SetDefaultUnit("MeV");
  fTrimEnergyMinCmd->AvailableForStates(G4State_Idle);

  fTrimEnergyMaxCmd =
    new G4UIcmdWithADoubleAndUnit("/IAEAphspReader/trimEnergyMax", this);
  fTrimEnergyMaxCmd->SetGuidance("Set maximum accepted kinetic energy (inclusive).");
  fTrimEnergyMaxCmd->SetParameterName("eMax", false);
  fTrimEnergyMaxCmd->SetRange("eMax >= 0.");
  fTrimEnergyMaxCmd->SetDefaultUnit("MeV");
  fTrimEnergyMaxCmd->AvailableForStates(G4State_Idle);

  fTrimTypesCmd = new G4UIcmdWithAString("/IAEAphspReader/trimTypes", this);
  fTrimTypesCmd->SetGuidance("Set allowed particle types as comma/space list.");
  fTrimTypesCmd->SetGuidance("Examples: gamma,e-   or   1 2 3");
  fTrimTypesCmd->SetParameterName("types", false);
  fTrimTypesCmd->AvailableForStates(G4State_Idle);
}


G4IAEAphspReaderMessenger::~G4IAEAphspReaderMessenger()
{ 
  delete fPhaseSpaceDir;
  delete fVerboseCmd;
  delete fNofParallelRunsCmd;
  delete fParallelRunCmd;
  delete fTimesRecycledCmd;
  delete fPhspGlobalTranslationCmd;
  delete fPhspRotationOrderCmd;
  delete fRotXCmd;
  delete fRotYCmd;
  delete fRotZCmd;
  delete fIsocenterPosCmd;
  delete fAxialSymmetryXCmd;
  delete fAxialSymmetryYCmd;
  delete fAxialSymmetryZCmd;
  delete fTrimEnableCmd;
  delete fTrimDirectionEnableCmd;
  delete fTrimTypeEnableCmd;
  delete fTrimAreaEnableCmd;
  delete fTrimCompactEventsCmd;
  delete fTrimDirectionMinCmd;
  delete fTrimDirectionMaxCmd;
  delete fTrimAreaMinCmd;
  delete fTrimAreaMaxCmd;
  delete fTrimEnergyEnableCmd;
  delete fTrimEnergyMinCmd;
  delete fTrimEnergyMaxCmd;
  delete fTrimTypesCmd;
}


void G4IAEAphspReaderMessenger::SetNewValue(G4UIcommand* command,
                                            G4String newValue)
{
  if( command == fVerboseCmd )
    fIAEAphspReader->SetVerbose(fVerboseCmd->GetNewIntValue(newValue));

  else if( command == fNofParallelRunsCmd )
    fIAEAphspReader
      ->SetTotalParallelRuns(fNofParallelRunsCmd->GetNewIntValue(newValue));

  else if( command == fParallelRunCmd )
    fIAEAphspReader->SetParallelRun(fParallelRunCmd->GetNewIntValue(newValue));

  else if( command == fTimesRecycledCmd )
    fIAEAphspReader
      ->SetTimesRecycled(fTimesRecycledCmd->GetNewIntValue(newValue) );

  else if( command == fPhspGlobalTranslationCmd )
    fIAEAphspReader
      ->SetGlobalPhspTranslation(fPhspGlobalTranslationCmd
                                 ->GetNew3VectorValue(newValue) );

  else if( command == fPhspRotationOrderCmd )
    fIAEAphspReader
      ->SetRotationOrder(fPhspRotationOrderCmd->GetNewIntValue(newValue) );

  else if( command == fRotXCmd )
    fIAEAphspReader->SetRotationX( fRotXCmd->GetNewDoubleValue(newValue) );

  else if( command == fRotYCmd )
    fIAEAphspReader->SetRotationY( fRotYCmd->GetNewDoubleValue(newValue) );

  else if( command == fRotZCmd )
    fIAEAphspReader->SetRotationZ( fRotZCmd->GetNewDoubleValue(newValue) );

  else if( command == fIsocenterPosCmd )
    fIAEAphspReader
      ->SetIsocenterPosition(fIsocenterPosCmd->GetNew3VectorValue(newValue));

  else if( command == fAxialSymmetryXCmd )
    fIAEAphspReader
      ->SetAxialSymmetryX( fAxialSymmetryXCmd->GetNewBoolValue(newValue));

  else if( command == fAxialSymmetryYCmd )
    fIAEAphspReader
      ->SetAxialSymmetryY( fAxialSymmetryYCmd->GetNewBoolValue(newValue) );

  else if( command == fAxialSymmetryZCmd )
    fIAEAphspReader
      ->SetAxialSymmetryZ( fAxialSymmetryZCmd->GetNewBoolValue(newValue) );

  else if( command == fTrimEnableCmd )
    fIAEAphspReader->SetTrimEnabled(fTrimEnableCmd->GetNewBoolValue(newValue));

  else if( command == fTrimDirectionEnableCmd )
    fIAEAphspReader
      ->SetTrimDirectionEnabled(fTrimDirectionEnableCmd->GetNewBoolValue(newValue));

  else if( command == fTrimTypeEnableCmd )
    fIAEAphspReader
      ->SetTrimTypeEnabled(fTrimTypeEnableCmd->GetNewBoolValue(newValue));

  else if( command == fTrimAreaEnableCmd )
    fIAEAphspReader
      ->SetTrimAreaEnabled(fTrimAreaEnableCmd->GetNewBoolValue(newValue));

  else if( command == fTrimCompactEventsCmd )
    fIAEAphspReader
      ->SetTrimCompactEvents(fTrimCompactEventsCmd->GetNewBoolValue(newValue));

  else if( command == fTrimDirectionMinCmd )
    fIAEAphspReader
      ->SetTrimDirectionMin(fTrimDirectionMinCmd->GetNew3VectorValue(newValue));

  else if( command == fTrimDirectionMaxCmd )
    fIAEAphspReader
      ->SetTrimDirectionMax(fTrimDirectionMaxCmd->GetNew3VectorValue(newValue));

  else if( command == fTrimAreaMinCmd )
    fIAEAphspReader
      ->SetTrimAreaMin(fTrimAreaMinCmd->GetNew3VectorValue(newValue));

  else if( command == fTrimAreaMaxCmd )
    fIAEAphspReader
      ->SetTrimAreaMax(fTrimAreaMaxCmd->GetNew3VectorValue(newValue));

  else if( command == fTrimEnergyEnableCmd )
    fIAEAphspReader
      ->SetTrimEnergyEnabled(fTrimEnergyEnableCmd->GetNewBoolValue(newValue));

  else if( command == fTrimEnergyMinCmd )
    fIAEAphspReader
      ->SetTrimEnergyMin(fTrimEnergyMinCmd->GetNewDoubleValue(newValue));

  else if( command == fTrimEnergyMaxCmd )
    fIAEAphspReader
      ->SetTrimEnergyMax(fTrimEnergyMaxCmd->GetNewDoubleValue(newValue));

  else if( command == fTrimTypesCmd )
    fIAEAphspReader->SetTrimTypes(newValue);

}
