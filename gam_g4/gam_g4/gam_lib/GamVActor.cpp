/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */

#include "G4SDManager.hh"
#include "GamVActor.h"
#include "GamHelpersDict.h"
#include "GamHelpers.h"
#include "GamMultiFunctionalDetector.h"

GamVActor::GamVActor(py::dict &user_info) :
    G4VPrimitiveScorer(DictGetStr(user_info, "name")) {
    fMotherVolumeName = DictGetStr(user_info, "mother");
}

GamVActor::~GamVActor() {
}

void GamVActor::AddActions(std::set<std::string> &actions) {
    fActions.insert(actions.begin(), actions.end());
}

G4bool GamVActor::ProcessHits(G4Step *step,
                              G4TouchableHistory *touchable) {
    /*
     The second argument is a G4TouchableHistory object for the Readout geometry
     described in the next section. The second argument is NULL if Readout geometry
     is not assigned to this sensitive detector. In this method, one or more G4VHit
     objects should be constructed if the current step is meaningful for your detector.
     */
    for (auto f: fFilters) {
        if (!f->Accept(step)) return true;
    }
    SteppingAction(step, touchable);
    return true;
}

void GamVActor::RegisterSD(G4LogicalVolume *lv) {
    // Look is a SD already exist for this LV
    auto currentSD = lv->GetSensitiveDetector();
    GamMultiFunctionalDetector *mfd;
    if (!currentSD) {
        // This is the first time a SD is set to this LV
        auto f = new GamMultiFunctionalDetector("mfd_" + lv->GetName());
        G4SDManager::GetSDMpointer()->AddNewDetector(f);
        lv->SetSensitiveDetector(f);
        mfd = f;
    } else {
        // A SD already exist, we reused it
        mfd = dynamic_cast<GamMultiFunctionalDetector *>(currentSD);
        for (auto i = 0; i < mfd->GetNumberOfPrimitives(); i++) {
            if (mfd->GetPrimitive(i)->GetName() == GetName()) {
                // In that case the actor is already registered, we skip to avoid
                // G4 exception. It happens when the LogVol has several PhysVol (repeater)
                return;
            }
        }
    }
    // Register the actor to the GamMultiFunctionalDetector
    mfd->RegisterPrimitive(this);
}

