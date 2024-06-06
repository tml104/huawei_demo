#pragma once
#include "json/json.h"
#include "Cell.h"
#include "pixel.h"
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
bool ReadSatJson(std::string path,std::vector<double> &x_pos,std::vector<double> &y_pos,std::vector<double> &z_pos);
bool read_one_cell_json(string filename, Ohm_slice::Cell& cell);
void DeleteOneCellPixel(Ohm_slice::Cell& cell);
bool SetCellPos(Ohm_slice::Cell& cell,std::vector<double> &x_pos,std::vector<double> &y_pos,std::vector<double> &z_pos);

void Cell2Json(std::string filePath, std::unordered_set<Ohm_slice::CellIdx, Ohm_slice::CellIdxHash, Ohm_slice::CellEqual>& cellidx);
void Json2Cell(std::string filePath, std::unordered_set<Ohm_slice::CellIdx, Ohm_slice::CellIdxHash, Ohm_slice::CellEqual>& cellidx);

void getFiles(string path, vector<string>& files);