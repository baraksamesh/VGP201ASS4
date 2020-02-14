#include "Movable.h"

Movable::Movable()
{
	Tin = Eigen::Transform<float, 3, Eigen::Affine>::Identity();
	Tout = Eigen::Transform<float, 3, Eigen::Affine>::Identity();
}

Eigen::Matrix4f Movable::MakeTrans()
{
	return (Tout * Tin).matrix();
}

void Movable::MyTranslate(Eigen::Vector3f amt)
{
	Tout.pretranslate(amt);
}

void Movable::MyTranslate(Eigen::Vector3f amt, Movable* scene)
{
	Eigen::Matrix3f mat = scene->Tout.rotation().matrix();
	mat.transposeInPlace();
	MyTranslate(mat * amt);
}

//angle in radians
void Movable::MyRotate(Eigen::Vector3f rotAxis, float angle)
{
	Tout.rotate(Eigen::AngleAxisf(angle, rotAxis));
}


void Movable::MyRotate(Eigen::Vector3f rotAxis, float angle, bool self)
{
	Eigen::Matrix3f mat = Tout.rotation().matrix();
	mat.transposeInPlace();
	if (self)
		Tout.rotate(Eigen::AngleAxisf(angle, rotAxis));
	else
		Tout.rotate(Eigen::AngleAxisf(angle, mat * rotAxis));
}

void Movable::MyRotate(Eigen::Vector3f rotAxis, float angle, Movable* scene)
{
	Eigen::Matrix3f mat = scene->Tout.rotation().matrix();
	mat.transposeInPlace();
	MyRotate(mat * rotAxis, angle);
}

void Movable::MyScale(Eigen::Vector3f amt)
{
	Tin.scale(amt);
}

void Movable::SetCenterOfRotation(Eigen::Vector3f amt) {
	Tin.translate(-amt);
	Tout.translate(amt);
}

Eigen::Vector3f Movable::GetCenterOfRotation() {
	return -Tin.translation();
}

Eigen::Matrix3f Movable::GetRotation() {
	return Tout.rotation().matrix();

}

Eigen::Vector3f Movable::GetTranslation() {
	return (Tout * Tin).matrix().block(0, 3, 3, 1);
}

void Movable::SetTranslation(Eigen::Vector3f position)
{
	Eigen::Vector3f old_position = (Tout * Tin).matrix().block(0, 3, 3, 1);
	Tout.pretranslate( -old_position + position);
}

void Movable::LookAt(Eigen::Vector3f forward)
{
	Tin = Eigen::Transform<float, 3, Eigen::Affine>::Identity();
	Tout = Eigen::Transform<float, 3, Eigen::Affine>::Identity();

	Eigen::Vector3f right = Eigen::Vector3f(0, 1, 0).cross(forward);
	Eigen::Vector3f up = forward.cross(right);

	Eigen::Matrix3f R;
	/**R << right[0], right[1], right[2],
		up[0], up[1], up[2],
		forward[0], forward[1], forward[2];**/

	R << right[0], up[0], forward[0],
		right[1], up[1], forward[1],
		right[2], up[2], forward[2];

	float angleX = 0;
	float angleY = 0;
	float angleZ = 0;

	if (R.row(0)[2] < 1) {
		if (R.row(0)[2] > -1) {
			angleY = asinf(R.row(0)[2]);
			angleX = atan2f(-R.row(1)[2], R.row(2)[2]);
			angleZ = atan2f(-R.row(0)[1], R.row(0)[0]);
		}
		else {
			angleY = -M_PI / 2;
			angleX = -atan2f(-R.row(1)[0], R.row(1)[1]);
			angleZ = 0;
		}
	}
	else {
		angleY = M_PI / 2;
		angleX = atan2f(R.row(1)[0], R.row(1)[1]);
		angleZ = 0;

	}

	Tout.rotate(Eigen::AngleAxisf(angleX, Eigen::Vector3f(1,0,0)));
	Tout.rotate(Eigen::AngleAxisf(angleY, Eigen::Vector3f(0,1,0)));
	Tout.rotate(Eigen::AngleAxisf(angleZ, Eigen::Vector3f(0,0,1)));

}

void Movable::Reset() {


	Tin = Eigen::Transform<float, 3, Eigen::Affine>::Identity();
	Tout = Eigen::Transform<float, 3, Eigen::Affine>::Identity();

}
