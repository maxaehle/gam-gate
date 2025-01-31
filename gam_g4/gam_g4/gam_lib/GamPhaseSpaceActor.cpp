/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */

#include <iostream>
#include "GamPhaseSpaceActor.h"
#include "GamHelpersDict.h"
#include "GamHitsCollectionManager.h"

G4Mutex GamPhaseSpaceActorMutex = G4MUTEX_INITIALIZER;

GamPhaseSpaceActor::GamPhaseSpaceActor(py::dict &user_info)
    : GamVActor(user_info) {
    fActions.insert("StartSimulationAction");
    fActions.insert("BeginOfRunAction");
    fActions.insert("PreUserTrackingAction");
    fActions.insert("SteppingAction");
    fActions.insert("EndOfRunAction");
    fActions.insert("EndOfSimulationWorkerAction");
    fActions.insert("EndSimulationAction");
    fOutputFilename = DictGetStr(user_info, "output");
    fHitsCollectionName = DictGetStr(user_info, "name");
    fUserHitAttributeNames = DictGetVecStr(user_info, "attributes");
    fHits = nullptr;
}

GamPhaseSpaceActor::~GamPhaseSpaceActor() {
}

// Called when the simulation start
void GamPhaseSpaceActor::StartSimulationAction() {
    fHits = GamHitsCollectionManager::GetInstance()->NewHitsCollection(fHitsCollectionName);
    fHits->SetFilename(fOutputFilename);
    fHits->InitializeHitAttributes(fUserHitAttributeNames);
    fHits->InitializeRootTupleForMaster();
}

// Called every time a Run starts
void GamPhaseSpaceActor::BeginOfRunAction(const G4Run *run) {
    if (run->GetRunID() == 0)
        fHits->InitializeRootTupleForWorker();
}

void GamPhaseSpaceActor::BeginOfEventAction(const G4Event *) {
}

// Called every time a Track starts (even if not in the volume attached to this actor)
void GamPhaseSpaceActor::PreUserTrackingAction(const G4Track *) {
}

// Called every time a batch of step must be processed
void GamPhaseSpaceActor::SteppingAction(G4Step *step, G4TouchableHistory *touchable) {
    // Only store if this is the first time 
    if (!step->IsFirstStepInVolume()) return;
    fHits->ProcessHits(step, touchable);
}

// Called every time a Run ends
void GamPhaseSpaceActor::EndOfRunAction(const G4Run *) {
    fHits->FillToRoot();
}

// Called every time a Run ends
void GamPhaseSpaceActor::EndOfSimulationWorkerAction(const G4Run *) {
    fHits->Write();
}

// Called when the simulation end
void GamPhaseSpaceActor::EndSimulationAction() {
    fHits->Write();
    fHits->Close();
}

