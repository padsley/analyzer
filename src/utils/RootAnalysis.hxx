///
/// \file RootAnalysis.hxx
/// \author G. Christian
/// \brief Defines classes and utilities to help in analysis of ROOT
///  files generated by `mid2root`.
///
#ifndef DRAGON_ROOT_ANALYSIS_HEADER
#define DRAGON_ROOT_ANALYSIS_HEADER
#include <map>
#include <memory>
#include <fstream>
#ifndef __MAKECINT__
#include <iostream>
#endif
#include <algorithm>

#include <TH1.h>
#include <TCut.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSelector.h>

#include "midas/libMidasInterface/TMidasStructs.h"
#include "Uncertainty.hxx"
#include "Constants.hxx"
#include "AutoPtr.hxx"
#include "Dragon.hxx"
#include "Vme.hxx"

class TGraph;
class TGraphErrors;
class TGraphAsymmErrors;

namespace dragon {

enum MeasurementType_t {
	kGammaSingles = 1,
	kHiSingles    = 3,
	kCoinc        = 5
};

/// Chain together all trees in multiple DRAGON files.
void MakeChains(const char* prefix, const Int_t* runnumbers, Int_t nruns, const char* format = "$DH/rootfiles/run%d.root", Bool_t sonik = kFALSE);

/// Chain together all trees in multiple DRAGON files.
void MakeChains(const Int_t* runnumbers, Int_t nruns, const char* format = "$DH/rootfiles/run%d.root", Bool_t sonik = kFALSE);

/// Chain together trees using a vector instead of array
void MakeChains(const std::vector<Int_t>& runnumbers, const char* format = "$DH/rootfiles/run%d.root", Bool_t sonik = kFALSE);

/// Chain together trees using a vector instead of array
void MakeChains(const char* prefix, const std::vector<Int_t>& runnumbers, const char* format = "$DH/rootfiles/run%d.root", Bool_t sonik = kFALSE);

/// Add another chain of files as a friend to an existing one
void FriendChain(TChain* chain, const char* friend_name, const char* friend_alias,
								 const char* format = "$DH/rootfiles/run%d.root",
								 const char* friend_format = "$DH/rootfiles/run%d_dsssd_recal.root");

/// Open a file just by run number
TFile* OpenRun(int runnum, const char* format = "$DH/rootfiles/run%d.root");

/// Calculate weighted average of measurements
template <class InputIterator>
UDouble_t MeasurementWeightedAverage(InputIterator begin, InputIterator end)
{
	///
	/// Procedure:
	///   -# Calculate `num` = `\Sum_i { x_i * (1./sigma_i^2) }
	///   -# Calculate `den` = `\Sum_i { 1./sigma_i^2 }
	///   -# Final result: nominal value = `num/den`, error = `sqrt(den)`
	///
	/// \attention Assumes symmetric errors.
	///
	Double_t num = 0, den = 0;
	while (begin != end) {
		Double_t val = begin->GetNominal(), weight = 1. / pow(begin->GetRelErrLow(), 2);
		num += val*weight;
		den += weight;
		++begin;
	}
	return UDouble_t (num/den, sqrt(1./den));
}

/// Utility class to convert metrix prefix strings into multiplication factors
class MetricPrefix {
public:
	static Double_t Get(const char* prefix);
};

/// Filters TChains (or TTrees) based on cut conditions.
/*!
 *  Example use:
 *  \code
 *  TString inFiles[] = {
 *    "run123.root";
 *    "run124.root";
 *    "run125.root";
 *  };
 *
 *  TChain ch1("t1"), ch3("t3");
 *  for(Int_t i=0; i< sizeof(inFiles) / sizeof(TString); ++i) {
 *    ch1.Add(inFiles[i]);
 *    ch3.Add(inFiles[i]);
 *  }
 *
 *  dragon::TTreeFilter filter("filtered.root");
 *  filter.SetFilterCondition(&ch1, "bgo.ecal[0] > 0");
 *  filter.SetFilterCondition(&ch3, "dsssd.ecal[0] > 0");
 *  filter.Run();
 *  filter.Close();
 *  \endcode
 *
 *  To speed things up, by default each input tree is filtered in a
 *  different thread (one thread per input). To turn off threading, call
 *  SetThreaded(kFALSE) before calling Run(). This will cause each input
 *  tree to be filtered in series rather than in parallel.
 */
class TTreeFilter {
public:
#ifndef DOXYGEN_SKIP
	struct Out_t {
		TTree* fTree;
		TString fCondition;
	};
	typedef std::map <TTree*, Out_t> Map_t;
	typedef std::pair<TTree*, Out_t> Pair_t;
#endif
private:
	/// No copy
	TTreeFilter(const TTreeFilter&) { }
	/// No assign
	TTreeFilter& operator= (const TTreeFilter&) { return *this; }
public:
	/// Ctor
	TTreeFilter(const char* filename, Option_t* option = "", const char* ftitle = "", Int_t compress = 1);
	/// Ctor
	TTreeFilter(TDirectory* output);
	/// Dtor
	~TTreeFilter();
	/// Check if the filter condition is valid.
	Bool_t CheckCondition(TTree* tree) const;
	/// Close the output directory (if the owner)
	void Close();
	/// Check if we're the file owner
	Bool_t IsFileOwner() const { return fFileOwner; }
	/// Get filter condition
	const char* GetFilterCondition(TTree* tree) const;
	/// Check if running threaded version or not.
	Bool_t GetThreaded() const { return fRunThreaded; }
	/// Get output directory
	TDirectory* GetOutDir() const { return fDirectory; }
	/// Do the filtering
	Int_t Run();
	/// Set a tree to filter and it's filter condition
	void SetFilterCondition(TTree* tree, const char* condition);
	/// Set output directory
	void SetOutDir(TDirectory* directory);
	/// Turn on/off threading for different trees, default is on
	void SetThreaded(Bool_t on) { fRunThreaded = on; }
	/// Check if output directory is valid.
	Bool_t IsZombie() const;
private:
	/// Use separate threads for each tree?
	Bool_t fRunThreaded;
	/// File (directory) owning the output tree
	TDirectory* fDirectory;
	/// Do we take ownership of fFile or not?
	Bool_t fFileOwner;
	/// The input trees and conditions
	Map_t fInputs;
};

/// Class to extract data from rossum output files
/*!
 *  This class facilitates extraction of data saved into `*.rossumData` files by the DRAGON
 * "autopilot" program `rossum`.  For each separate rossum reading, the class generates a ROOT
 * tree containig the current, time and cup information as branches. These trees can be identified either
 * by the run number of the run started directly after the cup readings were taken, or they can be identified
 * by the timestamp corresponding to when the rossum query was started. Note that in the case of rossum readings
 * that do not precede a run start, the timestamp is the only valid means of identification.
 *
 * Here is an example showing some of the class's more useful features:
 * \code
 * dragon::RossumData rossumData("rossumData", "/path/to/rossum/data/file/some_beam.rossumData");
 * // The above line constructs a dragon::RossumData instance to read information from the specified file
 *
 * rossumData.ListTrees(); // get a listing of all trees parsed from the file
 * TTree* tree = rossumData.GetTree(1000); // get a pointer to the tree preceding run 1000
 *
 * tree->Draw("current:time", "", "L"); // draw all cup readings vs. time, for the query preceding run 1000
 * tree->Draw("current:time", "cup == 0", ""); // only draw FC4 readings [see AverageCurrent() for more about the cup id]
 * tree->Draw("current:time", "cup == 0 && iteration == 0", ""); // only draw the first iteration of FC4 readings
 *
 * rossumData.AverageCurrent(1000, 0, 0).Print(); // print the average current in the first FC4 iteration
 * rossumData.AverageCurrent(1000, 1, 0).Print(); // print the average current in the first FC1 iteration
 *
 * \endcode
 */
class RossumData: public TNamed {
public:
	/// Empty constructor
	RossumData();
	/// Opens a rossumData file
	RossumData(const char* name, const char* filename);
	/// Free memry allocated to TTrees
	~RossumData();
	/// Close rossumData file
	void CloseFile();
	/// Return the tree containing rossum data for a given run
	TTree* GetTree(Int_t runnum, const char* time = 0) const;
    /// List available trees
    void ListTrees() const;
    /// List available trees
    std::vector<Int_t>& GetRunsVector() const;
	/// Open a rossumData file
	Bool_t OpenFile(const char* name, Bool_t parse = kTRUE);
	/// Parse the relevant information from a rossumData file
	Bool_t ParseFile();
	/// Check if a rossumData file is currently open
	Bool_t IsFileOpen() { return fFile.get(); }
	/// Get the average beam current from a set of cup readings
	UDouble_t AverageCurrent(Int_t run, Int_t cup, Int_t iteration = 0,
										Double_t skipBegin = 10, Double_t skipEnd = 5);
	/// Plot FC4 / FC1 for a series of runs
	TGraph* PlotTransmission(Int_t* runs, Int_t nruns);
private:
	void SetCups();
	TTree* MakeTree();
	RossumData(const RossumData&) { }
	RossumData& operator= (const RossumData&) { return *this; }
private:
	typedef std::multimap<Int_t, std::pair<TTree*, std::string> > TreeMap_t;
	static TTree* GetTree_ (TreeMap_t::const_iterator& it) { return it->second.first; }
	static TTree* GetTree_ (TreeMap_t::iterator& it) { return it->second.first; }
	static std::string GetTime_ (TreeMap_t::const_iterator& it) { return it->second.second; }
	static std::string GetTime_ (TreeMap_t::iterator& it) { return it->second.second; }
	dragon::utils::AutoPtr<std::ifstream> fFile; //!
	TreeMap_t fTrees;
	std::map<std::string, Int_t> fWhichCup;
	Int_t fCup;
	Int_t fIteration;
	Double_t fTime;
	Double_t fCurrent;

	ClassDef(RossumData, 1);
};

/// Helper class for lab -> CM conversions (fully relativistic)
/*!
 * Calulations are fully relativistic:
 * For a moving beam (1) and stationary target (2):
 *       Ecm^2 = m1^2 + m2^2 + 2*m2*E1
 * or in terms of kinetic energy:
 *       (Tcm + m1 + m2)^2 = m1^2 + m2^2 + 2*m2*(m1 + T1)
 */
class LabCM {
public:
	/// Ctor
	LabCM(int Zbeam, int Abeam, int Ztarget, int Atarget);
	/// Ctor, w/ CM energy specification
	LabCM(int Zbeam, int Abeam, int Ztarget, int Atarget, double ecm);
	/// Ctor, using amu masses
	LabCM(double mbeam, double mtarget, double ecm = 0.);
	/// Get CM energy
	double GetEcm() const { return fTcm; } // keV
	/// Get beam energy in keV
	double GetEbeam() const;  // keV
	/// Get beam energy in keV/u
	double GetV2beam() const; // keV/u
	/// Get target frame energy in keV
	double GetEtarget() const;  // keV
	/// Get target frame energy in keV/u
	double GetV2target() const; // keV/u
	/// Get beam mass in amu
	double GetM1() const { return fM1 / dragon::Constants::AMU(); }
	/// Get target mass in amu
	double GetM2() const { return fM2 / dragon::Constants::AMU(); }
	/// Set CM energy
	void SetEcm(double ecm);
	/// Set beam energy in keV
	void SetEbeam(double ebeam);
	/// Set beam energy in keV/u
	void SetV2beam(double ebeam);
	/// Set target energy in keV
	void SetEtarget(double etarget);
	/// Set target energy in keV/u
	void SetV2target(double etarget);
	/// Set beam mass in amu
	void SetM1(double m1) { fM1 = m1*dragon::Constants::AMU(); }
	/// Set target mass in amu
	void SetM2(double m2) { fM2 = m2*dragon::Constants::AMU(); }
private:
	/// Ctor helper
	void Init(int Zbeam, int Abeam, int Ztarget, int Atarget, double ecm);
private:
	/// Beam mass [keV/c^2]
	double fM1;
	/// Target mass [keV/c^2]
	double fM2;
	/// Center of mass kinetic energy [keV]
	double fTcm;
};

/// Class to handle calculation of beam normalization
class BeamNorm: public TNamed {
public:
	/// Summarizes relevant normalization data for a run
	struct RunData {
		/// The run number
		Int_t runnum; // run numner
		/// Time of sb readings (how many sec used for normalization)
		Double_t time; // sb read time
		/// Number of surface barrier counts in _time_, per detector
		UDouble_t sb_counts[dragon::SurfaceBarrier::MAX_CHANNELS]; // sb counts for norm period
		/// Number of sb counts in the whole run
		UDouble_t sb_counts_full[dragon::SurfaceBarrier::MAX_CHANNELS]; // sb counts whole run
		/// Live time in _time_ used for SB normalization
		UDouble_t live_time; // live time in norm period
		/// Tail live time across the whole run
		UDouble_t live_time_tail; // live time for tail, whole run
		/// Head live time across the whole run
		UDouble_t live_time_head; // live time for head, whole run
		/// Coinc live time across the whole run
		UDouble_t live_time_coinc; // live time for coinc, whole run
		/// Average pressure in sb_time
		UDouble_t pressure; // avg. pressure in sb norm period
		/// Average pressure across the whole run
		UDouble_t pressure_full; // avg. pressure for whole run
		/// FC4 current, per iteration
		UDouble_t fc4[3]; // fc4 current, per reading (preceding run)
		/// FC1 current
		UDouble_t fc1; // fc1 current preceding run
		/// Transmission correction
		UDouble_t trans_corr; // transmission correction
		/// Normalization factor `R`, per sb detector
		UDouble_t sbnorm[dragon::SurfaceBarrier::MAX_CHANNELS]; // R-factor
		/// Total integrated beam particles for the run
		UDouble_t nbeam[dragon::SurfaceBarrier::MAX_CHANNELS]; // number of incoming beam
		/// Number of recoils
		UDouble_t nrecoil; // number of detected recoils
		/// Yield, per SB norm
		UDouble_t yield[dragon::SurfaceBarrier::MAX_CHANNELS]; // yield
		/// Set defaults
		RunData(): time(0), live_time(0), live_time_tail(0), pressure(0), pressure_full(0), trans_corr(1,0)
			{
				std::fill_n(sb_counts,      dragon::SurfaceBarrier::MAX_CHANNELS, UDouble_t(0));
				std::fill_n(sb_counts_full, dragon::SurfaceBarrier::MAX_CHANNELS, UDouble_t(0));
				std::fill_n(sbnorm,         dragon::SurfaceBarrier::MAX_CHANNELS, UDouble_t(0));
				std::fill_n(nbeam,          dragon::SurfaceBarrier::MAX_CHANNELS, UDouble_t(0));
				std::fill_n(fc4, 3, UDouble_t(0));
			}
	};

public:
	//// Dummy constructor
	BeamNorm();
	/// Construct from rossum file
	BeamNorm(const char* name, const char* rossumFile);
	/// Switch to a new rossum file
	void ChangeRossumFile(const char* name);
	/// Get a pointer to the rossum file
	RossumData* GetRossumFile() const { return fRossum.get(); }
	/// Batch calculate over a chain of files
	void BatchCalculate(TChain* chain, Int_t chargeBeam,
											Double_t pkLow0, Double_t pkHigh0,
											Double_t pkLow1, Double_t pkHigh1,
											const char* recoilGate = 0,
											Double_t time = 120, Double_t skipBegin = 10, Double_t skipEnd = 5);
	/// Batch calculate over a chain of files
	void BatchCalculate(TChain* chain, Int_t chargeBeam,
											Double_t pkLow0, Double_t pkHigh0,
											Double_t pkLow1, Double_t pkHigh1,
											const TCut& recoilGate,
											Double_t time = 120, Double_t skipBegin = 10, Double_t skipEnd = 5)
		{
			BatchCalculate(chain, chargeBeam, pkLow0, pkHigh0, pkLow1, pkHigh1,
										 recoilGate.GetTitle(), time, skipBegin, skipEnd);
		}
	/// Calculate the number of recoils per run
	void CalculateRecoils(TFile* datafile, const char* tree, const char* gate);
	/// Integrate the surface barrier counts at the beginning and end of a run
	Int_t ReadSbCounts(TFile* datafile, Double_t pkLow0, Double_t pkHigh0,
										    Double_t pkLow1, Double_t pkHigh1,Double_t time = 120.);
	/// Read rossum FC4 current
	void ReadFC4(Int_t runnum, Double_t skipBegin = 10, Double_t skipEnd = 5);
	/// Calculate R and total beam particles
	void CalculateNorm(Int_t run, Int_t chargeState);
	/// Return stored values of run data
	RunData* GetRunData(Int_t runnum);
	/// Return a vector of run numbers used in the calculation
	std::vector<Int_t>& GetRuns() const;
	/// Plot some parameter as a function of run number
    TGraphAsymmErrors* Plot(const char* param, Marker_t marker = 21, Color_t markerColor = kBlack);
	/// Plot some parameter as a function of run number (alt. implementation)
	TGraphErrors* PlotVal(const TString& valstr, int which = 0,
												Marker_t marker = 21, Color_t markerColor = kBlack);
	/// Plot number of beam particles with manual sbnorm specification
	TGraphErrors* PlotNbeam(double sbnorm, int which = 0, Marker_t marker = 21, Color_t markerColor = kBlack);
	/// Draw run data information
	Long64_t Draw(const char* varexp, const char* selection, Option_t* option = "",
								Long64_t nentries = 1000000000, Long64_t firstentry = 0);
	/// TObject overloaded version of Draw()
	void Draw(Option_t* option) { Draw(option, "", "", 1000000000, 0); }
	///
	UDouble_t GetEfficiency(const char* name) const
		{
			std::map<std::string, UDouble_t>::const_iterator i = fEfficiencies.find(name);
			return i != fEfficiencies.end() ? i->second : UDouble_t(1,0);
		}
	void SetEfficiency(const char* name, UDouble_t value) { fEfficiencies[name] = value; }
	void SetEfficiency(const char* name, Double_t value)  { fEfficiencies[name] = UDouble_t(value, 0); }
	/// Correct for FC4->FC1 transmission changes relative to a single "good" run
	void CorrectTransmission(Int_t reference);
	UDouble_t CalculateEfficiency(Bool_t print = kTRUE);
	UDouble_t CalculateYield(Int_t whichSb, Int_t type = kHiSingles, Bool_t print = kTRUE); // type: 1 = gamma sing., 3 = hi sing. 5 = coinc.

private:
	/// Private GetRunData(), creates new if it's not made
	RunData* GetOrCreateRunData(Int_t runnum);
	Bool_t HaveRossumFile() { return fRossum.get(); }
	UInt_t GetParams(const char* param, std::vector<Double_t> *runnum, std::vector<UDouble_t> *parval);
	BeamNorm(const BeamNorm&) { }
	BeamNorm& operator= (const BeamNorm&) { return *this; }

public:
	TTree fRunDataTree;
private:
	RunData* fRunDataBranchAddr;
	std::map<Int_t, RunData> fRunData;
	dragon::utils::AutoPtr<RossumData> fRossum;
	std::map<std::string, UDouble_t> fEfficiencies;

	ClassDef(BeamNorm, 2);
};


/// Stopping power calculator
class StoppingPowerCalculator {
public:
	enum XAxisType_t {
		kPRESSURE,
		kDENSITY
	};
	enum YAxisType_t {
		kMD1,
		kENERGY
	};
	struct Measurement_t {
		UDouble_t pressure;
		UDouble_t density;
		UDouble_t md1;
		UDouble_t energy;
	};
public:
	/// Dummy constructor
	StoppingPowerCalculator() { }
	/// Construct a stopping power calculator with parameters
	StoppingPowerCalculator(Int_t beamCharge, Double_t beamMass, Int_t nmol,
											  Double_t targetLen = 12.3, Double_t targetLenErr = 0.4,
											  Double_t cmd1 = 4.815e-4, Double_t cmd1Err = 7.2345e-7,
											  Double_t temp = 300.);
	/// Get the target length
	UDouble_t GetTargetLength() const { return fTargetLength; } // cm
	/// Set the target length
	void SetTargetLength(Double_t len, Double_t err) { fTargetLength = UDouble_t(len, err); } // cm
	/// Get the temperature
	Double_t GetTemp() const { return fTemp; }     // kelvin
	/// Set the temperature
	void SetTemp(Double_t temp) { fTemp = temp; }  // kelvin
	/// Get beam charge
	Int_t GetQbeam() const { return fBeamCharge; } // e
	/// Set beam charge
	void SetQbeam(Int_t q) { fBeamCharge = q; }    // e
	/// Get beam mass
	Double_t GetBeamMass() const { return fBeamMass; }    // amu
	/// Set beam mass
	void SetBeamMass(Double_t mass) { fBeamMass = mass; } // amu
	/// Get atoms/molecule
	Int_t GetNmol() const { return fNmol; }        // atoms/molecule
	/// Set atoms/molecule
	void SetNmol(Int_t nmol) { fNmol = nmol; }     // atoms/molecule
	/// Get MD1 constant
	UDouble_t GetMd1Constant() const { return fMd1Constant; } // keV/u * Gauss^2
	/// Set MD1 constant
	void SetMd1Constant(Double_t md1, Double_t md1Err) { fMd1Constant = UDouble_t (md1, md1Err); } // keV/u * Gauss^2
	/// Add a pressure, energy measurement
	void AddMeasurement(Double_t pressure, Double_t pressureErr, Double_t md1, Double_t md1Err);
	/// Get a pressure, energy measurement
	Measurement_t GetMeasurement(Int_t index) const;
	/// Get the number of measurements
	Int_t GetNmeasurements() const { return fPressures.size(); }
	/// Remove a pressure, energy measurement
	void RemoveMeasurement(Int_t index);
	/// Plot energy vs. pressure or density
	TGraph* PlotMeasurements(XAxisType_t xaxis = kPRESSURE, YAxisType_t yaxis = kENERGY, Bool_t draw = kTRUE) const;
	/// Calculate the `epsilon` parameter - slope of eloss vs. (atoms/cm^2)
	UDouble_t CalculateEpsilon(TGraph** plot = 0, UDouble_t* ebeam = 0);
	/// Calculate the beam energy (intercept of E vs. P)
	UDouble_t CalculateEbeam(TGraph** plot = 0);
public:
	/// Convert pressure in torr to dyn/cm^2
	static Double_t TorrCgs(Double_t torr); // dyn/cm^2
	/// Convert pressure in torr to dyn/cm^2 (with uncertainty)
	static UDouble_t TorrCgs(UDouble_t torr); // dyn/cm^2
	/// Calculate target density in atoms/cm^2
	static Double_t CalculateDensity(Double_t pressure, Double_t length, Int_t nmol, Double_t temp = 300.);
	/// Calculate target density in atoms/cm^2 (with uncertainty)
	static UDouble_t CalculateDensity(UDouble_t pressure, UDouble_t length, Int_t nmol, Double_t temp = 300.);
	/// Calculate beam energy from MD1 field
	static UDouble_t CalculateEnergy(Double_t md1, Double_t md1Err, Int_t q, Double_t m,
												   Double_t cmd1 = 4.815e-4, Double_t cmd1Err = 0.0015*4.815e-4);
private:
	/// Atomic mass of beam
	Double_t fBeamMass;
	/// Charge of beam
	Int_t fBeamCharge;
	/// Target atoms / gas molecule
	Int_t fNmol;
	/// Target effective length
	UDouble_t fTargetLength;
	/// Temperature
	Double_t fTemp;
	/// MD1 constant
	UDouble_t fMd1Constant;
	/// Pressure  measurements
	std::vector<UDouble_t> fPressures;
	/// Pressure  measurements
	std::vector<UDouble_t> fDensities;
	/// MD1 measurements
	std::vector<UDouble_t> fMd1;
	/// Energy measurements
	std::vector<UDouble_t> fEnergies;
};


/// Live time calculator
/*!
 * Calculates the live time by looking at the measured
 * run time (with the IO32 clock, so 50 ns precision) and
 * sum of measured busy times for each event. The live time is
 * defined as:
 *      (<run time> - <busy time>) / <run time>
 *
 * Calculations can be performed for a complete run, subset of a
 * run or a chain of runs. This class also calculates the live
 * time for coincidences. For more information about how this is
 * done, see the dragon::CoincBusytime class documentation
 */
class LiveTimeCalculator {
public:
	/// Empty
	LiveTimeCalculator();
	/// Initialize with a run file and perform optional calculation
	LiveTimeCalculator(TFile* file, Bool_t calculate = kTRUE);

	/// Perform live time calculation across an entire run
	void Calculate();
	/// Perform live time calculation across a subset of the run
	void CalculateSub(Double_t tbegin, Double_t tend);
	/// Perform live time calculation across a chain of runs
	void CalculateChain(TChain* chain);

	/// Returns the sum of busy times during the run (or subrun) in seconds
	Double_t GetBusytime(const char* which) const; // seconds
	/// Returns the run time in seconds
	Double_t GetRuntime (const char* which) const; // seconds
	/// Returns the live time fraction
	Double_t GetLivetime(const char* which) const; // fraction

	/// \brief Returns the error on the livetime
	/// \details The relative error should
	Double_t GetLivetimeError(const char* which) const
		{ return GetLivetime(which) * 50/1e6; } // seconds

	/// Reuturn pointer to the currently set run file
	TFile* GetFile() const { return fFile; }
	/// \brief Change the run file
	/// \atention Does not clear previous calculations
	void SetFile(TFile* file) { fFile = file; }

	/// Reset all stored data (livetimes, etc.) to zero
	void Reset();

public:
	/// Calculate the run time from saved ODB file
	static Double_t CalculateRuntime(midas::Database* db, const char* which,
																	 Double_t& start, Double_t& stop); // seconds (static function)
	/// Calculate the run time from saved ODB file
	static Double_t CalculateRuntime(midas::Database* db, const char* which)
		{ Double_t a,b; return CalculateRuntime(db, which, a, b); } // seconds (static function)

private:
	/// Make sure fFile is valid
	Bool_t CheckFile(TTree*& t1, TTree*& t3, midas::Database*& db);
	/// Does the work of live time calculations
	void DoCalculate(Double_t tbegin, Double_t tend);

private:
	/// Run file (no ownership)
	TFile* fFile;
	/// Run times
	Double_t fRuntime [3];
	/// Busy times
	Double_t fBusytime[3];
	/// Live times
	Double_t fLivetime[3];
};

/// Class to calculate total busy time for coincidence events
/*!
 * For coincidence events, the total busy time is the sum or times during which the head _or_
 * tail DAQ is busy. Any genuine coincidence triggers arriving when either the head or tail is
 * busy will not be marked as such. This covers all possible classes of lost coincidence events:
 *   - Coincidences arriving when both the head and tail are busy: the trigger is simply not recorded.
 *   - Coincidences arriving when either the head or is tail is busy but not the other: the coincidence
 *     will be incorrectly marked as a singles event.
 *   - Coincidences whose match window is interrupted by an uncorrelated singles event. For example, a
 *     capture reaction happens, triggers the BGOs and then uncorrelated leaky beam triggers the end
 *     detector before the recoil gets there. This will still be marked as a coincidence event in the
 *     analysis, but the head event will not be correlated with the correct tail one. Thus the _real_
 *     coincidence has been lost, albeit to a false (random) coincidence.
 *
 * The coincidence dead time is then the sum the time during which the head or tail is busy divided by
 * the "coincidence runtime", i.e. the time during which both the head and tail are accepting triggers.
 *
 * For information about how the busy time is calculated in practice, see the documentation on
 * the Calculate() member funcion.
 */
class CoincBusytime {
public:
	/// Helper class to store relevant information about a triggered event.
	class Event {
 public:
		/// Trigger time
		Double_t fTrigger;
		/// Busy time
		Double_t fBusy;
 public:
		/// Initialize data
		Event(Double_t trig = 0, Double_t busy = 0):
			fTrigger(trig), fBusy(busy) { }
		/// Calculate the end time of event processing (trigger + busy)
		Double_t End() const { return fTrigger + fBusy; }
		/// Compare two events by the _trigger_ time: `return lhs.fTrigger < rhs.fTrigger;`
		static Bool_t TriggerCompare(const Event& lhs, const Event rhs)
			{	return lhs.fTrigger < rhs.fTrigger;	}
	};

public:
	/// Set flag specifying that events are not yet sorted by trigger time
	CoincBusytime(size_t reserve = 0);
	/// Insert event into the colletion of triggered events
	void AddEvent(Double_t trigger, Double_t busy);
	/// Calculate the total time during which the head or tail is busy
	Double_t Calculate(); // seconds

private:
	/// Sort events by the trigger time.
	void Sort();

public:
		/// Type of the event collection
	typedef std::vector<Event> Container_t;

private:
	/// Collection of triggered events
	Container_t fEvents;
	/// Flag specifying whether or not events are sorted
	Bool_t fIsSorted;
};


/// Class to calculate resonance strength (omega-gamma) from yield & stopping power measurements.
class ResonanceStrengthCalculator {
public:
	/// Ctor
	ResonanceStrengthCalculator(Double_t eres, Double_t mbeam, Double_t mtarget,
															dragon::BeamNorm* beamNorm, UDouble_t epsilon);

	/// Returns fBeamNorm
	BeamNorm* GetBeamNorm() const { return fBeamNorm; }
	/// Set a new BeamNorm calculator
	void SetBeamNorm(BeamNorm* norm) { fBeamNorm = norm; }
	/// Returns fEpsilon
	UDouble_t GetEpsilon() const { return fEpsilon; }          // eV*cm^2/atom
	/// Change fEpsilon
	void SetEpsilon(UDouble_t epsilon) { fEpsilon = epsilon; } // eV*cm^2/atom
	/// Returns the beam mass in amu
	Double_t GetBeamMass() const { return fBeamMass; }    // amu
	/// Set the beam mass in amu
	void SetBeamMass(Double_t mass) { fBeamMass = mass; } // amu
	/// Returns the target mass in amu
	Double_t GetTargetMass() const { return fTargetMass; }    // amu
	/// Set the target mass in amu
	void SetTargetMass(Double_t mass) { fTargetMass = mass; } // amu
	/// Returns the resonance energy in keV
	Double_t GetResonanceEnergy() const { return fResonanceEnergy; }    // keV
	/// Set the resonance energy in keV
	void SetResonanceEnergy(Double_t mass) { fResonanceEnergy = mass; } // keV
	/// Calculate total resonance strength from all runs.
	UDouble_t CalculateResonanceStrength(Int_t whichSb, Int_t type = kHiSingles, Bool_t print = kTRUE); // type: 1 = gamma sing., 3 = hi sing. 5 = coinc.

	/// Plot resonance strength vs. run number
	TGraph* PlotResonanceStrength(Int_t whichSb);

public:
	/// Calculate DeBroglie wavelength in the CM system
	static UDouble_t CalculateWavelength(UDouble_t eres /*keV*/, Double_t mbeam /*amu*/, Double_t mtarget /*amu*/); // cm
	/// Calculate resonance strength
	static UDouble_t CalculateResonanceStrength(UDouble_t yield, UDouble_t epsilon, UDouble_t wavelength,
																							Double_t mbeam, Double_t mtarget); // eV

private:
	BeamNorm* fBeamNorm;
	UDouble_t fEpsilon;
	Double_t fBeamMass;
	Double_t fTargetMass;
	Double_t fResonanceEnergy;  ///\todo should have uncertainty
};

/// Class to calculate cross sections from target data and yield.
class CrossSectionCalculator {
public:
	/// Constructor
	CrossSectionCalculator(dragon::BeamNorm* beamNorm, Int_t nmol /*atoms/molecule*/,
												 Double_t temp = 300 /*kelvin*/, Double_t targetLen = 12.3 /*cm*/);

public:
	/// Calculate cross sections for runs in fBeamNorm
	UDouble_t Calculate(Int_t whichSB, const char* prefix = "", Bool_t printTotal = true);

	/// Plot run-by-run cross sections
	TGraph* Plot(Marker_t marker = 21, Color_t color = 1);

	/// Print run-by-run cross sections
	void Print();

	/// Calculate cross section in barns
	static UDouble_t CalculateCrossSection(UDouble_t yield, UDouble_t pressure, Double_t length, Int_t nmol, Double_t temp = 300.);

private:
	/// Pointer to constructed beam norm class, takes no ownership
	dragon::BeamNorm* fBeamNorm;
	/// Number of target atoms per molecule (2 for hydrogen, 1 for helium)
	Int_t fNmol;
	/// Target temperature
	Double_t fTemp;
	/// Effective target length
	Double_t fTargetLen;
	/// Calculated cross sections, run-by-run
	std::vector<UDouble_t> fCrossSections;
	/// Total cross section across all runs
	UDouble_t fTotalCrossSection;
	/// Cross section unit prefix
	std::string fPrefix;
};


} // namespace dragon




#ifdef __MAKECINT__
#pragma link C++ function dragon::MeasurementWeightedAverage(std::vector<UDouble_t>::iterator, std::vector<UDouble_t>::iterator);
#pragma link C++ function dragon::MeasurementWeightedAverage(UDouble_t*, UDouble_t*);
#endif



#endif
