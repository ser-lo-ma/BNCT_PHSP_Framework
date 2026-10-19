// Author: S. Lopez Martinez, University College London
// BNCT phantom geometries (sphere, cylinder with borated rod, Snyder heads)
// for the BNCT phase-space framework. Replaces the DetectorConstruction of
// the Geant4 advanced example IAEAphsp (M.A. Cortes-Giraldo).

#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Box.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"
#include "G4GenericMessenger.hh"
#include "G4Tubs.hh"

class DetectorConstruction : public G4VUserDetectorConstruction{
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume *Construct() override;

    private:
    G4GenericMessenger *fMessenger;
    G4Material *worldMat, *Water, *boronMaterial, *brainMaterial, *boronTumour, *boronBrain, *skullMaterial, *skinMaterial, *cylinderMaterial;
    G4LogicalVolume *logicWorld, *logicPhantom, *logicHead, *logicSkull, *logicBrain, *logicTarget, *logicTarget2, *logicDebuggingDetector;

    G4VPhysicalVolume *physWorld;
    G4Box *solidWorld;
    G4double xWorld, yWorld, zWorld;
    G4double ax_brain, by_brain, cz_brain, ax_skull, by_skull, cz_skull, ax_head, by_head, cz_head, tumour_radius, tumour_posX, tumour2_radius, tumour2_posX;
    // Cylindrical phantom (phantomType 1): water cylinder + borated rod (tumour)
    G4double cyl_radius, cyl_halfZ, rod_radius, rod_halfZ, rod_posX, rod_posZ;
    G4int phantomType;
    G4bool enableDebugDetector;

    void ConstructBoronSphere();
    void ConstructBoronCylindricalPhantom();
    void ConstructSnyderHead1();
    void ConstructSnyderHead2();
    void ConstructDebugDetector();

    void DefineMaterials();
    void SetVisAttributes();

};


#endif