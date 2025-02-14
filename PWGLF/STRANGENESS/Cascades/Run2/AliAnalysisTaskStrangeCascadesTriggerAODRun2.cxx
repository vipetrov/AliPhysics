/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* AliAnalysisTaskStrangeCascadesTriggerAODRun2
 *
 * empty task which can serve as a starting point for building an analysis
 * as an example, one histogram is filled
 */

#include "TChain.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TList.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TRegexp.h"
#include "TObjString.h"

#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliMultEstimator.h"
#include "AliMultSelection.h"
#include "AliMultSelectionTask.h"
#include "AliPIDResponse.h"
#include "AliHeader.h"
#include "AliGenEventHeader.h"

#include "AliESDtrack.h"
#include "AliESDEvent.h"
#include "AliESDUtils.h"

#include "AliAODEvent.h"
#include "AliAODInputHandler.h"

#include "AliAnalysisTaskStrangeCascadesTriggerAODRun2.h"

class AliAnalysisTaskStrangeCascadesTriggerAODRun2;    // your analysis class

using namespace std;            // std namespace: so you can do things like 'cout'

ClassImp(AliAnalysisTaskStrangeCascadesTriggerAODRun2) // classimp: necessary for root

AliAnalysisTaskStrangeCascadesTriggerAODRun2::AliAnalysisTaskStrangeCascadesTriggerAODRun2() : 
AliAnalysisTaskSE(), fOutputList(0), fOutputTree(0),
fPIDResponse(0), fUtils(0), 

//---> Output objects
fArrayCascade       (new TClonesArray("AliCascadeContainer")),
fArrayV0            (new TClonesArray("AliV0Container")),
fArrayTrack         (new TClonesArray("AliTrackInfoContainer")),
fAnalysisEvent      (new AliEventContainer          ()),
fAnalysisCascade    (new AliCascadeContainer        ()),
fAnalysisV0         (new AliV0Container             ()),
fAnalysisTrack      (new AliTrackInfoContainer      ()),

//---> Flags controlling TTree outputs
fSaveCascades(kTRUE),
fSaveV0s(kTRUE), 
fSaveTracks(kTRUE), 
fTrigOnCascade(kTRUE),

//---> Variables controlling PV selections
fkMaxPVR2D(10.), 
fkMaxPVZ(10.), 

//---> Variables controlling cascade and V0 default selections
fMinNbrCrossedRows(0), 
fMinPtToSave(0), 
fMaxPtToSave(20), 
fMaxAbsEta(0.8), 
fMaxAbsRap(0.5),
fAODFilterBit( BIT(5) ),

//---> Flags controlling cascade and V0 custom selections
fCascSaveAddConfig(kFALSE), 
fV0SaveAddConfig(kFALSE),
fTrackSaveAddConfig(kFALSE),
fCascadeResult(0), 
fV0Result(0),
fTrackCuts(0),

fTrackCuts2010          (0x0),
fTrackCuts2011          (0x0),
fTrackCutsTPCRefit      (0x0),
fTrackCutsV0            (0x0),
fTrackCuts2011Sys       (0x0),
fTrackCutsHybrid_kNone  (0x0),
fTrackCutsHybrid_kOff   (0x0),

//Histos
fHistEventCounter(0)
{
    // default constructor, don't allocate memory here!
    // this is used by root for IO purposes, it needs to remain empty
}
//_____________________________________________________________________________
AliAnalysisTaskStrangeCascadesTriggerAODRun2::AliAnalysisTaskStrangeCascadesTriggerAODRun2(const char* name, Bool_t lSaveCascades, Bool_t lSaveV0s, Bool_t lSaveTracks) : 
AliAnalysisTaskSE(name), fOutputList(0), fOutputTree(0),
fPIDResponse(0), fUtils(0), 

//---> Output objects
fArrayCascade       (new TClonesArray("AliCascadeContainer")),
fArrayV0            (new TClonesArray("AliV0Container")),
fArrayTrack         (new TClonesArray("AliTrackInfoContainer")),
fAnalysisEvent      (new AliEventContainer          ()),
fAnalysisCascade    (new AliCascadeContainer        ()),
fAnalysisV0         (new AliV0Container             ()),
fAnalysisTrack      (new AliTrackInfoContainer          ()),

//---> Flags controlling TTree outputs
fSaveCascades(kTRUE),
fSaveV0s(kTRUE), 
fSaveTracks(kTRUE), 
fTrigOnCascade(kTRUE),

//---> Variables controlling PV selections
fkMaxPVR2D(10.), 
fkMaxPVZ(10.), 

//---> Variables controlling cascade and V0 default selections
fMinNbrCrossedRows(0), 
fMinPtToSave(0), 
fMaxPtToSave(20), 
fMaxAbsEta(0.8), 
fMaxAbsRap(0.5),
fAODFilterBit( BIT(5) ),

//---> Flags controlling cascade and V0 custom selections
fCascSaveAddConfig(kFALSE), 
fV0SaveAddConfig(kFALSE),
fTrackSaveAddConfig(kFALSE),
fCascadeResult(0), 
fV0Result(0),
fTrackCuts(0), 

fTrackCuts2010          (0x0),
fTrackCuts2011          (0x0),
fTrackCutsTPCRefit      (0x0),
fTrackCutsV0            (0x0),
fTrackCuts2011Sys       (0x0),
fTrackCutsHybrid_kNone  (0x0),
fTrackCutsHybrid_kOff   (0x0),

//Histos
fHistEventCounter(0)
{
    fSaveCascades       = lSaveCascades;
    fSaveV0s            = lSaveV0s;
    fSaveTracks      	= lSaveTracks;
    
    fTrackCuts2010 = new AliESDtrackCuts("AliESDtrackCuts2010","AliESDtrackCuts2010");
    fTrackCuts2010 = AliESDtrackCuts::GetStandardITSTPCTrackCuts2010(kTRUE, kTRUE); //If not running, set to kFALSE;
    fTrackCuts2010->SetEtaRange(-0.8,0.8);
    
    fTrackCuts2011 = new AliESDtrackCuts("AliESDtrackCuts2011","AliESDtrackCuts2011");
    fTrackCuts2011 = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kTRUE, kTRUE); //If not running, set to kFALSE;
    fTrackCuts2011->SetEtaRange(-0.8,0.8);
    
    fTrackCutsTPCRefit = new AliESDtrackCuts("AliESDtrackCutsTPCRefit","AliESDtrackCutsTPCRefit");
    fTrackCutsTPCRefit = AliESDtrackCuts::GetStandardTPCOnlyTrackCuts(); //If not running, set to kFALSE;
    fTrackCutsTPCRefit->SetRequireTPCRefit(kTRUE);
    fTrackCutsTPCRefit->SetEtaRange(-0.8,0.8);
    
    fTrackCutsV0 = new AliESDtrackCuts("AliESDtrackCutsV0", "AliESDtrackCutsV0");
    fTrackCutsV0 = AliESDtrackCuts::GetStandardV0DaughterCuts();
    fTrackCutsV0->SetEtaRange(-0.8,0.8);
    
    //Following is TC for systematics estimation. To compensate, once should probably reduce gamma DeltaM even further :)                                                                                    
    fTrackCuts2011Sys = new AliESDtrackCuts("AliESDtrackCuts2011Sys","AliESDtrackCuts2011Sys");
    fTrackCuts2011Sys = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kTRUE, kTRUE); //If not running, set to kFALSE;                                                                                            
    fTrackCuts2011Sys->SetEtaRange(-0.8,0.8);
    fTrackCuts2011Sys->SetMinNCrossedRowsTPC(60);
    fTrackCuts2011Sys->SetMaxChi2PerClusterTPC(5);
    fTrackCuts2011Sys->SetMaxDCAToVertexZ(3);

    fTrackCutsHybrid_kNone = new AliESDtrackCuts("AliESDtrackCutsHybrid_kNone","AliESDtrackCutsHybrid_kNone");
    fTrackCutsHybrid_kNone->SetMinNCrossedRowsTPC(70);
    fTrackCutsHybrid_kNone->SetMinRatioCrossedRowsOverFindableClustersTPC(0.8);
    fTrackCutsHybrid_kNone->SetMaxChi2PerClusterTPC(4);
    fTrackCutsHybrid_kNone->SetAcceptKinkDaughters(kFALSE);
    fTrackCutsHybrid_kNone->SetRequireTPCRefit(kTRUE);
    fTrackCutsHybrid_kNone->SetRequireITSRefit(kTRUE);
    fTrackCutsHybrid_kNone->SetMaxDCAToVertexXYPtDep("0.0105+0.0350/pt^1.1");
    fTrackCutsHybrid_kNone->SetMaxChi2TPCConstrainedGlobal(36);
    fTrackCutsHybrid_kNone->SetMaxDCAToVertexZ(2);
    fTrackCutsHybrid_kNone->SetDCAToVertex2D(kFALSE);
    fTrackCutsHybrid_kNone->SetRequireSigmaToVertex(kFALSE);
    fTrackCutsHybrid_kNone->SetMaxChi2PerClusterITS(36);
    fTrackCutsHybrid_kNone->SetEtaRange(-0.8,0.8);
    fTrackCutsHybrid_kNone->SetClusterRequirementITS(AliESDtrackCuts::kSPD, AliESDtrackCuts::kNone);
    
    fTrackCutsHybrid_kOff = new AliESDtrackCuts("AliESDtrackCutsHybrid_kOff","AliESDtrackCutsHybrid_kOff");
    fTrackCutsHybrid_kOff->SetMinNCrossedRowsTPC(70);
    fTrackCutsHybrid_kOff->SetMinRatioCrossedRowsOverFindableClustersTPC(0.8);
    fTrackCutsHybrid_kOff->SetMaxChi2PerClusterTPC(4);
    fTrackCutsHybrid_kOff->SetAcceptKinkDaughters(kFALSE);
    fTrackCutsHybrid_kOff->SetRequireTPCRefit(kTRUE);
    fTrackCutsHybrid_kOff->SetRequireITSRefit(kFALSE);
    fTrackCutsHybrid_kOff->SetMaxDCAToVertexXYPtDep("0.0105+0.0350/pt^1.1");
    fTrackCutsHybrid_kOff->SetMaxChi2TPCConstrainedGlobal(36);
    fTrackCutsHybrid_kOff->SetMaxDCAToVertexZ(2);
    fTrackCutsHybrid_kOff->SetDCAToVertex2D(kFALSE);
    fTrackCutsHybrid_kOff->SetRequireSigmaToVertex(kFALSE);
    fTrackCutsHybrid_kOff->SetMaxChi2PerClusterITS(36);	
    fTrackCutsHybrid_kOff->SetEtaRange(-0.8,0.8);
    fTrackCutsHybrid_kOff->SetClusterRequirementITS(AliESDtrackCuts::kSPD, AliESDtrackCuts::kOff);
    
    // constructor
    DefineInput(0, TChain::Class());    
    
    DefineOutput(1, TList::Class());   
    DefineOutput(2, TTree::Class());//Output Tree
}
//_____________________________________________________________________________
AliAnalysisTaskStrangeCascadesTriggerAODRun2::~AliAnalysisTaskStrangeCascadesTriggerAODRun2()
{
    // destructor
    if(fOutputList) {
        delete fOutputList;     
    }
    if(fOutputTree){
        delete fOutputTree;
    } 
    if( fAnalysisEvent )
    {
        delete fAnalysisEvent;
    }
    if( fAnalysisTrack )
    {
        delete fAnalysisTrack;
    }
    if( fAnalysisCascade )
    {
        delete fAnalysisCascade;
    }
    if( fAnalysisV0 )
    {
        delete fAnalysisV0;
    }
    if( fArrayCascade )
    {
        delete fArrayCascade;
    }
    if( fArrayV0 )
    {
        delete fArrayV0;
    }
    if( fArrayTrack )
    {
        delete fArrayTrack;
    }
    if(fUtils) {
        delete fUtils;    
    }
    if(fPIDResponse) {
        delete fPIDResponse;
    }
    if(fTrackCuts2010) {
        delete fTrackCuts2010;
    }
    if(fTrackCuts2011) {
        delete fTrackCuts2011;
    }
    if(fTrackCutsTPCRefit) {
        delete fTrackCutsTPCRefit;
    }
    if(fTrackCutsV0) {
        delete fTrackCutsV0;
    }
    if(fTrackCuts2011Sys) {
        delete fTrackCuts2011Sys;
    }
    if(fTrackCutsHybrid_kNone) {
        delete fTrackCutsHybrid_kNone;
    }
    if(fTrackCutsHybrid_kOff) {
        delete fTrackCutsHybrid_kOff;
    }
}
//_____________________________________________________________________________
void AliAnalysisTaskStrangeCascadesTriggerAODRun2::UserCreateOutputObjects()
{
    fOutputTree = new TTree("fOutputTree", "fOutputTree");
    fOutputTree->Branch("EventInfo"     , "AliEventContainer"   , &fAnalysisEvent   );
    fOutputTree->Branch("CascadeInfo"   , "TClonesArray"        , &fArrayCascade    );
    fOutputTree->Branch("V0Info"        , "TClonesArray"        , &fArrayV0         );
    fOutputTree->Branch("TrackInfo"     , "TClonesArray"        , &fArrayTrack      );
    
    //------------------------------------------------
    // Particle Identification Setup
    //------------------------------------------------
    
    AliAnalysisManager *man=AliAnalysisManager::GetAnalysisManager();
    AliInputEventHandler* inputHandler = (AliInputEventHandler*) (man->GetInputEventHandler());
    fPIDResponse = inputHandler->GetPIDResponse();
    
    //Analysis Utils
    if(! fUtils ) fUtils = new AliAnalysisUtils();
    
    fOutputList = new TList();   
    fOutputList->SetOwner(kTRUE);
    
    if(! fHistEventCounter ) {
        //Histogram Output: Event-by-Event
        fHistEventCounter = new TH1D( "fHistEventCounter", ";Evt. Sel. Step;Count",5,0,5);
        fHistEventCounter->GetXaxis()->SetBinLabel(1, "Selected");
        fHistEventCounter->GetXaxis()->SetBinLabel(2, "Evt");
		fHistEventCounter->GetXaxis()->SetBinLabel(3, "PrimVtxCuts");
        fOutputList->Add(fHistEventCounter);
    }
    
    PostData(1, fOutputList); 
    PostData(2, fOutputTree);
}
//_____________________________________________________________________________
void AliAnalysisTaskStrangeCascadesTriggerAODRun2::UserExec(Option_t *)
{
	// Is selected
    fHistEventCounter->Fill(0.5);
	
    // Main loop
    // Called for each event
    AliAODEvent *lAODevent = 0x0;
    AliMCEvent  *lMCevent  = 0x0;
	
    // Connect to the InputEvent
    lAODevent = dynamic_cast<AliAODEvent*>( InputEvent() );
    if( !lAODevent ) 
    {
        Printf("ERROR: AOD event not available \n");
        return;
    }
    
    // Get event
    fHistEventCounter->Fill(1.5);
    
    //------------------------------------------------
    // Primary Vertex Requirements Section:
    //  ---> pp: has vertex, |z|<10cm
    //------------------------------------------------
    
    const AliAODVertex *lPrimaryBestAODVtx     = lAODevent->GetPrimaryVertex();
    const AliAODVertex *lPrimaryTrackingAODVtx = lAODevent->GetPrimaryVertexTracks();
    const AliAODVertex *lPrimarySPDVtx         = lAODevent->GetPrimaryVertexSPD();
    
    Double_t lBestPrimaryVtxPos[3]          = {-100.0, -100.0, -100.0};
    lPrimaryBestAODVtx->GetXYZ( lBestPrimaryVtxPos );
    
    //Optional cut on the primary vertex
    if ( TMath::Sqrt( TMath::Power(lBestPrimaryVtxPos[0],2)+TMath::Power(lBestPrimaryVtxPos[1],2)) > fkMaxPVR2D ) return;
    if ( TMath::Abs( lBestPrimaryVtxPos[2] ) > fkMaxPVZ ) return;
	
	// Pass Primary vertex cuts
    fHistEventCounter->Fill(2.5);
    
    // multiplicity percentiles
    AliMultSelection *MultSelection = (AliMultSelection*) lAODevent -> FindListObject("MultSelection");
    //------------------------------------------------
    // EVENT INFO EXTRACTION
    //------------------------------------------------
    
    fAnalysisEvent->Reset();
    fAnalysisEvent->Fill(MultSelection, fUtils, lAODevent);
    
    fAnalysisEvent->SetIsCollisionCandidate( fInputHandler->IsEventSelected() & AliVEvent::kAny );
    fAnalysisEvent->SetIsEventSelected     ( fInputHandler->IsEventSelected() ) ;
    fAnalysisEvent->SetTriggerMask         ( fInputHandler->IsEventSelected() ) ;
    
    ULong64_t lEvtNumber = ( ( ((ULong64_t)lAODevent->GetPeriodNumber() ) << 36 ) |
							 ( ((ULong64_t)lAODevent->GetOrbitNumber () ) << 12 ) |
							   ((ULong64_t)lAODevent->GetBunchCrossNumber() )  );
    
    Double_t Cov[6]; 
    lPrimaryBestAODVtx->GetCovarianceMatrix( Cov );
    
    fAnalysisEvent->SetEventNumber          ( lEvtNumber                      );
    fAnalysisEvent->SetPrimaryVertexPos     ( lBestPrimaryVtxPos              );
    fAnalysisEvent->SetPrimaryVertexCov     ( Cov                             );
    fAnalysisEvent->SetVertexStatus         ( lPrimaryBestAODVtx->GetStatus() );
    
    //------------------------------------------------
    // MAIN CASCADE LOOP STARTS HERE
    //------------------------------------------------
    /* reset track array */
	//fArrayCascade->Clear("") ;
    //fArrayCascade->Clear("C") ;
    
    Long_t ncascades = 0;
    ncascades = lAODevent->GetNumberOfCascades();
    Int_t NCascadesSaved = 0;
	
	if( fSaveCascades )
	{
		for (Int_t iCasc = 0; iCasc < ncascades; iCasc++)
		{
			if( !lPrimaryBestAODVtx             ) continue;
			
			AliAODcascade *cascade = lAODevent->GetCascade(iCasc);
			if (!cascade) continue;
			
			//Reconstructing & Check bachelor & V0 daughter tracks
			AliAODTrack *bTrack = dynamic_cast<AliAODTrack*>( cascade->GetDecayVertexXi()->GetDaughter(0) );
			AliAODTrack *pTrack = dynamic_cast<AliAODTrack*>( cascade->GetDaughter(0) );
			AliAODTrack *nTrack = dynamic_cast<AliAODTrack*>( cascade->GetDaughter(1) );
			
			if (!bTrack || !pTrack || !nTrack)
				continue;
			UInt_t lIDpTrack  = (UInt_t) TMath::Abs(pTrack->GetID());
			UInt_t lIDnTrack  = (UInt_t) TMath::Abs(nTrack->GetID());
			UInt_t lIDbTrack   = (UInt_t) TMath::Abs(bTrack->GetID());
			if(lIDbTrack == lIDnTrack)
				continue;
			if(lIDbTrack == lIDpTrack)
				continue;
			
			//Cuts
			if (!(pTrack->IsOn(AliAODTrack::kTPCrefit)))
				continue;
			if (!(nTrack->IsOn(AliAODTrack::kTPCrefit)))
				continue;
			if (!(bTrack->IsOn(AliAODTrack::kTPCrefit)))
				continue;
			
			if ( pTrack->Pt() < 0.15 || nTrack->Pt() < 0.15 || bTrack->Pt() < 0.15 )
				continue;
			
			Double_t V0Px = cascade->MomV0X();
			Double_t V0Py = cascade->MomV0Y();
			Double_t V0Pz = cascade->MomV0Z();
			Double_t V0P  = TMath::Sqrt( cascade->Ptot2V0() );
			Double_t V0Pt = TMath::Sqrt( cascade->Pt2V0() );
			Double_t CascPx = cascade->MomXiX();
			Double_t CascPy = cascade->MomXiY();
			Double_t CascPz = cascade->MomXiZ();
			Double_t CascP  = TMath::Sqrt( cascade->Ptot2Xi() );
			Double_t CascPt = TMath::Sqrt( cascade->Pt2Xi() );
			Double_t V0P_Pz = V0P - V0Pz + 1.e-13;
			Double_t CascP_Pz = CascP - CascPz + 1.e-13;
			Double_t V0P_plus_Pz =  V0P + V0Pz;
			Double_t CascP_plus_Pz = CascP + CascPz;
			if(V0Pz>V0P)
				continue;
			if(CascPz>CascP)
				continue;
			if(V0P_plus_Pz/V0P_Pz <= 0)
				continue;
			if(CascP_plus_Pz/CascP_Pz <= 0)
				continue;
			
			Double_t V0Eta      = 0.5*TMath::Log(TMath::Abs((V0P_plus_Pz)/(V0P_Pz)));
			Double_t CascEta    = 0.5*TMath::Log(TMath::Abs((CascP_plus_Pz)/(CascP_Pz)));
			
			Double_t MassAsXi       = cascade->MassXi();
			Double_t MassAsOmega    = cascade->MassOmega();
			
			if( CascPt < fMinPtToSave ) 
				continue;
			if( CascPt > fMaxPtToSave ) 
				continue;
			
			fAnalysisCascade->Reset();
			fAnalysisCascade->Fill(cascade, fPIDResponse, lBestPrimaryVtxPos, fAnalysisEvent->GetMagneticField());
		
			Bool_t lCascadePassCuts = kFALSE;
			if( !fCascSaveAddConfig  )
			{
				if(
					TMath::Abs( CascEta      )                      < fMaxAbsEta                          &&
					TMath::Abs( V0Eta        )                      < fMaxAbsEta                          &&
					
					TMath::Abs( fAnalysisCascade->GetBachEta() )    < fMaxAbsEta                          && 
					TMath::Abs( fAnalysisCascade->GetPosEta() )     < fMaxAbsEta                          && 
					TMath::Abs( fAnalysisCascade->GetNegEta() )     < fMaxAbsEta                          &&
					
					(TMath::Abs( fAnalysisCascade->GetRapXi()    )  < fMaxAbsRap                          || 
					 TMath::Abs( fAnalysisCascade->GetRapOmega() )  < fMaxAbsRap                         )&&
					
					fAnalysisCascade->GetLeastNbrCrossedRows()      > fMinNbrCrossedRows                  &&
					
					//Do Selection : reject if the both mass selections are simultenously false
					(   //START XI SELECTIONS
						(1.32-0.075 < fAnalysisCascade->GetMassAsXi()      && fAnalysisCascade->GetMassAsXi()     < 1.32+0.075)    
						||
						//START OMEGA SELECTIONS
						(1.68-0.075 < fAnalysisCascade->GetMassAsOmega()   && fAnalysisCascade->GetMassAsOmega()  < 1.68+0.075)
					)
				)
				{
					lCascadePassCuts = kTRUE;
				}
			
			}
			
			if( fCascSaveAddConfig )
			{
				Int_t nCascRes = fCascadeResult.GetEntries();
				
				for (Int_t iCascRes = 0 ; iCascRes < nCascRes ; iCascRes++) 
				{
					// once the candidate has passed the selection of 
					// one of the cascade type (Xi+, Xi-, Omega+, Omega-), 
					// we can skip the other cut sets
					if( lCascadePassCuts ) break;
					
					AliCascadeResult *lCascadeResult = (AliCascadeResult *)fCascadeResult[iCascRes];
					
					if( fAnalysisCascade->IsSelected( lCascadeResult ) )
					{
						lCascadePassCuts = kTRUE;
					}
					
				}
				
			} 
			
			// If the cascade does pass the cuts, add it to the array
			if( lCascadePassCuts )
			{
				AliCascadeContainer* Cascade = new ((*fArrayCascade)[NCascadesSaved]) AliCascadeContainer();
				NCascadesSaved++;
				
				Cascade->Fill(cascade, fPIDResponse, lBestPrimaryVtxPos, fAnalysisEvent->GetMagneticField());
			}
		}
		
	}
    
    //------------------------------------------------
    // MAIN V0 LOOP STARTS HERE
    //------------------------------------------------
    Int_t NV0sSaved = 0;
    if( (!fTrigOnCascade || NCascadesSaved > 0 ) && fSaveV0s )
    {
        /* reset track array */
        //fArrayV0->Clear("C") ;
        
        Long_t nV0s = 0;
        nV0s = lAODevent->GetNumberOfV0s();
        for (Int_t iV0 = 0; iV0 < nV0s; iV0++)
        {
            AliAODv0 *v0 = lAODevent->GetV0(iV0);
            
            if( !v0 ) 
                continue;
            
            Double_t lPosV0[3] = {0.,0.,0.}; // Position of V0 
            v0->GetXYZ(lPosV0);
            
            //Gather tracks informations
            AliAODTrack *pTrack = dynamic_cast<AliAODTrack*>( v0->GetDaughter(0) ); //0->Positive Daughter
            AliAODTrack *nTrack = dynamic_cast<AliAODTrack*>( v0->GetDaughter(1) ); //1->Negative Daughter
            
            if(!pTrack || !nTrack) 
            {
                AliWarning("ERROR: Could not retrieve one of the V0 daughter tracks");
                continue;
            }
            
            if( !(pTrack->IsOn(AliAODTrack::kTPCrefit)) )
                continue;
            if( !(nTrack->IsOn(AliAODTrack::kTPCrefit)) )
                continue;
            
            if( pTrack->Pt() < 0.15 || nTrack->Pt() < 0.15 )
                continue;
            
            // Filter like-sign V0 
            if ( pTrack->GetSign() == nTrack->GetSign() ) 
                continue;
            
            //------------------------------------------------
            // Fill !
            //------------------------------------------------
            //Apply rough selections
            //pT window
            Double_t lV0Pt =  TMath::Sqrt( v0->Pt2V0() );
            
            if( lV0Pt < fMinPtToSave ) 
                continue;
            if( lV0Pt > fMaxPtToSave ) 
                continue;
            
            //Second Selection: rough 20-sigma band, parametric.
            //K0Short: Enough to parametrize peak broadening with linear function.
            Double_t lUpperLimitK0Short = (5.63707e-01) + (1.14979e-02)*lV0Pt;
            Double_t lLowerLimitK0Short = (4.30006e-01) - (1.10029e-02)*lV0Pt;
            //Lambda: Linear (for higher pt) plus exponential (for low-pt broadening)
            //[0]+[1]*x+[2]*TMath::Exp(-[3]*x)
            Double_t lUpperLimitLambda = (1.13688e+00) + (5.27838e-03)*lV0Pt + (8.42220e-02)*TMath::Exp(-(3.80595e+00)*lV0Pt);
            Double_t lLowerLimitLambda = (1.09501e+00) - (5.23272e-03)*lV0Pt - (7.52690e-02)*TMath::Exp(-(3.46339e+00)*lV0Pt);
            
            fAnalysisV0->Reset();
            fAnalysisV0->Fill(v0, fPIDResponse, lBestPrimaryVtxPos, fAnalysisEvent->GetMagneticField());
            
            Bool_t lV0PassCuts = kFALSE;
            if( !fV0SaveAddConfig )
            {
                
                if( //Reject on-the-fly V0s
                    !v0->GetOnFlyStatus()                                                   &&
                    
                    TMath::Abs(v0->PseudoRapV0())                   < fMaxAbsEta                    &&
                    
                    TMath::Abs(pTrack->Eta())                       < fMaxAbsEta                    && 
                    TMath::Abs(nTrack->Eta())                       < fMaxAbsEta                    &&
                    
                    (   
                        TMath::Abs( fAnalysisV0->GetRapK0Short() )  < fMaxAbsRap  || 
                        TMath::Abs( fAnalysisV0->GetRapLambda()  )  < fMaxAbsRap    
                    )                                                                       &&
                    
                    pTrack->GetTPCNcls()                            > fMinNbrCrossedRows            &&
                    nTrack->GetTPCNcls()                            > fMinNbrCrossedRows            &&
                    
                    //Do Selection : reject if the three mass selections are false
                    (   //Case 1: Lambda Selection
                        (lLowerLimitLambda   < fAnalysisV0->GetMassAsLambda()      && fAnalysisV0->GetMassAsLambda()       < lUpperLimitLambda ) ||
                        //Case 2: AntiLambda Selection
                        (lLowerLimitLambda   < fAnalysisV0->GetMassAsAntiLambda()  && fAnalysisV0->GetMassAsAntiLambda()   < lUpperLimitLambda ) ||
                        //Case 3: K0Short Selection
                        (lLowerLimitK0Short  < fAnalysisV0->GetMassAsK0s()         && fAnalysisV0->GetMassAsK0s()          < lUpperLimitK0Short)
                    )
                )
                {
                    lV0PassCuts = kTRUE;
                }
                
            }
            
            
            if( fV0SaveAddConfig )
            {
                Int_t nV0Res = fV0Result.GetEntries();
                
                for (Int_t iV0Res = 0 ; iV0Res < nV0Res ; iV0Res++) 
                {
                    // once the candidate has passed the selection of 
                    // one of the V0 type (K0s, Lambda, AntiLambda), 
                    // we can skip the other cut sets
                    if( lV0PassCuts ) break;
                    
                    AliV0Result *lV0Result = (AliV0Result *)fV0Result[iV0Res];
                    
                    if( fAnalysisV0->IsSelected( lV0Result ) )
                    {
                        lV0PassCuts = kTRUE;
                    }
                    
                }
                
            }
            
            // If the v0 does pass the cuts, add it.
            if( lV0PassCuts )
            {
                AliV0Container* V0 = new ((*fArrayV0)[NV0sSaved]) AliV0Container();
                NV0sSaved++;
                
                V0->Fill(v0, fPIDResponse, lBestPrimaryVtxPos, fAnalysisEvent->GetMagneticField());
            }
            
        }
        
    }
    
    //------------------------------------------------
    // MAIN TRACK LOOP STARTS HERE
    //------------------------------------------------
    Int_t NTracksSaved = 0;
    if( (!fTrigOnCascade || NCascadesSaved > 0) && fSaveTracks)
    {
        /* reset track array */
        //fArrayTrack->Clear() ;
        
        Int_t ntracksLoop = lAODevent->GetNumberOfTracks();
        for(Int_t iTr = 0 ; iTr < ntracksLoop ; iTr++)
        {
            AliAODTrack* primTrack = dynamic_cast<AliAODTrack*>(lAODevent->GetTrack(iTr));
            if (!primTrack) continue;
            
            if( !(primTrack->IsOn(AliAODTrack::kTPCrefit)) )
                continue;
            
            if( primTrack->Pt() < 0.15 )
                continue;
            
            if( !primTrack->TestFilterBit( fAODFilterBit ) ) 
                continue;
            
            //------------------------------------------------
            //Fill !
            //------------------------------------------------
            fAnalysisTrack->Reset();
            fAnalysisTrack->Fill(primTrack, fPIDResponse, lBestPrimaryVtxPos, fAnalysisEvent->GetMagneticField());
            
            Bool_t lTrackPassCuts = kFALSE;
            
            if( !fTrackSaveAddConfig )
            {
                if(
                    fAnalysisTrack->GetCharge()                          != 0                &&// Charge != 0
                    fAnalysisTrack->GetDCAxyToPV()                       < 0.5               &&// DCAxy < 0.5
                    fAnalysisTrack->GetDCAzToPV()                        < 2                 &&// DCAz < 2
                    fAnalysisTrack->GetTPCChi2PerCluster()               < 4.                &&// TPC Chi2/Cluster < 4
                    TMath::Abs(fAnalysisTrack->GetEta())                 < fMaxAbsEta        &&// |eta| < 0.8
                
                    fAnalysisTrack->GetNbrCrossedRows()                  > 70    && // Nbr of Crossed Rows > 70
                    fAnalysisTrack->GetTrackLength()                     > 80    && // Track length > 80cm
                    fAnalysisTrack->GetRatioCrossedRowsOverFindable()    > 0.8   && // Ratio Crossed Rows/Findable > 0.8
                
                    ( 
						TMath::Abs(fAnalysisTrack->GetNSigmaPion())          < 5.    ||
						TMath::Abs(fAnalysisTrack->GetNSigmaKaon())          < 5.    ||
						TMath::Abs(fAnalysisTrack->GetNSigmaProton())        < 5.
					)
                )
                {
                    lTrackPassCuts = kTRUE;
                }
            }
            
            if( fTrackSaveAddConfig )
            {
                if( fAnalysisTrack->IsSelected( fTrackCuts ) )
                {
                    lTrackPassCuts = kTRUE;
                }
            }
            //lTrackPassCuts = kTRUE;
            //If the track does pass the cuts, add it.
            if( lTrackPassCuts )
            {
                AliTrackInfoContainer* Track = new ((*fArrayTrack)[NTracksSaved]) AliTrackInfoContainer();
                NTracksSaved++;
                
                Track->Fill(primTrack, fPIDResponse, lBestPrimaryVtxPos, fAnalysisEvent->GetMagneticField());
                
                Track->SetPassesTrackCuts2010           ( fTrackCuts2010          ->AcceptVTrack(primTrack) );
                Track->SetPassesTrackCuts2011           ( fTrackCuts2011          ->AcceptVTrack(primTrack) );
                Track->SetPassesTrackCutsTPCRefit       ( fTrackCutsTPCRefit      ->AcceptVTrack(primTrack) );
                Track->SetPassesTrackCuts2011Sys        ( fTrackCuts2011Sys       ->AcceptVTrack(primTrack) );
                Track->SetPassesTrackCutsV0             ( fTrackCutsV0            ->AcceptVTrack(primTrack) );
                Track->SetPassesTrackCutsHybrid_kOff    ( fTrackCutsHybrid_kOff   ->AcceptVTrack(primTrack) );
                Track->SetPassesTrackCutsHybrid_kNone   ( fTrackCutsHybrid_kNone  ->AcceptVTrack(primTrack) );
            }
            
        }
        
    }
    
    if( NV0sSaved > 0 || NTracksSaved > 0 || NCascadesSaved > 0 )
    {
        fOutputTree->Fill();
    }
    
    // Post the data
    PostData(1, fOutputList); 
    PostData(2, fOutputTree);
	
    fArrayCascade   ->Clear("C");
    fArrayV0        ->Clear("C");
    fArrayTrack     ->Clear("C");
}

//_____________________________________________________________________________
void AliAnalysisTaskStrangeCascadesTriggerAODRun2::Terminate(Option_t *)
{
    // terminate
    // called at the END of the analysis (when all events are processed)
    
    TList *cRetrievedList = 0x0;
    cRetrievedList = (TList*)GetOutputData(1);
    if(!cRetrievedList) {
        AliError("ERROR - AliAnalysisTaskStrangeCascadesTriggerAODRun2 : ouput data container list not available\n");
        return;
    }
    
    fHistEventCounter = dynamic_cast<TH1D*> (  cRetrievedList->FindObject("fHistEventCounter")  );
    if (!fHistEventCounter) {
        AliError("ERROR - AliAnalysisTaskStrangeCascadesTriggerAODRun2 : fHistEventCounter not available");
        return;
    }
    
    TCanvas *canCheck = new TCanvas("AliAnalysisTaskStrangeCascadesTriggerAODRun2","V0 Multiplicity",10,10,510,510);
    canCheck->cd(1)->SetLogy();
    
    fHistEventCounter->SetMarkerStyle(22);
    fHistEventCounter->DrawCopy("E");
}
//_____________________________________________________________________________
