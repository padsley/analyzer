/// \file Tail.hxx
/// \brief Defines the DRAGON tail (heavy ion) detector classes
#ifndef DRAGON_TAIL_HXX
#define DRAGON_TAIL_HXX
#include "midas/Event.hxx"
#include "vme/IO32.hxx"
#include "vme/V792.hxx"
#include "vme/V1190.hxx"
#include "MCP.hxx"
#include "DSSSD.hxx"
#include "Auxillary.hxx"
#include "IonChamber.hxx"
#include "SurfaceBarrier.hxx"

namespace dragon {

/// Collection of all tail detectors and VME modules
class Tail {

public:
	/// Number of ADC (caen v785) modules
	static const int NUM_ADC = 2;

public:
	/// Heavy ion variables
	struct Variables {
		/// Channel of the V1190b TDC trigger
		int v1190_trigger_ch;
			
		/// Constructor, set data to generic values
		Variables();

		/// Destructor, nothing to do
		~Variables() { }

		/// Copy constructor
		Variables(const Variables& other);

		/// Equivalency operator
		Variables& operator= (const Variables& other);

		/// \brief Set variable values from an ODB file
		/// \param [in] odb_file Path of the odb file from which you are extracting variable values
		/// \todo Needs to be implemented once ODB is set up
		void set(const char* odb_file);
	};

public:
	/// Electronics modules
#ifdef DISPLAY_MODULES
	/// IO32 FPGA
	vme::IO32 io32; //#

	/// CAEN v785 ADC (x2)
	vme::caen::V785 v785[NUM_ADC]; //#

	/// CAEN v1190 TDC
	vme::caen::V1190 v1190; //#
#else
	/// IO32 FPGA
	vme::IO32 io32; //!

	/// CAEN v785 ADC (x2)
	vme::caen::V785 v785[NUM_ADC]; //!

	/// CAEN v1190 TDC
	vme::caen::V1190 v1190; //!
#endif

	/// Midas event header
	midas::Event::Header header; //#

	/// Tail::Variables instance
	Variables variables; //!

#ifndef DRAGON_OMIT_DSSSD
	/// DSSSD detector
	DSSSD dsssd; //#
#endif

#ifndef DRAGON_OMIT_IC
	/// Ionization chamber
	IonChamber ic; //#
#endif

	/// MCPs
	MCP mcp; //#

	/// Surface barrier detectors
	SurfaceBarrier sb; //#

#ifndef DRAGON_OMIT_NAI
	/// NaI detetors
	NaI nai; //#
#endif

#ifndef DRAGON_OMIT_GE
	/// Germanium detector
	Ge ge; //#
#endif

public:
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
};

} // namespace dragon



#endif
