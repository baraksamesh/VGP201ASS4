#pragma once
#include <Eigen/core>
#include <Eigen/Geometry>


class Movable
{
public:
	Movable();
	Eigen::Matrix4f MakeTrans();
	void MyTranslate(Eigen::Vector3f amt);
	void MyTranslate(Eigen::Vector3f amt, Movable* scene);
	void MyRotate(Eigen::Vector3f rotAxis, float angle);
	void MyRotate(Eigen::Vector3f rotAxis, float angle, bool self);
	void MyRotate(Eigen::Vector3f rotAxis, float angle, Movable* scene);
	void MyScale(Eigen::Vector3f amt);
	void SetCenterOfRotation(Eigen::Vector3f amt);
	Eigen::Vector3f GetCenterOfRotation();
	Eigen::Matrix3f GetRotation();
	Eigen::Vector3f GetTranslation();
	void SetTranslation(Eigen::Vector3f position);
	void LookAt(Eigen::Vector3f forward);
	void Reset();
private:
	Eigen::Transform<float, 3, Eigen::Affine> Tin;
	Eigen::Transform<float, 3, Eigen::Affine> Tout;

};

