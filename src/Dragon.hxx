///
/// \file Dragon.hxx
/// \author G. Christian
/// \author C. Stanford
/// \brief Defines dragon detector classes
/// \attention The '//!' or '//#' comments following variable declarations are not
///  optional! They are actually used in processing of the file with ROOTCINT.
///
#ifndef HAVE_DRAGON_HXX
#define HAVE_DRAGON_HXX
#include <stdint.h>
#include <string>
#include <sstream>
#include "utils/VariableStructs.hxx"
#include "utils/Banks.hxx"
#include "midas/Event.hxx"
#include "Vme.hxx"

// Forward declare midas coincidence event //
namespace midas { struct CoincEvent; class Database; }

// Forward declare vme classes //
namespace vme {
class V792;
class V1190;
typedef V792 V785;
}

///
/// Encloses dragon detector classes
///
namespace dragon {

class Tail; // forward declaration


// ======= Class definitions ======== //


///
/// Global run parameters
///
class RunParameters {
public: // Constants
	static const int MAX_FRONTENDS = 2; //!
public: // Methods
	/// Constructor, calls reset()
	RunParameters();
	/// Sets all data to defaults
	void reset();
	/// Reads data from the ODB or a midas file
	void read_data(const midas::Database& db);

public: // Data
	/// Run start time from the tsc
	/*! [0]: head, [1]: tail */
	double run_start[MAX_FRONTENDS];
	/// Run stop time from the tsc
	/*! [0]: head, [1]: tail */
	double run_stop[MAX_FRONTENDS];
	/// Trigger start time from the tsc
	/*! [0]: head, [1]: tail */
	double trigger_start[MAX_FRONTENDS];
	/// Trigger stop time from the tsc
	/*! [0]: head, [1]: tail */
	double trigger_stop[MAX_FRONTENDS];
};


///
/// The BGO array
///
class Bgo {
public: // Constants
	/// Number of channels in the BGO array
	static const int MAX_CHANNELS = 30; //!

public: // Methods
	/// Constructor, initializes data values
	Bgo();
	/// Sets all data values to dragon::NO_DATA
	void reset();
	/// Read adc & tdc data
	void read_data(const vme::V792& adc, const vme::V1190& tdc);
	/// Do higher-level parameter calculations
	void calculate();

public: // Data
	/// Calibrated energies
	double ecal[MAX_CHANNELS];    //#
	/// Calibrates times
	double tcal[MAX_CHANNELS];    //#
	/// Sorted (high->low) energies
	double esort[MAX_CHANNELS];   //#
	/// Sum of all \e valid energies
	double sum; //#
	/// Which detector was the highest energy hit
	int hit0;  //#
	/// x position of the highest energy hit
	double x0; //#
	/// y position of the highest energy hit
	double y0; //#
	/// z position of the highest energy hit
	double z0; //#
	/// time of the highest energy hit
	double t0; //#

public: // Subclasses
	///
	/// Bgo variables
	///
	class Variables {
 public: // Methods
		/// Sets data to defaults
		Variables();
		/// Set values to defaults
		void reset();
		/// Set variable values from an ODB file
		void set(const char* odb_file);

 public: // Data
		/// Adc variables
		utils::AdcVariables<MAX_CHANNELS> adc;
		/// Tdc variables
		utils::TdcVariables<MAX_CHANNELS> tdc;
		/// Detector positions in space
		utils::PositionVariables<MAX_CHANNELS> pos;
	};

public: // Subclass instances
	/// Instance of Bgo::Variables
	Bgo::Variables variables;   //!
};


///
/// Double-Sided Silicon Strip Detector
///
class Dsssd {
public: // Constants
	/// Number of channels
	static const int MAX_CHANNELS = 32; //!

public: // Methods
	/// Constructor, initialize data
	Dsssd();
	/// Reset all data to VME::none
	void reset();
	/// Read data from vme modules
	void read_data(const vme::V785 adcs[], const vme::V1190& tdc);
	/// Performs energy and time calibrations
	void calculate();

public:
	/// Calibrated energy signals
	double ecal[MAX_CHANNELS]; //#
	/// Highest energy signal in the front strips (0 - 15)
	double efront;      //#
	/// Highest energy signal in the back strips (16 - 31)
	double eback;       //#
	/// Which strip was hit in the front strips
	uint32_t hit_front; //#
	/// Which strip was hit in the back strips
	uint32_t hit_back;  //#
	/// Calibrated time signal
	double tcal;        //#

public: // Subclasses
	///
	/// Dsssd Variables Class
	///
	class Variables {
 public: // Methods
		/// Constructor, sets data to generic values
		Variables();
		/// Set variable values from an ODB file
		void set(const char* odb_file);
		/// Reset all variable values to defaults
		void reset();

 public: // Data
		/// Adc variables for the energy signals
		utils::AdcVariables<32> adc;
		/// Tdc variables
		utils::TdcVariables<1> tdc;
	};

public: // Subclass instances
	/// Instance of Dsssd::Variables
	Dsssd::Variables variables; //!
};

///
/// Ionization chamber
///
class IonChamber {
public: // Constants
	/// Number of anodes
	static const int MAX_CHANNELS = 4; //!

public: // Methods
	/// Constructor, initialize data
	IonChamber();
	/// Reset all data to VME::none
	void reset();
	/// Read midas event data
	void read_data(const vme::V785 adcs[], const vme::V1190& tdc);
	/// Calculate higher-level parameters
	void calculate();

public: // Data
	/// Calibrated anode signals
	double anode[MAX_CHANNELS]; //#
	/// Time signal
	double tcal; //#
	/// Sum of anode signals
	double sum;  //#

public: // Subclasses
	///
	/// Ion chamber variables
	///
	class Variables {
 public: // Methods
		/// Constructor, sets data to generic values
		Variables();
		/// Reset data to default values
		void reset();
		/// Set variable values from an ODB file
		void set(const char* odb_file);

 public: // Data
		/// Anode variables
		utils::AdcVariables<MAX_CHANNELS> adc;
		/// Tdc variables
		utils::TdcVariables<1> tdc;
	};

public: // Subclass instances
	/// Variables instance
	IonChamber::Variables variables; //!
};


///
/// Micro channel plates
///
class Mcp {
public: // Constants
	/// Number of anodes on MCP0
	static const int MAX_CHANNELS = 4;  //!
	/// Number of separate mcp detctors
	static const int NUM_DETECTORS = 2; //!

public: // Methods
	/// Constructor, initialize data
	Mcp();
	/// Reset all data to VME::none
	void reset();
	/// Read midas event data
	void read_data(const vme::V785 adcs[], const vme::V1190& tdc);
	/// Calibrate ADC/TDC signals, calculate x and y positions
	void calculate();

public: // Data
	/// Anode signals
	double anode[MAX_CHANNELS]; //#
	/// TDC signals
	double tcal[NUM_DETECTORS]; //#
	/// Sum of anode signals
	double esum; //#
	/// TAC signal (MCP_TOF).
	double tac;  //#
	/// x-position
	double x;    //#
	/// y-position
	double y;    //#

public: // Subclasses
	///
	/// MCP Variables
	///
	class Variables {
 public: // Methods
		/// Constructor, sets data to generic values
		Variables();
		/// Restet variables to default values
		void reset();
		/// Set variable values from an ODB file
		void set(const char* odb_file);

 public: // Data
		/// Adc variables for the anode signals
		utils::AdcVariables<MAX_CHANNELS> adc;
		/// Adc variables for the TAC signal
		utils::AdcVariables<1> tac_adc;
		/// Tdc variables
		utils::TdcVariables<NUM_DETECTORS> tdc;
	};

public: // Subclass instances
  /// Variables instance
	Mcp::Variables variables; //!
};


///
/// Surface barrier detectors
///
class SurfaceBarrier {
public: // Constants
	/// Number of detectors
	static const int MAX_CHANNELS = 2; //!

public: // Subclasses
	///
	/// Surface Barrier Variables
	///
	class Variables {
 public: // Methods
		/// Constructor, sets data to generic values
		Variables();
		/// Sets data to defaults
		void reset();
		/// Set variable values from an ODB file
		void set(const char* odb_file);

 public: // Data
		/// Adc variables
		utils::AdcVariables<MAX_CHANNELS> adc;
	};

public: // Methods
	/// Constructor, initialize data
	SurfaceBarrier();
	/// Reset all data to vme::NONE
	void reset();
	/// Read midas event data
	void read_data(const vme::V785 adcs[], const vme::V1190&);
	/// Final energy calculation
	void calculate();

public: // Data
	/// Energy signals
	double ecal[MAX_CHANNELS]; //#
	/// Variables instance
	SurfaceBarrier::Variables variables; //!
};


///
/// Sodium Iodide (NaI) detectors
///
class NaI {
public: // Constants
	/// Number of detectors
	static const int MAX_CHANNELS = 2; //!

public: // Methods
	/// Constructor, initialize data
	NaI();
	/// Reset all data to vme::NONE
	void reset();
	/// Read event data from vme modules
	void read_data(const vme::V785 adcs[], const vme::V1190&);
	/// Do energy calibrations
	void calculate();

public: // Data
	/// Calibrated energy signals
	double ecal[MAX_CHANNELS]; //#

public: // Subclasses
	///
	/// NaI variables
	///
	class Variables {
 public: // Methods
		/// Constructor, sets data to generic values
		Variables();
		/// Set variable values to defaults
		void reset();
		/// Set variable values from an ODB file
		void set(const char* odb_file);

 public: // Data
		/// Adc variables
		utils::AdcVariables<MAX_CHANNELS> adc;
	};

public: // Subclass instances
	/// Variables instance
	NaI::Variables variables; //!
};


///
/// Germanium (Ge) detector
///
class Ge {
public: // Methods
	/// Constructor, initialize data
	Ge();
	/// Reset all data to vme::NONE
	void reset();
  /// Read event data from the ADC
	void read_data(const vme::V785 adcs[], const vme::V1190&);
	/// Do energy calibrations, pedestal subtractions
	void calculate();

public: // Data
	/// Calibrated energy signal
	double  ecal; //#

public: // Subclasses
	///
	/// Ge variables
	///
	class Variables {
 public: // Methods
		/// Constructor, sets data to generic values
		Variables();
		/// Reset variables to default values
		void reset();
		/// Set variable values from an ODB file
		void set(const char* odb_file);

 public: // Data
		/// ADC variables
		utils::AdcVariables<1> adc;
	};

public: // Subclass instances
	/// Variables instance
	Ge::Variables variables; //!
};


///
/// \brief Generic time-of-flight class for heavy-ion detectors
/// \details Calculates time of flight between all detectors downstream of MCP0.
///
class HiTof {
public: // Methods
	/// Sets data to defaults
	HiTof();
	/// Sets data to dragon::NO_DATA
	void reset();
	/// Performs calibration of xover tdc and TOF calculations
	void calculate(const dragon::Tail* tail);

public: // Data
	/// MCP0 -> MCP1
	double mcp;
#ifndef DRAGON_OMIT_DSSSD
	/// MCP0 -> DSSSD
	double mcp_dsssd;
#endif
#ifndef DRAGON_OMIT_IC
	/// MCP0 -> Ion-chamber
	double mcp_ic;
#endif
};


///
/// Collection of all head detectors and VME modules
///
class Head {
public: // Methods
	/// Initializes data values
	Head();
	/// Sets all data values to vme::NONE or other defaults
	void reset();
	///  Reads all variable values from an ODB (file or online)
	void set_variables(const char* odb_file);
	/// Unpack raw data into VME modules
	void unpack(const midas::Event& event);
	/// Calculate higher-level data for each detector, or across detectors
	void calculate();

public: // Data
	/// Bank names
	utils::EventBanks<1, 1> banks; //!
	/// Midas event header
	midas::Event::Header header; //#

	// Vme modules
#ifdef DISPLAY_MODULES
	/// IO32 FPGA
	vme::Io32 io32;   //#
	/// CAEN V792 QDC
	vme::V792 v792;   //#
	/// CAEN V1190 TDC
	vme::V1190 v1190; //#
#else // Supress writing of vme modules to TTree
	vme::Io32 io32;   //!
	vme::V792 v792;   //!
	vme::V1190 v1190; //!
#endif // #ifdef DISPLAY_MODULES

  /// Bgo array
	dragon::Bgo bgo;
	/// Head [bgo] trigger time
	double tcal0;
	/// Crossover [tail] trigger time
	double tcalx;

public: // Subclasses
	///
	/// Head variables
	///
	class Variables {
 public: // Data
		/// Crossover TDC channel variables
		utils::TdcVariables<1> xtdc;
 public: // Methods
		/// Sets data to defaults
		Variables();
		/// Sets data to defaults
		void reset();
		/// Sets data from ODB
		void set(const char* odb);
	};

public: // Subclass instances
	/// Variables instance
	Head::Variables variables; //!
};


///
/// Collection of all tail detectors and VME modules
///
class Tail {
public: // Constants
	/// Number of ADC (caen v785) modules
	static const int NUM_ADC = 2;

public: // Methods
	/// Initializes data values
	Tail();
	/// Sets all data values to vme::NONE or other defaults
	void reset();
	/// Reads all variable values from an ODB (file or online)
	void set_variables(const char* odb_file);
	/// Unpack raw data into VME modules
	void unpack(const midas::Event& event);
	/// Calculate higher-level data for each detector, or across detectors
	void calculate();

public: // Class data
	/// Bank names
	utils::EventBanks<2, 1> banks; //!
	/// Midas event header
	midas::Event::Header header;   //#
	// Vme modules (write to TTree)
#ifdef DISPLAY_MODULES
	/// IO32 FPGA
	vme::Io32 io32;          //#
	/// CAEN v785 ADC (x2)
	vme::V785 v785[NUM_ADC]; //#
	/// CAEN v1190 TDC
	vme::V1190 v1190;        //#
#else // Supress writing of vme modules to TTree
	vme::Io32 io32;          //!
	vme::V785 v785[NUM_ADC]; //!
	vme::V1190 v1190;        //!
#endif
#ifndef DRAGON_OMIT_DSSSD  // Optional suppression of DSSSD
	/// DSSSD detector
	Dsssd dsssd;             //
#endif
#ifndef DRAGON_OMIT_IC     // Optional suppresion of IonChamber
	/// Ionization chamber
	IonChamber ic;           //
#endif
#ifndef DRAGON_OMIT_NAI    // Optional suppression of NaI detectors
	/// NaI detetors
	NaI nai;                 //
#endif
#ifndef DRAGON_OMIT_GE     // Optional suppression of Ge detector
	/// Germanium detector
	Ge ge;                   //
#endif

	/// MCPs
	Mcp mcp;                 //
	/// Surface barrier detectors
	SurfaceBarrier sb;       //
	/// Time-of-flights
	HiTof tof;               //
	/// Trigger [tail] tdc value
	double tcal0;
	/// Crossover [head] tdc value
	double tcalx;

public: // Subclasses
	// Subclasses //
	class Variables {
 public: // Methods
		/// Sets data to defaults
		Variables();
		/// Sets data to defaults
		void reset();
		/// Sets data from ODB
		void set(const char* odb);
 public: // Data
		/// Crossover TDC channel variables
		utils::TdcVariables<1> xtdc;
	};

public: // Subclass instances
	/// Variables instance
	Variables variables; //!
};


///
/// A complete Dragon (coincidence) event
///
class Coinc {
public: // Methods
	/// Empty
	Coinc();
	/// Construct from a head and tail event
	Coinc(const Head& head, const Tail& tail);
	/// Reset all modules (set data to defaults)
	void reset();
	/// Reads all variable values from an ODB (file or online)
	void set_variables(const char* odb_file);
	/// Copy data from head and tail coincidence events
	void compose_event(const Head& head_, const Tail& tail_);
	/// Unpack raw data from a midas::CoincEvent
	void unpack(const midas::CoincEvent& coincEvent);
	/// Calculates both singles and coincidence parameters
	void calculate();

public: // Data
	/// Head (gamma-ray) part of the event
	Head head;
	/// Tail (heavy-ion) part of the event
	Tail tail;
	/// (tail - head) io32 trigger times (usec)
	double xtrig;
	/// Crossover time-of-flight from the head TDC
	double xtofh;
	/// Crossover time-of-flight from the tail TDC
	double xtoft;
};


///
/// Generic dragon scaler class
///
class Scaler {
public: // Constants
	/// Number of scaler channels
	static const int MAX_CHANNELS = 17; //!

public: // Methods
  /// Initialize data
  Scaler(const char* name = "head");
  /// Reset all data to zero
  void reset();
  /// Unpack Midas event data into scalar data structiures
  void unpack(const midas::Event& event);
	/// Returns the name of a given scaler channel
	const std::string& channel_name(int ch) const;
	/// Sets variable values
	void set_variables(const char* odb);
	/// Set branch alises in a ROOT TTree.
	template <class T>
	void set_aliases(T* t, const char* branchName) const;

public: // Data
	/// Number of counts in a single read period
	uint32_t count[MAX_CHANNELS];    //#
	/// Number of counts over the course of a run
	uint32_t sum[MAX_CHANNELS];      //#
	/// Average count rate over the course of a run
	double rate[MAX_CHANNELS];       //#

public: // Subclasses
	///
  /// Scalar variables
	///
  class Variables {
 public: // Methods
    /// Constuctor
    Variables(const char* name);
		/// Resets names to default values
		void reset();
		/// Set names from ODB
		void set(const char* odb);
		/// Set bank names
		void set_bank_names(const char* base);

 public: // Data
		/// Name of a given channel
		std::string names[MAX_CHANNELS]; //!
		/// Frontend bank names
		utils::ScalerBanks bank_names; //!
		/// Base odb path
		std::string odb_path; //!
  };

public: // Subclass instances
	/// Variables instance
	Variables variables;
};

} // namespace dragon



// ======= Inlined Implementations ======== //

template <class T>
inline void dragon::Scaler::set_aliases(T* t, const char* branchName) const
{
	/*!
	 * Sets tree branch aliases based on variable values - results in easier to use
	 * branch names (e.g. something descriptive instead of 'scaler.sum[0]', etc.)
	 *
	 * \param t Pointer to a class having a "SetAlias(const char*, const char*) method
	 * (e.g. TTree) and for which you want to set aliases.
	 * \param branchName "Base" name of the branch
	 * \tparam T Some class with a "SetAlias" method.
	 *
	 * \note Intended for TTree, but defined as a template
	 * function to maintain compatability with systems that don't use ROOT. In the case that ROOT is
	 * not available and this function is never used, it simply gets ignored by the compiler. Note also,
	 * that for systems with ROOT, a CINT dictionary definition is automoatically provided for TTree
	 * as the template parameter.
	 *
	 * Example:
	 * \code
	 * TTree t("t","");
	 * dragon::Scaler scaler;
	 * scaler.variables.names[0] = "bgo_triggers_presented";
	 * void* pScaler = &scaler;
	 * t.Branch("scaler", "dragon::Scaler", &pScaler);
	 * scaler.set_aliases(&t, "scaler");
	 * t.Fill(); // adds a events worth of data
	 * t.Draw("scaler_count_bgo_triggers_presented"); // same as doing t.Draw("scaler.count[0]");
	 * \endcode
	 */
	const std::string chNames[3] = { "count", "sum", "rate" };
	for(int i=0; i< MAX_CHANNELS; ++i) {
		for(int j=0; j< 3; ++j) {
			std::stringstream oldName, newName;
			oldName << branchName << "." << chNames[j] << "[" << i << "]";
			newName << branchName << "_" << chNames[j] << "_" << variables.names[i];

			t->SetAlias(newName.str().c_str(), oldName.str().c_str());
		}
	}
}


// ======= ROOTCINT stuff ======== //

#ifdef __MAKECINT__
// Create CINT links to template classes //
#pragma link C++ class utils::AdcVariables<dragon::Bgo::MAX_CHANNELS>;
#pragma link C++ class utils::TdcVariables<dragon::Bgo::MAX_CHANNELS>;
#pragma link C++ class utils::PositionVariables<dragon::Bgo::MAX_CHANNELS>;

#pragma link C++ class utils::AdcVariables<dragon::Dsssd::MAX_CHANNELS>+;
#pragma link C++ class utils::TdcVariables<1>+;

#pragma link C++ class utils::AdcVariables<dragon::IonChamber::MAX_CHANNELS>+;
#pragma link C++ class utils::TdcVariables<1>+;

#pragma link C++ class utils::AdcVariables<dragon::SurfaceBarrier::MAX_CHANNELS>+;

#pragma link C++ class utils::AdcVariables<dragon::NaI::MAX_CHANNELS>+;
#pragma link C++ class utils::AdcVariables<1>+;
#endif


#endif // #ifndef HAVE_DRAGON__HXX