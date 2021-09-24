/*
  val3dity 

  Copyright (c) 2011-2021, 3D geoinformation research group, TU Delft

  This file is part of val3dity.

  val3dity is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  val3dity is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with val3dity.  If not, see <http://www.gnu.org/licenses/>.

  For any information or further details about the use of val3dity, contact
  Hugo Ledoux
  <h.ledoux@tudelft.nl>
  Faculty of the Built Environment & Architecture
  Delft University of Technology
  Julianalaan 134, Delft 2628BL, the Netherlands
*/


#include "nlohmann-json/json.hpp"

using json = nlohmann::json;

namespace val3dity
{

bool validate_one_geom(json& j,
                       double tol_snap=0.001, 
                       double planarity_d2p_tol=0.01, 
                       double planarity_n_tol=20.0, 
                       double overlap_tol=-1.0);
  
//-- with report
bool validate_one_geom(json& j,
                       json& jreport,
                       double tol_snap=0.001, 
                       double planarity_d2p_tol=0.01, 
                       double planarity_n_tol=20.0, 
                       double overlap_tol=-1.0);

///////////////////////////////////////////////////////////

std::vector<bool> validate_tu3djson(json& j,
                    double tol_snap=0.001, 
                    double planarity_d2p_tol=0.01, 
                    double planarity_n_tol=20.0, 
                    double overlap_tol=-1.0);
//-- with report
std::vector<bool> validate_tu3djson(json& j,
                    json& jreport,
                    double tol_snap=0.001, 
                    double planarity_d2p_tol=0.01, 
                    double planarity_n_tol=20.0, 
                    double overlap_tol=-1.0);

///////////////////////////////////////////////////////////

bool validate_cityjson(json& j, 
                       double tol_snap=0.001, 
                       double planarity_d2p_tol=0.01, 
                       double planarity_n_tol=20.0, 
                       double overlap_tol=-1.0);


//-- with report
bool validate_cityjson(json& j, 
                       json& jreport, 
                       double tol_snap=0.001, 
                       double planarity_d2p_tol=0.01, 
                       double planarity_n_tol=20.0, 
                       double overlap_tol=-1.0);



//////////////////////////////////////////////////////////
bool validate_arrays(std::string geom_type,
                     std::vector<std::vector<std::vector<int>>> boundaries,
                     std::vector<std::vector<double>> vertices,
                     double tol_snap=0.001,
                     double planarity_d2p_tol=0.01,
                     double planarity_n_tol=20.0,
                     double overlap_tol=-1.0);
}
