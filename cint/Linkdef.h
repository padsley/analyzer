//! \file UserLinkdef.h
//! \brief User-defined portion of the CINT Linkdef file.
//!
//! \full This file is where you can tell CINT about the classes you've written to store and analyze
//! your data.  At the least, you will need to do this for any class that stores parameters that you
//! wish to view or variables used in calculating those parameters.
//! Usually the easiest option is link everything in the header files which define your classes. \n
//! Example:
//! \code
//! #pragma link C++ defined_in /path/to/my/header.hxx;
//! \endcode
//! (note that you need to specify the fill path to the header file).
//!
//! If the above doesn't work, you might have to specify each class (or namespace) individually
//! (or figure out why not). \n
//! Example:
//! \code
//! #pragma link C++ namespace my;
//! #pragma link C++ class my::Class+;
//! // -- OR -- //
//! #pragma link C++ class MyClass+;
//! // -- ETC -- //
//! \endcode
//! (note that the '+' at the end is required except in special cases).
#ifdef __MAKECINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class dragon+;
#pragma link C++ class vme+;

#pragma link C++ defined_in ../src/utils/VariableStructs.hxx;
#pragma link C++ defined_in ../src/vme/V1190.hxx;
#pragma link C++ defined_in ../src/vme/Io32.hxx;

#pragma link C++ class dragon::Coinc+;
#pragma link C++ class dragon::Tail+;

#pragma link C++ class dragon::Mcp+;
#pragma link C++ class dragon::Mcp::Variables+;
#pragma link C++ class dragon::Dsssd+;
#pragma link C++ class dragon::Dsssd::Variables+;
#pragma link C++ class dragon::IonChamber+;
#pragma link C++ class dragon::IonChamber::Variables+;
#pragma link C++ class dragon::SurfaceBarrier+;
#pragma link C++ class dragon::SurfaceBarrier::Variables+;
#pragma link C++ class dragon::NaI+;
#pragma link C++ class dragon::NaI::Variables+;
#pragma link C++ class dragon::Ge+;
#pragma link C++ class dragon::Ge::Variables+;

#pragma link C++ class dragon::Head+;
#pragma link C++ class dragon::Bgo+;
#pragma link C++ class dragon::Bgo::Variables+;

#pragma link C++ class dragon::Tail::Tof+;
#pragma link C++ class dragon::Tail::Tof::Variables+;
#pragma link C++ class dragon::Head::Tof+;
#pragma link C++ class dragon::Head::Tof::Variables+;

#pragma link C++ defined_in ../src/dragon/Scaler.hxx;
#pragma link C++ defined_in ../src/utils/Banks.hxx;

class TTree;
#pragma link C++ function dragon::Scaler::set_aliases(TTree*, const char*);

#pragma link C++ class vme::V792+;
#pragma link C++ class vme::V785+;
#pragma link C++ class vme::V1190+;
#pragma link C++ class vme::Io32+;

#pragma link C++ class midas::Event::Header+;

#pragma link C++ class midas::Xml+;
#pragma link C++ class midas::Odb+;

#pragma link C++ class tstamp::Diagnostics+;
#pragma link C++ defined_in ../src/utils/TStamp.hxx;

/*
	for j in Adc Tdc
	do for i in {1..32}
	do echo "#pragma link C++ class dragon::${j}Variables<${i}>+;"
	done
	done
*/


#endif
