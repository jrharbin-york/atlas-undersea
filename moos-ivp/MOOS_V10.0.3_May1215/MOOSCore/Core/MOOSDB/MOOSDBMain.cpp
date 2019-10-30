///////////////////////////////////////////////////////////////////////////
//
//   This file is part of the MOOS project
//
//   MOOS : Mission Oriented Operating Suite A suit of 
//   Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) Paul Newman
//    
//   This software was written by Paul Newman at MIT 2001-2002 and 
//   the University of Oxford 2003-2013 
//   
//   email: pnewman@robots.ox.ac.uk. 
//              
//   This source code and the accompanying materials
//   are made available under the terms of the GNU Public License
//   which accompanies this distribution, and is available at
//   http://www.gnu.org/licenses/gpl.txt
//   distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "MOOS/libMOOS/MOOSLib.h"

#include "MOOS/libMOOS/DB/MOOSDB.h"
#include "MOOS/libMOOS/DB/MOOSDB_MQ.h"
#include "MOOS/libMOOS/DB/MOOSDB_ActiveFaults.h"

#include <iostream>
#include <string>

int main(int argc , char * argv[])
{

  MOOS::CommandLineParser P(argc, argv);
  std::string mission_file = P.GetFreeParameter(0, "Mission.moos");
  
  // This is the main MOOS DB Object
  CMOOSDB * DB;
  DB = new CMOOSDB_ActiveFaults(61616, mission_file);
       
  DB->Run(argc,argv);
  
  //nothing to do - all the threads in the DB object
  //do the work
  while(DB->IsRunning()) {
    MOOSPause(1000);
  }
  
  delete DB;
  return 0;
}

