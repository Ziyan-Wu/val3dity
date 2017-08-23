/*
 val3dity - Copyright (c) 2011-2017, Hugo Ledoux.  All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the authors nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL HUGO LEDOUX BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*/

#include "input.h"

using namespace std;
using json = nlohmann::json;

namespace val3dity
{

//-- XML namespaces map
std::map<std::string, std::string> NS; 

bool IOErrors::has_errors()
{
  if (_errors.size() == 0)
    return false;
  else
    return true;
}


void IOErrors::add_error(int code, std::string info)
{
  _errors[code].push_back(info);
  std::cout << "ERROR " << code << " : " << info << std::endl;
}


std::string IOErrors::get_report_text()
{
  std::stringstream ss;
  for (auto& err : _errors)
  {
    for (auto i : err.second)
    {
      ss << err.first << " -- " << errorcode2description(err.first) << std::endl;
      ss << "\tInfo: " << i << std::endl;
    }
  }
  return ss.str();
}


std::string IOErrors::get_report_xml()
{
  std::stringstream ss;
  for (auto& err : _errors)
  {
    for (auto i : err.second)
    {
      ss << "\t<Error>" << std::endl;
      ss << "\t\t<code>" << err.first << "</code>" << std::endl;
      ss << "\t\t<type>" << errorcode2description(err.first) << "</type>" << std::endl;
      ss << "\t\t<info>" << i << "</info>" << std::endl;
      ss << "\t</Error>" << std::endl;
    }
  }
  return ss.str();
}


std::string errorcode2description(int code) {
  switch(code)
  {
    case 0:   return string("STATUS_OK"); break;
    //-- RING
    case 101: return string("TOO_FEW_POINTS"); break;
    case 102: return string("CONSECUTIVE_POINTS_SAME"); break;
    case 103: return string("RING_NOT_CLOSED"); break;
    case 104: return string("RING_SELF_INTERSECTION"); break;
    case 105: return string("RING_COLLAPSED"); break;
    //-- POLYGON
    case 201: return string("INTERSECTION_RINGS"); break;
    case 202: return string("DUPLICATED_RINGS"); break;
    case 203: return string("NON_PLANAR_POLYGON_DISTANCE_PLANE"); break;
    case 204: return string("NON_PLANAR_POLYGON_NORMALS_DEVIATION"); break;
    case 205: return string("POLYGON_INTERIOR_DISCONNECTED"); break;
    case 206: return string("INNER_RING_OUTSIDE"); break;
    case 207: return string("INNER_RINGS_NESTED"); break;
    case 208: return string("ORIENTATION_RINGS_SAME"); break;
    //-- SHELL
    case 300: return string("NOT_VALID_2_MANIFOLD"); break;
    case 301: return string("TOO_FEW_POLYGONS"); break;
    case 302: return string("SHELL_NOT_CLOSED"); break;
    case 303: return string("NON_MANIFOLD_VERTEX"); break;
    case 304: return string("NON_MANIFOLD_EDGE"); break;
    case 305: return string("SEPARATE_PARTS"); break;
    case 306: return string("SHELL_SELF_INTERSECTION"); break;
    case 307: return string("POLYGON_WRONG_ORIENTATION"); break;
    case 309: return string("VERTICES_NOT_USED"); break;
    //-- SOLID & MULTISOLID
    case 401: return string("INTERSECTION_SHELLS"); break;
    case 402: return string("DUPLICATED_SHELLS"); break;
    case 403: return string("INNER_SHELL_OUTSIDE"); break;
    case 404: return string("INTERIOR_DISCONNECTED"); break;
    case 405: return string("WRONG_ORIENTATION_SHELL"); break;
    //-- COMPOSITESOLID
    case 501: return string("INTERSECTION_SOLIDS"); break;
    case 502: return string("DUPLICATED_SOLIDS"); break;
    case 503: return string("DISCONNECTED_SOLIDS"); break;
    //-- BUILDING
    case 601: return string("BUILDINGPARTS_OVERLAP"); break;
    //-- OTHERS
    case 901: return string("INVALID_INPUT_FILE"); break;
    case 902: return string("EMPTY_PRIMITIVE"); break;
    case 903: return string("WRONG_INPUT_PARAMETERS"); break;
    case 999: return string("UNKNOWN_ERROR"); break;
    default:  return string("UNKNOWN_ERROR"); break;
  }
}

//-- ignore XML namespace
std::string localise(std::string s)
{
  return "*[local-name(.) = '" + s + "']";
}

  
std::string remove_xml_namespace(const char* input)
{
  std::string s = input;
  return s.substr(s.find_first_of(":") + 1);
}


vector<int> process_gml_ring(const pugi::xml_node& n, Surface* sh, IOErrors& errs) {
  std::string s = "./" + NS["gml"] + "LinearRing" + "/" + NS["gml"] + "pos";
  pugi::xpath_node_set npos = n.select_nodes(s.c_str());
  std::vector<int> r;
  if (npos.size() > 0) //-- <gml:pos> used
  {
    for (pugi::xpath_node_set::const_iterator it = npos.begin(); it != npos.end(); ++it) {
      std::string buf;
      std::stringstream ss(it->node().child_value());
      std::vector<std::string> tokens;
      while (ss >> buf)
        tokens.push_back(buf);
      Point3 p(std::stod(tokens[0]), std::stod(tokens[1]), std::stod(tokens[2]));
      r.push_back(sh->add_point(p));
    }
  }
  else //-- <gml:posList> used
  {
    std::string s = "./" + NS["gml"] + "LinearRing" + "/" + NS["gml"] + "posList";
    pugi::xpath_node pl = n.select_node(s.c_str());
    if (pl == NULL)
    {
      throw 901;
    }
    std::string buf;
    std::stringstream ss(pl.node().child_value());
    std::vector<std::string> coords;
    while (ss >> buf)
      coords.push_back(buf);
    if (coords.size() % 3 != 0)
    {
      errs.add_error(901, "Error: <gml:posList> has bad coordinates.");
      return r;
    }
    for (int i = 0; i < coords.size(); i += 3)
    {
      Point3 p(std::stod(coords[i]), std::stod(coords[i+1]), std::stod(coords[i+2]));
      r.push_back(sh->add_point(p));
    }
  }
  return r;
}


Surface* process_gml_surface(const pugi::xml_node& n, int id, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs) 
{
  std::string s = ".//" + NS["gml"] + "surfaceMember";
  pugi::xpath_node_set nsm = n.select_nodes(s.c_str());
  Surface* sh = new Surface(id, tol_snap);
  int i = 0;
  for (pugi::xpath_node_set::const_iterator it = nsm.begin(); it != nsm.end(); ++it)
  {
    std::vector< std::vector<int> > oneface;
    bool bxlink = false;
    pugi::xml_node tmpnode = it->node();
    pugi::xpath_node p;
    bool fliporientation = false;
    for (pugi::xml_attribute attr = tmpnode.first_attribute(); attr; attr = attr.next_attribute())
    {
      if (strcmp(attr.value(), "xlink:href") != 0) {
        bxlink = true;
        break;
      }
    }
    if (bxlink == true) 
    {
      std::string k = it->node().attribute("xlink:href").value();
      if (k[0] == '#')
        k = k.substr(1);
      p = dallpoly[k];
    }
    else
    {
      for (pugi::xml_node child : it->node().children()) 
      {
        if (std::string(child.name()).find("Polygon") != std::string::npos) {
          p = child;
          break;
        }
        else if (std::string(child.name()).find("OrientableSurface") != std::string::npos) {
          if (std::strncmp(child.attribute("orientation").value(), "-", 1) == 0)
            fliporientation = true;
          for (pugi::xml_node child2 : child.children()) 
          {
            if (std::string(child2.name()).find("baseSurface") != std::string::npos) 
            {
              std::string k = child2.attribute("xlink:href").value();
              if (k != "")
              {
                if (k[0] == '#')
                  k = k.substr(1);
                p = dallpoly[k];
                break;
              }
              for (pugi::xml_node child3 : child2.children())
              {
                if (std::string(child3.name()).find("Polygon") != std::string::npos)
                {
                  p = child;
                  break;
                }
              }
            }
          }
          break;
        }
        else if (std::string(child.name()).find("CompositeSurface") != std::string::npos) 
          break;
        else {
          throw 901;
        }
      }
    }

    //-- this is to handle CompositeSurfaces part of MultiSurfaces
    if (p == NULL) 
      continue;
    
    if (std::strncmp(p.node().attribute("orientation").value(), "-", 1) == 0)
      fliporientation = true;
    //-- exterior ring (only 1)
    s = ".//" + NS["gml"] + "exterior";
    pugi::xpath_node ring = p.node().select_node(s.c_str());
    std::vector<int> r = process_gml_ring(ring.node(), sh, errs);
    if (fliporientation == true) 
      std::reverse(r.begin(), r.end());
    if (r.front() != r.back())
    {
      if (p.node().attribute("gml:id") == 0)
        sh->add_error(103, std::to_string(i));
      else
        sh->add_error(103, p.node().attribute("gml:id").value());
    }
    else
      r.pop_back(); 
    oneface.push_back(r);
    //-- interior rings
    s = ".//" + NS["gml"] + "interior";
    pugi::xpath_node_set nint = it->node().select_nodes(s.c_str());
    for (pugi::xpath_node_set::const_iterator it = nint.begin(); it != nint.end(); ++it) {
      std::vector<int> r = process_gml_ring(it->node(), sh, errs);
      if (fliporientation == true) 
        std::reverse(r.begin(), r.end());
      if (r.front() != r.back())
      {
        if (p.node().attribute("gml:id") == 0)
          sh->add_error(103, std::to_string(i));
        else
          sh->add_error(103, p.node().attribute("gml:id").value());
      }
      else
        r.pop_back(); 
      oneface.push_back(r);
    }
    sh->add_face(oneface, p.node().attribute("gml:id").value());
    i++;
  }
  return sh;
}


Solid* process_gml_solid(const pugi::xml_node& nsolid, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
{
  //-- exterior shell
  Solid* sol = new Solid;
  if (nsolid.attribute("gml:id") != 0)
    sol->set_id(std::string(nsolid.attribute("gml:id").value()));
  std::string s = "./" + NS["gml"] + "exterior";
  pugi::xpath_node next = nsolid.select_node(s.c_str());
  sol->set_oshell(process_gml_surface(next.node(), 0, dallpoly, tol_snap, errs));
  //-- interior shells
  s = "./" + NS["gml"] + "interior";
  pugi::xpath_node_set nint = nsolid.select_nodes(s.c_str());
  int id = 1;
  for (pugi::xpath_node_set::const_iterator it = nint.begin(); it != nint.end(); ++it)
  {
    sol->add_ishell(process_gml_surface(it->node(), id, dallpoly, tol_snap, errs));
    id++;
  }
  return sol;
}


MultiSolid* process_gml_multisolid(const pugi::xml_node& nms, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
{
  MultiSolid* ms = new MultiSolid;
  if (nms.attribute("gml:id") != 0)
    ms->set_id(std::string(nms.attribute("gml:id").value()));
  std::string s = ".//" + NS["gml"] + "Solid";
  pugi::xpath_node_set nn = nms.select_nodes(s.c_str());
  for (pugi::xpath_node_set::const_iterator it = nn.begin(); it != nn.end(); ++it)
  {
    Solid* s = process_gml_solid(it->node(), dallpoly, tol_snap, errs);
    if (s->get_id() == "")
      s->set_id(std::to_string(ms->number_of_solids()));
    ms->add_solid(s);
  }
  return ms;
}


CompositeSolid* process_gml_compositesolid(const pugi::xml_node& nms, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
{
  CompositeSolid* cs = new CompositeSolid;
  if (nms.attribute("gml:id") != 0)
    cs->set_id(std::string(nms.attribute("gml:id").value()));
  std::string s = ".//" + NS["gml"] + "Solid";
  pugi::xpath_node_set nn = nms.select_nodes(s.c_str());
  for (pugi::xpath_node_set::const_iterator it = nn.begin(); it != nn.end(); ++it)
  {
    Solid* s = process_gml_solid(it->node(), dallpoly, tol_snap, errs);
    if (s->get_id() == "")
      s->set_id(std::to_string(cs->number_of_solids()));
    cs->add_solid(s);
  }

  return cs;
}



MultiSurface* process_gml_multisurface(const pugi::xml_node& nms, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
{
  MultiSurface* ms = new MultiSurface;
  if (nms.attribute("gml:id") != 0)
    ms->set_id(std::string(nms.attribute("gml:id").value()));
  Surface* s = process_gml_surface(nms, 0, dallpoly, tol_snap, errs);
  ms->set_surface(s);
  return ms;
}

CompositeSurface* process_gml_compositesurface(const pugi::xml_node& nms, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
{
  CompositeSurface* cs = new CompositeSurface;
  if (nms.attribute("gml:id") != 0)
    cs->set_id(std::string(nms.attribute("gml:id").value()));
  Surface* s = process_gml_surface(nms, 0, dallpoly, tol_snap, errs);
  cs->set_surface(s);
  return cs;
}


// Solid* process_gml_solid(pugi::xpath_node& nsolid, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
// {
//   //-- exterior shell
//   Solid* sol = new Solid;
//   if (nsolid.node().attribute("gml:id") != 0)
//     sol->set_id(std::string(nsolid.node().attribute("gml:id").value()));
//   if (prim == SOLID) 
//   {
//     std::string s = "./" + localise("exterior");
//     pugi::xpath_node next = nsolid.node().select_node(s.c_str());
//     sol->set_oshell(process_gml_compositesurface(next.node(), 0, dallpoly, tol_snap, errs));
//     //-- interior shells
//     s = "./" + localise("interior");
//     pugi::xpath_node_set nint = nsolid.node().select_nodes(s.c_str());
//     int id = 1;
//     for (pugi::xpath_node_set::const_iterator it = nint.begin(); it != nint.end(); ++it)
//     {
//       sol->add_ishell(process_gml_compositesurface(it->node(), id, dallpoly, tol_snap, errs));
//       id++;
//     }
//   }
//   else //-- both for CS and MS it's the same parsing 
//   {
//     sol->set_oshell(process_gml_compositesurface(nsolid.node(), 0, dallpoly, tol_snap, errs));
//   }
//   return sol;
// }


// void process_gml_building(vector<Primitive*>& lsSolids, pugi::xpath_node nbuilding, Primitive3D prim, std::map<std::string, pugi::xpath_node>& dallpoly, double tol_snap, IOErrors& errs)
// {
//   std::string id_building;
//   std::string id_buildingpart;
//   if (nbuilding.node().attribute("gml:id") != 0)
//     id_building = std::string(nbuilding.node().attribute("gml:id").value());
//   else
//     id_building = "";
//   std::string s1 = ".//" + localise("BuildingPart");
//   std::string s2;
//   if (prim == SOLID)
//     s2 = ".//" + localise("Solid");
//   else if (prim == MULTISURFACE)
//     s2 = ".//" + localise("MultiSurface");
//   else
//     return;
//   pugi::xpath_node_set nbps = nbuilding.node().select_nodes(s1.c_str());
//   if (nbps.empty() == false)
//   {
//     for (auto& nbp : nbps)
//     {
//       if (nbp.node().attribute("gml:id") != 0)
//         id_buildingpart = std::string(nbp.node().attribute("gml:id").value());
//       else
//         id_buildingpart = "";
//       pugi::xpath_node_set nsolids = nbp.node().select_nodes(s2.c_str());
//       for (auto& nsolid : nsolids)
//       {
//         Solid* sol = process_gml_solid(nsolid, prim, dallpoly, tol_snap, errs);
// //        sol->set_id_building(id_building);
// //        sol->set_id_buildingpart(id_buildingpart);
//         lsSolids.push_back(sol);
//       }
//     }
//   }
//   else
//   {
//     pugi::xpath_node_set nsolids = nbuilding.node().select_nodes(s2.c_str());
//     for (auto& nsolid : nsolids)
//     {
//       Solid* sol = process_gml_solid(nsolid, prim, dallpoly, tol_snap, errs);
// //      sol->set_id_building(id_building);
//       lsSolids.push_back(sol);
//     }
//   }
// }

void print_information(std::string &ifile)
{
  std::cout << "Reading file: " << ifile << std::endl;
  pugi::xml_document doc;
  if (!doc.load_file(ifile.c_str())) 
  {
    std::cout << "Input file not found and/or incorrect GML file." << std::endl;
    return;
  }
  //-- parse namespace
  std::map<std::string, std::string> ns;
  pugi::xml_node ncm = doc.first_child();
  std::string vcitygml;
  get_namespaces(ncm, ns, vcitygml);
  if (vcitygml.empty() == true) {
    std::cout << "File does not have the CityGML namespace. Abort." << std::endl;
    return;
  }
  std::cout << "++++++++++++++++++++ GENERAL +++++++++++++++++++++" << std::endl;
  std::cout << "CityGML version: " << vcitygml << std::endl;
  report_primitives(doc, ns);
  report_building(doc, ns);
}

void report_building(pugi::xml_document& doc, std::map<std::string, std::string>& ns) {
  std::cout << "++++++++++++++++++++ BUILDINGS +++++++++++++++++++" << std::endl;
  
  std::string s = "//" + ns["building"] + "Building";
  int nobuildings = doc.select_nodes(s.c_str()).size();
  print_info_aligned("Building", nobuildings);

  s = "//" + ns["building"] + "Building" + "/" + ns["building"] + "consistsOfBuildingPart" + "[1]";
  int nobwbp = doc.select_nodes(s.c_str()).size();
  print_info_aligned("without BuildingPart", (nobuildings - nobwbp), true);
  print_info_aligned("having BuildingPart", nobwbp, true);
  s = "//" + ns["building"] + "Building" + "[@" + ns["gml"] + "id]";
  print_info_aligned("with gml:id", doc.select_nodes(s.c_str()).size(), true);

  s = "//" + ns["building"] + "BuildingPart";
  int nobuildingparts = doc.select_nodes(s.c_str()).size();
  print_info_aligned("BuildingPart", nobuildingparts);
  s = "//" + ns["building"] + "BuildingPart" + "[@" + ns["gml"] + "id]";
  print_info_aligned("with gml:id", doc.select_nodes(s.c_str()).size(), true);
  for (int lod = 1; lod <= 3; lod++) {
    std::cout << "LOD" << lod << std::endl;
    int totals = 0;
    int totalms = 0;
    int totalsem = 0;
    report_building_each_lod(doc, ns, lod, totals, totalms, totalsem);
    print_info_aligned("Building stored in gml:Solid", totals, true);
    print_info_aligned("Building stored in gml:MultiSurface", totalms, true);
    print_info_aligned("Building with semantics for surfaces", totalsem, true);
  }
  std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
}

void print_info_aligned(std::string o, size_t number, bool tab) {
  if (tab == false)
    std::cout << std::setw(40) << std::left  << o;
  else
    std::cout << "    " << std::setw(36) << std::left  << o;
  std::cout << std::setw(10) << std::right << number << std::endl;
}

void report_building_each_lod(pugi::xml_document& doc, std::map<std::string, std::string>& ns, int lod, int& total_solid, int& total_ms, int& total_sem) {
  total_solid = 0;
  total_ms = 0;
  total_sem = 0;
  std::string slod = "lod" + std::to_string(lod);
  std::string s = "//" + ns["building"] + "Building";
  pugi::xpath_node_set nb = doc.select_nodes(s.c_str());
  for (auto& b : nb) {
    std::string s1 = ".//" + ns["building"] + slod + "Solid";
    pugi::xpath_node_set tmp = b.node().select_nodes(s1.c_str());
    if (tmp.empty() == false) {
      for (auto& nbp : tmp) {
        total_solid++;
        break;
      }
    }
    s1 = ".//" + ns["building"] + slod + "MultiSurface";
    tmp = b.node().select_nodes(s1.c_str());
    if (tmp.empty() == false) {
      for (auto& nbp : tmp) {
        total_ms++;
        break;
      }
    }
    s1 = ".//" + ns["building"] + "boundedBy" + "//" + ns["building"] + slod + "MultiSurface";
    tmp = b.node().select_nodes(s1.c_str());
    if (tmp.empty() == false) {
      for (auto& nbp : tmp) {
        total_sem++;
        break;
      }
    }
  }
}


void get_namespaces(pugi::xml_node& root, std::map<std::string, std::string>& ns, std::string& vcitygml) {
  vcitygml = "";
  for (pugi::xml_attribute attr = root.first_attribute(); attr; attr = attr.next_attribute()) {
    std::string name = attr.name();
    if (name.find("xmlns") != std::string::npos) {
      std::string value = attr.value();
      std::string sns;
      if (value.find("http://www.opengis.net/citygml/0") != std::string::npos) {
        sns = "citygml";
        vcitygml = "v0.4";
      }
      else if (value.find("http://www.opengis.net/citygml/1") != std::string::npos) {
        sns = "citygml";
        vcitygml = "v1.0";
      }
      else if (value.find("http://www.opengis.net/citygml/2") != std::string::npos) {
        sns = "citygml";
        vcitygml = "v2.0";
      }
      else if (value.find("http://www.opengis.net/gml") != std::string::npos)
        sns = "gml";
      else if (value.find("http://www.opengis.net/citygml/building") != std::string::npos)
        sns = "building";
      else if (value.find("http://www.w3.org/1999/xlink") != std::string::npos)
        sns = "xlink";
      else
        sns = "";
      if (sns != "") {
        size_t pos = name.find(":");
        if (pos == std::string::npos) 
          ns[sns] = "";
        else 
          ns[sns] = name.substr(pos + 1) + ":";
      }    
    }
  }
}


void report_primitives(pugi::xml_document& doc, std::map<std::string, std::string>& ns) {
  std::cout << "+++++++++++++++++++ PRIMITIVES +++++++++++++++++++" << std::endl;
  
  std::string s = "//" + ns["gml"] + "Solid";
  print_info_aligned("gml:Solid", doc.select_nodes(s.c_str()).size());

  s = "//" + ns["gml"] + "MultiSolid";
  print_info_aligned("gml:MultiSolid", doc.select_nodes(s.c_str()).size());

  s = "//" + ns["gml"] + "CompositeSolid";
  print_info_aligned("gml:CompositeSolid", doc.select_nodes(s.c_str()).size());
  
  s = "//" + ns["gml"] + "MultiSurface";
  print_info_aligned("gml:MultiSurface", doc.select_nodes(s.c_str()).size());
  
  s = "//" + ns["gml"] + "CompositeSurface";
  print_info_aligned("gml:CompositeSurface", doc.select_nodes(s.c_str()).size());

  s = "//" + ns["gml"] + "Polygon";
  print_info_aligned("gml:Polygon", doc.select_nodes(s.c_str()).size());

  std::cout << std::endl;
}


void process_json_surface(std::vector< std::vector<int> >& pgn, json& j, Surface* sh)
{
  std::vector< std::vector<int> > pgnids;
  for (auto& r : pgn)
  {
    std::vector<int> newr;
    for (auto& i : r)
    {
      double x;
      double y;
      double z;
      if (j.count("transform") == 0)
      {
        x = double(j["vertices"][i][0]);
        y = double(j["vertices"][i][1]);
        z = double(j["vertices"][i][2]);
      }
      else
      {
        x = (double(j["vertices"][i][0]) * double(j["transform"]["scale"][0])) + double(j["transform"]["translate"][0]);
        y = (double(j["vertices"][i][1]) * double(j["transform"]["scale"][1])) + double(j["transform"]["translate"][1]);
        z = (double(j["vertices"][i][2]) * double(j["transform"]["scale"][2])) + double(j["transform"]["translate"][2]);
      }
      Point3 p3(x, y, z);
      newr.push_back(sh->add_point(p3));
    }
    pgnids.push_back(newr);
  }
  sh->add_face(pgnids);
}


void read_file_cityjson(std::string &ifile, std::map<std::string, std::vector<Primitive*> >& dPrimitives, IOErrors& errs, double tol_snap)
{
  std::ifstream input(ifile);
  json j;
  try 
  {
    input >> j;
    // TODO: other validation for CityJSON and just not JSON stuff?
  }
  catch (nlohmann::detail::parse_error e) 
  {
    errs.add_error(901, "Input file not a valid JSON file.");
    return;
  }
  std::cout << "CityJSON input file" << std::endl;
  std::cout << "# City Objects found: " << j["CityObjects"].size() << std::endl;
  for (json::iterator it = j["CityObjects"].begin(); it != j["CityObjects"].end(); ++it) 
  {
    // std::cout << "o " << it.key() << std::endl;
    std::string coid = it.value()["type"];
    coid += "|";
    coid += it.key();
    int idgeom = 0;
    for (auto& g : it.value()["geometry"]) {
      std::string theid = it.key() + "(" + std::to_string(idgeom) + ")";
      if  (g["type"] == "Solid")
      {
        Solid* s = new Solid(theid);
        bool oshell = true;
        for (auto& shell : g["boundaries"]) 
        {
          Surface* sh = new Surface(-1, tol_snap);
          for (auto& polygon : shell) { 
            std::vector< std::vector<int> > pa = polygon;
            process_json_surface(pa, j, sh);
          }
          if (oshell == true)
          {
            oshell = false;
            s->set_oshell(sh);
          }
          else
            s->add_ishell(sh);
        }
        dPrimitives[coid].push_back(s);
      }
      else if ( (g["type"] == "MultiSurface") || (g["type"] == "CompositeSurface") ) 
      {
        Surface* sh = new Surface(-1, tol_snap);
        for (auto& p : g["boundaries"]) 
        { 
          std::vector< std::vector<int> > pa = p;
          process_json_surface(pa, j, sh);
        }
        if (g["type"] == "MultiSurface")
        {
          MultiSurface* ms = new MultiSurface(theid);
          ms->set_surface(sh);
          dPrimitives[coid].push_back(ms);
        }
        else
        {
          CompositeSurface* cs = new CompositeSurface(theid);
          cs->set_surface(sh);
          dPrimitives[coid].push_back(cs);
        }
      }
      else if (g["type"] == "MultiSolid") 
      {
        MultiSolid* ms = new MultiSolid(theid);
        for (auto& solid : g["boundaries"]) 
        {
          Solid* s = new Solid();
          bool oshell = true;
          for (auto& shell : solid) 
          {
            Surface* sh = new Surface(-1, tol_snap);
            for (auto& polygon : shell) { 
              std::vector< std::vector<int> > pa = polygon;
              process_json_surface(pa, j, sh);
            }
            if (oshell == true)
            {
              oshell = false;
              s->set_oshell(sh);
            }
            else
              s->add_ishell(sh);
          }
          ms->add_solid(s);
        }
        dPrimitives[coid].push_back(ms);
      }
      else if (g["type"] == "CompositeSolid") 
      {
        CompositeSolid* cs = new CompositeSolid(theid);
        for (auto& solid : g["boundaries"]) 
        {
          Solid* s = new Solid();
          bool oshell = true;
          for (auto& shell : solid) 
          {
            Surface* sh = new Surface(-1, tol_snap);
            for (auto& polygon : shell) { 
              std::vector< std::vector<int> > pa = polygon;
              process_json_surface(pa, j, sh);
            }
            if (oshell == true)
            {
              oshell = false;
              s->set_oshell(sh);
            }
            else
              s->add_ishell(sh);
          }
          cs->add_solid(s);
        }
        dPrimitives[coid].push_back(cs);
      }      
    }
    idgeom++;
  }
}


void process_gml_file_primitives(pugi::xml_document& doc, std::map<std::string, std::vector<Primitive*> >& dPrimitives, std::map<std::string, pugi::xpath_node>& dallpoly, IOErrors& errs, double tol_snap)
{
  primitives_walker walker;
  doc.traverse(walker);
  std::cout << "# 3D primitives found: " << walker.lsNodes.size() << std::endl;
  int primid = 0;
  std::string coid = "Primitives";
  for (auto& prim : walker.lsNodes)
  {
    if (remove_xml_namespace(prim.name()).compare("Solid") == 0)
    {
      Solid* p = process_gml_solid(prim, dallpoly, tol_snap, errs);
      if (p->get_id().compare("") == 0)
        p->set_id(std::to_string(primid));
      dPrimitives[coid].push_back(p);
    }
    else if (remove_xml_namespace(prim.name()).compare("MultiSolid") == 0)
    {
      MultiSolid* p = process_gml_multisolid(prim, dallpoly, tol_snap, errs);
      if (p->get_id().compare("") == 0)
        p->set_id(std::to_string(primid));
      dPrimitives[coid].push_back(p);
    }      
    else if (remove_xml_namespace(prim.name()).compare("CompositeSolid") == 0)
    {
      CompositeSolid* p = process_gml_compositesolid(prim, dallpoly, tol_snap, errs);
      if (p->get_id().compare("") == 0)
        p->set_id(std::to_string(primid));
      dPrimitives[coid].push_back(p);
    }
    else if (remove_xml_namespace(prim.name()).compare("MultiSurface") == 0)
    {
      MultiSurface* p = process_gml_multisurface(prim, dallpoly, tol_snap, errs);
      if (p->get_id().compare("") == 0)
        p->set_id(std::to_string(primid));
      dPrimitives[coid].push_back(p);
    } 
    else if (remove_xml_namespace(prim.name()).compare("CompositeSurface") == 0)
    {
      CompositeSurface* p = process_gml_compositesurface(prim, dallpoly, tol_snap, errs);
      if (p->get_id().compare("") == 0)
        p->set_id(std::to_string(primid));
      dPrimitives[coid].push_back(p);
    } 
    primid++;
  }  
}


void process_gml_file_city_objects(pugi::xml_document& doc, std::map<std::string, std::vector<Primitive*> >& dPrimitives, std::map<std::string, pugi::xpath_node>& dallpoly, IOErrors& errs, double tol_snap)
{
  //-- read each CityObject in the file
  citygml_objects_walker walker;
  doc.traverse(walker);
  std::cout << "# City Objects found: " << walker.lsNodes.size() << std::endl;
  //-- for each City Object parse its primitives
  for (auto& co : walker.lsNodes)
  {
    std::string coid = remove_xml_namespace(co.name());
    coid += "|";
    if (co.attribute("gml:id") != 0)
      coid += co.attribute("gml:id").value();
    primitives_walker walker2;
    co.traverse(walker2);
    for (auto& prim : walker2.lsNodes)
    {
      std::string typeprim = remove_xml_namespace(prim.name());
      if (remove_xml_namespace(prim.name()).compare("Solid") == 0)
      {
        Solid* p = process_gml_solid(prim, dallpoly, tol_snap, errs);
        dPrimitives[coid].push_back(p);
      }
      else if (remove_xml_namespace(prim.name()).compare("MultiSolid") == 0)
      {
        MultiSolid* p = process_gml_multisolid(prim, dallpoly, tol_snap, errs);
        dPrimitives[coid].push_back(p);
      }      
      else if (remove_xml_namespace(prim.name()).compare("CompositeSolid") == 0)
      {
        CompositeSolid* p = process_gml_compositesolid(prim, dallpoly, tol_snap, errs);
        dPrimitives[coid].push_back(p);
      }
      else if (remove_xml_namespace(prim.name()).compare("MultiSurface") == 0)
      {
        MultiSurface* p = process_gml_multisurface(prim, dallpoly, tol_snap, errs);
        dPrimitives[coid].push_back(p);
      } 
      else if (remove_xml_namespace(prim.name()).compare("CompositeSurface") == 0)
      {
        CompositeSurface* p = process_gml_compositesurface(prim, dallpoly, tol_snap, errs);
        dPrimitives[coid].push_back(p);
      } 
    }
  }

}

void read_file_gml(std::string &ifile, std::map<std::string, std::vector<Primitive*> >& dPrimitives, IOErrors& errs, double tol_snap)
{
  std::cout << "Reading file: " << ifile << std::endl;
  pugi::xml_document doc;
  if (!doc.load_file(ifile.c_str())) 
  {
    errs.add_error(901, "Input file not found.");
    return;
  }
  //-- parse namespace
  pugi::xml_node ncm = doc.first_child();
  std::string vcitygml;
  get_namespaces(ncm, vcitygml); //-- results in global variable NS in this unit
  if (NS.count("gml") == 0)
  {
    errs.add_error(901, "Input file does not have the GML namespace.");
    return;
  }
  //-- build dico of xlinks for <gml:Polygon>
  std::map<std::string, pugi::xpath_node> dallpoly;
  build_dico_xlinks(doc, dallpoly, errs);
  if (NS.count("citygml") != 0)
  {
    std::cout << "CityGML input file" << std::endl;
    process_gml_file_city_objects(doc, dPrimitives, dallpoly, errs, tol_snap);
  }
  else
  {
    std::cout << "GML input file (ie not CityGML)" << std::endl;
    process_gml_file_primitives(doc, dPrimitives, dallpoly, errs, tol_snap);
  }
}


void get_namespaces(pugi::xml_node& root, std::string& vcitygml) {
  vcitygml = "";
  for (pugi::xml_attribute attr = root.first_attribute(); attr; attr = attr.next_attribute()) {
    std::string name = attr.name();
    if (name.find("xmlns") != std::string::npos) {
      // std::cout << attr.name() << "=" << attr.value() << std::endl;
      std::string value = attr.value();
      std::string sns;
      if (value.find("http://www.opengis.net/citygml/0") != std::string::npos) {
        sns = "citygml";
        vcitygml = "v0.4";
      }
      else if (value.find("http://www.opengis.net/citygml/1") != std::string::npos) {
        sns = "citygml";
        vcitygml = "v1.0";
      }
      else if (value.find("http://www.opengis.net/citygml/2") != std::string::npos) {
        sns = "citygml";
        vcitygml = "v2.0";
      }
      else if (value.find("http://www.opengis.net/gml") != std::string::npos)
        sns = "gml";
      else if (value.find("http://www.opengis.net/citygml/building") != std::string::npos)
        sns = "building";
      else if (value.find("http://www.opengis.net/citygml/relief") != std::string::npos)
        sns = "dem";
      else if (value.find("http://www.opengis.net/citygml/vegetation") != std::string::npos)
        sns = "veg";
      else if (value.find("http://www.opengis.net/citygml/waterbody") != std::string::npos)
        sns = "wtr";
      else if (value.find("http://www.opengis.net/citygml/landuse") != std::string::npos)
        sns = "luse";
      else if (value.find("http://www.opengis.net/citygml/transportation") != std::string::npos)
        sns = "tran";      
      else if (value.find("http://www.opengis.net/citygml/cityfurniture") != std::string::npos)
        sns = "frn";      
      else if (value.find("http://www.opengis.net/citygml/appearance") != std::string::npos)
        sns = "app";      
      else if (value.find("http://www.w3.org/1999/xlink") != std::string::npos)
        sns = "xlink";
      else
        sns = "";
      if (sns != "") {
        size_t pos = name.find(":");
        if (pos == std::string::npos) 
          NS[sns] = "";
        else 
          NS[sns] = name.substr(pos + 1) + ":";
      }    
    }
  }
}





void build_dico_xlinks(pugi::xml_document& doc, std::map<std::string, pugi::xpath_node>& dallpoly, IOErrors& errs)
{
  std::string s = "//" + NS["gml"] + "Polygon" + "[@" + NS["gml"] + "id]";
  pugi::xpath_node_set nallpoly = doc.select_nodes(s.c_str());
  if (nallpoly.size() > 0)
   std::cout << "XLinks found, resolving them..." << std::flush;
  for (pugi::xpath_node_set::const_iterator it = nallpoly.begin(); it != nallpoly.end(); ++it)
    dallpoly[it->node().attribute("gml:id").value()] = *it;
  //-- for <gml:OrientableSurface>
  s = "//" + NS["gml"] + "OrientableSurface" + "[@" + NS["gml"] + "id" + "]";
  pugi::xpath_node_set nallosurf = doc.select_nodes(s.c_str());
  for (pugi::xpath_node_set::const_iterator it = nallosurf.begin(); it != nallosurf.end(); ++it)
    dallpoly[it->node().attribute("gml:id").value()] = *it;
  //-- checking xlinks validity now, not to be bitten later
  s = "//" + NS["gml"] + "surfaceMember" + "[@" + NS["xlink"] + "href" + "]";
  pugi::xpath_node_set nsmxlink = doc.select_nodes(s.c_str());
  for (pugi::xpath_node_set::const_iterator it = nsmxlink.begin(); it != nsmxlink.end(); ++it) 
  {
    std::string k = it->node().attribute("xlink:href").value();
    if (k[0] == '#')
      k = k.substr(1);
    if (dallpoly.count(k) == 0) 
    {
      std::string r = "One XLink couldn't be resolved (";
      r += it->node().attribute("xlink:href").value();
      r += ")";
      errs.add_error(901, r);
      return;
    }
  }
  if (nallpoly.size() > 0)
    std::cout << "done." << std::endl;
}


Surface* read_file_poly(std::string &ifile, int shellid, IOErrors& errs)
{
  std::cout << "Reading file: " << ifile << std::endl;
  std::stringstream st;
  ifstream infile(ifile.c_str(), ifstream::in);
  if (!infile)
  {
    errs.add_error(901, "Input file not found.");
    return NULL;
  }
  Surface* sh = new Surface(shellid);  
  //-- read the points
  int num, tmpint;
  float tmpfloat;
  infile >> num >> tmpint >> tmpint >> tmpint;
  std::vector< Point3 >::iterator iPoint3;
  //-- read first line to decide if 0- or 1-based indexing
  bool zerobased = true;
  Point3 p;
  infile >> tmpint >> p;
  sh->add_point(p);
  if (tmpint == 1)
    zerobased = false;
  //-- process other vertices
  for (int i = 1; i < num; i++)
  {
    Point3 p;
    infile >> tmpint >> p;
    sh->add_point(p);
  }
  //-- read the facets
  infile >> num >> tmpint;
  int numf, numpt, numholes;
  string s;
  for (int i = 0; i < num; i++)
  {
    numholes = 0;
    infile >> numf;
    while(true) {
      if (infile.peek() == '\n')
        break;
      else if (infile.peek() == ' ')
        infile.ignore();
      else
        infile >> numholes;
    }
    //-- read oring (there's always one and only one)
    infile >> numpt;
    if (numpt == -1) {
      sh->add_error(103, std::to_string(i));
      continue;
    }
    std::vector<int> ids(numpt);
    for (int k = 0; k < numpt; k++)
      infile >> ids[k];
    if (zerobased == false)
    {
      for (int k = 0; k < numpt; k++)
        ids[k] = (ids[k] - 1);      
    }
    std::vector< std::vector<int> > pgnids;
    pgnids.push_back(ids);
    //-- check for irings
    for (int j = 1; j < numf; j++)
    {
      infile >> numpt;
      if (numpt == -1) {
        sh->add_error(103, std::to_string(i));
        continue;
      }
      std::vector<int> ids(numpt);
      for (int l = 0; l < numpt; l++)
        infile >> ids[l];
      if (zerobased == false)
      {
        for (int k = 0; k < numpt; k++)
          ids[k] = (ids[k] - 1);      
      }
      pgnids.push_back(ids);
    }
    //-- skip the line about points defining holes (if present)
    for (int j = 0; j < numholes; j++)
      infile >> tmpint >> tmpfloat >> tmpfloat >> tmpfloat;
    sh->add_face(pgnids);
  }
  return sh;
}

void printProgressBar(int percent) {
  std::string bar;
  for(int i = 0; i < 50; i++){
    if( i < (percent / 2)) {
      bar.replace(i, 1, "=");
    }
    else if( i == (percent / 2)) {
      bar.replace(i, 1, ">");
    }
    else{
      bar.replace(i, 1, " ");
    }
  }
  std::cout << "\r" "[" << bar << "] ";
  std::cout.width(3);
  std::cout << percent << "%     " << std::flush;
}

Surface* read_file_off(std::string &ifile, int shellid, IOErrors& errs)
{
  std::cout << "Reading file: " << ifile << std::endl;
  std::stringstream st;
  ifstream infile(ifile.c_str(), ifstream::in);
  if (!infile)
  {
    errs.add_error(901, "Input file not found.");
    return NULL;
  }
  Surface* sh = new Surface(shellid);  
  //-- read the points
  int numpt, numf, tmpint;
  float tmpfloat;
  std::string s;
  infile >> s;
  infile >> numpt >> numf >> tmpint;
  std::vector< Point3 >::iterator iPoint3;
  //-- read first line to decide if 0- or 1-based indexing
  for (int i = 0; i < numpt; i++)
  {
    Point3 p;
    infile >> p;
    sh->add_point(p);
  }
  //-- read the facets
  for (int i = 0; i < numf; i++)
  {
    infile >> tmpint;
    std::vector<int> ids(tmpint);
    for (int k = 0; k < tmpint; k++)
      infile >> ids[k];
    std::vector< std::vector<int> > pgnids;
    pgnids.push_back(ids);
    sh->add_face(pgnids);
  }
  return sh;
}


void read_file_obj(std::map<std::string, std::vector<Primitive*> >& dPrimitives, std::string &ifile, Primitive3D prim3d, IOErrors& errs, double tol_snap)
{
  std::cout << "Reading file: " << ifile << std::endl;
  std::ifstream infile(ifile.c_str(), std::ifstream::in);
  if (!infile)
  {
    errs.add_error(901, "Input file not found.");
    return;
  }
  int primid = 0;
  std::cout << "Parsing the file..." << std::endl; 
  Surface* sh = new Surface(0, tol_snap);
  std::string l;
  std::vector<Point3*> allvertices;
  while (std::getline(infile, l)) {
    std::istringstream iss(l);
    if (l.substr(0, 2) == "v ") {
      Point3 *p = new Point3();
      std::string tmp;
      iss >> tmp >> *p;
      allvertices.push_back(p);
    }
    else if (l.substr(0, 2) == "o ") {
      if (sh->is_empty() == false)
      {
        if (prim3d == SOLID)
        {
          Solid* sol = new Solid(std::to_string(primid));
          sol->set_oshell(sh);
          dPrimitives["Primitives"].push_back(sol);
        }
        else if ( prim3d == COMPOSITESURFACE)
        {
          CompositeSurface* cs = new CompositeSurface(std::to_string(primid));
          cs->set_surface(sh);
          dPrimitives["Primitives"].push_back(cs);
        }
        else if (prim3d == MULTISURFACE)
        {
          MultiSurface* ms = new MultiSurface(std::to_string(primid));
          ms->set_surface(sh);
          dPrimitives["Primitives"].push_back(ms);
        }
        primid++;
        sh = new Surface(0, tol_snap);
      }
    }
    else if (l.substr(0, 2) == "f ") {
      std::vector<int> r;
      std::string tmp;
      iss >> tmp;
      while (iss)
      {
        tmp.clear();
        iss >> tmp;
        if (tmp.compare("\\") == 0) {
          std::getline(infile, l);
          iss.str(l);
          continue;
        }
        if (tmp.empty() == false) {
          std::size_t k = tmp.find("/");
          Point3* tp = allvertices[std::stoi(tmp.substr(0, k)) - 1];
          r.push_back(sh->add_point(*tp));
        }
      }
      std::vector< std::vector<int> > pgnids;
      pgnids.push_back(r);
      sh->add_face(pgnids);
    }
  }
  if (prim3d == SOLID)
  {
    Solid* sol = new Solid(std::to_string(primid));
    sol->set_oshell(sh);
    dPrimitives["Primitives"].push_back(sol);
  }
  else if ( prim3d == COMPOSITESURFACE)
  {
    CompositeSurface* cs = new CompositeSurface(std::to_string(primid));
    cs->set_surface(sh);
    dPrimitives["Primitives"].push_back(cs);
  }
  else if (prim3d == MULTISURFACE)
  {
    MultiSurface* ms = new MultiSurface(std::to_string(primid));
    ms->set_surface(sh);
    dPrimitives["Primitives"].push_back(ms);
  }
  for (auto& each : allvertices)
    delete each;
  allvertices.clear();
} 

} // namespace val3dity