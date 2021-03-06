//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//  \author Markus Frank
//  \date   2015-11-03
//
//==========================================================================
#ifndef DD4HEP_DDG4_PYDDG4_H
#define DD4HEP_DDG4_PYDDG4_H

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {
  /// Namespace for the Geant4 based simulation part of the AIDA detector description toolkit
  namespace sim {
    // Forward declarations
    class Geant4Kernel;
  }    // End namespace sim
}      // End namespace dd4hep


/// Python interface class for Geant4 python involation
/**
 *  \author  M.Frank
 *  \version 1.0
 *  \ingroup DD4HEP_SIMULATION
 */
struct PyDDG4  {
  typedef dd4hep::sim::Geant4Kernel Kernel;

  static int execute();
  static int process(const char* fname);
  static int run(Kernel& kernel);
  static int run(const char* fname);
};
#endif // DD4HEP_DDG4_PYDDG4_H
