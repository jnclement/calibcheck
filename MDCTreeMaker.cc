#include "MDCTreeMaker.h"
#include <ffaobjects/EventHeaderv1.h>
#include <calobase/RawCluster.h>
#include <calobase/RawClusterContainer.h>
#include <calobase/RawTower.h>
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerGeomContainer.h>
#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfoContainerv1.h>
#include <globalvertex/GlobalVertexMap.h>
#include <globalvertex/GlobalVertex.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4Particle.h>
#include <mbd/MbdPmtContainer.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phhepmc/PHHepMCGenEvent.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>
#include <mbd/MbdVertexMap.h>
#include <mbd/MbdVertex.h>
#include <phhepmc/PHHepMCGenEventMap.h>
#include <ffaobjects/EventHeaderv1.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <HepMC/GenEvent.h>
#include <mbd/MbdPmtHit.h>
#include <jetbackground/TowerBackgroundv1.h>
#include <cmath>
#include <cdbobjects/CDBTTree.h>
#include <TLorentzVector.h>
#include <ffamodules/CDBInterface.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>  // for gsl_rng_uniform_pos

#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>

#include <iostream>

#include <centrality/CentralityInfov1.h>

using namespace std;
//____________________________________________________________________________..
MDCTreeMaker::MDCTreeMaker(const std::string &name, const int dataormc, const int debug, const int correct):
  SubsysReco((name+"test").c_str())
{
  _correct = correct;
  _foutname = name;  
  _dataormc = dataormc;
  _debug = debug;
}

//____________________________________________________________________________..
MDCTreeMaker::~MDCTreeMaker()
{

}

//____________________________________________________________________________..
int MDCTreeMaker::Init(PHCompositeNode *topNode)
{


  _f = new TFile( _foutname.c_str(), "RECREATE");
  
  _tree = new TTree("ttree","a persevering date tree");
  
  _tree->Branch("etabin",etabin,"etabin[24576]/I");
  _tree->Branch("phibin",phibin,"phibin[24576]/I");
  _tree->Branch("calval",calval,"calval[24576]/F");

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MDCTreeMaker::InitRun(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
//____________________________________________________________________________..
int MDCTreeMaker::process_event(PHCompositeNode *topNode)
{
  std::string calibname = "cemc_pi0_twrSlope_v1";
  std::string fieldname = "Femc_datadriven_qm1_correction";
  std::string calibdir = CDBInterface::instance()->getUrl(calibname);

  if(calibdir.empty())
    {
      if(_debug) cout << "calibdir empty!" << endl;
      calibname = "cemc_pi0_twrSlope_v1_default";
      calibdir = CDBInterface::instance()->getUrl(calibname);
      if(calibdir.empty())
	{
	  if(_debug) cout << "still empty" << endl;
	  return Fun4AllReturnCodes::EVENT_OK;
	}
    }
  CDBTTree* cdbttree = new CDBTTree(calibdir);

  TowerInfoContainer* raw_towers = findNode::getClass<TowerInfoContainer>(topNode,"TOWERS_CEMC");
  
  if(!cdbttree)
    {
      if(_debug) cout << "no cdbttree!" << endl;
      return Fun4AllReturnCodes::EVENT_OK;
    }
  if(!raw_towers)
    {
      if(_debug) cout << "no raw towers!" << endl;
      return Fun4AllReturnCodes::EVENT_OK;
    }

  for(int i=0; i<24576; ++i)
    {
      unsigned int key = raw_towers->encode_key(i);
      TowerInfo* tower = raw_towers->get_tower_at_channel(i);
      float calibconst = cdbttree->GetFloatValue(key,fieldname);
      int eta = raw_towers->getTowerEtaBin(key);
      if(eta < 9 || eta > 94)
	{
	  cout << "Eta bin / calibration constant / raw tower energy: " << eta << " / " << calibconst << " / " << tower->get_energy() << endl;
	}
      int phi = raw_towers->getTowerPhiBin(key);
      calval[i] = calibconst;
      etabin[i] = eta;
      phibin[i] = phi;
    }

  _tree->Fill();

  return Fun4AllReturnCodes::EVENT_OK;
  
}
//____________________________________________________________________________..
int MDCTreeMaker::ResetEvent(PHCompositeNode *topNode)
{
  if (Verbosity() > 0)
    {
      std::cout << "MDCTreeMaker::ResetEvent(PHCompositeNode *topNode) Resetting internal structures, prepare for next event" << std::endl;
    }
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MDCTreeMaker::EndRun(const int runnumber)
{
  if (Verbosity() > 0)
    {
      std::cout << "MDCTreeMaker::EndRun(const int runnumber) Ending Run for Run " << runnumber << std::endl;
    }
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MDCTreeMaker::End(PHCompositeNode *topNode)
{
  if (Verbosity() > 0)
    {
      std::cout << "MDCTreeMaker::End(PHCompositeNode *topNode) This is the End..." << std::endl;
    }

  _f->Write();
  _f->Close();

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int MDCTreeMaker::Reset(PHCompositeNode *topNode)
{
  if (Verbosity() > 0)
    {
      std::cout << "MDCTreeMaker::Reset(PHCompositeNode *topNode) being Reset" << std::endl;
    }
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
void MDCTreeMaker::Print(const std::string &what) const
{
  std::cout << "MDCTreeMaker::Print(const std::string &what) const Printing info for " << what << std::endl;
}
