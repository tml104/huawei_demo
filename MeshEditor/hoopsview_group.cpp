#include "stdafx.h"
#include "hoopsview.h"
#include "HSelectionSet.h"
#include "string.h"

HC_KEY HoopsView::render_mesh_group (VolumeMeshElementGroup *group, bool show_indices, 
	const char *vertex_color, const char *edge_color, const char *face_color, const char *cell_color, const char *text_color)
{
	if (mesh_group_exists (group))
		return INVALID_KEY;
	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	double hexa_shrink_ratio = 0.7;
	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);
			if (text_color){
				char color_buf[100];
				sprintf (color_buf, "text=%s", text_color);
				HC_Set_Color (color_buf);
			}
			HC_Open_Segment ("groupvertices");{
				if (vertex_color){
					char color_buf[100];
					sprintf (color_buf, "markers=%s", vertex_color);
					HC_Set_Color (color_buf);
				}
				for (auto v_it = group->vhs.begin (); v_it != group->vhs.end (); ++v_it)
				{
					int idx = (*v_it).idx ();
					auto pt = group->mesh->vertex (*v_it);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupedges");{
				if (edge_color){
					char color_buf[100];
					sprintf (color_buf, "lines=%s", edge_color);
					HC_Set_Color (color_buf);
					HC_Set_Line_Weight (4);
				}
				for (auto e_it = group->ehs.begin (); e_it != group->ehs.end (); ++e_it)
				{
					int idx = (*e_it).idx ();
					auto eg = group->mesh->edge (*e_it);
					auto pt1 = group->mesh->vertex (eg.from_vertex ()), 
						pt2 = group->mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						auto midpos = (pt1 + pt2) / 2;
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupfaces");{
				if (face_color){
					char color_buf[100];
					sprintf (color_buf, "faces=%s", face_color);
					HC_Set_Color (color_buf);
				}
				for (auto f_it = group->fhs.begin (); f_it != group->fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[4];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						auto pt = group->mesh->vertex (*fv_it);
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (4, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "f%d", idx);
						auto midpos = group->mesh->barycenter (*f_it);
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupcells");{
				HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
				HC_Set_Rendering_Options ("no lighting interpolation");
				if (cell_color){
					char color_buf[100];
					sprintf (color_buf, "edges=%s,faces=%s", cell_color, cell_color);
					HC_Set_Color (color_buf);
				}
				foreach (auto &ch, group->chs){
					int idx = ch.idx ();
					auto centre_pos = group->mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					std::vector<HPoint> pts;
					int count = 0;
					std::hash_map<OvmVeH, int> indices_map;
					for (auto hv_it = group->mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = group->mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (std::make_pair (*hv_it, count));
					}

					auto hfh_vec = group->mesh->cell (ch).halffaces ();
					std::vector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = group->mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = group->mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, idx_str);
					}
				}//end foreach (auto &ch, group->chs){...
			}HC_Close_Segment ();
		}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
	HC_Renumber_Key (old_key, group_key, "global");
	
	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}

HC_KEY HoopsView::render_mesh_group_1 (VolumeMeshElementGroup *group, bool show_indices, double _v_value,
	double _e_value, double _f_value, double _c_value, double _t_value)
{
	if (mesh_group_exists (group))
		return INVALID_KEY;
	auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
		OvmVec3d _rgb_;
		double n = 12;
		if (_scale_ == 0)
		{
			_rgb_ = OvmVec3d(227,160,93);
		}
		else if (_scale_ < double(1.0/n))
		{
			_rgb_ = OvmVec3d(128,26,28);
			//_rgb_ = OvmVec3d(255,255,179);
		}
		else if (_scale_ < double(2.0/n))
		{
			_rgb_ = OvmVec3d(55,126,184);
		}
		else if (_scale_ < double(3.0/n))
		{
			_rgb_ = OvmVec3d(77,175,74);//
		}
		else if (_scale_ < double(4.0/n))
		{
			_rgb_ = OvmVec3d(152,78,163);//
		}
		else if (_scale_ < double(5.0/n))
		{
			_rgb_ = OvmVec3d(255,127,0);//
		}
		else if (_scale_ < double(6.0/n))
		{
			_rgb_ = OvmVec3d(255,255,51);//
		}
		else if (_scale_ < double(7.0/n))
		{
			_rgb_ = OvmVec3d(251,154,153);//_rgb_ = OvmVec3d(166,86,40);
		}
		else if (_scale_ < double(8.0/n))
		{
			_rgb_ = OvmVec3d(247,129,191);//
		}
		else if(_scale_ < 9.0/n)
		{
			_rgb_ = OvmVec3d(141,211,199);
		}
		else if (_scale_ < 10.0/n)
		{
			_rgb_ = OvmVec3d(254,67,101);
		}
		else if (_scale_ < 11.0/n)
		{
			_rgb_ = OvmVec3d(29,191,151);
		}
		else if(_scale_ < 12.0/n)
		{
			_rgb_ = OvmVec3d(255,153,102);
		}
		else
		{
			_rgb_ =OvmVec3d(0,153,153);
		}
		std::ostringstream ros,bos,gos;
		ros<<_rgb_[0]/256;gos<<_rgb_[1]/256;bos<<_rgb_[2]/256;
		_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	};
	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	double hexa_shrink_ratio = 0.7;

	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);
			if (_t_value != -1){
				std::string t_color;
				fGet_RGB_Value(_t_value, t_color);
				std::string news = "text = "+t_color;
				HC_Set_Color (news.data());
			}
			HC_Open_Segment ("groupvertices");{
				if (_v_value != -1){
					std::string v_clor;
					fGet_RGB_Value(_v_value,v_clor);
					std::string newv = "markers = "+v_clor;
					HC_Set_Color (newv.data());
				}
				for (auto v_it = group->vhs.begin (); v_it != group->vhs.end (); ++v_it)
				{
					int idx = (*v_it).idx ();
					auto pt = group->mesh->vertex (*v_it);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupedges");{
				if (_e_value != -1){
					std::string e_clor;
					fGet_RGB_Value(_e_value,e_clor);
					std::string newe = "lines = "+e_clor;
					HC_Set_Color (newe.data());
					HC_Set_Line_Weight (4);
				}
				for (auto e_it = group->ehs.begin (); e_it != group->ehs.end (); ++e_it)
				{
					int idx = (*e_it).idx ();
					auto eg = group->mesh->edge (*e_it);
					auto pt1 = group->mesh->vertex (eg.from_vertex ()), 
						pt2 = group->mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						auto midpos = (pt1 + pt2) / 2;
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupfaces");{
				if (_f_value != -1){
					std::string f_clor;
					fGet_RGB_Value(_f_value,f_clor);
					std::string newf = "faces = "+f_clor;
					HC_Set_Color (newf.data());
				}
				for (auto f_it = group->fhs.begin (); f_it != group->fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[4];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						auto pt = group->mesh->vertex (*fv_it);
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (4, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "f%d", idx);
						auto midpos = group->mesh->barycenter (*f_it);
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupcells");{
				HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
				HC_Set_Rendering_Options ("no lighting interpolation");
				if (_c_value != -1){
					std::string c_clor;
					fGet_RGB_Value(_c_value,c_clor);
					std::string newc = "edges = "+c_clor+",faces = "+c_clor;
					HC_Set_Color (newc.data());
				}
				foreach (auto &ch, group->chs){
					int idx = ch.idx ();
					auto centre_pos = group->mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					std::vector<HPoint> pts;
					int count = 0;
					std::hash_map<OvmVeH, int> indices_map;
					for (auto hv_it = group->mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = group->mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (std::make_pair (*hv_it, count));
					}

					auto hfh_vec = group->mesh->cell (ch).halffaces ();
					std::vector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = group->mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = group->mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, idx_str);
					}
				}//end foreach (auto &ch, group->chs){...
			}HC_Close_Segment ();
	}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
		HC_Renumber_Key (old_key, group_key, "global");

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}


HC_KEY HoopsView::render_mesh_group_semitransparent (VolumeMeshElementGroup *group, bool show_indices, double _v_value,
	double _e_value, double _f_value, double _c_value, double _t_value)
{
	if (mesh_group_exists (group))
		return INVALID_KEY;
	auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
		OvmVec3d _rgb_;
		double n = 12;
		if (_scale_ == 0)
		{
			_rgb_ = OvmVec3d(227,160,93);
		}
		else if (_scale_ < double(1.0/n))
		{
			_rgb_ = OvmVec3d(128,26,28);
			//_rgb_ = OvmVec3d(255,255,179);
		}
		else if (_scale_ < double(2.0/n))
		{
			_rgb_ = OvmVec3d(55,126,184);
		}
		else if (_scale_ < double(3.0/n))
		{
			_rgb_ = OvmVec3d(77,175,74);//
		}
		else if (_scale_ < double(4.0/n))
		{
			_rgb_ = OvmVec3d(152,78,163);//
		}
		else if (_scale_ < double(5.0/n))
		{
			_rgb_ = OvmVec3d(255,127,0);//
		}
		else if (_scale_ < double(6.0/n))
		{
			_rgb_ = OvmVec3d(255,255,51);//
		}
		else if (_scale_ < double(7.0/n))
		{
			_rgb_ = OvmVec3d(251,154,153);//_rgb_ = OvmVec3d(166,86,40);
		}
		else if (_scale_ < double(8.0/n))
		{
			_rgb_ = OvmVec3d(247,129,191);//
		}
		else if(_scale_ < 9.0/n)
		{
			_rgb_ = OvmVec3d(141,211,199);
		}
		else if (_scale_ < 10.0/n)
		{
			_rgb_ = OvmVec3d(254,67,101);
		}
		else if (_scale_ < 11.0/n)
		{
			_rgb_ = OvmVec3d(29,191,151);
		}
		else if(_scale_ < 12.0/n)
		{
			_rgb_ = OvmVec3d(255,153,102);
		}
		else
		{
			_rgb_ =OvmVec3d(0,153,153);
		}
		std::ostringstream ros,bos,gos;
		ros<<_rgb_[0]/256;gos<<_rgb_[1]/256;bos<<_rgb_[2]/256;
		_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	};
	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	double hexa_shrink_ratio = 0.7;

	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);
			if (_t_value != -1){
				std::string t_color;
				fGet_RGB_Value(_t_value, t_color);
				std::string news = "text = "+t_color;
				HC_Set_Color (news.data());
			}
			HC_Open_Segment ("groupvertices");{
				if (_v_value != -1){
					std::string v_clor;
					fGet_RGB_Value(_v_value,v_clor);
					std::string newv = "markers = "+v_clor;
					HC_Set_Color (newv.data());
				}
				for (auto v_it = group->vhs.begin (); v_it != group->vhs.end (); ++v_it)
				{
					int idx = (*v_it).idx ();
					auto pt = group->mesh->vertex (*v_it);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupedges");{
				if (_e_value != -1){
					std::string e_clor;
					fGet_RGB_Value(_e_value,e_clor);
					std::string newe = "lines = "+e_clor;
					HC_Set_Color (newe.data());
					HC_Set_Line_Weight (4);
				}
				for (auto e_it = group->ehs.begin (); e_it != group->ehs.end (); ++e_it)
				{
					int idx = (*e_it).idx ();
					auto eg = group->mesh->edge (*e_it);
					auto pt1 = group->mesh->vertex (eg.from_vertex ()), 
						pt2 = group->mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						auto midpos = (pt1 + pt2) / 2;
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupfaces");{
				if (_f_value != -1){
					std::string f_clor;
					fGet_RGB_Value(_f_value,f_clor);
					//"faces = (transmission = (r=0.2 g=0.2 b=0.2), diffuse = cyan)"
					//std::string newf = "faces = (transmission = (r=0.9 g = 0.9 b=0.9)), diffuse = "+f_clor+")";
					std::string newf = "faces = "+f_clor;
					HC_Set_Color (newf.data());
					HC_Set_Color("transmission = (r=0.4 g = 0.4 b=0.4)");
				}
				for (auto f_it = group->fhs.begin (); f_it != group->fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[4];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						auto pt = group->mesh->vertex (*fv_it);
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (4, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "f%d", idx);
						auto midpos = group->mesh->barycenter (*f_it);
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupcells");{
				HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
				HC_Set_Rendering_Options ("no lighting interpolation");
				if (_c_value != -1){
					std::string c_clor;
					fGet_RGB_Value(_c_value,c_clor);
					std::string newc = "edges = "+c_clor+",faces = "+c_clor;
					HC_Set_Color (newc.data());
				}
				foreach (auto &ch, group->chs){
					int idx = ch.idx ();
					auto centre_pos = group->mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					std::vector<HPoint> pts;
					int count = 0;
					std::hash_map<OvmVeH, int> indices_map;
					for (auto hv_it = group->mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = group->mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (std::make_pair (*hv_it, count));
					}

					auto hfh_vec = group->mesh->cell (ch).halffaces ();
					std::vector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = group->mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = group->mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, idx_str);
					}
				}//end foreach (auto &ch, group->chs){...
			}HC_Close_Segment ();
	}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
		HC_Renumber_Key (old_key, group_key, "global");

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}

HC_KEY HoopsView::render_mesh_block (VolumeMeshElementGroup *group,double _value_)
{
	if (mesh_group_exists (group))
		return INVALID_KEY;
	//auto fGet_RGB_Value = [&](double _scale_, std::string& _color_)
	//{
	//	std::cout<<"_scale: "<<_scale_<<std::endl;
	//	double _r_,_g_,_b_;
	//	//////////////////////////////////////////////////////////////////////////
	//	if (_scale_ < 0)
	//	{
	//		_r_ = 0.5;_g_ = 0;_b_ = 0;
	//	}
	//	else if (_scale_ < 0.125)
	//	{
	//		_r_ = 4*_scale_+0.5;_g_ = 0;_b_ = 0;
	//	}
	//	else if (_scale_ < 0.375)
	//	{
	//		_r_ = 1;_g_ = 4*(_scale_-0.125);_b_ = 0;
	//	}
	//	else if (_scale_ < 0.625)
	//	{
	//		_r_ = 1-4*(_scale_-0.375);_g_ = 1; _b_ = 4*(_scale_-0.375);
	//	}
	//	else if (_scale_ < 0.875)
	//	{
	//		_r_ = 0;_g_ = 1-4*(_scale_-0.625);_b_ = 1;
	//	}
	//	else if (_scale_ <= 1)
	//	{
	//		_r_ = 0; _g_ = 0; _b_ = 1-4*(_scale_-0.875);
	//	}
	//	else 
	//	{
	//		_r_ = 0;_g_ = 0;_b_ = 0.5;
	//	}
	//	//////////////////////////////////////////////////////////////////////////
	//	//double Lu,US,VC;
	//	//VC = 80/240+_scale_*100/240;
	//	//Lu = 80/240+_scale_*100/240;
	//	//US = 80/240+_scale_*100/240;
	//	//_r_ = Lu+1.402*(VC-0.5);
	//	//_g_ = Lu-0.34414*(US-0.5)-0.71414*(VC-0.5);
	//	//_b_ = Lu+1.772*(US-0.5);
	//	//while(_r_ > 1)
	//	//	_r_ -= 1;
	//	//while(_g_ > 1)
	//	//	_g_ -= 1;
	//	//while(_b_ > 1)
	//	//	_b_ -= 1;
	//	//while(_r_ < 0)
	//	//	_r_ += 1;
	//	//while(_g_ < 0)
	//	//	_g_ += 1;
	//	//while (_b_ < 0)
	//	//	_b_ += 1;
	//	//////////////////////////////////////////////////////////////////////////
	//	std::ostringstream ros,bos,gos;
	//	ros<<_r_;gos<<_g_;bos<<_b_;
	//	_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	//};
	//auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
	//	std::cout<<"=.=.=.=..==.=.=.==.="<<_scale_<<std::endl;
	//	if (_scale_ < 0.125)
	//	{
	//		_color_ = "(r = 0.5578125 g = 0.82421875 b = 0.77734375)";
	//	}
	//	else if (_scale_ < 0.25)
	//	{
	//		_color_ = "(r = 1 g = 1 b = 0.69921875)";
	//	}
	//	else if (_scale_ < 0.375)
	//	{
	//		_color_ = "(r = 0.7421875 g = 0.7265625 b = 0.8515625)";
	//	}
	//	else if (_scale_ < 0.5)
	//	{
	//		_color_ = "(r = 0.98046875 g = 0.5 b = 0.4453125)";
	//	}
	//	else if (_scale_ < 0.625)
	//	{
	//		_color_ = "(r = 0.5 g = 0.69140625 b = 0.82421875)";
	//	}
	//	else if (_scale_ < 0.75)
	//	{
	//		_color_ = "(r = 0.98828125 g = 0.703125 b = 0.3828125)";
	//	}
	//	else if (_scale_ < 0.875)
	//	{
	//		_color_ = "(r = 0.69921875 g = 0.8671875 b = 0.41015625)";
	//	}
	//	else
	//	{
	//		_color_ = "(r = 0.984375 g = 0.80078125 b = 0.89453125)";
	//	}
	//};
	auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
		OvmVec3d _rgb_;
		double n = 10;
		if (_scale_ == 0)
		{
			_rgb_ = OvmVec3d(227,160,93);
		}
		else if (_scale_ < double(1.0/n))
		{
			_rgb_ = OvmVec3d(128,26,28);
			//_rgb_ = OvmVec3d(255,255,179);
		}
		else if (_scale_ < double(2.0/n))
		{
			_rgb_ = OvmVec3d(55,126,184);
		}
		else if (_scale_ < double(3.0/n))
		{
			_rgb_ = OvmVec3d(77,175,74);//
		}
		else if (_scale_ < double(4.0/n))
		{
			_rgb_ = OvmVec3d(152,78,163);//
		}
		else if (_scale_ < double(5.0/n))
		{
			_rgb_ = OvmVec3d(255,127,0);//
		}
		else if (_scale_ < double(6.0/n))
		{
			_rgb_ = OvmVec3d(255,255,51);//
		}
		else if (_scale_ < double(7.0/n))
		{
			_rgb_ = OvmVec3d(251,154,153);//_rgb_ = OvmVec3d(166,86,40);
		}
		else if (_scale_ < double(8.0/n))
		{
			_rgb_ = OvmVec3d(247,129,191);//
		}
		else if(_scale_ < 9.0/n)
		{
			_rgb_ = OvmVec3d(141,211,199);
		}
		else if (_scale_ < 10.0/n)
		{
			_rgb_ = OvmVec3d(254,67,101);
		}
		else 
		{
			_rgb_ = OvmVec3d(29,191,151);
		}
		std::ostringstream ros,bos,gos;
		ros<<_rgb_[0]/256;gos<<_rgb_[1]/256;bos<<_rgb_[2]/256;
		_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	};
	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	double hexa_shrink_ratio = 1;
	//////////////////////////////////////////////////////////////////////////
	std::unordered_set<OvmVeH> _vhs;
	for (auto cit = group->chs.begin();cit != group->chs.end();cit++)
	{
		for (auto cvit = group->mesh->cv_iter(*cit);cvit;cvit++)
		{
			_vhs.insert(*cvit);
		}
	}
	OvmVec3d center_p(0,0,0);
	for (auto vit = _vhs.begin();vit != _vhs.end();vit++)
	{
		center_p += group->mesh->vertex(*vit);
	}
	center_p /= _vhs.size();
	std::hash_map<OvmVeH,OvmVec3d> mapping;
	for (auto vit = _vhs.begin();vit != _vhs.end();vit++)
	{
		OvmVec3d p = hexa_shrink_ratio* group->mesh->vertex(*vit) + (1-hexa_shrink_ratio)*center_p;
		mapping[*vit] = p;
	}
	std::unordered_set<OvmFaH> boundary_fhs;
	std::vector<bool> if_f(group->mesh->n_faces(),false);
	for (auto cit = group->chs.begin();cit != group->chs.end();cit++)
	{
		auto adj_fhs = group->mesh->cell(*cit).halffaces();
		if (adj_fhs.size() == 0)
		{
			system("pause");
		}
		for (auto hfit = adj_fhs.begin();hfit != adj_fhs.end();hfit++)
		{
			OvmFaH curretn_fh = group->mesh->face_handle(*hfit);
			if (if_f[curretn_fh.idx()] == true)
			{
				if_f[curretn_fh.idx()] = false;
			}
			else
			{
				if_f[curretn_fh.idx()] = true;
			}
		}
	}
	for (auto fit = group->mesh->faces_begin();fit != group->mesh->faces_end();fit++)
	{
		if(if_f[fit->idx()])
		{
			boundary_fhs.insert(*fit);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);

	/*		HC_Open_Segment ("groupfaces");{
				if (_value_ != -1){
					std::string f_clor;
					fGet_RGB_Value(_value_,f_clor);
					std::string newf = "faces = "+f_clor;
					HC_Set_Color (newf.data());
				}
				for (auto f_it = boundary_fhs.begin (); f_it != boundary_fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[4];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						auto pt = group->mesh->vertex (*fv_it);
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (4, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
				}
			}HC_Close_Segment ();*/

			HC_Open_Segment ("groupcells");{
				HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
				HC_Set_Rendering_Options ("no lighting interpolation");
				if (_value_ != -1){
					std::string c_clor;
					fGet_RGB_Value(_value_,c_clor);
					//std::string newc = "edges = "+c_clor+",faces = "+c_clor;
					std::string newc = "edges = black, faces = "+c_clor;
					//std::string newc = "faces ="+c_clor;
					//std::cout<<"colors: "<<newc<<std::endl;
					HC_Set_Color (newc.data());
				}
				foreach (auto &ch, group->chs){
					int idx = ch.idx ();
					auto centre_pos = group->mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					std::vector<HPoint> pts;
					int count = 0;
					std::hash_map<OvmVeH, int> indices_map;
					for (auto hv_it = group->mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = group->mesh->vertex (*hv_it);
						//auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
						auto new_pt = mapping[*hv_it];
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (std::make_pair (*hv_it, count));
					}

					auto hfh_vec = group->mesh->cell (ch).halffaces ();
					std::vector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = group->mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = group->mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
				}//end foreach (auto &ch, group->chs){...
			}HC_Close_Segment ();


	}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
		HC_Renumber_Key (old_key, group_key, "global");

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}


HC_KEY HoopsView::render_mesh_block_boom(VolumeMeshElementGroup *group,double _value_,OvmVec3d center_pos)
{
	if (mesh_group_exists (group))
		return INVALID_KEY;
	//auto fGet_RGB_Value = [&](double _scale_, std::string& _color_)
	//{
	//	std::cout<<"_scale: "<<_scale_<<std::endl;
	//	double _r_,_g_,_b_;
	//	//////////////////////////////////////////////////////////////////////////
	//	if (_scale_ < 0)
	//	{
	//		_r_ = 0.5;_g_ = 0;_b_ = 0;
	//	}
	//	else if (_scale_ < 0.125)
	//	{
	//		_r_ = 4*_scale_+0.5;_g_ = 0;_b_ = 0;
	//	}
	//	else if (_scale_ < 0.375)
	//	{
	//		_r_ = 1;_g_ = 4*(_scale_-0.125);_b_ = 0;
	//	}
	//	else if (_scale_ < 0.625)
	//	{
	//		_r_ = 1-4*(_scale_-0.375);_g_ = 1; _b_ = 4*(_scale_-0.375);
	//	}
	//	else if (_scale_ < 0.875)
	//	{
	//		_r_ = 0;_g_ = 1-4*(_scale_-0.625);_b_ = 1;
	//	}
	//	else if (_scale_ <= 1)
	//	{
	//		_r_ = 0; _g_ = 0; _b_ = 1-4*(_scale_-0.875);
	//	}
	//	else 
	//	{
	//		_r_ = 0;_g_ = 0;_b_ = 0.5;
	//	}
	//	//////////////////////////////////////////////////////////////////////////
	//	//double Lu,US,VC;
	//	//VC = 80/240+_scale_*100/240;
	//	//Lu = 80/240+_scale_*100/240;
	//	//US = 80/240+_scale_*100/240;
	//	//_r_ = Lu+1.402*(VC-0.5);
	//	//_g_ = Lu-0.34414*(US-0.5)-0.71414*(VC-0.5);
	//	//_b_ = Lu+1.772*(US-0.5);
	//	//while(_r_ > 1)
	//	//	_r_ -= 1;
	//	//while(_g_ > 1)
	//	//	_g_ -= 1;
	//	//while(_b_ > 1)
	//	//	_b_ -= 1;
	//	//while(_r_ < 0)
	//	//	_r_ += 1;
	//	//while(_g_ < 0)
	//	//	_g_ += 1;
	//	//while (_b_ < 0)
	//	//	_b_ += 1;
	//	//////////////////////////////////////////////////////////////////////////
	//	std::ostringstream ros,bos,gos;
	//	ros<<_r_;gos<<_g_;bos<<_b_;
	//	_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	//};
	//auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
	//	std::cout<<"=.=.=.=..==.=.=.==.="<<_scale_<<std::endl;
	//	if (_scale_ < 0.125)
	//	{
	//		_color_ = "(r = 0.5578125 g = 0.82421875 b = 0.77734375)";
	//	}
	//	else if (_scale_ < 0.25)
	//	{
	//		_color_ = "(r = 1 g = 1 b = 0.69921875)";
	//	}
	//	else if (_scale_ < 0.375)
	//	{
	//		_color_ = "(r = 0.7421875 g = 0.7265625 b = 0.8515625)";
	//	}
	//	else if (_scale_ < 0.5)
	//	{
	//		_color_ = "(r = 0.98046875 g = 0.5 b = 0.4453125)";
	//	}
	//	else if (_scale_ < 0.625)
	//	{
	//		_color_ = "(r = 0.5 g = 0.69140625 b = 0.82421875)";
	//	}
	//	else if (_scale_ < 0.75)
	//	{
	//		_color_ = "(r = 0.98828125 g = 0.703125 b = 0.3828125)";
	//	}
	//	else if (_scale_ < 0.875)
	//	{
	//		_color_ = "(r = 0.69921875 g = 0.8671875 b = 0.41015625)";
	//	}
	//	else
	//	{
	//		_color_ = "(r = 0.984375 g = 0.80078125 b = 0.89453125)";
	//	}
	//};
	auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
		OvmVec3d _rgb_;
		double n = 12;
		if (_scale_ == 0)
		{
			_rgb_ = OvmVec3d(227,160,93);
		}
		else if (_scale_ < double(1.0/n))
		{
			_rgb_ = OvmVec3d(128,26,28);
			//_rgb_ = OvmVec3d(255,255,179);
		}
		else if (_scale_ < double(2.0/n))
		{
			_rgb_ = OvmVec3d(55,126,184);
		}
		else if (_scale_ < double(3.0/n))
		{
			_rgb_ = OvmVec3d(77,175,74);//
		}
		else if (_scale_ < double(4.0/n))
		{
			_rgb_ = OvmVec3d(152,78,163);//
		}
		else if (_scale_ < double(5.0/n))
		{
			_rgb_ = OvmVec3d(255,127,0);//
		}
		else if (_scale_ < double(6.0/n))
		{
			_rgb_ = OvmVec3d(255,255,51);//
		}
		else if (_scale_ < double(7.0/n))
		{
			_rgb_ = OvmVec3d(251,154,153);//_rgb_ = OvmVec3d(166,86,40);
		}
		else if (_scale_ < double(8.0/n))
		{
			_rgb_ = OvmVec3d(247,129,191);//
		}
		else if(_scale_ < 9.0/n)
		{
			_rgb_ = OvmVec3d(141,211,199);
		}
		else if (_scale_ < 10.0/n)
		{
			_rgb_ = OvmVec3d(254,67,101);
		}
		else if (_scale_ < 11.0/n)
		{
			_rgb_ = OvmVec3d(29,191,151);
		}
		else if(_scale_ < 12.0/n)
		{
			_rgb_ = OvmVec3d(255,153,102);
		}
		else
		{
			_rgb_ =OvmVec3d(0,153,153);
		}
		std::ostringstream ros,bos,gos;
		ros<<_rgb_[0]/256;gos<<_rgb_[1]/256;bos<<_rgb_[2]/256;
		_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	};
	//average length of mesh
	double ave_len = 0;
	for (auto eit = group->mesh->edges_begin();eit != group->mesh->edges_end();eit++)
	{
		ave_len += (group->mesh->vertex(group->mesh->edge(*eit).from_vertex())-group->mesh->vertex(group->mesh->edge(*eit).to_vertex())).length();
	}
	ave_len /= group->mesh->n_edges();
	//
	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	//double hexa_shrink_ratio = 0.95;
	double hexa_shrink_ratio = 1;
	//////////////////////////////////////////////////////////////////////////
	std::unordered_set<OvmVeH> _vhs;
	for (auto cit = group->chs.begin();cit != group->chs.end();cit++)
	{
		for (auto cvit = group->mesh->cv_iter(*cit);cvit;cvit++)
		{
			_vhs.insert(*cvit);
		}
	}
	OvmVec3d center_p(0,0,0);
	for (auto vit = _vhs.begin();vit != _vhs.end();vit++)
	{
		center_p += group->mesh->vertex(*vit);
	}
	center_p /= _vhs.size();
	std::hash_map<OvmVeH,OvmVec3d> mapping;
	OvmVec3d new_center = center_pos+ 0.65*log((center_p-center_pos).length()/ave_len+10)*(center_p-center_pos);//sphere_cube
	for (auto vit = _vhs.begin();vit != _vhs.end();vit++)
	{
		//缩略图
		OvmVec3d p = hexa_shrink_ratio* group->mesh->vertex(*vit) + (1-hexa_shrink_ratio)*center_p;
		//爆炸图效果
		//OvmVec3d p = group->mesh->vertex(*vit)-center_p+new_center;
		//最原始图
		//OvmVec3d p = group->mesh->vertex(*vit);
		mapping[*vit] = p;
	}
	std::unordered_set<OvmFaH> boundary_fhs;
	std::vector<bool> if_f(group->mesh->n_faces(),false);
	for (auto cit = group->chs.begin();cit != group->chs.end();cit++)
	{
		auto adj_fhs = group->mesh->cell(*cit).halffaces();
		if (adj_fhs.size() == 0)
		{
			system("pause");
		}
		for (auto hfit = adj_fhs.begin();hfit != adj_fhs.end();hfit++)
		{
			OvmFaH curretn_fh = group->mesh->face_handle(*hfit);
			if (if_f[curretn_fh.idx()] == true)
			{
				if_f[curretn_fh.idx()] = false;
			}
			else
			{
				if_f[curretn_fh.idx()] = true;
			}
		}
	}
	for (auto fit = group->mesh->faces_begin();fit != group->mesh->faces_end();fit++)
	{
		if(if_f[fit->idx()])
		{
			boundary_fhs.insert(*fit);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);

			HC_Open_Segment ("groupfaces");{
				if (_value_ != -1){
					std::string f_clor;
					fGet_RGB_Value(_value_,f_clor);
					std::string newf = "faces = "+f_clor;
					HC_Set_Color (newf.data());
					//HC_Set_Color("transmission = (r=0.2 g = 0.2 b=0.2)");
				}
				for (auto f_it = boundary_fhs.begin (); f_it != boundary_fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[4];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						//auto pt = group->mesh->vertex (*fv_it);
						auto pt = mapping[*fv_it];
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (4, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
				}
			}HC_Close_Segment ();

			//HC_Open_Segment ("groupcells");{
			//	HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
			//	HC_Set_Rendering_Options ("no lighting interpolation");
			//	if (_value_ != -1){
			//		std::string c_clor;
			//		fGet_RGB_Value(_value_,c_clor);
			//		//std::string newc = "edges = "+c_clor+",faces = "+c_clor;
			//		std::string newc = "edges = black, faces = "+c_clor;
			//		//std::string newc = "faces ="+c_clor;
			//		//std::cout<<"colors: "<<newc<<std::endl;
			//		HC_Set_Color (newc.data());
			//	}
			//	foreach (auto &ch, group->chs){
			//		int idx = ch.idx ();
			//		auto centre_pos = group->mesh->barycenter (ch);
			//		//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
			//		std::vector<HPoint> pts;
			//		int count = 0;
			//		std::hash_map<OvmVeH, int> indices_map;
			//		for (auto hv_it = group->mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
			//			auto pt = group->mesh->vertex (*hv_it);
			//			//auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
			//			auto new_pt = mapping[*hv_it];
			//			pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
			//			indices_map.insert (std::make_pair (*hv_it, count));
			//		}

			//		auto hfh_vec = group->mesh->cell (ch).halffaces ();
			//		std::vector<int> face_list;
			//		foreach (OvmHaFaH hfh, hfh_vec){
			//			auto heh_vec = group->mesh->halfface (hfh).halfedges ();
			//			face_list.push_back (heh_vec.size ());
			//			foreach (OvmHaEgH heh, heh_vec){
			//				auto vh = group->mesh->halfedge (heh).to_vertex ();
			//				face_list.push_back (indices_map[vh]);
			//			}
			//		}
			//		HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
			//		HC_Open_Geometry (old_key);{
			//			HC_Set_User_Data (0, &idx, sizeof (int));
			//		}HC_Close_Geometry ();
			//	}//end foreach (auto &ch, group->chs){...
			//}HC_Close_Segment ();


	}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
		HC_Renumber_Key (old_key, group_key, "global");

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}

HC_KEY HoopsView::render_mesh_group_gen_ver (GenMeshElementGroup *group, bool show_indices, 
	const char *vertex_color, const char *edge_color, const char *face_color, const char *cell_color, const char *text_color)
{
	if (mesh_group_exists_gen_ver (group))
		return INVALID_KEY;

	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	double hexa_shrink_ratio = 0.7;

	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);
			if (text_color){
				char color_buf[100];
				sprintf (color_buf, "text=%s", text_color);
				HC_Set_Color (color_buf);
			}
			HC_Open_Segment ("groupvertices");{
				if (vertex_color){
					char color_buf[100];
					sprintf (color_buf, "markers=%s", vertex_color);
					HC_Set_Color (color_buf);
				}
				for (auto v_it = group->vhs.begin (); v_it != group->vhs.end (); ++v_it)
				{
					int idx = (*v_it).idx ();
					auto pt = group->mesh->vertex (*v_it);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupedges");{
				if (edge_color){
					char color_buf[100];
					sprintf (color_buf, "lines=%s", edge_color);
					HC_Set_Color (color_buf);
					HC_Set_Line_Weight (10);
				}
				for (auto e_it = group->ehs.begin (); e_it != group->ehs.end (); ++e_it)
				{
					int idx = (*e_it).idx ();
					auto eg = group->mesh->edge (*e_it);
					auto pt1 = group->mesh->vertex (eg.from_vertex ()), 
						pt2 = group->mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						auto midpos = (pt1 + pt2) / 2;
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupfaces");{
				if (face_color){
					char color_buf[100];
					sprintf (color_buf, "faces=%s", face_color);
					HC_Set_Color (color_buf);
				}
				for (auto f_it = group->fhs.begin (); f_it != group->fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[3];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						auto pt = group->mesh->vertex (*fv_it);
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (3, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "f%d", idx);
						auto midpos = group->mesh->barycenter (*f_it);
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupcells");{
				HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
				HC_Set_Rendering_Options ("no lighting interpolation");
				if (cell_color){
					char color_buf[100];
					sprintf (color_buf, "edges=%s,faces=%s", cell_color, cell_color);
					HC_Set_Color (color_buf);
				}
				foreach (auto &ch, group->chs){
					int idx = ch.idx ();
					auto centre_pos = group->mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					std::vector<HPoint> pts;
					int count = 0;
					std::hash_map<OvmVeH, int> indices_map;
					for (auto hv_it = group->mesh->cv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = group->mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (std::make_pair (*hv_it, count));
					}

					auto hfh_vec = group->mesh->cell (ch).halffaces ();
					std::vector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = group->mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = group->mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, idx_str);
					}
				}//end foreach (auto &ch, group->chs){...
			}HC_Close_Segment ();
	}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
		HC_Renumber_Key (old_key, group_key, "global");

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}

HC_KEY HoopsView::render_mesh_group_gen_ver1 (GenMeshElementGroup *group, bool show_indices, double _v_value,
	double _e_value, double _f_value, double _c_value, double _t_value)
{
	if (mesh_group_exists_gen_ver (group))
		return INVALID_KEY;
	auto fGet_RGB_Value = [&](double _scale_, std::string& _color_){
		OvmVec3d _rgb_;
		double n = 12;
		if (_scale_ == 0)
		{
			_rgb_ = OvmVec3d(227,160,93);
		}
		else if (_scale_ < double(1.0/n))
		{
			_rgb_ = OvmVec3d(128,26,28);
			//_rgb_ = OvmVec3d(255,255,179);
		}
		else if (_scale_ < double(2.0/n))
		{
			_rgb_ = OvmVec3d(55,126,184);
		}
		else if (_scale_ < double(3.0/n))
		{
			_rgb_ = OvmVec3d(77,175,74);//
		}
		else if (_scale_ < double(4.0/n))
		{
			_rgb_ = OvmVec3d(152,78,163);//
		}
		else if (_scale_ < double(5.0/n))
		{
			_rgb_ = OvmVec3d(255,127,0);//
		}
		else if (_scale_ < double(6.0/n))
		{
			_rgb_ = OvmVec3d(255,255,51);//
		}
		else if (_scale_ < double(7.0/n))
		{
			_rgb_ = OvmVec3d(251,154,153);//_rgb_ = OvmVec3d(166,86,40);
		}
		else if (_scale_ < double(8.0/n))
		{
			_rgb_ = OvmVec3d(247,129,191);//
		}
		else if(_scale_ < 9.0/n)
		{
			_rgb_ = OvmVec3d(141,211,199);
		}
		else if (_scale_ < 10.0/n)
		{
			_rgb_ = OvmVec3d(254,67,101);
		}
		else if (_scale_ < 11.0/n)
		{
			_rgb_ = OvmVec3d(29,191,151);
		}
		else if(_scale_ < 12.0/n)
		{
			_rgb_ = OvmVec3d(255,153,102);
		}
		else
		{
			_rgb_ =OvmVec3d(0,153,153);
		}
		std::ostringstream ros,bos,gos;
		ros<<_rgb_[0]/256;gos<<_rgb_[1]/256;bos<<_rgb_[2]/256;
		_color_ = "(r="+ros.str()+" g="+gos.str()+" b="+bos.str()+")";
	};
	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_KEY old_key = INVALID_KEY;
	double hexa_shrink_ratio = 0.7;

	OPEN_GROUPS_SEG
		old_key = HC_Open_Segment ("");{
			char type[100], idx_str[50];
			sprintf (type, "group type=%s,group name=%s", group->type.toAscii ().data (), 
				group->name.toAscii ().data ());
			HC_Set_User_Options (type);
			HC_Set_Visibility ("text=on");
			HC_Set_Text_Size (0.7);
			if (_t_value != -1){
				std::string t_color;
				fGet_RGB_Value(_t_value, t_color);
				std::string news = "text = "+t_color;
				HC_Set_Color (news.data());
			}
			HC_Open_Segment ("groupvertices");{
				if (_v_value != -1){
					std::string v_clor;
					fGet_RGB_Value(_v_value,v_clor);
					std::string newv = "markers = "+v_clor;
					HC_Set_Color (newv.data());
				}
				for (auto v_it = group->vhs.begin (); v_it != group->vhs.end (); ++v_it)
				{
					int idx = (*v_it).idx ();
					auto pt = group->mesh->vertex (*v_it);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupedges");{
				if (_e_value != -1){
					std::string e_clor;
					fGet_RGB_Value(_e_value,e_clor);
					std::string newe = "lines = "+e_clor;
					HC_Set_Color (newe.data());
					HC_Set_Line_Weight (4);
				}
				for (auto e_it = group->ehs.begin (); e_it != group->ehs.end (); ++e_it)
				{
					int idx = (*e_it).idx ();
					auto eg = group->mesh->edge (*e_it);
					auto pt1 = group->mesh->vertex (eg.from_vertex ()), 
						pt2 = group->mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						auto midpos = (pt1 + pt2) / 2;
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupfaces");{
				if (_f_value != -1){
					std::string f_clor;
					fGet_RGB_Value(_f_value,f_clor);
					std::string newf = "faces = "+f_clor;
					HC_Set_Color (newf.data());
				}
				for (auto f_it = group->fhs.begin (); f_it != group->fhs.end (); ++f_it)
				{
					int idx = (*f_it).idx ();
					auto f = group->mesh->face (*f_it);
					auto hfh = group->mesh->halfface_handle (*f_it, 0);
					HPoint pts[3];
					int i = 0;
					for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i)
					{
						auto pt = group->mesh->vertex (*fv_it);
						pts[i] = HPoint (pt[0], pt[1], pt[2]);
					}
					HC_KEY old_key = HC_Insert_Polygon (3, pts);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "f%d", idx);
						auto midpos = group->mesh->barycenter (*f_it);
						HC_Insert_Text (midpos[0], midpos[1], midpos[2], idx_str);
					}
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("groupcells");{
				HC_Set_Visibility ("markers=off,lines=on,edges=on,faces=on");
				HC_Set_Rendering_Options ("no lighting interpolation");
				if (_c_value != -1){
					std::string c_clor;
					fGet_RGB_Value(_c_value,c_clor);
					std::string newc = "edges = "+c_clor+",faces = "+c_clor;
					std::cout<<"colors: "<<newc<<std::endl;
					HC_Set_Color (newc.data());
				}
				foreach (auto &ch, group->chs){
					int idx = ch.idx ();
					auto centre_pos = group->mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					std::vector<HPoint> pts;
					int count = 0;
					std::hash_map<OvmVeH, int> indices_map;
					for (auto hv_it = group->mesh->cv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = group->mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (std::make_pair (*hv_it, count));
					}

					auto hfh_vec = group->mesh->cell (ch).halffaces ();
					std::vector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = group->mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = group->mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					if (show_indices){
						sprintf (idx_str, "e%d", idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, idx_str);
					}
				}//end foreach (auto &ch, group->chs){...
			}HC_Close_Segment ();
	}HC_Close_Segment ();
	CLOSE_GROUPS_SEG
		HC_Renumber_Key (old_key, group_key, "global");

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
	return group_key;
}

void HoopsView::derender_one_mesh_group (VolumeMeshElementGroup *group)
{
	HC_KEY group_key = POINTER_TO_KEY(group);
	//since group_key is local to groups_key,
	//in order to delete it, we should first open groups_key
	OPEN_GROUPS_SEG
		char status[100];
		//we check the status of group key to see whether it is valid or not
		HC_Show_Key_Status (group_key, status);
		if (QString(status) != "invalid")
			HC_Delete_By_Key (group_key);
	CLOSE_GROUPS_SEG
	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
}

void HoopsView::derender_one_genmesh_group(GenMeshElementGroup *group)
{
	HC_KEY group_key = POINTER_TO_KEY(group);
	//since group_key is local to groups_key,
	//in order to delete it, we should first open groups_key
	OPEN_GROUPS_SEG
		char status[100];
	//we check the status of group key to see whether it is valid or not
	HC_Show_Key_Status (group_key, status);
	if (QString(status) != "invalid")
		HC_Delete_By_Key (group_key);
	CLOSE_GROUPS_SEG
		m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
}

void HoopsView::derender_mesh_groups (const char *group_type, const char *group_name, bool delete_groups)
{
	char type[100], name[100];
	char child[200], pathname[200];
	std::vector<QString> mesh_groups_to_delete;
	OPEN_GROUPS_SEG
		HC_Begin_Segment_Search ("*");{
			while (HC_Find_Segment (child)){
				HC_Open_Segment (child);
				HC_Show_One_User_Option ("group type", type);
				HC_Show_One_User_Option ("group name", name);
				HC_Close_Segment ();
				if (strcmp (group_type, type) == 0){
					if (group_name){
						if (!streq (group_name, name))
							continue;
					}
					mesh_groups_to_delete.push_back (child);
				}
			}
		}HC_End_Segment_Search ();
	CLOSE_GROUPS_SEG

	for (int i = 0; i != mesh_groups_to_delete.size (); ++i){
		HC_KEY key = HC_Open_Segment (mesh_groups_to_delete[i].toAscii ().data ());
		HC_Close_Segment ();
		if (delete_groups){
			auto group = (VolumeMeshElementGroup*)(KEY_TO_POINTER(key));
			delete group;
		}
		HC_Delete_By_Key (key);
	}

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
}

void HoopsView::derender_mesh_groups_gen_ver (const char *group_type, const char *group_name, bool delete_groups)
{
	char type[100], name[100];
	char child[200], pathname[200];
	std::vector<QString> mesh_groups_to_delete;
	OPEN_GROUPS_SEG
		HC_Begin_Segment_Search ("*");{
			while (HC_Find_Segment (child)){
				HC_Open_Segment (child);
				HC_Show_One_User_Option ("group type", type);
				HC_Show_One_User_Option ("group name", name);
				HC_Close_Segment ();
				if (strcmp (group_type, type) == 0){
					if (group_name){
						if (!streq (group_name, name))
							continue;
					}
					mesh_groups_to_delete.push_back (child);
				}
			}
	}HC_End_Segment_Search ();
	CLOSE_GROUPS_SEG

		for (int i = 0; i != mesh_groups_to_delete.size (); ++i){
			HC_KEY key = HC_Open_Segment (mesh_groups_to_delete[i].toAscii ().data ());
			HC_Close_Segment ();
			if (delete_groups){
				auto group = (GenMeshElementGroup*)(KEY_TO_POINTER(key));
				delete group;
			}
			HC_Delete_By_Key (key);
		}

		m_pHView->SetGeometryChanged ();
		m_pHView->Update ();
}

void HoopsView::derender_all_mesh_groups ()
{
	OPEN_GROUPS_SEG
		HC_Flush_Contents ("./...", "segment");
	CLOSE_GROUPS_SEG
	//HC_Delete_By_Key (groups_key);

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
}

bool HoopsView::mesh_group_exists (VolumeMeshElementGroup *group)
{
	HC_KEY group_key = POINTER_TO_KEY (group);
	char status[100];
	OPEN_GROUPS_SEG
		HC_Show_Key_Status (group_key, status);
	CLOSE_GROUPS_SEG
	if (strncmp (status, "invalid", 1) == 0) return false;
	else return true;
}

bool HoopsView::mesh_group_exists_gen_ver(GenMeshElementGroup *group)
{
	HC_KEY group_key = POINTER_TO_KEY (group);
	char status[100];
	OPEN_GROUPS_SEG
		HC_Show_Key_Status (group_key, status);
	CLOSE_GROUPS_SEG
		if (strncmp (status, "invalid", 1) == 0) return false;
		else return true;
}

void HoopsView::get_mesh_groups (std::vector<VolumeMeshElementGroup*> &groups, 
	std::vector<VolumeMeshElementGroup*> &invisible_groups,
	std::vector<VolumeMeshElementGroup*> &highlighted_groups,
	const char *group_type /* = NULL */, const char *group_name /* = NULL */)
{
	char type[100], name[100], visibility[100];
	char child[200], pathname[200];
	auto fSearch = [&] (std::vector<VolumeMeshElementGroup*> &mygroups){
		HC_Begin_Segment_Search ("*");{
			while (HC_Find_Segment (child)){
				HC_KEY key = HC_Open_Segment (child);
				HC_Show_One_User_Option ("group type", type);
				HC_Show_One_User_Option ("group name", name);
				sprintf (visibility, "");
				if (HC_Show_Existence ("visibility"))
					HC_Show_Visibility (visibility);
				HC_Close_Segment ();
				if (group_type){
					if (!streq (group_type, type))
						continue;
				}
				if (group_name){
					if (!streq (group_name, name))
						continue;
				}
				
				VolumeMeshElementGroup *group = (VolumeMeshElementGroup*)(KEY_TO_POINTER (key));
				if (streq (visibility, "off"))
					invisible_groups.push_back (group);
				else
					mygroups.push_back (group);
			}
		}HC_End_Segment_Search ();
	};
	OPEN_GROUPS_SEG
		fSearch (groups);
	CLOSE_GROUPS_SEG

	OPEN_HIGHLIGHT_SEG
		fSearch (highlighted_groups);
	CLOSE_HIGHLIGHT_SEG;
}

void HoopsView::show_mesh_group (VolumeMeshElementGroup *group, bool show)
{
	if (!mesh_group_exists (group)) return;

	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_Open_Segment_By_Key (group_key);{
		if (show){
			HC_UnSet_Visibility ();
			HC_UnSet_One_Rendering_Option("attribute lock");
		}else{
			HC_Set_Visibility ("off");
			HC_Set_Rendering_Options ("attribute lock = visibility");
		}
	}HC_Close_Segment ();
	m_pHView->Update ();
}

void HoopsView::update_mesh_group (VolumeMeshElementGroup *group)
{
	if (!mesh_group_exists (group)) return;
	double hexa_shrink_ratio = 0.7;

	HC_KEY group_key = POINTER_TO_KEY (group);
	char type[255];
	HC_KEY key = INVALID_KEY;
	int idx = -1;
	HC_Open_Segment_By_Key (group_key);{
		HC_Open_Segment ("groupvertices");{
			HC_Begin_Contents_Search (".", "marker");
			while (HC_Find_Contents (type, &key)){
				HC_Open_Geometry (key);{
					HC_Show_One_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
				HC_Delete_By_Key (key);
				OvmVec3d pos = group->mesh->vertex (OvmVeH (idx));
				key = HC_Insert_Marker (pos[0], pos[1], pos[2]);
				HC_Open_Geometry (key);{
					HC_Set_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
			}
			HC_End_Contents_Search ();
		}HC_Close_Segment ();

		HC_Open_Segment ("groupedges");{
			HC_Begin_Contents_Search (".", "line");
			while (HC_Find_Contents (type, &key)){
				HC_Open_Geometry (key);{
					HC_Show_One_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
				HC_Delete_By_Key (key);
				OvmEgH eh = OvmEgH (idx);
				OvmVeH vh1 = group->mesh->edge (eh).from_vertex (),
					vh2 = group->mesh->edge (eh).to_vertex ();
				OvmVec3d pos1 = group->mesh->vertex (vh1),
					pos2 = group->mesh->vertex (vh2);
				key = HC_Insert_Line (pos1[0], pos1[1], pos1[2],
					pos2[0], pos2[1], pos2[2]);
				HC_Open_Geometry (key);{
					HC_Set_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
			}
			HC_End_Contents_Search ();
		}HC_Close_Segment ();

		HC_Open_Segment ("groupfaces");{
			HC_Begin_Contents_Search (".", "polygon");
			while (HC_Find_Contents (type, &key)){
				HC_Open_Geometry (key);{
					HC_Show_One_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
				HC_Delete_By_Key (key);
				OvmFaH fh = OvmFaH (idx);
				OvmHaFaH hfh = group->mesh->halfface_handle (fh, 0);
				HPoint pts[4];
				int i = 0;
				for (auto fv_it = group->mesh->hfv_iter (hfh); fv_it; ++fv_it, ++i){
					auto pt = group->mesh->vertex (*fv_it);
					pts[i] = HPoint (pt[0], pt[1], pt[2]);
				}
				HC_KEY old_key = HC_Insert_Polygon (4, pts);
				HC_Open_Geometry (old_key);{
					HC_Set_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
			}
			HC_End_Contents_Search ();
		}HC_Close_Segment ();

		HC_Open_Segment ("groupcells");{
			HC_Begin_Contents_Search (".", "shell");
			while (HC_Find_Contents (type, &key)){
				HC_Open_Geometry (key);{
					HC_Show_One_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
				HC_Delete_By_Key (key);

				QVector<HPoint> pts;
				int count = 0;
				QMap<OvmVeH, int> indices_map;
				OvmCeH ch = OvmCeH (idx);
				OvmVec3d centre_pos = group->mesh->barycenter (ch);
				for (auto hv_it = group->mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
					auto pt = group->mesh->vertex (*hv_it);
					auto new_pt = centre_pos + (pt - centre_pos) * hexa_shrink_ratio;
					pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
					indices_map.insert (*hv_it, count);
				}

				auto hfh_vec = group->mesh->cell (ch).halffaces ();
				QVector<int> face_list;
				foreach (OvmHaFaH hfh, hfh_vec){
					auto heh_vec = group->mesh->halfface (hfh).halfedges ();
					face_list.push_back (heh_vec.size ());
					foreach (OvmHaEgH heh, heh_vec){
						auto vh = group->mesh->halfedge (heh).to_vertex ();
						face_list.push_back (indices_map[vh]);
					}
				}
				HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
				HC_Open_Geometry (old_key);{
					HC_Set_User_Data (0, &idx, sizeof (int));
				}HC_Close_Geometry ();
			}
			HC_End_Contents_Search ();
		}HC_Close_Segment ();
	}HC_Close_Segment ();

	m_pHView->SetGeometryChanged ();
	m_pHView->Update ();
}

void HoopsView::update_mesh_groups ()
{
	std::vector<VolumeMeshElementGroup*> groups;
	std::vector<VolumeMeshElementGroup*> invisible_groups;
	std::vector<VolumeMeshElementGroup*> highlighted_groups;
	get_mesh_groups (groups, invisible_groups, highlighted_groups);
	foreach (auto group, groups){
		update_mesh_group (group);
	}

	foreach (auto group, invisible_groups){
		update_mesh_group (group);
	}

	foreach (auto group, highlighted_groups){
		update_mesh_group (group);
	}
}

void HoopsView::update_mesh_group_rendering (VolumeMeshElementGroup *group, MeshGroupRenderParam *param)
{
	if (!mesh_group_exists (group)) return;

	HC_KEY group_key = POINTER_TO_KEY (group);
	HC_Open_Segment_By_Key (group_key);{
		HC_Open_Segment ("groupvertices");{
			if (param->vertex_size != 0){
				HC_Set_Marker_Size (param->vertex_size);
			}
			if (param->vertex_color != ""){
				QString color_str = QString ("markers=%1").arg (param->vertex_color);
				HC_Set_Color (color_str.toAscii ().data ());
			}
			if (!param->vertex_visible){
				HC_Set_Visibility ("everything=off");
			}else{
				HC_UnSet_Visibility ();
			}
		}HC_Close_Segment ();

		HC_Open_Segment ("groupedges");{
			if (param->edge_weight != 0){
				HC_Set_Line_Weight (param->edge_weight);
			}
			if (param->edge_color != ""){
				QString color_str = QString ("lines=%1").arg (param->edge_color);
				HC_Set_Color (color_str.toAscii ().data ());
			}
			if (!param->edge_visible){
				HC_Set_Visibility ("everything=off");
			}else{
				HC_UnSet_Visibility ();
			}
		}HC_Close_Segment ();

		HC_Open_Segment ("groupfaces");{
			if (param->face_color != ""){
				QString color_str = QString ("faces=%1").arg (param->face_color);
				HC_Set_Color (color_str.toAscii ().data ());
			}
			if (!param->face_visible){
				HC_Set_Visibility ("everything=off");
			}else{
				HC_UnSet_Visibility ();
			}
		}HC_Close_Segment ();

		HC_Open_Segment ("groupcells");{
			if (param->cell_color != ""){
				QString color_str = QString ("faces=%1,edges=%2").arg (param->cell_color).arg (param->cell_color);
				HC_Set_Color (color_str.toAscii ().data ());
			}
			if (!param->cell_visible){
				HC_Set_Visibility ("everything=off");
			}else{
				HC_UnSet_Visibility ();
			}
		}HC_Close_Segment ();
	}HC_Close_Segment ();
}

void HoopsView::highlight_mesh_group (VolumeMeshElementGroup *group)
{
	if (is_mesh_group_highlighted (group)) return;

	HC_KEY group_key = POINTER_TO_KEY (group);
	OPEN_HIGHLIGHT_SEG
			HC_Move_By_Key (group_key, ".");
	CLOSE_HIGHLIGHT_SEG
	
	m_pHView->Update ();
}

void HoopsView::dehighlight_mesh_group (VolumeMeshElementGroup *group)
{
	if (!is_mesh_group_highlighted (group)) return;

	HC_KEY group_key = POINTER_TO_KEY (group);
	OPEN_GROUPS_SEG
		HC_Move_By_Key (group_key, ".");
	CLOSE_GROUPS_SEG

		m_pHView->Update ();
}

bool HoopsView::is_mesh_group_highlighted (VolumeMeshElementGroup *group)
{
	HC_KEY group_key = POINTER_TO_KEY (group);
	char child[255];
	bool result = false;
	OPEN_HIGHLIGHT_SEG
			HC_Begin_Segment_Search ("*");{
				while (HC_Find_Segment (child)){
					HC_KEY key = HC_Open_Segment (child);
					HC_Close_Segment ();
					if (key == group_key){
						result = true;
						break;
					}
				}
			}HC_End_Segment_Search ();
	CLOSE_HIGHLIGHT_SEG
	return result;
}