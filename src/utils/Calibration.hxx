///
/// \file Calibration.hxx
/// \author G. Christian and D. Connolly
/// \brief Contains classes to help with calibrating DRAGON detectors
///
#ifndef DRAGON_CALIB_HEADER
#define DRAGON_CALIB_HEADER
#include <vector>
#include <Rtypes.h>
#include "Dragon.hxx"

class TH1;
class TH2F;
class TH1D;
class TTree;

namespace midas { class Database; }


namespace dragon {
namespace utils {

///
/// DSSSD calibration class
class DsssdCalibrator {
public:
	/// Simple stuct to hold linear fit params
	struct Param_t { Double_t slope; Double_t offset; Double_t inl; Double_t intercept; };

	Double_t fMaxSlope, fMinChan;
	TH2F*    fHdcal;
	TH1D*    fFrontcal;

	DsssdCalibrator(TTree* t_alpha, TTree* t_pulser, midas::Database* db);

	void                  DrawFrontCal(Option_t* opt = "");
	void                  DrawSummary(Option_t* opt = "") const;
	void                  DrawSummaryCal(Option_t* opt = "");
	std::vector<Double_t> FindPeaks(TH1* hst, Double_t sigma = 2, Double_t threshold = 0.05) const;
	void                  FitAlphaPeaks(Int_t ch, Bool_t grid = kTRUE);
	void                  FitPulserPeaks(Int_t ch, Int_t Npulser);
	void                  GainMatch();
	Param_t               GetOldParams(Int_t channel) const;
	Param_t               GetParams(Int_t channel) const;
	Double_t              GetPeak(Int_t channel, Int_t peak, Bool_t alpha = kTRUE) const;
	void                  PrintResults(const char* outfile = 0);
	void                  PrintOdb(const char* outfile = 0);
	Int_t                 RunAlpha(Double_t pklow = 500, Double_t pkhigh = 3840, Double_t sigma = 4, Double_t threshold = 0.15, Bool_t grid = kTRUE);
	Int_t                 RunPulser(Double_t pklow = 256, Double_t pkhigh = 3840, Double_t sigma = 4, Double_t threshold = 0.15);
	void                  WriteJson(const char* outfile = "$DH/../calibration/dsssdcal.json");
	void                  WriteOdb(Bool_t json = kTRUE, Bool_t xml = kTRUE);
	void                  WriteXml(const char* outfile = "$DH/../calibration/dsssdcal.xml");
private:
	TTree*           fAlphaTree;
	TTree*           fPulserTree;
	midas::Database* fDb;
	Double_t         fAlphaPeaks[dragon::Dsssd::MAX_CHANNELS][3];
	Double_t         fPulserPeaks[dragon::Dsssd::MAX_CHANNELS][13];
	Param_t          fParams[dragon::Dsssd::MAX_CHANNELS];
	Param_t          fOldParams[dragon::Dsssd::MAX_CHANNELS];
};

} //utils
} //dragon

#endif
