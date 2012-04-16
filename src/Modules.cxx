/// \file Modules.cxx
/// \brief Implements Modules.hxx
#include <cassert>
#include <sstream>
#include "Modules.hxx"
#include "vme/vme.hxx"


// ====== Class dragon::gamma::Modules ======= //

dragon::gamma::Modules::Modules():
	v792(), v1190b(), io32()
{
	reset();
};

dragon::gamma::Modules::Modules(const Modules& other)
{
	v792   = other.v792;
	v1190b = other.v1190b;
	io32   = other.io32;
}

dragon::gamma::Modules& dragon::gamma::Modules::operator= (const dragon::gamma::Modules& other)
{
	v792   = other.v792;
	v1190b = other.v1190b;
	io32   = other.io32;
	return *this;
}

void dragon::gamma::Modules::reset()
{
	vme::reset(v792);
	vme::reset<vme::caen::V1190b, 64> (v1190b);
}

void dragon::gamma::Modules::unpack(const TMidasEvent& event)
{
	vme::caen::unpack_adc(event, "VADC", v792);
	vme::caen::unpack_adc(event, "ADC0", v792);
	vme::caen::unpack_v1190(event, "VTDC", v1190b);
	vme::caen::unpack_v1190(event, "TDC0", v1190b);
//	vme::unpack_io32(event, "VI032", io32);
}

int16_t dragon::gamma::Modules::v792_data(unsigned ch) const
{
#ifdef DEBUG
	assert(ch < 32);
#endif
	return v792.data[ch];
}

int16_t dragon::gamma::Modules::v1190b_data(unsigned ch) const
{
#ifdef DEBUG
	assert(ch < 64);
#endif
	return v1190b.data[ch];
}

int32_t dragon::gamma::Modules::tstamp() const
{
	return io32.tstamp;
}


// ====== Class dragon::hion::Modules ======= //

dragon::hion::Modules::Modules():
	v1190b(), io32() { };

dragon::hion::Modules::Modules(const Modules& other)
{
	for(int i=0; i< 2; ++i) {
		v785[i] = other.v785[i];
	}
	v1190b = other.v1190b;
	io32   = other.io32;
}

void dragon::hion::Modules::reset()
{
	for(int i=0; i< 2; ++i) {
		vme::reset(v785[i]);
	}
	vme::reset<vme::caen::V1190b, 64> (v1190b);
}

dragon::hion::Modules& dragon::hion::Modules::operator= (const Modules& other)
{
	for(int i=0; i< 2; ++i) {
		v785[i] = other.v785[i];
	}
	v1190b = other.v1190b;
	io32   = other.io32;
	return* this;
}

void dragon::hion::Modules::unpack(const TMidasEvent& event)
{
/*
	for(int i=0; i< 2; ++i) {
		std::stringstream bank; bank << "VADC" << i;
		vme::caen::unpack_adc(event, bank.str().c_str(), v785[i]);
	}
*/
//	vme::caen::unpack_v1190b(event, "VTDC", v1190b);
//	vme::unpack_io32(event, "VI032", io32);
}

int16_t dragon::hion::Modules::v785_data(unsigned which, unsigned ch) const
{
#ifdef DEBUG
	assert(which < 2);
	assert(ch < 32);
#endif
	return v785[which].data[ch];
}

int16_t dragon::hion::Modules::v1190b_data(unsigned ch) const
{
#ifdef DEBUG
	assert(ch < 64);
#endif
	return v1190b.data[ch];
}

int32_t dragon::hion::Modules::tstamp() const
{
	return io32.tstamp;
}
