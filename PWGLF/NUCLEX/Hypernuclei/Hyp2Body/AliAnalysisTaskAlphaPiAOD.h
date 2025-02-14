/// \class AliAnalysisTaskAlphaPiAOD

#ifndef __AliAnalysisTaskAlphaPiAOD__
#define __AliAnalysisTaskAlphaPiAOD__

#include <Rtypes.h>
#include <TString.h>

#include "AliAnalysisTaskSE.h"
#include "AliEventCuts.h"

class AliPIDResponse;
class TH2F;
class TList;
class TTree;

struct StructHyper {
    Double32_t pt;
    Double32_t Rapidity;
    Double32_t m;
    Double32_t ct;
    Double32_t V0CosPA;
    Double32_t V0radius;
    Double32_t Lrec;
    Double32_t fZ;
    Double32_t TPCnSigmaPi;        //[-5,5,8]
    Double32_t TPCnSigmaalpha;     //[-5,5,8]
    Double32_t TPCmomalpha;        //[0.0,10.22,8]
    Double32_t TPCsignalalpha;     //[0.,2046.,12]
    Double32_t alphaProngPvDCAXY;  //[0.0,5.10,10]
    Double32_t PiProngPvDCAXY;     //[0.0,20.46,12]
    Double32_t alphaProngPvDCA;    //[0.0,10.22,10]
    Double32_t PiProngPvDCA;       //[0.0,40.94,12]
    Double32_t ProngsDCA;          //[0.0,2.54,8]
    unsigned char NpidClustersPion;
    unsigned char NpidClustersalpha;
    unsigned char NitsClustersalpha;
    unsigned char centrality;
    unsigned char trigger;
    bool Matter;
};

struct StructHyperMC : public StructHyper {
    float ptMC;
    float etaMC;
    float ctMC;
    float yMC;
    int pdg;
    bool isReconstructed;
    bool isDuplicated = false;
};

class AliAnalysisTaskAlphaPiAOD : public AliAnalysisTaskSE {
   public:
    enum kReducedTrigger {
        kINT7 = BIT(0),
        kCentral = BIT(1),
        kSemiCentral = BIT(2),
        kPositiveB = BIT(3),
        kHighMultV0 = BIT(4)
    };
    AliAnalysisTaskAlphaPiAOD(bool isMC = false, TString taskname = "HyperAOD");
    static AliAnalysisTaskAlphaPiAOD *AddTask(bool isMC, TString tskname, TString suffix);
    virtual ~AliAnalysisTaskAlphaPiAOD();

    virtual void UserCreateOutputObjects();
    virtual void UserExec(Option_t *);
    virtual void Terminate(Option_t *) {}

    AliEventCuts fEventCut;  ///<

    void SetCustomBetheBloch(float resolution, const float bethe[5]);
    double customNsigma(double mom, double sig);
    void SaveOnlyTrueCandidates(bool toggle = true) { fOnlyTrueCandidates = toggle; }
    void UseOnTheFly(bool toggle = true) { fUseOnTheFly = toggle; }
    void SetMassRange(float min, float max) {
        fMassRange[0] = min;
        fMassRange[1] = max;
    }

   private:
    AliAnalysisTaskAlphaPiAOD(const AliAnalysisTaskAlphaPiAOD &source);
    AliAnalysisTaskAlphaPiAOD &operator=(const AliAnalysisTaskAlphaPiAOD &source);

    void PostAllData();

    TList *fList = nullptr;  //!<! List of the output histograms
    TTree *fTree = nullptr;  //!<! Tree for Hyper

    StructHyper *fRecHyper = nullptr;  //!<! Transient fRecHyper
    StructHyperMC fGenHyper;
    AliPIDResponse *fPID = nullptr;  //!<! ALICE PID framework
    bool fMC;
    bool fOnlyTrueCandidates = true;  ///< Save only true Hyperhydrogens
    bool fUseOnTheFly = false;

    float fCustomBethe[5] = {-166.11733, -0.11020473, 0.10851357, 2.7018593, -0.087597824};  /// default values are for LHC18qr
    float fCustomResolution = 0.05871;                                                       /// default values are for LHC18qr
    double fMassRange[2] = {2.9, 3.1};

    float Eta2y(float pt, float m, float eta) const;

    /// \cond CLASSDEF
    ClassDef(AliAnalysisTaskAlphaPiAOD, 1);
    /// \endcond
};

#endif /* defined(__AliAnalysisTaskAlphaPiAOD__) */
