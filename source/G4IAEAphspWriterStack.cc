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
// Author: M.A. Cortes-Giraldo
//
// Modified by S. Lopez Martinez, University College London, 2026,
// for the BNCT phase-space framework:
//   phase-space scoring at volume boundaries (pre -> post) or at volume
//   entries, instead of planes at constant z.
//
// 2025-08-27: Its objects work in local runs, their mission is to store the
//   info of the particles to be written in the IAEAphsp output files at
//   the end of the run. In other words, they constitute a stack for
//   IAEAphsp particles until the run finishes, when the IAEAphsp file is
//   actually written.
//


#include "G4IAEAphspWriterStack.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4IAEAphspWriter.hh"

#include <set>
#include <utility>
#include <vector>


//==============================================================================

G4IAEAphspWriterStack::G4IAEAphspWriterStack(const G4String filename)
{
  fFileName = filename;
  fScoringVolumePairs = new std::vector<std::pair<G4String, G4String>>;
  fIncrNumberVec = new std::vector<G4int>;
  fPassingTracksVec = new std::vector< std::set<G4int>* >;
  fPDGMtrx = new std::vector< std::vector<G4int>* >;
  fPosMtrx = new std::vector< std::vector<G4ThreeVector>* >;
  fMomMtrx = new std::vector< std::vector<G4ThreeVector>* >;
  fEneMtrx = new std::vector< std::vector<G4double>* >;
  fWtMtrx = new std::vector< std::vector<G4double>* >;
  fNstatMtrx = new std::vector< std::vector<G4int>* >;

  G4cout << "G4IAEAphspWriterStack object constructed for files \""
   << fFileName << "_<boundary>.IAEA*\"" << G4endl;
}


//==============================================================================

G4IAEAphspWriterStack::~G4IAEAphspWriterStack()
{
  if (fScoringVolumePairs) delete fScoringVolumePairs;
  if (fIncrNumberVec)    delete fIncrNumberVec;
  if (fPassingTracksVec) delete fPassingTracksVec;
  if (fPDGMtrx)    delete fPDGMtrx;
  if (fPosMtrx)    delete fPosMtrx;
  if (fMomMtrx)    delete fMomMtrx;
  if (fEneMtrx)    delete fEneMtrx;
  if (fWtMtrx)     delete fWtMtrx;
  if (fNstatMtrx)  delete fNstatMtrx;
}


//==============================================================================

void G4IAEAphspWriterStack::AddScoringVolume(const G4String& postVolName)
{
  fScoringVolumePairs->push_back(std::make_pair(G4String(""), postVolName));
  G4cout << "G4IAEAphspWriterStack: Registered scoring volume '"
	 << postVolName << "'." << G4endl;
}


//==============================================================================

void G4IAEAphspWriterStack::AddScoringVolumeBoundary(const G4String& preVolName,
							     const G4String& postVolName)
{
  fScoringVolumePairs->push_back(std::make_pair(preVolName, postVolName));
  G4cout << "G4IAEAphspWriterStack: Registered scoring boundary '"
	 << preVolName << " -> " << postVolName << "'." << G4endl;
}


//==============================================================================

void G4IAEAphspWriterStack::ClearScoringVolumePairs()
{
  G4cout << "G4IAEAphspWriterStack: Removing all registered scoring volumes!"
	 << G4endl;
  fScoringVolumePairs->clear();
}


//==============================================================================

void G4IAEAphspWriterStack::SetDataFromWriter(const G4IAEAphspWriter* writer)
{
  if (!writer) {
    G4ExceptionDescription msg;
    msg <<  "No G4IAEAphspWriter has been constructed!" << G4endl;
    G4Exception("G4IAEAphspWriterStack::SetDataFromWriter()",
		"IAEAphspWriterStack001", FatalException, msg );
    return;
  }
  else {
    fFileName = writer->GetFileName();

    if (writer->GetScoringVolumePairs()->size() > 0) {
      (*fScoringVolumePairs) = *(writer->GetScoringVolumePairs()); // copy objects, not pointers

      G4cout << "G4IAEAphspWriterStack::fFileName = " << fFileName << G4endl;
      G4cout << "G4IAEAphspWriterStack::fScoringVolumePairs->size() = "
	     << fScoringVolumePairs->size() << G4endl;
    }
    else {
      G4ExceptionDescription msg;
      msg << "No scoring volume or boundary has been defined!" << G4endl;
      G4Exception("G4IAEAphspWriterStack::SetDataFromWriter()",
		  "IAEAphspWriterStack002", FatalErrorInArgument, msg );
      return;
    }
  }
}


//==============================================================================

void G4IAEAphspWriterStack::PrepareRun()
{
  size_t nZphsps = fScoringVolumePairs->size();

  fIncrNumberVec->reserve(nZphsps);
  fPassingTracksVec->reserve(nZphsps);
  fPDGMtrx->reserve(nZphsps);
  fPosMtrx->reserve(nZphsps);
  fMomMtrx->reserve(nZphsps);
  fEneMtrx->reserve(nZphsps);
  fWtMtrx->reserve(nZphsps);
  fNstatMtrx->reserve(nZphsps);
  
  for (size_t ii = 0; ii < nZphsps; ii++) {
    fIncrNumberVec->push_back(0);

    auto aSet = new std::set<G4int>;
    fPassingTracksVec->push_back(aSet);
    auto pdgVec = new std::vector<G4int>;
    fPDGMtrx->push_back(pdgVec);
    auto posVec = new std::vector<G4ThreeVector>;
    fPosMtrx->push_back(posVec);
    auto momVec = new std::vector<G4ThreeVector>;
    fMomMtrx->push_back(momVec);
    auto eneVec = new std::vector<G4double>;
    fEneMtrx->push_back(eneVec);
    auto wtVec = new std::vector<G4double>;
    fWtMtrx->push_back(wtVec);
    auto nstatVec = new std::vector<G4int>;
    fNstatMtrx->push_back(nstatVec);
  }
  G4cout << "G4IAEAphspWriterStack::PrepareRun() done!" << G4endl;
}


//==============================================================================

void G4IAEAphspWriterStack::PrepareNextEvent()
{
  // Update all the incremental history numbers.
  for ( auto& ii : (*fIncrNumberVec) )
    ii++;

  // Remove the track ID's stored during this event.
  for ( auto& trackIDs : (*fPassingTracksVec) )
    trackIDs->clear();

  // -- DEBUG!!
  // G4cout << "G4IAEAphspWriterStack ready for the next event!" << G4endl;
}



//==============================================================================

void G4IAEAphspWriterStack::StoreParticleIfEligible(const G4Step* aStep)
{
  const auto* preVolume = aStep->GetPreStepPoint()->GetPhysicalVolume();
  const auto* postVolume = aStep->GetPostStepPoint()->GetPhysicalVolume();

  if (!preVolume || !postVolume) {
    return;
  }

  if (aStep->GetPostStepPoint()->GetStepStatus() != fGeomBoundary) {
    return;
  }

  const G4String preName = preVolume->GetName();
  const G4String postName = postVolume->GetName();
  const G4int trackID = aStep->GetTrack()->GetTrackID();

  size_t boundaryIdx = 0;
  for (const auto& scoringPair : (*fScoringVolumePairs)) {
    const G4bool matchesPost = scoringPair.second == postName;
    const G4bool matchesPre = scoringPair.first.empty() || scoringPair.first == preName;

    if (matchesPre && matchesPost) {
      auto* passingTracks = (*fPassingTracksVec)[boundaryIdx];
      if (passingTracks->find(trackID) == passingTracks->end()) {
	const G4int pdgCode =
	  aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
	if (pdgCode == 22 || pdgCode == 11 || pdgCode == -11 ||
	    pdgCode == 2112 || pdgCode == 2212)
	  StoreIAEAParticle(aStep, boundaryIdx, pdgCode);
      }
    }

    boundaryIdx++;
  }
}


//==============================================================================

void G4IAEAphspWriterStack::StoreIAEAParticle(const G4Step* aStep,
				      const G4int boundaryIndex,
					      const G4int pdgCode)
{
  const G4Track* aTrack = aStep->GetTrack();

  // Get step info
  // --------------------------------
  const G4ThreeVector postR = aStep->GetPostStepPoint()->GetPosition();
  const G4ThreeVector preR = aStep->GetPreStepPoint()->GetPosition();

  // Set kinetic energy
  G4double kinEnergy;
  if (pdgCode == 22 || pdgCode == 2112) {  // gamma or neutron
    kinEnergy = aStep->GetPostStepPoint()->GetKineticEnergy();
  }
  else if (pdgCode == 11 || pdgCode == -11 ||
	   pdgCode == 2212 ) {  // electron, positron or proton
    kinEnergy = aStep->GetPostStepPoint()->GetKineticEnergy();
  }
  else { // not a particle for the IAEA format
    G4ExceptionDescription ED;
    ED << "\"" << aTrack->GetDefinition()->GetParticleName()
       << "\" is not supported by the IAEAphsp format; not recorded.";
    G4Exception("G4IAEAphspWriterStack::StoreIAEAParticle()",
		"IAEAphspWriterStack003", JustWarning, ED );
    return;
  }

  // Position
  const G4ThreeVector phspPos = postR;

  // Momentum direction
  const G4ThreeVector phspMomDir =
    aStep->GetPreStepPoint()->GetMomentumDirection();

  // Track weight
  const G4double wt = aTrack->GetWeight();

  // n_stat value
  const G4int nStat = (*fIncrNumberVec)[boundaryIndex];

  // Store info in stacking vectors
  // ------------------------------
  ((*fPDGMtrx)[boundaryIndex])->push_back(pdgCode);
  ((*fNstatMtrx)[boundaryIndex])->push_back(nStat);
  ((*fPosMtrx)[boundaryIndex])->push_back(phspPos);
  ((*fMomMtrx)[boundaryIndex])->push_back(phspMomDir);
  ((*fEneMtrx)[boundaryIndex])->push_back(kinEnergy);
  ((*fWtMtrx)[boundaryIndex])->push_back(wt);

  // Once stored, reset the incremental history number (n_stat = 0)
  (*fIncrNumberVec)[boundaryIndex] = 0;

  // And now register this trackID to protect against multiple crossers
  (*fPassingTracksVec)[boundaryIndex]->insert( aTrack->GetTrackID() );

  // -- DEBUG!!
  // G4cout << "G4IAEAphspWriterStack: Particle stored in phsp plane ["
  // 	 << phspIndex << "] at place #" << ((*fPDGMtrx)[phspIndex])->size()
  // 	 << " with the following values:" << G4endl;
  // G4cout << "\tPDG = " << pdgCode << "   nStat = " << nStat
  // 	 << "   phspPos = " << phspPos << "   phspMomDir = " << phspMomDir
  // 	 << "   kinEnergy = " << kinEnergy << "   weight = " << wt
  // 	 << G4endl;

}


//==============================================================================

void G4IAEAphspWriterStack::ClearRunVectors()
{
  // Clear the run vectors at run termination

  fIncrNumberVec->clear();

  for (auto& trackIDs : (*fPassingTracksVec) ) delete trackIDs;
  fPassingTracksVec->clear();

  for (auto& pdgVec : (*fPDGMtrx) ) delete pdgVec;
  fPDGMtrx->clear();

  for (auto& posVec : (*fPosMtrx) ) delete posVec;
  fPosMtrx->clear();

  for (auto& momVec : (*fMomMtrx) ) delete momVec;
  fMomMtrx->clear();

  for (auto& eneVec : (*fEneMtrx) ) delete eneVec;
  fEneMtrx->clear();

  for (auto& wtVec : (*fWtMtrx) ) delete wtVec;
  fWtMtrx->clear();

  for (auto& nStatVec : (*fNstatMtrx) ) delete nStatVec;
  fNstatMtrx->clear();

  G4cout << "G4IAEAphspWriterStack run vectors cleaned!" << G4endl;
}
