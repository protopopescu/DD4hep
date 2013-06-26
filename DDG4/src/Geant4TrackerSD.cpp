// $Id$
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
// Framework include files
#include "DDG4/Geant4SensitiveDetector_inline.h"
#include "DDG4/Factories.h"
#include "DDG4/Geant4StepHandler.h"
#include "DDG4/Geant4Mapping.h"

#include "DD4hep/BitField64.h"

//#include "G4Step.hh"


/*
 *   DD4hep::Simulation namespace declaration
 */
namespace DD4hep {  namespace Simulation {

  /// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ///               Geant4GenericSD<Tracker>
  /// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  struct Tracker {};
  /// Method for generating hit(s) using the information of G4Step object.
    template <> bool Geant4GenericSD<Tracker>::buildHits(G4Step* step,G4TouchableHistory* /*hist*/ ) {
      
      StepHandler h(step);
      

      //------------ get the cellID -----------------------------------
      
      Geant4Mapping&    mapping = Geant4Mapping::instance();
      Geant4StepHandler stepH(step);
      const G4VPhysicalVolume* pv  = stepH.volume(stepH.pre);
      
      int cellID = 0 ;
      
      Geometry::PlacedVolume place = mapping.placement(pv);
      
      if ( place.isValid() )   {
	if ( place.volume().isSensitive() )  {
	  
	  //	  G4cout << "----------- Geant4GenericSD<Tracker>::buildHits - get volumeID from  " << place.toString()  << std::endl ;
	  
	  Geometry::VolumeManager vm = mapping.lcdd().volumeManager();
	  
	  Geometry::VolumeManager::VolumeID cell_id = vm.lookupID( place );
	  
	  // const Geometry::PlacedVolume::VolIDs& ids = place.volIDs();
	  // printf(" Found Sensitive TGeoNode:%s CellID: %llx #VolIDs: %d  --- at level: %d \n", place.name(), cell_id , ids.size() , i );
	  // for( Geometry::PlacedVolume::VolIDs::const_iterator it = ids.begin() ; it != ids.end() ; ++it ){
	  //   G4cout << "--- " << it->first << " -- " << it->second << std::endl ;
	  // }
	  
	  Geometry::Volume            vol    = place.volume();
	  Geometry::SensitiveDetector sd     = vol.sensitiveDetector();
	  Geometry::Readout           ro     = sd.readout();
	  Geometry::IDDescriptor      iddesc = ro.idSpec();
	  

	  //-----  use a BitField64 object to store the cellID in the Linear collider convention
	  BitField64 bf( iddesc.toString()  ) ; 

	  for( unsigned i=0, N=bf.size() ;i<N;++i){
	    
	    BitFieldValue& v = bf[i] ;
	    
	    long val =  iddesc.field( v.name() ).decode( cell_id )   ;
	    
	    if( v.isSigned() && ( val  & ( 1LL << ( v.width() - 1 ) ) ) != 0 ) { // negative value
		
	      val -= ( 1LL << v.width() );
	    }
	    v = val ;

	    // this does not work as the IDDescriptor does not handle signed (negative) values correctly
	    //	    b[i] = iddesc.field( b[i].name() ).decode( cell_id )  ; 
	  }
	  
	  // G4cout << "---  IDDescriptor: " << iddesc.toString() 
	  // 	 << " BitField64 : " <<  bf  
	  // 	 <<   std::endl ; 
	  
	  // Geometry::IDDescriptor::Field modFld = iddesc.field("module");
	  // int mod_id = modFld.decode( cell_id );
	  // int layer  = iddesc.field("layer").decode( cell_id )  ;	  
	  // G4cout << "---  layer " << layer << "  ---- module: " << mod_id << std::endl ;
	  
	  
	  cellID = bf.lowWord() ;
	  
	} else {
	  
	  G4cerr << "**ERROR** Geant4GenericSD<Tracker>::buildHits - called for insensitive volume: " 
 	 	 << place.toString()
 	 	 << std::endl ;
	  
	} 

      } else{

	G4cerr << "----------- Geant4GenericSD<Tracker>::buildHits -  invalid Volume found: " << std::endl ;
      }
      
      Position prePos    = h.prePos();
      Position postPos   = h.postPos();
      Position direction = postPos - prePos;
      Position position  = mean_direction(prePos,postPos);
      double   hit_len   = direction.R();
      if (hit_len > 0) {
	double new_len = mean_length(h.preMom(),h.postMom())/hit_len;
	direction *= new_len/hit_len;
      }
      
      //    G4cout << "----------- Geant4GenericSD<Tracker>::buildHits : position : " << prePos << G4endl ;
      
      Geant4TrackerHit* hit = 
	new Geant4TrackerHit(h.track->GetTrackID(),
			     h.track->GetDefinition()->GetPDGEncoding(),
			     step->GetTotalEnergyDeposit(),
			     h.track->GetGlobalTime());
      
      
      HitContribution contrib = Geant4Hit::extractContribution(step);
      
      // get the copy number of the parent volume -
      // we need a more generic way of finding the volume that has the right copy number assigned...
      // otherwise we couple detector construction sensitive detector !!
      //    G4StepPoint* preStepPoint = step->GetPreStepPoint();    
      //    G4TouchableHistory* theTouchable = (G4TouchableHistory*)( preStepPoint->GetTouchable() );
      // G4int copyNo = theTouchable->GetVolume()->GetCopyNo();
      // G4int motherCopyNo = theTouchable->GetVolume(1)->GetCopyNo();

      hit->cellID  = cellID ;


      hit->energyDeposit =  contrib.deposit ;

      hit->position = position;
      hit->momentum = direction;
      hit->length   = hit_len;
      collection(0)->insert(hit);
      return hit != 0;
    }
    typedef Geant4GenericSD<Tracker> Geant4Tracker;
  }}    // End namespace DD4hep::Simulation

DECLARE_GEANT4SENSITIVEDETECTOR(Geant4Tracker)
