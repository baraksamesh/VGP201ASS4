// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozillA->org/MPL/2.0/.

#include "Viewer.h"

//#include <chrono>
#include <thread>

#include <Eigen/LU>


#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <cassert>

#include <igl/project.h>
//#include <igl/get_seconds.h>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/adjacency_list.h>
#include <igl/writeOBJ.h>
#include <igl/writeOFF.h>
#include <igl/massmatrix.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/quat_mult.h>
#include <igl/axis_angle_to_quat.h>
#include <igl/trackball.h>
#include <igl/two_axis_valuator_fixed_up.h>
#include <igl/snap_to_canonical_view_quat.h>
#include <igl/unproject.h>
#include <igl/serialize.h>
#include <igl/get_seconds.h>

// Internal global variables used for glfw event handling
//static igl::opengl::glfw::Viewer * __viewer;
static double highdpi = 1;
static double scroll_x = 0;
static double scroll_y = 0;



namespace igl
{
	namespace opengl
	{
		namespace glfw
		{

			IGL_INLINE void Viewer::init()
			{
				lastTimeStamp = 0;

			}

			//IGL_INLINE void Viewer::init_plugins()
			//{
			//  // Init all plugins
			//  for (unsigned int i = 0; i<plugins.size(); ++i)
			//  {
			//    plugins[i]->init(this);
			//  }
			//}

			//IGL_INLINE void Viewer::shutdown_plugins()
			//{
			//  for (unsigned int i = 0; i<plugins.size(); ++i)
			//  {
			//    plugins[i]->shutdown();
			//  }
			//}

			IGL_INLINE Viewer::Viewer() :
				data_list(1),
				selected_data_index(0),
				next_data_id(1)
			{
				data_list.front().id = 0;



				// Temporary variables initialization
			   // down = false;
			  //  hack_never_moved = true;
				scroll_position = 0.0f;

				// Per face
				data().set_face_based(false);


#ifndef IGL_VIEWER_VIEWER_QUIET
				const std::string usage(R"(igl::opengl::glfw::Viewer usage:
  [drag]  Rotate scene
  A,a     Toggle animation (tight draw loop)
  F,f     Toggle face based
  I,i     Toggle invert normals
  L,l     Toggle wireframe
  O,o     Toggle orthographic/perspective projection
  T,t     Toggle filled faces
  [,]     Toggle between cameras
  1,2     Toggle between models
  ;       Toggle vertex labels
  :       Toggle face labels)"
				);
				std::cout << usage << std::endl;
#endif
			}

			IGL_INLINE Viewer::~Viewer()
			{
			}

			IGL_INLINE bool Viewer::load_mesh_from_file(
				const std::string& mesh_file_name_string)
			{

				// Create new data slot and set to selected
				if (!(data().F.rows() == 0 && data().V.rows() == 0))
				{
					append_mesh();
				}
				data().clear();

				size_t last_dot = mesh_file_name_string.rfind('.');
				if (last_dot == std::string::npos)
				{
					std::cerr << "Error: No file extension found in " <<
						mesh_file_name_string << std::endl;
					return false;
				}

				std::string extension = mesh_file_name_string.substr(last_dot + 1);

				if (extension == "off" || extension == "OFF")
				{
					Eigen::MatrixXd V;
					Eigen::MatrixXi F;
					if (!igl::readOFF(mesh_file_name_string, V, F))
						return false;
					data().set_mesh(V, F);
				}
				else if (extension == "obj" || extension == "OBJ")
				{
					Eigen::MatrixXd corner_normals;
					Eigen::MatrixXi fNormIndices;

					Eigen::MatrixXd UV_V;
					Eigen::MatrixXi UV_F;
					Eigen::MatrixXd V;
					Eigen::MatrixXi F;

					if (!(
						igl::readOBJ(
							mesh_file_name_string,
							V, UV_V, corner_normals, F, UV_F, fNormIndices)))
					{
						return false;
					}

					data().set_mesh(V, F);
					data().set_uv(UV_V, UV_F);

				}
				else
				{
					// unrecognized file type
					printf("Error: %s is not a recognized file type.\n", extension.c_str());
					return false;
				}

				data().compute_normals();
				data().uniform_colors(Eigen::Vector3d(51.0 / 255.0, 43.0 / 255.0, 33.3 / 255.0),
					Eigen::Vector3d(255.0 / 255.0, 228.0 / 255.0, 58.0 / 255.0),
					Eigen::Vector3d(255.0 / 255.0, 235.0 / 255.0, 80.0 / 255.0));

				// Alec: why?
				if (data().V_uv.rows() == 0)
				{
					data().grid_texture();
				}


				//for (unsigned int i = 0; i<plugins.size(); ++i)
				//  if (plugins[i]->post_load())
				//    return true;

				return true;
			}

			IGL_INLINE bool Viewer::save_mesh_to_file(
				const std::string& mesh_file_name_string)
			{
				// first try to load it with a plugin
				//for (unsigned int i = 0; i<plugins.size(); ++i)
				//  if (plugins[i]->save(mesh_file_name_string))
				//    return true;

				size_t last_dot = mesh_file_name_string.rfind('.');
				if (last_dot == std::string::npos)
				{
					// No file type determined
					std::cerr << "Error: No file extension found in " <<
						mesh_file_name_string << std::endl;
					return false;
				}
				std::string extension = mesh_file_name_string.substr(last_dot + 1);
				if (extension == "off" || extension == "OFF")
				{
					return igl::writeOFF(
						mesh_file_name_string, data().V, data().F);
				}
				else if (extension == "obj" || extension == "OBJ")
				{
					Eigen::MatrixXd corner_normals;
					Eigen::MatrixXi fNormIndices;

					Eigen::MatrixXd UV_V;
					Eigen::MatrixXi UV_F;

					return igl::writeOBJ(mesh_file_name_string,
						data().V,
						data().F,
						corner_normals, fNormIndices, UV_V, UV_F);
				}
				else
				{
					// unrecognized file type
					printf("Error: %s is not a recognized file type.\n", extension.c_str());
					return false;
				}
				return true;
			}

			IGL_INLINE bool Viewer::load_scene()
			{
				std::string fname = igl::file_dialog_open();
				if (fname.length() == 0)
					return false;
				return load_scene(fname);
			}

			IGL_INLINE bool Viewer::load_scene(std::string fname)
			{
				// igl::deserialize(core(),"Core",fname.c_str());
				igl::deserialize(data(), "Data", fname.c_str());
				return true;
			}

			IGL_INLINE bool Viewer::save_scene()
			{
				std::string fname = igl::file_dialog_save();
				if (fname.length() == 0)
					return false;
				return save_scene(fname);
			}

			IGL_INLINE bool Viewer::save_scene(std::string fname)
			{
				//igl::serialize(core(),"Core",fname.c_str(),true);
				igl::serialize(data(), "Data", fname.c_str());

				return true;
			}

			IGL_INLINE void Viewer::open_dialog_load_mesh()
			{
				std::string fname = igl::file_dialog_open();

				if (fname.length() == 0)
					return;

				this->load_mesh_from_file(fname.c_str());
			}

			IGL_INLINE void Viewer::open_dialog_save_mesh()
			{
				std::string fname = igl::file_dialog_save();

				if (fname.length() == 0)
					return;

				this->save_mesh_to_file(fname.c_str());
			}

			IGL_INLINE ViewerData& Viewer::data(int mesh_id /*= -1*/)
			{
				assert(!data_list.empty() && "data_list should never be empty");
				int index;
				if (mesh_id == -1)
					index = selected_data_index;
				else
					index = mesh_index(mesh_id);

				assert((index >= 0 && index < data_list.size()) &&
					"selected_data_index or mesh_id should be in bounds");
				return data_list[index];
			}

			IGL_INLINE const ViewerData& Viewer::data(int mesh_id /*= -1*/) const
			{
				assert(!data_list.empty() && "data_list should never be empty");
				int index;
				if (mesh_id == -1)
					index = selected_data_index;
				else
					index = mesh_index(mesh_id);

				assert((index >= 0 && index < data_list.size()) &&
					"selected_data_index or mesh_id should be in bounds");
				return data_list[index];
			}

			IGL_INLINE int Viewer::append_mesh(bool visible /*= true*/)
			{
				assert(data_list.size() >= 1);

				data_list.emplace_back();
				selected_data_index = data_list.size() - 1;
				data_list.back().id = next_data_id++;
				//if (visible)
				//    for (int i = 0; i < core_list.size(); i++)
				//        data_list.back().set_visible(true, core_list[i].id);
				//else
				//    data_list.back().is_visible = 0;
				return data_list.back().id;
			}

			IGL_INLINE bool Viewer::erase_mesh(const size_t index)
			{
				assert((index >= 0 && index < data_list.size()) && "index should be in bounds");
				assert(data_list.size() >= 1);
				if (data_list.size() == 1)
				{
					// Cannot remove last mesh
					return false;
				}
				data_list[index].meshgl.free();
				data_list.erase(data_list.begin() + index);
				if (selected_data_index >= index && selected_data_index > 0)
				{
					selected_data_index--;
				}

				return true;
			}

			IGL_INLINE size_t Viewer::mesh_index(const int id) const {
				for (size_t i = 0; i < data_list.size(); ++i)
				{
					if (data_list[i].id == id)
						return i;
				}
				return 0;
			}



			///////////////////////////////////////////////////////////////////////////////////////////////////////
			/// IK ////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////



			void Viewer::loadIk(int links)
			{
				parents.resize(links + 1);
				iFirstLink = 0;
				iLastLink = links - 1;
				iSphere = links;
				std::ifstream file("configuration.txt");
				std::string line;
				bool is_exe = false;
				if (!std::getline(file, line)) {
					line = "C:/VGP201/EngineForAnimationCourse/tutorial/data/ycylinder.obj";
				}
				float y_top = 0.8;
				for (int i = 0; i < links; i++) {
					load_mesh_from_file(line);
					data().SetCenterOfRotation(Eigen::Vector3f(data().V.colwise().mean()[0], data().V.colwise().minCoeff()[1], data().V.colwise().mean()[2]));
					y_top = data().V.colwise().maxCoeff()[1];
					data().MyTranslate(Eigen::Vector3f(0, 2 * y_top, 0));
					make_axis();
					parents[i] = i - 1;
				}
				float link_size = y_top * 2;
				MyTranslate(Eigen::Vector3f(0, -2, -9));
				data(0).MyTranslate(Eigen::Vector3f(0, -link_size, 0));

				if (!std::getline(file, line)) {
					line = "C:/VGP201/EngineForAnimationCourse/tutorial/data/sphere.obj";
				}
				std::getline(file, line);
				load_mesh_from_file(line);
				data().MyTranslate(Eigen::Vector3f(5, 0, 0));
				Eigen::MatrixXd color(1, 3);
				color << 1, 0, 0;
				data().set_colors(color);
				parents[links] = links;
			}

			void Viewer::make_axis() {

				float length = data().V.colwise().maxCoeff()[1] - data().V.colwise().minCoeff()[1];
				Eigen::Vector3f mid = data().GetCenterOfRotation();
				mid[1] = mid[1] + length;
				// Corners of the bounding box
				Eigen::MatrixXd V_Axis(7, 3);
				V_Axis <<
					mid(0) + length, mid(1), mid(2), // x
					mid(0) - length, mid(1), mid(2), // x
					mid(0), mid(1) + length, mid(2), // y
					mid(0), mid(1) - length, mid(2), // y
					mid(0), mid(1), mid(2) + length, // z
					mid(0), mid(1), mid(2) - length, // z
					mid(0), mid(1), mid(2); // mid


				// Edges of the bounding box
				Eigen::MatrixXi E_axis(3, 2);
				E_axis <<
					0, 1,
					2, 3,
					4, 5;

				// Plot the corners of the bounding box as points
				data().add_points(V_Axis.row(6), Eigen::RowVector3d(0, 0, 1));

				// Plot the edges of the bounding box
				data().add_edges
				(
					V_Axis.row(0),
					V_Axis.row(1),
					Eigen::RowVector3d(1, 0, 0)
				);
				data().add_edges
				(
					V_Axis.row(2),
					V_Axis.row(3),
					Eigen::RowVector3d(0, 1, 0)
				);
				data().add_edges
				(
					V_Axis.row(4),
					V_Axis.row(5),
					Eigen::RowVector3d(0, 0, 1)
				);
				data().show_overlay_depth = false;
				data().point_size = 10;
				data().line_width = 3;
				data().show_lines = false;
			}

			int igl::opengl::glfw::Viewer::getParentIndex(int index)
			{
				return parents[index];
			}


			Eigen::Matrix4f Viewer::MakeParentTrans(int mesh_id) {
				if (getParentIndex(mesh_id) == -1 || getParentIndex(mesh_id) == mesh_id)
					return Eigen::Transform<float, 3, Eigen::Affine>::Identity().matrix();
				Eigen::Matrix4f t = data_list[getParentIndex(mesh_id)].MakeTrans();
				Eigen::Matrix4f tp = MakeParentTrans(getParentIndex(mesh_id));
				return MakeParentTrans(getParentIndex(mesh_id)) * data_list[getParentIndex(mesh_id)].MakeTrans();
			}

			void Viewer::ik() {
				//iking = false;
				Eigen::Vector3f d = data(iSphere).MakeTrans().block(0, 1, 3, 3).col(2);
				Eigen::Vector3f arm_base = data(iFirstLink).MakeTrans().block(0, 1, 3, 3).col(2);
				float link_size = 2 * data(iFirstLink).V.colwise().maxCoeff()[1];
				float num_of_links = iLastLink + 1;
				if ((d - arm_base).norm() > link_size* num_of_links) {
					iking = false;
					std::cout << "cannot reach" << std::endl;
					return;
				}
				int i = iLastLink;
				while (i != -1 || i > iLastLink)
				{
					//iking = false;
					Eigen::Vector4f rCenter(data(i).GetCenterOfRotation()[0], data(i).GetCenterOfRotation()[1], data(i).GetCenterOfRotation()[2], 1);
					Eigen::Vector4f r4 = MakeParentTrans(i) * data(i).MakeTrans() * rCenter;
					Eigen::Vector3f r = r4.head<3>();

					Eigen::Vector4f eCenter(data(iLastLink).GetCenterOfRotation()[0], data(iLastLink).GetCenterOfRotation()[1] + 1.6, data(iLastLink).GetCenterOfRotation()[2], 1);
					Eigen::Vector4f e4 = MakeParentTrans(iLastLink) * data(iLastLink).MakeTrans() * eCenter;
					Eigen::Vector3f e = e4.head<3>();

					Eigen::Vector3f rd = d - r;
					Eigen::Vector3f re = e - r;

					float distance = (d - e).norm();

					if (distance < 0.1) {
						std::cout << "distance: " << distance << std::endl;
						iking = false;
						fin_rotate();
						return;
					}

					Eigen::Vector3f rotation_axis = GetParentsRotationInverse(i) * re.cross(rd).normalized();
					float dot = rd.normalized().dot(re.normalized());
					if (dot > 1)
						dot = 1;
					if (dot < -1)
						dot = -1;
					float angle = distance < 1 ? acosf(dot) : acosf(dot) / 10;
					//--------------bonus------------------------- 
					int parent_i = getParentIndex(i);
					if (parent_i != -1) {
						//update the vector re
						data(i).MyRotate(rotation_axis, angle);
						eCenter = Eigen::Vector4f(data(iLastLink).GetCenterOfRotation()[0], data(iLastLink).GetCenterOfRotation()[1] + 1.6, data(iLastLink).GetCenterOfRotation()[2], 1);
						e4 = MakeParentTrans(iLastLink) * data(iLastLink).MakeTrans() * eCenter;
						e = e4.head<3>();
						re = e - r;
						//get parent vector
						Eigen::Vector4f parent_rCenter(data(parent_i).GetCenterOfRotation()[0], data(parent_i).GetCenterOfRotation()[1], data(parent_i).GetCenterOfRotation()[2], 1);
						Eigen::Vector4f parent_r4 = MakeParentTrans(parent_i) * data(parent_i).MakeTrans() * parent_rCenter;
						Eigen::Vector3f parent_r = parent_r4.head<3>();
						Eigen::Vector3f parent_vec = parent_r - r;

						//find angle
						float parent_dot = parent_vec.normalized().dot(re.normalized());
						if (parent_dot > 1)
							parent_dot = 1;
						if (parent_dot < -1)
							parent_dot = -1;
						float parent_angle = acosf(parent_dot);
						float deg2rad = 0.017453292;

						//constrain
						float constrain = 30 * deg2rad;
						float fix = 0;
						if (parent_angle < constrain) {
							fix = constrain - parent_angle;
						}
						data(i).MyRotate(rotation_axis, -angle); //rollback
						angle -= fix;
					}
					//--------------bonus-------------------------
					data(i).MyRotate(rotation_axis, angle);

					i = getParentIndex(i);
				}
			}

			Eigen::Matrix3f Viewer::GetParentsRotationInverse(int index) {

				Eigen::Matrix3f parentsInverse = data(index).GetRotation().inverse();
				int i = getParentIndex(index);
				while (i != -1)
				{
					parentsInverse = parentsInverse * data(i).GetRotation().inverse();
					i = getParentIndex(i);
				}

				return parentsInverse;
			}

			void Viewer::fin_rotate()
			{
				Eigen::Vector3f Y(0, 1, 0);
				for (int i = iFirstLink; i <= iLastLink; i++) {

					Eigen::Matrix3f R = data(i).GetRotation();

					float r00 = R.row(0)[0];
					float r01 = R.row(0)[1];
					float r02 = R.row(0)[2];
					float r10 = R.row(1)[0];
					float r11 = R.row(1)[1];
					float r12 = R.row(1)[2];
					float r20 = R.row(2)[0];
					float r21 = R.row(2)[1];
					float r22 = R.row(2)[2];

					float angleY0;
					float angleX;
					float angleY1;

					//YXY
					if (r11 < 1) {
						if (r11 > -1) {
							angleX = acosf(r11);
							angleY0 = atan2f(r01, r21);
							angleY1 = atan2f(r10, -r12);
						}
						else { //r11 = -1
							angleX = acosf(-1);
							angleY0 = -atan2f(r02, r00);
							angleY1 = 0;
						}
					}
					else {//r11 = 1
						angleX = 0;
						angleY0 = atan2f(r02, r00);
						angleY1 = 0;
					}

					data(i).MyRotate(Y, -angleY1);
					if (i != iLastLink)
						data(i + 1).MyRotate(Y, angleY1, false);
				}
			}

			///////////////////////////////////////////////////////////////////////////////////////////////////////
			/// COLLISION ////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////////////

			void Viewer::move() {
				//std::cout << get_seconds()/10000000000 << std::endl;
				data(0).MyTranslate(dir * speed * get_seconds() / 10000000000);
			}

			void Viewer::make_tree(Eigen::MatrixXd V, Eigen::MatrixXi F, int idx) {

				igl::AABB<Eigen::MatrixXd, 3> tree;
				tree.init(V, F);
				trees[idx] = tree;
				add_box(tree, idx, Eigen::RowVector3d(0, 1, 0));
			}

			void Viewer::initBoxes() {
				for (int i = 0; i < data_list.size(); i++) {
					add_box(trees[i], i, Eigen::RowVector3d(0, 1, 0), true);
				}
			}

			void Viewer::add_box(igl::AABB<Eigen::MatrixXd, 3>& tree, int idx, Eigen::RowVector3d color, bool deleteOld) {

				Eigen::RowVector3d blc = tree.m_box.corner(tree.m_box.BottomLeftCeil);
				Eigen::RowVector3d brc = tree.m_box.corner(tree.m_box.BottomRightCeil);
				Eigen::RowVector3d blf = tree.m_box.corner(tree.m_box.BottomLeftFloor);
				Eigen::RowVector3d brf = tree.m_box.corner(tree.m_box.BottomRightFloor);
				Eigen::RowVector3d tlc = tree.m_box.corner(tree.m_box.TopLeftCeil);
				Eigen::RowVector3d trc = tree.m_box.corner(tree.m_box.TopRightCeil);
				Eigen::RowVector3d tlf = tree.m_box.corner(tree.m_box.TopLeftFloor);
				Eigen::RowVector3d trf = tree.m_box.corner(tree.m_box.TopRightFloor);

				Eigen::MatrixXd dots(8, 3);
				dots << blc, brc, blf, brf, tlc, trc, tlf, trf;

				Eigen::MatrixXi edges(12, 2);
				edges << 0, 1,
					0, 2,
					1, 3,
					2, 3,
					4, 5,
					4, 6,
					5, 7,
					6, 7,
					0, 4,
					3, 7,
					1, 5,
					2, 6;

				if (deleteOld)
					data(idx).set_edges(dots, edges, color);
				else {
					data(idx).add_edges(blc, brc, color);
					data(idx).add_edges(blc, blf, color);
					data(idx).add_edges(brc, brf, color);
					data(idx).add_edges(blf, brf, color);
					data(idx).add_edges(tlc, trc, color);
					data(idx).add_edges(tlc, tlf, color);
					data(idx).add_edges(trc, trf, color);
					data(idx).add_edges(tlf, trf, color);
					data(idx).add_edges(blc, tlc, color);
					data(idx).add_edges(brf, trf, color);
					data(idx).add_edges(brc, trc, color);
					data(idx).add_edges(blf, tlf, color);
				}


				data(idx).line_width = 2;
				data(idx).show_lines = false;
				data(idx).show_overlay_depth = false;

			}


			bool Viewer::are_trees_touching(Eigen::AlignedBox3d box1, Eigen::AlignedBox3d box2, int idx1, int idx2, Eigen::Matrix3d* A, Eigen::Matrix3d* B, Eigen::Matrix3d* C) {

				float a0 = box1.sizes()[0] / 2;
				float a1 = box1.sizes()[1] / 2;
				float a2 = box1.sizes()[2] / 2;

				float b0 = box2.sizes()[0] / 2;
				float b1 = box2.sizes()[1] / 2;
				float b2 = box2.sizes()[2] / 2;

				Eigen::RowVector3d A0 = A->col(0);
				Eigen::RowVector3d A1 = A->col(1);
				Eigen::RowVector3d A2 = A->col(2);

				Eigen::Vector3d B0 = B->col(0);
				Eigen::Vector3d B1 = B->col(1);
				Eigen::Vector3d B2 = B->col(2);

				Eigen::Vector4d c1 = Eigen::Vector4d(box1.center()[0], box1.center()[1], box1.center()[2], 1);
				Eigen::Vector4d c2 = Eigen::Vector4d(box2.center()[0], box2.center()[1], box2.center()[2], 1);

				Eigen::Vector4d D4 = data(idx2).MakeTrans().cast<double>() * c2 - data(idx1).MakeTrans().cast<double>() * c1;
				Eigen::Vector3d D = D4.head(3);


				//A0
				double R0 = a0;
				double R1 = b0 * abs(C->row(0)[0]) + b1 * abs(C->row(0)[1]) + b2 * abs(C->row(0)[2]);
				double R = abs(A0.dot(D));
				if (R > R0 + R1)
					return false;
				//A1
				R0 = a1;
				R1 = b0 * abs(C->row(1)[0]) + b1 * abs(C->row(1)[1]) + b2 * abs(C->row(1)[2]);
				R = abs(A1.dot(D));
				if (R > R0 + R1)
					return false;
				//A2
				R0 = a2;
				R1 = b0 * abs(C->row(2)[0]) + b1 * abs(C->row(2)[1]) + b2 * abs(C->row(2)[2]);
				R = abs(A2.dot(D));
				if (R > R0 + R1)
					return false;
				//B0
				R0 = a0 * abs(C->row(0)[0]) + a1 * abs(C->row(1)[0]) + a2 * abs(C->row(2)[0]);
				R1 = b0;
				R = abs(B0.dot(D));
				if (R > R0 + R1)
					return false;
				//B1
				R0 = a0 * abs(C->row(0)[1]) + a1 * abs(C->row(1)[1]) + a2 * abs(C->row(2)[1]);
				R1 = b1;
				R = abs(B1.dot(D));
				if (R > R0 + R1)
					return false;
				//B2
				R0 = a0 * abs(C->row(0)[2]) + a1 * abs(C->row(1)[2]) + a2 * abs(C->row(2)[2]);
				R1 = b2;
				R = abs(B2.dot(D));
				if (R > R0 + R1)
					return false;
				//A0 X B0
				R0 = a1 * abs(C->row(2)[0]) + a2 * abs(C->row(1)[0]);
				R1 = b1 * abs(C->row(0)[2]) + b2 * abs(C->row(0)[1]);
				R = abs(C->row(1)[0] * A2.dot(D) - C->row(2)[0] * A1.dot(D));
				if (R > R0 + R1)
					return false;
				//A0 X B1
				R0 = a1 * abs(C->row(2)[1]) + a2 * abs(C->row(1)[1]);
				R1 = b0 * abs(C->row(0)[2]) + b2 * abs(C->row(0)[0]);
				R = abs(C->row(1)[1] * A2.dot(D) - C->row(2)[1] * A1.dot(D));
				if (R > R0 + R1)
					return false;
				//A0 X B2
				R0 = a1 * abs(C->row(2)[2]) + a2 * abs(C->row(1)[2]);
				R1 = b0 * abs(C->row(0)[1]) + b1 * abs(C->row(0)[0]);
				R = abs(C->row(1)[2] * A2.dot(D) - C->row(2)[2] * A1.dot(D));
				if (R > R0 + R1)
					return false;
				//A1 X B0
				R0 = a0 * abs(C->row(2)[0]) + a2 * abs(C->row(0)[0]);
				R1 = b1 * abs(C->row(1)[2]) + b2 * abs(C->row(1)[1]);
				R = abs(C->row(2)[0] * A0.dot(D) - C->row(0)[0] * A2.dot(D));
				if (R > R0 + R1)
					return false;
				//A1 X B1
				R0 = a0 * abs(C->row(2)[1]) + a2 * abs(C->row(0)[1]);
				R1 = b0 * abs(C->row(1)[2]) + b2 * abs(C->row(1)[0]);
				R = abs(C->row(2)[1] * A0.dot(D) - C->row(0)[1] * A2.dot(D));
				if (R > R0 + R1)
					return false;
				//A1 X B2
				R0 = a0 * abs(C->row(2)[2]) + a2 * abs(C->row(0)[2]);
				R1 = b0 * abs(C->row(1)[1]) + b1 * abs(C->row(1)[0]);
				R = abs(C->row(2)[2] * A0.dot(D) - C->row(0)[2] * A2.dot(D));
				if (R > R0 + R1)
					return false;
				//A2 X B0
				R0 = a0 * abs(C->row(1)[0]) + a1 * abs(C->row(0)[0]);
				R1 = b1 * abs(C->row(2)[2]) + b2 * abs(C->row(2)[1]);
				R = abs(C->row(0)[0] * A1.dot(D) - C->row(1)[0] * A0.dot(D));
				if (R > R0 + R1)
					return false;
				//A2 X B1
				R0 = a0 * abs(C->row(1)[1]) + a1 * abs(C->row(0)[1]);
				R1 = b0 * abs(C->row(2)[2]) + b2 * abs(C->row(2)[0]);
				R = abs(C->row(0)[1] * A1.dot(D) - C->row(1)[1] * A0.dot(D));
				if (R > R0 + R1)
					return false;
				//A2 X B2
				R0 = a0 * abs(C->row(1)[2]) + a1 * abs(C->row(0)[2]);
				R1 = b0 * abs(C->row(2)[1]) + b1 * abs(C->row(2)[0]);
				R = abs(C->row(0)[2] * A1.dot(D) - C->row(1)[2] * A0.dot(D));
				if (R > R0 + R1)
					return false;

				return true;
			}

			bool Viewer::collision_detection(int idx1, int idx2) {
				igl::AABB<Eigen::MatrixXd, 3> tree1 = trees[idx1];
				igl::AABB<Eigen::MatrixXd, 3> tree2 = trees[idx2];

				Eigen::Matrix3d A = data(idx1).GetRotation().cast<double>();
				Eigen::Matrix3d B = data(idx2).GetRotation().cast<double>();
				Eigen::Matrix3d C = A.transpose() * B;


				return collision_detection(tree1, tree2, idx1, idx2, &A, &B, &C);
			}

			bool Viewer::collision_detection(igl::AABB<Eigen::MatrixXd, 3>& tree1, igl::AABB<Eigen::MatrixXd, 3>& tree2, int idx1, int idx2, Eigen::Matrix3d* A, Eigen::Matrix3d* B, Eigen::Matrix3d* C) {

				bool touching = false;

				if (tree1.is_leaf() && tree2.is_leaf()) {
					add_box(tree1, 0, Eigen::RowVector3d(1, 1, 1));
					add_box(tree2, 1, Eigen::RowVector3d(1, 1, 1));
					return true;
				}

				else if (tree1.is_leaf() && !tree2.is_leaf()) {
					if (are_trees_touching(tree1.m_box, tree2.m_left->m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(tree1, *tree2.m_left, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
					if (are_trees_touching(tree1.m_box, tree2.m_right->m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(tree1, *tree2.m_right, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
				}

				else if (!tree1.is_leaf() && tree2.is_leaf()) {
					if (are_trees_touching(tree1.m_left->m_box, tree2.m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(*tree1.m_left, tree2, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
					if (are_trees_touching(tree1.m_right->m_box, tree2.m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(*tree1.m_right, tree2, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
				}
				else {//neither are leaves
					if (are_trees_touching(tree1.m_left->m_box, tree2.m_left->m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(*tree1.m_left, *tree2.m_left, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
					if (are_trees_touching(tree1.m_left->m_box, tree2.m_right->m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(*tree1.m_left, *tree2.m_right, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
					if (are_trees_touching(tree1.m_right->m_box, tree2.m_left->m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(*tree1.m_right, *tree2.m_left, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
					if (are_trees_touching(tree1.m_right->m_box, tree2.m_right->m_box, idx1, idx2, A, B, C)) {
						touching = collision_detection(*tree1.m_right, *tree2.m_right, idx1, idx2, A, B, C);
						if (touching)
							return true;
					}
				}
				return false;
			}

		} // end namespace
	} // end namespace
}