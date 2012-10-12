/// \file Events.cxx
/// \author G. Christian
/// \brief Implements Events.hxx
#include <memory>
#include <stdexcept>
#include <TFile.h>
#include <TString.h>
#include <TObjArray.h>
#include <TObjString.h>
#include "utils/definitions.h"
#include "midas/Event.hxx"
#include "Histos.hxx"
#include "Events.hxx"

#include "Globals.h"

rootana::EventHandler* rootana::EventHandler::Instance()
{
	static EventHandler* e = 0;
	if(!e) e = new EventHandler();
	return e;
}

void rootana::EventHandler::AddHisto(rootana::HistBase* hist, uint16_t eventId, TDirectory* file, const char* dirPath)
{
	if (!file) throw (std::invalid_argument("'file' is NULL"));
	if (file->IsZombie()) throw (std::invalid_argument("'file' is zombie"));
	file->cd();

	// Create directory in file
	TString sdp(dirPath);
	std::auto_ptr<TObjArray> dirs (sdp.Tokenize("/"));
	for (int i=0; i< dirs->GetEntries(); ++i) {
		TString dirname = static_cast<TObjString*>(dirs->At(i))->GetString();
		TDirectory* dir = dynamic_cast<TDirectory*>(gDirectory->FindObject(dirname.Data()));
		if(dir) dir->cd();
		else {
			TDirectory* d = gDirectory->mkdir(dirname.Data());
			d->cd();
		}
	}
	hist->set_directory(gDirectory);
	
	fHistos[eventId].push_back(hist);
	file->cd();
}

namespace {

struct hFill   { void operator() (rootana::HistBase* h) { h->fill();  } };
struct hWrite  { void operator() (rootana::HistBase* h) { h->write(); } };
struct hClear  { void operator() (rootana::HistBase* h) { h->clear(); } };
struct hDelete { void operator() (rootana::HistBase* h) { delete h;   } };
struct hDestruct { void operator() (rootana::HistBase* h) { h->write(); delete h; } };

struct hListWrite {
	void operator() (std::pair<const uint16_t, std::list<rootana::HistBase*> >& p)
		{ std::for_each (p.second.begin(), p.second.end(), hWrite()); }
};

struct hListClear {
	void operator() (std::pair<const uint16_t, std::list<rootana::HistBase*> >& p)
		{ std::for_each (p.second.begin(), p.second.end(), hClear()); }
};

struct hListDestruct {
	void operator() (std::pair<const uint16_t, std::list<rootana::HistBase*> >& p)
		{ std::for_each (p.second.begin(), p.second.end(), hDestruct()); }
};

template <class T, class E>
inline void handle_event(T& data, const E& buf)
{
	data.reset();
	data.unpack(buf);
	data.calculate();
} }

void rootana::EventHandler::Process(const midas::Event& event)
{
	const uint16_t EID = event.GetEventId();
	switch (EID) {

	case DRAGON_HEAD_EVENT:  /// - DRAGON_HEAD_EVENT: Calculate head params & fill head histos
		handle_event(rootana::gHead, event);
		std::for_each(fHistos[EID].begin(), fHistos[EID].end(), hFill());
		break;

	case DRAGON_TAIL_EVENT:  /// - DRAGON_TAIL_EVENT: Calculate tail params & fill tail histos
		handle_event(rootana::gTail, event);
		std::for_each(fHistos[EID].begin(), fHistos[EID].end(), hFill());
		break;

	case DRAGON_HEAD_SCALER: /// - DRAGON_HEAD_SCALER: \todo implement C. Stanford's scaler codes
		// <...process...> //
		break;

	case DRAGON_TAIL_SCALER: /// - DRAGON_TAIL_SCALER: \todo implement C. Stanford's scaler codes
		// <...process...> //
		break;

	default: /// - Silently ignore other event types
		break;
	}
}

void rootana::EventHandler::Process(const midas::CoincEvent& coincEvent)
{
	/// Calculate coinc params & fill histograms
	/// \todo Avoid extra unpacking step
	handle_event(rootana::gCoinc, coincEvent);
	std::for_each(fHistos[DRAGON_COINC_EVENT].begin(), fHistos[DRAGON_COINC_EVENT].end(), hFill());
}

void rootana::EventHandler::EndRun()
{
	std::for_each (fHistos.begin(), fHistos.end(), hListDestruct());
}

void rootana::EventHandler::BeginRun()
{
	// std::for_each (fHistos.begin(), fHistos.end(), hListClear());
}
