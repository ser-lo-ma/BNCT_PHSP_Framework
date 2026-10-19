// Author: S. Lopez Martinez, University College London
// BNCT phantom geometries (sphere, cylinder with borated rod, Snyder heads)
// for the BNCT phase-space framework. Replaces the DetectorConstruction of
// the Geant4 advanced example IAEAphsp (M.A. Cortes-Giraldo).

#include "DetectorConstruction.hh"
#include "G4UserLimits.hh"
#include "G4Sphere.hh"
#include "G4Ellipsoid.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4VisAttributes.hh"
#include <cmath>
#include "G4MaterialPropertiesTable.hh"

DetectorConstruction::DetectorConstruction(){
    
    fMessenger = new G4GenericMessenger(this, "/geometry/", "Detector Construction");
    fMessenger->DeclareProperty("xWorld", xWorld, "Size of the world in the x direction.");
    fMessenger->DeclareProperty("yWorld", yWorld, "Size of the world in the y direction.");
    fMessenger->DeclareProperty("zWorld", zWorld, "Size of the world in the z direction.");
    fMessenger->DeclareProperty("phantomType", phantomType, "Type of phantom: 0 sphere, 1 cylinder, 2/3 Snyder head, 4 world only.");
    fMessenger->DeclareProperty("enableDebugDetector", enableDebugDetector, "Enable/disable debugging detector shell.");
    fMessenger->DeclarePropertyWithUnit("rodHalfLength", "cm", rod_halfZ, "Cylindrical phantom: half-length of the borated rod (tumour).");
    fMessenger->DeclarePropertyWithUnit("rodPosZ", "cm", rod_posZ, "Cylindrical phantom: z of the rod centre (beam enters at +z).");

    // Set the default values
    
    xWorld = 0.5*m;
    yWorld = 0.5*m;
    zWorld = 0.5*m;
    ax_brain = 6. *cm;
    by_brain = 9. *cm;
    cz_brain = 6.5 *cm;
    ax_skull = 6.8 *cm;
    by_skull = 9.8 *cm;
    cz_skull = 8.3 *cm;
    ax_head = 7.0 * cm;
    by_head = 10.0 * cm;
    cz_head = 8.50 * cm;
    tumour_radius = 1 * cm;
    tumour_posX = 0 * cm;
    tumour2_radius = 1 * cm;
    tumour2_posX = 2 * cm;
    // Cylindrical phantom. 2026-10-05: the rod (tumour) is 3 cm tall and centred
    // 4 cm above the mid-plane, i.e. spans z = 2.5..5.5 cm, 4.5 cm below the top
    // cap where the beam enters. Before this date it ran the full height
    // (half-length 10 cm, flush with both caps), which left a scoring gap.
    cyl_radius = 9.0 * cm;
    cyl_halfZ = 10.0 * cm;
    rod_radius = 1.5 * cm;
    rod_halfZ = 1.5 * cm;
    rod_posX = 4.0 * cm;
    rod_posZ = 4.0 * cm;
    enableDebugDetector = false;

    DefineMaterials();
    phantomType = 0; 
    logicHead = nullptr;
    logicSkull = nullptr;
    logicBrain = nullptr;
    logicTarget = nullptr;
    logicPhantom = nullptr; 
    logicDebuggingDetector = nullptr;
}

DetectorConstruction::~DetectorConstruction()
{}

void DetectorConstruction::DefineMaterials(){
    G4NistManager *nist = G4NistManager::Instance();
    worldMat = nist->FindOrBuildMaterial("G4_AIR");
    Water = nist->FindOrBuildMaterial("G4_WATER");

    // Boron phantom materials - Snyder Head Phantom
    nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
    nist->FindOrBuildMaterial("G4_SKIN_ICRP");
    brainMaterial = nist->FindOrBuildMaterial("G4_BRAIN_ICRP");
    auto* B10 = new G4Isotope("Boron10", 5, 10, 10. * g / mole);
    auto* elB10 = new G4Element("Boron10", "B10", 1);
    elB10->AddIsotope(B10, 1.0);
    boronMaterial = new G4Material("boronMaterial", 2.37 * g / cm3, 1);
    boronMaterial->AddElement(elB10, 1.0);

    double ppm  = 1e-6;
    double tumorBoronfrac = 62.5 * ppm;
    double brainBoronfrac = 25 * ppm;

    boronTumour = new G4Material("boronTumour", 1.0*g/cm3, 2);
    boronTumour->AddMaterial(boronMaterial, tumorBoronfrac);
    boronTumour->AddMaterial(brainMaterial, 1.0 - tumorBoronfrac);

    boronBrain = new G4Material("boronBrain", 1.0*g/cm3, 2);
    boronBrain->AddMaterial(boronMaterial, brainBoronfrac);
    boronBrain->AddMaterial(brainMaterial, 1.0 - brainBoronfrac);

    skullMaterial = nist->FindOrBuildMaterial("G4_BONE_CORTICAL_ICRP");
    skinMaterial  = nist->FindOrBuildMaterial("G4_SKIN_ICRP");

    // Boron phantom materials - Cylindrical Minsky Phantom
    double cylinderBoronfrac = 400 * ppm;
    cylinderMaterial = new G4Material("cylinderMaterial", 1.0*g/cm3, 2);
    cylinderMaterial->AddMaterial(boronMaterial, cylinderBoronfrac);
    cylinderMaterial->AddMaterial(Water, 1-cylinderBoronfrac);

}

void DetectorConstruction::ConstructSnyderHead1(){

    // Skin layer
    auto* solidHead = new G4Ellipsoid("Head", ax_head, by_head, cz_head);
    logicHead = new G4LogicalVolume(solidHead, skinMaterial, "Head");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicHead, "physHead", logicWorld, false, 0, 1);

    // Skull
    auto* craniumOut = new G4Ellipsoid("CraniumOut", ax_skull, by_skull, cz_skull);
    auto* craniumIn = new G4Ellipsoid("CraniumIn", ax_brain, by_brain, cz_brain);

    auto* solidSkull = new G4SubtractionSolid("Skull", craniumOut, craniumIn, nullptr, G4ThreeVector(0.0, 0.0, 1. *cm));
    logicSkull = new G4LogicalVolume(solidSkull, skullMaterial, "Skull");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicSkull, "physSkull", logicHead, false, 0, 1);

    // Brain
    auto* solidBrain = new G4Ellipsoid("Brain", ax_brain, by_brain, cz_brain);
    logicBrain = new G4LogicalVolume(solidBrain, boronBrain, "Brain");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 1 *cm), logicBrain, "physBrain", logicHead,false,0,1);

    // Tumour
    auto *solidTarget = new G4Sphere("Target", 0, tumour_radius, 0, 2*M_PI, 0, M_PI);
    logicTarget = new G4LogicalVolume(solidTarget, boronTumour, "Target");
    new G4PVPlacement(nullptr, G4ThreeVector(tumour_posX, 0 *cm, 0. *cm), logicTarget, "physTarget", logicBrain, false, 0, 1);
}

void DetectorConstruction::ConstructSnyderHead2(){

    // Skin layer
    auto* solidHead = new G4Ellipsoid("Head", ax_head, by_head, cz_head);
    logicHead = new G4LogicalVolume(solidHead, skinMaterial, "Head");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicHead, "physHead", logicWorld, false, 0, 1);

    // Skull
    auto* craniumOut = new G4Ellipsoid("CraniumOut", ax_skull, by_skull, cz_skull);
    auto* craniumIn = new G4Ellipsoid("CraniumIn", ax_brain, by_brain, cz_brain);

    auto* solidSkull = new G4SubtractionSolid("Skull", craniumOut, craniumIn, nullptr, G4ThreeVector(0.0, 0.0, 1. *cm));
    logicSkull = new G4LogicalVolume(solidSkull, skullMaterial, "Skull");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicSkull, "physSkull", logicHead, false, 0, 1);

    // Brain
    auto* solidBrain = new G4Ellipsoid("Brain", ax_brain, by_brain, cz_brain);
    logicBrain = new G4LogicalVolume(solidBrain, boronBrain, "Brain");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 1 *cm), logicBrain, "physBrain", logicHead,false,0,1);

    // Tumour
    auto *solidTarget1 = new G4Sphere("Target1", 0, tumour_radius, 0, 2*M_PI, 0, M_PI);
    logicTarget = new G4LogicalVolume(solidTarget1, boronTumour, "Target1");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0. *cm), logicTarget,"physTarget1",logicBrain,false ,0 ,1);

    auto *solidTarget2 = new G4Sphere("Target2", 0 , tumour_radius , 0 , 2*M_PI , 0 , M_PI);
    logicTarget2 = new G4LogicalVolume(solidTarget2, boronTumour, "Target2");
    new G4PVPlacement(nullptr, G4ThreeVector(0. , 0., 2. *cm), logicTarget2,"physTarget2",logicBrain,false ,0 ,1);
}

void DetectorConstruction::ConstructBoronSphere(){
    auto* solidPhantom = new G4Sphere("Phantom", 0.0 * cm, 10 * cm, 0.0, 2 * M_PI, 0.0, M_PI);
    logicPhantom = new G4LogicalVolume(solidPhantom, boronMaterial, "Phantom");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicPhantom, "physPhantom", logicWorld, false, 0, 1);
}

void DetectorConstruction::ConstructBoronCylindricalPhantom(){ 
    // Water cylinder: 9 cm radius, 20 cm total height (half-length = 10 cm), axis z
    auto* solidPhantom = new G4Tubs("Phantom", 0.0 * cm, cyl_radius, cyl_halfZ, 0.0, 2.0 * M_PI);
    logicPhantom = new G4LogicalVolume(solidPhantom, Water, "Phantom");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicPhantom, "physPhantom", logicWorld, false, 0, 1);

    // The rod must sit strictly inside the cylinder: a rod face flush with a cap
    // lets particles leave the rod straight into the world, bypassing the
    // physPhantom->physWorld scoring boundary.
    if (rod_posX + rod_radius >= cyl_radius || std::abs(rod_posZ) + rod_halfZ >= cyl_halfZ) {
        G4Exception("DetectorConstruction::ConstructBoronCylindricalPhantom", "RodOutside",
                    FatalException, "Borated rod touches or crosses the cylinder surface.");
    }

    // Borated rod (tumour): 1.5 cm radius, 3 cm tall, centred at (4, 0, 4) cm
    auto* solidTarget = new G4Tubs("Target", 0.0 * cm, rod_radius, rod_halfZ, 0.0, 2.0 * M_PI);
    logicTarget = new G4LogicalVolume(solidTarget, cylinderMaterial, "Target");
    new G4PVPlacement(nullptr, G4ThreeVector(rod_posX, 0.0, rod_posZ), logicTarget, "physTarget", logicPhantom, false, 0, 1);
}

void DetectorConstruction::ConstructDebugDetector(){
    auto* vacuum = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
    auto* solidDebuggingDetector = new G4Sphere("DebuggingDetector", 30.0 * cm, 31.0 * cm, 0.0, 2 * M_PI, 0.0, M_PI);
    logicDebuggingDetector = new G4LogicalVolume(solidDebuggingDetector, vacuum, "DebuggingDetector");
    new G4PVPlacement(nullptr, G4ThreeVector(0., 0., 0.), logicDebuggingDetector, "physDebuggingDetector", logicWorld, false, 0, 1);
}

void DetectorConstruction::SetVisAttributes(){
    if (logicHead && logicSkull && logicBrain && logicTarget) {
        G4VisAttributes* headVisAtt = new G4VisAttributes(G4Colour(1, 0, 0, 0.3));
        headVisAtt->SetVisibility(true);
        logicHead->SetVisAttributes(headVisAtt);
        G4VisAttributes* skullVisAtt = new G4VisAttributes(G4Colour(1, 1, 1, 0.3));
        skullVisAtt->SetVisibility(true);
        logicSkull->SetVisAttributes(skullVisAtt);
        G4VisAttributes* brainVisAtt = new G4VisAttributes(G4Colour(1, 0.75, 0.8, 0.3));
        brainVisAtt->SetVisibility(true);
        logicBrain->SetVisAttributes(brainVisAtt);
        G4VisAttributes* targetVisAtt = new G4VisAttributes(G4Colour(1, 1, 0, 0.3));
        targetVisAtt->SetVisibility(true);
        logicTarget->SetVisAttributes(targetVisAtt);
    }
    if (logicPhantom && logicTarget){
        G4VisAttributes* phantomVisAtt = new G4VisAttributes(G4Colour(0, 0, 1, 0.3));
        phantomVisAtt->SetVisibility(true);
        logicPhantom->SetVisAttributes(phantomVisAtt);
        G4VisAttributes* sphereVisAtt = new G4VisAttributes(G4Colour(1, 0, 0, 0.3));
        sphereVisAtt->SetVisibility(true);
        logicTarget->SetVisAttributes(sphereVisAtt);
    }
    if (logicPhantom && !logicSkull){
        G4VisAttributes* phantomVisAtt = new G4VisAttributes(G4Colour(0, 0, 1, 0.3));
        phantomVisAtt->SetVisibility(true);
        logicPhantom->SetVisAttributes(phantomVisAtt);
    }
    if (logicDebuggingDetector) {
        G4VisAttributes* debugVisAtt = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 0.15));
        debugVisAtt->SetVisibility(true);
        logicDebuggingDetector->SetVisAttributes(debugVisAtt);
    }
}

G4VPhysicalVolume *DetectorConstruction::Construct(){	
	
	// Construct the world
	solidWorld = new G4Box("solidWorld", xWorld, yWorld, zWorld);
	logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
	physWorld = new G4PVPlacement(0, G4ThreeVector(0,0,0), logicWorld, "physWorld", 0, false, 0, true);

    // Construct the phantom based on the selected type
    switch (phantomType) {
        case 0:
            ConstructBoronSphere();
            break;
        case 1:
            ConstructBoronCylindricalPhantom();
            break;
        case 2:
            ConstructSnyderHead1();
            break;
        case 3:
            ConstructSnyderHead2();
            break;
        case 4:
            break;
        default:
            G4Exception("Detecto rConstruction::Construct", "InvalidPhantomType",
                        JustWarning, "Invalid phantom type selected. No phantom is constructed.");
    }

    if (enableDebugDetector) {
        ConstructDebugDetector();
    }

    SetVisAttributes();

return physWorld;
}

