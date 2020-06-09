#include "Quaternion.h"

BRWL_NS


const Quaternion Quaternion::identity = Quaternion(0, 0, 0, 1);

Quaternion::Quaternion() : x(0), y(0), z(0), w(1)
{ }

Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{ }

Quaternion::Quaternion(const Vec3& axis, float angle)
{
	float halfAngle = angle * 0.5f;
	float sin_a = sin(halfAngle);
	x = axis.x * sin_a;
	y = axis.y * sin_a;
	z = axis.z * sin_a;
	w = cos(halfAngle);
	normalize();
}

Quaternion::Quaternion(const Quaternion& other)
{
	// other quaternion is expected to be already normalized
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
}


void Quaternion::toAxisAngle(const Quaternion& q, Vec3& outAxis, float& outAngle) {
	// from www.j3d.org/matrix_faq/matrfaq_latest.html
	float sin_a = sqrt(1 - q.w * q.w);
	if (abs(sin_a) < 0.0002) {
		outAxis.x = q.x;
		outAxis.y = q.y;
		outAxis.z = q.z;
	}

	outAxis.x = q.x / sin_a;
	outAxis.y = q.y / sin_a;
	outAxis.z = q.z / sin_a;
	outAngle = 2 * acos(q.w);
}

// todo: fix order of rotation z, x, y
// bank, attitude, heading
Vec3 Quaternion::toEuler(const Quaternion& q)
{
	float  x, y, z; // rotation around z, x, y or roll, pitch, yaw/head
	double test = q.x * q.y + q.z * q.w;
	if (test > 0.499) { // singularity at north pole
		y = (float)(2 * atan2(q.x, q.w));
		x = (float)(PI_F / 2);
		z = 0;
	}
	if (test < -0.499) { // singularity at south pole
		y = (-2 * atan2(q.x, q.w));
		x = (-PI_F / 2);
		z = 0;
	}
	else
	{
		double sqx = q.x * q.x;
		double sqy = q.y * q.y;
		double sqz = q.z * q.z;

		y = (float)atan2(2 * q.y * q.w - 2 * q.x * q.z, 1 - 2 * sqy - 2 * sqz);
		x = (float)asin(2 * test);
		z = (float)atan2(2 * q.x * q.w - 2 * q.y * q.z, 1 - 2 * sqx - 2 * sqz);
	}

	return Vec3(z, y, x);
}

Vec3 Quaternion::toEuler()
{
	return Quaternion::toEuler(*this);
}

Quaternion Quaternion::makeFromEuler(const Vec3& euler)
{
	return Quaternion().fromEuler(euler);
}

Quaternion& Quaternion::fromEuler(const Vec3& euler)
{
	return fromEuler(euler.x, euler.y, euler.z);
}

Quaternion& Quaternion::fromEuler(float eulerX, float eulerY, float eulerZ)
{
	float half = eulerY * 0.5f;
	float sp = sin(half);
	float cp = cos(half);
	half = eulerZ * 0.5f;
	float sy = sin(half);
	float cy = cos(half);
	half = eulerX * 0.5f;
	float sr = sin(half);
	float cr = cos(half);

	float cpcy = cp * cy;
	float spcy = sp * cy;
	float cpsy = cp * sy;
	float spsy = sp * sy;


	set
	(
		sr * cpcy - cr * spsy,
		cr * spcy + sr * cpsy,
		cr * cpsy - sr * spcy,
		cr * cpcy + sr * spsy
	);

	normalize();
	return *this;
}

Quaternion Quaternion::fromAxisAngle(const Vec3& axis, float angle)
{
	Quaternion q;
	Quaternion::fromAxisAngle(axis, angle, q);
	return q;
}

void Quaternion::fromAxisAngle(const Vec3& axis, float angle, Quaternion& out)
{
	Quaternion::fromAxisAngle(axis.x, axis.y, axis.z, angle, out);
}

void Quaternion::fromAxisAngle(float axisX, float axisY, float axisZ, float angle, Quaternion& out)
{
	float sin_a = sin(angle * 0.5f);
	float cos_a = cos(angle * 0.5f);
	out.x = axisX * sin_a;
	out.y = axisY * sin_a;
	out.z = axisZ * sin_a;
	out.w = cos_a;
	out.normalize();
}

float Quaternion::magnitude()
{
	return sqrt(x * x + y * y + z * z + w * w);
}

float Quaternion::angle()
{
	return 2.0f * acos(w);
}

Quaternion& Quaternion::normalize()
{
	float n = magnitude();
	if (n == 1) return *this;
	return *this *= (1.0f / n);
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion ret;
	ret.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
	ret.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
	ret.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
	ret.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);
	ret.normalize();
	return ret;
}

Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	float tmp_x = x;
	float tmp_y = y;
	float tmp_z = z;
	float tmp_w = w;
	w = (other.w * tmp_w) - (other.x * tmp_x) - (other.y * tmp_y) - (other.z * tmp_z);
	x = (other.w * tmp_x) + (other.x * tmp_w) + (other.y * tmp_z) - (other.z * tmp_y);
	y = (other.w * tmp_y) + (other.y * tmp_w) + (other.z * tmp_x) - (other.x * tmp_z);
	z = (other.w * tmp_z) + (other.z * tmp_w) + (other.x * tmp_y) - (other.y * tmp_x);
	normalize();
	return *this;
}

Quaternion Quaternion::operator*(float f) const
{
	return Quaternion(f * x, f * y, f * z, f * w);
}

Quaternion& Quaternion::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

Vec3 Quaternion::operator*(const Vec3& rhs) const
{
	Vec3 xyz(x, y, z);
	Vec3 t = cross(xyz, rhs) * 2.0f;
	return  rhs + t * w + cross(xyz, t);
}

Quaternion Quaternion::inverse() const
{
	return Quaternion(-x, -y, -z, w);
}

Quaternion& Quaternion::invert()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

Quaternion::operator Vec4() const
{
	return Vec4(x, y, z, w);
}

void Quaternion::set(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
	normalize();
}

void Quaternion::set(Quaternion other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
}

bool Quaternion::operator==(const Quaternion& other) const
{
	return x == other.x && y == other.y && z == other.z && w == other.w;
}

bool Quaternion::operator!=(const Quaternion& other) const
{
	return !(*this == other);
}

Vec3 Quaternion::forward() const
{
	return Vec3(
		2.0f * (x * z + w * y),
		2.0f * (y * z + w * x),
		-1.0f + 2.0f * (x * x + y * y)
	);
}

Vec3 Quaternion::up() const
{
	return Vec3(
		2.0f * (x * y - w * z),
		1.0f - 2.0f * (x * x + z * z),
		2.0f * (y * z + w * x)
	);
}

Vec3 Quaternion::right() const
{
	return Vec3(
		1.0f - 2.0f * (y * y + z * z),
		2.0f * (x * y + w * z),
		2.0f * (x * z - w * y)
	);
}


BRWL_NS_END