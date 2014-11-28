// $Id: SiTrackerBarrel_geo.cpp 1360 2014-10-27 16:32:06Z Nikiforos.Nikiforou@cern.ch $
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
#ifndef DD4HEP_DDREC_SURFACEINSTALLER_H
#define DD4HEP_DDREC_SURFACEINSTALLER_H 1

// Framework include files
#include "DD4hep/LCDD.h"
#include "DD4hep/DetectorTools.h"
#include "DD4hep/DetFactoryHelper.h"

// C/C++ include files
#include <map>
#include <algorithm>

/// Namespace for the tracking surfaces of the AIDA detector description toolkit
namespace DDSurfaces  {  
  /// Class describing the surface types
  class SurfaceType;
}

/// Namespace for the AIDA detector description toolkit
namespace DD4hep  {

  /// Namespace for the reconstruction part of the AIDA detector description toolkit
  namespace DDRec  {  
    /// Class describing surface data
    class SurfaceData;
  }

  /** Base class to implement surface installers for known detector patterns
   *
   *  The class scans the geometry of a subdetector and gives callbacks
   *  to user classes, which then should install in the proper callback
   *  routines the surface instances to the detector elements.
   *
   *  \author  M.Frank
   *  \version 1.0
   *  \ingroup DD4HEP
   */
  class SurfaceInstaller  {
  protected:
    typedef Geometry::LCDD         LCDD;
    typedef Geometry::DetElement   DetElement;
    typedef Geometry::Volume       Volume;
    typedef Geometry::PlacedVolume PlacedVolume;
    typedef Geometry::DetectorTools::ElementPath   ElementPath;
    typedef Geometry::DetectorTools::PlacementPath PlacementPath;
    typedef DDRec::SurfaceData      SurfaceData;
    typedef DDSurfaces::SurfaceType SurfaceType;

    typedef std::map<TGeoVolume*, SurfaceData* > Surfaces;
    /// Reference to the LCDD instance
    LCDD&         m_lcdd;
    /// Reference to the detector element of the subdetector
    DetElement    m_det;
    /// Map of surface instances keyed by the logical volume
    Surfaces      m_surfaces;

    /// Scan through tree of detector elements
    void scan(DetElement de);

  public:
    /// Initializing constructor
    SurfaceInstaller(LCDD& lcdd, const std::string& det_name);
    /// Default destructor
    virtual ~SurfaceInstaller() {}
    /// Indicate error message and throw exception
    void invalidInstaller(const std::string& msg) const;
    /// Scan through tree of detector elements
    void scan();
    /// Install volume information. Default implementation only prints!
    virtual void install(DetElement e, PlacedVolume pv);
    /// Executor
    template <typename T> static long run(LCDD& lcdd,int argc,char** argv);
  };

  /// Action routine to execute the test
  template <typename T> inline long SurfaceInstaller::run(Geometry::LCDD& lcdd,int argc,char** argv)
  {
    for(; argc>0; --argc)   {
      std::string name = argv[argc-1];
      T installer(lcdd,name);
      installer.scan();
    }
    return 1;
  }

#define DECLARE_SURFACE_INSTALLER(name,class)		\
  namespace DD4hep { \
    template long SurfaceInstaller::run< class >(Geometry::LCDD& lcdd,int argc,char** argv); \
  } \
  DECLARE_APPLY(name,SurfaceInstaller::run< class >)

}   // End namespace DD4hep


#ifdef DD4HEP_USE_SURFACEINSTALL_HELPER

#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"

/** If you want to save yourself some typing when creating surface installers, 
 *  set the compile macro DD4HEP_USE_SURFACEINSTALL_HELPER LOCALLY !
 *  This will then enable the code below and the only thing you will have to
 *  type is the installer member function to create a measurement surface for
 *  a given volume.
 *
 *  \author  M.Frank
 *  \version 1.0
 *  \ingroup DD4HEP
 */
namespace {

  /** Install class template to create surface installer plugins
   *
   *  See the base class SurfaceInstaller for further details.
   *
   *  \author  M.Frank
   *  \version 1.0
   *  \ingroup DD4HEP
   */
  class Installer : public DD4hep::SurfaceInstaller {
  public:
    typedef DD4hep::DDRec::Vector3D     Vector3D;
    typedef DD4hep::DDRec::VolSurface   VolSurface;
    typedef DD4hep::DDRec::VolPlane     VolPlane;
    typedef DDSurfaces::SurfaceType     Type;
  public:
    /// Initializing constructor
    Installer(LCDD& lcdd, const std::string& nam) : DD4hep::SurfaceInstaller(lcdd, nam) {}
    /// Default destructor
    virtual ~Installer() {}
    /// Install volume information. Default implementation only prints!
    virtual void install(DetElement component, PlacedVolume pv);
    /// Try to handle surface using the surface cache
    bool handleUsingCache(DetElement comp, Volume vol)  const  {
      Surfaces::const_iterator is = m_surfaces.find(vol.ptr());
      if ( is != m_surfaces.end() )  {
	VolSurface surf((*is).second);
	DD4hep::DDRec::volSurfaceList(comp)->push_back(surf);
	return true;
      }
      return false;
    }
    Volume parentVolume(DetElement component)  const  {
      DetElement module = component.parent();
      if ( module.isValid() )   {
	return module.placement().volume();
      }
      return Volume();
    }
    const double* placementTranslation(DetElement component)  const  {
      TGeoMatrix* mat = component.placement()->GetMatrix();
      const double* trans = mat->GetTranslation();
      return trans;
    }
    void addSurface(DetElement component, const DD4hep::DDRec::VolSurface& surf)   {
      m_surfaces.insert(std::make_pair(surf.volume().ptr(),surf.ptr()));
      DD4hep::DDRec::volSurfaceList(component)->push_back(surf);
    }
    template <typename T> bool checkShape(const T& shape) const   {
      if ( shape.isValid() ) return true;
      invalidInstaller("Shape is not of the required type:"+DD4hep::typeName(typeid(T)));
     return false;
    }
  };
}

DECLARE_SURFACE_INSTALLER(DD4HEP_USE_SURFACEINSTALL_HELPER,Installer)

#endif /* DD4HEP_USE_SURFACEINSTALL_HELPER */

#endif /* DD4HEP_DDREC_SURFACEINSTALLER_H */