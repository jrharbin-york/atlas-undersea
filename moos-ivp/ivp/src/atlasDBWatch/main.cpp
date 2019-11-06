/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: main.cpp                                             */
/*    DATE: Jun 26th 2008                                        */
/*                                                               */
/* This file is part of MOOS-IvP                                 */
/*                                                               */
/* MOOS-IvP is free software: you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation, either version  */
/* 3 of the License, or (at your option) any later version.      */
/*                                                               */
/* MOOS-IvP is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty   */
/* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See  */
/* the GNU General Public License for more details.              */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with MOOS-IvP.  If not, see                     */
/* <http://www.gnu.org/licenses/>.                               */
/*****************************************************************/

#include "ATLASDBWatch_App.h"
#include "ColorParse.h"
#include "MBUtils.h"
#include "ReleaseInfo.h"

using namespace std;

int main(int argc, char *argv[]) {
  string mission_file = "ATLASDBWatch.moos";
  string run_command = argv[0];

  cout << termColor("green");
  cout << "ATLASDBWatch_App launching as " << run_command << endl;
  cout << termColor() << endl;

  ATLASDBWatch atlasapp;
  atlasapp.Run(run_command.c_str(), mission_file.c_str(), argc, argv);
  return 0;
}
